#include "libcmmk.h"

#include <unistd.h> /* getuid() */
#include <string.h> /* memset() */

#include <libusb-1.0/libusb.h>

/* Initialize keyboard layouts */
typedef int8_t keyboard_layout[6][22];

#include "keymap_eu.h"
#include "keymap_us.h"

static keyboard_layout const *keyboard_layouts[] = {
	[CMMK_LAYOUT_US_S] = &layout_us_s,
	[CMMK_LAYOUT_US_L] = &layout_us_l,
	[CMMK_LAYOUT_EU_S] = &layout_eu_s,
	[CMMK_LAYOUT_EU_L] = &layout_eu_l
};

/* Some global definitions */
enum {
	CMMK_USB_VENDOR = 0x2516,

	CMMK_USB_INTERFACE = 1,

	CMMK_USB_EP_IN = 0x04,
	CMMK_USB_EP_OUT = 0x83
};

static int send_command(libusb_device_handle *dev, unsigned char *data, size_t datasiz)
{
	int tx;

	if (libusb_interrupt_transfer(dev, CMMK_USB_EP_IN, data, datasiz, &tx, 0) != 0)
		return 1;

	if (libusb_interrupt_transfer(dev, CMMK_USB_EP_OUT, data, datasiz, &tx, 0) != 0)
		return 1;

	return 0;
}

/*
 * Attach to and detach from USB device
 */
int cmmk_attach(struct cmmk *state, int product, int layout)
{
	if (getuid() != 0)
		return 1;

	if (libusb_init(&state->cxt) != 0)
		goto out_step0;

	state->dev = libusb_open_device_with_vid_pid(
			state->cxt,
			CMMK_USB_VENDOR,
			product);

	state->product = product;
	state->layout = layout;

	if (state->dev == NULL)
		goto out_step1;

	if (libusb_kernel_driver_active(state->dev, CMMK_USB_INTERFACE))
		if (libusb_detach_kernel_driver(state->dev,  CMMK_USB_INTERFACE) != 0)
			goto out_step2;

	if (libusb_claim_interface(state->dev, CMMK_USB_INTERFACE) != 0)
		goto out_step2;

	return 0;

out_step2: libusb_close(state->dev);
out_step1: libusb_exit(state->cxt);
out_step0: return 1;
}


int cmmk_detach(struct cmmk *state)
{
	libusb_release_interface(state->dev, CMMK_USB_INTERFACE);
	libusb_attach_kernel_driver(state->dev, CMMK_USB_INTERFACE);

	libusb_close(state->dev);
	libusb_exit(state->cxt);

	return 0;
}

/*
 * Enter and leave direct control mode. Any control commands outside of control
 * mode are ignored.
 */
int cmmk_set_control_mode(struct cmmk *dev, int mode)
{
	unsigned char data[64] = {0x41, mode};

	return send_command(dev->dev, data, sizeof(data));
}

int cmmk_set_active_profile(struct cmmk *dev, int prof)
{
	unsigned char setprof[64] = {0x51, 0x00, 0x00, 0x00, prof};

	return send_command(dev->dev, setprof, sizeof(setprof));
}

int cmmk_get_active_profile(struct cmmk *dev)
{
	unsigned char getprof[64] = {0x52, 0x00};

	if (send_command(dev->dev, getprof, sizeof(getprof)) != 0)
		return -1;

	return getprof[4];
}

static int set_effect1(struct cmmk *dev, int eff)
{
	unsigned char data[64] = {0x41, 0x01};

	send_command(dev->dev, data, sizeof(data));

	data[0] = 0x51;
	data[1] = 0x28;
	data[4] = eff;

	return send_command(dev->dev, data, sizeof(data));
}


static int set_effect(
	struct cmmk *dev,
	int eff,
	int p1, int p2, int p3,
	struct rgb const *col1,
	struct rgb const *col2)
{
	unsigned char data[64] = {
		0x51, 0x2c, 0x00, 0x00, eff,  p1,   p2,   p3,
		0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (col1 != NULL) {
		data[10] = col1->R;
		data[11] = col1->G;
		data[12] = col1->B;
	}

	if (col2 != NULL) {
		data[13] = col2->R;
		data[14] = col2->G;
		data[15] = col2->B;
	}

	memset(data + 16, 0xff, 48);

	set_effect1(dev, eff);

	return send_command(dev->dev, data, sizeof(data));
}

int cmmk_set_effect_fully_lit(struct cmmk *dev, struct rgb const *color)
{
	return set_effect(dev, 0x00, 0x00, 0x00, 0xff, color, NULL);
}

int cmmk_set_effect_breathe(struct cmmk *dev, int speed, struct rgb const *color)
{
	return set_effect(dev, 0x01, speed, 0x00, 0xff, color, NULL);
}

int cmmk_set_effect_cycle(struct cmmk *dev, int speed)
{
	int speed_conv;

	/* For some reason, this effect uses its own set of speedsteps */
	switch (speed) {
	case CMMK_SPEED0: speed_conv = 0x96; break;
	case CMMK_SPEED1: speed_conv = 0x8c; break;
	case CMMK_SPEED2: speed_conv = 0x80; break;
	case CMMK_SPEED3: speed_conv = 0x6e; break;
	case CMMK_SPEED4: speed_conv = 0x68; break;
	default: return 1;
	}

	return set_effect(dev, 0x02, speed_conv, 0x00, 0xff, NULL, NULL);
}

int cmmk_set_effect_single(struct cmmk *dev, int speed,
		struct rgb const *active,
		struct rgb const *rest)
{
	return set_effect(dev, 0x03, speed, 0x00, 0xff, active, rest);
}

int cmmk_set_effect_wave(struct cmmk *dev, int speed, int direction, struct rgb const *color)
{
	return set_effect(dev, 0x04, speed, direction, 0xff, color, NULL);
}

int cmmk_set_effect_ripple(struct cmmk *dev, int speed, struct rgb const *color)
{
	return set_effect(dev, 0x05, speed, (color == NULL) ? 0x80 : 0x00, 0xff, color, NULL);
}

int cmmk_set_effect_cross(struct cmmk *dev, int speed,
		struct rgb const *active,
		struct rgb const *rest)
{
	return set_effect(dev, 0x06, speed, 0x00, 0xff, active, rest);
}

int cmmk_set_effect_raindrops(struct cmmk *dev, int speed,
		struct rgb const *drop,
		struct rgb const *sky)
{
	return set_effect(dev, 0x07, speed, 0x00, 0x0a, drop, sky);
}

int cmmk_set_effect_stars(struct cmmk *dev, int speed,
		struct rgb const *star,
		struct rgb const *sky)
{
	return set_effect(dev, 0x08, speed, 0x00, 0x0a, star, sky);
}

int cmmk_set_effect_snake(struct cmmk *dev, int speed)
{
	return set_effect(dev, 0x09, speed, 0x00, 0xff, NULL, NULL);
}

int cmmk_set_effect_customized(struct cmmk *dev)
{
	return set_effect1(dev, 0x0A);
}

int cmmk_set_effect_off(struct cmmk *dev)
{
	return set_effect1(dev, 0xFE);
}

int cmmk_from_row_col(struct cmmk *dev, unsigned row, unsigned col)
{
	keyboard_layout const *layout = keyboard_layouts[dev->layout];

	return (*layout)[row][col];
}

/*
 * Set the single key `key' (indized via enum cmmk_keycode) to the given color.
 */
int cmmk_set_single_key(struct cmmk *dev, int key, struct rgb const *col)
{
	unsigned char data[64] = {0xc0, 0x01, 0x01, 0x00, key, col->R, col->G, col->B};

	return send_command(dev->dev, data, sizeof(data));
}

/*
 * Set the entire keyboard to the given color.
 */
int cmmk_set_all_single(struct cmmk *dev, struct rgb const *col)
{
	unsigned char data[64] = {0xc0, 0x00, 0x00, 0x00, col->R, col->G, col->B};

	return send_command(dev->dev, data, sizeof(data));
}


/*
 * Set the entire keyboard in one step from the given map.
 *
 * colmap *must* be at least CMMK_KEYLIST_SIZE entries long.
 * Otherwise, segmentation faults ensue.
 *
 * Keys in the map are indized by their individual mappings, so
 * colmap[K_ESC] will address the ESC key, much like
 * set_single_key(..., K_ESC, ...) will.
 */
int cmmk_set_leds(struct cmmk *dev, struct rgb *colmap)
{
	unsigned char data[64];

	int i;
	int j;

	struct rgb *nextcol = colmap;

	for (i = 0; i < 8; ++i) {
		data[0] = 0xc0;
		data[1] = 0x02;
		data[2] = i*2;
		data[3] = 0x00;

		for (j = 0; j < 16; ++j) {
			int const offset = 4 + (j * 3);

			data[offset] = nextcol->R;
			data[offset + 1] = nextcol->G;
			data[offset + 2] = nextcol->B;

			++nextcol;
		}

		send_command(dev->dev, data, sizeof(data));
	}

	return 0;
}

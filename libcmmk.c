#include "libcmmk.h"

#include <unistd.h> /* getuid() */
#include <string.h> /* memset() */

#include <libusb-1.0/libusb.h>

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
int cmmk_attach(struct cmmk *state, int product)
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

/*
 * Translate row/col notation from the official SDK into a key code
 */
int cmmk_from_row_col(struct cmmk *dev, unsigned row, unsigned col)
{
	//(void)dev; /* TODO: use device information to determine layout */

	if (dev->product == CMMK_USB_MASTERKEYS_PRO_S) {
		/* Model: MK Pro S [Black/White] EU */
		int map[6][22] = {
			{K_ESC, K_F1, K_F2, K_F3, K_F4, -1, K_F5, K_F6, K_F7, K_F8, -1, K_F9, K_F10, K_F11, K_F12,
			K_PRTSCR, K_SCRLCK, K_PAUSE, -1, -1, -1, -1},

			{K_TICK, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_HYPHEN, K_PLUS, -1,
			K_BACKSPACE, K_INS, K_HOME, K_PGUP, -1, -1, -1, -1},

			{K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_LSQUARE_BRACKET, K_RSQUARE_BRACKET, -1, K_BACK_SLASH,
			K_DEL, K_END, K_PGDWN, -1, -1, -1, -1},

			{K_CAPSLCK, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_SEMICOLON, K_QUOTE, K_ENTER, -1, -1,
			-1, -1, -1, -1, -1, -1, -1},

			{K_LSHIFT, -1, K_Z, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_PERIOD, K_FORWARD_SLASH,
			K_RSHIFT, -1, -1, -1, K_ARROW_UP, -1, -1, -1, -1, -1},

			{K_LCTRL, K_LWIN, K_LALT, -1, -1, -1, K_SPACE, -1, -1, -1, K_ALTGR, K_RWIN, K_FN, -1,
			K_RCTRL, K_ARROW_LEFT, K_ARROW_DOWN, K_ARROW_RIGHT, -1, -1, -1, -1}
		};

		return map[row][col];
	} else {
		/* Model: MK Pro L [Black/White] EU/Ger */
		int map[6][22] = {
			{K_ESC, K_F1, K_F2, K_F3, K_F4, -1, K_F5, K_F6, K_F7, K_F8, -1, K_F9, K_F10, K_F11, K_F12,
			K_PRTSCR, K_SCRLCK, K_PAUSE, K_P1, K_P2, K_P3, K_P4},

			{K_CARRET, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0, K_QUESTION, K_TICK, -1,
			K_BACKSPACE, K_INS, K_HOME, K_PGUP, K_NUMLCK, K_NUMDIV, K_NUMMULT, K_NUMMINUS},

			{K_TAB, K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P, K_UE, K_PLUS, -1, K_ENTER,
			K_DEL, K_END, K_PGDWN, K_NUM7, K_NUM8, K_NUM9, K_NUMPLUS},

			{K_CAPSLCK, K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, K_OE, K_AE, K_HASH, -1, -1,
			-1, -1, -1, K_NUM4, K_NUM5, K_NUM6, -1},

			{K_LSHIFT, K_ANGLE_BRACKET, K_Y, K_X, K_C, K_V, K_B, K_N, K_M, K_COMMA, K_PERIOD, K_HYPHEN,
			1, -1, K_RSHIFT, -1, K_ARROW_UP, -1, K_NUM1, K_NUM2, K_NUM3, K_NUMENTER},

			{K_LCTRL, K_LWIN, K_LALT, -1, -1, -1, K_SPACE, -1, -1, -1, K_ALTGR, K_RWIN, K_FN, -1,
			K_RCTRL, K_ARROW_LEFT, K_ARROW_DOWN, K_ARROW_LEFT, K_NUM0, -1, K_NUMDEL, -1}
		};

		return map[row][col];
	}

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

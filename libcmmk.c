#include "libcmmk.h"

#include <unistd.h> /* getuid() */
#include <libusb-1.0/libusb.h>

/* Some global definitions */
enum {
	CMMK_USB_VENDOR = 0x2516,
	CMMK_USB_PRODUCT = 0x003b,

	CMMK_USB_INTERFACE = 1,

	CMMK_USB_EP_IN = 0x04,
	CMMK_USB_EP_OUT = 0x83
};

/*
 * TODO:
 *	- Function for converting the official SDK row/col coordinates to keymap entries
 */


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
int cmmk_attach(struct cmmk *state)
{
	if (getuid() != 0)
		return 1;

	if (libusb_init(&state->cxt) != 0)
		goto out_step0;

	state->dev = libusb_open_device_with_vid_pid(
			state->cxt,
			CMMK_USB_VENDOR,
			CMMK_USB_PRODUCT);

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
int cmmk_enable_control(struct cmmk *dev)
{
	unsigned char data[64] = {0x41, 0x02};

	return send_command(dev->dev, data, sizeof(data));
}

int cmmk_disable_control(struct cmmk *dev)
{
	unsigned char data[64] = {0x41, 0x00};

	return send_command(dev->dev, data, sizeof(data));
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

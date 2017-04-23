#ifndef LIBCMMK_H
#define LIBCMMK_H

#include <libusb-1.0/libusb.h>

#define CMMK_LAYOUT_EU 1 /* keymap_eu.h */
#define CMMK_LAYOUT_US 2 /* keymap_us.h, XXX: needs outside help for this one */

#define CMMK_LAYOUT CMMK_LAYOUT_EU

/* Include correct keymap */
#if CMMK_LAYOUT == CMMK_LAYOUT_EU
#	include "keymap_eu.h"
#else
#	include "keymap_us.h"
#endif

#define CMMK_KEYLIST_SIZE 128 /* 8x16 RGB values => 128 distinct */

struct rgb {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

enum cmmk_effect_speed {
	CMMK_SPEED0 = 0x46,
	CMMK_SPEED1 = 0x41,
	CMMK_SPEED2 = 0x38,
	CMMK_SPEED3 = 0x3D,
	CMMK_SPEED4 = 0x27,
};

/*
 * Attach to and detach from USB device
 */
struct cmmk {
	libusb_context *cxt;
	libusb_device_handle *dev;
};

int cmmk_attach(struct cmmk *state);
int cmmk_detach(struct cmmk *state);

/*
 * Enter and leave direct control mode. Any control commands outside of control
 * mode are ignored.
 */
int cmmk_enable_control(struct cmmk *dev);
int cmmk_disable_control(struct cmmk *dev);

int cmmk_set_effect_star(struct cmmk *dev, int speed,
		struct rgb const *star,
		struct rgb const *sky);

int cmmk_set_effect_raindrop(struct cmmk *dev, int speed,
		struct rgb const *drop,
		struct rgb const *sky);

/*
 * Translate row/col notation from the official SDK into a key code
 */
int cmmk_from_row_col(struct cmmk *dev, unsigned row, unsigned col);

/*
 * Set the single key `key' (indized via enum cmmk_keycode) to the given color.
 */
int cmmk_set_single_key(struct cmmk *dev, int key, struct rgb const *col);

/*
 * Set the entire keyboard to the given color.
 */
int cmmk_set_all_single(struct cmmk *dev, struct rgb const *col);

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
int cmmk_set_leds(struct cmmk *dev, struct rgb *colmap);

#endif /* !defined(LIBCMMK_H) */

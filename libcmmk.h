#ifndef LIBCMMK_H
#define LIBCMMK_H

#include <libusb-1.0/libusb.h>

#define CMMK_LAYOUT_EU 1 /* keymap_eu.h */
#define CMMK_LAYOUT_US 2 /* keymap_us.h, XXX: needs outside help for this one */

#define CMMK_LAYOUT CMMK_LAYOUT_US

/* Include correct keymap */
#if CMMK_LAYOUT == CMMK_LAYOUT_EU
#	include "keymap_eu.h"
#else
#	include "keymap_us.h"
#endif

#define CMMK_KEYLIST_SIZE 128 /* 8x16 RGB values => 128 distinct */

/*
 * If we have C99 support (which we do, because libusb-1.0 requires it...), define some handy
 * macros.
 */
#if __STDC_VERSION__ >= 199901L
	/* struct rgb from 0xRRGGBB */
	#define MKRGB(hex) (struct rgb){((hex) >> 16) & 0xFF, ((hex) >> 8) & 0xFF, (hex) & 0xFF }

	/* struct rgb from single parts */
	#define MKRGBS(r, g, b) (struct rgb){ (r), (g), (b) }
#endif

struct rgb {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

enum cmmk_product {
	CMMK_USB_MASTERKEYS_PRO_L = 0x003b,
	CMMK_USB_MASTERKEYS_PRO_S = 0x003c
};

enum cmmk_effect_speed {
	CMMK_SPEED0 = 0x46,
	CMMK_SPEED1 = 0x41,
	CMMK_SPEED2 = 0x38,
	CMMK_SPEED3 = 0x3D,
	CMMK_SPEED4 = 0x27
};

enum cmmk_wave_direction {
	CMMK_LEFT_TO_RIGHT = 0x00,
	CMMK_RIGHT_TO_LEFT = 0x04,
	CMMK_BACK_TO_FRONT = 0x02,
	CMMK_FRONT_TO_BACK = 0x06
};

enum cmmk_control_mode {
	/* Firmware controls everything */
	CMMK_FIRMWARE = 0x00,

	/* Firmware controlled effect, configured via software */
	CMMK_EFFECT = 0x01,

	/* Manual control of everything */
	CMMK_MANUAL = 0x02,

	/* Profile setup */
	CMMK_PROFILE_CUSTOMIZATION = 0x03
};

/*
 * Attach to and detach from USB device
 */
struct cmmk {
	libusb_context *cxt;
	libusb_device_handle *dev;
	int product;
};

int cmmk_attach(struct cmmk *state, int product);
int cmmk_detach(struct cmmk *state);

/*
 * Enter and leave direct control mode. Any control commands outside of control
 * mode are ignored. Enabling control mode while inside control mode will reset
 * active effect and allow direct control over LEDs.
 */
int cmmk_set_control_mode(struct cmmk *dev, int mode);

/* Only possible in profile mode */
int cmmk_set_active_profile(struct cmmk *dev, int prof);
int cmmk_get_active_profile(struct cmmk *dev);

/* Predefined effects */
int cmmk_set_effect_fully_lit(struct cmmk *dev, struct rgb const *color);

int cmmk_set_effect_breathe(struct cmmk *dev, int speed, struct rgb const *color);

int cmmk_set_effect_cycle(struct cmmk *dev, int speed);

int cmmk_set_effect_single(struct cmmk *dev, int speed,
		struct rgb const *active,
		struct rgb const *rest);

int cmmk_set_effect_wave(struct cmmk *dev, int speed, int direction, struct rgb const *color);

/* color == NULL => random */
int cmmk_set_effect_ripple(struct cmmk *dev, int speed, struct rgb const *color);

int cmmk_set_effect_cross(struct cmmk *dev, int speed,
		struct rgb const *active,
		struct rgb const *rest);

int cmmk_set_effect_raindrops(struct cmmk *dev, int speed,
		struct rgb const *drop,
		struct rgb const *sky);


int cmmk_set_effect_stars(struct cmmk *dev, int speed,
		struct rgb const *star,
		struct rgb const *sky);

int cmmk_set_effect_snake(struct cmmk *dev, int speed);
int cmmk_set_effect_customized(struct cmmk *dev);
int cmmk_set_effect_off(struct cmmk *dev);

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

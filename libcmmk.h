/*
 * This file is part of libcmmk.
 *
 * libcmmk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libcmmk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libcmmk.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBCMMK_H
#define LIBCMMK_H

#include <libusb-1.0/libusb.h>

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

enum cmmk_layout {
	CMMK_LAYOUT_US_S,
	CMMK_LAYOUT_US_L, /* TODO */
	CMMK_LAYOUT_EU_S, /* TODO */
	CMMK_LAYOUT_EU_L,

	CMMK_LAYOUT_INVAL /* end marker */
};

/* Apparently can be anything in range [0x00, 0x50].
 *  Over 0x50 it just stops animating */
enum cmmk_effect_speed {
	CMMK_SPEED0 = 0x46,
	CMMK_SPEED1 = 0x41,
	CMMK_SPEED2 = 0x38,
	CMMK_SPEED3 = 0x3D,
	CMMK_SPEED4 = 0x31
};

enum cmmk_wave_direction {
	CMMK_LEFT_TO_RIGHT = 0x00,
	CMMK_RIGHT_TO_LEFT = 0x04,
	CMMK_BACK_TO_FRONT = 0x02,
	CMMK_FRONT_TO_BACK = 0x06
};

enum cmmk_ripple_type {
	CMMK_RIPPLE_GIVEN_COLOR, /* use the given color */
	CMMK_RIPPLE_RANDOM_COLOR = 0x80 /* use a random color */
};

enum cmmk_control_mode {
	/* Firmware controls everything */
	CMMK_FIRMWARE = 0x00,

	/* Firmware controlled effect, configured via software */
	CMMK_EFFECT = 0x01,

	/* Manual control of everything */
	CMMK_MANUAL = 0x02,

	/* Profile setup (may actually be a misnomer, as saving the profile works
	 * in effect mode as well */
	CMMK_PROFILE_CUSTOMIZATION = 0x03
};

enum cmmk_effect_id {
	CMMK_EFFECT_FULLY_LIT = 0x00,
	CMMK_EFFECT_BREATHE = 0x01,
	CMMK_EFFECT_CYCLE = 0x02,
	CMMK_EFFECT_SINGLE = 0x03,
	CMMK_EFFECT_WAVE = 0x04,
	CMMK_EFFECT_RIPPLE = 0x05,
	CMMK_EFFECT_CROSS = 0x06,
	CMMK_EFFECT_RAINDROPS = 0x07,
	CMMK_EFFECT_STARS = 0x08,
	CMMK_EFFECT_SNAKE = 0x09,
	CMMK_EFFECT_CUSTOMIZED = 0x0a,
	CMMK_EFFECT_MULTILAYER = 0xe0,
	CMMK_EFFECT_OFF = 0xfe
};


/*
 * Attach to and detach from USB device
 */
struct cmmk {
	libusb_context *cxt;
	libusb_device_handle *dev;

	int product;
	int layout;

	/*
	 * Lookup map to get matrix positions for keys in constant time.
	 */
	int8_t rowmap[CMMK_KEYLIST_SIZE];
	int8_t colmap[CMMK_KEYLIST_SIZE];

	int multilayer_mode;
};

/* Helper types because passing multi dim arrays as parameter is yucky */
struct cmmk_color_matrix {
	struct rgb data[6][22];
};

struct cmmk_effect_matrix {
	uint8_t data[6][22]; /* values as type of enum cmmk_effect_id */
};

/* Generic effect type for when type safety becomes too verbose.
 *
 * No sanity checking is done before sending this off to the firmware, so try to stay within
 * normal parameters. */
struct cmmk_generic_effect {
	int p1;
	int p2;
	int p3;

	struct rgb color1;
	struct rgb color2;
};

struct cmmk_effect_fully_lit {
	struct rgb color;
};

struct cmmk_effect_breathe {
	int speed;

	struct rgb color;
};

struct cmmk_effect_cycle {
	int speed;
};

struct cmmk_effect_single {
	int speed;

	struct rgb active;
	struct rgb rest;
};

struct cmmk_effect_wave {
	int speed;

	enum cmmk_wave_direction direction;

	struct rgb start;
};

struct cmmk_effect_ripple {
	int speed;

	enum cmmk_ripple_type ripple_type;

	struct rgb active;
	struct rgb rest;
};

struct cmmk_effect_cross {
	int speed;

	struct rgb active;
	struct rgb rest;
};

struct cmmk_effect_raindrops {
	int speed;
	int interval;

	struct rgb active;
	struct rgb rest;
};

struct cmmk_effect_stars {
	int speed;
	int interval;

	struct rgb active;
	struct rgb rest;
};

struct cmmk_effect_snake {
	int speed;
};

int cmmk_attach(struct cmmk *state, int product, int layout);
int cmmk_detach(struct cmmk *state);

/*
 * Enter and leave direct control mode. Any control commands outside of control
 * mode are ignored. Enabling control mode while inside control mode will reset
 * active effect and allow direct control over LEDs.
 */
int cmmk_set_control_mode(struct cmmk *dev, int mode);

/* Only meaningful in profile customization mode */
int cmmk_get_active_profile(struct cmmk *dev, int *prof);
int cmmk_set_active_profile(struct cmmk *dev, int prof);

int cmmk_save_active_profile(struct cmmk *dev);

/* Predefined effects */
int cmmk_get_active_effect(struct cmmk *dev, enum cmmk_effect_id *eff);
int cmmk_set_active_effect(struct cmmk *dev, enum cmmk_effect_id eff);

/*
 * Get and set effect configurations.
 *
 * Caveeat: In customization mode, you can only change the configuration of an effect when it is
 * currently active. This does not seem to be the case in effects mode.
 */
int cmmk_get_effect(struct cmmk *dev, enum cmmk_effect_id id, struct cmmk_generic_effect *eff);
int cmmk_set_effect(struct cmmk *dev, enum cmmk_effect_id id, struct cmmk_generic_effect const *eff);

int cmmk_get_effect_fully_lit(struct cmmk *dev, struct cmmk_effect_fully_lit *eff);
int cmmk_set_effect_fully_lit(struct cmmk *dev, struct cmmk_effect_fully_lit const *eff);

int cmmk_get_effect_breathe(struct cmmk *dev, struct cmmk_effect_breathe *eff);
int cmmk_set_effect_breathe(struct cmmk *dev, struct cmmk_effect_breathe const *eff);

int cmmk_get_effect_cycle(struct cmmk *dev, struct cmmk_effect_cycle *eff);
int cmmk_set_effect_cycle(struct cmmk *dev, struct cmmk_effect_cycle const *eff);

int cmmk_get_effect_single(struct cmmk *dev, struct cmmk_effect_single *eff);
int cmmk_set_effect_single(struct cmmk *dev, struct cmmk_effect_single const *eff);

int cmmk_get_effect_wave(struct cmmk *dev, struct cmmk_effect_wave *eff);
int cmmk_set_effect_wave(struct cmmk *dev, struct cmmk_effect_wave const *eff);

int cmmk_get_effect_ripple(struct cmmk *dev, struct cmmk_effect_ripple *eff);
int cmmk_set_effect_ripple(struct cmmk *dev, struct cmmk_effect_ripple const *eff);

int cmmk_get_effect_cross(struct cmmk *dev, struct cmmk_effect_cross *eff);
int cmmk_set_effect_cross(struct cmmk *dev, struct cmmk_effect_cross const *eff);

int cmmk_get_effect_raindrops(struct cmmk *dev, struct cmmk_effect_raindrops *eff);
int cmmk_set_effect_raindrops(struct cmmk *dev, struct cmmk_effect_raindrops const *eff);

int cmmk_get_effect_stars(struct cmmk *dev, struct cmmk_effect_stars *eff);
int cmmk_set_effect_stars(struct cmmk *dev, struct cmmk_effect_stars const *eff);

int cmmk_get_effect_snake(struct cmmk *dev, struct cmmk_effect_snake *eff);
int cmmk_set_effect_snake(struct cmmk *dev, struct cmmk_effect_snake const *eff);

/*
 * colmap *must* be at least 6x22. Otherwise, segmentation faults ensue.
 *
 * CAVEAT: The result will be wrong immediately after switching profiles. A few milliseconds
 * of delay need to be inserted after the switch and before the query.
 */
int cmmk_get_customized_leds(struct cmmk *dev, struct cmmk_color_matrix *colmap);
int cmmk_set_customized_leds(struct cmmk *dev, struct cmmk_color_matrix const *colmap);

/*
 * Switch multilayer mode on (active > 0) or off (active == 0).
 *
 * Affects effect configuration getters and setters.
 */
int cmmk_switch_multilayer(struct cmmk *dev, int active);

int cmmk_get_multilayer_map(struct cmmk *dev, struct cmmk_effect_matrix *effmap);
int cmmk_set_multilayer_map(struct cmmk *dev, struct cmmk_effect_matrix const *effmap);

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
 * Keys in the map are indized by their individual mappings, so
 * colmap[K_ESC] will address the ESC key, much like
 * set_single_key(..., K_ESC, ...) will.
 */
int cmmk_set_leds(struct cmmk *dev, struct cmmk_color_matrix const *colmap);

#endif /* !defined(LIBCMMK_H) */

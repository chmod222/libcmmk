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

/* linear -> matrix */
static int transpose(struct cmmk *dev, struct rgb const *linear, struct cmmk_color_matrix *matrix)
{
	int i;

	for (i = 0; i < CMMK_KEYLIST_SIZE; ++i) {
		if (dev->rowmap[i] < 0 || dev->colmap[i] < 0) {
			continue;
		}

		matrix->data[dev->rowmap[i]][dev->colmap[i]] = linear[i];
	}

	return 0;
}

/* Too bad C doesn't have templates */
static int transpose_effects(struct cmmk *dev, uint8_t const *linear, struct cmmk_effect_matrix *matrix)
{
	int i;

	for (i = 0; i < CMMK_KEYLIST_SIZE; ++i) {
		if (dev->rowmap[i] < 0 || dev->colmap[i] < 0) {
			continue;
		}

		matrix->data[dev->rowmap[i]][dev->colmap[i]] = linear[i];
	}

	return 0;
}

/* matrix -> linear */
int transpose_reverse(struct cmmk *dev, struct cmmk_color_matrix const *matrix, struct rgb *linear)
{
	keyboard_layout const *layout = keyboard_layouts[dev->layout];

	int i;
	int j;

	for (i = 0; i < 6; ++i) {
		for (j = 0; j < 22; ++j) {
			int pos = 0;

			if ((pos = (*layout)[i][j]) < 0 || pos > CMMK_KEYLIST_SIZE) {
				continue;
			}

			linear[pos] = matrix->data[i][j];
		}
	}

	return 0;
}

int transpose_effects_reverse(struct cmmk *dev, struct cmmk_effect_matrix const *matrix, uint8_t *linear)
{
	keyboard_layout const *layout = keyboard_layouts[dev->layout];

	int i;
	int j;

	for (i = 0; i < 6; ++i) {
		for (j = 0; j < 22; ++j) {
			int pos = 0;

			if ((pos = (*layout)[i][j]) < 0 || pos > CMMK_KEYLIST_SIZE) {
				continue;
			}

			linear[pos] = matrix->data[i][j];
		}
	}

	return 0;
}

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
	int i;
	int j;

	keyboard_layout const *keyboard_layout = keyboard_layouts[layout];

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

	/*
	 * Generate lookup map
	 */
	memset(state->rowmap, -1, sizeof(state->rowmap));
	memset(state->colmap, -1, sizeof(state->colmap));

	for (i = 0; i < 6; ++i) {
		for (j = 0; j < 22; ++j) {
			int p = (*keyboard_layout)[i][j];

			if (p < 0) {
				continue;
			}

			state->rowmap[p] = i;
			state->colmap[p] = j;
		}
	}

	state->multilayer_mode = 0;

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

int cmmk_get_active_profile(struct cmmk *dev, int *prof)
{
	int r;

	unsigned char getprof[64] = {0x52, 0x00};

	if ((r = send_command(dev->dev, getprof, sizeof(getprof))) != 0)
		return r;

	*prof = getprof[4];

	return 0;
}

int cmmk_save_active_profile(struct cmmk *dev)
{
	unsigned char saveprof[64] = {0x50, 0x55};

	return send_command(dev->dev, saveprof, sizeof(saveprof));
}


static int set_effect1(struct cmmk *dev, int eff)
{
	unsigned char data[64] = {0x51, 0x28, 0x00, 0x00, eff};

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
		0x51, 0x2c, dev->multilayer_mode, 0x00, eff,  p1,   p2,   p3,
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

	return send_command(dev->dev, data, sizeof(data));
}

static int get_effect(
	struct cmmk *dev,
	int eff,
	int *p1, int *p2, int *p3,
	struct rgb *col1,
	struct rgb *col2)
{
	int r;

	unsigned char data[64] = {
		0x52, 0x2c, dev->multilayer_mode, 0x00, eff
	};

	memset(data + 5, 0xff, 59);

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	if (p1) {
		*p1 = data[5];
	}

	if (p2) {
		*p2 = data[6];
	}

	if (p3) {
		*p3 = data[7];
	}

	if (col1) {
		col1->R = data[10];
		col1->G = data[11];
		col1->B = data[12];
	}

	if (col2) {
		col2->R = data[13];
		col2->G = data[14];
		col2->B = data[15];
	}

	return 0;
}

int cmmk_set_active_effect(struct cmmk *dev, enum cmmk_effect_id eff)
{
	if (eff < 0 || (eff > CMMK_EFFECT_CUSTOMIZED
			&& eff != CMMK_EFFECT_OFF
			&& eff != CMMK_EFFECT_MULTILAYER)) {
		return 1;
	}

	return set_effect1(dev, eff);
}

int cmmk_get_active_effect(struct cmmk *dev, enum cmmk_effect_id *eff)
{
	unsigned char data[64] = {0x52, 0x28};
	int r;

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	*eff = data[4];

	return 0;
}

int cmmk_get_effect(struct cmmk *dev, enum cmmk_effect_id id, struct cmmk_generic_effect *eff)
{
	return get_effect(dev, id, &eff->p1, &eff->p2, &eff->p3, &eff->color1, &eff->color2);
}
int cmmk_set_effect(struct cmmk *dev, enum cmmk_effect_id id, struct cmmk_generic_effect const *eff)
{
	return set_effect(dev, id, eff->p1, eff->p2, eff->p3, &eff->color1, &eff->color2);
}


int cmmk_get_effect_fully_lit(struct cmmk *dev, struct cmmk_effect_fully_lit *eff)
{
	return get_effect(dev, CMMK_EFFECT_FULLY_LIT, NULL, NULL, NULL, &eff->color, NULL);
}

int cmmk_set_effect_fully_lit(struct cmmk *dev, struct cmmk_effect_fully_lit const *eff)
{
	return set_effect(dev, CMMK_EFFECT_FULLY_LIT, 0x00, 0x00, 0xff, &eff->color, NULL);
}


int cmmk_get_effect_breathe(struct cmmk *dev, struct cmmk_effect_breathe *eff)
{
	return get_effect(dev, CMMK_EFFECT_BREATHE, &eff->speed, NULL, NULL, &eff->color, NULL);
}

int cmmk_set_effect_breathe(struct cmmk *dev, struct cmmk_effect_breathe const *eff)
{
	return set_effect(dev, CMMK_EFFECT_BREATHE, eff->speed, 0x00, 0xff, &eff->color, NULL);
}


int cmmk_get_effect_cycle(struct cmmk *dev, struct cmmk_effect_cycle *eff)
{
	return get_effect(dev, CMMK_EFFECT_CYCLE, &eff->speed, NULL, NULL, NULL, NULL);
}

int cmmk_set_effect_cycle(struct cmmk *dev, struct cmmk_effect_cycle const *eff)
{
	return set_effect(dev, CMMK_EFFECT_CYCLE, eff->speed, 0x00, 0xff, NULL, NULL);
}


int cmmk_get_effect_single(struct cmmk *dev, struct cmmk_effect_single *eff)
{
	return get_effect(dev, CMMK_EFFECT_SINGLE, &eff->speed, NULL, NULL, &eff->active, &eff->rest);
}

int cmmk_set_effect_single(struct cmmk *dev, struct cmmk_effect_single const *eff)
{
	return set_effect(dev, CMMK_EFFECT_SINGLE, eff->speed, 0x00, 0xff, &eff->active, &eff->rest);
}


int cmmk_get_effect_wave(struct cmmk *dev, struct cmmk_effect_wave *eff)
{
	int r;
	int p2;

	if ((r = get_effect(dev, CMMK_EFFECT_WAVE, &eff->speed, &p2, NULL, &eff->start, NULL)) != 0) {
		return r;
	}

	eff->direction = p2;
	return 0;
}

int cmmk_set_effect_wave(struct cmmk *dev, struct cmmk_effect_wave const *eff)
{
	return set_effect(dev, CMMK_EFFECT_WAVE, eff->speed, eff->direction, 0xff, &eff->start, NULL);
}


int cmmk_get_effect_ripple(struct cmmk *dev, struct cmmk_effect_ripple *eff)
{
	int r;
	int p2;

	if ((r = get_effect(dev, CMMK_EFFECT_RIPPLE, &eff->speed, &p2, NULL, &eff->active, &eff->rest)) != 0) {
		return r;
	}

	eff->ripple_type = (p2 == 0x80) ? CMMK_RIPPLE_RANDOM_COLOR : CMMK_RIPPLE_GIVEN_COLOR;

	return 0;
}

int cmmk_set_effect_ripple(struct cmmk *dev, struct cmmk_effect_ripple const *eff)
{
	return set_effect(dev, CMMK_EFFECT_RIPPLE, eff->speed, eff->ripple_type ? 0x80 : 0x00, 0xff, &eff->active, &eff->rest);
}


int cmmk_get_effect_cross(struct cmmk *dev, struct cmmk_effect_cross *eff)
{
	return get_effect(dev, CMMK_EFFECT_CROSS, &eff->speed, NULL, NULL, &eff->active, &eff->rest);
}

int cmmk_set_effect_cross(struct cmmk *dev, struct cmmk_effect_cross const *eff)
{
	return set_effect(dev, CMMK_EFFECT_CROSS, eff->speed, 0x00, 0xff, &eff->active, &eff->rest);
}


int cmmk_get_effect_raindrops(struct cmmk *dev, struct cmmk_effect_raindrops *eff)
{
	return get_effect(dev, CMMK_EFFECT_RAINDROPS, &eff->speed, NULL, &eff->interval, &eff->active, &eff->rest);
}

int cmmk_set_effect_raindrops(struct cmmk *dev, struct cmmk_effect_raindrops const *eff)
{
	return set_effect(dev, CMMK_EFFECT_RAINDROPS, eff->speed, 0x00, eff->interval, &eff->active, &eff->rest);
}


int cmmk_get_effect_stars(struct cmmk *dev, struct cmmk_effect_stars *eff)
{
	return get_effect(dev, CMMK_EFFECT_STARS, &eff->speed, NULL, &eff->interval, &eff->active, &eff->rest);
}

int cmmk_set_effect_stars(struct cmmk *dev, struct cmmk_effect_stars const *eff)
{
	return set_effect(dev, CMMK_EFFECT_STARS, eff->speed, 0x00, eff->interval, &eff->active, &eff->rest);
}

int cmmk_get_effect_snake(struct cmmk *dev, struct cmmk_effect_snake *eff)
{
	return get_effect(dev, CMMK_EFFECT_SNAKE, &eff->speed, NULL, NULL, NULL, NULL);
}

int cmmk_set_effect_snake(struct cmmk *dev, struct cmmk_effect_snake const *eff)
{
	return set_effect(dev, CMMK_EFFECT_SNAKE, eff->speed, 0x00, 0xff, NULL, NULL);
}

int cmmk_set_customized_leds(struct cmmk *dev, struct cmmk_color_matrix const *colmap)
{
	unsigned char data[64] = {0x51, 0xa8};

	int i;
	int j;

	struct rgb linear[CMMK_KEYLIST_SIZE] = {};
	struct rgb *nextcol = linear;

	transpose_reverse(dev, colmap, linear);

	for (i = 0; i < 8; ++i) {
		data[2] = i*2;

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

int cmmk_get_customized_leds(struct cmmk *dev, struct cmmk_color_matrix *colmap)
{
	struct rgb linear[CMMK_KEYLIST_SIZE] = {};

	unsigned char data[64] = {0x52, 0xa8};

	int i;
	int j;

	struct rgb *ptr = linear;

	for (i = 0; i < 8; ++i) {
		data[2] = i * 2;

		send_command(dev->dev, data, sizeof(data));

		for (j = 0; j < 16; ++j) {
			int const offset = 4 + (j * 3);

			ptr->R = data[offset];
			ptr->G = data[offset + 1];
			ptr->B = data[offset + 2];

			++ptr;
		}
	}

	transpose(dev, linear, colmap);

	return 0;
}

int cmmk_switch_multilayer(struct cmmk *dev, int active)
{
	dev->multilayer_mode = active > 0;

	return 0;
}

int cmmk_get_multilayer_map(struct cmmk *dev, struct cmmk_effect_matrix *effmap)
{
	int r;

	unsigned char data[64] = {0x52, 0xa0, 0x01, 0x00};
	uint8_t linear[CMMK_KEYLIST_SIZE];

	/* Call 1 */
	data[4] = 0x00;
	data[5] = 0x07;

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	memcpy(linear, data + 8, 56);

	/* Call 2 */
	data[4] = 0x07;
	data[5] = 0x07;

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	memcpy(linear + 56, data + 8, 56);

	/* Call 3 */
	data[4] = 0x0e;
	data[5] = 0x01;

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	memcpy(linear + 112, data + 8, 8);

	transpose_effects(dev, linear, effmap);

	return 0;
}

int cmmk_set_multilayer_map(struct cmmk *dev, struct cmmk_effect_matrix const *effmap)
{
	int r;

	unsigned char data[64] = {0x51, 0xa0, 0x01, 0x00};
	uint8_t linear[CMMK_KEYLIST_SIZE] = {0};

	transpose_effects_reverse(dev, effmap, linear);

	/* Call 1 */
	data[4] = 0x00;
	data[5] = 0x07;

	memcpy(data + 8, linear, 56);

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	/* Call 2 */
	data[4] = 0x07;
	data[5] = 0x07;

	memcpy(data + 8, linear + 56, 56);

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	/* Call 3 */
	data[4] = 0x0e;
	data[5] = 0x01;

	memcpy(data + 8, linear + 112, 8);

	if ((r = send_command(dev->dev, data, sizeof(data))) != 0) {
		return r;
	}

	return 0;
}

static int cmmk_from_row_col(struct cmmk *dev, unsigned row, unsigned col)
{
	keyboard_layout const *layout = keyboard_layouts[dev->layout];

	return (*layout)[row][col];
}

/*
 * Set the single key `key' to the given color.
 */
int cmmk_set_single_key_by_id(struct cmmk *dev, int key, struct rgb const *color)
{
	unsigned char data[64] = {0xc0, 0x01, 0x01, 0x00, key, color->R, color->G, color->B};

	return send_command(dev->dev, data, sizeof(data));
}

/*
 * Set the single key in row `row` and column `col` to the given color.
 */
int cmmk_set_single_key(struct cmmk *dev, int row, int col, struct rgb const *color)
{
	int key = cmmk_from_row_col(dev, row, col);

	return cmmk_set_single_key_by_id(dev, key, color);
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
int cmmk_set_leds(struct cmmk *dev, struct cmmk_color_matrix const *colmap)
{
	unsigned char data[64];

	int i;
	int j;

	struct rgb linear[CMMK_KEYLIST_SIZE] = {};
	struct rgb *nextcol = linear;

	transpose_reverse(dev, colmap, linear);

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

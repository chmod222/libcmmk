#include "libcmmk.h"

#include <unistd.h> /* usleep() */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


int g_stop = 0;

static void interrupted(int sig)
{
	(void)sig;

	g_stop = 1;
}

int cmmk_set_profile(struct cmmk *dev, int prof);

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	struct cmmk state;

	if (cmmk_attach(&state, 0x003b, CMMK_LAYOUT_EU_L) != 0)
		return 1;

	cmmk_set_control_mode(&state, CMMK_MANUAL);

	signal(SIGINT, interrupted);

	static struct rgb col = MKRGB(0xFF00FF);

	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 22; ++j) {
			int8_t k = cmmk_from_row_col(&state, i, j);

			if (g_stop) {
				goto rip;
			} else if (k < 0) {
				continue;
			}

			cmmk_set_single_key(&state, k, &col);
			usleep(25000);
		}
	}

rip:
	cmmk_set_control_mode(&state, CMMK_FIRMWARE);
	cmmk_detach(&state);

	return 0;
}

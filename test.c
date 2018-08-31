#include "libcmmk.h"

#include <unistd.h> /* sleep() */

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

	if (cmmk_attach(&state) != 0)
		return 1;

	cmmk_set_control_mode(&state, CMMK_EFFECT);
	cmmk_set_effect_raindrops(&state, CMMK_SPEED0, &MKRGB(0xFF00FF), &MKRGB(0x000000));

	signal(SIGINT, interrupted);

	while (!g_stop) {
		sleep(1);
	}

	cmmk_set_active_profile(&state, 1);
	cmmk_set_control_mode(&state, CMMK_FIRMWARE);
	cmmk_detach(&state);

	return 0;
}
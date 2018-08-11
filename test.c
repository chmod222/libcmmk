#include "libcmmk.h"

#include <unistd.h> /* sleep() */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

	printf("Zaczynam\n");

	struct cmmk state;

	if (cmmk_attach(&state, CMMK_USB_MASTERKEYS_PRO_S) != 0) {
		printf("Wrong state\n");

		return 1;
	}

	cmmk_set_control_mode(&state, CMMK_MANUAL);
	//cmmk_set_effect_raindrops(&state, CMMK_SPEED0, &MKRGB(0xFF00FF), &MKRGB(0x000000));



	for(int i = 0; i < 6; i++)
	{
		for(int j = 0; j < 22; j++)
		{
			printf("Keymap: %i\n", j);
			int key = cmmk_from_row_col(&state, i, j);
			cmmk_set_single_key(&state, key, &MKRGB(0xFF00F0));
			if (key != -1)
				usleep(20000);
		}
	}

	usleep(40000);

	for(int i = 5; i >= 0; i--)
	{
		for(int j = 21; j >= 0; j--)
		{
			printf("Keymap: %i\n", j);
			int key = cmmk_from_row_col(&state, i, j);
			cmmk_set_single_key(&state, key, &MKRGB(0x000000));
			if (key != -1)
				usleep(20000);
		}
	}


	//cmmk_from_row_col(&state, 0, 0);
	cmmk_set_all_single(&state, &MKRGB(0x000000));

	signal(SIGINT, interrupted);

	while (!g_stop) {
		sleep(1);
	}

	cmmk_set_active_profile(&state, 1);
	cmmk_set_control_mode(&state, CMMK_FIRMWARE);
	cmmk_detach(&state);

	return 0;
}

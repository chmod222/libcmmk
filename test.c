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

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	struct cmmk state;

	if (cmmk_attach(&state) != 0)
		return 1;

	cmmk_enable_control(&state);

	signal(SIGINT, interrupted);

	cmmk_set_effect_raindrops(&state, CMMK_SPEED4,
			&(struct rgb){ 0xFF, 0xFF, 0x00},
			&(struct rgb){ 000, 0x00, 0x00});

	while (!g_stop)
		sleep(1);

	cmmk_disable_control(&state);
	cmmk_detach(&state);
	
	return 0;
}

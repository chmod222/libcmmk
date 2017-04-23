#include "libcmmk.h"

#include <unistd.h> /* sleep() */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	struct cmmk state;

	if (cmmk_attach(&state) != 0)
		return 1;

	cmmk_enable_control(&state);

	cmmk_set_effect_cross(&state,
			CMMK_SPEED4,
			&(struct rgb){0xFF, 0x00, 0x00},
			&(struct rgb){0xFF, 0xFF, 0xFF});

	sleep(5);

	cmmk_disable_control(&state);
	cmmk_detach(&state);
	
	return 0;
}

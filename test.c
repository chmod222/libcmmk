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

	struct rgb map[CMMK_KEYLIST_SIZE] = {{0, 0, 0}};
	map[K_ESC].R = 0x7F;
	map[K_ESC].G = 0x7F;
	map[K_ESC].B = 0x7F;

	cmmk_set_leds(&state, map);

	sleep(2);

	cmmk_disable_control(&state);
	cmmk_detach(&state);
	
	return 0;
}

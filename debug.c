#define CMMK_DECLARE_DEBUG_FUNCTIONS
#include "libcmmk.h"

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>

static void hexdump(void const *ptr, size_t buflen)
{
        unsigned char *buf = (unsigned char*)ptr;
        size_t i;
        size_t j;

        printf("        0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        for (i = 0; i < buflen; i += 16) {
                printf("%06lx: ", i);

                for (j = 0; j < 16; j++) {
                        if (i+j < buflen) {
                                printf("%02x ", buf[i+j]);
                        } else {
                                printf("   ");
                        }
                }

                printf(" ");

                for (j = 0; j < 16; j++) {
                        if (i+j < buflen) {
                                printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
                        }
                }

                printf("\n");
        }
        printf("        0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
}

int test_handshake1(struct cmmk *dev)
{
	unsigned char data[64] = {0x01, 0x02};

	if (cmmk_send_anything(dev, data, sizeof(data)) != 0) {
		fprintf(stderr, "test_handshake1(): send failed...?");

		return 1;
	}

	printf("Hexdump of handshake #1:\n");
	hexdump(data, sizeof(data));
	return 0;
}

int test_handshake2(struct cmmk *dev)
{
	unsigned char data[64] = {0x40, 0x20};

	if (cmmk_send_anything(dev, data, sizeof(data)) != 0) {
		fprintf(stderr, "test_handshake1(): send failed...?");

		return 1;
	}

	printf("Hexdump of handshake #2:\n");
	hexdump(data, sizeof(data));
	return 0;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	int product;

	struct cmmk state;

	if (cmmk_find_device(&product) != 0) {
		fprintf(stderr, "No device coult be found\n");

		return 1;
	}

	printf("Attaching to 2516:%04x...\n", product);

	if (cmmk_attach(&state, product, CMMK_LAYOUT_US_S) != 0) {
		fprintf(stderr, "Could not attach to device! Missing permissions?\n");

		return 1;
	}

	test_handshake1(&state);

	printf("\n");
	test_handshake2(&state);

	cmmk_detach(&state);
}

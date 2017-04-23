CC=cc
CFLAGS=-Wall -Wextra -std=c99
LDFLAGS=-lusb-1.0

test: libcmmk.o test.o
	${CC} ${LDFLAGS} $^ -o $@

test.o: test.c
libcmmk.o: libcmmk.h libcmmk.c

clean:
	rm *.o
	rm test

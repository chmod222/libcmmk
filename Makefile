CC=cc
CFLAGS=-Wall -Wextra -std=c99 -fPIC
LDFLAGS=-lusb-1.0

test: libcmmk.o test.o
	${CC} ${LDFLAGS} $^ -o $@

staticlib: libcmmk.o
	ar rcs libcmmk.a $^

test.o: test.c
libcmmk.o: libcmmk.h libcmmk.c

clean:
	rm *.a
	rm *.o
	rm test

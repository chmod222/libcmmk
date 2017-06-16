CC=gcc
CFLAGS=-s -Wall -Wextra -std=c99 -fPIC
LDFLAGS=-lusb-1.0

cmmk-test: libcmmk.o test.o
	${CC} ${CFLAGS} $^ -o out/$@ ${LDFLAGS}

libcmmk: libcmmk.o
	mkdir -p out
	ar rcs out/libcmmk.a $^
	${CC} -shared ${CFLAGS} $^ -o out/$@.so

libcmmk.o: libcmmk.h libcmmk.c
test.o: test.c

clean:
	rm -f *.o
	rm -rf out

CC=gcc
CFLAGS=-s -Wall -Wextra -std=c99 -fPIC
LDFLAGS=-lcmmk -lusb-1.0 -Lout/

cmmk-test: libcmmk test.o
	${CC} ${CFLAGS} test.o -o out/$@ ${LDFLAGS}

libcmmk: libcmmk.o
	mkdir -p out
	ar rcs out/libcmmk.a $^
	${CC} -shared ${CFLAGS} $^ -o out/$@.so

libcmmk.o: libcmmk.h libcmmk.c
test.o: test.c

clean:
	rm -f *.o
	rm -rf out

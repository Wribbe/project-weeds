BINS := $(patsubst src/%.c,bin/%,$(wildcard src/*.c))
SHARED := $(patsubst src/shared/%.c,shared/%.so,$(wildcard src/shared/*.c))
HEADERS := $(wildcard lib/*.h)

LIBS := $(wildcard lib/*.c)

FLAGS := -Wall --pedantic -lGL -lX11 -g -lpulse -lpthread -lm -ldl -I.

all: ${SHARED} ${BINS}

bin/% : src/%.c Makefile ${LIBS} ${HEADERS} | bin
	gcc $(filter %.c,$^) -o $@ ${FLAGS}

shared/%.so : src/shared/%.c Makefile ${HEADERS} | shared
	gcc -shared -o $@ -fpic $(filter %.c,$^)

bin shared:
	mkdir -p $@

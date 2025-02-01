CC=gcc

BIN=bin/borticles

CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT= -lm
LOPT+= -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
COPT=

HEADERS=$(wildcard src/*.h)
SOURCES=$(filter-out src/main.c, $(wildcard src/*.c))
SOURCES+=$(wildcard src/algorithms/*.c) $(wildcard src/qtree/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

INCS=-Isrc

# glad (can be removed when fully migrated to raylib)
SOURCES+=$(wildcard src/external/*.c)
OBJECTS+=lib/glad/src/glad.o
INCS+=-Ilib/glad/include
# /glad

.PHONY:	clean all prepare

all:	clean prepare $(BIN)

prepare:
	mkdir -p bin

$(BIN):	$(OBJECTS) src/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOPT)

%.o:	%.c $(HEADERS)
	$(CC) $(COPT)-c $< -o $@ $(INCS)

test:	$(OBJECTS) $(patsubst %.c, %.o, $(wildcard test/*.c)) test/main.o
	$(CC) $(CFLAGS) -o bin/test $^ $(LOPT)

tests/%.o:	%.c $(HEADERS) test/test.h
	$(CC) $(COPT)-c $< -o $@ -I$(INCDIR) -Itests

clean:
	find ./src/ -name \*.o -type f -delete; rm -f bin/*

CC=gcc

BIN=bin/borticles

CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT=-lm -lGL
LOPT+= -lglfw -lm -I.

HEADERS=$(wildcard src/*.h)
SOURCES=$(filter-out src/main.c, $(wildcard src/*.c))
SOURCES+=$(wildcard src/external/*.c)
SOURCES+=$(wildcard src/algorithms/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
OBJECTS+=lib/glad/src/glad.o

INCS=-Isrc -Ilib/glad/include

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

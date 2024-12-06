CC=gcc

BIN=bin/borticles

CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT=-lm -lGL
LOPT+= -lglfw -lm -I.

HEADERS=$(wildcard src/*.h)
SOURCES= $(filter-out src/main.c, $(wildcard src/*.c))
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

clean:
	rm -f src/*.o bin/*

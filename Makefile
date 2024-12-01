CC=gcc

SRCDIR=src
INCDIR=$(SRCDIR)
BINDIR=bin
LIBDIR=lib

BIN=bin/borticles

CFLAGS=-Wall -Wextra -Werror -Wpedantic -pedantic-errors
LOPT=-lm -lGL
LOPT+= -lglfw -lm -I.

HEADERS=$(INCDIR)/utils.h
OBJECTS=$(SRCDIR)/utils.o
OBJECTS+=$(LIBDIR)/glad/src/glad.o

INCS=-I$(INCDIR) -I$(LIBDIR)/glad/include

.PHONY:	clean all prepare

all:	clean prepare $(BIN)

prepare:
	mkdir -p $(BINDIR)

$(BIN):	$(OBJECTS) $(SRCDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOPT)

%.o:	%.c $(HEADERS)
	$(CC) $(COPT)-c $< -o $@ $(INCS)

clean:
	rm -f $(SRCDIR)/*.o $(BIN)

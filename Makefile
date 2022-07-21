CFLAGS += -std=c99 -Wall -Wextra -pedantic -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
CC     ?= gcc

all: tiawm

tiawm: tiawm.c tiawm.h config.h Makefile
	$(CC) -O3 $(CFLAGS) -o $@ $< -lxcb -lxcb-keysyms $(LDFLAGS)

clean:
	rm -f tiawm *.o

.PHONY: all tiawm clean
CFLAGS += -std=c99 -Wall -Wextra -pedantic -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
CC     ?= gcc

all: tiawm

config.h:
	cp config.def.h config.h

tiawm: tiawm.c tiawm.h config.h Makefile
	$(CC) -O3 $(CFLAGS) -o $@ $< -lxcb -lxcb-keysyms $(LDFLAGS)

install: all
	install -Dm755 tiawm $(DESTDIR)$(BINDIR)/tiawm

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tiawm

clean:
	rm -f tiawm *.o

.PHONY: all tiawm clean

CC     ?= gcc

CFLAGS += -Os -std=c99 -pedantic -Wall -Wextra -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

SRC    += list.h  wm.c  xcb.c  xcb.h
LIB    := -lxcb-keysyms -lxcb

all: tiawm

tiawm:
	$(CC) -o $@ -Os $(CFLAGS)  $(SRC) $(LIB) $(LDFLAGS)

install: all
	install -Dm755 tiawm $(DESTDIR)$(BINDIR)/tiawm

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tiawm

clean:
	rm -f tiawm *.o

.PHONY: all tiawm clean

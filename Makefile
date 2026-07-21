CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra
CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

LIBS = $(shell pkg-config --libs ncursesw 2>/dev/null || pkg-config --libs ncurses 2>/dev/null || echo "-lncursesw")
CFLAGS += $(shell pkg-config --cflags ncursesw 2>/dev/null || pkg-config --cflags ncurses 2>/dev/null || true)

all: meowdo

meowdo: meowdo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

debug: CFLAGS += -g
debug: meowdo

clean:

	rm -f meowdo


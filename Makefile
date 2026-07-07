CC     = gcc
CFLAGS = -O2 -Wall -Wextra

# Try ncursesw first (Debian/Ubuntu), fall back to ncurses (Arch/macOS)
LIBS = $(shell pkg-config --libs ncursesw 2>/dev/null || \
        pkg-config --libs ncurses  2>/dev/null || \
        echo -lncurses)

CFLAGS += $(shell pkg-config --cflags ncursesw 2>/dev/null || \
           pkg-config --cflags ncurses  2>/dev/null || true)

meowdo: meowdo.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

debug: CFLAGS += -g
debug: meowdo

clean:
	rm -f meowdo

CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra
CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib
LIBS = -lncursesw

all: meowdo

meowdo: meowdo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

debug: CFLAGS += -g
debug: meowdo

clean:

	rm -f meowdo


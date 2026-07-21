CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra
CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib
LIBS = -lncursesw

all: meowdo

meowdo: meowdo.c
	$(CC) $(CFLAGS) -o meowdo meowdo.c $(LDFLAGS) $(LIBS)

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -g" all

clean:
	rm -f meowdo

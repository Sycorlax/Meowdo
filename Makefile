CC = cc
CFLAGS = -O2 -Wall -Wextra
LIBS = -lncurses

all: meowdo

meowdo: meowdo.c
	$(CC) $(CFLAGS) -o meowdo meowdo.c $(LIBS)

debug:
	$(MAKE) CFLAGS="$(CFLAGS) -g" all

clean:
	rm -f meowdo

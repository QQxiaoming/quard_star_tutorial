prefix = /usr
CC=gcc
LIBEVENTDIR=/usr
CFLAGS := -O0
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
#CFLAGS += -s

cu: *.c
	$(CC) $(CFLAGS) *.c -o cu -I$(LIBEVENTDIR)/include -L$(LIBEVENTDIR)/lib -levent

.PHONY: clean install
clean:
	rm -vf cu
install:
	cp cu $(prefix)/bin/cu

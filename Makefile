CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra

all: -i

-i: i.c
	$(CC) $(CFLAGS) -std=c99 -o ./-i i.c

check: -i
	@sh test.sh

clean:
	rm -f ./-i

.PHONY: all check clean

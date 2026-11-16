CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra

all: -i

-i: i.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=c99 -o ./-i i.c $(LDFLAGS) $(LDLIBS)

check: -i
	@sh test.sh

clean:
	rm -f ./-i

.PHONY: all check clean

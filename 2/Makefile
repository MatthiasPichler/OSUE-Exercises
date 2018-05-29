
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = dsort.o
NAME = dsort
.PHONY: all clean
all: dsort

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

dsort.o : dsort.c

clean:
	rm -rf $(OBJECTS) $(NAME)
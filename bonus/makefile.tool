CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

INC_DIR=./include
OBJ_DIR=./svctl_src

_DEPS = debug.h secvault.h
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

svctl: $(OBJ)
	$(CC) $(CFLAGS) $? -o $@

clean:
	- rm $(OBJ_DIR)/*.o svctl

.PHONY: clean	
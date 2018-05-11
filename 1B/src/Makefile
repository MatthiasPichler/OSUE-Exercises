# Author: Matthias Pichler, 01634256

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

IDIR =../include
ODIR=../obj
BINDIR=../bin

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

COMMON_OBJ = common.o map.o ship.o msg.o

_DEPS = common.h map.h ship.h mag.h solver.h deque.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_SERVER_OBJ = server.o $(COMMON_OBJ)
SERVER_OBJ = $(patsubst %,$(ODIR)/%,$(_SERVER_OBJ))

_CLIENT_OBJ = client.o solver.o deque.o $(COMMON_OBJ)
CLIENT_OBJ = $(patsubst %,$(ODIR)/%,$(_CLIENT_OBJ))

all: server client

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) $? -o $(BINDIR)/$@

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $? -o $(BINDIR)/$@

.PHONY: clean

clean:
	- rm $(ODIR)/*.o $(BINDIR)/server $(BINDIR)/client
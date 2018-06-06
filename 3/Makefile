CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -lpthread -lrt
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

IDIR =./include
OBJDIR=./obj
SCRDIR=./src
BINDIR=./bin

$(OBJDIR)/%.o: $(SCRDIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

COMMON_OBJ = ring_buffer.o

_DEPS = vertex.h ring_buffer.h graph.h common.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_GENERATOR_OBJ = generator.o graph.o vertex.o $(COMMON_OBJ)
GENERATOR_OBJ = $(patsubst %,$(OBJDIR)/%,$(_GENERATOR_OBJ))

_SUPERVISOR_OBJ = supervisor.o $(COMMON_OBJ)
SUPERVISOR_OBJ = $(patsubst %,$(OBJDIR)/%,$(_SUPERVISOR_OBJ))

all: generator supervisor

generator: $(GENERATOR_OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $? -o $(BINDIR)/$@

supervisor: $(SUPERVISOR_OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $? -o $(BINDIR)/$@

.PHONY: clean

clean:
	- rm $(OBJDIR)/*.o $(BINDIR)/generator $(BINDIR)/supervisor
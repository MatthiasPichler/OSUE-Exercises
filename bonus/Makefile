obj-m += secvault.o
secvault-objs := src/main.o

EXTRA_CFLAGS += -I$(src)/include
EXTRA_CFLAGS += -std=gnu99

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

$(src)/src/main.o: $(src)/src/main.c $(src)/include/debug.h
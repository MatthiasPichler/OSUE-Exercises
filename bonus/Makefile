KERN_DIR=/lib/modules/$(shell uname -r)/build

INC_DIR=$(src)/include
DRV_OBJ_DIR=driver_src
CTL_OBJ_DIR=svctl_src

_DEPS = debug.h secvault.h
DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))

_DRV_OBJ = main.o ctl.o vault.o
DRV_OBJ = $(patsubst %,$(DRV_OBJ_DIR)/%,$(_DRV_OBJ))

_CTL_OBJ = main.o
CTL_OBJ = $(patsubst %,$(CTL_OBJ_DIR)/%,$(_CTL_OBJ))

EXTRA_CFLAGS += -I$(INC_DIR)
EXTRA_CFLAGS += -std=gnu99
EXTRA_CFLAGS += -DDEBUG

obj-m += secvault.o
secvault-objs := $(DRV_OBJ)

all: driver svctl

driver:
	make -C $(KERN_DIR) M=$(PWD) ARCH=um V=1 modules 

svctl: $(CTL_OBJ)
	gcc -std=c99 -pedantic -Wall -g -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L $? -o $@	

clean:
	make -C $(KERN_DIR) M=$(PWD) ARCH=um V=1 clean
	rm -f $(CTL_OBJ_DIR)/*.o svctl
	scripts/clean.sh
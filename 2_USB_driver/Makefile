MODULE_NAME = my_usb_data_module

SRC := ./src/USB_data_transfer.c 

# Path to target Linux Kernel
KDIR := /lib/modules/$(shell uname -r)/build # <--- Fill in with path to kernel you're compiling against

$(MODULE_NAME)-objs = $(SRC:.c=.o)

obj-m       := $(MODULE_NAME).o
PWD := $(shell pwd)

#EXTRA_CFLAGS := -I$(PWD)/src -I$(PWD)/include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

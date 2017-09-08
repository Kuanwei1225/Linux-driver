MODULE_NAME = hello_module

SRC := ./src/hello.c 

# Path to target Linux Kernel
KDIR := /lib/modules/$(shell uname -r)/build # <--- Fill in with path to kernel you're compiling against

$(MODULE_NAME)-objs = $(SRC:.c=.o)

obj-m       := $(MODULE_NAME).o
PWD := $(shell pwd)

#EXTRA_CFLAGS := -I$(PWD)/src -I$(PWD)/include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

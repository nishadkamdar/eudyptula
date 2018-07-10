# obj-m:
#   specifies object files which are built as loadable kernel modules.

# make modifiers:
#  -C dir:
#     Change  to directory dir before reading the makefiles or doing any-
#     thing else.  We need to compile in root directory of kernel source.
#   M=dir:
#     The path to the source file(s) of the kernel module to be built
obj-m := prod-cons.o

KVERSION := $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean


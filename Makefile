ifeq ($(KERNELRELEASE),)

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -w -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko .mod.o *.mod.c *.symvers *.order

else
	obj-m := devinput.o
	devinput-objs = drv_input.o
endif

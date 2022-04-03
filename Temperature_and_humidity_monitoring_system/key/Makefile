modname?=demo
arch?=x86
ifeq ($(arch),x86)	
	KERNELDIR:= /lib/modules/$(shell uname -r)/build/
else
	KERNELDIR:= /home/ubuntu/linux_5_10_kernel/sources/arm-ostl-linux-gnueabi/linux-stm32mp-5.10.10-r0/linux-5.10.10/
endif

PWD:= $(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

obj-m:= $(modname).o

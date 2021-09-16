GLIB_ELF_CROSS_COMPILE_DIR=/opt/gcc-riscv64-unknown-linux-gnu
GLIB_ELF_CROSS_PREFIX=$(GLIB_ELF_CROSS_COMPILE_DIR)/bin/riscv64-unknown-linux-gnu

ifeq ($(KERNELRELEASE), )
KERNELDIR := ../../linux-5.10.65
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) ARCH=riscv CROSS_COMPILE=$(GLIB_ELF_CROSS_PREFIX)-
clean:
	rm -rf .tmp_versions Module.symvers *.mod *.mod.c *.o *.ko .*.cmd built-in.a Module.markers modules.order .cache.mk
load:
	insmod virts.ko
unload:
	rmmod virts
install:
	cp virts.ko /lib/modules/$(shell uname -r)/kernel/drivers/virts.ko
else
	obj-m := virts.o
endif

#Deisgn by wolfnx
-include $(ZPL_MAKE_DIR)/makelinux.mk

-include config.mk

Q       := @

KO_OBJS  += eth_netlink.o
KO_OBJS  += eth_drv.o

ifeq ($(KERNELRELEASE), )


#使用自定义路径下的kernel编译
#KERNELDIR ?=/home/lost/lost/source/kernel
#使用系统路径的kernel源码编译,根据自己系统路径修改
KERNELDIR ?=/home/zhurish/workspace/working/sunxi/linux-5.15.14

PWD :=$(shell pwd)

modules: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(Q)rm -f *.ko *.o *.mod.c *.cmd *.mod .*.o.cmd .*.cmd modules.order Module.symvers

.PHONY:modules modules_install clean

all: clean modules
else
	eth_drv-objs := $(KO_OBJS)
	obj-m := eth_drv.o
endif

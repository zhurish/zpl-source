include $(ZPL_MAKE_DIR)/makelinux.mk


MKBUILD_OUTPUT = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)/sdk/kernel

KERN_DIR = /home/zhurish/workspace/working/sunxi/linux-5.15.14 

ifeq ($(ZPL_ARCH_X86),true)
KERN_DIR = /home/zhurish/workspace/working/sunxi/centos/linux-5.15.14 
endif
ifeq ($(ZPL_ARCH_X86_64),true)
KERN_DIR = /usr/src/kernels/4.18.0-348.7.1.el8_5.x86_64 
endif

ZPLKCFLAGS=$(ZPLDEFINE) $(ZPLDEBUG) -Wno-error=incompatible-pointer-types 
MODULEDIR = product

KO_DIR = $(ZPL_INSTALL_KO_DIR)

# arch
KARCH_TYPE = $(shell echo $(ARCH_TYPE) | tr A-Z a-z)

# module 
ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
LINUX_KBE_NAME := linux-sdk-kbe
else
ZPLKCFLAGS+=-UZPL_SDK_USER -UZPL_SDK_NONE
LINUX_KBE_NAME := linux-dev-kbe
endif

LINUX_MDIODEV_NAME := linux-mdio-dev


# obj file
MDIO_OBJ += mdiodev.o

KBSP_OBJ += khal_ipcmsg.o
KBSP_OBJ += khal_ipccmd.o
KBSP_OBJ += khal_netlink.o
KBSP_OBJ += khal_netpkt.o
KBSP_OBJ += keth_drv.o
KBSP_OBJ += kloopback.o
KBSP_OBJ += khal_util.o
KBSP_OBJ += khal_client.o

ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
BSP_OBJ += ../../bsp/hal/bsp_global.o
BSP_OBJ += ../../bsp/hal/bsp_l3if.o
BSP_OBJ += ../../bsp/hal/bsp_route.o
BSP_OBJ += ../../bsp/hal/bsp_port.o
BSP_OBJ += ../../bsp/hal/bsp_misc.o
BSP_OBJ += ../../bsp/hal/bsp_mstp.o	
BSP_OBJ += ../../bsp/hal/bsp_8021x.o		
BSP_OBJ += ../../bsp/hal/bsp_dos.o			
BSP_OBJ += ../../bsp/hal/bsp_mac.o		
BSP_OBJ += ../../bsp/hal/bsp_vlan.o	
BSP_OBJ += ../../bsp/hal/bsp_qinq.o
BSP_OBJ += ../../bsp/hal/bsp_qos.o		
BSP_OBJ += ../../bsp/hal/bsp_trunk.o		
BSP_OBJ += ../../bsp/hal/bsp_mirror.o	
BSP_OBJ += ../../bsp/hal/bsp_cpu.o

SDK_OBJ += ../user/b53_mdio.o 
SDK_OBJ += ../user/b53_cpu.o  
SDK_OBJ += ../user/b53_global.o  
SDK_OBJ += ../user/b53_mirror.o  
SDK_OBJ += ../user/b53_port.o  
SDK_OBJ += ../user/b53_rate.o  
SDK_OBJ += ../user/b53_stp.o    
SDK_OBJ += ../user/b53_vlan.o 
SDK_OBJ += ../user/b53_dos.o  
SDK_OBJ += ../user/b53_eap.o     
SDK_OBJ += ../user/b53_mac.o  
SDK_OBJ += ../user/b53_phy.o     
SDK_OBJ += ../user/b53_qos.o   
SDK_OBJ += ../user/b53_snoop.o  
SDK_OBJ += ../user/b53_trunk.o  
SDK_OBJ += ../user/b53_driver.o  

SDK_OBJ += ../user/sdk_driver.o  
endif

# module obg
$(LINUX_KBE_NAME)-objs += $(KBSP_OBJ)
ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
$(LINUX_KBE_NAME)-objs += $(BSP_OBJ)
$(LINUX_KBE_NAME)-objs += $(SDK_OBJ)
endif

$(LINUX_MDIODEV_NAME)-objs += $(MDIO_OBJ)


EXTRA_CFLAGS += -I$(BASE_ROOT)/include \
				-I$(BASE_ROOT)/$(MODULEDIR)/bsp/hal \
				-I$(BASE_ROOT)/$(MODULEDIR)/bsp/kernel \

ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
EXTRA_CFLAGS += -I$(BASE_ROOT)/$(MODULEDIR)/sdk/user
endif
  	
#make -C $(KERN_DIR) M=$(shell pwd) modules clean 
ccflags-y += $(ZPLKCFLAGS) $(EXTRA_CFLAGS)
obj-m += $(LINUX_KBE_NAME).o  
obj-m += $(LINUX_MDIODEV_NAME).o 
 

make_prepare:
	@echo ""

all:  lib    
  
lib: 
	@if test ! -d $(KO_DIR) ; \
		then \
		$(MKDIR) -p $(KO_DIR) ; \
	fi
	make ARCH=$(KARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(shell pwd) modules 

obj: 
	@if test ! -d $(KO_DIR) ; \
		then \
		$(MKDIR) -p $(KO_DIR) ; \
	fi
	make ARCH=$(KARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(shell pwd) modules 
# 
# KBUILD_OUTPUT=$(MKBUILD_OUTPUT)
#
install: 
	@install -d $(KO_DIR)
	@install -m 755 $(LINUX_KBE_NAME).ko $(KO_DIR)
	@install -m 755 $(LINUX_MDIODEV_NAME).ko $(KO_DIR)
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers .tmp*
ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
	@$(RM) -rf ../user/*.ko ../user/*.o ../user/.*.cmd 
	@$(RM) -rf ../../bsp/hal/*.ko ../../bsp/hal/*.o ../../bsp/hal/.*.cmd
endif

clean:  objclean   
	@if test -f $(KO_DIR)/$(LINUX_KBE_NAME).ko ; \
		then \
		$(RM) $(KO_DIR)/$(LINUX_KBE_NAME).ko ; \
	fi
	@if test -f $(KO_DIR)/$(LINUX_MDIODEV_NAME).ko ; \
		then \
		$(RM) $(KO_DIR)/$(LINUX_MDIODEV_NAME).ko ; \
	fi                                


objclean: 
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers .tmp*
ifeq ($(strip $(ZPL_SDK_KERNEL)),true)
	@$(RM) -rf ../user/*.ko ../user/*.o ../user/.*.cmd 
	@$(RM) -rf ../../bsp/hal/*.ko ../../bsp/hal/*.o ../../bsp/hal/.*.cmd 
endif

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare 

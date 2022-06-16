include $(ZPL_MAKE_DIR)/makelinux.mk

ZPLKCFLAGS=$(ZPLDEFINE) $(ZPLDEBUG)
MODULEDIR = product

KO_DIR = $(ZPL_INSTALL_KO_DIR)

KARCH_TYPE = $(shell echo $(ARCH_TYPE) | tr A-Z a-z)

MODULE_NAME := b53125-kbe

KBSP_OBJ += hal_ipcmsg.o
KBSP_OBJ += hal_ipccmd.o
KBSP_OBJ += hal_client.o

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
SDK_OBJ += ../user/b53_mac_tbl.o  
SDK_OBJ += ../user/b53_phy.o     
SDK_OBJ += ../user/b53_qos.o   
SDK_OBJ += ../user/b53_snoop.o  
SDK_OBJ += ../user/b53_trunk.o  
SDK_OBJ += ../user/b53_driver.o  

SDK_OBJ += ../user/sdk_driver.o  

$(MODULE_NAME)-objs += $(KBSP_OBJ)
$(MODULE_NAME)-objs += $(BSP_OBJ)
$(MODULE_NAME)-objs += $(SDK_OBJ)

EXTRA_CFLAGS := -I$(BASE_ROOT)/include \
				-I$(BASE_ROOT)/$(MODULEDIR)/bsp/hal \
				-I$(BASE_ROOT)/$(MODULEDIR)/sdk/kernel \
				-I$(BASE_ROOT)/$(MODULEDIR)/sdk/user

MKBUILD_OUTPUT = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)/sdk/kernel


KERN_DIR = /home/zhurish/workspace/working/sunxi/linux-5.15.14  

  	
#make -C $(KERN_DIR) M=$(shell pwd) modules clean 
ccflags-y += $(ZPLKCFLAGS) $(EXTRA_CFLAGS)
obj-m += $(MODULE_NAME).o  


make_prepare:
	@echo ""

all:  lib    
  
lib: 
	make ARCH=$(KARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(shell pwd) modules 

obj: 
	make ARCH=$(KARCH_TYPE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(shell pwd) modules 
# 
# KBUILD_OUTPUT=$(MKBUILD_OUTPUT)
#
install: 
	@install -d $(KO_DIR)
	@install -m 755 $(MODULE_NAME).ko $(KO_DIR)
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers
	@$(RM) -rf ../user/*.ko ../user/*.o ../user/.*.cmd ../user/*.a
	@$(RM) -rf ../../bsp/hal/*.ko ../../bsp/hal/*.o ../../bsp/hal/.*.cmd ../../bsp/hal/*.a


clean:  objclean                                   
	@$(RM) $(KO_DIR)/$(MODULE_NAME).ko

objclean: 
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers
	@$(RM) -rf ../user/*.ko ../user/*.o ../user/.*.cmd ../user/*.a
	@$(RM) -rf ../../bsp/hal/*.ko ../../bsp/hal/*.o ../../bsp/hal/.*.cmd ../../bsp/hal/*.a


all: lib install

.PHONY:	obj all lib install objclean clean make_prepare 

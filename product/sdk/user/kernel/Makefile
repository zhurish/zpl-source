include $(ZPL_MAKE_DIR)/makelinux.mk

ZPLKCFLAGS=$(ZPLDEFINE) $(ZPLDEBUG)
MODULEDIR = product

KO_DIR = $(ZPL_INSTALL_KO_DIR)

KARCH_TYPE = $(shell echo $(ARCH_TYPE) | tr A-Z a-z)

MODULE_NAME := mdio-dev

KBSP_OBJ += mdiodev.o


$(MODULE_NAME)-objs += $(KBSP_OBJ)


EXTRA_CFLAGS := -I$(BASE_ROOT)/include \
				-I$(BASE_ROOT)/$(MODULEDIR)/sdk/user/kernel

MKBUILD_OUTPUT = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)/sdk/user/kernel


KERN_DIR = /home/zhurish/workspace/working/sunxi/linux-5.15.14  

  	
#make -C $(KERN_DIR) M=$(shell pwd) modules clean 
ccflags-y += $(ZPLKCFLAGS) $(EXTRA_CFLAGS)
obj-m += $(MODULE_NAME).o  


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
	@install -m 755 $(MODULE_NAME).ko $(KO_DIR)
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers



clean:  objclean                                   
	@$(RM) $(KO_DIR)/$(MODULE_NAME).ko

objclean: 
	@$(RM) -rf *.ko *.o *.mod *.mod.* .*.cmd *.order *.symvers



all: lib install

.PHONY:	obj all lib install objclean clean make_prepare 

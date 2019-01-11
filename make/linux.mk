##
#

#
#
#
ifeq ($(BUILD_TYPE),X86)
export AS=as
export LD=ld
export CC=gcc
export CXX=g++
export CPP=$(CC) -E
export AR=ar
export NM=nm
export LDR=ldr
export STRIP=strip
export OBJCOPY=objcopy
export OBJDUMP=objdump
export RANLIB=ranlib
PL_CFLAGS += -DBUILD_$(BUILD_TYPE)
endif
#
ifneq ($(BUILD_TYPE),X86)
ifeq ($(CROSS_COMPILE),)
$(error CROSS_COMPILE is not define)
endif
export AS=$(CROSS_COMPILE)as
export LD=$(CROSS_COMPILE)ld
export CC=$(CROSS_COMPILE)gcc
export CXX=$(CROSS_COMPILE)g++
export CPP=$(CC) -E
export AR=$(CROSS_COMPILE)ar
export NM=$(CROSS_COMPILE)nm
export LDR=$(CROSS_COMPILE)ldr
export STRIP=$(CROSS_COMPILE)strip
export OBJCOPY=$(CROSS_COMPILE)objcopy
export OBJDUMP=$(CROSS_COMPILE)objdump
export RANLIB=$(CROSS_COMPILE)ranlib

PL_CFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib
PL_CFLAGS += -DBUILD_$(BUILD_TYPE)

endif
#
#
#
ifeq ($(ARCH_BIT),64)
PLOS_CFLAGS += -m64
else
ifneq ($(BUILD_TYPE),X86)
#PLOS_CFLAGS += -m32
endif
endif
#
#
#
PLBASE = $(BASE_ROOT)
#
include $(MAKE_DIR)/module.mk

include $(MAKE_DIR)/module-config.mk

#include $(MAKE_DIR)/platform-module.mk
#
PLINCLUDE += -I$(PLBASE)/include
#
#
#
ifeq ($(strip $(MODULE_WIFI_SRC)),true)
#PL_LDLIBS += -lnl-3 -lnl-genl-3
endif
ifeq ($(strip $(MODULE_SSH)),true)
#PL_LDLIBS += -lutil -lcrypto -lrt
#-lz -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err
endif
#
ifeq ($(BUILD_TYPE),X86)
ifeq ($(strip $(MODULE_SQLITE)),true)
PL_LDLIBS += -lsqlite3
endif
endif
#
#
#
#
ifeq ($(USE_IPCOM_STACK),true)
#IPSTACK_LIBDIR
IPCOM_DEF=-DIPCOM_DRIVER_CB -DIPUTIL -DIPCOM_OS_THREAD -DIPCOM_THREAD -DIPCOM_NETSNMP \
		  -DIPNET6 -DIPCOM_USE_INET6 -DIPNET -DIPCOM_USE_INET -DIPVRRP -DIPNET_MSP_DISTRIBUTION \
		  -DIPNET_STATISTICS -DIPTCP -DIPSCTP -DIPPPP -DIPIPSEC2 -DIPFIREWALL -DIP8021X -DIPDHCPC \
		  -DIPDNSC -DIPTFTPC -DIPTFTPS -DIPFTPC -DIPFTPS -DIPCRYPTO -DIPCRYPTO_USE_TYPE_MAPPING \
		  -DIPDHCPC6 -DIPDHCPR -DIPDHCPS6 -DIPDHCPS -DIPDIAMETER -DIPEAP -DIPIKE -DIPCOM_USE_FLOAT \
		  -DIPL2TP -DIPMPLS -DIPMPLS_MULTIPLE_ROUTE_TABLES -DIPRADIUS -DIPRIPNG -DIPRIP -DIPSNTP \
		  -DIPSSH -DIPSSL -DWRSNMP -DIPSNMP -DIPCOM_USE_MIB2 -DIPCOM_NETSNMP -DIPRIPNG \
		  -DIPDIAMETER -DIPWLAN -DIPWPS -DIPCOM_DRV_ETH_SIMULATION
#IPINCLUDE += $(IPLIBINCLUDE) $(IPSTACK_INCLUDE)
#
PL_DEBUG += $(IPCOM_DEF) -DUSE_IPSTACK_IPCOM

PLINCLUDE += $(IPSTACK_INCLUDE)
else
PL_DEBUG += -DUSE_IPSTACK_KERNEL -DUSE_LINUX_OS

endif
#


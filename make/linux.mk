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
PLOS_DEFINE += -DBUILD_$(BUILD_TYPE)
PLOS_DEFINE += -DSYS_REAL_DIR=\"$(BASE_ROOT)/$(RELEASEDIR)\"
PLOS_CFLAGS += -I/usr/include -I/usr/local/include 
PLOS_LDFLAGS += -L/lib -L/usr/lib -L/lib64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/lib64
endif
#
ifneq ($(BUILD_TYPE),X86)
ifeq ($(CROSS_COMPILE),)
$(error CROSS_COMPILE is not define)
endif
#export CROSS_COMPILE
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

PLOS_CFLAGS += -I$(CROSS_COMPILE_ROOT)/include -I$(CROSS_COMPILE_ROOT)/usr/include 
PLOS_LDFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib
PLOS_DEFINE += -DBUILD_$(BUILD_TYPE)
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
PLOS_INCLUDE += -I$(PLBASE)/include
#
#
#PLOS_LDFLAGS += $(PLEX_LDFLAGS)
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
#PLOS_LDLIBS += -lsqlite3
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
PL_DEFINE += $(IPCOM_DEF) -DUSE_IPSTACK_IPCOM

PL_INCLUDE += $(IPSTACK_INCLUDE)
else
PLOS_DEFINE += -DUSE_IPSTACK_KERNEL -DUSE_LINUX_OS -D__linux__

endif
#


##
#
#
ifneq ($(TOP_DIR),)
ROOT_DIR = $(TOP_DIR)
else
ROOT_DIR = /home/zhurish/workspace/SWPlatform
endif

BASE_ROOT = $(ROOT_DIR)

export MAKE_DIR = $(BASE_ROOT)/make
export BASE_ROOT = $(ROOT_DIR)
#
#
#
include $(MAKE_DIR)/board.cfg
#
#
BUILD_TYPE	=$(ARCH_TYPE)
#
BUILD_DEBUG	=$(ARCH_DEBUG)
#
#
#
#
#
ifeq ($(SIM),yes)
export SIMULATION = YES
else
ifeq ($(SIM),YES)
export SIMULATION = YES
endif
endif

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
PL_CFLAGS += -m64
else
ifneq ($(BUILD_TYPE),X86)
#PL_CFLAGS += -m32
endif
endif
#
#
ifeq ($(BUILD_DEBUG),NO)
RELEASEDIR = release
OBJDIR = $(RELEASEDIR)/obj
LIBDIR = $(RELEASEDIR)/lib
BINDIR = $(RELEASEDIR)/bin
SBINDIR = $(RELEASEDIR)/sbin
ETCDIR = $(RELEASEDIR)/etc
PL_CFLAGS += -s
else
RELEASEDIR = debug
OBJDIR = $(RELEASEDIR)/obj
LIBDIR = $(RELEASEDIR)/lib
BINDIR = $(RELEASEDIR)/bin
SBINDIR = $(RELEASEDIR)/sbin
ETCDIR = $(RELEASEDIR)/etc
endif
#
#
ECHO = echo
CD = cd
RM = rm
MV = mv
CP = cp
CHMOD = chmod
MKDIR = mkdir
TAR = tar
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
#C_TYPE=MUSL
#ifeq ($(C_TYPE),MUSL)
#PL_CFLAGS += -DBUILD_STD_MUSL
#endif
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
#
#
ifeq ($(BUILD_DEBUG),YES)
PL_CFLAGS +=  -g
endif
#
#
VERSION = 0.0.0.2
#
BUILD_TIME=$(shell date -u "+%y%m%d-%H%M%S")
#
PLVER = $(VERSION)-$(BUILD_TIME).bin
#
ifeq ($(BUILD_DEBUG),YES)
PLVER = $(VERSION).bin
endif
#
#
#
PL_LDLIBS += -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s

ifeq ($(strip $(MODULE_WIFI_SRC)),true)
#PL_LDLIBS += -lnl-3 -lnl-genl-3
endif

#
ifeq ($(BUILD_TYPE),X86)
ifeq ($(strip $(MODULE_SQLITE)),true)
PL_LDLIBS += -lsqlite3
endif
endif
#
PL_CFLAGS += -fsigned-char -O2  
#
# WANRING
#	
PL_CFLAGS += -Wall -Wextra -Wnested-externs -Wmissing-prototypes \
			 -Wredundant-decls -Wcast-align -Wunreachable-code -Wshadow	\
			 -Wimplicit-function-declaration -Wimplicit	-Wreturn-type -Wunused \
			 -Wswitch -Wformat -Wuninitialized -Wchar-subscripts  \
			 -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
# -Werror=implicit-function-declaration -Werror=switch
PL_CFLAGS += -Werror=return-type -Werror=format-extra-args 
#			  -Werror=overlength-strings 
#			 -Werror=switch-default -Werror=missing-format-attribute
#			 
PL_CFLAGS += -fmessage-length=0 -Wcast-align
#PL_CFLAGS += -Werror
#

#
#
export DSTBINDIR = $(BASE_ROOT)/$(BINDIR)
export DSTSBINDIR = $(BASE_ROOT)/$(SBINDIR)
export DSTETCDIR = $(BASE_ROOT)/$(ETHDIR)
export DSTLIBDIR = $(BASE_ROOT)/$(LIBDIR)
#
#
#
#
#
#-include $(foreach prod,$(PLPRODS),$(prod)/gmake/$(firstword $(subst -,$(empty) $(empty),$(notdir $(prod)))).mk)
#
#
PLINCLUDE += $(PLLIBINCLUDE)
#
PL_DEBUG += $(PLDEFINE) $(EXTRA_DEFINE) 
#
export CFLAGS =  $(PL_CFLAGS) $(PL_DEBUG) -fPIC $(PLINCLUDE)
export LDCLFLAG = $(PL_LDLIBS) 
#
#
#
#
#

#
#
#
# $(CC) $(OBJS) $(CFLAGS)
#%.o: %.c iw.h nl80211.h
#	@$(NQ) ' CC  $(CFLAGS)' $@
#	$(Q)$(CC) $(CFLAGS) -c -o $@ $<
	
PL_OBJ_COMPILE = @$(CC) -fPIC $(CFLAGS) $(LDCLFLAG) -c  $< -o $@ $(PLINCLUDE)
PL_LIB_COMPILE = $(CC) -fPIC $(CFLAGS) $(LDCLFLAG) $^ -o $@ $(PLINCLUDE)
#
PL_MAKE_LIBSO = $(CC) -shared -o 
PL_MAKE_LIB = @$(AR) -rs
#
#
#
#
#

##
#

#
#
#
ifeq ($(CROSS_COMPILE),)
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
#ZPLOS_DEFINE += -DSYS_REAL_DIR=\"$(BASE_ROOT)/$(ZPL_RELEASEDIR)\"
ZPLOS_INCLUDE += -I/usr/include -I/usr/local/include 
ifeq ($(ZPL_BUILD_ARCH),X86_64)
ZPLOS_LDFLAGS += -L/lib64 -L/usr/lib64 -L/usr/local/lib64
else ifeq ($(ZPL_BUILD_ARCH),X86)
ZPLOS_LDFLAGS += -L/lib -L/usr/lib -L/usr/local/lib 
endif
#ZPLOS_LDFLAGS += -L/lib -L/usr/lib -L/lib64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/lib64
#
else
#
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

#export CROSS_COMPILE
#ZPLOS_INCLUDE += -I$(CROSS_COMPILE_INC)
#ZPLOS_LDFLAGS += -L$(CROSS_COMPILE_LIB)
#/home/zhurish/arm-himix200-linux/target/usr/include/
CROSS_COMPILE_PATH=$(CROSS_COMPILE_ROOT)
ZPLOS_INCLUDE += -I$(CROSS_COMPILE_PATH)/include -I$(CROSS_COMPILE_PATH)/usr/include 
ZPLOS_LDFLAGS += -L$(CROSS_COMPILE_PATH)/lib -L$(CROSS_COMPILE_PATH)/usr/lib

#ifneq ($(findstring $(CROSS_COMPILE), $(hi)), )
ZPLOS_INCLUDE += -I$(CROSS_COMPILE_PATH)/target/usr/include 
ZPLOS_CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
ZPLOS_LDFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
#endif

#ZPLOS_LDFLAGS += -L$(CROSS_COMPILE_PATH)/lib -L$(CROSS_COMPILE_PATH)/usr/lib
endif
#
#
#
ifeq ($(ZPL_BUILD_ARCH),X86_64)
ZPLOS_CFLAGS += -m64
else ifeq ($(ZPL_BUILD_ARCH),AARCH64)
ZPLOS_CFLAGS += -m64
endif
#
#
#
ZPLBASE = $(BASE_ROOT)
#

include $(ZPL_MAKE_DIR)/module.mk

include $(ZPL_MAKE_DIR)/module-config.mk

include $(ZPL_MAKE_DIR)/ipcom.mk

#
ZPLOS_INCLUDE += -I$(ZPLBASE)/include
#
ZPL_CFLAGS = -DBASE_DIR=\"$(ZPL_RUNNING_BASE_PATH)\" -DSYS_REAL_DIR=\"$(ZPL_REAL_SYSCONFIG_PATH)\"
#
#ZPLOS_LDFLAGS += $(ZPLEX_LDFLAGS)
#
#
ifeq ($(strip $(ZPL_WIFI_MODULE_SRC)),true)
#ZPL_LDLIBS += -lnl-3 -lnl-genl-3
endif
ifeq ($(strip $(ZPL_LIBSSH_MODULE)),true)
#ZPL_LDLIBS += -lutil -lcrypto -lrt
#-lz -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err
endif
#
ifeq ($(ZPL_BUILD_ARCH),X86_64)
ifeq ($(strip $(ZPL_SQLITE_MODULE)),true)
#ZPLOS_LDLIBS += -lsqlite3
endif
endif
#
#
#
#
ifeq ($(ZPL_IPCOM_STACK_MODULE),true)
ZPL_DEFINE += $(IPCOM_DEF) 
#-DZPL_IPCOM_STACK_MODULE
ZPL_INCLUDE += $(IPCOM_INCLUDE)
else
ZPLOS_DEFINE += -DZPL_BUILD_LINUX -DGNU_LINUX -D__linux__ -DLINUX=1
endif
#


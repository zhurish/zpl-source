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
#PLOS_DEFINE += -DSYS_REAL_DIR=\"$(BASE_ROOT)/$(RELEASEDIR)\"
PLOS_INCLUDE += -I/usr/include -I/usr/local/include 
ifeq ($(PL_BUILD_ARCH),X86_64)
PLOS_LDFLAGS += -L/lib64 -L/usr/lib64 -L/usr/local/lib64
else ifeq ($(PL_BUILD_ARCH),X86)
PLOS_LDFLAGS += -L/lib -L/usr/lib -L/usr/local/lib 
endif
#PLOS_LDFLAGS += -L/lib -L/usr/lib -L/lib64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/lib64
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

export CROSS_COMPILE
#PLOS_INCLUDE += -I$(CROSS_COMPILE_INC)
#PLOS_LDFLAGS += -L$(CROSS_COMPILE_LIB)

PLOS_INCLUDE += -I$(CROSS_COMPILE_PATH)/include -I$(CROSS_COMPILE_PATH)/usr/include 
PLOS_LDFLAGS += -L$(CROSS_COMPILE_PATH)/lib -L$(CROSS_COMPILE_PATH)/usr/lib

endif
#
#
#
ifeq ($(PL_BUILD_ARCH),X86_64)
PLOS_CFLAGS += -m64
else ifeq ($(PL_BUILD_ARCH),AARCH64)
PLOS_CFLAGS += -m64
endif
#
#
#
PLBASE = $(BASE_ROOT)
#

include $(MAKE_DIR)/module.mk

include $(MAKE_DIR)/module-config.mk

include $(MAKE_DIR)/ipcom.mk

#
PLOS_INCLUDE += -I$(PLBASE)/include
#
PL_CFLAGS = -DBASE_DIR=\"$(PL_RUNNING_BASE_PATH)\" -DSYS_REAL_DIR=\"$(PL_REAL_SYSCONFIG_PATH)\"
#
#PLOS_LDFLAGS += $(PLEX_LDFLAGS)
#
#
ifeq ($(strip $(PL_WIFI_MODULE_SRC)),true)
#PL_LDLIBS += -lnl-3 -lnl-genl-3
endif
ifeq ($(strip $(PL_LIBSSH_MODULE)),true)
#PL_LDLIBS += -lutil -lcrypto -lrt
#-lz -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err
endif
#
ifeq ($(PL_BUILD_ARCH),X86_64)
ifeq ($(strip $(PL_SQLITE_MODULE)),true)
#PLOS_LDLIBS += -lsqlite3
endif
endif
#
#
#
#
ifeq ($(PL_IPCOM_STACK_MODULE),true)
PL_DEFINE += $(IPCOM_DEF) -DUSE_IPSTACK_IPCOM

PL_INCLUDE += $(IPCOM_INCLUDE)
else
PLOS_DEFINE += -DUSE_IPSTACK_KERNEL -DUSE_LINUX_OS -D__linux__
endif
#


##
#
#
#TARGET_CFLAGS:=$(TARGET_OPTIMIZATION)$(if $(CONFIG_DEBUG), -g3) $(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))
#TARGET_CXXFLAGS = $(TARGET_CFLAGS)
#TARGET_ASFLAGS_DEFAULT = $(TARGET_CFLAGS)
#TARGET_ASFLAGS = $(TARGET_ASFLAGS_DEFAULT)
#TARGET_CPPFLAGS:=-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include
#TARGET_LDFLAGS:=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib
#
#TOOLCHAIN_DIR=/home/zhurish/application/openwrt-lede/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24"
#TARGET_DSTROOTFSDIR=/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24
#TOOLCHAIN_DIR_NAME=toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#PL_BUILD_DIR_TOOLCHAIN=/home/zhurish/application/openwrt-lede/build_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#TARGET_CROSS=mipsel-openwrt-linux-gnu-
#TARGET_CONFIGURE_OPTS=AR=mipsel-openwrt-linux-gnu-gcc-ar AS=mipsel-openwrt-linux-gnu-gcc -c -Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt -fhonour-copts -Wno-error=unused-but-set-variable -Wno-error=unused-result -msoft-float -iremap/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24/X5-B:X5-B -Wformat -Werror=format-security -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro LD=mipsel-openwrt-linux-gnu-ld NM=mipsel-openwrt-linux-gnu-gcc-nm CC=mipsel-openwrt-linux-gnu-gcc GCC=mipsel-openwrt-linux-gnu-gcc CXX=mipsel-openwrt-linux-gnu-g++ RANLIB=mipsel-openwrt-linux-gnu-gcc-ranlib STRIP=mipsel-openwrt-linux-gnu-strip OBJCOPY=mipsel-openwrt-linux-gnu-objcopy OBJDUMP=mipsel-openwrt-linux-gnu-objdump SIZE=mipsel-openwrt-linux-gnu-size
#
#ifneq ($(TOOLCHAIN_DIR),)
#export CROSS_COMPILE_ROOT = $(TOOLCHAIN_DIR)
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/$(TARGET_CROSS)
#endif
#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc
#CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
#
#ifneq ($(PL_BUILD_ARCH),X86_64)
ifneq ($(CROSS_COMPILE),)
#$(error CROSS_COMPILE is not define)
#endif
export AS=$(CROSS_COMPILE)as
export LD=$(CROSS_COMPILE)ld
export CC=$(CROSS_COMPILE)gcc
export CXX=$(CROSS_COMPILE)g++
export CPP=$(CC) -E
export AR=$(CROSS_COMPILE)ar
export NM=$(CROSS_COMPILE)nm
export LDR=$(TARGET_CROSS)ldr
export STRIP=$(CROSS_COMPILE)strip
export OBJCOPY=$(CROSS_COMPILE)objcopy
export OBJDUMP=$(CROSS_COMPILE)objdump
export RANLIB=$(CROSS_COMPILE)ranlib
endif


ifneq ($(TARGET_CFLAGS),)
PLOS_CFLAGS += $(TARGET_CFLAGS) 
PLOS_CPPFLAGS += $(TARGET_CXXFLAGS) $(TARGET_CPPFLAGS)
PLOS_ASFLAGS += $(TARGET_ASFLAGS) 
PLOS_LDFLAGS += $(TARGET_LDFLAGS) -luci -lssp 
#-lubacktrace
PLOS_CFLAGS += -fstack-protector-all -fstack-protector
endif

ifneq ($(CROSS_COMPILE_ROOT),)
PLOS_INCLUDE += -I$(CROSS_COMPILE_ROOT)/include -I$(CROSS_COMPILE_ROOT)/usr/include 
PLOS_LDFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib 
endif

#PLOS_CFLAGS += -mips32
#endif
#
#
#
ifeq ($(PL_BUILD_ARCH),X86_64)
PLOS_CFLAGS += -m64
else
ifneq ($(PL_BUILD_ARCH),AARCH64)
PLOS_CFLAGS += -m64
endif
endif
#
#
export OPENEWRT_BASE = $(TOOLCHAIN_DIR)
#

#PL_CFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib

#
#
#
#
PLBASE = $(BASE_ROOT)
#
include $(MAKE_DIR)/module.mk

include $(MAKE_DIR)/module-config.mk

include $(MAKE_DIR)/ipcom.mk
#
PLOS_INCLUDE += $(OPENWRT_INCLUDE) -I$(PLBASE)/include
#
PL_CFLAGS = -DBASE_DIR=\"$(PL_RUNNING_BASE_PATH)\" -DSYS_REAL_DIR=\"$(PL_REAL_SYSCONFIG_PATH)\"
#
PLOS_CFLAGS += $(OPENWRT_LDFLAGS)
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
PLOS_DEFINE += -DUSE_LINUX_OS -D__linux__
#


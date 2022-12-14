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
#ZPL_BUILD_DIR_TOOLCHAIN=/home/zhurish/application/openwrt-lede/build_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
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
#ifneq ($(ZPL_BUILD_ARCH),X86_64)
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
ZPLOS_CFLAGS += $(TARGET_CFLAGS) 
ZPLOS_CPPFLAGS += $(TARGET_CXXFLAGS) $(TARGET_CPPFLAGS)
ZPLOS_ASFLAGS += $(TARGET_ASFLAGS) 
ZPLOS_LDFLAGS += $(TARGET_LDFLAGS) -luci -lssp 
#-lubacktrace
ZPLOS_CFLAGS += -fstack-protector-all -fstack-protector
endif

ifneq ($(CROSS_COMPILE_ROOT),)
ZPLOS_INCLUDE += -I$(CROSS_COMPILE_ROOT)/include -I$(CROSS_COMPILE_ROOT)/usr/include 
ZPLOS_LDFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib 
endif

#ZPLOS_CFLAGS += -mips32
#endif
#
#
#
ifeq ($(ZPL_BUILD_ARCH),X86_64)
ZPLOS_CFLAGS += -m64
else
ifneq ($(ZPL_BUILD_ARCH),AARCH64)
ZPLOS_CFLAGS += -m64
endif
endif
#
#
export OPENEWRT_BASE = $(TOOLCHAIN_DIR)
#

#ZPL_CFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib

#
#
#
#
ZPLBASE = $(BASE_ROOT)
#
include $(ZPL_MAKE_DIR)/module.mk

include $(ZPL_MAKE_DIR)/module-config.mk

include $(ZPL_MAKE_DIR)/ipcom.mk
#
ZPLOS_INCLUDE += $(OPENWRT_INCLUDE) -I$(ZPLBASE)/include
#
ZPL_CFLAGS = -DBASE_DIR=\"$(ZPL_RUNNING_BASE_PATH)\" -DSYS_REAL_DIR=\"$(ZPL_REAL_SYSCONFIG_PATH)\"
#
ZPLOS_CFLAGS += $(OPENWRT_LDFLAGS)
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


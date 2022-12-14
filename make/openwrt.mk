##
#
#export TOOLCHAIN_DIR
#TARGET_CFLAGS:=$(TARGET_OPTIMIZATION)$(if $(CONFIG_DEBUG), -g3) $(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))
#TARGET_CXXFLAGS = $(TARGET_CFLAGS)
#TARGET_ASFLAGS_DEFAULT = $(TARGET_CFLAGS)
#TARGET_ASFLAGS = $(TARGET_ASFLAGS_DEFAULT)
#TARGET_CPPFLAGS:=-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include
#TARGET_LDFLAGS:=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib
#echo "=========TOOLCHAIN_DIR========/home/zhurish/application/openwrt-lede/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24"
#=========TOOLCHAIN_DIR========/home/zhurish/application/openwrt-lede/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#echo "=========TARGET_ROOTFS_DIR====/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24"
#=========TARGET_ROOTFS_DIR====/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24
#echo "======TOOLCHAIN_DIR_NAME=======toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24"
#======TOOLCHAIN_DIR_NAME=======toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#echo "=====BUILD_DIR_TOOLCHAIN===/home/zhurish/application/openwrt-lede/build_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24"
#=====BUILD_DIR_TOOLCHAIN===/home/zhurish/application/openwrt-lede/build_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#echo "=======TARGET_CROSS======mipsel-openwrt-linux-gnu-"
#=======TARGET_CROSS======mipsel-openwrt-linux-gnu-
#echo "=====TARGET_CONFIGURE_OPTS========AR="mipsel-openwrt-linux-gnu-gcc-ar" AS="mipsel-openwrt-linux-gnu-gcc -c -Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt -fhonour-copts -Wno-error=unused-but-set-variable -Wno-error=unused-result -msoft-float -iremap/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24/X5-B:X5-B -Wformat -Werror=format-security -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro" LD=mipsel-openwrt-linux-gnu-ld NM="mipsel-openwrt-linux-gnu-gcc-nm" CC="mipsel-openwrt-linux-gnu-gcc" GCC="mipsel-openwrt-linux-gnu-gcc" CXX="mipsel-openwrt-linux-gnu-g++" RANLIB="mipsel-openwrt-linux-gnu-gcc-ranlib" STRIP=mipsel-openwrt-linux-gnu-strip OBJCOPY=mipsel-openwrt-linux-gnu-objcopy OBJDUMP=mipsel-openwrt-linux-gnu-objdump SIZE=mipsel-openwrt-linux-gnu-size"	
#=====TARGET_CONFIGURE_OPTS========AR=mipsel-openwrt-linux-gnu-gcc-ar AS=mipsel-openwrt-linux-gnu-gcc -c -Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt -fhonour-copts -Wno-error=unused-but-set-variable -Wno-error=unused-result -msoft-float -iremap/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24/X5-B:X5-B -Wformat -Werror=format-security -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro LD=mipsel-openwrt-linux-gnu-ld NM=mipsel-openwrt-linux-gnu-gcc-nm CC=mipsel-openwrt-linux-gnu-gcc GCC=mipsel-openwrt-linux-gnu-gcc CXX=mipsel-openwrt-linux-gnu-g++ RANLIB=mipsel-openwrt-linux-gnu-gcc-ranlib STRIP=mipsel-openwrt-linux-gnu-strip OBJCOPY=mipsel-openwrt-linux-gnu-objcopy OBJDUMP=mipsel-openwrt-linux-gnu-objdump SIZE=mipsel-openwrt-linux-gnu-size


#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc
#CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
#
#ifneq ($(BUILD_TYPE),X86)
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
PL_CFLAGS += $(TARGET_LDFLAGS)
#-L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib

#endif
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

#PL_CFLAGS += -L$(CROSS_COMPILE_ROOT)/lib -L$(CROSS_COMPILE_ROOT)/usr/lib
PL_CFLAGS += -DBUILD_$(BUILD_TYPE)
#
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


#
#
# Define LINUX PL_BUILD
#
# X86, ARM, MIPS, PPC, SUN, SPARC, S390, PARISC, POWERPC, SH, AARCH, TILE
#
ifeq ($(MENUCONFIG_PL_BUILD),true)
include $(MENUCONFIG_PL_CONFIG)
else

ifeq ($(ARCH_TYPE),)
#ARCH_TYPE=ARM
ARCH_TYPE=X86
#ARCH_TYPE=MIPS
endif
#
ifeq ($(GCC_TYPE),)
#GCC_TYPE=UCLIBC
GCC_TYPE=GCLIBC
#GCC_TYPE=MUSL
endif
#
#
#
VERSION = V0.0.0.20
#
ARCH_OS	= linux
#
IPV6_ENABLE = true
#
ifeq ($(OPENWRT),true)
PL_BUILD_OPENWRT=true
else
PL_BUILD_OPENWRT=false
endif
#
#
ARCH_DEBUG=YES
#
#
ifneq ($(ARCH_TYPE),ARM)
ifeq ($(PL_BUILD_OPENWRT),false)
#ifeq ($(CROSS_COMPILE),)
#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_musl
CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc
#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2
#CROSS_COMPILE_ROOT = /home/zhurish/workspace/openwrt_CC_mt76xx_zhuotk_source/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2
#
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/arm-cortexa9-linux-gnueabihf-
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
#endif
else
ifeq ($(CROSS_COMPILE),)
#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
endif
endif
endif
#
#
ifeq ($(ARCH_TYPE),ARM)
CROSS_COMPILE_ROOT = /opt/toolchain/arm-cortexa9/4.9.3/
export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/arm-cortexa9-linux-gnueabihf-
endif
#
#
ifeq ($(ARCH_TYPE),X86)
CROSS_COMPILE_ROOT =
export CROSS_COMPILE =
endif
#
#
#
endif
#
#
#

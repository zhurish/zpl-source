# ==========================================================================
# Build system
# ==========================================================================
include $(SOURCE_ROOT)/build/module-def.mk

ifeq ($(ZPL_SYSTEM_LINUX),true)
export ARCH_OS	= linux
export OPENWRT=false
endif

ifeq ($(ZPL_SYSTEM_OPENWRT),true)
export ARCH_OS	= linux
export OPENWRT=true
endif

ifeq ($(ZPL_SYSTEM_MINGW32),true)
export ARCH_OS	= MINGW32
endif

ifeq ($(ZPL_SYSTEM_MINGW64),true)
export ARCH_OS	= MINGW32
endif

ifeq ($(ZPL_SYSTEM_MSVC32),true)
export ARCH_OS	= WIN32
endif

ifeq ($(ZPL_SYSTEM_MSVC64),true)
export ARCH_OS	= WIN64
endif


ifeq ($(ZPL_ARCH_ARM),true)
export ARCH_TYPE=ARM
endif

ifeq ($(ZPL_ARCH_ARM64),true)
export ARCH_TYPE=ARM64
endif

ifeq ($(ZPL_ARCH_AARCH64),true)
export ARCH_TYPE=AARCH64
endif

ifeq ($(ZPL_ARCH_X86),true)
export ARCH_TYPE=X86
endif

ifeq ($(ZPL_ARCH_X86_64),true)
export ARCH_TYPE=X86_64
endif

ifeq ($(ZPL_ARCH_MIPS32),true)
export ARCH_TYPE=MIPS32
endif

ifeq ($(ZPL_ARCH_MIPS64),true)
export ARCH_TYPE=MIPS64
endif

ifeq ($(ZPL_ARCH_LOONGARCH64),true)
export ARCH_TYPE=LOONGARCH64
endif

ifeq ($(ZPL_ARCH_RISCV32),true)
export ARCH_TYPE=RISCV32
endif

ifeq ($(ZPL_ARCH_RISCV64),true)
export ARCH_TYPE=RISCV64
endif


ifeq ($(ZPL_EXTERNAL_TOOLCHAIN_MODULE),true)

ifeq ($(ZPL_TOOLCHAIN_GLIBC),true)
export GCC_TYPE=GCLIBC
endif
ifeq ($(ZPL_TOOLCHAIN_UCLIBC),true)
export GCC_TYPE=UCLIBC
endif

else
export GCC_TYPE=GCLIBC
endif

ifneq ($(ZPL_COMPILE_OPTIONS),)
export ARCH_DEBUG=YES
endif

export VERSION = V0.0.0.27
export IPV6_ENABLE = true

ifeq ($(ZPL_EXTERNAL_TOOLCHAIN_MODULE),true)
CROSS_COMPILE_ROOT = $(subst ",,$(ZPL_TOOLCHAIN_PATH))
TOOLCHAIN_PREFIX = $(subst ",,$(ZPL_TOOLCHAIN_PREFIX))
export CROSS_COMPILE=$(CROSS_COMPILE_ROOT)/bin/$(TOOLCHAIN_PREFIX)


#
#去掉双引号
#
ZPL_DEFINE = $(subst ",,$(ZPL_TOOLCHAIN_CFLAGS))
#
#CROSS_COMPILE_ROOT = /opt/gcc-linaro-7.5.0-arm-linux-gnueabihf
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/arm-linux-gnueabihf-
#CROSS_COMPILE_ROOT = /home/zhurish/arm-himix200-linux
#export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/arm-himix200-linux-
#
#ZPLOS_CFLAGS += -D__ARM_PCS_VFP
#ZPLOS_CPPFLAGS  += -D__ARM_PCS_VFP
#
else
export CROSS_COMPILE=
endif

#
ifeq ($(ZPL_KERNEL_MODULE),true)
export ZPL_KERNEL_MODULE=true 
export ZPL_KERNEL_NETLINK=true
endif
ifeq ($(ZPL_IPCOM_MODULE),true)
export ZPL_IPCOM_MODULE=true
export ZPL_IPCOM_BASE_PATH=$(subst ",,$(ZPL_IPCOM_ROOT_PATH))
endif

#
#
#
#

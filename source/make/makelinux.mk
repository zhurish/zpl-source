##
#
#ZPLOS_CFLAGS		C 系统FLAGS
#ZPLEX_CFLAGS		C 第三方库FLAGS
#ZPL_CFLAGS			C 平台FLAGS
#
#ZPLOS_CPPFLAGS		C++ 系统FLAGS
#ZPLEX_CPPFLAGS		C++ 第三方库FLAGS
#ZPL_CPPFLAGS		C++ 平台FLAGS
#
#ZPLOS_ASFLAGS		汇编 系统FLAGS
#ZPLEX_ASFLAGS		汇编 第三方库FLAGS
#ZPL_ASFLAGS			汇编 平台FLAGS
#
#ZPLOS_LDFLAGS		系统库目录
#ZPLEX_LDFLAGS		第三方库目录
#ZPL_LDFLAGS			平台库目录
#
#ZPLOS_LDLIBS		系统库
#ZPLEX_LDLIBS		第三方库
#ZPL_LDLIBS			平台库
#
#ZPLOS_DEFINE		系统定义
#ZPLEX_DEFINE		第三方定义
#ZPL_DEFINE			平台库定义
#
#ZPLOS_INCLUDE		系统头文件
#ZPLEX_INCLUDE		第三方头文件
#ZPL_INCLUDE			平台库头文件
#
#ZPL_CFLAGS			C 平台FLAGS
#
#ZPLM_CPPFLAGS		模块系统FLAGS
#ZPLM_ASFLAGS		模块汇编 系统FLAGS
#ZPLM_LDFLAGS		模块库目录
#ZPLM_LDLIBS			模块库
#ZPLM_DEFINE			模块定义
#ZPLM_INCLUDE		模块头文件
#
#= 是最基本的赋值
#:= 是覆盖之前的值
#?= 是如果没有被赋值过就赋予等号后面的值
#+= 是添加等号后面的值
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
INSTALL = install -c
LN = ln -s
#SHELL := C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe
SHELL := sh
#
#
#
ifneq ($(TOP_DIR),)
ROOT_DIR = $(TOP_DIR)
else
ROOT_DIR = $(CURDIR)
#$(shell pwd)
endif
#ROOT_DIR = D:/source/SWPlatform/source
BASE_ROOT = $(ROOT_DIR)

export ZPL_MAKE_DIR = $(BASE_ROOT)/make
export BASE_ROOT = $(ROOT_DIR)
#
#
#include $(ZPL_MAKE_DIR)/board.mk
include $(MENUCONFIG_ZPL_CONFIG)
# 
#
#
ZPL_BUILD_ARCH	=$(shell echo $(ARCH_TYPE) | tr a-z A-Z)
#
ZPL_BUILD_DEBUG	=$(shell echo $(ARCH_DEBUG) | tr a-z A-Z)
#
#ZPL_BUILD_OS	=$(ARCH_OS)
ZPL_BUILD_OS	=$(shell echo $(ARCH_OS) | tr a-z A-Z) 
#
ZPL_RUN_ARCH	=$(shell echo $(ARCH_TYPE) | tr A-Z a-z)
ZPL_RUN_OS	=$(shell echo $(ARCH_OS) | tr A-Z a-z) 
#
#ZPL_BUILD_IPV6	= false
ZPL_BUILD_IPV6	= $(ZPL_IPV6_MODULE)
#
#
#
ifeq ($(ZPL_BUILD_DEBUG),NO)
ZPL_RELEASEDIR = debug
#release
ZPL_OBJ_DIR = $(ZPL_RELEASEDIR)/obj
ZPL_LIB_DIR = $(ZPL_RELEASEDIR)/lib
ZPL_BIN_DIR = $(ZPL_RELEASEDIR)/bin
ZPL_SBIN_DIR = $(ZPL_RELEASEDIR)/sbin
ZPL_ETC_DIR = $(ZPL_RELEASEDIR)/etc
ZPL_ULIB_DIR = $(ZPL_RELEASEDIR)/usr/lib
ZPL_UINC_DIR = $(ZPL_RELEASEDIR)/usr/include
ZPL_WWW_DIR = $(ZPL_RELEASEDIR)/www
ZPL_KO_DIR = $(ZPL_RELEASEDIR)/ko
ZPLOS_CFLAGS += -s
ZPLOS_MAP =
else
ZPL_RELEASEDIR = debug
ZPL_OBJ_DIR = $(ZPL_RELEASEDIR)/obj
ZPL_LIB_DIR = $(ZPL_RELEASEDIR)/lib
ZPL_BIN_DIR = $(ZPL_RELEASEDIR)/bin
ZPL_SBIN_DIR = $(ZPL_RELEASEDIR)/sbin
ZPL_ETC_DIR = $(ZPL_RELEASEDIR)/etc
ZPL_ULIB_DIR = $(ZPL_RELEASEDIR)/usr/lib
ZPL_UINC_DIR = $(ZPL_RELEASEDIR)/usr/include
ZPL_WWW_DIR = $(ZPL_RELEASEDIR)/www
ZPL_KO_DIR = $(ZPL_RELEASEDIR)/ko
#ZPL_ULIB_DIR = $(ZPL_RELEASEDIR)/www/spool
#ZPL_ULIB_DIR = $(ZPL_RELEASEDIR)/www/spool/cache
#ZPLOS_MAP = -Wl,-Map,target-app.map
ZPLOS_MAP = -Wl,-Map,
endif
#
#

ifeq ($(strip $(ZPL_BUILD_ARCH)),$(filter $(ZPL_BUILD_ARCH),X86_64 X86))
ZPL_DEFINE = -UBASE_DIR -USYS_REAL_DIR -DBASE_DIR=\"$(BASE_ROOT)/$(ZPL_RELEASEDIR)/tmp/app\" -DSYS_REAL_DIR=\"$(BASE_ROOT)/$(ZPL_RELEASEDIR)\"
endif
#
#
ifeq ($(ZPL_BUILD_DEBUG),YES)
ZPLOS_CFLAGS += -g
#3 -ggdb
ZPLOS_CPPFLAGS += -g
#3 -ggdb
else
ZPLOS_CFLAGS += -O2
ZPLOS_CPPFLAGS += -O2
endif
#
#
-include $(ZPL_MAKE_DIR)/gcc-config.mk
#
#

ROOTFS_DIR = rootfs_install
export ZPL_INSTALL_ROOTFS_DIR = $(BASE_ROOT)/$(ROOTFS_DIR)
export DSTROOTFSDIR=$(ZPL_INSTALL_ROOTFS_DIR)
#
#
ifeq ($(ZPL_BUILD_OPENWRT),true)
include $(ZPL_MAKE_DIR)/openwrt.mk
ZPLOS_DEFINE += -DZPL_BUILD_OPENWRT
ZPLOS_DEFINE += -DZPL_BUILD_LINUX -DGNU_LINUX -D__linux__ -DLINUX=1
endif
ifeq ($(ZPL_SYSTEM_LINUX),true)
include $(ZPL_MAKE_DIR)/linux.mk
ZPLOS_DEFINE += -DZPL_BUILD_LINUX -DGNU_LINUX -D__linux__ -DLINUX=1
endif
#
ifeq ($(ZPL_SYSTEM_MINGW32),true)
include $(ZPL_MAKE_DIR)/mingw.mk #mingw_nt
ZPL_SYSTEM_WIN32=true
endif
ifeq ($(ZPL_SYSTEM_MINGW64),true)
include $(ZPL_MAKE_DIR)/mingw.mk
ZPL_SYSTEM_WIN32=true
endif
ifeq ($(ZPL_SYSTEM_MSVC32),true)
include $(ZPL_MAKE_DIR)/mingw.mk
ZPL_SYSTEM_WIN32=true
endif
ifeq ($(ZPL_SYSTEM_MSVC64),true)
include $(ZPL_MAKE_DIR)/mingw.mk
ZPL_SYSTEM_WIN32=true
endif
#
ifeq ($(GCC_TYPE),UCLIBC)
ZPLOS_CFLAGS += -D__UCLIBC__
endif
#
#
ZPLOS_DEFINE += -DZPL_BUILD_ARCH_$(ZPL_BUILD_ARCH)
ZPLOS_DEFINE += -DZPL_BUILD_OS_$(ZPL_BUILD_OS)
#
#
#
ifeq ($(ZPL_IPCOM_MODULE),true)
ZPLOS_DEFINE += -DZPL_IPCOM_MODULE
else
ZPLOS_DEFINE += -DZPL_KERNEL_MODULE
endif
#
#
#ZPL_BUILD_TIME=$(shell date -u "+%Y%m%d%H%M%S")
ZPL_BUILD_TIME=$(shell date "+%Y%m%d%H%M%S")
#
#
ifeq ($(ZPL_BUILD_DEBUG),YES)
ZPLVER = $(VERSION).bin
ZPL_DEFINE += -DZPL_BUILD_DEBUG
else
ZPLVER = $(VERSION)-$(ZPL_BUILD_TIME).bin
endif
#
#
#
ifeq ($(strip $(ZPL_BUILD_IPV6)),true)
ZPLOS_DEFINE += -DZPL_BUILD_IPV6
endif
#
#
#
#
ZPLOS_LDLIBS += -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s -lstdc++ -lresolv
#
ZPL_LDFLAGS += -L$(BASE_ROOT)/$(ZPL_LIB_DIR) -L$(BASE_ROOT)/$(ZPL_ULIB_DIR)
#
#
#
ZPLOS_DEFINE += -DZPL_BUILD_VERSION=\"$(VERSION)\" -DZPL_BUILD_TIME=\"$(ZPL_BUILD_TIME)\"
#
#
export ZPL_INSTALL_BIN_DIR = $(BASE_ROOT)/$(ZPL_BIN_DIR)
export ZPL_INSTALL_SBIN_DIR = $(BASE_ROOT)/$(ZPL_SBIN_DIR)
export ZPL_INSTALL_ETC_DIR = $(BASE_ROOT)/$(ZPL_ETC_DIR)
export ZPL_INSTALL_LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)
export ZPL_INSTALL_ULIB_DIR = $(BASE_ROOT)/$(ZPL_ULIB_DIR)
export ZPL_INSTALL_WWW_DIR = $(BASE_ROOT)/$(ZPL_WWW_DIR)
export ZPL_INSTALL_INC_DIR = $(BASE_ROOT)/$(ZPL_UINC_DIR)
export ZPL_INSTALL_PREFIX_DIR = $(BASE_ROOT)/$(ZPL_RELEASEDIR)
export ZPL_INSTALL_KO_DIR = $(BASE_ROOT)/$(ZPL_KO_DIR)

#
#
#
#
#-include $(foreach prod,$(ZPLPRODS),$(prod)/gmake/$(firstword $(subst -,$(empty) $(empty),$(notdir $(prod)))).mk)
#
#
# gcc的 -MMD 选项可以自动生成带有依赖规则的.d文件，为创建头文件依赖带来了方便
#
#
#
export ZPLCFLAGS = $(ZPLOS_CFLAGS) $(ZPLEX_CFLAGS) $(ZPL_CFLAGS) $(ZPLM_CFLAGS)
#
export ZPLCPPFLAGS = $(ZPLOS_CPPFLAGS) $(ZPLEX_CPPFLAGS) $(ZPL_CPPFLAGS) $(ZPLM_CPPFLAGS)
#
export ZPLASFLAGS = $(ZPLOS_ASFLAGS) $(ZPLEX_ASFLAGS) $(ZPL_ASFLAGS) $(ZPLM_ASFLAGS)
#
export ZPLLDFLAGS = $(ZPLOS_LDFLAGS) $(ZPLEX_LDFLAGS) $(ZPL_LDFLAGS) $(ZPLM_LDFLAGS)
#
export ZPLLDLIBS = $(ZPLOS_LDLIBS) $(ZPLEX_LDLIBS) $(ZPL_LDLIBS) $(ZPLM_LDLIBS)
#
export ZPLINCLUDE = $(ZPLOS_INCLUDE) $(ZPLEX_INCLUDE) $(ZPL_INCLUDE) $(ZPLM_INCLUDE)
#
export ZPLDEFINE = $(ZPLOS_DEFINE) $(ZPLEX_DEFINE) $(ZPL_DEFINE) $(ZPLM_DEFINE)
#
export ZPLDEBUG = $(ZPL_DEBUG)
#
# 
export ZPLSTRIP_CFLAGS= --strip-unneeded
# 
#export ZPLCFLAGS += $(ZPLOS_CFLAGS) $(ZPLEX_CFLAGS) $(ZPL_CFLAGS) $(ZPL_DEBUG) -fPIC $(ZPLINCLUDE)
#export ZPLLDCLFLAG += $(ZPLOS_LDFLAGS) $(ZPLEX_LDFLAGS) $(ZPL_LDFLAGS) $(ZPLOS_LDLIBS) $(ZPLEX_LDLIBS) $(ZPL_LDLIBS) 
#
#
#
#ifeq ($(VERBOSE),)
#SILENCE=@$(ECHO) "building : "$@ ;
#else
#SILENCE=
#endif
#	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
#	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#
ifneq ($(strip $(V)),)
ZPL_ECHO_CC = $(ECHO) "building " $@   '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS)';
ZPL_ECHO_CXX = $(ECHO) "building " $@   '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS)';
ZPL_ECHO_AS = $(ECHO) "building " $@   '$(ZPLASFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS)';
else
ZPL_ECHO_CC = $(ECHO) "building " $@   '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG)';
ZPL_ECHO_CXX = $(ECHO) "building " $@   '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG)';
ZPL_ECHO_AS = $(ECHO) "building " $@   '$(ZPLASFLAGS) $(ZPLDEFINE) $(ZPLDEBUG)';
endif
#
#
#
#

#
#
ZPL_LIB_COMPILE = $(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#ZPL_OBJ_COMPILE = @$(CC) -fPIC $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCFLAGS) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#ZPL_LIB_COMPILE = $(CC) -fPIC $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCFLAGS) $(ZPLLDFLAGS) $^ -o $@ $(ZPLINCLUDE)
#
#ZPL_MAKE_LIBSO = $(CC) -shared -o 
#
#ZPL_CXX_OBJ_COMPILE = @$(CXX) -fPIC $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCPPFLAGS) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#ZPL_CXX_LIB_COMPILE = $(CXX) -fPIC $(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $^ -o $@ $(ZPLINCLUDE)
ZPL_CXX_LIB_COMPILE = $(CXX) -fPIC $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCPPFLAGS) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#

#ZPL_CXX_MAKE_LIBSO = $(CXX) -shared -o 
#
#ZPL_MAKE_LIB = @$(AR) -rs
#
# files = $(patsubst %.o,%.c,$(objects))
#
#
#

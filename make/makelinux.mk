##
#
#PLOS_CFLAGS		C 系统FLAGS
#PLEX_CFLAGS		C 第三方库FLAGS
#PL_CFLAGS			C 平台FLAGS
#
#PLOS_CPPFLAGS		C++ 系统FLAGS
#PLEX_CPPFLAGS		C++ 第三方库FLAGS
#PL_CPPFLAGS		C++ 平台FLAGS
#
#PLOS_ASFLAGS		汇编 系统FLAGS
#PLEX_ASFLAGS		汇编 第三方库FLAGS
#PL_ASFLAGS			汇编 平台FLAGS
#
#PLOS_LDFLAGS		系统库目录
#PLEX_LDFLAGS		第三方库目录
#PL_LDFLAGS			平台库目录
#
#PLOS_LDLIBS		系统库
#PLEX_LDLIBS		第三方库
#PL_LDLIBS			平台库
#
#PLOS_DEFINE		系统定义
#PLEX_DEFINE		第三方定义
#PL_DEFINE			平台库定义
#
#PLOS_INCLUDE		系统头文件
#PLEX_INCLUDE		第三方头文件
#PL_INCLUDE			平台库头文件
#
#PL_CFLAGS			C 平台FLAGS
#
#PLM_CPPFLAGS		模块系统FLAGS
#PLM_ASFLAGS		模块汇编 系统FLAGS
#PLM_LDFLAGS		模块库目录
#PLM_LDLIBS			模块库
#PLM_DEFINE			模块定义
#PLM_INCLUDE		模块头文件
#
#= 是最基本的赋值
#:= 是覆盖之前的值
#?= 是如果没有被赋值过就赋予等号后面的值
#+= 是添加等号后面的值
#
#
#
ifneq ($(TOP_DIR),)
ROOT_DIR = $(TOP_DIR)
else
ROOT_DIR = $(shell pwd)
#/home/zhurish/workspace/SWPlatform
endif

BASE_ROOT = $(ROOT_DIR)

export MAKE_DIR = $(BASE_ROOT)/make
export BASE_ROOT = $(ROOT_DIR)
#
#
include $(MAKE_DIR)/board.cfg
#
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
#
#
BUILD_TYPE	=$(ARCH_TYPE)
#
BUILD_DEBUG	=$(ARCH_DEBUG)
#
#
#BUILD_IPV6	= false
BUILD_IPV6	= true
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
ifeq ($(BUILD_DEBUG),NO)
RELEASEDIR = debug
#release
OBJDIR = $(RELEASEDIR)/obj
LIBDIR = $(RELEASEDIR)/lib
BINDIR = $(RELEASEDIR)/bin
SBINDIR = $(RELEASEDIR)/sbin
ETCDIR = $(RELEASEDIR)/etc
ULIBDIR = $(RELEASEDIR)/usr/lib
WWWDIR = $(RELEASEDIR)/www
PLOS_CFLAGS += -s
PLOS_MAP =
else
RELEASEDIR = debug
OBJDIR = $(RELEASEDIR)/obj
LIBDIR = $(RELEASEDIR)/lib
BINDIR = $(RELEASEDIR)/bin
SBINDIR = $(RELEASEDIR)/sbin
ETCDIR = $(RELEASEDIR)/etc
ULIBDIR = $(RELEASEDIR)/usr/lib
WWWDIR = $(RELEASEDIR)/www
#ULIBDIR = $(RELEASEDIR)/www/spool
#ULIBDIR = $(RELEASEDIR)/www/spool/cache
#PLOS_MAP = -Wl,-Map,target-app.map
PLOS_MAP = -Wl,-Map,
endif
#
#
ifeq ($(BUILD_OPENWRT),true)
include $(MAKE_DIR)/openwrt.mk
PLOS_DEFINE += -DBUILD_OPENWRT
else
include $(MAKE_DIR)/linux.mk
PLOS_DEFINE += -DBUILD_LINUX
endif
#

#
PL_LDFLAGS += -L$(BASE_ROOT)/$(LIBDIR) -L$(BASE_ROOT)/$(ULIBDIR)
#
#
ifeq ($(BUILD_DEBUG),YES)
PL_CFLAGS +=  -g
endif
#
#
ifeq ($(USE_IPCOM_STACK),true)
PLOS_DEFINE += -DUSE_IPCOM_STACK
else
PLOS_DEFINE += -DUSE_IPSTACK_KERNEL
endif
#
#
#BUILD_TIME=$(shell date -u "+%Y%m%d%H%M%S")
BUILD_TIME=$(shell date "+%Y%m%d%H%M%S")
#
#
ifeq ($(BUILD_DEBUG),YES)
PLVER = $(VERSION).bin
else
PLVER = $(VERSION)-$(BUILD_TIME).bin
endif
#
#
#
ifeq ($(strip $(BUILD_IPV6)),true)
PLOS_DEFINE += -DBUILD_IPV6
endif
#
#
#
ifeq ($(GCC_TYPE),UCLIBC)
PLOS_CFLAGS += -D__UCLIBC__
endif
#
#
PLOS_LDLIBS += -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s -lstdc++
#
#
#PLOS_LDLIBS += -std=c99 
PLOS_CFLAGS += -std=gnu99 -fgnu89-inline
#PLOS_CPPFLAGS += -std=c++11 -Wno-write-strings
PLOS_CPPFLAGS += -std=c++98 -Wno-write-strings -D_GLIBCXX_USE_CXX11_ABI=0
#
#
# WANRING
#	
PLOS_CFLAGS += -MMD -MP -Wfatal-errors -Wall -Wextra -Wnested-externs -Wmissing-prototypes \
			 -Wredundant-decls -Wcast-align -Wunreachable-code -Wshadow	\
			 -Wimplicit-function-declaration -Wimplicit	-Wreturn-type -Wunused \
			 -Wswitch -Wformat -Wuninitialized -Wchar-subscripts  \
			 -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
			 
PLOS_CPPFLAGS += -MMD -MP -Wfatal-errors -Wall -Wextra -Wnested-externs -Wmissing-prototypes \
			 -Wredundant-decls -Wcast-align -Wunreachable-code -Wshadow	\
			 -Wimplicit-function-declaration -Wimplicit	-Wreturn-type -Wunused \
			 -Wswitch -Wformat -Wuninitialized -Wchar-subscripts  \
			 -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
			 			 
# -Werror=implicit-function-declaration -Werror=switch
PLOS_CFLAGS += -Werror=return-type -Werror=format-extra-args  \
			  -Werror=unreachable-code -Werror=unused-function -Werror=unused-variable \
			  -Werror=unused-value -Werror=implicit-int -Werror=missing-parameter-type\
			  -Werror=parentheses -Werror=char-subscripts -Werror=parentheses  \
			  -Werror=invalid-memory-model -Werror=sizeof-pointer-memaccess \
			  -Werror=overflow  -Werror=format-security -Werror=missing-prototypes -Werror=shadow \
			  -Werror=unsafe-loop-optimizations -Werror=init-self 
			  #-Werror=stack-protector   
			  #-Werror=suggest-attribute=format -Werror=missing-format-attribute  -Werror=overlength-stringsl
			  #-Werror=sign-compare 有符号和无符号参数比较
			  #-Werror=format-overflow
			  #-Werror=shift-count-overflow
			  #-Werror=pointer-arith 
			  #sequence-point:违反顺序点的代码,比如 a[i] = c[i++];
			  #-Werror=cast-qual 
			  #-Werror=type-limits 参数类型限制
			  #-Werror=float-equal 对浮点数使用等号，这是不安全的
			  #-Werror=redundant-decls -Werror=format -Werror=missingbraces
#			 -Werror=switch-default -Werror=missing-format-attribute 
#				-Werror=overlength-strings -Werror=cast-align 
#PLOS_CPPFLAGS
#			 
PLOS_CFLAGS += -fmessage-length=0 -Wcast-align
#
PLOS_CFLAGS += -fsigned-char
ifeq ($(BUILD_DEBUG),YES)
PLOS_CFLAGS += -g2 -ggdb
else
PLOS_CFLAGS += -O1
endif

#
#PLOS_CFLAGS += -Werror
#
PLOS_DEFINE += -DBUILD_VERSION=\"$(VERSION)\" -DBUILD_TIME=\"$(BUILD_TIME)\"
#
#
export DSTBINDIR = $(BASE_ROOT)/$(BINDIR)
export DSTSBINDIR = $(BASE_ROOT)/$(SBINDIR)
export DSTETCDIR = $(BASE_ROOT)/$(ETCDIR)
export DSTLIBDIR = $(BASE_ROOT)/$(LIBDIR)
export DSTULIBDIR = $(BASE_ROOT)/$(ULIBDIR)
export DSTWWWDIR = $(BASE_ROOT)/$(WWWDIR)

#
#
#
#
#
#-include $(foreach prod,$(PLPRODS),$(prod)/gmake/$(firstword $(subst -,$(empty) $(empty),$(notdir $(prod)))).mk)
#
#
# gcc的 -MMD 选项可以自动生成带有依赖规则的.d文件，为创建头文件依赖带来了方便
#
#
#
export PLCFLAGS = $(PLOS_CFLAGS) $(PLEX_CFLAGS) $(PL_CFLAGS) $(PLM_CFLAGS)
#
export PLCPPFLAGS = $(PLOS_CPPFLAGS) $(PLEX_CPPFLAGS) $(PL_CPPFLAGS) $(PLM_CPPFLAGS)
#
export PLASFLAGS = $(PLOS_ASFLAGS) $(PLEX_ASFLAGS) $(PL_ASFLAGS) $(PLM_ASFLAGS)
#
export PLLDFLAGS = $(PLOS_LDFLAGS) $(PLEX_LDFLAGS) $(PL_LDFLAGS) $(PLM_LDFLAGS)
#
export PLLDLIBS = $(PLOS_LDLIBS) $(PLEX_LDLIBS) $(PL_LDLIBS) $(PLM_LDLIBS)
#
export PLINCLUDE = $(PLOS_INCLUDE) $(PLEX_INCLUDE) $(PL_INCLUDE) $(PLM_INCLUDE)
#
export PLDEFINE = $(PLOS_DEFINE) $(PLEX_DEFINE) $(PL_DEFINE) $(PLM_DEFINE)
#
export PLDEBUG = $(PL_DEBUG)
#
# 
export PLSTRIP_CFLAGS= --strip-unneeded
# 
#export PLCFLAGS += $(PLOS_CFLAGS) $(PLEX_CFLAGS) $(PL_CFLAGS) $(PL_DEBUG) -fPIC $(PLINCLUDE)
#export PLLDCLFLAG += $(PLOS_LDFLAGS) $(PLEX_LDFLAGS) $(PL_LDFLAGS) $(PLOS_LDLIBS) $(PLEX_LDLIBS) $(PL_LDLIBS) 
#
#
#
#
#
#PL_ECHO_CC = $(ECHO) CC '$(PLDEFINE) $(PLDEBUG) $(PLCFLAGS) $(PLLDFLAGS) $(PLINCLUDE)' $@
#PL_ECHO_CPP = $(ECHO) CXX '$(PLDEFINE) $(PLDEBUG) $(PLCPPFLAGS) $(PLLDFLAGS) $(PLINCLUDE)' $@
#PL_ECHO_AS = $(ECHO) AS '$(PLDEFINE) $(PLDEBUG) $(PLASFLAGS) $(PLLDFLAGS)' $@
#
#
#PL_OBJ_COMPILE = @$(CC) -fPIC $(PLDEFINE) $(PLDEBUG) $(PLCFLAGS) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
#PL_LIB_COMPILE = $(CC) -fPIC $(PLDEFINE) $(PLDEBUG) $(PLCFLAGS) $(PLLDFLAGS) $^ -o $@ $(PLINCLUDE)
#
#PL_MAKE_LIBSO = $(CC) -shared -o 
#
#PL_CXX_OBJ_COMPILE = @$(CXX) -fPIC $(PLDEFINE) $(PLDEBUG) $(PLCPPFLAGS) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
#PL_CXX_LIB_COMPILE = $(CXX) -fPIC $(PLDEFINE) $(PLDEBUG) $(PLCPPFLAGS) $(PLLDFLAGS) $^ -o $@ $(PLINCLUDE)
#
#PL_CXX_MAKE_LIBSO = $(CXX) -shared -o 
#
#PL_MAKE_LIB = @$(AR) -rs
#
#

#
#
#

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
include $(MAKE_DIR)/board.cfg
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
ifeq ($(BUILD_OPENWRT),true)
include $(MAKE_DIR)/openwrt.mk
PL_CFLAGS += -DBUILD_OPENWRT
else
include $(MAKE_DIR)/linux.mk
PL_CFLAGS += -DBUILD_LINUX
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
PLOS_CFLAGS += -s
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
ifeq ($(BUILD_DEBUG),YES)
PLOS_CFLAGS +=  -g
endif
#
#
ifeq ($(USE_IPCOM_STACK),true)
PL_CFLAGS += -DUSE_IPCOM_STACK
else
PL_CFLAGS += -DUSE_IPSTACK_KERNEL
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
PL_CFLAGS += -DBUILD_IPV6
endif
#
#
#
ifeq ($(ARCH_TYPE),MIPS)
PLOS_CFLAGS += -D__UCLIBC__
endif
#
#
PL_LDLIBS += -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s
#
PLOS_CFLAGS += -fsigned-char -O2  
#
# WANRING
#	
PLOS_CFLAGS += -MMD -MP -Wall -Wextra -Wnested-externs -Wmissing-prototypes \
			 -Wredundant-decls -Wcast-align -Wunreachable-code -Wshadow	\
			 -Wimplicit-function-declaration -Wimplicit	-Wreturn-type -Wunused \
			 -Wswitch -Wformat -Wuninitialized -Wchar-subscripts  \
			 -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
			 
			 
# -Werror=implicit-function-declaration -Werror=switch
PLOS_CFLAGS += -Werror=return-type -Werror=format-extra-args -Werror=missing-prototypes \
			  -Werror=unreachable-code -Werror=unused-function -Werror=unused-variable \
			  -Werror=unused-value -Werror=implicit-int \
			  -Werror=parentheses -Werror=shadow -Werror=char-subscripts
			  #-Werror=pointer-arith 
			  #-Werror=cast-qual 
			  #-Werror=redundant-decls -Werror=format -Werror=missingbraces
#			 -Werror=switch-default -Werror=missing-format-attribute 
#				-Werror=overlength-strings -Werror=cast-align  \
#			 
PLOS_CFLAGS += -fmessage-length=0 -Wcast-align
#PLOS_CFLAGS += -Werror
#
PL_CFLAGS += -DBUILD_VERSION=\"$(VERSION)\" -DBUILD_TIME=\"$(BUILD_TIME)\"
#
#
export DSTBINDIR = $(BASE_ROOT)/$(BINDIR)
export DSTSBINDIR = $(BASE_ROOT)/$(SBINDIR)
export DSTETCDIR = $(BASE_ROOT)/$(ETCDIR)
export DSTLIBDIR = $(BASE_ROOT)/$(LIBDIR)
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
PLINCLUDE += $(PLLIBINCLUDE)
#
PL_DEBUG += $(PLDEFINE) $(EXTRA_DEFINE) 
# -MMD -MP 
export CFLAGS += $(PLOS_CFLAGS) $(PL_CFLAGS) $(PL_DEBUG) -fPIC $(PLINCLUDE)
export LDCLFLAG += $(PL_LDLIBS) 
#
#
#
#
#
#$(OBJS_DIR)/%.d: $(SRC_DIR)/%.c
#	@set -e; \
#	rm -f $@; \
#	$(CC) -MM $(PLINCLUDE) $< > $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$
	
#-include $(OBJS:.o=.d)

	
#
#
#
# $(CC) $(OBJS) $(CFLAGS)
#%.o: %.c iw.h nl80211.h
#	@$(NQ) ' CC  $(CFLAGS)' $@
#	$(Q)$(CC) $(CFLAGS) -c -o $@ $<
#   $(CC) -MM $(CFLAGS) $< > $@.
	
PL_OBJ_COMPILE = @$(CC) -fPIC $(CFLAGS) $(LDCLFLAG) -c  $< -o $@ $(PLINCLUDE)
PL_LIB_COMPILE = $(CC) -fPIC $(CFLAGS) $(LDCLFLAG) $^ -o $@ $(PLINCLUDE)
#
PL_MAKE_LIBSO = $(CC) -shared -o 

PL_CXX_OBJ_COMPILE = @$(CXX) -fPIC $(CFLAGS) $(LDCLFLAG) -c  $< -o $@ $(PLINCLUDE)
PL_CXX_LIB_COMPILE = $(CXX) -fPIC $(CFLAGS) $(LDCLFLAG) $^ -o $@ $(PLINCLUDE)
#
PL_CXX_MAKE_LIBSO = $(CXX) -shared -o 

PL_MAKE_LIB = @$(AR) -rs
#
#

#
#
#

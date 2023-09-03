#
#
#SHELL :=C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe
SHELL := sh
#
export ZPLTOP_DIR =$(CURDIR)
#export ZPLTOP_DIR =$(shell pwd)
#export ZPLTOP_DIR=D:\source\SWPlatform\source
#
#
#
#
#prepare:
#	cd make;./version.sh $(TAGET)
#
#
#export GITVERSION=$(shell cd make;./version.sh)
#
#GITVER_FILE=include/gitversion.h
#include/gitversion.h: 
#$(GITVER_FILE):
#	@/bin/sh $(ZPLTOP_DIR)/make/version.sh
#	#/usr/bin/perl $(SRC_DIR)/route_types.pl < $(SRC_DIR)/route_types.txt > $@
#
#
#
#
#
#
include make/makelinux.mk
#
#
#
#
#
TAGET=SWP-$(ZPLVER)
#
#
#PLOS_MAP = -Wl,-Map,target-app.map
ifneq ($(ZPLOS_MAP),)
TAGETMAP=SWP-$(ZPLVER).map
ZPLOS_MAP = -Wl,-Map=$(TAGETMAP)
endif
#
#
#

	
#
#
#
LIBS1 = $(shell $(CD) $(BASE_ROOT)/$(ZPL_LIB_DIR)/ && ls *.a)
LIBS2 = $(subst .a,,$(LIBS1))
LIBC += $(subst lib,-l,$(LIBS2))
ifneq ($(ZPL_BUILD_ARCH),X86)
LIBSO1 = $(shell $(CD) $(BASE_ROOT)/$(ZPL_LIB_DIR)/ && ls *.so)
LIBSO2 = $(subst .so,,$(LIBSO1))
LIBC += $(subst lib,-l,$(LIBSO2))
endif
#
#
#ZPL_LDCLFLAG=$(ZPLOS_LDLIBS) $(ZPLEX_LDLIBS) $(ZPL_LDLIBS) 
ZPLLSLIBS += $(LIBC)
ZPLLDSOLIBS = $(ZPLLDLIBS)
#
#ZPLINCLUDE += -I$(BASE_ROOT)/include
#
#ZPLINCLUDE += $(IPSTACK_INCLUDE)
#IPSTACK_LIBDIR
#
ifeq ($(ZPL_IPCOM_MODULE),true)
IPLIBS1 = $(shell $(CD) $(IPSTACK_LIBDIR)/ && ls *.a)
IPLIBS2 = $(subst .a,,$(IPLIBS1))
IPLIBC += $(subst lib,-l,$(IPLIBS2))

ZPLLDFLAGS += -L$(IPSTACK_LIBDIR)
endif
#
#
#
#ULIBSOFILE = $(shell $(CD) $(BASE_ROOT)/$(ZPL_ULIB_DIR)/ && ls *.so)
#ULIBSOFILE += $(shell $(CD) $(BASE_ROOT)/$(ZPL_ULIB_DIR)/ && ls *.so*)
#
#
#
ZPLLDLIBS += $(IPLIBC)
#ZPLLDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
			 -lpjsua -lsrtp -lgsmcodec -lspeex -lilbccodec -lg7221codec \
			 -lwebrtc -lyuv -lv4l2 -lopenh264 -lresample
#
#
#export ZPLLDCLFLAG += -L$(BASE_ROOT)/$(ZPL_LIB_DIR)/  
#$(ZPLDEFINE) $(ZPLINCLUDE) $(ZPL_DEBUG) -g #-lcrypto
# $(ZPL_CFLAGS)
#
%.o: %.c 
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)
#	@$(ZPL_ECHO_CC)
#	@$(ZPL_LIB_COMPILE)
#	
#@$(CC) -fPIC $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCFLAGS) $(ZPLLDSOLIBS) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)


SOURCES = $(wildcard *.c *.cpp)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
#
#
-include $(OBJS:.o=.d)
#
#
#				
#
#
#	
.PHONY:	obj objclean all lib clean exlib exlib_clean install rebuild dist  app target usage help config_install prebuilts
#
# awk -F= '/^NAME/{print $2}' /etc/os-release
# ubuntu 系统要把链接库放在行的末尾处
#
#
ifeq ($(ZPL_BUILD_DEBUG),YES)
target : $(OBJS) $(BASE_ROOT)/$(ZPL_LIB_DIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) -Xlinker "-(" $(ZPLLSLIBS) -Xlinker "-)" $(ZPLLDFLAGS) $(ZPLLDSOLIBS)
	$(CHMOD) a+x $(TAGET)
	$(RM) -rf ./*.o ./*.d
else
target : $(OBJS) $(BASE_ROOT)/$(ZPL_LIB_DIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) -Xlinker "-(" $(ZPLLSLIBS) -Xlinker "-)" $(ZPLOS_MAP) $(ZPLLDFLAGS) $(ZPLLDSOLIBS)
	$(CHMOD) a+x $(TAGET)
	$(STRIP) $(TAGET)
	$(RM) -rf ./*.o ./*.d
endif

	
#	
#
#
#
#
#
help:usage
#
#
usage:
	@$(ECHO) ""
	@$(ECHO) "Targets:"
	@$(ECHO) "  obj        build objects"
	@$(ECHO) "  clean      remove all"
	@$(ECHO) "  objclean   remove all objects"
	@$(ECHO) "  install    build all and install"
	@$(ECHO) "  all        build all module"
	@$(ECHO) "  lib        same as install"
	@$(ECHO) "  rebuild    clean and install"
	@$(ECHO) "  demo       build demo app"
	@$(ECHO) "  app        build demo app"
	@$(ECHO) "  usage      make usage"
	@$(ECHO) "  help       make help"	
	@$(ECHO) ""
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
#
#
#
make_prepare:
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
#
objclean:  
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
	@if test -f os_main.o ; \
		then \
		$(RM) os_main.o; \
	fi
	@if test -f os_main.d ; \
		then \
		$(RM) os_main.d; \
	fi
	@if test -f $(TAGET) ; \
		then \
		$(RM) $(TAGET); \
	fi

clean: 
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
	@if test -f os_main.o ; \
		then \
		$(RM) os_main.o; \
	fi
	@if test -f os_main.d ; \
		then \
		$(RM) os_main.d; \
	fi	
	@if test -f $(TAGET) ; \
		then \
		$(RM) $(TAGET); \
	fi
	@if test -f $(TAGETMAP) ; \
		then \
		$(RM) $(TAGETMAP); \
	fi
	
obj: 
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 

exlib:
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@

exlib_clean:
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@

#install: obj
install: 
	$(INSTALL) -d ${ZPL_BIN_DIR}
	$(INSTALL) -m 755 ${TAGET} ${ZPL_BIN_DIR}
	
#all: install $(TAGET)
all: 
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
#lib: install
lib:
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
	
kernel_module:
	${MAKE} -C  $(ZPLTOP_DIR)/make/ $@ 
	
rebuild: clean all $(TAGET)

app: all
	@if test -f os_main.o ; \
		then \
		$(RM) os_main.o; \
	fi
	@if test -f os_main.d ; \
		then \
		$(RM) os_main.d; \
	fi	
	@if test -f $(TAGET) ; \
		then \
		$(RM) $(TAGET); \
	fi
	${MAKE} target
#
#
dist: all
	@$(ECHO) 'packet...'
	@$(CD) make; ./packet.sh $(TAGET)	
	
	

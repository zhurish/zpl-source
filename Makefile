#
#
#SHELL :=C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe
SHELL := sh
#
export TOP_DIR =$(CURDIR)
#export TOP_DIR =$(shell pwd)
#export TOP_DIR=D:\source\SWPlatform\source
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
#	@/bin/sh $(TOP_DIR)/make/version.sh
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
ZPLOS_MAP = -Wl,-Map,$(TAGETMAP)
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
ifeq ($(ZPL_IPCOM_STACK_MODULE),true)
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
	$(ZPL_ECHO_CC)
	@$(CC) -fPIC -D__ARM_PCS_VFP $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLCFLAGS) -g $(ZPLLDSOLIBS) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)


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
.PHONY:	obj objclean clean install all lib rebuild dist demo app target usage help config_install prebuilts
#
# awk -F= '/^NAME/{print $2}' /etc/os-release
# ubuntu 系统要把链接库放在行的末尾处
#
#
ifeq ($(ZPL_BUILD_DEBUG),YES)
target : $(OBJS) $(BASE_ROOT)/$(ZPL_LIB_DIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) -Xlinker "-(" $(ZPLLSLIBS) -Xlinker "-)" $(ZPLOS_MAP) $(ZPLLDFLAGS) $(ZPLLDSOLIBS)
	$(CHMOD) a+x $(TAGET)
else
target : $(OBJS) $(BASE_ROOT)/$(ZPL_LIB_DIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) -Xlinker "-(" $(ZPLLSLIBS) -Xlinker "-)" $(ZPLOS_MAP) $(ZPLLDFLAGS) $(ZPLLDSOLIBS)
	$(CHMOD) a+x $(TAGET)
	$(STRIP) $(TAGET)
endif

	
#	
#
ifneq ($(APP_BIN_DIR),)
app_bin_install:
	$(INSTALL) -d $(APP_BIN_DIR)
	$(INSTALL) -m 755 debug/bin/*bin* $(APP_BIN_DIR)
	$(INSTALL) -m 755 debug/sbin/* $(APP_BIN_DIR)
else
app_bin_install:
	@$(ECHO) " install -m 755 bin "
endif
#
ifneq ($(APP_ETC_DIR),)
app_etc_install:
	$(INSTALL) -d $(APP_ETC_DIR)
	$(INSTALL) -m 755 debug/etc/* $(APP_ETC_DIR)
else
app_etc_install:
	@$(ECHO) " install -m 755 etc "
endif
#
ifneq ($(APP_WEB_DIR),)
app_web_install:
	$(INSTALL) -d $(APP_WEB_DIR)
	$(INSTALL) -m 755 debug/etc/web/* $(APP_WEB_DIR)
else
app_web_install:
	@$(ECHO) " install -m 755 etc/web "
endif
#
ifneq ($(APP_WWW_DIR),)
app_www_install:
	$(INSTALL) -d $(APP_WWW_DIR)
	$(INSTALL) -m 755 debug/www/* $(APP_WWW_DIR)
else
app_www_install:
	@$(ECHO) " install -m 755 www "
endif
#
ifneq ($(APP_LIB_DIR),)
app_lib_install:
	$(INSTALL) -d $(APP_LIB_DIR)
	$(INSTALL) -m 755 debug/usr/lib/*so* $(APP_LIB_DIR)
else
app_lib_install:
	@$(ECHO) " install -m 755 lib "
endif
#
#
#
#
ifeq ($(BUILD_OPENWRT),true)
openwrt_install:
	$(INSTALL) -d ${ZPL_INSTALL_ETC_DIR} 
	cd make;./setup.sh $(TAGET)
else
openwrt_install:
	$(INSTALL) -d ${ZPL_INSTALL_ETC_DIR} 
endif
#
#
config_install: openwrt_install 
	$(INSTALL) -d ${ZPL_INSTALL_ETC_DIR} 
	$(INSTALL) -m 755 make/start-boot.sh ${ZPL_INSTALL_ETC_DIR}  	
	$(INSTALL) -m 755 startup/etc/plat.conf ${ZPL_INSTALL_ETC_DIR}
	$(INSTALL) -m 755 startup/etc/openconfig ${ZPL_INSTALL_ETC_DIR}
	$(INSTALL) -m 755 startup/etc/product ${ZPL_INSTALL_ETC_DIR}
	$(INSTALL) -m 755 startup/etc/voipconfig ${ZPL_INSTALL_ETC_DIR}
#	
#	$(INSTALL) -d ${ZPL_BIN_DIR}
#	$(INSTALL) -m 755 ${TAGET} ${ZPL_BIN_DIR}
#	$(STRIP) $(ZPL_BIN_DIR)/$(TAGET) 
#	$(INSTALL) -d ${ZPL_INSTALL_ETC_DIR} 
#	cd make;./setup.sh $(TAGET)
#	$(INSTALL) -m 755 make/start-boot.sh ${ZPL_INSTALL_ETC_DIR}  	
#	$(INSTALL) -m 755 startup/etc/plat.conf ${ZPL_INSTALL_ETC_DIR}
	#$(INSTALL) -m 755 startup/etc/default-config.cfg ${ZPL_INSTALL_ETC_DIR}
	#$(CP) $(TAGET) /home/zhurish/Downloads/tftpboot/
#
#
#
app_all_install: app_bin_install app_etc_install app_web_install app_www_install app_lib_install  make_prepare

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
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
#
#
#
make_prepare:
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
#
objclean:  
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
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
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
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
	${MAKE} -C  $(TOP_DIR)/make/ $@ 

#install: obj
install: config_install
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
	$(INSTALL) -d ${ZPL_BIN_DIR}
	$(INSTALL) -m 755 ${TAGET} ${ZPL_BIN_DIR}
	#$(STRIP) $(ZPL_BIN_DIR)/$(TAGET) 
	
	#install -d ${ZPL_INSTALL_ULIB_DIR}
	#$(CP) $(ULIBSOFILE) ${ZPL_INSTALL_ULIB_DIR}
	
	#install -d ${ZPL_INSTALL_ETC_DIR}
	#cd make;./setup.sh $(TAGET)
	#install -m 755 make/start-boot.sh ${ZPL_INSTALL_ETC_DIR}  	
	#install -m 755 startup/etc/plat.conf ${ZPL_INSTALL_ETC_DIR}
	#install -m 755 startup/etc/default-config.cfg ${ZPL_INSTALL_ETC_DIR}
	#$(CP) $(TAGET) /home/zhurish/Downloads/tftpboot/
	
#all: install $(TAGET)
all: 
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
#lib: install
lib:
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
	
rebuild: clean all $(TAGET)

app: demo

demo: all
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
	
	

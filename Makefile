#
#
#
export TOP_DIR =$(shell pwd)
#TOP_DIR=/home/zhurish/workspace/SWPlatform
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
	#/usr/bin/perl $(SRC_DIR)/route_types.pl < $(SRC_DIR)/route_types.txt > $@
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
TAGET=SWP-$(PLVER)
#
#
#PLOS_MAP = -Wl,-Map,target-app.map
ifneq ($(PLOS_MAP),)
TAGETMAP=SWP-$(PLVER).map
PLOS_MAP = -Wl,-Map,$(TAGETMAP)
endif
#
#
#
#
#
#
LIBS1 = $(shell $(CD) $(BASE_ROOT)/$(LIBDIR)/ && ls *.a)
LIBS2 = $(subst .a,,$(LIBS1))
LIBC += $(subst lib,-l,$(LIBS2))
ifneq ($(BUILD_TYPE),X86)
LIBSO1 = $(shell $(CD) $(BASE_ROOT)/$(LIBDIR)/ && ls *.so)
LIBSO2 = $(subst .so,,$(LIBSO1))
LIBC += $(subst lib,-l,$(LIBSO2))
endif
#
#
#PL_LDCLFLAG=$(PLOS_LDLIBS) $(PLEX_LDLIBS) $(PL_LDLIBS) 
PLLDLIBS += $(LIBC)
#
#PLINCLUDE += -I$(BASE_ROOT)/include
#
#PLINCLUDE += $(IPSTACK_INCLUDE)
#IPSTACK_LIBDIR
#
ifeq ($(USE_IPCOM_STACK),true)
IPLIBS1 = $(shell $(CD) $(IPSTACK_LIBDIR)/ && ls *.a)
IPLIBS2 = $(subst .a,,$(IPLIBS1))
IPLIBC += $(subst lib,-l,$(IPLIBS2))

PLLDFLAGS += -L$(IPSTACK_LIBDIR)
endif
#
#
#
#ULIBSOFILE = $(shell $(CD) $(BASE_ROOT)/$(ULIBDIR)/ && ls *.so)
#ULIBSOFILE += $(shell $(CD) $(BASE_ROOT)/$(ULIBDIR)/ && ls *.so*)
#
#
#
PLLDLIBS += $(IPLIBC)
#
#
#export PLLDCLFLAG += -L$(BASE_ROOT)/$(LIBDIR)/  
#$(PLDEFINE) $(PLINCLUDE) $(PL_DEBUG) -g #-lcrypto
# $(PL_CFLAGS)
#
%.o: %.c
	$(PL_OBJ_COMPILE)


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
.PHONY:	obj objclean clean install all lib rebuild dist demo app target usage help config_install
#
#
ifeq ($(BUILD_DEBUG),YES)
target : $(OBJS) $(BASE_ROOT)/$(LIBDIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) $(PLDEFINE) $(PLCFLAGS) $(PLLDFLAGS) $(PLINCLUDE) -Xlinker "-(" $(PLLDLIBS) -Xlinker "-)" $(PLOS_MAP)
	$(CHMOD) a+x $(TAGET)
	$(STRIP) --strip-unneeded $(TAGET)
else
target : $(OBJS) $(BASE_ROOT)/$(LIBDIR)/*.a 
	$(CC) -o $(TAGET) $(OBJS) $(PLDEFINE) $(PLCFLAGS) $(PLLDFLAGS) $(PLINCLUDE) -Xlinker "-(" $(PLLDLIBS) -Xlinker "-)" $(PLOS_MAP)
	$(CHMOD) a+x $(TAGET)
	$(STRIP) $(TAGET)
endif

	
#	
#
ifneq ($(APP_BIN_DIR),)
app_bin_install:
	install -d $(APP_BIN_DIR)
	install -m 755 debug/bin/*bin* $(APP_BIN_DIR)
	install -m 755 debug/sbin/* $(APP_BIN_DIR)
else
app_bin_install:
	@$(ECHO) " install -m 755 bin "
endif
#
ifneq ($(APP_ETC_DIR),)
app_etc_install:
	install -d $(APP_ETC_DIR)
	install -m 755 debug/etc/* $(APP_ETC_DIR)
else
app_etc_install:
	@$(ECHO) " install -m 755 etc "
endif
#
ifneq ($(APP_WEB_DIR),)
app_web_install:
	install -d $(APP_WEB_DIR)
	install -m 755 debug/etc/web/* $(APP_WEB_DIR)
else
app_web_install:
	@$(ECHO) " install -m 755 etc/web "
endif
#
ifneq ($(APP_WWW_DIR),)
app_www_install:
	install -d $(APP_WWW_DIR)
	install -m 755 debug/www/* $(APP_WWW_DIR)
else
app_www_install:
	@$(ECHO) " install -m 755 www "
endif
#
ifneq ($(APP_LIB_DIR),)
app_lib_install:
	install -d $(APP_LIB_DIR)
	install -m 755 debug/usr/lib/*so* $(APP_LIB_DIR)
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
	install -d ${DSTETCDIR} 
	cd make;./setup.sh $(TAGET)
else
openwrt_install:
	install -d ${DSTETCDIR} 
endif
#
#
config_install: openwrt_install 
	install -d ${DSTETCDIR} 
	install -m 755 make/start-boot.sh ${DSTETCDIR}  	
	install -m 755 startup/etc/plat.conf ${DSTETCDIR}
	install -m 755 startup/etc/openconfig ${DSTETCDIR}
	install -m 755 startup/etc/product ${DSTETCDIR}
	install -m 755 startup/etc/voipconfig ${DSTETCDIR}
#	
#	install -d ${BINDIR}
#	install -m 755 ${TAGET} ${BINDIR}
#	$(STRIP) $(BINDIR)/$(TAGET) 
#	install -d ${DSTETCDIR} 
#	cd make;./setup.sh $(TAGET)
#	install -m 755 make/start-boot.sh ${DSTETCDIR}  	
#	install -m 755 startup/etc/plat.conf ${DSTETCDIR}
	#install -m 755 startup/etc/default-config.cfg ${DSTETCDIR}
	#$(CP) $(TAGET) /home/zhurish/Downloads/tftpboot/
#
#
#
app_all_install: app_bin_install app_etc_install app_web_install app_www_install app_lib_install 

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
	install -d ${BINDIR}
	install -m 755 ${TAGET} ${BINDIR}
	#$(STRIP) $(BINDIR)/$(TAGET) 
	
	#install -d ${DSTULIBDIR}
	#$(CP) $(ULIBSOFILE) ${DSTULIBDIR}
	
	#install -d ${DSTETCDIR}
	#cd make;./setup.sh $(TAGET)
	#install -m 755 make/start-boot.sh ${DSTETCDIR}  	
	#install -m 755 startup/etc/plat.conf ${DSTETCDIR}
	#install -m 755 startup/etc/default-config.cfg ${DSTETCDIR}
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
	
	

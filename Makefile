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
LDCLFLAG += $(LIBC)
#
PLINCLUDE += -I$(BASE_ROOT)/include
#
#PLINCLUDE += $(IPSTACK_INCLUDE)
#IPSTACK_LIBDIR
#
ifeq ($(USE_IPCOM_STACK),true)
IPLIBS1 = $(shell $(CD) $(IPSTACK_LIBDIR)/ && ls *.a)
IPLIBS2 = $(subst .a,,$(IPLIBS1))
LIBC += $(subst lib,-l,$(IPLIBS2))

CFLAGS += -L$(IPSTACK_LIBDIR)
endif
#
#LDCLFLAG += -lssl
#
#
export CFLAGS += -L$(BASE_ROOT)/$(LIBDIR)/  
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
$(TAGET) : $(OBJS) $(BASE_ROOT)/$(LIBDIR)/*.a 
	$(CC) $(OBJS) $(CFLAGS) -Xlinker "-(" $(LDCLFLAG) -Xlinker "-)" -o $(TAGET) 
	$(CHMOD) a+x $(TAGET)
	install -d ${BINDIR}
	install -m 755 ${TAGET} ${BINDIR}
	$(STRIP) $(BINDIR)/$(TAGET) 
	install -d ${DSTETCDIR} 
	cd make;./setup.sh $(TAGET)
	install -m 755 make/start-boot.sh ${DSTETCDIR}  	
	install -m 755 startup/etc/plat.conf ${DSTETCDIR}
	#install -m 755 startup/etc/default-config.cfg ${DSTETCDIR}
	#$(CP) $(TAGET) /home/zhurish/Downloads/tftpboot/
#
#
#	
.PHONY:	obj objclean clean install all lib rebuild dist demo app usage help
#
help:usage
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
	
obj:
	${MAKE} -C  $(TOP_DIR)/make/ $@ 

#install: obj
install: 
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
	
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
	${MAKE}
#
#
dist: all
	@$(ECHO) 'packet...'
	@$(CD) make; ./packet.sh $(TAGET)	
	
	

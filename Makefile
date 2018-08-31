#
#
#
#TOP_DIR =$(PWD)
TOP_DIR=/home/zhurish/workspace/SWPlatform
#
#
#
export MAKE_DIR
export BASE_ROOT
#
#
#
include make/makelinux.mk
#
#
#
#
#
#
#
TAGET=SWP-V$(PLVER)
#
#
LIBS1 = $(shell $(CD) $(BASE_ROOT)/$(LIBDIR)/ && ls *.a)
LIBS2 = $(subst .a,,$(LIBS1))
LIBC += $(subst lib,-l,$(LIBS2))
ifeq ($(BUILD_TYPE),ARM)
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
#
#
export CFLAGS += -L$(BASE_ROOT)/$(LIBDIR)/ $(PLDEFINE) $(PLINCLUDE) $(PL_DEBUG) -g #-lcrypto
#
#
%.o: %.c
#	@$(CC) $(CFLAGS) $(LDCLFLAG)  $< -o $@ $(PLINCLUDE)
	$(PL_OBJ_COMPILE)


SOURCES = $(wildcard *.c *.cpp)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
#
#
$(TAGET) : $(OBJS) $(BASE_ROOT)/$(LIBDIR)/*.a 
	$(CC) $(OBJS) $(CFLAGS) -Xlinker "-(" $(LDCLFLAG) -Xlinker "-)" -o $(TAGET) 
#	$(CC) $(OBJS) $(CFLAGS) -Xlinker "-(" $(LDCLFLAG) -Xlinker "-)" -o $(TAGET) 
#	$(STRIP) $(TAGET)
	$(CHMOD) a+x $(TAGET)
#
#
#	
.PHONY:	obj objclean clean install all lib rebuild dist demo usage help
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
	@$(ECHO) "  all        build all"
	@$(ECHO) "  lib        same as install"
	@$(ECHO) "  rebuild    clean and install"
	@$(ECHO) "  demo       build demo app"
	@$(ECHO) "  usage      make usage"
	@$(ECHO) "  help       make help"	
	@$(ECHO) ""
#
#
#
objclean:  
	${MAKE} -C  $(TOP_DIR)/make/ $@ 

clean: 
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
	
obj:
	${MAKE} -C  $(TOP_DIR)/make/ $@ 

install: obj
	${MAKE} -C  $(TOP_DIR)/make/ $@ 
	
all: install $(TAGET)
	
lib: install
		
	
rebuild: clean all

demo: all

#OBJDIR = debug/obj
#LIBDIR = debug/lib
#BINDIR = debug/bin
#SBINDIR = debug/sbin
#ETCDIR = debug/etc
dist: all
	@$(ECHO) 'packet...'
	@./tools/build.sh $(ARCH_TYPE) $(TAGET)
	#@$(CD) $(RELEASEDIR) && $(TAR) -zcvf $(TAGET).tar.gz --exclude=$(LIBDIR)/*.a $(LIBDIR) $(ETCDIR) $(SBINDIR) $(BINDIR) && $(MV) $(TAGET).tar.gz ../$(TAGET).tar.gz

#	${MAKE}	
	
	
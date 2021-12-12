include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

ZPL_PJINC = -I$(ZPL_INSTALL_ROOTFS_DIR)/include

#PJPROJDIR=$(ZPLBASE)/externsions/pjproject-2.10
#PJCC=$(CC)
#PJCXX=$(CXX)
#$(PJLIBS): 
#	cd $(PJPROJDIR); ./configure --prefix=$(PJPROJDIR)/_install --target=mipsel-openwrt-linux CC=$(PJCC) \
#		CXX=$(PJCXX) 
#		
#	make -C $(PJPROJDIR)
	
	
OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)

ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
MODULE_PJSUA2_OBJS  := $(addprefix $(OBJS_DIR)/,$(PJSUA2OBJ))
PJSUA2TEST_OBJS  := $(addprefix $(OBJS_DIR)/,$(PJSUA2TEST))

MODULE_2OBJS_DEP:=$(MODULE_PJSUA2_OBJS:%.o=%.d)
MODULE_2OBJS_DEP +=$(PJSUA2TEST_OBJS:%.o=%.d)
-include $(MODULE_2OBJS_DEP)

endif

ZPLCFLAGS +=-DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
ZPLCPPFLAGS += -g -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 
#ZPLCFLAGS += -Werror=format
#
#CFLAGS += -UHAVE_CONFIG_H
# It's a kind of magic...
#
#ZPL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
#			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
#				-lpjsua -lsrtp


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c 
#ifeq ($(strip $(ZPL_PJPROJECT_MODULE)),true)
#	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl lib
#	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl install
#endif	
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE) $(ZPL_PJINC)

#
#
#
ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
$(OBJS_DIR)/%.o: $(SRC_DIR)/pjsua2/%.cpp 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CXX '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CXX) -fPIC $(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -c  $< -o $@ $(ZPLINCLUDE) $(ZPL_PJINC)

#
$(OBJS_DIR)/%.o: $(SRC_DIR)/pjsua2/%.c 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -c  $< -o $@ $(ZPLINCLUDE) $(ZPL_PJINC)
endif
#	
#
ifeq ($(PJMEDIA_SDL2_ENABLE),true)
SDL2LIBS:
	$(MAKE) -C $(SDL2PROJDIR) -f Makefile-pl all

SDL2CLEAN:
	$(MAKE) -C $(SDL2PROJDIR) -f Makefile-pl clean	
else
SDL2LIBS:
	$(ECHO) NO SDL2

SDL2CLEAN:
	$(ECHO) NO SDL2
endif


ifeq ($(strip $(ZPL_PJPROJECT_MODULE)),0)
ZPL_PJPROJECT_LIBS:
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl lib
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl install

ZPL_PJPROJECT_UNLIBS:
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl uninstall
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl clean

else
ZPL_PJPROJECT_LIBS:
	@$(ECHO) " "

ZPL_PJPROJECT_UNLIBS:
	@$(ECHO) " "
endif

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

$(LIBS) : $(ZPL_PJPROJECT_LIBS) $(SDL2LIBS) $(MODULE_OBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 

ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
$(PJSUA2LIBS) : $(MODULE_PJSUA2_OBJS)
	#$(CXX) -shared -o  $(PJSUA2LIBS) $(MODULE_PJSUA2_OBJS)
	@$(AR) -rs $(PJSUA2LIBS) $(MODULE_PJSUA2_OBJS)

PJPLLDCLFLAG += -L$(BASE_ROOT)/$(ZPL_ULIB_DIR) -L$(LIB_DIR) -L$(ZPL_INSTALL_ROOTFS_DIR)/lib


$(PJSUA2TEST_BIN) : $(PJSUA2TEST_OBJS) 
	$(CXX) $(PJSUA2TEST_OBJS) $(ZPLCPPFLAGS) $(PJPLLDCLFLAG) $(ZPLLDLIBS) -Xlinker "-(" -ljsoncpp -lpjsipc++ -Xlinker "-)" -o $(PJSUA2TEST_BIN) 
endif
#
#
-include $(MODULE_OBJS:.o=.d)
ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
-include $(MODULE_PJSUA2_OBJS:.o=.d)
-include $(PJSUA2TEST_OBJS:.o=.d)
endif
#
#
#
ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
lib: $(LIBS) $(PJSUA2LIBS)

obj: $(MODULE_OBJS) $(MODULE_PJSUA2_OBJS) $(PJSUA2TEST_OBJS) 

install-lib:
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	@install -m 755 $(PJSUA2LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(PJSUA2LIBS)
	 
install: install-lib 
#	@install -m 755 ${PJSUA2TEST_BIN} ${ZPL_INSTALL_SBIN_DIR}

clean: $(SDL2CLEAN) objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)
	@$(RM) $(LIB_DIR)/$(PJSUA2LIBS)
	@$(RM) $(PJSUA2LIBS)
else
lib: $(LIBS)

obj: $(MODULE_OBJS)

install-lib:
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	 
install: install-lib

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

endif

objclean: $(ZPL_PJPROJECT_LIBS) 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean install-lib make_prepare

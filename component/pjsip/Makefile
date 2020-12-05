include $(MAKE_DIR)/makelinux.mk

include config.mk

PL_PJINC = -I$(DSTROOTFSDIR)/include

#PJPROJDIR=$(PLBASE)/externsions/pjproject-2.10
#PJCC=$(CC)
#PJCXX=$(CXX)
#$(PJLIBS): 
#	cd $(PJPROJDIR); ./configure --prefix=$(PJPROJDIR)/_install --target=mipsel-openwrt-linux CC=$(PJCC) \
#		CXX=$(PJCXX) 
#		
#	make -C $(PJPROJDIR)
	
	
OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

ifeq ($(strip $(PL_PJSIP_PJSUA2)),true)
MODULE_PJSUA2_OBJS  := $(addprefix $(OBJS_DIR)/,$(PJSUA2OBJ))
PJSUA2TEST_OBJS  := $(addprefix $(OBJS_DIR)/,$(PJSUA2TEST))
endif

PLCFLAGS +=-DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
PLCPPFLAGS += -g -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 
#PLCFLAGS += -Werror=format
#
#CFLAGS += -UHAVE_CONFIG_H
# It's a kind of magic...
#
#PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
#			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
#				-lpjsua -lsrtp


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c 
#ifeq ($(strip $(PL_PJPROJECT_MODULE)),true)
#	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl lib
#	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl install
#endif	
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE) $(PL_PJINC)

#
#
#
ifeq ($(strip $(PL_PJSIP_PJSUA2)),true)
$(OBJS_DIR)/%.o: $(SRC_DIR)/pjsua2/%.cpp 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CXX '$(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLINCLUDE)' $@
	@$(CXX) -fPIC $(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) -c  $< -o $@ $(PLINCLUDE) $(PL_PJINC)

#
$(OBJS_DIR)/%.o: $(SRC_DIR)/pjsua2/%.c 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) -c  $< -o $@ $(PLINCLUDE) $(PL_PJINC)
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


ifeq ($(strip $(PL_PJPROJECT_MODULE)),0)
PL_PJPROJECT_LIBS:
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl lib
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl install

PL_PJPROJECT_UNLIBS:
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl uninstall
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl clean

else
PL_PJPROJECT_LIBS:
	@$(ECHO) " "

PL_PJPROJECT_UNLIBS:
	@$(ECHO) " "
endif



$(LIBS) : $(PL_PJPROJECT_LIBS) $(SDL2LIBS) $(MODULE_OBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 

ifeq ($(strip $(PL_PJSIP_PJSUA2)),true)
$(PJSUA2LIBS) : $(MODULE_PJSUA2_OBJS)
	#$(CXX) -shared -o  $(PJSUA2LIBS) $(MODULE_PJSUA2_OBJS)
	@$(AR) -rs $(PJSUA2LIBS) $(MODULE_PJSUA2_OBJS)

PJPLLDCLFLAG += -L$(BASE_ROOT)/$(ULIBDIR) -L$(LIB_DIR) -L$(DSTROOTFSDIR)/lib


$(PJSUA2TEST_BIN) : $(PJSUA2TEST_OBJS) 
	$(CXX) $(PJSUA2TEST_OBJS) $(PLCPPFLAGS) $(PJPLLDCLFLAG) $(PLLDLIBS) -Xlinker "-(" -ljsoncpp -lpjsipc++ -Xlinker "-)" -o $(PJSUA2TEST_BIN) 
endif
#
#
-include $(MODULE_OBJS:.o=.d)
ifeq ($(strip $(PL_PJSIP_PJSUA2)),true)
-include $(MODULE_PJSUA2_OBJS:.o=.d)
-include $(PJSUA2TEST_OBJS:.o=.d)
endif
#
#
#
ifeq ($(strip $(PL_PJSIP_PJSUA2)),true)
lib: $(LIBS) $(PJSUA2LIBS)

obj: $(MODULE_OBJS) $(MODULE_PJSUA2_OBJS) $(PJSUA2TEST_OBJS) 

install-lib:
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	@install -m 755 $(PJSUA2LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(PJSUA2LIBS)
	 
install: install-lib $(PJSUA2TEST_BIN)
	@install -m 755 ${PJSUA2TEST_BIN} ${DSTSBINDIR}

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
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	 
install: install-lib

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

endif

objclean: $(PL_PJPROJECT_LIBS) 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean install-lib

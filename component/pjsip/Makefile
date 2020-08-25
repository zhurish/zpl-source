include $(MAKE_DIR)/makelinux.mk

include config.mk

PL_PJINC = -I$(PJPROJDIR)/_install/include

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

ifneq ($(TOBJS),)
TARGET = auddemo
TMODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(TOBJS))
endif
PLCFLAGS +=-DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
#PLCFLAGS += -Werror=format
#
#CFLAGS += -UHAVE_CONFIG_H
# It's a kind of magic...
#
#PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
#			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
#				-lpjsua -lsrtp
					
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c 
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl lib
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl install
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE) $(PL_PJINC)
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


$(LIBS) : $(SDL2LIBS) $(MODULE_OBJS) 
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 dep
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 lib
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 install
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 

ifneq ($(TOBJS),)
LDCLFLAG = $(PLOS_LDLIBS) -L$(DSTULIBDIR) $(PL_LDLIBS) 

$(TARGET): $(TMODULE_OBJS)
	$(CC) $(TMODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 
endif		


#lib: $(LIBS)
#
#
-include $(MODULE_OBJS:.o=.d)
#
#
#$(LIBS) : $(MODULE_OBJS)
#	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)
#
lib: $(LIBS)


ifneq ($(TOBJS),)
obj: $(MODULE_OBJS) $(MODULE_LIBOBJS) $(TMODULE_OBJS)

install: $(TARGET)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
#	$(STRIP) $(LIB_DIR)/$(LIBS)
#	install -m 755 lib*.so $(LIB_DIR)
#	install -m 755 lib*.so.* $(LIB_DIR)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${TARGET} ${DSTSBINDIR} 
	#$(STRIP) ${DSTSBINDIR}/${TARGET}
else
obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

install: 
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
#	$(STRIP) $(LIB_DIR)/$(LIBS)
#	install -m 755 lib*.so $(LIB_DIR)
#	install -m 755 lib*.so.* $(LIB_DIR)	
endif

clean: $(SDL2CLEAN) objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl uninstall
	$(MAKE) -C $(PJPROJDIR) -f Makefile-pl clean
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean

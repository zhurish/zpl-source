include $(MAKE_DIR)/makelinux.mk

include config.mk

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
ifeq ($(WITH_SHARED_LIBRARIES),yes)
LIB_DIR = $(BASE_ROOT)/$(ULIBDIR)
endif
ifeq ($(WITH_STATIC_LIBRARIES),yes)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)
endif

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(MOSQ_OBJS))

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
#
#
APPOBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEAPPDIR)
APPSRC_DIR = $(BASE_ROOT)/$(MODULEAPPDIR)
#APPLIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_APPOBJS  := $(addprefix $(APPOBJS_DIR)/,$(MQTT_OBJS))

$(APPOBJS_DIR)/%.o: $(APPSRC_DIR)/%.c
	@if test ! -d $(APPOBJS_DIR) ; \
		then \
		$(MKDIR) -p $(APPOBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
#
#
-include $(MODULE_APPOBJS:.o=.d)
#
#$(APPLIBS) : $(MODULE_APPOBJS) 
#	$(PL_MAKE_LIB) $(APPLIBS) $(MODULE_APPOBJS) 

#
#
#
#
#
ifeq ($(WITH_SHARED_LIBRARIES),yes)
$(LIBS) : $(MODULE_OBJS) $(MODULE_APPOBJS) 
	$(CC) -shared -o  $(LIBS) $(MODULE_OBJS) $(MODULE_APPOBJS)  
else
$(LIBS) : $(MODULE_OBJS) $(MODULE_APPOBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) $(MODULE_APPOBJS) 
endif


#
#
lib: $(LIBS)
	@${MAKE} -C  src all

obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

ifeq ($(BUILD_DEBUG),YES)
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	@${MAKE} -C  src install DEST_BIN_DIR=$(DSTSBINDIR)
else
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	$(STRIP) $(PLSTRIP_CFLAGS)$(LIB_DIR)/$(LIBS)	
	@${MAKE} -C  src install DEST_BIN_DIR=$(DSTSBINDIR)
endif

#install: $(LIBS)
#	install -d $(LIB_DIR)
#	install -m 755 $(LIBS) $(LIB_DIR)
#	$(STRIP) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)
	@${MAKE} -C  src clean DEST_BIN_DIR=$(DSTSBINDIR)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean
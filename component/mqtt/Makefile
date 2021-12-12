include $(ZPL_MAKE_DIR)/makelinux.mk

ZPL_MQTT_BROKER=false
include config-def.mk
include config.mk

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
ifeq ($(WITH_SHARED_LIBRARIES),yes)
LIB_DIR = $(BASE_ROOT)/$(ZPL_ULIB_DIR)
endif
ifeq ($(WITH_STATIC_LIBRARIES),yes)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)
endif

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(MOSQ_OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#
#
APPOBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEAPPDIR)
APPSRC_DIR = $(BASE_ROOT)/$(MODULEAPPDIR)
#APPLIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_APPOBJS  := $(addprefix $(APPOBJS_DIR)/,$(MQTT_OBJS))
MODULE_APPOBJS_DEP:=$(MODULE_APPOBJS:%.o=%.d)
-include $(MODULE_APPOBJS_DEP)

$(APPOBJS_DIR)/%.o: $(APPSRC_DIR)/%.c
	@if test ! -d $(APPOBJS_DIR) ; \
		then \
		$(MKDIR) -p $(APPOBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#
#
-include $(MODULE_APPOBJS:.o=.d)
#
#$(APPLIBS) : $(MODULE_APPOBJS) 
#	$(ZPL_MAKE_LIB) $(APPLIBS) $(MODULE_APPOBJS) 

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


files = $(patsubst %.o,%.c,$(MQTT_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done
#
#
ifeq ($(ZPL_MQTT_SRV),true)
lib: $(LIBS)
	@${MAKE} -C  src all
else	
lib: $(LIBS)
endif

obj: $(MODULE_OBJS)

ifeq ($(ZPL_MQTT_SRV),true)
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
	@${MAKE} -C  src install DEST_BIN_DIR=$(ZPL_INSTALL_SBIN_DIR)
else
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)	
endif


ifeq ($(ZPL_MQTT_SRV),true)
clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)
	@${MAKE} -C  src clean DEST_BIN_DIR=$(ZPL_INSTALL_SBIN_DIR)
else
clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)
endif

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare
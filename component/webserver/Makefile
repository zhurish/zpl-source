include $(MAKE_DIR)/makelinux.mk

include config.mk

WEBGUI_DEBUG=false
WEBGUI_TRACE=true

export THEME_MATERIAL=false
export THEME_BOOTSTRAP=false
export THEME_V9UI=false

ifeq ($(WEBGUI_DEBUG),true)
WEB_DEFINE = -DME_DEBUG=1 -DME_GOAHEAD_LOGGING=1 -DME_GOAHEAD_EXTLOG=1 -D__WEBGUI_DEBUG -DWEBGUI_SRC_DIR=\"$(WEBGUI_ROOT)\" 
#-DME_GOAHEAD_TRACING=1 
else
WEB_DEFINE = -DME_DEBUG=1 -DME_GOAHEAD_LOGGING=1 -DME_GOAHEAD_EXTLOG=1
#-DME_GOAHEAD_TRACING=0
endif

ifeq ($(WEBGUI_TRACE),true)
WEB_DEFINE += -DME_GOAHEAD_TRACING=1
else
WEB_DEFINE += -DME_GOAHEAD_TRACING=0
endif


ifeq ($(THEME_MATERIAL),true)
WEB_DEFINE += -DTHEME_MATERIAL
endif

ifeq ($(THEME_BOOTSTRAP),true)
WEB_DEFINE += -DTHEME_BOOTSTRAP
endif

ifeq ($(THEME_V9UI),true)
WEB_DEFINE += -DTHEME_V9UI
endif

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)/src
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

WEB_DIR = $(BASE_ROOT)/$(MODULEDIR)
MODULE_WEB_OBJS  := $(addprefix $(OBJS_DIR)/,$(WEBOBJS))

APP_WEB_DIR = $(BASE_ROOT)/$(MODULEDIR)/app/web
MODULE_APP_WEB_OBJS  := $(addprefix $(OBJS_DIR)/,$(APPWEBOBJS))

APP_JST_DIR = $(BASE_ROOT)/$(MODULEDIR)/app/jst
MODULE_APP_JST_OBJS  := $(addprefix $(OBJS_DIR)/,$(APPJSTOBJS))


PLCFLAGS +=$(ME_DFLAGS) $(WEB_DEFINE) 
#-DME_GOAHEAD_UPLOAD_DIR=\"/tmp/app/tftpboot\"
PLINCLUDE +=-I$(WEBGUI_ROOT)/src

ifeq ($(ME_COM_MBEDTLS),1)
mbedtls_DIR = $(BASE_ROOT)/$(MODULEDIR)/src/mbedtls
mbedtls_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(mbedtls_OBJS))

goahead_mbedtls_DIR = $(BASE_ROOT)/$(MODULEDIR)/src/goahead-mbedtls
goahead_mbedtls_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(goahead_mbedtls_OBJS))
endif
ifeq ($(ME_COM_OPENSSL),1)
goahead_openssl_DIR = $(BASE_ROOT)/$(MODULEDIR)/src/goahead-openssl
goahead_openssl_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(goahead_openssl_OBJS))
endif
gopass_DIR = $(BASE_ROOT)/$(MODULEDIR)/src/utils
gopass_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(gopass_OBJS))

# It's a kind of magic...
DEP_H = me.h embedtls.h mbedtls.h osdep.h

me.h:
	@[ ! -x ./include ] && mkdir -p ./include; true
	@[ ! -f include/me.h ] && cp goahead-linux-default-me.h include/me.h ; true
	@if ! diff include/me.h goahead-linux-default-me.h >/dev/null ; then\
		cp goahead-linux-default-me.h include/me.h  ; \
	fi; true

embedtls.h:
	@[ ! -x ./include ] && mkdir -p ./include; true
	@[ ! -f include/embedtls.h ] && cp src/mbedtls/embedtls.h include/embedtls.h ; true
	@if ! diff include/embedtls.h src/mbedtls/embedtls.h >/dev/null ; then\
		cp src/mbedtls/embedtls.h include/embedtls.h  ; \
	fi; true
	
mbedtls.h:
	@[ ! -x ./include ] && mkdir -p ./include; true
	@[ ! -f include/mbedtls.h ] && cp src/mbedtls/mbedtls.h include/mbedtls.h ; true
	@if ! diff include/mbedtls.h src/mbedtls/mbedtls.h >/dev/null ; then\
		cp src/mbedtls/mbedtls.h include/mbedtls.h  ; \
	fi; true
	
osdep.h:
	@[ ! -x ./include ] && mkdir -p ./include; true
	@[ ! -f include/osdep.h ] && cp src/osdep/osdep.h include/osdep.h ; true
	@if ! diff include/osdep.h src/osdep/osdep.h >/dev/null ; then\
		cp src/osdep/osdep.h include/osdep.h  ; \
	fi; true
		
		
ifeq ($(ME_COM_MBEDTLS),1)
$(OBJS_DIR)/%.o: $(mbedtls_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
	
$(OBJS_DIR)/%.o: $(goahead_mbedtls_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
endif

ifeq ($(ME_COM_OPENSSL),1)	
$(OBJS_DIR)/%.o: $(goahead_openssl_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
endif
	
$(OBJS_DIR)/%.o: $(gopass_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)		
					
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
	
$(OBJS_DIR)/%.o: $(WEB_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

$(OBJS_DIR)/%.o: $(APP_WEB_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)	

$(OBJS_DIR)/%.o: $(APP_JST_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)
	
		
#
#
$(LIBS) : $(DEP_H) $(MODULE_OBJS) $(mbedtls_MODULE_OBJS) $(goahead_mbedtls_MODULE_OBJS) \
		  $(goahead_openssl_MODULE_OBJS) $(gopass_MODULE_OBJS) $(MODULE_WEB_OBJS) \
		  $(MODULE_APP_WEB_OBJS) $(MODULE_APP_JST_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) $(mbedtls_MODULE_OBJS) \
	 $(goahead_mbedtls_MODULE_OBJS) $(goahead_openssl_MODULE_OBJS) \
	 $(gopass_MODULE_OBJS) $(MODULE_WEB_OBJS) $(MODULE_APP_WEB_OBJS) $(MODULE_APP_JST_OBJS)


-include $(MODULE_WEB_OBJS:.o=.d)
-include $(MODULE_APP_WEB_OBJS:.o=.d)
-include $(MODULE_APP_JST_OBJS:.o=.d)

ifeq ($(ME_COM_MBEDTLS),1)
-include $(mbedtls_MODULE_OBJS:.o=.d)
-include $(goahead_mbedtls_MODULE_OBJS:.o=.d)
endif
ifeq ($(ME_COM_OPENSSL),1)
-include $(goahead_openssl_MODULE_OBJS:.o=.d)
endif
-include $(gopass_MODULE_OBJS:.o=.d)



ifeq ($(ME_COM_SSL),1)
sslinstall: 
	@install -d $(DSTETCDIR)/web
	@$(CP) src/certs/samples/ca.crt $(DSTETCDIR)/web/ca.crt
	@$(CP) src/certs/samples/ca.key $(DSTETCDIR)/web/ca.key
	@$(CP) src/certs/samples/ec.crt $(DSTETCDIR)/web/ec.crt
	@$(CP) src/certs/samples/ec.key $(DSTETCDIR)/web/ec.key
	@$(CP) src/certs/samples/roots.crt $(DSTETCDIR)/web/roots.crt
	@$(CP) src/certs/samples/self.crt $(DSTETCDIR)/web/self.crt
	@$(CP) src/certs/samples/self.key $(DSTETCDIR)/web/self.key
#
#
sslclean: 
	@$(RM) $(DSTETCDIR)/web/ca.crt
	@$(RM) $(DSTETCDIR)/web/ca.key
	@$(RM) $(DSTETCDIR)/web/ec.crt
	@$(RM) $(DSTETCDIR)/web/ec.key
	@$(RM) $(DSTETCDIR)/web/roots.crt
	@$(RM) $(DSTETCDIR)/web/self.crt
	@$(RM) $(DSTETCDIR)/web/self.key
#
#
else
sslinstall: 
#
#
sslclean: 
#	
endif


#
-include $(MODULE_OBJS:.o=.d)
#
#
#$(LIBS) : $(MODULE_OBJS)
#	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)
#
lib: $(LIBS)



obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

webinstall: 
	@${MAKE} -C  web/ install 
webclean: 
	@${MAKE} -C  web/ clean 

install: $(LIBS) webinstall sslinstall
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)

clean: objclean webclean sslclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean webclean webinstall sslinstall sslclean
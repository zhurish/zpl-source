#
#
export APP_WEB_DIR =$(shell pwd)

ECHO = echo
CD = cd
RM = rm
MV = mv
CP = cp
CHMOD = chmod
MKDIR = mkdir
TAR = tar

ifneq ($(INSTALLDSTWEB),)
DSTWEBDIR=$(INSTALLDSTWEB)
else
DSTWEBDIR=$(shell cd ../ && pwd)
endif
#
#
#
CONTROLDIR=$(DSTWEBDIR)/usr/lib/lua/luci/controller
CBIDIR=$(DSTWEBDIR)/usr/lib/lua/luci/model/cbi
VIEWDIR=$(DSTWEBDIR)/usr/lib/lua/luci/view
ETCDIR=$(DSTWEBDIR)/etc/config
#
#
#	
.PHONY:	clean install all
#

clean: 
	@if test -f $(CONTROLDIR)/admin/voip.lua ; \
		then \
		$(RM) $(CONTROLDIR)/admin/voip.lua; \
	fi
	@if test -f $(CBIDIR)/admin_voip/sipconfig.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_voip/sipconfig.lua; \
	fi	
	@if test -f $(CBIDIR)/admin_voip/voipconfig.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_voip/voipconfig.lua; \
	fi	
	@if test -f $(CBIDIR)/admin_voip/voip_testing.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_voip/voip_testing.lua; \
	fi	
			
	@if test -f $(VIEWDIR)/admin_voip/voip_status.htm ; \
		then \
		$(RM) $(VIEWDIR)/admin_voip/voip_status.htm; \
	fi
	@if test -f $(VIEWDIR)/admin_voip/voip_testing.htm ; \
		then \
		$(RM) $(VIEWDIR)/admin_voip/voip_testing.htm; \
	fi
	@if test -f $(ETCDIR)/voipconfig ; \
		then \
		$(RM) $(ETCDIR)/voipconfig; \
	fi
	
install: 
	$(MKDIR) -p  $(CONTROLDIR)/admin
	$(CP) $(APP_WEB_DIR)/luasrc/controller/voip.lua $(CONTROLDIR)/admin/
	
	$(MKDIR) -p  $(CBIDIR)/admin_voip
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/sipconfig.lua $(CBIDIR)/admin_voip/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/voipconfig.lua $(CBIDIR)/admin_voip/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/voip_testing.lua $(CBIDIR)/admin_voip/
		
	$(MKDIR) -p  $(VIEWDIR)/admin_voip
	$(CP) $(APP_WEB_DIR)/luasrc/view/voip_status.htm $(VIEWDIR)/admin_voip/
	$(CP) $(APP_WEB_DIR)/luasrc/view/voip_testing.htm $(VIEWDIR)/admin_voip/
	$(CP) $(APP_WEB_DIR)/luasrc/view/capture*.htm $(VIEWDIR)/admin_voip/
	
	$(MKDIR) -p  $(ETCDIR)
	$(CP) $(APP_WEB_DIR)/root/etc/voipconfig.cfg $(ETCDIR)/voipconfig
	#$(CP) $(APP_WEB_DIR)/root/etc/dbase $(ETCDIR)/dbase
	
all: install 
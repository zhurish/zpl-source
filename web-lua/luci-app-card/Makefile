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
	@if test -f $(CONTROLDIR)/admin/card.lua ; \
		then \
		$(RM) $(CONTROLDIR)/admin/card.lua; \
	fi
	@if test -f $(CBIDIR)/admin_card/card.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_card/card.lua; \
	fi		
	@if test -f $(CBIDIR)/admin_card/makefacecard.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_card/makefacecard.lua; \
	fi
	@if test -f $(CBIDIR)/admin_card/face.lua ; \
		then \
		$(RM) $(CBIDIR)/admin_card/face.lua; \
	fi
	$(RM) $(VIEWDIR)/admin_card/datetime*;
	@if test -f $(VIEWDIR)/admin_card/img_upload.htm ; \
		then \
		$(RM) $(VIEWDIR)/admin_card/img_upload.htm; \
	fi
	@if test -f $(VIEWDIR)/admin_card/refresh_button.htm ; \
		then \
		$(RM) $(VIEWDIR)/admin_card/refresh_button.htm; \
	fi
	@if test -f $(VIEWDIR)/admin_card/img_show.htm ; \
		then \
		$(RM) $(VIEWDIR)/admin_card/img_show.htm; \
	fi	
install: 
	$(MKDIR) -p  $(CONTROLDIR)/admin
	$(CP) $(APP_WEB_DIR)/luasrc/controller/card.lua $(CONTROLDIR)/admin/
	
	$(MKDIR) -p  $(CBIDIR)/admin_card
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/card.lua $(CBIDIR)/admin_card/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/makefacecard.lua $(CBIDIR)/admin_card/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/face.lua $(CBIDIR)/admin_card/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/userauth.lua $(CBIDIR)/admin_card/
	
	$(MKDIR) -p  $(VIEWDIR)/admin_card
	$(CP) -arf $(APP_WEB_DIR)/luasrc/view/datetime* $(VIEWDIR)/admin_card/
	$(CP) $(APP_WEB_DIR)/luasrc/view/img_upload.htm $(VIEWDIR)/admin_card/	
	$(CP) $(APP_WEB_DIR)/luasrc/view/refresh_button.htm $(VIEWDIR)/admin_card/	
	$(CP) $(APP_WEB_DIR)/luasrc/view/img_show.htm $(VIEWDIR)/admin_card/	
	
	
all: install 
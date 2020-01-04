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
	$(RM) $(CONTROLDIR)/admin/factory.lua; 
	$(RM) $(CBIDIR)/admin_factory/factoryconfig.lua;
	$(RM) $(CBIDIR)/admin_factory/roomdbconfig.lua;
	$(RM) $(CBIDIR)/admin_factory/appsystemlog.lua;
	$(RM) $(CBIDIR)/admin_factory/openconfig.lua;			
	$(RM) $(VIEWDIR)/admin_factory/factoryconfig.htm;
	$(RM) $(VIEWDIR)/admin_factory/room.htm;
	$(RM) $(ETCDIR)/openconfig;
	$(RM) $(ETCDIR)/dbase;
	$(RM) $(ETCDIR)/product;
	
	
install: 
	$(MKDIR) -p  $(CONTROLDIR)/admin
	$(CP) $(APP_WEB_DIR)/luasrc/controller/factory.lua $(CONTROLDIR)/admin/
	
	$(MKDIR) -p  $(CBIDIR)/admin_factory
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/factoryconfig.lua $(CBIDIR)/admin_factory/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/roomdbconfig.lua $(CBIDIR)/admin_factory/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/appsystemlog.lua $(CBIDIR)/admin_factory/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/openconfig.lua $(CBIDIR)/admin_factory/
	$(CP) $(APP_WEB_DIR)/luasrc/model/cbi/appcard.lua $(CBIDIR)/admin_factory/
	
	$(MKDIR) -p  $(VIEWDIR)/admin_factory
	$(CP) $(APP_WEB_DIR)/luasrc/view/factoryconfig.htm $(VIEWDIR)/admin_factory/
	$(CP) $(APP_WEB_DIR)/luasrc/view/room.htm $(VIEWDIR)/admin_factory/
	
	
	$(MKDIR) -p  $(ETCDIR)
	$(CP) $(APP_WEB_DIR)/root/etc/openconfig $(ETCDIR)/openconfig
	$(CP) $(APP_WEB_DIR)/root/etc/dbase $(ETCDIR)/dbase
	$(CP) $(APP_WEB_DIR)/root/etc/product $(ETCDIR)/product
	
all: install 
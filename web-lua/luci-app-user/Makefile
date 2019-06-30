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
	$(RM) $(CONTROLDIR)/user.lua;
	$(RM) $(CBIDIR)/user.lua;
	$(RM) $(ETCDIR)/sysuser;
	
install: 
	$(MKDIR) -p  $(CONTROLDIR)
	$(CP) ./luasrc/controller/user.lua $(CONTROLDIR)/user.lua
	
	$(MKDIR) -p  $(CBIDIR)
	$(CP) ./luasrc/model/cbi/user.lua $(CBIDIR)/user.lua
	
	$(MKDIR) -p  $(ETCDIR)
	$(CP) $(APP_WEB_DIR)/root/etc/sysuser $(ETCDIR)/sysuser
	
all: install 


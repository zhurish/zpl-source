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
	$(RM) $(CONTROLDIR)/filetransfer.lua;
	$(RM) $(CBIDIR)/updownload.lua;
	$(RM) $(VIEWDIR)/cbi/other_upload.htm;

	
install: 
	$(MKDIR) -p  $(CONTROLDIR)
	$(CP) ./luasrc/controller/filetransfer.lua $(CONTROLDIR)/filetransfer.lua
	
	$(MKDIR) -p  $(CBIDIR)
	$(CP) ./luasrc/model/cbi/updownload.lua $(CBIDIR)/updownload.lua
		
	$(MKDIR) -p  $(VIEWDIR)/cbi
	$(CP) ./luasrc/view/cbi/other_upload.htm $(VIEWDIR)/cbi/

all: install 


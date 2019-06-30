#
#
export WEB_DIR =$(shell pwd)

#include ../make/makelinux.mk
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
.PHONY:	clean install all 
#

clean: 
	$(MAKE) -C luci-app-factory clean INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-voip clean INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-card clean INSTALLDSTWEB=$(DSTWEBDIR)	
	$(MAKE) -C luci-app-filetransfer clean INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-user clean INSTALLDSTWEB=$(DSTWEBDIR)
	
install: 
	$(MAKE) -C luci-app-factory install INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-voip install INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-card install INSTALLDSTWEB=$(DSTWEBDIR)	
	$(MAKE) -C luci-app-filetransfer install INSTALLDSTWEB=$(DSTWEBDIR)
	$(MAKE) -C luci-app-user install INSTALLDSTWEB=$(DSTWEBDIR)
	
all: install 
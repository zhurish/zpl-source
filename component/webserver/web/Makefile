include $(MAKE_DIR)/makelinux.mk

#
#

#
#
#
ifeq ($(THEME_MATERIAL),true)
    WEB_THEME_DIR = ./theme/material/
endif
ifeq ($(THEME_BOOTSTRAP),true)
    WEB_THEME_DIR = ./theme/bootstrap/
endif

ifeq ($(THEME_V9UI),true)
    #WEB_THEME_DIR = 
endif

#ifeq ($(THEME),THEME_MATERIAL)
#    WEB_THEME_DIR = ./theme/material/
#endif
#ifeq ($(THEME),THEME_BOOTSTRAP)
#    WEB_THEME_DIR = ./theme/bootstrap/
#endif
#
ifeq ($(THEME_V9UI),true)

install: 
	@install -d $(DSTETCDIR)/web
	@$(CP) ./auth.txt $(DSTETCDIR)/web/auth.txt
	@$(CP) ./route.txt $(DSTETCDIR)/web/route.txt
	@install -d $(DSTWWWDIR)
	@$(CP) ./theme/build.tar.gz $(DSTWWWDIR)/
	
clean:  
	@$(RM) $(DSTETCDIR)/web/auth.txt
	@$(RM) $(DSTETCDIR)/web/route.txt
	@$(RM) -rf $(DSTWWWDIR)/*
	
.PHONY:	install clean	
endif

#
ifneq ($(WEB_THEME_DIR),)
#
web_theme_install: 
		${MAKE} -C  $(WEB_THEME_DIR) install	
			
#
#
web_theme_clean: 
		${MAKE} -C  $(WEB_THEME_DIR) clean
#
#		
#	
webinstall: 
	@install -d $(DSTETCDIR)/web
	@$(CP) ./auth.txt $(DSTETCDIR)/web/auth.txt
	@$(CP) ./route.txt $(DSTETCDIR)/web/route.txt
	#install -d $(DSTWWWDIR)
	#$(CP) ./theme/build.tar.gz $(DSTWWWDIR)/
	

webclean: 
	@$(RM) $(DSTETCDIR)/web/auth.txt
	@$(RM) $(DSTETCDIR)/web/route.txt
	@$(RM) -rf $(DSTWWWDIR)/*
#
#
install: webinstall web_theme_install
#
#
#
clean: webclean web_theme_clean
#
#
#
.PHONY:	install clean webclean webinstall web_theme_install web_theme_clean

endif
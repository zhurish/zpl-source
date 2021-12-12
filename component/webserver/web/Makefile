include $(ZPL_MAKE_DIR)/makelinux.mk

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
   # WEB_THEME_DIR = aa
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
	@install -d $(ZPL_INSTALL_ETC_DIR)/web
	@$(CP) ./auth.txt $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(CP) ./route.txt $(ZPL_INSTALL_ETC_DIR)/web/route.txt
	@install -d $(ZPL_INSTALL_WWW_DIR)
	@$(CP) ./theme/build.tar.gz $(ZPL_INSTALL_WWW_DIR)/
	
clean:  
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/route.txt
	@$(RM) -rf $(ZPL_INSTALL_WWW_DIR)/*
	
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
	@install -d $(ZPL_INSTALL_ETC_DIR)/web
	@$(CP) ./auth.txt $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(CP) ./route.txt $(ZPL_INSTALL_ETC_DIR)/web/route.txt
	#install -d $(ZPL_INSTALL_WWW_DIR)
	#$(CP) ./theme/build.tar.gz $(ZPL_INSTALL_WWW_DIR)/
	

webclean: 
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/route.txt
	@$(RM) -rf $(ZPL_INSTALL_WWW_DIR)/*
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

else

install: 
	@install -d $(ZPL_INSTALL_ETC_DIR)/web
	@$(CP) ./auth.txt $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(CP) ./route.txt $(ZPL_INSTALL_ETC_DIR)/web/route.txt

clean:  
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/auth.txt
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/web/route.txt
	@$(RM) -rf $(ZPL_INSTALL_WWW_DIR)/*
	
.PHONY:	install clean


endif
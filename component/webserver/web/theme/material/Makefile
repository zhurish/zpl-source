include $(ZPL_MAKE_DIR)/makelinux.mk

#
#
#
#
#
# /www/
# css js res res/cbi res/icons asp cgi-bin html fonts
#
#	
web_js_install:
	install -d $(ZPL_INSTALL_WWW_DIR)/js
	$(CP) -arf ./js/* $(ZPL_INSTALL_WWW_DIR)/js/
	
#
#
web_cgi_install: 
	install -d $(ZPL_INSTALL_WWW_DIR)/cgi-bin
	#install -m 755 ./cgi-bin/test.cgi $(ZPL_INSTALL_WWW_DIR)/cgi-bin
#
#	
web_css_install:
	install -d $(ZPL_INSTALL_WWW_DIR)/css
	$(CP) ./cascade.css $(ZPL_INSTALL_WWW_DIR)/css/cascade.css
#
#	
web_res_install:
	install -d $(ZPL_INSTALL_WWW_DIR)/res
	$(CP) -arf ./res/* $(ZPL_INSTALL_WWW_DIR)/res/
	$(CP) -arf ./logo.png $(ZPL_INSTALL_WWW_DIR)/res/
	$(CP) -arf ./favicon.ico $(ZPL_INSTALL_WWW_DIR)/res/	
#
#
web_fonts_install:
	install -d $(ZPL_INSTALL_WWW_DIR)/fonts
	$(CP) -arf ./fonts/* $(ZPL_INSTALL_WWW_DIR)/fonts/
#
#
#				
#
#
web_html_install:
	install -d $(ZPL_INSTALL_WWW_DIR)/html
	$(CP) ./html/login.html $(ZPL_INSTALL_WWW_DIR)/html/login.html
	#$(CP) ./html/upload.html $(ZPL_INSTALL_WWW_DIR)/html/upload.html
	#$(CP) ./html/test.html $(ZPL_INSTALL_WWW_DIR)/html/test.html
	#$(CP) ./html/login.html $(ZPL_INSTALL_WWW_DIR)/html/login.html		
	#$(CP) .//html/main.html $(ZPL_INSTALL_WWW_DIR)/html/main.html
	#$(CP) ./html/upload.html $(ZPL_INSTALL_WWW_DIR)/html/upload.html
	#$(CP) ./html/overview.html $(ZPL_INSTALL_WWW_DIR)/html/overview.html
	#$(CP) ./html/version.html $(ZPL_INSTALL_WWW_DIR)/html/version.html	
	#$(CP) ./html/menu.html $(ZPL_INSTALL_WWW_DIR)/html/menu.html
	#$(CP) ./html/header.html $(ZPL_INSTALL_WWW_DIR)/html/header.html	
	#$(CP) ./html/footer.html $(ZPL_INSTALL_WWW_DIR)/html/footer.html	
	#$(CP) ./html/*.html $(ZPL_INSTALL_WWW_DIR)/html/

#	
	
#
#	
webinstall: 
	install -d $(ZPL_INSTALL_WWW_DIR)
	$(CP) ./html/index.html $(ZPL_INSTALL_WWW_DIR)/index.html	


webclean:
	$(RM) -rf $(ZPL_INSTALL_WWW_DIR)/*
#
#
install: webinstall web_cgi_install web_css_install web_res_install web_html_install \
			web_js_install web_fonts_install
#
#
#
clean: webclean
#
#
#
.PHONY:	install clean webclean webinstall web_cgi_install web_css_install \
		web_res_install web_fonts_install web_js_install web_html_install
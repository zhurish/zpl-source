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
	@install -d $(ZPL_INSTALL_WWW_DIR)/js
	@$(CP) -arf ./js/bootstrap.min.js $(ZPL_INSTALL_WWW_DIR)/js/bootstrap.min.js
	@$(CP) -arf ./js/bootstrap-switch.min.js $(ZPL_INSTALL_WWW_DIR)/js/bootstrap-switch.min.js
	@$(CP) -arf ./js/jquery-2.1.1.min.js $(ZPL_INSTALL_WWW_DIR)/js/jquery-2.1.1.min.js
	@$(CP) -arf ./js/xhr.js $(ZPL_INSTALL_WWW_DIR)/js/xhr.js
	@$(CP) -arf ./js/data-type.js $(ZPL_INSTALL_WWW_DIR)/js/data-type.js
	
	#$(CP) -arf ./js/complex-menu.js $(ZPL_INSTALL_WWW_DIR)/js/complex-menu.js
	@$(CP) -arf ./js/bootstrap-menu.js $(ZPL_INSTALL_WWW_DIR)/js/bootstrap-menu.js		
#
#
web_cgi_install: 
	@install -d $(ZPL_INSTALL_WWW_DIR)/cgi-bin
	#install -m 755 ./cgi-bin/test.cgi $(ZPL_INSTALL_WWW_DIR)/cgi-bin
#
#	
web_css_install:
	@install -d $(ZPL_INSTALL_WWW_DIR)/css
	@$(CP) ./css/bootstrap.min.css $(ZPL_INSTALL_WWW_DIR)/css/bootstrap.min.css
	@$(CP) ./css/bootstrap-switch.min.css $(ZPL_INSTALL_WWW_DIR)/css/bootstrap-switch.min.css
	@$(CP) ./css/private.css $(ZPL_INSTALL_WWW_DIR)/css/private.css
	#$(CP) ./css/login.css $(ZPL_INSTALL_WWW_DIR)/css/login.css
	#$(CP) ./css/complex-menu.css $(ZPL_INSTALL_WWW_DIR)/css/complex-menu.css		
	#$(CP) ./css/bootstrap-menu.css $(ZPL_INSTALL_WWW_DIR)/css/bootstrap-menu.css			
#
#	
web_res_install:
	@install -d $(ZPL_INSTALL_WWW_DIR)/res
	@$(CP) -arf ./res/user.png $(ZPL_INSTALL_WWW_DIR)/res/
	@$(CP) -arf ./res/pwd.png $(ZPL_INSTALL_WWW_DIR)/res/
	@$(CP) -arf ./res/logo.png $(ZPL_INSTALL_WWW_DIR)/res/
	#$(CP) -arf ./res/login.jpg $(ZPL_INSTALL_WWW_DIR)/res/
	@$(CP) -arf ./images/face.jpg $(ZPL_INSTALL_WWW_DIR)/res/
	
	#install -d $(ZPL_INSTALL_WWW_DIR)/res/cbi
	@install -d $(ZPL_INSTALL_WWW_DIR)/res/icons
	@$(CP) -arf ./res/icons/signal-* $(ZPL_INSTALL_WWW_DIR)/res/icons/
	@$(CP) -arf ./res/icons/port_* $(ZPL_INSTALL_WWW_DIR)/res/icons/
	@$(CP) -arf ./res/icons/loading.gif $(ZPL_INSTALL_WWW_DIR)/res/icons/
	#$(CP) -arf ./res/* $(ZPL_INSTALL_WWW_DIR)/res/		
#
#
web_fonts_install:
	@install -d $(ZPL_INSTALL_WWW_DIR)/fonts
	@$(CP) -arf ./fonts/* $(ZPL_INSTALL_WWW_DIR)/fonts/
	#$(CP) ./css/login.css $(ZPL_INSTALL_WWW_DIR)/css/login.css
	#$(CP) ./css/login.css $(ZPL_INSTALL_WWW_DIR)/css/login.css
	#$(CP) ./css/login.css $(ZPL_INSTALL_WWW_DIR)/css/login.css
#
#
#				
#

web_app_html_install:
	@install -d $(ZPL_INSTALL_WWW_DIR)/html
	@$(CP) ./html/bootstrap-v9.html $(ZPL_INSTALL_WWW_DIR)/html/bootstrap.html
	@$(CP) ./html/boardcard.html $(ZPL_INSTALL_WWW_DIR)/html/boardcard.html
	@$(CP) ./html/general.html $(ZPL_INSTALL_WWW_DIR)/html/general.html	
	@$(CP) ./html/rtsp.html $(ZPL_INSTALL_WWW_DIR)/html/rtsp.html	
	@$(CP) ./html/algorithm.html $(ZPL_INSTALL_WWW_DIR)/html/algorithm.html	
	@$(CP) ./html/facelib.html $(ZPL_INSTALL_WWW_DIR)/html/facelib.html	
	@$(CP) ./html/upload.html $(ZPL_INSTALL_WWW_DIR)/html/upload.html						

#
web_html_install:
	@install -d $(ZPL_INSTALL_WWW_DIR)/html
	@$(CP) ./html/systemview.html $(ZPL_INSTALL_WWW_DIR)/html/systemview.html
	@$(CP) ./html/system.html $(ZPL_INSTALL_WWW_DIR)/html/system.html	
	@$(CP) ./html/about.html $(ZPL_INSTALL_WWW_DIR)/html/about.html
	@$(CP) ./html/admin.html $(ZPL_INSTALL_WWW_DIR)/html/admin.html
	@$(CP) ./html/dhcp.html $(ZPL_INSTALL_WWW_DIR)/html/dhcp.html		
	@$(CP) ./html/dns.html $(ZPL_INSTALL_WWW_DIR)/html/dns.html
	@$(CP) ./html/filesystem.html $(ZPL_INSTALL_WWW_DIR)/html/filesystem.html
	@$(CP) ./html/nat.html $(ZPL_INSTALL_WWW_DIR)/html/nat.html		
	@$(CP) ./html/netstatus.html $(ZPL_INSTALL_WWW_DIR)/html/netstatus.html
	@$(CP) ./html/network.html $(ZPL_INSTALL_WWW_DIR)/html/network.html
	@$(CP) ./html/route.html $(ZPL_INSTALL_WWW_DIR)/html/route.html	
	@$(CP) ./html/switch.html $(ZPL_INSTALL_WWW_DIR)/html/switch.html
	@$(CP) ./html/syslog.html $(ZPL_INSTALL_WWW_DIR)/html/syslog.html
	@$(CP) ./html/login.html $(ZPL_INSTALL_WWW_DIR)/html/login.html
	@$(CP) ./html/reboot.html $(ZPL_INSTALL_WWW_DIR)/html/reboot.html



#	
	
#
#	
webinstall: 
	@install -d $(ZPL_INSTALL_WWW_DIR)
	@$(CP) ./html/index.html $(ZPL_INSTALL_WWW_DIR)/index.html	


webclean:
	@$(RM) -rf $(ZPL_INSTALL_WWW_DIR)/*
#
#
install: webinstall web_cgi_install web_css_install web_res_install web_html_install \
			web_js_install web_fonts_install web_app_html_install
#
#
#
clean: webclean
#
#
#
.PHONY:	install clean webclean webinstall web_cgi_install web_css_install \
		web_res_install web_fonts_install web_js_install web_html_install web_app_html_install
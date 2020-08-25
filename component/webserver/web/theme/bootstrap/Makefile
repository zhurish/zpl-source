include $(MAKE_DIR)/makelinux.mk

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
	@install -d $(DSTWWWDIR)/js
	@$(CP) -arf ./js/bootstrap.min.js $(DSTWWWDIR)/js/bootstrap.min.js
	@$(CP) -arf ./js/bootstrap-switch.min.js $(DSTWWWDIR)/js/bootstrap-switch.min.js
	@$(CP) -arf ./js/jquery-2.1.1.min.js $(DSTWWWDIR)/js/jquery-2.1.1.min.js
	@$(CP) -arf ./js/xhr.js $(DSTWWWDIR)/js/xhr.js
	@$(CP) -arf ./js/data-type.js $(DSTWWWDIR)/js/data-type.js
	
	#$(CP) -arf ./js/complex-menu.js $(DSTWWWDIR)/js/complex-menu.js
	@$(CP) -arf ./js/bootstrap-menu.js $(DSTWWWDIR)/js/bootstrap-menu.js		
#
#
web_cgi_install: 
	@install -d $(DSTWWWDIR)/cgi-bin
	#install -m 755 ./cgi-bin/test.cgi $(DSTWWWDIR)/cgi-bin
#
#	
web_css_install:
	@install -d $(DSTWWWDIR)/css
	@$(CP) ./css/bootstrap.min.css $(DSTWWWDIR)/css/bootstrap.min.css
	@$(CP) ./css/bootstrap-switch.min.css $(DSTWWWDIR)/css/bootstrap-switch.min.css
	@$(CP) ./css/private.css $(DSTWWWDIR)/css/private.css
	#$(CP) ./css/login.css $(DSTWWWDIR)/css/login.css
	#$(CP) ./css/complex-menu.css $(DSTWWWDIR)/css/complex-menu.css		
	#$(CP) ./css/bootstrap-menu.css $(DSTWWWDIR)/css/bootstrap-menu.css			
#
#	
web_res_install:
	@install -d $(DSTWWWDIR)/res
	@$(CP) -arf ./res/user.png $(DSTWWWDIR)/res/
	@$(CP) -arf ./res/pwd.png $(DSTWWWDIR)/res/
	@$(CP) -arf ./res/logo.png $(DSTWWWDIR)/res/
	#$(CP) -arf ./res/login.jpg $(DSTWWWDIR)/res/
	@$(CP) -arf ./images/face.jpg $(DSTWWWDIR)/res/
	
	#install -d $(DSTWWWDIR)/res/cbi
	@install -d $(DSTWWWDIR)/res/icons
	@$(CP) -arf ./res/icons/signal-* $(DSTWWWDIR)/res/icons/
	@$(CP) -arf ./res/icons/port_* $(DSTWWWDIR)/res/icons/
	@$(CP) -arf ./res/icons/loading.gif $(DSTWWWDIR)/res/icons/
	#$(CP) -arf ./res/* $(DSTWWWDIR)/res/		
#
#
web_fonts_install:
	@install -d $(DSTWWWDIR)/fonts
	@$(CP) -arf ./fonts/* $(DSTWWWDIR)/fonts/
	#$(CP) ./css/login.css $(DSTWWWDIR)/css/login.css
	#$(CP) ./css/login.css $(DSTWWWDIR)/css/login.css
	#$(CP) ./css/login.css $(DSTWWWDIR)/css/login.css
#
#
#				
#
ifeq ($(strip $(EN_APP_X5BA)),true)
web_app_html_install:
	@install -d $(DSTWWWDIR)/html
	@$(CP) ./html/switch.html $(DSTWWWDIR)/html/switch.html
	@$(CP) ./html/factory.html $(DSTWWWDIR)/html/factory.html
	@$(CP) ./html/sip.html $(DSTWWWDIR)/html/sip.html	
	@$(CP) ./html/wireless.html $(DSTWWWDIR)/html/wireless.html
	@$(CP) ./html/card.html $(DSTWWWDIR)/html/card.html
	@$(CP) ./html/bootstrap-x5.html $(DSTWWWDIR)/html/bootstrap.html
endif

ifeq ($(strip $(EN_APP_V9)),true)
web_app_html_install:
	@install -d $(DSTWWWDIR)/html
	@$(CP) ./html/bootstrap-v9.html $(DSTWWWDIR)/html/bootstrap.html
	@$(CP) ./html/boardcard.html $(DSTWWWDIR)/html/boardcard.html
	@$(CP) ./html/general.html $(DSTWWWDIR)/html/general.html	
	@$(CP) ./html/rtsp.html $(DSTWWWDIR)/html/rtsp.html	
	@$(CP) ./html/algorithm.html $(DSTWWWDIR)/html/algorithm.html	
	@$(CP) ./html/facelib.html $(DSTWWWDIR)/html/facelib.html	
	#$(CP) ./html/upload.html $(DSTWWWDIR)/html/upload.html						
endif
#
web_html_install:
	@install -d $(DSTWWWDIR)/html

	@$(CP) ./html/systemview.html $(DSTWWWDIR)/html/systemview.html
	@$(CP) ./html/system.html $(DSTWWWDIR)/html/system.html	
	@$(CP) ./html/about.html $(DSTWWWDIR)/html/about.html
	#$(CP) ./html/admin.html $(DSTWWWDIR)/html/admin.html
	#$(CP) ./html/dhcp.html $(DSTWWWDIR)/html/dhcp.html		
	#$(CP) ./html/dns.html $(DSTWWWDIR)/html/dns.html
	@$(CP) ./html/filesystem.html $(DSTWWWDIR)/html/filesystem.html
	#$(CP) ./html/firewall.html $(DSTWWWDIR)/html/firewall.html	
	#$(CP) ./html/nat.html $(DSTWWWDIR)/html/nat.html		
	#$(CP) ./html/netstatus.html $(DSTWWWDIR)/html/netstatus.html
	@$(CP) ./html/network.html $(DSTWWWDIR)/html/network.html
	#$(CP) ./html/route.html $(DSTWWWDIR)/html/route.html
	#$(CP) ./html/status.html $(DSTWWWDIR)/html/status.html		
	#$(CP) ./html/switch.html $(DSTWWWDIR)/html/switch.html
	@$(CP) ./html/syslog.html $(DSTWWWDIR)/html/syslog.html
			
	@$(CP) ./html/login.html $(DSTWWWDIR)/html/login.html
	@$(CP) ./html/reboot.html $(DSTWWWDIR)/html/reboot.html



#	
	
#
#	
webinstall: 
	@install -d $(DSTWWWDIR)
	@$(CP) ./html/index.html $(DSTWWWDIR)/index.html	


webclean:
	@$(RM) -rf $(DSTWWWDIR)/*
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
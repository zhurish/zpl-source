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
	install -d $(DSTWWWDIR)/js
	$(CP) -arf ./js/* $(DSTWWWDIR)/js/
	
#
#
web_cgi_install: 
	install -d $(DSTWWWDIR)/cgi-bin
	#install -m 755 ./cgi-bin/test.cgi $(DSTWWWDIR)/cgi-bin
#
#	
web_css_install:
	install -d $(DSTWWWDIR)/css
	$(CP) ./cascade.css $(DSTWWWDIR)/css/cascade.css
#
#	
web_res_install:
	install -d $(DSTWWWDIR)/res
	$(CP) -arf ./res/* $(DSTWWWDIR)/res/
	$(CP) -arf ./logo.png $(DSTWWWDIR)/res/
	$(CP) -arf ./favicon.ico $(DSTWWWDIR)/res/	
#
#
web_fonts_install:
	install -d $(DSTWWWDIR)/fonts
	$(CP) -arf ./fonts/* $(DSTWWWDIR)/fonts/
#
#
#				
#
#
web_html_install:
	install -d $(DSTWWWDIR)/html
	$(CP) ./html/login.html $(DSTWWWDIR)/html/login.html
	#$(CP) ./html/upload.html $(DSTWWWDIR)/html/upload.html
	#$(CP) ./html/test.html $(DSTWWWDIR)/html/test.html
	#$(CP) ./html/login.html $(DSTWWWDIR)/html/login.html		
	#$(CP) .//html/main.html $(DSTWWWDIR)/html/main.html
	#$(CP) ./html/upload.html $(DSTWWWDIR)/html/upload.html
	#$(CP) ./html/overview.html $(DSTWWWDIR)/html/overview.html
	#$(CP) ./html/version.html $(DSTWWWDIR)/html/version.html	
	#$(CP) ./html/menu.html $(DSTWWWDIR)/html/menu.html
	#$(CP) ./html/header.html $(DSTWWWDIR)/html/header.html	
	#$(CP) ./html/footer.html $(DSTWWWDIR)/html/footer.html	
	#$(CP) ./html/*.html $(DSTWWWDIR)/html/

#	
	
#
#	
webinstall: 
	install -d $(DSTWWWDIR)
	$(CP) ./html/index.html $(DSTWWWDIR)/index.html	


webclean:
	$(RM) -rf $(DSTWWWDIR)/*
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
include $(MAKE_DIR)/makelinux.mk

#
#
THEME_MATERIAL=true
#
#
#
#
#
#
HTML_INSTALL = ./html/html-install.sh
#
# css js res res/cbi res/icons asp cgi-bin html
#
#
material_theme_install: 
	$(CP) ./theme/material/cascade.css $(DSTWWWDIR)/css/cascade.css
	$(CP) ./theme/material/favicon.ico $(DSTWWWDIR)/res/
	$(CP) ./theme/material/logo.png $(DSTWWWDIR)/res/
	$(CP) ./theme/material/js/* $(DSTWWWDIR)/js/	
	$(CP) ./theme/material/html/login.html $(DSTWWWDIR)/html/login.html		
	$(CP) ./theme/material/html/main.html $(DSTWWWDIR)/html/main.html

	$(CP) ./theme/material/html/upload.html $(DSTWWWDIR)/html/upload.html
	$(CP) ./theme/material/html/overview.html $(DSTWWWDIR)/html/overview.html
	$(CP) ./theme/material/html/version.html $(DSTWWWDIR)/html/version.html	
	
	$(CP) ./theme/material/html/menu.html $(DSTWWWDIR)/html/menu.html
	$(CP) ./theme/material/html/header.html $(DSTWWWDIR)/html/header.html	
	$(CP) ./theme/material/html/footer.html $(DSTWWWDIR)/html/footer.html	
	$(CP) ./theme/material/html/*.html $(DSTWWWDIR)/html/				
#
#
web_cgi_install: 
	install -d $(DSTWWWDIR)/cgi-bin
	install -m 755 ./cgi-bin/test.cgi $(DSTWWWDIR)/cgi-bin
#
#	
web_css_install:
	install -d $(DSTWWWDIR)/css

#	
web_res_install:
	install -d $(DSTWWWDIR)/res
	$(CP) -arf ./res/* $(DSTWWWDIR)/res/

#
web_html_install:
	install -d $(DSTWWWDIR)/html
	$(CP) ./html/login.html $(DSTWWWDIR)/html/login.html
	#$(CP) ./html/main.html $(DSTWWWDIR)/html/main.html
	$(CP) ./html/upload.html $(DSTWWWDIR)/html/upload.html
	#./html/html-install.sh main.html $(DSTWWWDIR)/html/main.html
	#$(HTML_INSTALL) ./html/main.html $(DSTWWWDIR)/html/main.html
	$(CP) ./html/test.html $(DSTWWWDIR)/html/test.html
	#$(CP) ./html/testt.html $(DSTWWWDIR)/html/main.html
	
#	
web_js_install:
	install -d $(DSTWWWDIR)/js
	$(CP) -arf ./js/* $(DSTWWWDIR)/js/
	
#
#	
web_theme_install: material_theme_install

	
#
#	
webinstall: 
	install -d $(DSTETCDIR)/web
	$(CP) ./auth.txt $(DSTETCDIR)/web/auth.txt
	$(CP) ./route.txt $(DSTETCDIR)/web/route.txt
	
	install -d $(DSTWWWDIR)
	$(CP) ./html/index.html $(DSTWWWDIR)/index.html	


webclean: 
	$(RM) $(DSTETCDIR)/web/auth.txt
	$(RM) $(DSTETCDIR)/web/route.txt
	$(RM) -rf $(DSTWWWDIR)/*
#
#
install: webinstall web_cgi_install web_css_install web_res_install web_html_install \
			web_js_install web_theme_install
#
#
#
clean: webclean
#
#
#
.PHONY:	install clean webclean webinstall web_cgi_install web_css_install \
		web_res_install web_html_install web_js_install web_theme_install
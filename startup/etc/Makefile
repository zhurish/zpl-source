include $(MAKE_DIR)/makelinux.mk


lib:install
#
#
#
obj:install
#
#
config_install:
	install -d ${DSTETCDIR} 
	install -m 755 openconfig ${DSTETCDIR}
	install -m 755 product ${DSTETCDIR}
	install -m 755 voipconfig ${DSTETCDIR}	
#
#	
config_clean:
	@$(RM) $(DSTETCDIR)/voipconfig
	@$(RM) $(DSTETCDIR)/product
	@$(RM) $(DSTETCDIR)/openconfig	
#
#
#
install: config_install
	install -d ${DSTETCDIR} 
	install -m 755 plat.conf ${DSTETCDIR}
	install -m 755 default-config.cfg ${DSTETCDIR}

clean: config_clean
	@$(RM) $(DSTETCDIR)/plat.conf
	@$(RM) $(DSTETCDIR)/default-config.cfg

objclean: 

all: lib install

.PHONY:	obj all lib install objclean clean config_clean config_install

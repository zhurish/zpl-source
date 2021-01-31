include $(MAKE_DIR)/makelinux.mk


lib:install
#
#
#
obj:install
#

make_prepare:
#
config_install:
	@install -d ${DSTETCDIR} 
	@install -m 755 product ${DSTETCDIR}
ifeq ($(PL_APP_X5_MODULE),true)
	@install -m 755 openconfig ${DSTETCDIR}
	@install -m 755 voipconfig ${DSTETCDIR}	
endif	
#
#	
config_clean:
	@$(RM) $(DSTETCDIR)/product
ifeq ($(PL_APP_X5_MODULE),true)
	@$(RM) $(DSTETCDIR)/voipconfig
	@$(RM) $(DSTETCDIR)/openconfig	
endif	
#
#
#
install: config_install
	@install -d ${DSTETCDIR} 
	@install -m 755 plat.conf ${DSTETCDIR}
	@install -m 755 default-config.cfg ${DSTETCDIR}
	@install -m 755 nginx.conf ${DSTETCDIR}
	
clean: config_clean
	@$(RM) $(DSTETCDIR)/plat.conf
	@$(RM) $(DSTETCDIR)/default-config.cfg
	@$(RM) $(DSTETCDIR)/nginx.conf
	
objclean: 

all: lib install

.PHONY:	obj all lib install objclean clean config_clean config_install

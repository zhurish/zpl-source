include $(MAKE_DIR)/makelinux.mk


lib:install
#
#
#
obj:install
#
install:
	install -d ${DSTETCDIR} 
	install -m 755 plat.conf ${DSTETCDIR}
	install -m 755 default-config.cfg ${DSTETCDIR}

clean:
	@$(RM) $(DSTETCDIR)/plat.conf
	@$(RM) $(DSTETCDIR)/default-config.cfg

objclean: 

all: lib install

.PHONY:	obj all lib install objclean clean

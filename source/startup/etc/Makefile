include $(ZPL_MAKE_DIR)/makelinux.mk


lib:install
#
#
#
obj:install
#

make_prepare:

#
#
#
#
install: 
	@install -d ${ZPL_INSTALL_ETC_DIR} 
	@install -m 755 plat.conf ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 default-config.cfg ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 nginx.conf ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 os_netservice.cfg ${ZPL_INSTALL_ETC_DIR}

clean: 
	@if test -f $(ZPL_INSTALL_ETC_DIR)/plat.conf ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/plat.conf ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/default-config.cfg ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/default-config.cfg ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/nginx.conf ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/nginx.conf ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/os_netservice.cfg ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/os_netservice.cfg ; \
	fi

objclean: 
	@if test -f $(ZPL_INSTALL_ETC_DIR)/plat.conf ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/plat.conf ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/default-config.cfg ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/default-config.cfg ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/nginx.conf ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/nginx.conf ; \
	fi
	@if test -f $(ZPL_INSTALL_ETC_DIR)/os_netservice.cfg ; \
		then \
		$(RM) $(ZPL_INSTALL_ETC_DIR)/os_netservice.cfg ; \
	fi

all: lib install

.PHONY:	obj all lib install objclean clean config_clean config_install make_prepare

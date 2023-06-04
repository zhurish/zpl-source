include $(ZPL_MAKE_DIR)/makelinux.mk


lib:install
#
#
#
obj:install
#

make_prepare:

#
config_install:
	@install -d ${ZPL_INSTALL_ETC_DIR} 
ifeq ($(ZPL_APP_X5_MODULE),true)
	@install -m 755 openconfig ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 voipconfig ${ZPL_INSTALL_ETC_DIR}	
endif	
#
#	
config_clean:
ifeq ($(ZPL_APP_X5_MODULE),true)
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/voipconfig
	@$(RM) $(ZPL_INSTALL_ETC_DIR)/openconfig	
endif	
#
#
#
install: config_install
	@install -d ${ZPL_INSTALL_ETC_DIR} 
	@install -m 755 plat.conf ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 default-config.cfg ${ZPL_INSTALL_ETC_DIR}
	@install -m 755 nginx.conf ${ZPL_INSTALL_ETC_DIR}
	
clean: config_clean
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

all: lib install

.PHONY:	obj all lib install objclean clean config_clean config_install make_prepare

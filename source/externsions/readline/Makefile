include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

LIB_DIR = $(BASE_ROOT)/$(ZPL_ULIB_DIR)
INSTALL_DIR=$(CURDIR)

lib: 
	@if test ! -f ncurses-6.2/Makefile ; \
		then \
		$(CHMOD) +x build.sh ncurses-6.2/configure; \
		$(SHELL) build.sh lib arm-linux $(CC) $(INSTALL_DIR); \
	fi
	@if test ! -f readline-8.1/Makefile ; \
		then \
		$(CHMOD) +x buildrl.sh readline-8.1/configure; \
		$(SHELL) buildrl.sh lib arm-linux $(CC) $(INSTALL_DIR); \
	fi


install: $(LIBS)
	$(SHELL) build.sh install
	$(SHELL) buildrl.sh install
	@install -d $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libhistory.so $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libreadline.so $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libncurses.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libmenu.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libform.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libpanel.a $(LIB_DIR)


clean: 
	$(SHELL) build.sh clean
	$(RM) readline-8.1/Makefile

all: lib

.PHONY:	 all lib install clean 

include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk



RUNHOST = $(ZPL_RUN_ARCH)-$(ZPL_RUN_OS)


LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)
INSTALL_DIR=$(CURDIR)

lib: 
	@if test ! -f libnl-3.6.0/Makefile ; \
		then \
		$(CHMOD) +x build.sh libnl-3.6.0/configure; \
		$(SHELL) build.sh lib $(RUNHOST) $(CC) $(INSTALL_DIR); \
	fi


install: $(LIBS)
	$(SHELL) build.sh install
	@install -d $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-3.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-genl-3.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-idiag-3.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-nf-3.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-route-3.a $(LIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib/libnl-xfrm-3.a $(LIB_DIR)


clean: 
	$(SHELL) build.sh clean

all: lib

.PHONY:	 all lib install clean 

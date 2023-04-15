include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk



RUNHOST = $(ZPL_RUN_ARCH)-$(ZPL_RUN_OS)


INSTALL_DIR=$(CURDIR)
#ZPL_INSTALL_ROOTFS_DIR ZPL_INSTALL_ULIB_DIR

lib: 
	$(SHELL) build.sh lib $(RUNHOST) $(CC) $(INSTALL_DIR); 


install: $(LIBS)
	$(SHELL) build.sh install
	@install -d $(ZPL_INSTALL_ULIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib64/libssl* $(ZPL_INSTALL_ULIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/lib64/libcrypto* $(ZPL_INSTALL_ULIB_DIR)

clean: 
	$(SHELL) build.sh clean

all: lib

.PHONY:	 all lib install clean 

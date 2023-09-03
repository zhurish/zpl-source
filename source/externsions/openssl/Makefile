include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk



RUNHOST = $(ZPL_RUN_ARCH)-$(ZPL_RUN_OS)


INSTALL_DIR=$(CURDIR)
#ZPL_INSTALL_ROOTFS_DIR ZPL_INSTALL_ULIB_DIR

OPENSSL_SRC_DIR=$(ZPLBASE)/$(OPENSSL_ROOT)/openssl-3.1.0-alpha1


lib: 
	@if test ! -f $(OPENSSL_SRC_DIR)/Makefile ; \
		then \
		$(CHMOD) +x $(OPENSSL_SRC_DIR)/Configure; \
		$(CHMOD) +x $(OPENSSL_SRC_DIR)/config; \
		cd $(OPENSSL_SRC_DIR); ./config shared --prefix=${INSTALL_DIR}/_install no-threads CC="gcc" CXX="g++" AR="ar" RANLIB="ranlib" ${OPENSSL_OS}; \
	fi
	$(MAKE) -C $(OPENSSL_SRC_DIR) build_sw
#
#
libm: 
	$(SHELL) build.sh lib $(RUNHOST) $(CC) $(INSTALL_DIR); 

ifeq ($(ZPL_BUILD_ARCH),X86_64)
OLIBDIR=lib64
else
OLIBDIR=lib
endif

install: $(LIBS)
	$(MAKE) -C $(OPENSSL_SRC_DIR) install_sw 
	$(MAKE) -C $(OPENSSL_SRC_DIR) install_ssldirs
	@install -d $(ZPL_INSTALL_ULIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/$(OLIBDIR)/libssl* $(ZPL_INSTALL_ULIB_DIR)
	@install -m 755 $(INSTALL_DIR)/_install/$(OLIBDIR)/libcrypto* $(ZPL_INSTALL_ULIB_DIR)

clean: 
	$(MAKE) -C $(OPENSSL_SRC_DIR) clean

all: lib

.PHONY:	 all lib install clean 

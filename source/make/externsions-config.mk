#include $(ZPL_MAKE_DIR)/module-dir.mk

ifeq ($(strip $(ZPL_EXTERNSIONS_MODULE)),true)

ifeq ($(strip $(ZPL_ZLIB_MODULE)),true)
ZPLEX_DEFINE	+= -DZPL_ZLIB_MODULE

ifneq ($(ZPL_BUILD_ARCH),X86_64)
ifeq ($(strip $(ZPL_EXZLIB_MODULE)),true)
ZPLEX_DEFINE	+= -DZPL_ZLIB_MODULE
ZPLEX_INCLUDE += -I$(EXTERNSION_BASE_ROOT)/zlib/_install/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_BASE_ROOT)/zlib/_install/lib
ZPLOS_LDLIBS += -lz
else
ZLIB_ROOT=$(EXTERNSION_DIR)/zlib
ZPLEX_DIR += $(ZPLBASE)/$(ZLIB_ROOT)
ZLIB_SRC = $(ZPLBASE)/$(ZLIB_ROOT)/zlib-1.2.11
ZPLEX_INCLUDE += -I$(ZLIB_SRC)
endif #($(strip $(ZPL_EXZLIB_MODULE)),true)
else
ZPLOS_LDLIBS += -lz
endif #($(ZPL_BUILD_ARCH),X86_64)
endif

#
# Externsion openssl
#
ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
OPENSSL_ROOT=$(EXTERNSION_DIR)/openssl
ifneq ($(ZPL_BUILD_ARCH),X86_64)
ZPLEX_DIR += $(ZPLBASE)/$(OPENSSL_ROOT)
export OPENSSL_OS=linux-armv4
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(OPENSSL_ROOT)/_install/include
ZPLEX_LDLIBS += -lutil -lssl -lcrypto
else 
ZPLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),X86_64)
ZPL_DEFINE += -DZPL_OPENSSL_MODULE
else
ZPL_DEFINE += -DNO_OPENSSL=1
endif #($(strip $(ZPL_OPENSSL_MODULE)),true)


ifeq ($(strip $(ZPL_MBEDTLS_MODULE)),true)
MBEDTLS_ROOT=$(EXTERNSION_DIR)/mbedtls-3.4.0
ZPLPRODS_LAST += $(ZPLBASE)/$(MBEDTLS_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(MBEDTLS_ROOT)/include
#ZPLEX_LDLIBS += -lmbedcrypto -lmbedx509 -lmbedtls
ZPL_DEFINE += -DZPL_MBEDTLS_MODULE
endif #($(strip $(ZPL_MBEDTLS_MODULE)),true)

ifeq ($(strip $(ZPL_OPENSSH_MODULE)),true)
OPENSSH_ROOT=$(EXTERNSION_DIR)/openssh
ifneq ($(ZPL_BUILD_ARCH),X86_64)
ZPLEX_DIR += $(ZPLBASE)/$(OPENSSH_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(OPENSSH_ROOT)/_install/include
#ZPLEX_LDLIBS += -lssh
else 
#ZPLOS_LDLIBS += -lssh
endif #($(ZPL_BUILD_ARCH),X86_64)
ZPL_DEFINE += -DZPL_OPENSSH_MODULE
else
endif #($(strip $(ZPL_OPENSSL_MZPL_OPENSSH_MODULEODULE)),true)


ifeq ($(strip $(ZPL_READLINE_MODULE)),true)
ZPLEX_DEFINE	+= -DZPL_READLINE_MODULE
ifneq ($(strip $(ZPL_BUILD_ARCH)),$(filter $(ZPL_BUILD_ARCH),X86_64 X86))
READLINE_ROOT=$(EXTERNSION_DIR)/readline
ZPLEX_DIR += $(ZPLBASE)/$(READLINE_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(READLINE_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(READLINE_ROOT)/_install/include
ZPLEX_LDFLAGS += -L$(ZPLBASE)/$(READLINE_ROOT)/_install/lib
endif
ZPLEX_LDLIBS += -lreadline -lhistory -lncurses
endif

ifeq ($(strip $(ZPL_LIBNL_MODULE)),true)
#ZPLEX_DEFINE	+= -DZPL_LIBNL_MODULE
LIBNL_ROOT=$(EXTERNSION_DIR)/libnl
ZPLEX_DIR += $(ZPLBASE)/$(LIBNL_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBNL_ROOT)
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBNL_ROOT)/_install/include/libnl3
ZPLEX_LDFLAGS += -L$(ZPLBASE)/$(LIBNL_ROOT)/_install/lib
#ZPLEX_LDLIBS += -lnl-3 -lnl-genl-3 -lnl-route-3 -lnl-idiag-3 -lnl-nf -lnl-xfrm
endif

ifeq ($(strip $(ZPL_FREETYPE_MODULE)),true)
LIBMFREETYPE_ROOT=$(EXTERNSION_DIR)/freetype-2.10.0
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMFREETYPE_ROOT)/include
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMFREETYPE_ROOT)/include/freetype/config
ZPL_DEFINE += -DZPL_FREETYPE_MODULE
ifeq ($(strip $(ZPL_EXFREETYPE_MODULE)),true)
ZPLEX_INCLUDE += -I$(ZPL_EXFREETYPE_INC_PATH)
ZPLEX_LDFLAGS += -L$(ZPL_EXFREETYPE_LIB_PATH)
ZPLEX_LDLIBS += -lfreetype
else
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBMFREETYPE_ROOT)
endif

endif



endif


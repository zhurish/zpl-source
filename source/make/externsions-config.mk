#include $(ZPL_MAKE_DIR)/module-dir.mk

ifeq ($(strip $(ZPL_EXTERNSIONS_MODULE)),true)

#
# Externsion openssl
#
ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
ifneq ($(ZPL_BUILD_ARCH),X86_64)
ZPLEX_DIR += $(EXTERNSION_ROOT)/openssl/openssl-1.1.1/
export PLATFORM=linux-armv4
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include
ZPLEX_LDFLAGS += -L$(ZPL_INSTALL_ROOTFS_DIR)/lib
ZPLEX_LDLIBS += -lutil -lssl -lcrypto
ZPLEX_DIR += $(EXTERNSION_ROOT)/zlib/zlib-1.2.11/
ZPL_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include
ZPLEX_LDFLAGS += -L$(ZPL_INSTALL_ROOTFS_DIR)/lib
ZPLEX_LDLIBS += -lz
else 
ZPLOS_LDLIBS += -lutil -lssl -lcrypto -lz
endif #($(ZPL_BUILD_ARCH),X86_64)
ZPL_DEFINE += -DZPL_OPENSSL_MODULE
endif #($(strip $(ZPL_OPENSSL_MODULE)),true)


ifeq ($(strip $(ZPL_READLINE_MODULE)),true)
ZPLEX_DEFINE	+= -DZPL_READLINE_MODULE
ifneq ($(strip $(ZPL_BUILD_ARCH)),$(filter $(ZPL_BUILD_ARCH),X86_64 X86))
ZPLEX_DIR += $(EXTERNSION_ROOT)/readline
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/readline
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/readline/_install/include
ZPLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/readline/_install/lib
endif
ZPLEX_LDLIBS += -lreadline -lhistory -lncurses
endif

ifeq ($(strip $(ZPL_LIBNL_MODULE)),true)
ZPLEX_DEFINE	+= -DZPL_LIBNL_MODULE
ZPLEX_DIR += $(EXTERNSION_ROOT)/libnl
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/libnl
ZPLEX_INCLUDE += -I$(EXTERNSION_ROOT)/libnl/_install/include/libnl3
ZPLEX_LDFLAGS += -L$(EXTERNSION_ROOT)/libnl/_install/lib
#ZPLEX_LDLIBS += -lnl-3 -lnl-genl-3 -lnl-route-3 -lnl-idiag-3 -lnl-nf -lnl-xfrm
endif

ifeq ($(strip $(ZPL_EXFREETYPE_MODULE)),true)
LIBMFREETYPE_ROOT=$(EXTERNSION_ROOT)/freetype-2.10.0
ZPLEX_DIR += $(EXTERNSION_ROOT)/freetype-2.10.0
ZPLEX_INCLUDE += -I$(ZPL_EXFREETYPE_LIB_PATH)/
ZPLEX_LDFLAGS += -L$(ZPL_EXFREETYPE_INC_PATH)/
ZPL_DEFINE += -DZPL_FREETYPE_MODULE -DZPL_EXFREETYPE_MODULE
endif

endif


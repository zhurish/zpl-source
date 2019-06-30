#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/webgui
#
#PLINCLUDE += -I$(WEBGUI_ROOT)/include
#
#
ME_COM_COMPILER       = 1
ME_COM_LIB            = 1
ME_COM_MATRIXSSL      = 0
ME_COM_MBEDTLS        = 0
ME_COM_NANOSSL        = 0
ME_COM_OPENSSL        = 0
ME_COM_OSDEP          = 0
ME_COM_SSL            = 0
ME_COM_VXWORKS        = 0
#
#
ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER = 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL = 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL = 1
endif
#
ME_DFLAGS = -DME_DEBUG=1 -D_REENTRANT -DPIC -DME_COM_COMPILER=$(ME_COM_COMPILER) \
			-DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) \
			-DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) \
			-DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) \
			-DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) \
			-D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\"
#OS

OBJS += action.o
OBJS += alloc.o
OBJS += auth.o
OBJS += cgi.o
OBJS += file.o
OBJS += fs.o
OBJS += http.o
OBJS += js.o
OBJS += jst.o
OBJS += options.o
OBJS += osdep.o
OBJS += rom.o
OBJS += route.o
OBJS += runtime.o
OBJS += socket.o
OBJS += time.o
OBJS += upload.o

OBJS += crypt.o

ifeq ($(ME_COM_MBEDTLS),1)
mbedtls_OBJS += mbedtls.o #src/mbedtls/mbedtls.c
endif
ifeq ($(ME_COM_MBEDTLS),1)
goahead_mbedtls_OBJS += goahead-mbedtls.o #goahead-mbedtls/goahead-mbedtls.c
endif
ifeq ($(ME_COM_OPENSSL),1)
goahead_openssl_OBJS += goahead-openssl.o #goahead-openssl/goahead-openssl.c
endif

OBJS += goahead.o
#gopass_OBJS += gopass.o #src/utils/gopass.c

WEBOBJS += webgui_app.o
#
APPOBJS += web_tool_html.o
APPOBJS += web_tool_jst.o
APPOBJS += web_tool_onclick.o

APPOBJS += web_login_html.o
APPOBJS += web_admin_html.o
APPOBJS += web_updownload_html.o
#############################################################################
# LIB
###########################################################################
LIBS = libgohead.a

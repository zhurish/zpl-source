#############################################################################
# DEFINE
###########################################################################
#

#
#vpath %.c src
#ZPLINCLUDE += -I$(WEBGUI_ROOT)/include
#
#
ME_COM_COMPILER       = 1
ME_COM_LIB            = 1
ifeq ($(ZPL_WEBSERVER_MATRIXSSL),true)
ME_COM_MATRIXSSL      = 1
else
ME_COM_MATRIXSSL      = 0
endif
ifeq ($(ZPL_WEBSERVER_MBEDTLS),true)
ME_COM_MBEDTLS        = 1
else
ME_COM_MBEDTLS        = 0
endif
ifeq ($(ZPL_WEBSERVER_NANOSSL),true)
ME_COM_NANOSSL        = 1
else
ME_COM_NANOSSL        = 0
endif
ifeq ($(ZPL_WEBSERVER_OPENSSL),true)
ME_COM_OPENSSL        = 1
else
ME_COM_OPENSSL        = 0
endif
ME_COM_OSDEP          = 0
#ifeq ($(ME_COM_OPENSSL),1)
#ME_COM_SSL            = 1
#else
#ME_COM_SSL            = 0
#endif
#登陆认证采用js交互，不然就使用route配置信息完成页面跳转
ME_GOAHEAD_LOGIN_JS   = 1
#
ifeq ($(strip $(ZPL_OS_JSON)),true)
ME_GOAHEAD_JSON   = 1	
else
ME_GOAHEAD_JSON   = 0
endif
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
ifeq ($(ZPL_WEBSERVER_NONESSL),true)
    ME_COM_SSL = 0
endif

#
ME_DFLAGS = -D_REENTRANT -DPIC -DME_COM_COMPILER=$(ME_COM_COMPILER) \
			-DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) \
			-DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) \
			-DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) \
			-DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) \
			-DME_GOAHEAD_LOGIN_JS=$(ME_GOAHEAD_LOGIN_JS) \
			-DME_GOAHEAD_JSON=$(ME_GOAHEAD_JSON) \
			-D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 \
			-DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\"
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
OBJS += webutil.o
OBJS += webutton.o

ifeq ($(ME_COM_MBEDTLS),1)
mbedtls_OBJS += mbedtls.o #src/mbedtls/mbedtls.c
endif
ifeq ($(ME_COM_MBEDTLS),1)
goahead_mbedtls_OBJS += goahead-mbedtls.o #goahead-mbedtls/goahead-mbedtls.c
endif
ifeq ($(ME_COM_OPENSSL),1)
goahead_openssl_OBJS += goahead-openssl.o #goahead-openssl/goahead-openssl.c
endif

#OBJS += goahead.o
#OBJS += gopass.o
gopass_util += gopass-util.o 
gopass_OBJS += gopass.o  gopass-util.o 
#src/utils/gopass.c
WEBOBJS += web_app.o
WEBOBJS += web_api.o

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
WEBOBJS += cmd_web.o
endif

ifeq ($(strip $(ZPL_WEBAPP_MODULE)),true)
#jst
APPJSTOBJS += web_html_jst.o
APPJSTOBJS += web_system_jst.o

#form and action

APPWEBOBJS += web_menu_html.o
APPWEBOBJS += web_admin_html.o
APPWEBOBJS += web_sntp_html.o
APPWEBOBJS += web_syslog_html.o
APPWEBOBJS += web_network_html.o
APPWEBOBJS += web_wireless_html.o
APPWEBOBJS += web_updownload_html.o
APPWEBOBJS += web_system_html.o
APPWEBOBJS += web_upgrade_html.o
APPWEBOBJS += web_netservice_html.o

ifeq ($(strip $(ZPL_APP_X5_MODULE)),true)
APPWEBOBJS += web_switch_html.o
APPWEBOBJS += web_sip_html.o
APPWEBOBJS += web_factory_html.o
APPWEBOBJS += web_card_html.o
endif
ifeq ($(strip $(ZPL_APP_V9_MODULE)),true)
APPWEBOBJS += web_boardcard_html.o
APPWEBOBJS += web_general_html.o
APPWEBOBJS += web_rtsp_html.o
APPWEBOBJS += web_algorithm_html.o
APPWEBOBJS += web_facelib_html.o
APPWEBOBJS += web_db_html.o
endif
endif

WEBBIN = web-gopass
#############################################################################
# LIB
###########################################################################
LIBS = libgoahead.a

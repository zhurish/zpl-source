#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/pjsip
#
PLINCLUDE += -I$(PJSIP_ROOT)/include
#
#
#OS
OBJS += pjsua_app_legacy.o
OBJS += pjsua_app_config.o
OBJS += pjsua_app_common.o
OBJS += pjsua_app_cli.o
OBJS += pjsua_app.o
OBJS += pjsip_main.o

#############################################################################
# LIB
###########################################################################
LIBS = libpjsipvoip.a

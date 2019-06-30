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
#OBJS += pjsua_app_legacy.o
OBJS += pjsua_app_config.o
OBJS += pjsua_app_common.o
OBJS += pjsua_app_cb.o
OBJS += pjsua_app_cli.o
OBJS += pjsua_app.o
OBJS += pjsip_main.o
OBJS += pjsip_app_api.o

OBJS += voip_volume.o
ifeq ($(strip $(MODULE_APP)),true)
OBJS += voip_util.o
OBJS += voip_uci.o
#OBJS += voip_state.o
OBJS += voip_log.o
OBJS += voip_app.o
endif

TOBJS += auddemo.o
#############################################################################
# LIB
###########################################################################
LIBS = libpjsipvoip.a

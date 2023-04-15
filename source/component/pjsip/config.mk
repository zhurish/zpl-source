#############################################################################
# DEFINE
###########################################################################
#

#
#ZPLINCLUDE += -I$(PJSIP_ROOT)/include
#
#
#OS
OBJS += pjsip_util.o
OBJS += pjsua_app_config.o
OBJS += pjsua_app_common.o
OBJS += pjsua_app_cb.o
OBJS += pjsua_app_cli.o
OBJS += pjsua_app.o
OBJS += pjsip_buddy.o
OBJS += pjsip_jsoncfg.o
OBJS += pjsip_cfg.o
OBJS += pjsip_main.o
OBJS += pjsip_app_api.o

ifeq ($(strip $(ZPL_APPLICATION_MODULE)),true)
OBJS += voip_volume.o
OBJS += voip_util.o
OBJS += voip_uci.o
OBJS += voip_log.o
OBJS += voip_app.o
endif
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_pjsip.o
OBJS += cmd_voip_test.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libpjsipvoip.a


ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
PJSUA2OBJ := pjsipjsonconfig.o \
				pjsipaccount.o \
				pjsipcall.o \
				pjsipobserver.o \
				pjsipbuddy.o \
				pjsiptimer.o \
				pjsiplogwriter.o \
				pjsipapp.o \
				pjsipsampleobserver.o \
				pjsipsample.o \
				pjsipmain.o

PJSUA2TEST := pjsiptest.o
PJSUA2TEST_BIN:= pjsua2-app				

PJSUA2LIBS = libpjsipc++.a
endif
#############################################################################
# DEFINE
###########################################################################
#

#
#ZPLINCLUDE += -I$(PJSIP_ROOT)/include
#
#
#OS
OBJS += pjsua_app_config.o
OBJS += pjsua_app_common.o
OBJS += pjsua_app.o
OBJS += pjsua_app_cli.o
OBJS += pjsua_app_cb.o
OBJS += pjsua_app_cfgapi.o
OBJS += pjsua_app.o
OBJS += pjsua_app_api.o
OBJS += pjapp_app_util.o
OBJS += pjapp_stream.o
OBJS += pjapp_media_file.o

OBJS += pjsip_util.o
OBJS += pjsip_buddy.o
OBJS += pjsip_main.o


ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_pjsip.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libpjvoipc.a


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

PJSUA2LIBS = libpjvoipsua2.a
endif
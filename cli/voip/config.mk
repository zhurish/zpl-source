#############################################################################
# DEFINE
###########################################################################
MODULEDIR = cli/voip
#OS
ifeq ($(strip $(MODULE_OSIP)),true)
OBJS += cmd_voip.o
OBJS += cmd_sip.o
OBJS += cmd_voip_test.o
endif
ifeq ($(strip $(MODULE_PJSIP)),true)
OBJS += cmd_pjsip.o
OBJS += cmd_voip_test.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libclivoip.a

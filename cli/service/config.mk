#############################################################################
# DEFINE
###########################################################################
MODULEDIR = cli/service
#OS
OBJS	+= cmd_sntp.o
OBJS	+= cmd_systools.o

OBJS	+= cmd_ssh.o
ifeq ($(strip $(MODULE_WEB)),true)
OBJS	+= cmd_web.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libcliservice.a

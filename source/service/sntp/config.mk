#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/sntp
#OS
ifeq ($(strip $(ZPL_SERVICE_SNTPC)),true)
OBJS	+= sntpcLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_SNTPS)),true)
OBJS	+= sntpsLib.o
endif
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_sntp.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libsntp.a

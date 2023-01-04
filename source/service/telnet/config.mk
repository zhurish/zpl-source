#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/telnet
#OS

ifeq ($(strip $(ZPL_SERVICE_TELNET)),true)
OBJS	+= telnetcLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_TELNETD)),true)
OBJS	+= telnetdLib.o
endif

#############################################################################
# LIB
###########################################################################
LIBS = libtelnet.a
#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/systools
#OS
OBJS	+= systools.o
ifeq ($(strip $(ZPL_SERVICE_TFTPC)),true)
OBJS	+= tftpLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_TFTPD)),true)
OBJS	+= tftpdLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_FTPC)),true)
OBJS	+= ftpLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_FTPD)),true)
OBJS	+= ftpdLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_TELNET)),true)
OBJS	+= telnetcLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_TELNETD)),true)
OBJS	+= telnetdLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_PING)),true)
OBJS	+= pingLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_TRACEROUTE)),true)
OBJS	+= tracerouteLib.o
endif
ifeq ($(strip $(ZPL_SERVICE_UBUS_SYNC)),true)
OBJS	+= ubus_sync.o				
endif
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_systools.o
endif
#OBJS	+= bsdsignal.o
#OBJS	+= tftp.o
#OBJS	+= tftpsubs.o
#OBJS	+= main.o
#############################################################################
# LIB
###########################################################################
LIBS = libsystools.a

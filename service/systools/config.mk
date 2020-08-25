#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/systools
#OS
OBJS	+= systools.o
ifeq ($(strip $(MODULE_TFTPC)),true)
OBJS	+= tftpLib.o
endif
ifeq ($(strip $(MODULE_TFTPD)),true)
OBJS	+= tftpdLib.o
endif
ifeq ($(strip $(MODULE_FTPC)),true)
OBJS	+= ftpLib.o
endif
ifeq ($(strip $(MODULE_FTPD)),true)
OBJS	+= ftpdLib.o
endif
ifeq ($(strip $(MODULE_TELNET)),true)
OBJS	+= telnetcLib.o
endif
ifeq ($(strip $(MODULE_TELNETD)),true)
OBJS	+= telnetdLib.o
endif
ifeq ($(strip $(MODULE_PING)),true)
OBJS	+= pingLib.o
endif
ifeq ($(strip $(MODULE_TRACEROUTE)),true)
OBJS	+= tracerouteLib.o
endif
ifeq ($(strip $(MODULE_UBUS)),true)
OBJS	+= ubus_sync.o				
endif

#OBJS	+= bsdsignal.o
#OBJS	+= tftp.o
#OBJS	+= tftpsubs.o
#OBJS	+= main.o
#############################################################################
# LIB
###########################################################################
LIBS = libsystools.a

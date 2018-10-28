#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/systools
#OS
OBJS	+= systools.o

OBJS	+= tftpLib.o
OBJS	+= tftpdLib.o

OBJS	+= ftpLib.o
OBJS	+= ftpdLib.o


OBJS	+= telnetcLib.o

OBJS	+= pingLib.o
OBJS	+= tracerouteLib.o

#OBJS	+= bsdsignal.o
#OBJS	+= tftp.o
#OBJS	+= tftpsubs.o
#OBJS	+= main.o
#############################################################################
# LIB
###########################################################################
LIBS = libsystools.a

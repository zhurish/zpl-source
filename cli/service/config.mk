#############################################################################
# DEFINE
###########################################################################
MODULEDIR = cli/service
#OS
ifeq ($(strip $(ZPL_SERVICE_SNTPC)),true)
OBJS	+= cmd_sntp.o
endif

ifeq ($(strip $(ZPL_SERVICE_MODULE)),true)
OBJS	+= cmd_systools.o
endif

ifeq ($(strip $(ZPL_LIBSSH_MODULE)),true)
OBJS	+= cmd_ssh.o
endif

ifeq ($(strip $(ZPL_WEBSERVER_MODULE)),true)
OBJS	+= cmd_web.o
endif


#############################################################################
# LIB
###########################################################################
LIBS = libcliservice.a

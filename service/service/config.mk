#############################################################################
# DEFINE
###########################################################################
MODULEDIR = service/service
#OS
OBJS	+= service.o

ifeq ($(strip $(ZPL_SERVICE_UBUS_SYNC)),true)
OBJS	+= ubus_sync.o				
endif
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_service.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libservice.a

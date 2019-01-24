#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/voip
#
PLINCLUDE += -I$(VOIP_ROOT)/include
#
#
#OS
OBJS += voip_util.o
OBJS += voip_api.o
OBJS += voip_ring.o
OBJS += voip_task.o
OBJS += voip_event.o
OBJS += voip_statistics.o
OBJS += voip_volume.o
OBJS += voip_state.o
OBJS += voip_app.o
OBJS += voip_error.o
OBJS += voip_socket.o
OBJS += voip_stream.o
OBJS += voip_mediastream.o
OBJS += voip_dbtest.o
OBJS += voip_sip.o
ifeq ($(strip $(MODULE_EXSIP)),true)
OBJS += voip_sip_ctl.o
endif
ifeq ($(strip $(MODULE_OSIP)),true)
OBJS += voip_osip.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libvoip.a

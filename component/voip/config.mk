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
OBJS += voip_sip.o
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
#############################################################################
# LIB
###########################################################################
LIBS = libvoip.a

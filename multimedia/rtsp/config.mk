#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/rtsp

ifeq ($(strip $(ZPL_LIBRTSP_MODULE)),true)
endif
OBJS += 	\
			zpl_rtsp_base64.o \
			zpl_rtsp_client.o \
			zpl_rtsp_session.o \
			zpl_rtsp_server.o \
			zpl_rtsp_media.o \
			zpl_rtsp_socket.o \
			zpl_rtsp_sdp.o \
			zpl_rtsp_sdp_attr.o \
			zpl_rtsp_sdp_hdr.o \
			zpl_rtsp_util.o \
			zpl_rtsp_rtp.o \
			zpl_rtsp_transport.o \
			zpl_rtsp_sdpfmtp.o \
			zpl_rtsp_adap.o \
			zpl_rtsp_auth.o \
			zpl_rtsp_api.o
#OBJS +=list.o  
#############################################################################
# LIB
###########################################################################
LIBS = librtsp.a

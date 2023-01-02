#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/rtsp

ifeq ($(strip $(ZPL_LIBRTSP_MODULE)),true)
endif
ifeq ($(strip $(ZPL_LIVE555_MODULE)),true)
OBJS += DynamicRTSPServer.o \
			livertsp_server.o \
			livertsp_client.o \
			zpl_rtsp_api.o
#RTSPServerSupportingHTTPStreaming.o \			
else
OBJS += 	\
			zpl_rtsp_base64.o \
			zpl_rtsp_client.o \
			zpl_rtsp_session.o \
			zpl_rtsp_server.o \
			zpl_rtsp_media.o \
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
endif			
#OBJS +=list.o   			zpl_rtsp_socket.o \
#############################################################################
# LIB
###########################################################################
LIBS = librtsp.a

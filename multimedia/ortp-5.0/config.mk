#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/ortp-5.0

ifeq ($(strip $(ZPL_LIBORTP_MODULE)),true)

ZPLEX_DEFINE += -DHAVE_CONFIG_H -D_REENTRANT -DORTP_VERSION="1.0.1"
endif
OBJS += 	avprofile.o  \
			b64.o \
			congestiondetector.o \
			event.o \
			extremum.o \
			kalmanrls.o \
			jitterctl.o \
			logging.o \
			nack.o \
			netsim.o \
			ortp.o \
			payloadtype.o \
			port.o \
			posixtimer.o \
			rtcp.o \
			rtcp_fb.o \
			rtcp_xr.o \
			rtcpparse.o \
			rtpparse.o  \
			rtpprofile.o \
			rtpsession.o \
			rtpsession_inet.o \
			rtpsignaltable.o  \
			rtptimer.o	\
			scheduler.o \
			sessionset.o  \
			rtp_queue.o  \
			str_utils.o 	\
			telephonyevents.o  \
			videobandwidthestimator.o \
			utils.o \
			ortp_list.o \
			ortp_srtp.o \
			zrtp.o \
			stun.o \
			stun_udp.o \
			zpl_rtp_h264.o \
			zpl_rtp_h265.o \
			zpl_rtp_g7xx.o \
			rtpsend.o rtprecv.o rtpbundle.o 

#############################################################################
# LIB 		
###########################################################################
LIBS = libortp.a

#############################################################################
# DEFINE
###########################################################################


ifeq ($(strip $(ZPL_LIBORTP_MODULE)),true)

ZPLEX_DEFINE += -DHAVE_CONFIG_H -D_REENTRANT -DORTP_VERSION="1.0.1" -DORTP_TIMESTAMP 
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
			str_utils.o 	\
			telephonyevents.o  \
			videobandwidthestimator.o \
			rtpframemarking.o \
			utils.o \
			fecstream.o \
			rtpaudiolevel.o \
			ortp_list.o \
			dblk.o \
			rtpbundle.o vfs.o vfs_standard.o 

ifeq ($(strip $(ZPL_MBEDTLS_MODULE)),true)
OBJS += 	mbedtls.o mbedtls_cpp.o
endif
OBJS += 	utils_cpp.o regex.o exception.o
#############################################################################
# LIB 		
###########################################################################
LIBS = libortp.a

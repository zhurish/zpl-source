#############################################################################
# DEFINE
###########################################################################


#ZPLEX_DEFINE += -DCMAKE_INSTALL_DO_STRIP

OBJS += rtcpapppacket.o \
	rtcpbyepacket.o \
	rtcpcompoundpacket.o \
	rtcpcompoundpacketbuilder.o \
	rtcppacket.o \
	rtcppacketbuilder.o \
	rtcprrpacket.o \
	rtcpscheduler.o \
	rtcpsdesinfo.o \
	rtcpsdespacket.o \
	rtcpsrpacket.o \
	rtpabortdescriptors.o \
	rtpbyteaddress.o \
	rtpcollisionlist.o \
	rtpdebug.o \
	rtperrors.o \
	rtpexternaltransmitter.o \
	rtpfaketransmitter.o \
	rtpinternalsourcedata.o \
	rtpipv4address.o \
	rtpipv4destination.o \
	rtpipv6address.o \
	rtpipv6destination.o \
	rtplibraryversion.o \
	rtppacket.o \
	rtppacketbuilder.o \
	rtppollthread.o \
	rtprandom.o \
	rtprandomrand48.o \
	rtprandomrands.o \
	rtprandomurandom.o \
	rtpsecuresession.o \
	rtpsession.o \
	rtpsessionparams.o \
	rtpsessionsources.o \
	rtpsourcedata.o \
	rtpsources.o \
	rtptcpaddress.o \
	rtptcptransmitter.o \
	rtptimeutilities.o \
	rtpudpv4transmitter.o \
	rtpudpv6transmitter.o 

OBJS += jrtplib_api.o jrtp_avprofile.o jrtp_payloadtype.o  jrtp_rtpprofile.o
#############################################################################
# LIB 		
###########################################################################
LIBS = libjrtp.a

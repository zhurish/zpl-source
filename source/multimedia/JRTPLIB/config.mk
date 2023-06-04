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

#OBJS += h264-nal-packer.o h264-nal-unpacker.o h264-utils.o \
	h265-nal-packer.o h265-nal-unpacker.o h26x/h265-utils.o \
	h26x-utils.o h26x/nal-packer.o nal-unpacker.o rfc3984.o

#############################################################################
# LIB 		
###########################################################################
LIBS = libjrtp.a

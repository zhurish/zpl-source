#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/media-app


OBJS += H264UDPServerMediaSubsession.o
OBJS += BasicFdSource.o
OBJS += H264BasicFdServerMediaSubsession.o
OBJS += FramedQueue.o
OBJS += FramedQueueSource.o
OBJS += BasicQueueServerMediaSubsession.o

OBJS += rtsp_server.o
OBJS += rtsp_client.o

OBJS += v4l2_driver.o

OBJS += videoDevice.o
OBJS += h264Encoder.o
OBJS += h264Decoder.o

OBJS += ffmpegDevice.o
OBJS += ffmpegEncoder.o
OBJS += ffmpegSource.o

#############################################################################
# LIB
###########################################################################
LIBS = libmediaapp.a

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
OBJS += v4l2Device.o

ifeq ($(strip $(PL_OPENH264_MODULE)),true)
OBJS += h264Encoder.o
OBJS += h264Decoder.o
else ifeq ($(strip $(PL_LIBX264_MODULE)),true)
OBJS += h264Encoder.o
OBJS += h264Decoder.o
endif
ifeq ($(strip $(PL_LIBVPX_MODULE)),true)
OBJS += vpxEncoder.o
OBJS += vpxDecoder.o
endif
ifeq ($(strip $(PL_FFMPEG_MODULE)),true)
OBJS += ffmpegDevice.o
OBJS += ffmpegEncoder.o
OBJS += ffmpegSource.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libmediaapp.a

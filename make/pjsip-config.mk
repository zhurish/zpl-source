#include $(ZPL_MAKE_DIR)/module-dir.mk

ifeq ($(strip $(ZPL_PJPROJECT_MODULE)),true)
PJPROJECT_ROOT=$(MULTIMEDIA_ROOT)/pjproject-2.10
ZPLEX_DIR += $(MULTIMEDIA_ROOT)/pjproject-2.10

ifeq ($(ZPL_PJ_RESAMPLE_ENABLE),true)
export PJMEDIA_RESAMPLE_ENABLE = true
else
export PJMEDIA_RESAMPLE_ENABLE = false
endif
ifeq ($(ZPL_PJ_SIMPLE_ENABLE),true)
export PJMEDIA_SIMPLE_ENABLE = true
else
export PJMEDIA_SIMPLE_ENABLE = false
endif
ifeq ($(ZPL_PJ_SRTP_ENABLE),true)
export PJMEDIA_SRTP_ENABLE = true
else
export PJMEDIA_SRTP_ENABLE = false
endif
ifeq ($(ZPL_PJ_VIDEO_ENABLE),true)
export PJMEDIA_VIDEODEV_ENABLE = true
else
export PJMEDIA_VIDEODEV_ENABLE = false
endif
ifeq ($(ZPL_PJ_VIDEO_YUV_ENABLE),true)
export PJMEDIA_YUV_ENABLE = true
else
export PJMEDIA_YUV_ENABLE = false
endif
ifeq ($(ZPL_PJ_VIDEO_H264_ENABLE),true)
export PJMEDIA_H264_ENABLE = true
else
export PJMEDIA_H264_ENABLE = false
endif
ifeq ($(ZPL_PJ_CODEC_GSM_ENABLE),true)
export PJMEDIA_GSM_ENABLE = true
else
export PJMEDIA_GSM_ENABLE = false
endif
ifeq ($(ZPL_PJ_CODEC_SPEEX_ENABLE),true)
export PJMEDIA_SPEEX_ENABLE = true
else
export PJMEDIA_SPEEX_ENABLE = false
endif
ifeq ($(ZPL_PJ_CODEC_ILBC_ENABLE),true)
export PJMEDIA_ILBC_ENABLE = true
else
export PJMEDIA_ILBC_ENABLE = false
endif
ifeq ($(ZPL_PJ_CODEC_G722_ENABLE),true)
export PJMEDIA_G722_ENABLE = true
else
export PJMEDIA_G722_ENABLE = false
endif
ifeq ($(ZPL_PJ_CODEC_WEBRTC_ENABLE),true)
export PJMEDIA_WEBRTC_ENABLE = true
else
export PJMEDIA_WEBRTC_ENABLE = false
endif
ifeq ($(ZPL_PJ_AUDIO_ALSA),true)
export PJMEDIA_AUDIO_ALSA = true
export PJMEDIA_AUDIO_PORTAUDIO = false
endif
ifeq ($(ZPL_PJ_AUDIO_PORTAUDIO),true)
export PJMEDIA_AUDIO_ALSA = false
export PJMEDIA_AUDIO_PORTAUDIO = true
endif
ifeq ($(ZPL_PJ_CODEC_VPX_ENABLE),true)
export PJMEDIA_VPX_ENABLE = true
else
export PJMEDIA_VPX_ENABLE = false
endif
ifeq ($(ZPL_PJ_FFMPEG_ENABLE),true)
export PJMEDIA_FFMPEG_ENABLE = true
else
export PJMEDIA_FFMPEG_ENABLE = false
endif
ifeq ($(ZPL_PJ_SDL_ENABLE),true)
export PJMEDIA_SDL2_ENABLE = true
else
export PJMEDIA_SDL2_ENABLE = false
endif


export PJSHARE_ENABLE = true
export PJMEDIA_ENABLE = true
export PJMEDIA_AUDIODEV_ENABLE = true
export PJMEDIA_CODEC_ENABLE = true
export PJMEDIA_NATH_ENABLE = true
export PJMEDIA_UA_ENABLE = true
ifeq ($(ZPL_PJ_VIDEO_ENABLE),true)
export PJMEDIA_V4L2_ENABLE = true
endif


# SDL flags
ifeq ($(PJMEDIA_SDL2_ENABLE),true)
SDL2PROJDIR = $(EXTERNSION_ROOT)/SDL2-2.0.12
SDL2_DIR = $(ZPL_PJ_SDL_LIB_PATH)

SDL_CFLAGS ?= -DPJMEDIA_VIDEO_DEV_HAS_SDL=1 -D_REENTRANT -I$(SDL2_DIR)/include/SDL2 
SDL_LDFLAGS ?= -Wl,--enable-new-dtags -lSDL2 -L$(SDL2_DIR)/lib 
else
SDL_CFLAGS ?=
SDL_LDFLAGS ?=
endif

# FFMPEG flags
ifeq ($(PJMEDIA_FFMPEG_ENABLE),true)
ifeq ($(strip $(ZPL_FFMPEG_MODULE)),true)
FFMPEG_DIR = $(ZPL_INSTALL_ROOTFS_DIR)
else
FFMPEG_DIR = $(ZPL_PJ_FFMPEG_LIB_PATH)
endif

FFMPEG_CFLAGS ?= \
				-DPJMEDIA_HAS_LIBAVDEVICE=1 -DPJMEDIA_HAS_LIBAVFORMAT=1 \
				-DPJMEDIA_HAS_LIBAVCODEC=1 -DPJMEDIA_HAS_LIBSWSCALE=1 \
				-DPJMEDIA_HAS_LIBAVUTIL=1 \
				-I$(FFMPEG_DIR)/include  

FFMPEG_LDFLAGS ?=   -L$(FFMPEG_DIR)/lib -lavdevice -lxcb -lxcb-shm \
				-lxcb-shape -lxcb-xfixes -lasound -lavfilter -lm  \
				-lavformat -lbz2 -lavcodec -lz -lswresample -lswscale -lavutil -pthread 
else
FFMPEG_CFLAGS ?=
FFMPEG_LDFLAGS ?=
endif

# Video4Linux2
ifeq ($(PJMEDIA_V4L2_ENABLE),true)
V4L2_CFLAGS ?= -DPJMEDIA_VIDEO_DEV_HAS_V4L2=1
V4L2_LDFLAGS ?= -lv4l2 
else
V4L2_CFLAGS ?=
V4L2_LDFLAGS ?=
endif

# OPENH264 flags
ifeq ($(PJMEDIA_H264_ENABLE),true)
ifeq ($(strip $(ZPL_OPENH264_MODULE)),true)
H264_DIR = $(ZPL_INSTALL_ROOTFS_DIR)
else
H264_DIR = $(ZPL_PJ_CODEC_H264_LIB_PATH)
endif
OPENH264_CFLAGS ?= -DPJMEDIA_HAS_OPENH264_CODEC=1 -DPJMEDIA_HAS_VIDEO=1 -I$(H264_DIR)/include
OPENH264_LDFLAGS ?= -L$(H264_DIR)/lib -lopenh264 -lstdc++
else
OPENH264_CFLAGS ?=
OPENH264_LDFLAGS ?=
endif

# VPX flags 
ifeq ($(PJMEDIA_VPX_ENABLE),true)
ifeq ($(strip $(ZPL_LIBVPX_MODULE)),true)
VPX_DIR = $(ZPL_INSTALL_ROOTFS_DIR)
else
VPX_DIR = $(ZPL_PJ_CODEC_VPX_LIB_PATH)
endif
VPX_CFLAGS ?= -DPJMEDIA_HAS_VPX_CODEC=1 -I$(VPX_DIR)/include 
VPX_LDFLAGS ?=  -L$(VPX_DIR)/lib -lvpx
else
VPX_CFLAGS ?=
VPX_LDFLAGS ?=
endif

endif #ZPL_PJPROJECT_MODULE


ifeq ($(strip $(ZPL_PJSIP_MODULE)),true)
PJSIP_ROOT=$(COMPONENT_ROOT)/pjsip
#export PJPROJDIR = $(ZPLBASE)/externsions/pjproject-2.10
#ZPL_INCLUDE += -I$(ZPLBASE)/externsions/pjproject-2.8
ZPLPRODS += $(PJSIP_ROOT)
ZPL_INCLUDE += -I$(PJSIP_ROOT)
ZPL_DEFINE += -DZPL_PJSIP_MODULE
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include


ZPL_LDLIBS += -lpj -lpjlib-util -lpjsip  -lpjsua2
			 
	
ifeq ($(PJSHARE_ENABLE),true)			 
ifeq ($(PJMEDIA_ENABLE),true)
ZPL_LDLIBS += -lpjmedia
endif
ifeq ($(PJMEDIA_AUDIODEV_ENABLE),true)
ZPL_LDLIBS += -lpjmedia-audiodev
endif
ifeq ($(PJMEDIA_CODEC_ENABLE),true)
ZPL_LDLIBS += -lpjmedia-codec
endif

ifeq ($(PJMEDIA_VIDEODEV_ENABLE),true)
ZPL_LDLIBS += -lpjmedia-videodev

# SDL flags
ifeq ($(PJMEDIA_SDL2_ENABLE),true)
ZPL_LDLIBS += -lSDL2 -L$(SDL2_DIR)/lib
endif

ifeq ($(PJMEDIA_FFMPEG_ENABLE),true)
# FFMPEG flags
ZPL_LDLIBS += -lavdevice -lxcb -lxcb-shm \
				-lxcb-shape -lxcb-xfixes -lasound -lavfilter  \
				-lavformat -lbz2 -lavcodec -lz -lswresample -lswscale -lavutil
endif

ifeq ($(PJMEDIA_V4L2_ENABLE),true)
# Video4Linux2
ZPL_LDLIBS += -lv4l2
endif

# OPENH264 flags
ifeq ($(PJMEDIA_H264_ENABLE),true)
ZPL_LDLIBS += -lopenh264 
ZPL_LDFLAGS += -L$(H264_DIR)/lib
endif

# VPX flags 
ifeq ($(PJMEDIA_VPX_ENABLE),true)
ZPL_LDLIBS += -lvpx -L$(VPX_DIR)/lib
endif

endif
ifeq ($(PJMEDIA_NATH_ENABLE),true)
ZPL_LDLIBS += -lpjnath
endif
ifeq ($(PJMEDIA_SIMPLE_ENABLE),true)
ZPL_LDLIBS += -lpjsip-simple
endif
ifeq ($(PJMEDIA_RESAMPLE_ENABLE),true)
ZPL_LDLIBS += -lresample
endif
ifeq ($(PJMEDIA_UA_ENABLE),true)
ZPL_LDLIBS += -lpjsua -lpjsip-ua
endif
ifeq ($(PJMEDIA_SRTP_ENABLE),true)
ZPL_LDLIBS += -lsrtp
endif
ifeq ($(PJMEDIA_YUV_ENABLE),true)
ZPL_LDLIBS += -lyuv
endif
ifeq ($(PJMEDIA_GSM_ENABLE),true)
ZPL_LDLIBS += -lgsmcodec
endif
ifeq ($(PJMEDIA_SPEEX_ENABLE),true)
ZPL_LDLIBS += -lspeex
endif
ifeq ($(PJMEDIA_ILBC_ENABLE),true)
ZPL_LDLIBS += -lilbccodec
endif
ifeq ($(PJMEDIA_G722_ENABLE),true)
ZPL_LDLIBS += -lg7221codec
endif
ifeq ($(PJMEDIA_WEBRTC_ENABLE),true)
ZPL_LDLIBS += -lwebrtc
endif

endif


ifeq ($(PJMEDIA_AUDIO_PORTAUDIO),true)
ZPLOS_LDLIBS += -lportaudio
endif

#./configure  --prefix=/home/zhurish/workspace/home-work/pjproject-2.8-x86/_install
# --enable-epoll --enable-sound --enable-video --enable-speex-aec --enable-g711-codec
# --enable-l16-codec --enable-gsm-codec --enable-g722-codec --enable-g7221-codec 
#--enable-speex-codec --enable-ilbc-codec --enable-v4l2 --disable-ipp --enable-libwebrtc
#ZPL_LDFLAGS += -L$(ZPLBASE)/externsions/pjproject-2.8/_install/lib 
#ZPL_LDFLAGS += -L$(ZPLBASE)/externsions/pjproject-2.8/third_party/lib 
#ZPL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
			 -lpjsua -lsrtp -lgsmcodec -lspeex -lilbccodec -lg7221codec 
			 
#ZPL_LDLIBS += -luuid -lasound


endif #($(strip $(ZPL_PJSIP_MODULE)),true)



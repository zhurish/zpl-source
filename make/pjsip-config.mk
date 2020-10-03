#include $(MAKE_DIR)/module-dir.mk

ifeq ($(strip $(PL_PJPROJECT_MODULE)),true)
PJPROJECT_ROOT=$(EXTERNSION_ROOT)/pjproject-2.10
PLEX_DIR += $(EXTERNSION_ROOT)/pjproject-2.10

ifeq ($(PL_PJ_RESAMPLE_ENABLE),true)
export PJMEDIA_RESAMPLE_ENABLE = true
else
export PJMEDIA_RESAMPLE_ENABLE = false
endif
ifeq ($(PL_PJ_SIMPLE_ENABLE),true)
export PJMEDIA_SIMPLE_ENABLE = true
else
export PJMEDIA_SIMPLE_ENABLE = false
endif
ifeq ($(PL_PJ_SRTP_ENABLE),true)
export PJMEDIA_SRTP_ENABLE = true
else
export PJMEDIA_SRTP_ENABLE = false
endif
ifeq ($(PL_PJ_VIDEO_ENABLE),true)
export PJMEDIA_VIDEODEV_ENABLE = true
else
export PJMEDIA_VIDEODEV_ENABLE = false
endif
ifeq ($(PL_PJ_VIDEO_YUV_ENABLE),true)
export PJMEDIA_YUV_ENABLE = true
else
export PJMEDIA_YUV_ENABLE = false
endif
ifeq ($(PL_PJ_VIDEO_H264_ENABLE),true)
export PJMEDIA_H264_ENABLE = true
else
export PJMEDIA_H264_ENABLE = false
endif
ifeq ($(PL_PJ_CODEC_GSM_ENABLE),true)
export PJMEDIA_GSM_ENABLE = true
else
export PJMEDIA_GSM_ENABLE = false
endif
ifeq ($(PL_PJ_CODEC_SPEEX_ENABLE),true)
export PJMEDIA_SPEEX_ENABLE = true
else
export PJMEDIA_SPEEX_ENABLE = false
endif
ifeq ($(PL_PJ_CODEC_ILBC_ENABLE),true)
export PJMEDIA_ILBC_ENABLE = true
else
export PJMEDIA_ILBC_ENABLE = false
endif
ifeq ($(PL_PJ_CODEC_G722_ENABLE),true)
export PJMEDIA_G722_ENABLE = true
else
export PJMEDIA_G722_ENABLE = false
endif
ifeq ($(PL_PJ_CODEC_WEBRTC_ENABLE),true)
export PJMEDIA_WEBRTC_ENABLE = true
else
export PJMEDIA_WEBRTC_ENABLE = false
endif
ifeq ($(PL_PJ_AUDIO_ALSA),true)
export PJMEDIA_AUDIO_ALSA = true
export PJMEDIA_AUDIO_PORTAUDIO = false
endif
ifeq ($(PL_PJ_AUDIO_PORTAUDIO),true)
export PJMEDIA_AUDIO_ALSA = false
export PJMEDIA_AUDIO_PORTAUDIO = true
endif

export PJSHARE_ENABLE = true
export PJMEDIA_ENABLE = true
export PJMEDIA_AUDIODEV_ENABLE = true
export PJMEDIA_CODEC_ENABLE = true

export PJMEDIA_NATH_ENABLE = true
export PJMEDIA_UA_ENABLE = true
export PJMEDIA_VPX_ENABLE = false
export PJMEDIA_V4L2_ENABLE = true
export PJMEDIA_SDL2_ENABLE = false
export PJMEDIA_FFMPEG_ENABLE = false


# SDL flags
ifeq ($(PJMEDIA_SDL2_ENABLE),true)
SDL2PROJDIR = $(EXTERNSION_ROOT)/SDL2-2.0.12
SDL2_DIR = $(EXTERNSION_ROOT)/SDL2-2.0.12/_install

SDL_CFLAGS ?= -DPJMEDIA_VIDEO_DEV_HAS_SDL=1 -D_REENTRANT -I$(SDL2_DIR)/include/SDL2 
SDL_LDFLAGS ?= -Wl,--enable-new-dtags -lSDL2 -L$(SDL2_DIR)/lib 

endif

ifeq ($(PJMEDIA_FFMPEG_ENABLE),true)
# FFMPEG flags
FFMPEG_DIR = /home/zhurish/workspace/android/pjsip/ffmpeg/_install

FFMPEG_CFLAGS ?=   -DPJMEDIA_USE_OLD_FFMPEG=1 \
				-DPJMEDIA_HAS_LIBAVDEVICE=1 -DPJMEDIA_HAS_LIBAVFORMAT=1 \
				-DPJMEDIA_HAS_LIBAVCODEC=1 -DPJMEDIA_HAS_LIBSWSCALE=1 \
				-DPJMEDIA_HAS_LIBAVUTIL=1 \
				-I$(FFMPEG_DIR)/include  

FFMPEG_LDFLAGS ?=   -L$(FFMPEG_DIR)/lib -lavdevice -lxcb -lxcb-shm \
				-lxcb-shape -lxcb-xfixes -lasound -lavfilter -lm  \
				-lavformat -lbz2 -lavcodec -lz -lswresample -lswscale -lavutil -pthread 

endif

ifeq ($(PJMEDIA_V4L2_ENABLE),true)
# Video4Linux2
V4L2_CFLAGS ?= -DPJMEDIA_VIDEO_DEV_HAS_V4L2=1
V4L2_LDFLAGS ?= -lv4l2 
endif

# OPENH264 flags
ifeq ($(PJMEDIA_H264_ENABLE),true)
H264_DIR = /usr/local
OPENH264_CFLAGS ?= -DPJMEDIA_HAS_OPENH264_CODEC=1 -DPJMEDIA_HAS_VIDEO=1 -I$(H264_DIR)/include
OPENH264_LDFLAGS ?= -L$(H264_DIR)/lib -lopenh264 -lstdc++
endif

# VPX flags 
ifeq ($(PJMEDIA_VPX_ENABLE),true)
VPX_DIR = /home/zhurish/workspace/android/pjsip/libvpx-1.9.0/_install-x86

VPX_CFLAGS ?= -DPJMEDIA_HAS_VPX_CODEC=1 -I$(VPX_DIR)/include 
VPX_LDFLAGS ?=  -L$(VPX_DIR)/lib -lvpx

endif


endif


ifeq ($(strip $(PL_PJSIP_MODULE)),true)
PJSIP_ROOT=$(COMPONENT_ROOT)/pjsip
#export PJPROJDIR = $(PLBASE)/externsions/pjproject-2.10
#PL_INCLUDE += -I$(PLBASE)/externsions/pjproject-2.8
PLPRODS += $(PJSIP_ROOT)
PL_INCLUDE += -I$(PJSIP_ROOT)
PL_DEFINE += -DPL_PJSIP_MODULE
PLEX_INCLUDE += -I$(PJPROJECT_ROOT)/_install/include


PL_LDLIBS += -lpj -lpjlib-util -lpjsip  -lpjsua2
			 
	
ifeq ($(PJSHARE_ENABLE),true)			 
ifeq ($(PJMEDIA_ENABLE),true)
PL_LDLIBS += -lpjmedia
endif
ifeq ($(PJMEDIA_AUDIODEV_ENABLE),true)
PL_LDLIBS += -lpjmedia-audiodev
endif
ifeq ($(PJMEDIA_CODEC_ENABLE),true)
PL_LDLIBS += -lpjmedia-codec
endif
ifeq ($(PJMEDIA_VIDEODEV_ENABLE),true)
PL_LDLIBS += -lpjmedia-videodev -lopenh264

# SDL flags
ifeq ($(PJMEDIA_SDL2_ENABLE),true)
PL_LDLIBS += -lSDL2 -L$(SDL2_DIR)/lib
endif

ifeq ($(PJMEDIA_FFMPEG_ENABLE),true)
# FFMPEG flags
PL_LDLIBS += -lavdevice -lxcb -lxcb-shm \
				-lxcb-shape -lxcb-xfixes -lasound -lavfilter  \
				-lavformat -lbz2 -lavcodec -lz -lswresample -lswscale -lavutil
endif

ifeq ($(PJMEDIA_V4L2_ENABLE),true)
# Video4Linux2
PL_LDLIBS += -lv4l2
endif

# OPENH264 flags
ifeq ($(PJMEDIA_H264_ENABLE),true)
PL_LDLIBS += -lopenh264 -L$(H264_DIR)/lib
endif

# VPX flags 
ifeq ($(PJMEDIA_VPX_ENABLE),true)
PL_LDLIBS += -lvpx -L$(VPX_DIR)/lib
endif

endif
ifeq ($(PJMEDIA_NATH_ENABLE),true)
PL_LDLIBS += -lpjnath
endif
ifeq ($(PJMEDIA_SIMPLE_ENABLE),true)
PL_LDLIBS += -lpjsip-simple
endif
ifeq ($(PJMEDIA_RESAMPLE_ENABLE),true)
PL_LDLIBS += -lresample
endif
ifeq ($(PJMEDIA_UA_ENABLE),true)
PL_LDLIBS += -lpjsua -lpjsip-ua
endif
ifeq ($(PJMEDIA_SRTP_ENABLE),true)
PL_LDLIBS += -lsrtp
endif
ifeq ($(PJMEDIA_YUV_ENABLE),true)
PL_LDLIBS += -lyuv
endif
ifeq ($(PJMEDIA_GSM_ENABLE),true)
PL_LDLIBS += -lgsmcodec
endif
ifeq ($(PJMEDIA_SPEEX_ENABLE),true)
PL_LDLIBS += -lspeex
endif
ifeq ($(PJMEDIA_ILBC_ENABLE),true)
PL_LDLIBS += -lilbccodec
endif
ifeq ($(PJMEDIA_G722_ENABLE),true)
PL_LDLIBS += -lg7221codec
endif
ifeq ($(PJMEDIA_WEBRTC_ENABLE),true)
PL_LDLIBS += -lwebrtc
endif

endif


ifeq ($(PJMEDIA_AUDIO_PORTAUDIO),true)
PLOS_LDLIBS += -lportaudio
endif

#./configure  --prefix=/home/zhurish/workspace/home-work/pjproject-2.8-x86/_install
# --enable-epoll --enable-sound --enable-video --enable-speex-aec --enable-g711-codec
# --enable-l16-codec --enable-gsm-codec --enable-g722-codec --enable-g7221-codec 
#--enable-speex-codec --enable-ilbc-codec --enable-v4l2 --disable-ipp --enable-libwebrtc
#PL_LDFLAGS += -L$(PLBASE)/externsions/pjproject-2.8/_install/lib 
#PL_LDFLAGS += -L$(PLBASE)/externsions/pjproject-2.8/third_party/lib 
#PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
			 -lpjsua -lsrtp -lgsmcodec -lspeex -lilbccodec -lg7221codec 
			 
#PL_LDLIBS += -luuid -lasound

PLCLI_DIR += $(CLI_ROOT)/voip

endif #($(strip $(PL_PJSIP_MODULE)),true)



#include $(MAKE_DIR)/module-dir.mk

ifeq ($(strip $(PL_MULTIMEDIA_MODULE)),true)

ifeq ($(strip $(PL_LIVE555_MODULE)),true)

LIVE555_ROOT=$(MULTIMEDIA_ROOT)/live555
PLEX_DIR += $(MULTIMEDIA_ROOT)/live555
PL_DEFINE += -DPL_LIVE555_MODULE
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include/liveMedia
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include/groupsock
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include/UsageEnvironment
PLEX_INCLUDE += -I$(DSTROOTFSDIR)/include/BasicUsageEnvironment
#PL_LDLIBS += -lBasicUsageEnvironment  -lgroupsock  -lliveMedia  -lUsageEnvironment

PLPRODS += $(MULTIMEDIA_ROOT)/media-app
PL_INCLUDE += -I$(MULTIMEDIA_ROOT)/media-app
	 
endif #($(strip $(PL_PJSIP_MODULE)),true)


ifeq ($(strip $(PL_LIBX264_MODULE)),true)
LIBX264_ROOT=$(MULTIMEDIA_ROOT)/x264
PLEX_DIR += $(MULTIMEDIA_ROOT)/x264
PL_DEFINE += -DPL_LIBX264_MODULE
endif

ifeq ($(strip $(PL_OPENH264_MODULE)),true)
LIBOPENH264_ROOT=$(MULTIMEDIA_ROOT)/openh264-2.1.1
PLEX_DIR += $(MULTIMEDIA_ROOT)/openh264-2.1.1
PL_DEFINE += -DPL_OPENH264_MODULE
endif


ifeq ($(strip $(PL_LIBVPX_MODULE)),true)
LIBVPX_ROOT=$(MULTIMEDIA_ROOT)/libvpx-1.9.0
PLEX_DIR += $(MULTIMEDIA_ROOT)/libvpx-1.9.0
PL_DEFINE += -DPL_LIBVPX_MODULE
endif

ifeq ($(strip $(PL_FFMPEG_MODULE)),true)
FFMPEG_ROOT=$(MULTIMEDIA_ROOT)/ffmpeg
PLEX_DIR += $(MULTIMEDIA_ROOT)/ffmpeg
PL_DEFINE += -DPL_FFMPEG_MODULE
endif
endif
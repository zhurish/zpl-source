#include $(ZPL_MAKE_DIR)/module-dir.mk

ifeq ($(strip $(ZPL_MULTIMEDIA_MODULE)),true)

ifeq ($(strip $(ZPL_LIVE555_MODULE)),true)
LIVE555_ROOT=$(MULTIMEDIA_DIR)/live555
#ZPLEX_DIR += $(ZPLBASE)/$(LIVE555_ROOT)
ZPL_DEFINE += -DZPL_LIVE555_MODULE -DUSE_RTSP_OPT
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_INC_DIR)
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_INC_DIR)/liveMedia
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_INC_DIR)/groupsock
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_INC_DIR)/UsageEnvironment
ZPLEX_INCLUDE += -I$(ZPL_INSTALL_INC_DIR)/BasicUsageEnvironment
ZPL_LDLIBS += -lBasicUsageEnvironment  -lgroupsock  -lliveMedia  -lUsageEnvironment 
endif #($(strip $(ZPL_PJSIP_MODULE)),true)


ifeq ($(strip $(ZPL_LIBRTSP_MODULE)),true)
LIBRTSP_DIR=$(MULTIMEDIA_DIR)/rtsp
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBRTSP_DIR)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBRTSP_DIR)
ZPL_DEFINE += -DZPL_LIBRTSP_MODULE
endif

#ifneq ($(strip $(ZPL_LIVE555_MODULE)),true)
ifeq ($(strip $(ZPL_LIBORTP_MODULE)),true)
LIBORTP_ROOT=$(MULTIMEDIA_DIR)/ortp-5.0
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBORTP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBORTP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBORTP_ROOT)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBORTP_ROOT)/src
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBORTP_ROOT)/playload
ZPL_DEFINE += -DZPL_LIBORTP_MODULE
#ZPL_LDLIBS += -L/home/zhurish/workspace/working/zpl-source/source/multimedia/ortp-5.0/include/lib -lbcunit  -lbctoolbox

#ifneq ($(strip $(ZPL_MEDIASTREAM_MODULE)),true)
#endif
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_MODULE)),true)
MEDIASTREAM_ROOT=$(MULTIMEDIA_DIR)/mediastreamer2
ZPLPRODS_LAST += $(ZPLBASE)/$(MEDIASTREAM_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MEDIASTREAM_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src
ZPL_DEFINE += -DZPL_MEDIASTREAM_MODULE
ZPL_LDLIBS += -lasound 


ZPLEX_CFLAGS += -Wno-error=missing-prototypes -Wno-error=redundant-decls -DHAVE_CONFIG_H -DMS2_INTERNAL \
	 -DVIDEO_ENABLED -DMS2_FILTERS -DORTP_TIMESTAMP

ZPLEX_INCLUDE	+=   \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/include/ \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/base \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/utils \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/utils/filter-interface \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/utils/filter-wrapper \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/voip \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/voip/h26x \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/audiofilters \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/otherfilters \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/videofilters \
	-I$(ZPLBASE)/$(MEDIASTREAM_ROOT)/src/tools 

ifeq ($(strip $(ZPL_MEDIASTREAM_V4L2)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_V4L2 
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_ALSA)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_ALSA  -D__ALSA_ENABLED__
#__PORTAUDIO_ENABLED__
#__PULSEAUDIO_ENABLED__
endif
ifeq ($(strip $(ZPL_MEDIASTREAM_SPEEX)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_SPEEX
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_SRTP)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_SRTP
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_DTLS_SRTP)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_DTLS_SRTP
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_ZRTP)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_ZRTP
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_OPENH264)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_OPENH264
endif

ifeq ($(strip $(ZPL_MEDIASTREAM_UPNP)),true)
ZPL_DEFINE += -DZPL_MEDIASTREAM_UPNP
endif


endif

ifeq ($(strip $(ZPL_JRTPLIB_MODULE)),true)
JRTPLIB_ROOT=$(MULTIMEDIA_DIR)/JRTPLIB
ZPLPRODS_LAST += $(ZPLBASE)/$(JRTPLIB_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(JRTPLIB_ROOT)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(JRTPLIB_ROOT)/src
ZPL_INCLUDE += -I$(ZPLBASE)/$(JRTPLIB_ROOT)/c-layer
ZPL_INCLUDE += -I$(ZPLBASE)/$(JRTPLIB_ROOT)/c-layer/h26x
ZPL_DEFINE += -DZPL_JRTPLIB_MODULE
endif
#endif

ifeq ($(strip $(ZPL_EXOSIP_MODULE)),true)
EXOSIP_ROOT=$(MULTIMEDIA_DIR)/exosip
ZPLEX_DIR += $(ZPLBASE)/$(EXOSIP_ROOT)
ZPL_DEFINE += -DZPL_EXOSIP_MODULE
endif

ifeq ($(strip $(ZPL_LIBRTMP_MODULE)),true)
LIBRTMP_ROOT=$(MULTIMEDIA_DIR)/librtmp
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBRTMP_ROOT)
#ZPLEX_DIR += $(ZPLBASE)/$(LIBRTMP_ROOT)
ZPL_DEFINE += -DZPL_LIBRTMP_MODULE
endif


ifeq ($(strip $(ZPL_LIBJPEG_MODULE)),true)
LIBJPEG_ROOT=$(MULTIMEDIA_DIR)/libjpeg
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBJPEG_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBJPEG_ROOT)
ZPL_DEFINE += -DZPL_LIBJPEG_MODULE
endif


ifeq ($(strip $(ZPL_LIBMEDIA_MODULE)),true)
LIBMEDIA_ROOT=$(MULTIMEDIA_DIR)/media
ZPLPRODS_LAST += $(ZPLBASE)/$(LIBMEDIA_ROOT)
ZPL_DEFINE += -DZPL_LIBMEDIA_MODULE
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMEDIA_ROOT)/include
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMEDIA_ROOT)/include/hal
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMEDIA_ROOT)/src/hal
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMEDIA_ROOT)/src/framediscrete
ifeq ($(strip $(ZPL_HISIMPP_MODULE)),true)
ZPL_DEFINE += -DZPL_HISIMPP_MODULE
ifeq ($(strip $(ZPL_HISIMPP_HWDEBUG)),true)
ZPL_DEFINE += -DZPL_HISIMPP_HWDEBUG
endif
ZPLEX_INCLUDE += -I$(ZPLBASE)/$(LIBMEDIA_ROOT)/hisidir/include
ZPLEX_LDFLAGS += -L$(ZPLBASE)/$(LIBMEDIA_ROOT)/hisidir/lib
ifeq ($(ZPL_BUILD_ARCH),ARM)
ZPLEX_LDLIBS += -laaccomm
ZPLEX_LDLIBS += -lisp
ZPLEX_LDLIBS += -laacdec
ZPLEX_LDLIBS += -live
ZPLEX_LDLIBS += -laacenc
ZPLEX_LDLIBS += -lmd
ZPLEX_LDLIBS += -laacsbrdec
ZPLEX_LDLIBS += -lmpi
ZPLEX_LDLIBS += -laacsbrenc
ZPLEX_LDLIBS += -lnnie
ZPLEX_LDLIBS += -ldnvqe
ZPLEX_LDLIBS += -lsecurec
ZPLEX_LDLIBS += -lhdmi
ZPLEX_LDLIBS += -l_hiae
ZPLEX_LDLIBS += -l_hiawb_natura
ZPLEX_LDLIBS += -l_hiawb
ZPLEX_LDLIBS += -l_hicalcflicker
ZPLEX_LDLIBS += -lhi_cipher
ZPLEX_LDLIBS += -l_hidehaze
#ZPLEX_LDLIBS += -L$(ZPLBASE)/source/debug/lib -l:lib_hidehaze.a -l:libisp.a
ZPLEX_LDLIBS += -l_hidrc
ZPLEX_LDLIBS += -lhifisheyecalibrate
ZPLEX_LDLIBS += -l_hiir_auto
ZPLEX_LDLIBS += -l_hildci
ZPLEX_LDLIBS += -lhive_AEC
ZPLEX_LDLIBS += -lhive_AGC
ZPLEX_LDLIBS += -lhive_ANR
ZPLEX_LDLIBS += -lhive_common
ZPLEX_LDLIBS += -lhive_EQ
ZPLEX_LDLIBS += -lsvpruntime
ZPLEX_LDLIBS += -lhive_HPF
ZPLEX_LDLIBS += -ltde
ZPLEX_LDLIBS += -lhive_record
ZPLEX_LDLIBS += -lupvqe
ZPLEX_LDLIBS += -lhive_RES_ext
ZPLEX_LDLIBS += -lVoiceEngine
ZPLEX_LDLIBS += -lhive_RES

ZPLEX_LDLIBS += -lsns_imx335_forcar
ZPLEX_LDLIBS += -lsns_imx335
ZPLEX_LDLIBS += -lsns_imx327
ZPLEX_LDLIBS += -lsns_imx327_2l
ZPLEX_LDLIBS += -lsns_imx307
ZPLEX_LDLIBS += -lsns_imx307_2l
ZPLEX_LDLIBS += -lsns_imx415
ZPLEX_LDLIBS += -lsns_mn34220
ZPLEX_LDLIBS += -lsns_imx458
ZPLEX_LDLIBS += -lsns_os05a
ZPLEX_LDLIBS += -lsns_os04b10
ZPLEX_LDLIBS += -lsns_os08a10
ZPLEX_LDLIBS += -lsns_ov12870
ZPLEX_LDLIBS += -lsns_sc4210
ZPLEX_LDLIBS += -lsns_gc2053

SENSOR0_TYPE ?= SONY_IMX327_2L_MIPI_2M_30FPS_12BIT
SENSOR1_TYPE ?= SONY_IMX327_2L_MIPI_2M_30FPS_12BIT
ZPLM_CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
ZPLM_CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)
endif #ZPL_BUILD_ARCH
endif #ZPL_HISIMPP_MODULE

ZPL_DEFINE += -DZPL_PIPELINE_MODULE
PIPELINE_ROOT=$(MULTIMEDIA_DIR)/pipeline
ZPLPRODS_LAST += $(ZPLBASE)/$(PIPELINE_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PIPELINE_ROOT)/include
ZPL_INCLUDE += -I$(ZPLBASE)/$(PIPELINE_ROOT)/src

ZPLEX_CFLAGS += -DVIDEO_ENABLED -DORTP_TIMESTAMP -Wno-error=missing-prototypes -Wno-error=redundant-decls

endif #ZPL_LIBMEDIA_MODULE


ifeq ($(strip $(ZPL_LIBX264_MODULE)),true)
LIBX264_ROOT=$(MULTIMEDIA_DIR)/x264
ZPLEX_DIR += $(ZPLBASE)/$(LIBX264_ROOT)
ZPL_DEFINE += -DZPL_LIBX264_MODULE
endif

ifeq ($(strip $(ZPL_OPENH264_MODULE)),true)
LIBOPENH264_ROOT=$(MULTIMEDIA_DIR)/openh264-2.1.1
ZPLEX_DIR += $(ZPLBASE)/$(LIBOPENH264_ROOT)
ZPL_DEFINE += -DZPL_OPENH264_MODULE
endif


ifeq ($(strip $(ZPL_LIBVPX_MODULE)),true)
LIBVPX_ROOT=$(MULTIMEDIA_DIR)/libvpx-1.9.0
ZPLEX_DIR += $(ZPLBASE)/$(LIBVPX_ROOT)
ZPL_DEFINE += -DZPL_LIBVPX_MODULE
endif

ifeq ($(strip $(ZPL_FFMPEG_MODULE)),true)
FFMPEG_ROOT=$(MULTIMEDIA_DIR)/ffmpeg
ZPLEX_DIR += $(ZPLBASE)/$(FFMPEG_ROOT)
ZPL_DEFINE += -DZPL_FFMPEG_MODULE
endif

#ifeq ($(strip $(ZPL_FFMPEG_MODULE)),true)
PJPROJ_ROOT=$(MULTIMEDIA_DIR)/pjproject
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjlib
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjlib-util
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjmedia
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjnath
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjsip
ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/third_party
#ZPLPRODS += $(ZPLBASE)/$(PJPROJ_ROOT)/pjsip-apps
ZPL_INCLUDE	+=   \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjlib/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjlib-util/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjmedia/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjnath/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjsip/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/g7221/common \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/g7221/decode \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/g7221/encode \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/gsm/inc \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/ilbc \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/resample/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/resample/src \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/speex/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/speex/libspeex \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/srtp/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/srtp/crypto/include \
	-I$(ZPLBASE)/$(PJPROJ_ROOT)/third_party/yuv/include
#	
#	-I$(ZPLBASE)/$(PJPROJ_ROOT)/pjsip-apps/include


ZPL_DEFINE += -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -fPIC -Wno-error=redundant-decls
ZPL_DEFINE += -DPJSIP_ALLOW_PORT_IN_FROMTO_HDR=1 -fPIC -DPJ_OS_NAME=\"linux\" \
	-Wno-error=int-conversion -Wno-error=type-limits -Wno-error=unused-label \
	-DPJ_LINUX_ZPLMEDIA -DPJMEDIA_HAS_VIDEO=1 -DHAVE_CONFIG_H -DOPENSSL \
	-DSASR -DWAV49 -DNeedFunctionPrototypes=1 -DPJMEDIA_AUDIO_DEV_HAS_ALSA=1 \
	-DPJMEDIA_VIDEO_DEV_HAS_V4L2=1 -Wno-error=unused-variable -Wno-error=unused-const-variable \
	-Wno-error=unused-function -Wno-error=overlength-strings -Wno-error=unused-value -DPJMEDIA_AUDIO_DEV_HAS_ALSA=1

ZPL_LDLIBS += -lasound -lv4l2 -lssl -lcrypto -lportaudio



ifeq ($(strip $(ZPL_PJSIP_MODULE)),true)
PJSIP_ROOT=$(COMPONENT_DIR)/pjsip
ZPLPRODS += $(ZPLBASE)/$(PJSIP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)
ZPL_DEFINE += -DZPL_PJSIP_MODULE
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)/apps
#ZPLEX_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include
#ZPL_LDLIBS += -lpjbase -lpjlib-util -lpjsip 
ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)/pjsua2
#ZPL_LDLIBS += -lpjsua2
endif
endif

endif # ZPL_MULTIMEDIA_MODULE



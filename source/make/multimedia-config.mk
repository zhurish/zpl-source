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

endif #ZPL_LIBMEDIA_MODULE


ifeq ($(strip $(ZPL_PJSIP_MODULE)),true)

PJPROJ_ROOT=$(MULTIMEDIA_DIR)/pjproject
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/pjlib
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/pjlib-util
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/pjmedia
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/pjnath
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/pjsip
ZPLPRODS_LAST += $(ZPLBASE)/$(PJPROJ_ROOT)/third_party
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

#
ZPL_DEFINE += -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_LINUX=1 -fPIC -Wno-error=redundant-decls
ZPL_DEFINE += -DPJSIP_ALLOW_PORT_IN_FROMTO_HDR=1 -fPIC -DPJ_OS_NAME=\"linux\" \
	-Wno-error=int-conversion -Wno-error=type-limits -Wno-error=unused-label \
	-DPJ_LINUX_ZPLMEDIA -DHAVE_CONFIG_H  \
	-DSASR -DWAV49 -DNeedFunctionPrototypes=1 \
	-Wno-error=unused-variable -Wno-error=unused-const-variable \
	-Wno-error=unused-function -Wno-error=overlength-strings -Wno-error=unused-value 


ifeq ($(strip $(ZPL_PJSIP_SRTP)),true)
ZPL_DEFINE += -DOPENSSL
ZPL_DEFINE += -DPJMEDIA_HAS_SRTP=1
ZPL_LDLIBS += -lv4l2 -lssl -lcrypto
else
ZPL_DEFINE += -DPJMEDIA_HAS_SRTP=0
endif

ifeq ($(strip $(ZPL_PJSIP_VIDEO)),true)
ZPL_DEFINE += -DPJMEDIA_HAS_VIDEO=1
ifeq ($(strip $(ZPL_PJSIP_VIDEO_V4L2)),true)
ZPL_DEFINE += -DPJMEDIA_VIDEO_DEV_HAS_V4L2=1
ZPL_LDLIBS += -lv4l2
else
ZPL_DEFINE += -DPJMEDIA_VIDEO_DEV_HAS_V4L2=0 
endif
else
ZPL_DEFINE += -DPJMEDIA_HAS_VIDEO=0 -DPJMEDIA_VIDEO_DEV_HAS_V4L2=0 
endif

ifeq ($(strip $(ZPL_PJSIP_ALSA)),true)
ZPL_DEFINE += -DPJMEDIA_AUDIO_DEV_HAS_ALSA=1
ZPL_LDLIBS += -lasound
else
ZPL_DEFINE += -DPJMEDIA_AUDIO_DEV_HAS_ALSA=0
endif

ifeq ($(ZPL_BUILD_ARCH),X86_64)
ifeq ($(strip $(ZPL_PJSIP_PORTAUDIO)),true)
ZPL_DEFINE += -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=1
ZPL_LDLIBS += -lportaudio -lasound
else
ZPL_DEFINE += -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0
endif
endif

ifeq ($(strip $(ZPL_PJSIP_HISIVIDEO)),true)
ZPL_DEFINE += -DZPL_PJSIP_HISIVIDEO=1
else
ZPL_DEFINE += -DZPL_PJSIP_HISIVIDEO=0
endif
ifeq ($(strip $(ZPL_PJSIP_HISIAUDIO)),true)
ZPL_DEFINE += -DZPL_PJSIP_HISIAUDIO=1
else
ZPL_DEFINE += -DZPL_PJSIP_HISIAUDIO=0
endif

PJSIP_ROOT=$(COMPONENT_DIR)/pjsip
ZPLPRODS_LAST += $(ZPLBASE)/$(PJSIP_ROOT)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)
ZPL_DEFINE += -DZPL_PJSIP_MODULE
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)/apps
ifeq ($(strip $(ZPL_PJSIP_PJSUA2)),true)
ZPL_INCLUDE += -I$(ZPLBASE)/$(PJSIP_ROOT)/pjsua2
endif
endif


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

endif # ZPL_MULTIMEDIA_MODULE



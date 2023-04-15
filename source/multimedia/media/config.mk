#############################################################################
# DEFINE
###########################################################################

#OS

ZPLEX_INCLUDE += -I$(LIBMEDIA_ROOT)/src/framediscrete -I$(LIBMEDIA_ROOT)/bsp

OSOBJ	+= zpl_media_channel.o
OSOBJ	+= zpl_media_file.o
OSOBJ	+= zpl_media_client.o
OSOBJ	+= zpl_media_buffer.o
OSOBJ	+= zpl_media_hardadap.o
OSOBJ	+= zpl_media_api.o
OSOBJ	+= zpl_media_codecmap.o
OSOBJ	+= zpl_media.o

OSOBJ	+= zpl_media_cmd.o
OSOBJ	+= zpl_media_task.o
OSOBJ	+= zpl_media_log.o
OSOBJ	+= zpl_media_resources.o
OSOBJ	+= zpl_media_event.o
OSOBJ	+= zpl_loadbmp.o
OSOBJ	+= zpl_media_extradata.o
OSOBJ	+= zpl_media_record.o
OSOBJ	+= zpl_media_capture.o
OSOBJ	+= zpl_media_image.o
OSOBJ	+= zpl_media_database.o
OSOBJ	+= zpl_media_bmp.o
OSOBJ	+= zpl_media_text.o
OSOBJ	+= zpl_media_area.o
OSOBJ	+= zpl_media_proxy.o
OSOBJ	+= zpl_media_frame_adap.o
OSOBJ	+= zpl_media_sys.o
OSOBJ	+= zpl_media_rtp.o

OSOBJ	+= zpl_media_video_encode.o
OSOBJ	+= zpl_media_video_vpss.o
OSOBJ	+= zpl_media_video_input.o
#OSOBJ	+= zpl_media_video_dev.o
#
OSOBJ	+= nal-h264.o
OSOBJ	+= nal-hevc.o
OSOBJ	+= nal-rbsp.o
OSOBJ	+= rtp_h264.o
OSOBJ	+= rtp_g7xx.o

#ifeq ($(strip $(ZPL_HISIMPP_MODULE)),true)
OSOBJ	+= zpl_vidhal_mipi.o
OSOBJ	+= zpl_vidhal_sensor.o
OSOBJ	+= zpl_vidhal_isp.o
#OSOBJ	+= zpl_vidhal_dev.o
OSOBJ	+= zpl_vidhal_input.o
OSOBJ	+= zpl_vidhal_hdmi.o
OSOBJ	+= zpl_vidhal_region.o
OSOBJ	+= zpl_vidhal_venc.o
OSOBJ	+= zpl_vidhal_vpss.o
OSOBJ	+= zpl_vidhal_vgs.o
OSOBJ	+= zpl_vidhal_ive.o
OSOBJ	+= zpl_vidhal_svp.o
OSOBJ	+= zpl_vidhal_nnie.o

OSOBJ	+= zpl_syshal.o
OSOBJ	+= zpl_vidhal.o	
#endif


ifeq ($(strip $(ZPL_HISIMPP_MODULE)),true)
#OSOBJ	+= sample_comm_audio.o
##OSOBJ	+= sample_comm_isp.o
#OSOBJ	+= sample_comm_region.o	
##OSOBJ	+= sample_comm_sys.o
#OSOBJ	+= sample_comm_vdec.o
##OSOBJ	+= sample_comm_venc.o
##OSOBJ	+= sample_comm_vi.o
#OSOBJ	+= sample_comm_vo.o
##OSOBJ	+= sample_comm_vpss.o
#OSOBJ	+= loadbmp.o
##OSOBJ	+= sample_venc.o
endif
#############################################################################
# LIB
###########################################################################
LIBS = libzplmedia.a

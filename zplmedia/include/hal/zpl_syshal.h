/*
 * zpl_syshal.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_SYSHAL_H__
#define __ZPL_SYSHAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_media.h>
#include <zpl_vidhal.h>

const char *zpl_syshal_strerror(zpl_uint32 halerrno);

extern int zpl_syshal_mem_init(void);
void zpl_syshal_mem_exit(void);

extern zpl_void * zpl_sys_iommap(zpl_uint64 u64PhyAddr, zpl_uint32 u32Size);
extern int zpl_sys_munmap(zpl_void* pVirAddr, zpl_uint32 u32Size);

extern int zpl_syshal_vbmem_init(void);

extern int zpl_syshal_vivpss_mode_set(zpl_int32 inputpipe, zpl_int32 mode);
/* 获取视频分辨率大小 resolution ratio */
extern int zpl_syshal_get_video_resolution(ZPL_VIDEO_FORMAT_E format, zpl_video_size_t* pstSize);
/* 获取一帧图像的缓冲区大小 */
extern zpl_uint32 zpl_syshal_get_buffer_size(ZPL_VIDEO_FORMAT_E format, zpl_uint32 pixelformat);
extern zpl_uint32 zpl_syshal_get_membuf_size(zpl_uint32 u32Width, zpl_uint32 u32Height,
        zpl_uint32 enPixelFormat, zpl_uint32 enBitWidth, zpl_uint32 enCmpMode, zpl_uint32 u32Align);

extern int zpl_syshal_input_bind_vpss(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vpssgrp);
extern int zpl_syshal_input_unbind_vpss(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vpssgrp);

extern int zpl_syshal_vpss_bind_venc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_int32 vencchn);
extern int zpl_syshal_vpss_unbind_venc(zpl_int32 vpssgrp, zpl_int32 vpsschn, zpl_int32 vencchn);


extern int zpl_syshal_input_bind_hdmi(zpl_int32 inputpipe, zpl_int32 inputchn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn);
extern int zpl_syshal_input_unbind_hdmi(zpl_int32 inputpipe, zpl_int32 inputchn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn);

extern int zpl_syshal_input_bind_venc(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vencchn);
extern int zpl_syshal_input_unbind_venc(zpl_int32 inputpipe, zpl_int32 inputchn, zpl_int32 vencchn);

extern int zpl_syshal_vpss_bind_avs(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 avsgrp, zpl_int32 avschn);
extern int zpl_syshal_vpss_unbind_avs(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 avsgrp, zpl_int32 avschn);

extern int zpl_syshal_vpss_bind_hdmi(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn);
extern int zpl_syshal_vpss_unbind_hdmi(zpl_int32 vpssgrp, zpl_int32 vpsschn, 
	zpl_int32 hdmilayer, zpl_int32 hdmichn);

extern int zpl_syshal_avs_bind_avs(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 dstavsgrp, zpl_int32 dstavschn);
extern int zpl_syshal_avs_unbind_avs(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 dstavsgrp, zpl_int32 dstavschn);

extern int zpl_syshal_avs_bind_vpss(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 vpssgrp, zpl_int32 vpsschn);
extern int zpl_syshal_avs_unbind_vpss(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 vpssgrp, zpl_int32 vpsschn);

extern int zpl_syshal_avs_bind_venc(zpl_int32 avsgrp, zpl_int32 avschn, zpl_int32 vencchn);
extern int zpl_syshal_avs_unbind_venc(zpl_int32 avsgrp, zpl_int32 avschn, zpl_int32 vencchn);

extern int zpl_syshal_avs_bind_hdmi(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 hdmilayer, zpl_int32 hdmichn);
extern int zpl_syshal_avs_unbind_hdmi(zpl_int32 avsgrp, zpl_int32 avschn,
	zpl_int32 hdmilayer, zpl_int32 hdmichn);

extern int zpl_syshal_vdec_bind_vpss(zpl_int32 vdecchn, zpl_int32 vpssgrp);
extern int zpl_syshal_vdec_unbind_vpss(zpl_int32 vdecchn, zpl_int32 vpssgrp);

extern int zpl_syshal_hdmi_bind_hdmi(zpl_int32 hdmilayer, zpl_int32 hdmichn, 
	zpl_int32 dsthdmilayer, zpl_int32 dsthdmichn);
extern int zpl_syshal_hdmi_unbind_hdmi(zpl_int32 hdmilayer, zpl_int32 hdmichn);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_SYSHAL_H__ */
/**
 * @file     zpl_media_pjdev.h
 * @brief     : Description
 * @author   zhurish (zhurish@163.com)
 * @version  1.0
 * @date     2023-08-13
 * 
 * @copyright   Copyright (c) 2023 {author}({email}).Co.Ltd. All rights reserved.
 * 
 */
#ifndef __ZPL_MEDIA_PJDEV_H__
#define __ZPL_MEDIA_PJDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ZPL_PJSIP_HISIAUDIO 1
#define ZPL_PJSIP_HISIVIDEO 1

#if defined(ZPL_PJSIP_HISIVIDEO) && ZPL_PJSIP_HISIVIDEO!=0
extern pjmedia_vid_dev_factory* pjmedia_pjdev_hwvideo_factory(pj_pool_factory *pf);
#endif

#if defined(ZPL_PJSIP_HISIAUDIO) && ZPL_PJSIP_HISIAUDIO!=0
extern pjmedia_aud_dev_factory* pjmedia_pjdev_hwaudio_factory(pj_pool_factory *pf);
#endif


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_PJDEV_H__ */

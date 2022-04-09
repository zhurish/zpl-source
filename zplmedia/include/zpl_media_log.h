/*
 * zpl_media_log.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_LOG_H__
#define __ZPL_MEDIA_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media_hal.h"

#define zpl_media_debugmsg_err(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_error(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_warn(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_info(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_notice(format, ...) 	    zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_debug(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_trap(format, ...) 		zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)
#define zpl_media_debugmsg_force_trap(format, ...) 	zlog_force_trap (MODULE_ZPLMEDIA, format, ##__VA_ARGS__)

#define zpl_video_assert(EX) zassert(EX)

/*
#define ZPL_DEBUG_DETAIL    0X00000010
#define ZPL_DEBUG_EVENT     0X00000008
#define ZPL_DEBUG_WARN      0X00000002
#define ZPL_DEBUG_DEBUG     0X00000004
#define ZPL_DEBUG_ERROR     0X00000001
#define ZPL_DEBUG_INPUT     0X00010000
#define ZPL_DEBUG_VPSS      0X00020000
#define ZPL_DEBUG_VENC      0X00040000
#define ZPL_DEBUG_IVE       0X00080000
#define ZPL_DEBUG_VGS       0X00100000
#define ZPL_DEBUG_CHANNEL   0X00200000

#define VIDEO_ISDEBUG(n)        ((_video_debug)&(ZPL_DEBUG_ ##n ) )
#define VIDEO_DEBUG_ON(n)       ((_video_debug)|=(ZPL_DEBUG_ ##n ) )
#define VIDEO_DEBUG_OFF(n)      ((_video_debug)&=~(ZPL_DEBUG_ ##n ) )

extern zpl_uint32   _video_debug;
*/
#ifdef ZPL_HISIMPP_HWDEBUG
#define ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL		300
#define ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL		300
#endif

typedef enum 
{
    ZPL_VIDEO_DEBUG_EVENT   = 0x00000001,
    ZPL_VIDEO_DEBUG_SEND    = 0x00000002,      
    ZPL_VIDEO_DEBUG_RECV    = 0x00000004, 
    ZPL_VIDEO_DEBUG_DETAIL  = 0x00000008, 
    ZPL_VIDEO_DEBUG_HARDWARE= 0x00000010, 
    ZPL_VIDEO_DEBUG_ERROR   = 0x00000020,
    ZPL_VIDEO_DEBUG_WARN    = 0x00000040,
    ZPL_VIDEO_DEBUG_BUFDETAIL    = 0x00000080,
    ZPL_VIDEO_DEBUG_BUFFILE = 0x00000100,
    ZPL_VIDEO_DEBUG_MAX     = 0x80000000, 
} ZPL_VIDEO_DEBUG_E;


extern zpl_uint32 __video_debug[ZPL_MEDIA_HARDADAP_MAX];

#define ZPL_MEDIA_DEBUG_ON(m, v)	  ( __video_debug[ ZPL_MEDIA_HARDADAP_ ##m ] |= (ZPL_VIDEO_DEBUG_ ##v ) )
#define ZPL_MEDIA_DEBUG_OFF(m, v)	  ( __video_debug[ ZPL_MEDIA_HARDADAP_ ##m ] &= ~(ZPL_VIDEO_DEBUG_ ##v ) )
#define ZPL_MEDIA_DEBUG(m, v)	      ( __video_debug[ ZPL_MEDIA_HARDADAP_ ##m ] & (ZPL_VIDEO_DEBUG_ ##v ) )

int zpl_media_debugmsg_init();
int zpl_bufdata_detail_debug(zpl_uint8 *buf, zpl_uint32 len);

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_LOG_H__ */

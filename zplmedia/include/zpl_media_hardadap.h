/*
 * zpl_media_callback.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_HARDADAP_H__
#define __ZPL_MEDIA_HARDADAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>

typedef enum 
{
    ZPL_MEDIA_HARDADAP_DEVICE     = 0x00,
    ZPL_MEDIA_HARDADAP_INPUTPIPE  = 0x01,
    ZPL_MEDIA_HARDADAP_INPUT      = 0x02,
    ZPL_MEDIA_HARDADAP_VPSSGRP    = 0x03,
    ZPL_MEDIA_HARDADAP_VPSS       = 0x04,
    ZPL_MEDIA_HARDADAP_ENCODE     = 0x05,
    ZPL_MEDIA_HARDADAP_ISP        = 0x06,
    ZPL_MEDIA_HARDADAP_HDMI       = 0x07,
    ZPL_MEDIA_HARDADAP_REGION     = 0x08,
    ZPL_MEDIA_HARDADAP_VGS        = 0x09,
    ZPL_MEDIA_HARDADAP_YUV        = 0x0A,
    ZPL_MEDIA_HARDADAP_IVE        = 0x0B,
    ZPL_MEDIA_HARDADAP_NNIE       = 0x0C,
    ZPL_MEDIA_HARDADAP_SVP        = 0x0D,
    ZPL_MEDIA_HARDADAP_SYS        = 0x0E,
    ZPL_MEDIA_HARDADAP_CHANNEL    = 0x0F,
    ZPL_MEDIA_HARDADAP_MAX        = 0x10,
} ZPL_MEDIA_HARDADAP_E;



#define ZPL_MEDIA_HARDADAP_MAX     8


typedef int(*zpl_media_hardadap_handler)(zpl_void *, zpl_int32, zpl_int32, void *, zpl_int32 );

typedef struct zpl_media_hardadap_s {
    zpl_int32           module;             // 模块ID
    zpl_int32           dst_device;            // 发送目的ID
    zpl_int32           dst_channel;            // 发送目的通道
    zpl_int32           src_device;            // 发送目的ID
    zpl_int32           src_channel;            // 发送目的通道
    zpl_bool            *online;
    zpl_void            *dst_private;
    zpl_media_hardadap_handler hardadap_sendto;// 发送回调
} zpl_media_hardadap_t;

typedef struct
{
    zpl_media_hardadap_t callback[ZPL_MEDIA_HARDADAP_MAX];

}zpl_media_hardadap_lst_t;

//Hardware Adaptation


int zpl_media_hardadap_init(zpl_media_hardadap_lst_t *);
int zpl_media_hardadap_install(zpl_media_hardadap_lst_t *, zpl_media_hardadap_t *);
int zpl_media_hardadap_uninstall(zpl_media_hardadap_lst_t *,
    zpl_int32 , zpl_int32 , zpl_int32 );
int zpl_media_hardadap_handle(zpl_media_hardadap_lst_t *, void *, zpl_int );

int zpl_media_hal_input_sendto_vpss_default(zpl_void *, zpl_int32 , zpl_int32 ,
        void *, zpl_int32 );
int zpl_media_hal_vpss_sendto_encode_default(zpl_void *, zpl_int32 , zpl_int32 ,
        void *, zpl_int32 );


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_HARDADAP_H__ */

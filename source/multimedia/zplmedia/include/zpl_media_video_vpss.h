
/*
 * zpl_video_vpss.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_VPSS_H__
#define __ZPL_VIDEO_VPSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>

#define ZPL_VIDEO_VPSSGRP_ENABLE

#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
typedef struct 
{
    NODE                node;
    zpl_int32           vpss_group;     //底层设备编号
    zpl_video_size_t	input_size;		//视频输入大小
    zpl_void            *video_input;   //绑定的输入
    int (*vpss_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VPSS 接收到数据发送到下一级(如：VENC)

    zpl_uint32          bindcount;      //绑定的数量（VPSS CHN）
    zpl_bool            online;         //资源状态，在线不在线
    zpl_bool            hwbind;         //硬件绑定
    zpl_uint32          res_flag;       //资源标志 

#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32          dbg_send_count;
    zpl_uint32          dbg_recv_count;    
#endif
}zpl_media_video_vpssgrp_t;
#endif

typedef struct 
{
    NODE                node;
    zpl_int32           vpss_channel;   //底层通道号
    zpl_socket_t         vpssfd;
	zpl_video_size_t	input_size;		//视频输入大小  

    int (*vpss_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VPSS 接收到数据发送到下一级(如：VENC)

#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
    zpl_media_video_vpssgrp_t *vpssgrp;   //绑定的分组
#else
    zpl_void            *video_input;   //绑定的输入
#endif
    zpl_uint32          bindcount;      //绑定的数量（VENC）
    zpl_void            *t_read;
    zpl_void            *t_master;
    
    zpl_bool            online;         //资源状态，在线不在线
    zpl_bool            hwbind;         //硬件绑定
    zpl_uint32          res_flag;       //资源标志 
#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32          dbg_send_count;
    zpl_uint32          dbg_recv_count;    
#endif
}zpl_media_video_vpss_channel_t;

int zpl_media_video_vpss_init(void);


#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
int zpl_media_video_vpssgrp_count(zpl_int32 vpssgrp);
int zpl_media_video_vpssgrp_destroy(zpl_int32 vpssgrp);
int zpl_media_video_vpssgrp_create(zpl_int32 vpssgrp);
zpl_media_video_vpssgrp_t * zpl_media_video_vpssgrp_lookup(zpl_int32 vpssgrp);
int zpl_media_video_vpssgrp_bindcount_set(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool add);
int zpl_media_video_vpssgrp_bindcount_get(zpl_media_video_vpssgrp_t *vpssgrp);
int zpl_media_video_vpssgrp_online_set(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool online);
int zpl_media_video_vpssgrp_online_get(zpl_media_video_vpssgrp_t *vpssgrp, zpl_bool *online);
int zpl_media_video_vpssgrp_input_set(zpl_int32 vpssgrp, void *video_input);
int zpl_media_video_vpssgrp_sendto(zpl_media_video_vpssgrp_t *vpssgrp,  void *p, zpl_int timeout);
int zpl_meida_video_vpssgrp_hal_create(zpl_media_video_vpssgrp_t *vpssgrp);
int zpl_media_video_vpssgrp_start(zpl_media_video_vpssgrp_t *vpssgrp);
int zpl_media_video_vpssgrp_stop(zpl_media_video_vpssgrp_t *vpssgrp);
int zpl_meida_video_vpssgrp_hal_destroy(zpl_media_video_vpssgrp_t *vpssgrp);
#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpssgrp_show(void *pvoid);
#endif
#endif

int zpl_media_video_vpss_channel_new(zpl_int32 vpss_channel);
int zpl_media_video_vpss_channel_delete(zpl_int32 vpss_channel);
zpl_media_video_vpss_channel_t * zpl_media_video_vpss_channel_lookup(zpl_int32 vpss_channel);
int zpl_media_video_vpss_channel_count(zpl_int32 vpssgrp);
#ifndef ZPL_VIDEO_VPSSGRP_ENABLE
int zpl_media_video_vpss_channel_input_set(zpl_int32 vpss_channel, void *video_input);
#else
int zpl_media_video_vpss_channel_vpssgrp_set(zpl_int32 vpss_channel, void *vpssgrp);
#endif
int zpl_media_video_vpss_channel_encode_set(zpl_int32 vpss_channel, void *encode);

int zpl_media_video_vpss_channel_bindcount_set(zpl_media_video_vpss_channel_t *vpss, zpl_bool add);
int zpl_media_video_vpss_channel_bindcount_get(zpl_media_video_vpss_channel_t *vpss);
int zpl_media_video_vpss_channel_read_start(zpl_void *master, zpl_media_video_vpss_channel_t *vpss);
int zpl_media_video_vpss_channel_read_stop(zpl_media_video_vpss_channel_t *vpss);

int zpl_meida_video_vpss_channel_hal_create(zpl_media_video_vpss_channel_t *vpss);
int zpl_media_video_vpss_channel_start(zpl_media_video_vpss_channel_t *vpss);
int zpl_media_video_vpss_channel_stop(zpl_media_video_vpss_channel_t *vpss);
int zpl_meida_video_vpss_channel_hal_destroy(zpl_media_video_vpss_channel_t *vpss);
int zpl_media_video_vpss_channel_online_set(zpl_media_video_vpss_channel_t *vpss, zpl_bool online);
int zpl_media_video_vpss_channel_online_get(zpl_media_video_vpss_channel_t *vpss, zpl_bool *online);

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpss_channel_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_VPSS_H__ */

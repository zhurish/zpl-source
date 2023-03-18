
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
#ifdef ZPL_VIDEO_VPSSGRP_ENABLE
//#include <zpl_media_video_vpssgrp.h>
#endif
/* Video Process Sub-System */


typedef struct 
{
    NODE                node;
    zpl_int32           vpss_group;     //底层组编号
    zpl_int32           vpss_channel;   //底层通道号
    zpl_socket_t        vpssfd;

	zpl_video_size_t	input_size;		//视频输入大小  
    zpl_video_size_t	output_size;	//视频输出大小 

    int (*vpsschn_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VPSS 接收到数据发送到下一级(如：VENC)


    zpl_void            *source_input;   //绑定的输入
    volatile zpl_uint32 reference;      //绑定的数量（VENC）
    zpl_void            *t_read;
    zpl_void            *t_master;
    
    zpl_void            *parent;    //指向父级

    zpl_bool            online;         //资源状态，在线不在线
    zpl_bool            hwbind;         //硬件绑定
    //zpl_uint32          res_flag;       //资源标志 
    //zpl_uint32          res_grp_flag;   //资源标志 
    zpl_uint32          grp_bind_count; //同一个组绑定通道数量 
    zpl_uint32          flags;
#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32          dbg_send_count;
    zpl_uint32          dbg_recv_count;    
#endif
}zpl_media_video_vpsschn_t;

int zpl_media_video_vpsschn_init(void);


zpl_media_video_vpsschn_t * zpl_media_video_vpsschn_create(zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_video_size_t output_size);
int zpl_media_video_vpsschn_hal_create(zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_destroy(zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_hal_destroy(zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_start(void *master, zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_stop(zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_thread(zpl_media_video_vpsschn_t *vpsschn, zpl_bool start);
int zpl_media_video_vpsschn_outputsize_get(zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_video_size_t *output_size);
extern zpl_bool zpl_media_video_vpsschn_state_check(zpl_media_video_vpsschn_t *vpsschn, int bit);

zpl_media_video_vpsschn_t * zpl_media_video_vpsschn_lookup(zpl_int32 vpss_group, zpl_int32 vpss_channel);
int zpl_media_video_vpssgrp_lookup(zpl_int32 vpss_group);
int zpl_media_video_vpsschn_source_set(zpl_int32 venc_group, zpl_int32 venc_channel, void *video_input, zpl_bool hwbind);

extern int zpl_media_video_vpsschn_addref(zpl_media_video_vpsschn_t *vpsschn);
extern int zpl_media_video_vpsschn_delref(zpl_media_video_vpsschn_t *vpsschn);
extern int zpl_media_video_vpsschn_reference(zpl_media_video_vpsschn_t *vpsschn);
int zpl_media_video_vpsschn_sendto(zpl_media_video_vpsschn_t *vpsschn,  void *p, zpl_int timeout);

#ifdef ZPL_SHELL_MODULE
int zpl_media_video_vpsschn_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_VPSS_H__ */


/*
 * zpl_video_input.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_INPUT_H__
#define __ZPL_VIDEO_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_video_input_pipe.h>

typedef struct 
{
    NODE                    node;
    zpl_int32               input_chn;                     //底层通道号
    zpl_socket_t               chnfd;
	zpl_video_size_t		input_size;		 //视频输出大小 
    zpl_video_size_t        sacle_size;      //缩放大小

    int (*input_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VI 接收到数据发送到下一级(如：VPSS)

    zpl_void                *t_read;
    zpl_void                *t_master;
    zpl_video_input_pipe_t  *inputpipe;

    zpl_uint32              bindcount;      //绑定的数量
    zpl_bool                online;         //资源状态，在线不在线
    zpl_bool                hwbind;         //硬件绑定
    zpl_uint32              res_flag;       //资源标志    
#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32              dbg_send_count;
    zpl_uint32              dbg_recv_count;    
#endif
}zpl_video_input_channel_t;

extern int zpl_video_input_init();
extern int zpl_video_input_exit();
extern int zpl_video_input_channel_create(zpl_int32 input_channel);
extern int zpl_video_input_channel_destroy(zpl_int32 input_channel);
extern zpl_video_input_channel_t * zpl_video_input_channel_lookup(zpl_int32 input_channel);

extern int zpl_video_input_channel_read_start(zpl_void *master, zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_read_stop(zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_sendto(zpl_video_input_channel_t *input,  void *p, zpl_int timeout);
extern int zpl_video_input_channel_active(zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_start(zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_stop(zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_inactive(zpl_video_input_channel_t *input);
extern int zpl_video_input_channel_online_set(zpl_video_input_channel_t *input, zpl_bool online);
extern int zpl_video_input_channel_online_get(zpl_video_input_channel_t *input, zpl_bool *online);

extern int zpl_video_input_channel_bindcount_set(zpl_video_input_channel_t *input, zpl_bool add);
extern int zpl_video_input_channel_bindcount_get(zpl_video_input_channel_t *input);


extern int zpl_video_input_channel_crop(zpl_video_input_channel_t *input, zpl_bool out, zpl_video_size_t cropsize);
extern int zpl_video_input_channel_mirror_flip(zpl_video_input_channel_t *input, zpl_bool mirror, zpl_bool flip);
extern int zpl_video_input_channel_ldc(zpl_video_input_channel_t *input, zpl_video_size_t cropsize);//畸形矫正
extern int zpl_video_input_channel_fish_eye(zpl_video_input_channel_t *input, void * LMF);//鱼眼校正
extern int zpl_video_input_channel_rotation(zpl_video_input_channel_t *input, zpl_uint32 rotation);
extern int zpl_video_input_channel_rotation_angle(zpl_video_input_channel_t *input, void *p);
extern int zpl_video_input_channel_spread(zpl_video_input_channel_t *input, void *p);


#ifdef ZPL_SHELL_MODULE
int zpl_video_input_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_INPUT_H__ */

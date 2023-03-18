
/*
 * zpl_video_input_pipe.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_INPUT_PIPE_H__
#define __ZPL_VIDEO_INPUT_PIPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>


typedef struct 
{
    NODE                    node;
    zpl_int32               input_dev;                     //底层设备编号
    zpl_int32               input_pipe;                    //底层硬件pipe
    zpl_socket_t                pipefd;
	zpl_video_size_t		input_size;		 //视频输入大小 
    zpl_video_size_t        sacle_size;      //缩放大小

    int (*input_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VI 接收到数据发送到下一级(如：VPSS)


    zpl_void                *t_read;
    zpl_void                *t_master;

    zpl_uint32              bindcount;      //绑定的数量
    zpl_bool                online;         //资源状态，在线不在线
    zpl_bool                hwbind;         //硬件绑定
    zpl_uint32              res_flag;    //资源标志 
#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32              dbg_send_count;
    zpl_uint32              dbg_recv_count;    
#endif  
}zpl_meida_video_input_pipe_t;


int zpl_meida_video_input_pipe_init(void);

int zpl_meida_video_input_pipe_create(zpl_int32 , zpl_int32 );
int zpl_meida_video_input_pipe_destroy(zpl_int32 , zpl_int32 );
zpl_meida_video_input_pipe_t * zpl_meida_video_input_pipe_lookup(zpl_int32 , zpl_int32 );
int zpl_meida_video_input_pipe_channel_count(zpl_int32 );

int zpl_meida_video_input_pipe_read_start(zpl_void *master, zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_read_stop(zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_sendto(zpl_meida_video_input_pipe_t *input,  void *p, zpl_int timeout);
int zpl_meida_video_input_pipe_hal_create(zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_start(zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_stop(zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_hal_destroy(zpl_meida_video_input_pipe_t *input);
int zpl_meida_video_input_pipe_online_set(zpl_meida_video_input_pipe_t *input, zpl_bool online);
int zpl_meida_video_input_pipe_online_get(zpl_meida_video_input_pipe_t *input, zpl_bool *online);

int zpl_meida_video_input_pipe_bindcount_set(zpl_meida_video_input_pipe_t *input, zpl_bool add);
int zpl_meida_video_input_pipe_bindcount_get(zpl_meida_video_input_pipe_t *input);
#ifdef ZPL_SHELL_MODULE
int zpl_meida_video_input_pipe_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_INPUT_PIPE_H__ */
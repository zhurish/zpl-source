
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



typedef struct 
{
    zpl_int32               mipidev;
    zpl_int32               snsdev;
    zpl_int32               snstype;
}zpl_video_inputdev_t;


typedef struct 
{
    NODE                    node;
    zpl_int32               devnum;           //底层设备编号
    zpl_int32               input_chn;        //底层物理通道号
    zpl_int32               input_extchn;     //底层扩展通道号
    zpl_int32               input_pipe;

    zpl_video_inputdev_t    inputdev;

    zpl_socket_t            pipefd;
    zpl_socket_t            chnfd;

	zpl_video_size_t		input_size;		 //视频输入大小 
    zpl_video_size_t		output_size;	 //视频输出大小 


    int (*inputchn_frame_handle)(void *, zpl_uint32, zpl_video_size_t);//应用层处理回调(添加OSD等)

    zpl_media_hardadap_lst_t   callback; //VI 接收到数据发送到下一级(如：VPSS)

    zpl_void                *t_read;
    zpl_void                *t_master;
    zpl_void                *dest_output;

    volatile zpl_uint32     reference;      //绑定的数量
    zpl_bool                online;         //资源状态，在线不在线
    zpl_bool                hwbind;         //硬件绑定

    zpl_uint32              flags; 
    zpl_uint32              input_pipe_bind_chn; //资源标志 
#ifdef ZPL_HISIMPP_HWDEBUG
    zpl_uint32              dbg_send_count;
    zpl_uint32              dbg_recv_count;    
#endif
}zpl_media_video_inputchn_t;


extern int zpl_media_video_inputchn_init(void);
extern zpl_media_video_inputchn_t * zpl_media_video_inputchn_create(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel, zpl_video_size_t output_size);
extern int zpl_media_video_inputchn_hal_param(zpl_media_video_inputchn_t *inputchn, int snstype, int mipidev, int snsdev);
extern int zpl_media_video_inputchn_hal_create(zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_destroy(zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_hal_destroy(zpl_media_video_inputchn_t *inputchn);
extern zpl_media_video_inputchn_t * zpl_media_video_inputchn_lookup(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel);
extern int zpl_media_video_inputchn_outputsize_get(zpl_int32 devnum, zpl_int32 input_pipe, zpl_int32 input_channel, zpl_video_size_t *output_size);

extern zpl_bool zpl_media_video_inputchn_state_check(zpl_media_video_inputchn_t *inputchn, int bit);

extern int zpl_media_video_input_pipe_lookup(zpl_int32 input_pipe);
extern int zpl_media_video_inputchn_start(zpl_void *master, zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_stop(zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_thread(zpl_media_video_inputchn_t *inputchn, zpl_bool start);
extern int zpl_media_video_inputchn_connect(zpl_media_video_inputchn_t *inputchn, zpl_int32 vpss_group, zpl_int32 vpss_channel, zpl_bool hwbind);

extern int zpl_media_video_inputchn_sendto(zpl_media_video_inputchn_t *inputchn,  void *p, zpl_int timeout);

extern int zpl_media_video_inputchn_addref(zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_delref(zpl_media_video_inputchn_t *inputchn);
extern int zpl_media_video_inputchn_reference(zpl_media_video_inputchn_t *inputchn);


extern int zpl_media_video_inputchn_crop(zpl_media_video_inputchn_t *inputchn, zpl_bool out, zpl_video_size_t cropsize);
extern int zpl_media_video_inputchn_mirror_flip(zpl_media_video_inputchn_t *inputchn, zpl_bool mirror, zpl_bool flip);
extern int zpl_media_video_inputchn_ldc(zpl_media_video_inputchn_t *inputchn, zpl_video_size_t cropsize);//畸形矫正
extern int zpl_media_video_inputchn_fish_eye(zpl_media_video_inputchn_t *inputchn, void * LMF);//鱼眼校正
extern int zpl_media_video_inputchn_rotation(zpl_media_video_inputchn_t *inputchn, zpl_uint32 rotation);
extern int zpl_media_video_inputchn_rotation_angle(zpl_media_video_inputchn_t *inputchn, void *p);
extern int zpl_media_video_inputchn_spread(zpl_media_video_inputchn_t *inputchn, void *p);


#ifdef ZPL_SHELL_MODULE
int zpl_media_video_inputchn_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_INPUT_H__ */

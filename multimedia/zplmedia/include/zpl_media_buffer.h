/*
 * zpl_media_buffer.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_BUFFER_H__
#define __ZPL_MEDIA_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>


#define ZPL_MEDIA_BUFFER_DEBUG_ONFILE
//epoll_dispatch
//dispatch
//Scheduler
#define ZPL_MEDIA_BUFFER_FRAME_MAXSIZE		(1920*1080*3)//(BMP位图)
#define ZPL_MEDIA_BUFFER_FRAME_CACHESIZE	(2)//(临时缓存2帧)

typedef enum
{
    ZPL_BUFFER_DATA_ENCODE      = 0x00,           //编码后
    ZPL_BUFFER_DATA_RECORD      = 0x01,			  //录像
    ZPL_BUFFER_DATA_CAPTURE     = 0x02,           //抓拍
    ZPL_BUFFER_DATA_YUV420      = 0x04,		      //YUV输入
    ZPL_BUFFER_DATA_YUV422      = 0x08,		      //
    ZPL_BUFFER_DATA_BMP         = 0x10,           //
} ZPL_BUFFER_DATA_E;


#pragma pack(1)

typedef struct
{
    zpl_uint16 	ID;                 //ID 通道号
    zpl_uint8 	buffer_type;        //音频视频
    zpl_uint8 	buffer_codec;       //编码类型
    zpl_uint8 	buffer_key;         //帧类型
    zpl_uint8 	buffer_rev;         //
    zpl_uint16 	buffer_flags;        //ZPL_BUFFER_DATA_E
    zpl_uint32 	buffer_timetick;    //时间戳毫秒
    zpl_uint32 	buffer_seq;         //序列号底层序列号
    zpl_uint32	buffer_len;         //帧长度
}zpl_media_buffer_hdr_t;
#pragma pack(0)

typedef struct
{
	NODE	node;
    zpl_uint16 	ID;                 //ID 通道号
    zpl_uint8 	buffer_type;        //音频视频
    zpl_uint8 	buffer_codec;       //编码类型
    zpl_uint8 	buffer_key;         //帧类型
    zpl_uint8 	buffer_rev;         //
    zpl_uint16 	buffer_flags;        //ZPL_BUFFER_DATA_E

    zpl_uint32 	buffer_timetick;    //时间戳 毫秒
    //zpl_uint32 	buffer_seq;         //序列号 底层序列号
    zpl_int32	buffer_len;         //当前缓存帧的长度
    zpl_int32	buffer_maxsize;		//buffer 的长度
    zpl_void	*buffer_data;       //buffer
} __attribute__ ((packed)) zpl_media_buffer_data_t, zpl_buffer_data_t ;

typedef struct 
{
    char    *name;
	os_sem_t	*sem;
	os_mutex_t	*mutex;
    zpl_uint32	maxsize;
	LIST	list;			//add queue
	LIST	rlist;			//ready queue
	LIST	ulist;			//unuse queue
	zpl_int32                  channel;	//通道号
	ZPL_MEDIA_CHANNEL_INDEX_E  channel_index;	    //码流类型
	zpl_void	*media_channel;
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
    FILE    *debug_fp;
#endif
}zpl_media_buffer_t;


extern char *zpl_media_timerstring(void);
extern zpl_uint32 zpl_media_timerstamp(void);
extern void zpl_media_msleep(zpl_uint32 msec);

extern zpl_media_buffer_t *zpl_media_buffer_create(char *name, zpl_uint32 maxsize, zpl_bool sem, zpl_void *chn);

extern zpl_media_buffer_data_t * zpl_media_buffer_data_malloc(zpl_media_buffer_t *queue, ZPL_MEDIA_E type, ZPL_BUFFER_DATA_E flag, zpl_uint32 len);
extern zpl_media_buffer_data_t * zpl_media_buffer_data_clone(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
extern int zpl_media_buffer_data_copy(zpl_media_buffer_data_t *dst, zpl_media_buffer_data_t *src);
extern int zpl_media_buffer_data_free(zpl_media_buffer_data_t *data);

extern int zpl_media_buffer_enqueue(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
extern zpl_media_buffer_data_t * zpl_media_buffer_dequeue(zpl_media_buffer_t *queue);
extern int zpl_media_buffer_finsh(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);

extern int zpl_media_buffer_destroy(zpl_media_buffer_t *queue);
extern int zpl_media_buffer_data_append(zpl_media_buffer_data_t *bufdata, uint8_t *data, uint32_t len);
extern int zpl_media_buffer_add(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
extern int zpl_media_buffer_put(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
extern zpl_media_buffer_data_t *zpl_media_buffer_get(zpl_media_buffer_t *queue);
extern int zpl_media_buffer_add_and_distribute(zpl_media_buffer_t *queue, zpl_media_buffer_data_t *data);
extern int zpl_media_buffer_distribute(zpl_media_buffer_t *queue);
#ifdef ZPL_MEDIA_BUFFER_DEBUG_ONFILE
extern int zpl_media_buffer_debug_onfile_close(zpl_media_buffer_t *queue);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_BUFFER_H__ */

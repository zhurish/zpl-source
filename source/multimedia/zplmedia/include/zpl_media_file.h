#ifndef __ZPL_MEDIA_FILE_H__
#define __ZPL_MEDIA_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_media_extradata.h>
#include <zpl_media_buffer.h>
#include <zpl_media_codec.h>
#ifdef ZPL_BUILD_LINUX
#define     RTP_MEDIA_BASE_PATH     "/home/zhurish/workspace/working/zpl-source/source/multimedia/zplmedia/"
//#define     RTP_MEDIA_BASE_PATH     "/mnt/hgfs/ubuntu-share/qt-project/live555-test/"
#else
#define     RTP_MEDIA_BASE_PATH     "D:/qt-project/live555-test/"
#endif


#define ZPL_MEDIA_FILE_TASK


#pragma pack(1)

typedef struct
{
    uint8_t  hdrstr[4];
    zpl_video_codec_t video;
    zpl_audio_codec_t audio;
    uint32_t    begintime;
    uint32_t    endtime;
    uint32_t    videoframe;
    uint32_t    audioframe;
}zpl_media_filedesc_t;

typedef struct
{
# if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t     video:4;
    uint8_t     audio:4;
#else
    uint8_t     audio:4;
    uint8_t     video:4;
#endif
    uint32_t    video_len;  //缓冲区大小
    uint32_t    audio_len;  //缓冲区大小
}zpl_packet_head_t;
#pragma pack(0)



typedef struct zpl_media_file_s zpl_media_file_t;

//typedef int         (*zpl_media_frame_func)(zpl_media_file_t*, zpl_framepacket_t *);

struct zpl_media_file_s
{
    uint8_t     filename[128];
    FILE        *fp;
    int         file_size;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    uint32_t    offset_len;
    int         msec;           //定时间隔
#ifndef ZPL_MEDIA_FILE_TASK
    void        *t_master;
    void        *t_read;
#endif
    zpl_media_filedesc_t filedesc;

    zpl_bool     b_video;
    zpl_bool     b_audio;

    uint32_t    flags;

    zpl_uint32  last_ts;
#ifdef ZPL_MEDIA_FILE_TASK
    zpl_taskid_t    taskid;
    uint32_t    run;
#endif
    uint32_t    pack_seq;
    uint32_t    cnt;
    void        *pdata;                     //稀有数据
    zpl_skbqueue_t    *buffer_queue;              //消息队列
    zpl_void    *parent;                    //父节点
    
    zpl_packet_head_t tmphead;
    zpl_media_bufcache_t tmppacket;

    int         (*get_frame)(zpl_media_file_t*, zpl_media_bufcache_t *);//读取一帧数据回调函数
    int         (*get_extradata)(zpl_media_file_t*, zpl_video_extradata_t *);//读取额外数据回调函数
    int         (*put_frame)(zpl_media_file_t*, zpl_media_bufcache_t *);
    int         (*put_extradata)(zpl_media_file_t*, zpl_video_extradata_t *);
};


extern int zpl_media_file_update(zpl_media_file_t * channel, bool add);

extern int zpl_media_file_start(zpl_media_file_t *media, bool start);
extern int zpl_media_file_master(zpl_media_file_t *file, void *master, int msec);

extern zpl_media_file_t *zpl_media_file_create(const char *name, const char *op);
extern int zpl_media_file_destroy(zpl_media_file_t *file);
extern int zpl_media_file_open(zpl_media_file_t *file);
extern int zpl_media_file_close(zpl_media_file_t *file);
extern int zpl_media_file_write(zpl_media_file_t *file, zpl_skbuffer_t *bufdata);
extern int zpl_media_file_write_data(zpl_media_file_t *file, uint8_t *data, uint32_t len);
extern int zpl_media_file_read(zpl_media_file_t *file, zpl_skbuffer_t *bufdata);

extern int zpl_media_file_codecdata(zpl_media_file_t *file, zpl_bool video, void *codec);
extern int zpl_media_file_writehdr(zpl_media_file_t *file, zpl_media_filedesc_t *hdr);
extern int zpl_media_file_readhdr(zpl_media_file_t *file, zpl_media_filedesc_t *hdr);

extern int zpl_media_file_extradata(zpl_media_file_t *file, zpl_video_extradata_t *extradata);
extern int zpl_media_file_pdata(zpl_media_file_t *file, void *pdata);
extern int zpl_media_file_interval(zpl_media_file_t *file, int interval);

extern int zpl_media_file_get_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *));
extern int zpl_media_file_get_extradata_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *));

extern int zpl_media_file_put_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *));
extern int zpl_media_file_put_extradata_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *));


#ifdef __cplusplus
}
#endif

#endif /* __ZPL_MEDIA_FILE_H__ */

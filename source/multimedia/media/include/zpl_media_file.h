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


#define ZPL_MEDIA_FILE_NAME_MAX 128
#pragma pack(1)

typedef struct
{
    uint8_t  hdrstr[4];
    zpl_video_codec_t video;
    zpl_audio_codec_t audio;
}zpl_media_filedesc_t;
#pragma pack(0)


typedef int (* media_get_frame_hander)(FILE *, zpl_media_bufcache_t*);
typedef int (* media_put_frame_hander)(FILE *, char *, int);

typedef struct zpl_media_file_s zpl_media_file_t;


struct zpl_media_file_s
{
    uint8_t     filename[ZPL_MEDIA_FILE_NAME_MAX];
    FILE        *fp;
    int         file_size;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)

    zpl_media_filedesc_t filedesc;

    zpl_bool     b_video;
    zpl_bool     b_audio;
    zpl_bool     b_create;

    os_mutex_t *_mutex;
    zpl_media_bufcache_t bufcache;
    media_get_frame_hander get_frame;            // 读取一帧数据回调函数
    media_put_frame_hander put_frame;
};

#define zpl_media_file_getptr(m)             (((zpl_media_file_t*)m))

extern char *zpl_media_file_basename(const char *name);

extern zpl_media_file_t *zpl_media_file_create(zpl_media_channel_t *chn, const char *name);
extern zpl_media_file_t *zpl_media_file_open(const char *name);
extern int zpl_media_file_destroy(zpl_media_file_t *media_file);
extern int zpl_media_file_reopen(zpl_media_file_t *media_file);
extern int zpl_media_file_close(zpl_media_file_t *media_file);
extern int zpl_media_file_write(zpl_media_file_t *media_file, zpl_skbuffer_t *bufdata);
extern int zpl_media_file_check(zpl_media_file_t *media_file, const char *name);
extern int zpl_media_file_lookup(const char *name);
extern int zpl_media_filedesc_create(zpl_media_file_t *media_file);
extern int zpl_media_file_codecdata(zpl_media_file_t *media_file, zpl_bool video, void *codec);

extern int zpl_media_file_get_frame_callback(zpl_media_file_t *media_file, int (*func)(FILE*, zpl_media_bufcache_t *));
extern int zpl_media_file_put_frame_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, char *, int));


extern int zpl_media_file_read(zpl_media_file_t *media_file, zpl_media_bufcache_t *bufcache);
extern int zpl_media_file_extradata(zpl_media_file_t *media_file, zpl_video_extradata_t *extradata);



#ifdef __cplusplus
}
#endif

#endif /* __ZPL_MEDIA_FILE_H__ */

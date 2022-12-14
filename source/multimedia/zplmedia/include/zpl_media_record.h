#ifndef __ZPL_MEDIA_RECORD_H__
#define __ZPL_MEDIA_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_media_channel.h>
#include <zpl_media_file.h>


typedef struct zpl_media_record_s
{
    zpl_uint32          cbid;
    void                *event_queue;
    void                *buffer_queue;
    zpl_media_file_t    *record_file;
}zpl_media_record_t;

int zpl_media_channel_record_enable(zpl_media_channel_t *mediachn, zpl_bool enable);
zpl_bool zpl_media_channel_record_state(zpl_media_channel_t *mediachn);

#ifdef __cplusplus
}
#endif
#endif // __ZPL_MEDIA_RECORD_H__

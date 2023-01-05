#ifndef __ZPL_MEDIA_CAPTURE_H__
#define __ZPL_MEDIA_CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_media_channel.h>
#include <zpl_media_image.h>

typedef struct zpl_media_capture_s
{
    void                *event_queue;
    void                *images_queue;
   
}zpl_media_capture_t;

extern int zpl_media_channel_capture_enable(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_bool enable);
extern zpl_bool zpl_media_channel_capture_state(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index);

extern int zpl_media_channel_capture_image_add(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, const zpl_media_image_t *bufdata);
extern int zpl_media_channel_capture_imagedata_add(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_size_t vsize, 
    ZPL_COLOR_SPACE color, zpl_uint8 quality, uint8_t *img, zpl_uint32 len);

#ifdef __cplusplus
}
#endif
#endif // __ZPL_MEDIA_CAPTURE_H__

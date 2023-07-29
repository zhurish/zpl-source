#ifndef __ZPL_MEDIA_RECORD_H__
#define __ZPL_MEDIA_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif




typedef struct zpl_media_record_s
{
    zpl_media_file_t    *record_file;
    zpl_uint32          record_frame;
}zpl_media_record_t;

int zpl_media_channel_record_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable);
zpl_bool zpl_media_channel_record_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

/*
int zpl_media_channel_proxy_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable);
zpl_bool zpl_media_channel_proxy_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);

int zpl_media_channel_multicast_rtp_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable);
zpl_bool zpl_media_channel_multicast_rtp_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index);
*/

#ifdef __cplusplus
}
#endif
#endif // __ZPL_MEDIA_RECORD_H__

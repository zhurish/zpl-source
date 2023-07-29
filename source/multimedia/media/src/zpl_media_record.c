#include "zpl_media.h"
#include "zpl_media_internal.h"

#define zpl_media_recordgetptr(m)             (((zpl_media_record_t*)m->p_record.param))



static char * zpl_media_channel_record_filename(zpl_media_channel_t *mediachn)
{
    static zpl_uint32 data[128];
    os_memset(data, 0, sizeof(data));
    sprintf(data, "%d-%d-%s-%s", mediachn->channel, mediachn->channel_index, os_time_fmt("filename",0),
        (mediachn->media_type==ZPL_MEDIA_VIDEO)?zpl_media_codec_name(mediachn->media_param.video_media.codec.codectype):
        zpl_media_codec_name(mediachn->media_param.audio_media.codec.codectype));
    return data;
}

static int zpl_media_buffer_data_record(zpl_media_channel_t *mediachn,
        const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    zpl_media_record_t *record = pVoidUser;
    if(mediachn == NULL || bufdata == NULL || pVoidUser == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);    
    if(mediachn && mediachn->p_record.enable && record)
    {
        if(zpl_media_file_write(record->record_file, bufdata))
            record->record_frame++;
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return OK;
}
int zpl_media_channel_record_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable)
{
    zpl_media_channel_t *mediachn = NULL;
    zpl_media_record_t *record = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    // 通道使能录像
    if(enable == zpl_true)
    {
        record = mediachn->p_record.param = malloc(sizeof(zpl_media_record_t));
        if(record == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        memset(record, 0, sizeof(zpl_media_record_t));    
        record->record_file = zpl_media_file_create(mediachn, zpl_media_channel_record_filename(mediachn));
        if (record->record_file == NULL)
        {
            free(record);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        } 
        mediachn->p_record.cbid = zpl_media_channel_client_add(channel, channel_index, zpl_media_buffer_data_record, mediachn->p_record.param);
        if(mediachn->p_record.cbid && zpl_media_channel_client_start(channel, channel_index, mediachn->p_record.cbid, zpl_true) != OK)
        { 
            zpl_media_file_close(record->record_file);
            zpl_media_file_remove(record->record_file);
            zpl_media_file_destroy(record->record_file);
            free(record);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        mediachn->p_record.enable = enable; 
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_record.enable = enable; 
        if(mediachn->p_record.cbid)
        { 
            zpl_media_channel_client_start(channel, channel_index, mediachn->p_record.cbid, zpl_false);
            zpl_media_channel_client_del(channel, channel_index, mediachn->p_record.cbid);
        }
        mediachn->p_record.cbid = 0;
        if(mediachn->p_record.param) 
        {
            record = mediachn->p_record.param;
            zpl_media_file_close(record->record_file);
            if(record->record_frame == 0)
                zpl_media_file_remove(record->record_file);
            zpl_media_file_destroy(record->record_file);
            free(record);
            mediachn->p_record.param = NULL;
        }
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}


zpl_bool zpl_media_channel_record_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = mediachn->p_record.enable;
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;
}


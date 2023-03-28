#include "zpl_media_record.h"
#include "zpl_media_event.h"
#include "zpl_media_buffer.h"

#define zpl_media_recordgetptr(m)             (((zpl_media_record_t*)m->p_record.param))

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
static int media_buffer_data_record_handle(zpl_media_event_t *event)
{
    zpl_media_record_t *record = event->event_data;
    zpl_skbuffer_t *bufdata = zpl_skbqueue_get(record->buffer_queue);
    if(record)
        record->evid = 0;
    while(record && bufdata)
    {
        //memcpy(&record->record_file->packet.data, bufdata, sizeof(zpl_skbuffer_t));
        //record->record_file->packet.data.buffer_data = bufdata->buffer_data;
        zpl_media_file_write(record->record_file, bufdata);
        zpl_skbqueue_finsh(record->buffer_queue, bufdata);
        bufdata = zpl_skbqueue_get(record->buffer_queue);
    }
    return OK;
}

static int zpl_media_buffer_data_record(zpl_media_channel_t *mediachn,
        const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    zpl_media_record_t *record = pVoidUser;
    if(mediachn == NULL || bufdata == NULL || pVoidUser == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);    
    if(mediachn && mediachn->p_record.enable && record && record->event_queue)
    {
        zpl_skbuffer_t *rdata = zpl_skbuffer_clone(record->buffer_queue, bufdata);
        if(rdata)
        {
            zpl_skbqueue_add(record->buffer_queue, rdata);

            if(record->evid == 0)
                record->evid = zpl_media_event_register(record->event_queue, ZPL_MEDIA_GLOAL_VIDEO_ENCODE,  ZPL_MEDIA_EVENT_RECORD,
                    media_buffer_data_record_handle, record);
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return OK;
}
#endif

static char * zpl_media_channel_record_filename(zpl_media_channel_t *mediachn)
{
    static zpl_uint32 data[128];
    os_memset(data, 0, sizeof(data));
    sprintf(data, "%d-%d-%s-%s", mediachn->channel, mediachn->channel_index, os_time_fmt("filename",0),
        (mediachn->media_type==ZPL_MEDIA_VIDEO)?zpl_media_codec_name(mediachn->media_param.video_media.codec.codectype):
        zpl_media_codec_name(mediachn->media_param.audio_media.codec.codectype));
    return data;
}

#ifdef ZPL_MEDIA_QUEUE_DISTPATH
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
        record->event_queue = zpl_media_event_default();
		record->buffer_queue = zpl_skbqueue_create(os_name_format("record-%d/%d", mediachn->channel, mediachn->channel_index), 
            ZPL_MEDIA_BUFFER_FRAME_CACHESIZE, zpl_false);
		if (record->buffer_queue == NULL)
		{
			os_free(record);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
		zpl_skbqueue_attribute_set(record->buffer_queue, ZPL_SKBQUEUE_FLAGS_LIMIT_MAX);
        
        record->record_file = zpl_media_file_create(mediachn, zpl_media_channel_record_filename(mediachn));
        if (record->record_file == NULL)
        {
            zpl_skbqueue_destroy(record->buffer_queue);
            record->buffer_queue = NULL;
            free(record);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        mediachn->p_record.cbid = zpl_media_channel_client_add(mediachn->channel, mediachn->channel_index, zpl_media_buffer_data_record, mediachn->p_record.param);
        mediachn->p_record.enable = enable; 
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_record.enable = enable; 
        zpl_media_channel_client_del(mediachn->channel, mediachn->channel_index, mediachn->p_record.cbid);
        
        mediachn->p_record.cbid = 0;
        if(mediachn->p_record.param) 
        {
            record = mediachn->p_record.param;
            if(record->evid)
            {
                zpl_media_event_del(record->event_queue, record->evid);
                record->evid = 0;
            }
            zpl_media_file_close(record->record_file);
            zpl_media_file_destroy(record->record_file);
            zpl_skbqueue_destroy(record->buffer_queue);
            record->buffer_queue = NULL;
            free(record);
            mediachn->p_record.param = NULL;
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}
#else
static int zpl_media_buffer_data_record(zpl_media_channel_t *mediachn,
        const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    zpl_media_record_t *record = pVoidUser;
    if(mediachn == NULL || bufdata == NULL || pVoidUser == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);    
    if(mediachn && mediachn->p_record.enable && record && record->event_queue)
    {
        //memcpy(&record->record_file->packet.data, bufdata, sizeof(zpl_skbuffer_t));
        //record->record_file->packet.data.buffer_data = bufdata->buffer_data;
        zpl_media_file_write(record->record_file, bufdata);
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
        record->event_queue = zpl_media_event_default();
        record->record_file = zpl_media_file_create(mediachn, zpl_media_channel_record_filename(mediachn));
        if (record->record_file == NULL)
        {
            free(record);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        mediachn->p_record.cbid = zpl_media_channel_client_add(mediachn->channel, mediachn->channel_index, zpl_media_buffer_data_record, mediachn->p_record.param);
        mediachn->p_record.enable = enable; 
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_record.enable = enable; 
        zpl_media_channel_client_del(mediachn->channel, mediachn->channel_index, mediachn->p_record.cbid);
        
        mediachn->p_record.cbid = 0;
        if(mediachn->p_record.param) 
        {
            record = mediachn->p_record.param;
            zpl_media_file_close(record->record_file);
            zpl_media_file_destroy(record->record_file);
            free(record);
            mediachn->p_record.param = NULL;
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}
#endif

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


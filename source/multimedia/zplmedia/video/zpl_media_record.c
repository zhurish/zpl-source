#include "zpl_media_record.h"
#include "zpl_media_event.h"
#include "zpl_media_buffer.h"



static int media_buffer_data_record_handle(zpl_media_event_t *event)
{
    zpl_media_record_t *record = event->event_data;
    zpl_skbuffer_t *bufdata = zpl_skbqueue_get(record->buffer_queue);
    while(bufdata)
    {
        //memcpy(&record->record_file->packet.data, bufdata, sizeof(zpl_skbuffer_t));
        //record->record_file->packet.data.buffer_data = bufdata->buffer_data;
        memset(&record->record_file->tmphead, 0, sizeof(zpl_packet_head_t));
        if(bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_VIDEO)
        {
            record->record_file->tmphead.video = 1;
            record->record_file->tmphead.audio = 0;
            record->record_file->tmphead.video_len = bufdata->skb_len;
            record->record_file->tmphead.audio_len = 0;
        }
        else if(bufdata->skb_header.media_header.buffer_type == ZPL_MEDIA_AUDIO)
        {
            record->record_file->tmphead.video = 0;
            record->record_file->tmphead.audio = 1;
            record->record_file->tmphead.video_len = 0;
            record->record_file->tmphead.audio_len = bufdata->skb_len;
        } 
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
    if(mediachn && mediachn->p_record.enable && record && record->event_queue)
    {
        zpl_skbuffer_t *rdata = zpl_skbuffer_clone(record->buffer_queue, bufdata);
        if(rdata)
        {
            zpl_skbqueue_add(record->buffer_queue, rdata);

            if(record->cbid == 0)
                record->cbid = zpl_media_event_register(record->event_queue, ZPL_MEDIA_NODE_RECORD,  ZPL_MEDIA_EVENT_RECORD,
                    media_buffer_data_record_handle, record);
        }
    }
    return OK;
}

static char * zpl_media_channel_record_filename(zpl_media_channel_t *mediachn)
{
    static zpl_uint32 data[128];
    os_memset(data, 0, sizeof(data));
    sprintf(data, "%d-%d-%s", mediachn->channel, mediachn->channel_index, os_time_fmt("filename",0));
    return data;
}

int zpl_media_channel_record_enable(zpl_media_channel_t *mediachn, zpl_bool enable)
{
    zpl_media_record_t *record = NULL;
          //通道使能录像
    if(enable == zpl_true)
    {
        record = mediachn->p_record.param = malloc(sizeof(zpl_media_record_t));
        if(record == NULL)
            return ERROR;
        memset(record, 0, sizeof(zpl_media_record_t));    
        record->event_queue = zpl_media_event_default();
		record->buffer_queue = zpl_skbqueue_create(os_name_format("record-%d/%d", mediachn->channel, mediachn->channel_index), 
            ZPL_MEDIA_BUFFER_FRAME_CACHESIZE, zpl_false);
		if (record->buffer_queue == NULL)
		{
			os_free(record);
            return ERROR;
        }
        record->record_file = zpl_media_file_create(zpl_media_channel_record_filename(mediachn), "a+");
        if (record->record_file == NULL)
        {
            zpl_skbqueue_destroy(record->buffer_queue);
            record->buffer_queue = NULL;
            free(record);
            return ERROR;
        }
        mediachn->p_record.cbid = zpl_media_channel_client_add(mediachn, zpl_media_buffer_data_record, mediachn->p_record.param);
        mediachn->p_record.enable = enable; 
        return OK;
    }
    else
    {
        mediachn->p_record.enable = enable; 
        zpl_media_channel_client_del(mediachn, mediachn->p_record.cbid);
        
        mediachn->p_record.cbid = 0;
        if(mediachn->p_record.param) 
        {
            record = mediachn->p_record.param;
            if(record->cbid)
            {
                zpl_media_event_del(record->event_queue, record->cbid);
                record->cbid = 0;
            }
            zpl_media_file_close(record->record_file);
            zpl_media_file_destroy(record->record_file);
            zpl_skbqueue_destroy(record->buffer_queue);
            record->buffer_queue = NULL;
            free(record);
            mediachn->p_record.param = NULL;
            return OK;
        }
    }
    return ERROR;
}


zpl_bool zpl_media_channel_record_state(zpl_media_channel_t *mediachn)
{
    return mediachn->p_record.enable;
}
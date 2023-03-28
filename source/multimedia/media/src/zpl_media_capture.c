#include "zpl_media_capture.h"
#include "zpl_media_event.h"
#include "zpl_media_image.h"
#include "zpl_media_buffer.h"
#include "zpl_media_internal.h"

#define zpl_media_capgetptr(m)             (((zpl_media_capture_t*)m->p_capture.param))

static int zpl_media_capture_hal_create(zpl_media_channel_t *chn)
{
    zpl_media_hwres_t *hwres = ZPL_MEDIA_HALRES_ID_LOAD(chn->channel, chn->channel_index, ALL);
    zpl_int32 venc_channel = -1;//ZPL_MEDIA_HALRES_ID_LOAD(chn->channel, chn->channel_index, CAPTURE_VENCCHN);
    if (hwres && hwres->capvencchn >= 0)
    {
        venc_channel = hwres->capvencchn;
        zpl_media_video_encode_t *video_encode = NULL;
        zpl_media_video_encode_t *main_video_encode = chn->media_param.video_media.halparam;
        zpl_media_video_vpsschn_t *video_vpsschn = NULL;
        video_encode = zpl_media_video_encode_create(venc_channel, zpl_true);
        if( video_encode == NULL)
        {
            if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
            {
                zm_msg_error("create video_encode channel(%d)", venc_channel);
            } 
            return ERROR;
        } 
        if (zpl_media_video_encode_frame_queue_set(venc_channel, chn->frame_queue) != OK)
        {
            zm_msg_error("can not set buffer queue for video_encode channel(%d)", venc_channel);
            zpl_media_video_encode_destroy(video_encode);
            return ERROR;
        }
        
        video_encode->b_capture = zpl_true;
        video_encode->media_channel = chn;
        video_encode->source_input = main_video_encode->source_input;
        video_encode->pCodec = &zpl_media_capgetptr(chn)->codec;
		zpl_media_capgetptr(chn)->codec.vidsize.width = chn->media_param.video_media.codec.vidsize.width;
		zpl_media_capgetptr(chn)->codec.vidsize.height = chn->media_param.video_media.codec.vidsize.height;
        zpl_media_capgetptr(chn)->halparam = video_encode;
        video_vpsschn = video_encode->source_input;
        zpl_media_video_encode_source_set(venc_channel, main_video_encode->source_input);
        zpl_media_video_vpsschn_connect(video_vpsschn->vpss_group, video_vpsschn->vpss_channel, venc_channel, zpl_true);
        return OK;
    }
    return ERROR;
}

static int zpl_media_capture_hal_destroy(zpl_media_channel_t *chn)
{
    zpl_media_video_encode_t *video_encode = NULL;
    zpl_media_video_vpsschn_t *video_vpsschn = NULL;
    video_encode = zpl_media_capgetptr(chn)->halparam;
    if (video_encode == NULL || video_encode->source_input == NULL)
    {
        if(ZPL_MEDIA_DEBUG(CHANNEL, ERROR))
        {
            zm_msg_error("video_encode channel or vpss NULL");
        } 
        return ERROR;
    } 
    video_vpsschn = video_encode->source_input;
    zpl_media_video_vpsschn_connect(video_vpsschn->vpss_group, video_vpsschn->vpss_channel, video_encode->venc_channel, zpl_false);
    zpl_media_video_encode_source_set(video_encode->venc_channel, NULL);
    zpl_media_video_encode_destroy(video_encode);
    zpl_media_capgetptr(chn)->halparam = NULL;
    return OK;
}

int zpl_media_channel_capture_create(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = ERROR;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = zpl_media_capture_hal_create(mediachn);
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;  
}

int zpl_media_channel_capture_destroy(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = ERROR;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = zpl_media_capture_hal_destroy(mediachn);
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;  
}

int zpl_media_channel_capture_start(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = 0;
    zpl_media_video_encode_t *video_encode = NULL;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL || zpl_media_capgetptr(mediachn)->halparam == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    video_encode = zpl_media_capgetptr(mediachn)->halparam;
    ret = zpl_media_video_encode_start(video_encode->t_master, video_encode);
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;  
}

int zpl_media_channel_capture_stop(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    int ret = 0;
    zpl_media_video_encode_t *video_encode = NULL;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL || zpl_media_capgetptr(mediachn)->halparam == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    video_encode = zpl_media_capgetptr(mediachn)->halparam;
    ret = zpl_media_video_encode_stop(video_encode);
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret; 
}

static int media_buffer_data_capture_handle(zpl_media_event_t *event)
{
    zpl_media_capture_t *capture = event->event_data;
    zpl_media_image_t *bufdata = zpl_media_image_dequeue(capture->images_queue);
    if(capture)
        capture->evid = 0;
    while(capture && bufdata)
    {
        //TODO
        zpl_media_image_finsh(capture->images_queue, bufdata);
        bufdata = zpl_media_image_dequeue(capture->images_queue);
    }
    return OK;
}

static int zpl_media_buffer_data_capture(zpl_media_channel_t *mediachn,
        const zpl_media_image_t *bufdata,  void *pVoidUser)
{
    zpl_media_capture_t *capture = pVoidUser;
    if(mediachn == NULL || bufdata == NULL || pVoidUser == NULL)
        return 0;
    if(mediachn && mediachn->p_capture.enable && capture && capture->event_queue)
    {
        zpl_media_image_t *rdata = zpl_media_image_clone(capture->images_queue, bufdata);
        if(rdata)
        {
            zpl_media_image_enqueue(capture->images_queue, rdata);
            if(zpl_media_capgetptr(mediachn)->evid <= 0)
                zpl_media_capgetptr(mediachn)->evid = zpl_media_event_register(capture->event_queue, ZPL_MEDIA_GLOAL_VIDEO_ENCODE,  ZPL_MEDIA_EVENT_CAPTURE, 
                media_buffer_data_capture_handle, capture);
        }
    }
    return OK;
}


zpl_bool zpl_media_channel_capture_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = mediachn->p_capture.enable;
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;    
}

int zpl_media_channel_capture_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable)
{
    zpl_media_capture_t *capture = NULL;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn); 
          //通道使能录像
    if(enable == zpl_true)
    {
        capture = mediachn->p_capture.param = malloc(sizeof(zpl_media_capture_t));
        if(capture == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        memset(capture, 0, sizeof(zpl_media_capture_t));    
        capture->event_queue = zpl_media_event_default();
		capture->images_queue = zpl_media_imglst_create("capture", ZPL_MEDIA_BUFFER_FRAME_CACHESIZE);
		if (capture->images_queue == NULL)
		{
			os_free(capture);
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        mediachn->p_capture.enable = enable; 
        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_capture.enable = enable; 
        zpl_media_capgetptr(mediachn)->evid = 0;
        if(mediachn->p_capture.param) 
        {
            capture = mediachn->p_capture.param;
            if(zpl_media_capgetptr(mediachn)->evid)
            {
                zpl_media_event_del(capture->event_queue, zpl_media_capgetptr(mediachn)->evid);
                zpl_media_capgetptr(mediachn)->evid = 0;
            }
            zpl_media_imglst_destroy(capture->images_queue);
            free(capture);
            mediachn->p_capture.param = NULL;
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}

int zpl_media_channel_capture_image_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index,const zpl_media_image_t *bufdata)
{
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);    
    if(mediachn && mediachn->p_capture.param && mediachn->p_capture.enable)
    {
        zpl_media_buffer_data_capture(mediachn, bufdata,  mediachn->p_capture.param);
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn); 
    return OK;
}

int zpl_media_channel_capture_imagedata_add(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index,zpl_video_size_t vsize, 
    ZPL_COLOR_SPACE color, zpl_uint8 quality, uint8_t *img, zpl_uint32 len)
{
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;    
    if(mediachn && mediachn->p_capture.param && mediachn->p_capture.enable)
    {
        zpl_media_capture_t *capture = mediachn->p_capture.param;
        zpl_media_image_t * image = zpl_media_image_create(capture->images_queue,  vsize.width,  vsize.height,
                                                   color,  quality, img,  len);
        if(image)                                           
            zpl_media_buffer_data_capture(mediachn, image,  mediachn->p_capture.param);
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn); 
    return OK;
}

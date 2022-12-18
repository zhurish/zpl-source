#include "zpl_media_capture.h"
#include "zpl_media_event.h"
#include "zpl_media_image.h"
#include "zpl_media_buffer.h"

static int media_buffer_data_capture_handle(zpl_media_event_t *event)
{
    zpl_media_capture_t *capture = event->event_data;
    zpl_media_image_t *bufdata = zpl_media_image_dequeue(capture->images_queue);
    while(bufdata)
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
            if(mediachn->p_capture.cbid <= 0)
                mediachn->p_capture.cbid = zpl_media_event_register(capture->event_queue, ZPL_MEDIA_NODE_CAPTURE,  ZPL_MEDIA_EVENT_CAPTURE, 
                media_buffer_data_capture_handle, capture);
        }
    }
    return OK;
}

#if 0
static char * zpl_media_channel_capture_filename(zpl_media_channel_t *mediachn)
{
    static zpl_uint32 data[128];
    os_memset(data, 0, sizeof(data));
    sprintf(data, "%d-%d-%s", mediachn->channel, mediachn->channel_index, os_time_fmt("filename",0));
    return data;
}
#endif

zpl_bool zpl_media_channel_capture_state(zpl_media_channel_t *mediachn)
{
    return mediachn->p_capture.enable;
}

int zpl_media_channel_capture_enable(zpl_media_channel_t *mediachn, zpl_bool enable)
{
    zpl_media_capture_t *capture = NULL;
          //通道使能录像
    if(enable == zpl_true)
    {
        capture = mediachn->p_capture.param = malloc(sizeof(zpl_media_capture_t));
        if(capture == NULL)
            return ERROR;
        memset(capture, 0, sizeof(zpl_media_capture_t));    
        capture->event_queue = zpl_media_event_default();
		capture->images_queue = zpl_media_imglst_create("capture", ZPL_MEDIA_BUFFER_FRAME_CACHESIZE);
		if (capture->images_queue == NULL)
		{
			os_free(capture);
            return ERROR;
        }
        //mediachn->p_capture.cbid = zpl_media_channel_client_add(mediachn, zpl_media_buffer_data_capture, mediachn->p_capture.param);
        mediachn->p_capture.enable = enable; 
        return OK;
    }
    else
    {
        mediachn->p_capture.enable = enable; 
        //zpl_media_channel_client_del(mediachn, mediachn->p_capture.cbid);
        mediachn->p_capture.cbid = 0;
        if(mediachn->p_capture.param) 
        {
            capture = mediachn->p_capture.param;
            if(mediachn->p_capture.cbid)
            {
                zpl_media_event_del(capture->event_queue, mediachn->p_capture.cbid);
                mediachn->p_capture.cbid = 0;
            }
            zpl_media_imglst_destroy(capture->images_queue);
            free(capture);
            mediachn->p_capture.param = NULL;
            return OK;
        }
    }
    return ERROR;
}

int zpl_media_channel_capture_image_add(zpl_media_channel_t *mediachn, const zpl_media_image_t *bufdata)
{
    if(mediachn && mediachn->p_capture.param && mediachn->p_capture.enable)
    {
        zpl_media_buffer_data_capture(mediachn, bufdata,  mediachn->p_capture.param);
    }
    return OK;
}

int zpl_media_channel_capture_imagedata_add(zpl_media_channel_t *mediachn, zpl_video_size_t vsize, 
    ZPL_COLOR_SPACE color, zpl_uint8 quality, uint8_t *img, zpl_uint32 len)
{
    if(mediachn && mediachn->p_capture.param && mediachn->p_capture.enable)
    {
        zpl_media_capture_t *capture = mediachn->p_capture.param;
        zpl_media_image_t * image = zpl_media_image_create(capture->images_queue,  vsize.width,  vsize.height,
                                                   color,  quality, img,  len);
        if(image)                                           
            zpl_media_buffer_data_capture(mediachn, image,  mediachn->p_capture.param);
    }
    return OK;
}

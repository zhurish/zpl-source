#include "zpl_media_image.h"
#include "zpl_media_buffer.h"

int zpl_media_image_free(zpl_media_image_t *data)
{
    if(data)
    {
        if(data->image_data)
        {
            free(data->image_data);
        }
        free(data);
    }
    return OK;
}

zpl_media_imglst_t *zpl_media_imglst_create(char *name, zpl_uint32 maxsize)
{
    zpl_media_imglst_t *queue = malloc(sizeof(zpl_media_imglst_t));
    if(queue)
    {
        zpl_media_image_t * dnode = NULL;
        memset(queue, 0, sizeof(zpl_media_imglst_t));
        queue->name = strdup(name);
        queue->maxsize = maxsize;
        queue->mutex = os_mutex_name_init(os_name_format("%s-mutex", name));

        lstInitFree (&queue->ulist, zpl_media_image_free);
        lstInitFree (&queue->list, zpl_media_image_free);
        dnode = zpl_media_image_malloc(queue, ZPL_MEDIA_BUFFER_FRAME_MAXSIZE);
        if(dnode)
            lstAdd (&queue->ulist, dnode);
        dnode = zpl_media_image_malloc(queue, ZPL_MEDIA_BUFFER_FRAME_MAXSIZE);
        if(dnode)
            lstAdd (&queue->ulist, dnode);
        return queue;
    }
    return NULL;
}

int zpl_media_imglst_destroy(zpl_media_imglst_t *queue)
{
    //zpl_video_assert(queue);
    if(!queue)
        return ERROR;
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    lstFree(&queue->ulist);
    lstFree(&queue->list);
    if(queue->mutex)
        os_mutex_exit(queue->mutex);
    if(queue->name)
        free(queue->name);
    free(queue);
    return OK;
}

zpl_media_image_t * zpl_media_image_create(zpl_media_imglst_t *queue, zpl_uint32 width, zpl_uint32 height,
                                                  ZPL_COLOR_SPACE color, zpl_uint8 quality, uint8_t *img, zpl_uint32 len)
{
    zpl_media_image_t *bufdata = zpl_media_image_malloc(queue, len);
    if(bufdata)
    {
        bufdata->width = width;
        bufdata->height= height;
        bufdata->quality = quality;
        bufdata->color   = color;
        bufdata->image_len     = len;         //当前缓存帧的长度
        memset(bufdata->image_data, 0, bufdata->image_maxsize);
        memcpy(bufdata->image_data, img, len);
        return bufdata;
    }
    return NULL;
}

zpl_media_image_t * zpl_media_image_malloc(zpl_media_imglst_t *queue, zpl_uint32 len)
{
    NODE node;
    zpl_media_image_t *bufdata = NULL;
    //zpl_video_assert(queue);
    if(queue && queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    if(queue && lstCount(&queue->list) >= queue->maxsize)
    {
        if(queue->mutex)
            os_mutex_unlock(queue->mutex);
        return NULL;
    }
    if(queue)
    {
        for(bufdata = (zpl_media_image_t *)lstFirst(&queue->ulist); bufdata != NULL;
            bufdata = (zpl_media_image_t *)lstNext(&node))
        {
            node = bufdata->node;
            if(bufdata && bufdata->image_maxsize >= len)
            {
                lstDelete (&queue->ulist, (NODE *)bufdata);
                break;
            }
        }
    }
    if(bufdata == NULL)
    {
        bufdata = malloc(sizeof(zpl_media_image_t));
        if(bufdata)
        {
            memset(bufdata, 0, sizeof(zpl_media_image_t));
            bufdata->image_maxsize = ZPL_MEDIA_BUF_ALIGN(len);		//buffer 的长度
            bufdata->image_data = malloc(len);	//buffer
            if(bufdata->image_data == NULL)
            {
                memset(bufdata->image_data, 0, len);
                bufdata->image_maxsize = 0;
                free(bufdata);
                bufdata = NULL;
            }
        }
    }
    if(queue && queue->mutex)
        os_mutex_unlock(queue->mutex);
    return bufdata;
}

zpl_media_image_t * zpl_media_image_clone(zpl_media_imglst_t *queue, zpl_media_image_t *data)
{
    zpl_media_image_t *bufdata = NULL;
    bufdata = zpl_media_image_malloc(queue, data->image_maxsize);
    if(bufdata)
    {
        bufdata->width = data->width;
        bufdata->height= data->height;
        bufdata->quality = data->quality;
        bufdata->color   = data->color;
        bufdata->image_len     = data->image_len;         //当前缓存帧的长度
        bufdata->image_maxsize = data->image_maxsize;		//buffer 的长度
        memset(bufdata->image_data, 0, bufdata->image_maxsize);
        memcpy(bufdata->image_data, data->image_data, data->image_len);
        return bufdata;
    }
    return NULL;
}


int zpl_media_image_copy(zpl_media_image_t *dst, zpl_media_image_t *src)
{
    if(dst && src && src->image_data && src->image_len && (src->image_len <= src->image_maxsize))
    {
        if(dst->image_data)
        {
            if(src->image_maxsize > dst->image_maxsize)
            {
                dst->image_data = realloc(dst->image_data, src->image_maxsize);
                dst->image_maxsize = src->image_maxsize;		//buffer 的长度
            }
        }
        else
        {
            dst->image_data = malloc(src->image_maxsize);	//buffer
            dst->image_maxsize = src->image_maxsize;		//buffer 的长度
        }

        if(dst->image_data == NULL)
        {
            return 0;
        }
        dst->width = src->width;
        dst->height= src->height;
        dst->quality = src->quality;
        dst->color   = src->color;
        dst->image_len     = src->image_len;         //当前缓存帧的长度

        memset(dst->image_data, 0, dst->image_maxsize);
        memcpy(dst->image_data, src->image_data, src->image_len);
        return dst->image_len;
    }
    return 0;
}



int zpl_media_image_finsh(zpl_media_imglst_t *queue, zpl_media_image_t *data)
{
    //zpl_video_assert(queue);
    //zpl_video_assert(data);
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    lstAdd(&queue->ulist, data);
    if(queue->mutex)
        os_mutex_unlock(queue->mutex);
    return OK;
}


zpl_media_image_t * zpl_media_image_dequeue(zpl_media_imglst_t *queue)
{
    zpl_media_image_t *bufdata = NULL;
    //zpl_video_assert(queue);
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    if(lstCount(&queue->list))
    {
        bufdata = lstFirst(&queue->list);
        if(bufdata)
            lstDelete (&queue->list, (NODE *)bufdata);
    }
    if(queue->mutex)
        os_mutex_unlock(queue->mutex);
    return bufdata;
}


int zpl_media_image_enqueue(zpl_media_imglst_t *queue, zpl_media_image_t *data)
{
    //zpl_video_assert(queue);
    //zpl_video_assert(data);
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    lstAdd (&queue->list, data);
    if(queue->mutex)
        os_mutex_unlock(queue->mutex);
    return OK;
}


int zpl_media_image_add(zpl_media_imglst_t *queue, zpl_media_image_t *data)
{
    //zpl_video_assert(queue);
    //zpl_video_assert(data);
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    lstAdd (&queue->list, data);
    if(queue->mutex)
        os_mutex_unlock(queue->mutex);
    return OK;
}

zpl_media_image_t * zpl_media_image_get(zpl_media_imglst_t *queue)
{
    zpl_media_image_t *bufdata = NULL;
    //zpl_video_assert(queue);
    if(queue->mutex)
        os_mutex_lock(queue->mutex, OS_WAIT_FOREVER);
    if(lstCount(&queue->list))
    {
        bufdata = lstFirst(&queue->list);
        if(bufdata)
            lstDelete (&queue->list, (NODE *)bufdata);
    }
    if(queue->mutex)
        os_mutex_unlock(queue->mutex);
    return bufdata;
}

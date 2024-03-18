/*
 * zpl_video_encode.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"


zpl_media_global_t _media_global;


int zpl_media_global_init(void)
{
    memset(&_media_global, 0, sizeof(_media_global));
	if(os_mutex_name_init(&_media_global.video_encode_mutex, "video_encode_mutex") == OK)
	{
		lstInit(&_media_global.video_encode_list);
	}
	if(os_mutex_name_init(&_media_global.video_decode_mutex, "video_decode_mutex") == OK)
	{
		lstInit(&_media_global.video_decode_list);
	}
	if(os_mutex_name_init(&_media_global.video_output_mutex, "video_output_mutex") == OK)
	{
		lstInit(&_media_global.video_output_list);
	}
	if(os_mutex_name_init(&_media_global.vpss_channel_mutex, "vpss_channel_mutex") == OK)
	{
		lstInit(&_media_global.vpss_channel_list);
	}

	if(os_mutex_name_init(&_media_global.video_input_mutex, "video_input_mutex") == OK)
	{
		lstInit(&_media_global.video_input_list);
	}

	if(os_mutex_name_init(&_media_global.input_dev_mutex, "input_dev_mutex") == OK)
	{
		lstInit(&_media_global.input_dev_list);
	}
	if(os_mutex_name_init(&_media_global.audio_mutex, "audio_mutex") == OK)
	{
		lstInit(&_media_global.audio_list);
	}
	return OK;
}

int zpl_media_global_exit(void)
{
	if(os_mutex_destroy(&_media_global.video_encode_mutex) == OK)
	{
		lstFree(&_media_global.video_encode_list);
	}
	if(os_mutex_destroy(&_media_global.video_decode_mutex) == OK)
	{
		lstFree(&_media_global.video_decode_list);
	}
	if(os_mutex_destroy(&_media_global.video_output_mutex) == OK)
	{
		lstFree(&_media_global.video_output_list);
	}
	if(os_mutex_destroy(&_media_global.vpss_channel_mutex) == OK)
	{
		lstFree(&_media_global.vpss_channel_list);
	}
	if(os_mutex_destroy(&_media_global.video_input_mutex) == OK)
	{
		lstFree(&_media_global.video_input_list);
	}
	if(os_mutex_destroy(&_media_global.input_dev_mutex) == OK)
	{
		lstFree(&_media_global.input_dev_list);
	}
	if(os_mutex_destroy(&_media_global.audio_mutex) == OK)
	{
		lstFree(&_media_global.audio_list);
	}
	return OK;
}

static int zpl_media_global_list_node_get(ZPL_MEDIA_GLOBAL_E type, LIST **lst, os_mutex_t **mutex)
{
    switch(type)
    {
    case ZPL_MEDIA_GLOAL_VIDEO_DEV: 
        *lst = &_media_global.input_dev_list; 
        *mutex = &_media_global.input_dev_mutex;
        break;

    case ZPL_MEDIA_GLOAL_VIDEO_INPUT: 
        *lst = &_media_global.video_input_list; 
        *mutex = &_media_global.video_input_mutex;
        break;

    case ZPL_MEDIA_GLOAL_VIDEO_VPSS:  
        *lst = &_media_global.vpss_channel_list; 
        *mutex = &_media_global.vpss_channel_mutex;
        break;
    case ZPL_MEDIA_GLOAL_VIDEO_ENCODE:  
        *lst = &_media_global.video_encode_list; 
        *mutex = &_media_global.video_encode_mutex;
        break;
    case ZPL_MEDIA_GLOAL_VIDEO_DECODE:  
        *lst = &_media_global.video_decode_list; 
        *mutex = &_media_global.video_decode_mutex;
        break;
    case ZPL_MEDIA_GLOAL_VIDEO_OUTPUT:  
        *lst = &_media_global.video_output_list; 
        *mutex = &_media_global.video_output_mutex;
        break;

    case ZPL_MEDIA_GLOAL_AUDIO:  
        *lst = &_media_global.audio_list; 
        *mutex = &_media_global.audio_mutex;
        break;
    }
    return OK;
}

static int zpl_media_global_list_add(ZPL_MEDIA_GLOBAL_E type, void *node)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst && node)    
        lstAdd(_lst, (NODE *)node);
	if(_mutex)
		os_mutex_unlock(_mutex);
    return OK;    
}

static int zpl_media_global_list_del(ZPL_MEDIA_GLOBAL_E type, void *node)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst && node)    
        lstDelete(_lst, (NODE *)node);
	if(_mutex)
		os_mutex_unlock(_mutex);
    return OK;    
}

static void * zpl_media_global_list_lookup(ZPL_MEDIA_GLOBAL_E type, void *node)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    NODE    *gnode = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst && node)    
        gnode = lstLookup(_lst, (NODE *)node);
	if(_mutex)
		os_mutex_unlock(_mutex);
    return gnode;    
}

static int zpl_media_global_list_cmp(ZPL_MEDIA_GLOBAL_E type, int (*cmpfunc)(void*, void*))
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst)    
        _lst->cmp = cmpfunc;
	if(_mutex)
		os_mutex_unlock(_mutex);
    return OK;    
}

int zpl_media_global_lock(ZPL_MEDIA_GLOBAL_E type)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		;//os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    return OK;  
}
int zpl_media_global_unlock(ZPL_MEDIA_GLOBAL_E type)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if(_mutex)
		;//os_mutex_unlock(_mutex);
    return OK;  
}

int zpl_media_global_freeset(ZPL_MEDIA_GLOBAL_E type, void (*cmpfunc)(void*))
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst)    
        _lst->free = cmpfunc;
	if(_mutex)
		os_mutex_unlock(_mutex);
    return OK;  
}

int zpl_media_global_gkey_cmpset(ZPL_MEDIA_GLOBAL_E type, int (*cmpfunc)(void*, void*))
{
    return zpl_media_global_list_cmp(type, cmpfunc);
}

int zpl_media_global_del(ZPL_MEDIA_GLOBAL_E type, void *node)
{
    return zpl_media_global_list_del(type, node);
}

int zpl_media_global_add(ZPL_MEDIA_GLOBAL_E type, void *node)
{
    return zpl_media_global_list_add(type, node);
}

void * zpl_media_global_lookup(ZPL_MEDIA_GLOBAL_E type, int channel, int group, int ID)
{
    zpl_media_gkey_t mgkey;
    mgkey.channel = channel;
    mgkey.group = group;   
    mgkey.ID = ID;
    return zpl_media_global_list_lookup(type, &mgkey);
}

int zpl_media_global_get(ZPL_MEDIA_GLOBAL_E type, LIST **lst, os_mutex_t **mutex)
{
    return zpl_media_global_list_node_get( type, lst, mutex);
}

int zpl_media_global_foreach(ZPL_MEDIA_GLOBAL_E type, int (*func)(void*, void*), void *p)
{
    LIST *_lst = NULL;
    os_mutex_t *_mutex = NULL;
    NODE *node;
    zpl_media_global_list_node_get(type, &_lst, &_mutex);
	if (_mutex)
		os_mutex_lock(_mutex, OS_WAIT_FOREVER);
    if(_lst) 
    {   
        for (node = lstFirst(_lst); node != NULL; node = lstNext(node))
        {
            if (node && func)
            {
                (func)(node, p);
            }
        }
    }
	if(_mutex)
		os_mutex_unlock(_mutex);
    return OK;    
}
/*
 * zpl_media_client.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"


#define ZPL_MEDIA_CLIENT_INDEX_SET(n)  ((n) + 0XAEB0)
#define ZPL_MEDIA_CLIENT_INDEX_GET(n)  ((n) - 0XAEB0)



/* 多媒体数据接收客户端， 需要摄像头完成编码的音视频数据的模块当作多媒体客户端 */
zpl_bool zpl_media_client_lookup(zpl_media_client_t *client, zpl_media_buffer_handler cb_handler, void *pUser)
{
    zpl_uint32 i = 0;
    for(i = 0; i < ZPL_MEDIA_CLIENT_MAX; i++)
    {
        if(client[i].enable == zpl_true && client[i]._pmedia_buffer_handler == cb_handler)
        {
            if(pUser && client[i].pVoidUser && client[i].pVoidUser == pUser)
                return zpl_true;
            else if(pUser == NULL && client[i].pVoidUser == NULL && client[i].pVoidUser == pUser)
                return zpl_true;
        }
    }
    return zpl_false;
}

int zpl_media_client_add(zpl_media_client_t *client, zpl_media_buffer_handler cb_handler, void *pUser)
{
	zpl_uint32 i = 0;
    if(zpl_media_client_lookup(client, cb_handler, pUser))
        return ERROR;
	for(i = 0; i < ZPL_MEDIA_CLIENT_MAX; i++)
	{
		if(client[i].enable == zpl_false)
		{
			client[i].enable = zpl_true;
			client[i]._pmedia_buffer_handler = cb_handler;
			client[i].pVoidUser = pUser;

            return ZPL_MEDIA_CLIENT_INDEX_SET(i);
		}
	}
    return ERROR;
}

int zpl_media_client_del(zpl_media_client_t *client, zpl_int32 index)
{
    zpl_int32 i = ZPL_MEDIA_CLIENT_INDEX_GET(index);
	if(i >= 0 && i < ZPL_MEDIA_CLIENT_MAX)
	{
		if(client[i].enable == zpl_true)
		{
			client[i].enable = zpl_false;
			client[i]._pmedia_buffer_handler = NULL;
			client[i].pVoidUser = NULL;
            return OK;
		}
	}
    return ERROR;
}

int zpl_media_client_foreach(zpl_skbuffer_t *buffer_data, void *p)
{
	zpl_uint32 i = 0;
	zpl_media_client_t *client = NULL;
	zpl_media_channel_t *media_channel = NULL;

	media_channel = zpl_media_channel_lookup(ZPL_MEDIA_CHANNEL_GET_C(buffer_data->skb_header.media_header.ID), 
		ZPL_MEDIA_CHANNEL_GET_I(buffer_data->skb_header.media_header.ID));
    if(media_channel == NULL)
    {
        media_channel = zpl_media_channel_lookup_sessionID(buffer_data->skb_header.media_header.sessionID);
    }    
	if(media_channel)
	{
		client = (media_channel)->media_client;
		zpl_media_channel_extradata_update(buffer_data, media_channel);
	}	

	if(client)
	{
        //zpl_media_debugmsg_debug("======== zpl_media_client_foreach");
		for(i = 0; i < ZPL_MEDIA_CLIENT_MAX; i++)
		{
			if(client[i].enable == zpl_true && client[i]._pmedia_buffer_handler != NULL)
			{
                //zpl_media_debugmsg_debug("======== zpl_media_client_foreach _pmedia_buffer_handler");
                client[i]._pmedia_buffer_handler(media_channel,  buffer_data, client[i].pVoidUser);
			}
		}
	}
    else
        zpl_media_debugmsg_debug("========zpl_media_client_foreach NULL");
	return OK;
}


#undef ZPL_MEDIA_CLIENT_INDEX_SET
#undef ZPL_MEDIA_CLIENT_INDEX_GET

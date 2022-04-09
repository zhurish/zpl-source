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
            fprintf(stdout, " =============================================zpl_media_client_add %p %p \r\n", cb_handler, pUser);
            fflush(stdout);
            return ZPL_MEDIA_CLIENT_INDEX_SET(i);
		}
	}
    return ERROR;
}

int zpl_media_client_del(zpl_media_client_t *client, zpl_int32 index)
{
    zpl_uint32 i = ZPL_MEDIA_CLIENT_INDEX_GET(index);
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

int zpl_media_client_foreach(zpl_media_buffer_t *queue, const zpl_media_buffer_data_t *buffer_data)
{
	zpl_uint32 i = 0;
	zpl_media_client_t *client = NULL;
	if(queue->media_channel != NULL)
	{
		client = ((zpl_media_channel_t*)queue->media_channel)->media_client;
	}
	if(client)
	{
		for(i = 0; i < ZPL_MEDIA_CLIENT_MAX; i++)
		{
			if(client[i].enable == zpl_true && client[i]._pmedia_buffer_handler != NULL)
			{
                client[i]._pmedia_buffer_handler(queue->media_channel,  buffer_data, client[i].pVoidUser);
			}
		}
	}
	return OK;
}


#undef ZPL_MEDIA_CLIENT_INDEX_SET
#undef ZPL_MEDIA_CLIENT_INDEX_GET

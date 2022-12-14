/*
 * zpl_media_client.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_CLIENT_H__
#define __ZPL_MEDIA_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>

#include "zpl_media_buffer.h"
#include "zpl_media_channel.h"





extern zpl_bool zpl_media_client_lookup(zpl_media_client_t *client, zpl_media_buffer_handler cb_handler, void *pUser);
extern int zpl_media_client_add(zpl_media_client_t *client, zpl_media_buffer_handler cb_handler, void *pUser);
extern int zpl_media_client_del(zpl_media_client_t *client, zpl_int32 index);
extern int zpl_media_client_foreach(zpl_skbuffer_t *buffer_data, void *p);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_CLIENT_H__ */

/*
 * zpl_media_proxy.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_PROXY_H__
#define __ZPL_VIDEO_PROXY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media_internal.h"

typedef struct
{
    NODE        node;
    zpl_socket_t     sock;
    uint16_t    port;
    char        *address;
    void        *t_read;
}proxy_client_t;

typedef struct
{
    uint16_t    port;
    zpl_socket_t         sock;

    os_mutex_t	*mutex;
    LIST        list;			//add queue
    void        *t_master;
    void        *t_read;
    int         initalition;
}proxy_server_t;


int zpl_media_proxy_init(void);
int zpl_media_proxy_exit(void);


int zpl_media_proxy_buffer_data_distribute(zpl_media_channel_t *mediachn, 
         const zpl_media_buffer_data_t *bufdata,  void *pVoidUser);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_PROXY_H__ */

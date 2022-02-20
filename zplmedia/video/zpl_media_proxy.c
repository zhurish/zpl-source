/*
 * zpl_media_proxy.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "vty_include.h"

#include "zpl_media.h"
#include "zpl_media_internal.h"


static proxy_server_t proxy_server = 
{
    .initalition = 0,
};


static int zpl_proxy_server_accept(struct eloop *t);
static int zpl_proxy_server_read(struct eloop *t);


int zpl_media_proxy_buffer_data_distribute(zpl_media_channel_t *mediachn, 
        const zpl_media_buffer_data_t *bufdata,  void *pVoidUser)
{
	NODE node;
    proxy_client_t *client = NULL;
    zpl_media_buffer_hdr_t  hdr;
    if(mediachn == NULL)
        return 0;

    if(proxy_server.initalition == 0 || proxy_server.mutex == NULL)
        return OK;
    if (proxy_server.mutex)
		os_mutex_lock(proxy_server.mutex, OS_WAIT_FOREVER);
    if(lstCount(&proxy_server.list))
    {
        for (client = (proxy_client_t *)lstFirst(&proxy_server.list);
            client != NULL; client = (proxy_client_t *)lstNext(&node))
        {
            node = client->node;
            if (client && !ipstack_invalid(client->sock))
            {
                hdr.ID = bufdata->ID;                 //ID 通道号
                hdr.buffer_type = bufdata->buffer_type;        //音频视频
                hdr.buffer_codec = bufdata->buffer_codec;       //编码类型
                hdr.buffer_key = bufdata->buffer_key;         //帧类型
                hdr.buffer_rev = bufdata->buffer_rev;         //
                hdr.buffer_flags = bufdata->buffer_flags;        //ZPL_BUFFER_DATA_E
                hdr.buffer_timetick = bufdata->buffer_timetick;    //时间戳毫秒
                //hdr.buffer_seq = bufdata->ID;         //序列号底层序列号
                hdr.buffer_len = bufdata->buffer_len;         //帧长度

                //bufdata->buffer_data;       //buffer

                ipstack_writemsg(client->sock, &hdr, sizeof(hdr), bufdata->buffer_data, bufdata->buffer_len);
                //write(client->sock, &hdr, sizeof(hdr));
                //write(client->sock, bufdata->buffer_data, bufdata->buffer_len);
            }
        }
    }
    if (proxy_server.mutex)
		os_mutex_unlock(proxy_server.mutex);  

    return OK;
}



static int proxy_server_client_destroy(proxy_client_t * client)
{
    if(client)
    {

        ipstack_close(client->sock);

        client->port = 0;
        if(client->address)
        {
            free(client->address);
            client->address = NULL;
        }
        free(client);
    }
    return OK;
}

static int proxy_server_init(void *master, int port)
{
    if(proxy_server.initalition == 0)
    {
        memset(&proxy_server, 0, sizeof(proxy_server));

        proxy_server.port = port;
        proxy_server.mutex = os_mutex_init();
		lstInitFree(&proxy_server.list, proxy_server_client_destroy);
        proxy_server.sock = ipstack_sock_create(IPCOM_STACK, zpl_true);

        if(ipstack_sock_bind(proxy_server.sock, NULL, proxy_server.port) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_exit(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            ipstack_close(proxy_server.sock);
            return ERROR;
        }
        if(ipstack_sock_listen(proxy_server.sock, 5) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_exit(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            ipstack_close(proxy_server.sock);
            return ERROR;
        }
        ipstack_set_nonblocking(proxy_server.sock);
        if(!ipstack_invalid(proxy_server.sock))
        {
            proxy_server.t_master = master;
            if(proxy_server.t_master)
                proxy_server.t_read  = eloop_add_read(master, zpl_proxy_server_accept, &proxy_server, proxy_server.sock);
            proxy_server.initalition = 1;
            return OK;
        }
    }
    return ERROR;
}

static int proxy_server_destroy()
{
    if(proxy_server.initalition == 0)
        return OK;
    if(proxy_server.t_read)
    {
        eloop_cancel(proxy_server.t_read);
        proxy_server.t_read = NULL;
    }
	lstFree(&proxy_server.list);
    if (proxy_server.mutex)
    {
        if (os_mutex_exit(proxy_server.mutex) == OK)
            proxy_server.mutex = NULL;
    }
    ipstack_close(proxy_server.sock);
    proxy_server.initalition = 0;
    return OK;
}



static int zpl_proxy_server_accept(struct eloop *t)
{
    zpl_socket_t sock;
	proxy_server_t *proxy = THREAD_ARG(t);
    if(proxy && !ipstack_invalid(proxy->sock))
    {
        struct ipstack_sockaddr_in clientaddr;
		if (proxy->mutex)
			os_mutex_lock(proxy->mutex, OS_WAIT_FOREVER);
        proxy->t_read = NULL;
        sock = ipstack_sock_accept (proxy->sock, &clientaddr);
        if(!ipstack_invalid(sock))
        {
            proxy_client_t *client = malloc(sizeof(proxy_client_t));
            if(client)
            {
                client->port = ntohs(clientaddr.sin_port);
                client->address = strdup(inet_ntoa(clientaddr.sin_addr));
                client->sock = sock;
                lstAdd(&proxy->list, (NODE *)client);
                ipstack_set_nonblocking(sock);
                if(proxy->t_master)
                    client->t_read  = eloop_add_read(proxy->t_master, zpl_proxy_server_read, client, client->sock);
            }
        }
        if(proxy->t_master)
            proxy->t_read  = eloop_add_read(proxy->t_master, zpl_proxy_server_accept, proxy, proxy->sock);
		if (proxy->mutex)
			os_mutex_unlock(proxy->mutex);   
    }
	return OK;
}

static int zpl_proxy_server_read(struct eloop *t)
{
    proxy_client_t *client = THREAD_ARG(t);
    if(client)
    {
        char buf[64];
        client->t_read = NULL;
        if(ipstack_read(client->sock, buf, sizeof(buf)) == 0)
        {
            if (proxy_server.mutex)
			    os_mutex_lock(proxy_server.mutex, OS_WAIT_FOREVER);
            lstDelete(&proxy_server.list, (NODE *)client);
            proxy_server_client_destroy(client);
            if (proxy_server.mutex)
			    os_mutex_unlock(proxy_server.mutex);  
        }
        if(proxy_server.t_master)
            client->t_read  = eloop_add_read(proxy_server.t_master, zpl_proxy_server_read, client, client->sock);
    }
    return OK;
}

int zpl_media_proxy_init()
{
    proxy_server_init(NULL, 0);
    return OK;
}

int zpl_media_proxy_exit()
{
    proxy_server_destroy();
    return OK;
}


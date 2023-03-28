/*
 * zpl_media_proxy.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"

#include "zpl_media.h"
#include "zpl_media_internal.h"
#ifdef ZPL_LIBORTP_MODULE
#include "ortp/ortp.h"
#endif

struct module_list module_list_medie_proxy = {
		.module = MODULE_MEDIA_PROXY,
		.name = "ZPLRTSP\0",
		.module_init = zpl_media_proxy_init,
		.module_exit = zpl_media_proxy_exit,
		.module_task_init = zpl_media_proxy_task_init,
		.module_task_exit = zpl_media_proxy_task_exit,
		.module_cmd_init = NULL,
		.taskid = 0,
};

static zpl_media_proxy_server_t proxy_server = 
{
    .initalition = 0,
};


static int zpl_media_proxy_server_accept(struct eloop *t);
static int zpl_media_proxy_server_read(struct eloop *t);
static int zpl_media_proxy_server_client_destroy(zpl_media_proxy_client_t * client);
static int zpl_media_proxy_buffer_data_distribute(zpl_media_channel_t *mediachn, 
        const zpl_skbuffer_t *bufdata,  void *pVoidUser);


int zpl_media_proxy_msg_create_header(struct zpl_media_msg *ipcmsg, zpl_uint32 command, int channel, int level)
{
    zpl_media_proxy_msg_header_t *hdr = (zpl_media_proxy_msg_header_t *)ipcmsg->buf;
    hdr->length = htons(ZPL_MEDIA_MSG_HEADER_SIZE);
    hdr->marker = ZPL_MEDIA_MSG_HEADER_MARKER;
    hdr->command = htons(command);
    hdr->channel = channel;
    hdr->level = level;
    ipcmsg->setp = sizeof(zpl_media_proxy_msg_header_t);
    return ipcmsg->setp;
}

int zpl_media_proxy_msg_get_header(struct zpl_media_msg *ipcmsg, zpl_media_proxy_msg_header_t *header)
{
    zpl_media_proxy_msg_header_t *hdr = (zpl_media_proxy_msg_header_t *)ipcmsg->buf;
    header->length = ntohs(hdr->length);
    header->marker = hdr->marker ;
    header->command = ntohs(hdr->command);
    header->channel = hdr->channel ;
    header->level = hdr->level;    
    ipcmsg->getp = sizeof(zpl_media_proxy_msg_header_t);
    return ipcmsg->getp;
}


static int zpl_media_proxy_msg_data_destroy(struct zpl_media_msg * bufdata)
{
	if(bufdata && bufdata->buf)
	{
		free(bufdata->buf);
		bufdata->buf = NULL;
        bufdata->length_max = 0;
		return OK;
	}
	return ERROR;
}


static int zpl_media_proxy_msg_data_create(struct zpl_media_msg * bufdata, int maxlen)
{
	if(bufdata == NULL)
		return ERROR;
	memset(bufdata, 0, sizeof(zpl_media_bufcache_t));
	bufdata->length_max = ZPL_SKSIZE_ALIGN(maxlen);
	bufdata->buf = malloc(bufdata->length_max);
	if(bufdata->buf)
	{
		memset(bufdata->buf, 0, bufdata->length_max);
		return OK;
	}
	return ERROR;
}

static int zpl_media_proxy_msg_data_add(struct zpl_media_msg * bufdata, char *data, int datalen)
{
	if (datalen <= 0)
		return OK;
	if (bufdata->buf == NULL)
	{
        zpl_media_proxy_msg_data_create(bufdata, datalen);
	}
	else
	{
		if (bufdata->length_max < (bufdata->setp + ZPL_SKBUF_ALIGN(datalen)))
		{
			zpl_uint32 length_max = bufdata->length_max + ZPL_SKBUF_ALIGN(datalen);
			bufdata->buf = realloc(bufdata->buf, length_max);
			if (bufdata->buf)
				bufdata->length_max = length_max;
		}
	}

	if (bufdata->buf != NULL && (bufdata->setp + datalen) < bufdata->length_max)
	{
		memcpy(bufdata->buf + bufdata->setp, data, datalen);
		bufdata->setp += datalen;
		return bufdata->setp;
	}
	return OK;
}

static int zpl_media_proxy_send_result_msg(zpl_media_proxy_client_t *client, zpl_uint16 command, int ret, char *data, int len)
{
    struct zpl_media_proxy_msg_result *msg;
    zpl_media_proxy_msg_create_header(&client->obuf, command, client->channel, client->level);
    msg = (struct zpl_media_proxy_msg_result *)(client->obuf.buf + client->obuf.setp);
    msg->result = htonl(ret);
    client->obuf.setp += 4;
    if(data)
        zpl_media_proxy_msg_data_add(&client->obuf, data, strlen(data));
    if(ipstack_write(client->sock, client->obuf.buf, client->obuf.setp) < 0)
    {
        zpl_media_proxy_server_client_destroy(client);
    }
    return 0;
}

static int zpl_media_proxy_read_hander(zpl_media_proxy_client_t *client, zpl_uint16 command, char *data, int datalen)
{
    int ret = -1;
    char *result = NULL;
    struct zpl_media_proxy_msg_cmd *msgcmd = (struct zpl_media_proxy_msg_cmd *)data;
    if (ZPL_MEDIA_CMD_GET(command) == ZPL_MEDIA_MSG_REGISTER)
    {
        struct zpl_media_proxy_msg_register *reg = (struct zpl_media_proxy_msg_register *)data;
        client->type = reg->type;
        ret = zpl_media_channel_client_add(client->channel, client->level, zpl_media_proxy_buffer_data_distribute, NULL);
        if(ret > 0)
        {
            client->call_index = ret; 
            ret = OK;
        }  
        else
            ret = -1; 
        //zpl_media_proxy_send_result_msg(client, ZPL_MEDIA_MSG_ACK, ret, NULL, 0);
    }
    else if (ZPL_MEDIA_CMD_GET(command) == ZPL_MEDIA_MSG_SET_CMD)
    {
        switch (ZPL_MEDIA_SUBCMD_GET(command))
        {
        case ZPL_MEDIA_MSG_SUB_START: // 开启
            if(client->call_index)
                ret = zpl_media_channel_client_start(client->channel, client->level, client->call_index, zpl_true);
            break;
        case ZPL_MEDIA_MSG_SUB_STOP: // 停止
            if(client->call_index)
                ret = zpl_media_channel_client_start(client->channel, client->level, client->call_index, zpl_false);
            break;
        case ZPL_MEDIA_MSG_SUB_RECORD: // 录像
            ret = zpl_media_channel_record_enable(client->channel, client->level, ntohl(msgcmd->record));
            break;
        case ZPL_MEDIA_MSG_SUB_CAPTURE: // 抓拍
             ret = zpl_media_channel_capture_enable(client->channel, client->level, ntohl(msgcmd->capture));
            break;
        case ZPL_MEDIA_MSG_SUB_CODEC: // 设置通道编解码
            break;
        case ZPL_MEDIA_MSG_SUB_RESOLVING: // 设置通道分辨率
            break;
        case ZPL_MEDIA_MSG_SUB_FRAME_RATE: // 设置通道帧率
            break;
        case ZPL_MEDIA_MSG_SUB_BITRATE: // 设置通道码率
            break;
        case ZPL_MEDIA_MSG_SUB_IKEYRATE: // I帧间隔
            break;
        case ZPL_MEDIA_MSG_SUB_PROFILE: // 编码等级
            break;
        case ZPL_MEDIA_MSG_SUB_RCMODE: // ZPL_VENC_RC_E
            break;
        case ZPL_MEDIA_MSG_SUB_GOPMODE: /* the gop mode */
            break;
        } 
        zpl_media_proxy_send_result_msg(client, ZPL_MEDIA_MSG_ACK, ret, result, strlen(result));
        return ret;
    }
    else if (ZPL_MEDIA_CMD_GET(command) == ZPL_MEDIA_MSG_GET_CMD)
    {
        switch (ZPL_MEDIA_SUBCMD_GET(command))
        {
        case ZPL_MEDIA_MSG_SUB_START: // 开启
            break;
        case ZPL_MEDIA_MSG_SUB_STOP: // 停止
            break;
        case ZPL_MEDIA_MSG_SUB_RECORD: // 录像
            break;
        case ZPL_MEDIA_MSG_SUB_CAPTURE: // 抓拍
            break;
        case ZPL_MEDIA_MSG_SUB_CODEC: // 设置通道编解码
            break;
        case ZPL_MEDIA_MSG_SUB_RESOLVING: // 设置通道分辨率
            break;
        case ZPL_MEDIA_MSG_SUB_FRAME_RATE: // 设置通道帧率
            break;
        case ZPL_MEDIA_MSG_SUB_BITRATE: // 设置通道码率
            break;
        case ZPL_MEDIA_MSG_SUB_IKEYRATE: // I帧间隔
            break;
        case ZPL_MEDIA_MSG_SUB_PROFILE: // 编码等级
            break;
        case ZPL_MEDIA_MSG_SUB_RCMODE: // ZPL_VENC_RC_E
            break;
        case ZPL_MEDIA_MSG_SUB_GOPMODE: /* the gop mode */
            break;
        }
        zpl_media_proxy_send_result_msg(client, ZPL_MEDIA_MSG_GET_ACK, ret, result, strlen(result));
    }
    return 0;
}

static int zpl_media_proxy_buffer_data_distribute(zpl_media_channel_t *mediachn, 
        const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
	NODE node;
    zpl_media_proxy_client_t *client = NULL;
    struct zpl_media_proxy_msg_data msgdata;
    if(mediachn == NULL)
        return 0;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    if(proxy_server.initalition == 0 || proxy_server.mutex == NULL)
        return OK;
    if (proxy_server.mutex)
		os_mutex_lock(proxy_server.mutex, OS_WAIT_FOREVER);

    if(lstCount(&proxy_server.list))
    {
        for (client = (zpl_media_proxy_client_t *)lstFirst(&proxy_server.list);
            client != NULL; client = (zpl_media_proxy_client_t *)lstNext(&node))
        {
            node = client->node;
            if (client && client->type == 1 && !ipstack_invalid(client->sock) && client->drop == 0)
            {
                if(mediachn->media_type == ZPL_MEDIA_VIDEO)
                {
                    if(media_header->buffertype == ZPL_MEDIA_FRAME_DATA_ENCODE)
                        zpl_media_proxy_msg_create_header(&client->obuf, ZPL_MEDIA_MSG_DATA, mediachn->channel, mediachn->channel_index);
                    else if(media_header->buffertype == ZPL_MEDIA_FRAME_DATA_CAPTURE)
                        zpl_media_proxy_msg_create_header(&client->obuf, ZPL_MEDIA_MSG_CAPTURE, mediachn->channel, mediachn->channel_index);
                }
                msgdata.type = media_header->type;        //音频/视频 ZPL_MEDIA_E
                msgdata.codectype = media_header->codectype;   //编码类型 ZPL_VIDEO_CODEC_E
                msgdata.frame_type = media_header->frame_type;  //帧类型  ZPL_VIDEO_FRAME_TYPE_E
                msgdata.buffertype = media_header->buffertype;    //ZPL_MEDIA_FRAME_DATA_E
                msgdata.length = htonl(media_header->length);      //帧长度  
                zpl_media_proxy_msg_data_add(&client->obuf, &msgdata, sizeof(msgdata));

                zpl_media_proxy_msg_data_add(&client->obuf, ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));

                if(ipstack_write(client->sock, client->obuf.buf, client->obuf.setp) < 0)
                {
                    zpl_media_proxy_server_client_destroy(client);
                }
                client->obuf.setp = client->obuf.getp = 0;
            }
        }
    }
    if (proxy_server.mutex)
		os_mutex_unlock(proxy_server.mutex);  

    return OK;
}



static int zpl_media_proxy_server_client_destroy(zpl_media_proxy_client_t * client)
{
    if(client)
    {
        lstDelete(&proxy_server.list, (NODE *)client);
        ipstack_close(client->sock);
        zpl_media_proxy_msg_data_destroy(&client->ibuf);
        zpl_media_proxy_msg_data_destroy(&client->obuf);
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

static int zpl_media_proxy_server_init(void *master, int port)
{
    if(proxy_server.initalition == 0)
    {
        memset(&proxy_server, 0, sizeof(proxy_server));
        proxy_server.t_master = master;
        proxy_server.port = port;
        proxy_server.cmd_port = port+1;
        proxy_server.mutex = os_mutex_name_create("proxy_server.mutex");
		lstInitFree(&proxy_server.list, zpl_media_proxy_server_client_destroy);

        proxy_server.sock = ipstack_sock_create(IPSTACK_IPCOM, zpl_true);
        proxy_server.cmd_sock = ipstack_sock_create(IPSTACK_IPCOM, zpl_true);
        if(ipstack_sock_bind(proxy_server.sock, NULL, proxy_server.port) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_destroy(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            if(!ipstack_invalid(proxy_server.cmd_sock))
            {
                ipstack_close(proxy_server.cmd_sock);
            }
            ipstack_close(proxy_server.sock);
            return ERROR;
        }
        if(ipstack_sock_bind(proxy_server.cmd_sock, NULL, proxy_server.cmd_port) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_destroy(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            if(!ipstack_invalid(proxy_server.sock))
            {
                ipstack_close(proxy_server.sock);
            }
            ipstack_close(proxy_server.cmd_sock);
            return ERROR;
        }
        if(ipstack_sock_listen(proxy_server.sock, 5) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_destroy(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            if(!ipstack_invalid(proxy_server.cmd_sock))
            {
                ipstack_close(proxy_server.cmd_sock);
            }
            ipstack_close(proxy_server.sock);
            return ERROR;
        }
        if(ipstack_sock_listen(proxy_server.cmd_sock, 5) != OK)
        {
            if (proxy_server.mutex)
            {
                if (os_mutex_destroy(proxy_server.mutex) == OK)
                    proxy_server.mutex = NULL;
            }
            if(!ipstack_invalid(proxy_server.sock))
            {
                ipstack_close(proxy_server.sock);
            }
            ipstack_close(proxy_server.cmd_sock);
            return ERROR;
        }
        ipstack_set_nonblocking(proxy_server.sock);
        ipstack_set_nonblocking(proxy_server.cmd_sock);
        if(!ipstack_invalid(proxy_server.sock))
        {
            if(proxy_server.t_master)
                proxy_server.t_read  = eloop_add_read(master, zpl_media_proxy_server_accept, &proxy_server, proxy_server.sock);
        }
        if(!ipstack_invalid(proxy_server.cmd_sock))
        {
            if(proxy_server.t_master)
                proxy_server.t_cmd_read  = eloop_add_read(master, zpl_media_proxy_server_accept, &proxy_server, proxy_server.cmd_sock);
        }
        proxy_server.initalition = 1;
        return OK;
    }
    return ERROR;
}

static int zpl_media_proxy_server_destroy(void)
{
    if(proxy_server.initalition == 0)
        return OK;
    if(proxy_server.t_read)
    {
        eloop_cancel(proxy_server.t_read);
        proxy_server.t_read = NULL;
    }
    if(proxy_server.t_cmd_read)
    {
        eloop_cancel(proxy_server.t_cmd_read);
        proxy_server.t_cmd_read = NULL;
    }
	lstFree(&proxy_server.list);
    if (proxy_server.mutex)
    {
        if (os_mutex_destroy(proxy_server.mutex) == OK)
            proxy_server.mutex = NULL;
    }
    if(!ipstack_invalid(proxy_server.sock))
    {
        ipstack_close(proxy_server.sock);
    }
    if(!ipstack_invalid(proxy_server.cmd_sock))
    {
        ipstack_close(proxy_server.cmd_sock);
    }
    proxy_server.initalition = 0;
    return OK;
}



static int zpl_media_proxy_server_accept(struct eloop *t)
{
    zpl_socket_t sock = THREAD_FD(t);
    zpl_socket_t client_sock = NULL;
	zpl_media_proxy_server_t *proxy = THREAD_ARG(t);
    if(proxy && !ipstack_invalid(sock))
    {
        struct ipstack_sockaddr_in clientaddr;
		if (proxy->mutex)
			os_mutex_lock(proxy->mutex, OS_WAIT_FOREVER);
        if(proxy->sock == sock)    
            proxy->t_read = NULL;
        else 
            proxy->t_cmd_read = NULL;   
        client_sock = ipstack_sock_accept (sock, &clientaddr);
        if(!ipstack_invalid(client_sock))
        {
            zpl_media_proxy_client_t *client = malloc(sizeof(zpl_media_proxy_client_t));
            if(client)
            {
                client->port = ntohs(clientaddr.sin_port);
                client->address = strdup(inet_ntoa(clientaddr.sin_addr));
                client->sock = client_sock;

                lstAdd(&proxy->list, (NODE *)client);
                ipstack_set_nonblocking(client_sock);

                zpl_media_proxy_msg_data_create(&client->ibuf, ZPL_MEDIA_MSG_MAX_PACKET_SIZ);
                zpl_media_proxy_msg_data_create(&client->obuf, ZPL_MEDIA_MSG_MAX_PACKET_SIZ);
                if(proxy->t_master)
                    client->t_read  = eloop_add_read(proxy->t_master, zpl_media_proxy_server_read, client, client->sock);
            }
        }
        if(proxy->t_master)
        {
            if(proxy->sock == sock) 
                proxy->t_read  = eloop_add_read(proxy->t_master, zpl_media_proxy_server_accept, proxy, sock);
            if(proxy->cmd_sock == sock) 
                proxy->t_cmd_read  = eloop_add_read(proxy->t_master, zpl_media_proxy_server_accept, proxy, sock);
        }
		if (proxy->mutex)
			os_mutex_unlock(proxy->mutex);   
    }
	return OK;
}

static int zpl_media_proxy_server_read(struct eloop *t)
{
    zpl_socket_t sock;
    zpl_media_proxy_client_t *client = NULL;
    zpl_media_proxy_msg_header_t *hdr = NULL;
    struct zpl_media_msg *input_ipcmsg = NULL;
    /* Get thread data.  Reset reading thread because I'm running. */
    sock = ELOOP_FD(t);
    client = ELOOP_ARG(t);
    zpl_assert(client);
    client->t_read = NULL;
    input_ipcmsg = &client->ibuf;
    hdr = (zpl_media_proxy_msg_header_t *)input_ipcmsg->buf;
    if (proxy_server.mutex)
		os_mutex_lock(proxy_server.mutex, OS_WAIT_FOREVER);
    /* Read length and command (if we don't have it already). */
    if (input_ipcmsg->setp < ZPL_MEDIA_MSG_HEADER_SIZE)
    {
        ssize_t nbyte = 0;
        nbyte = ipstack_read(sock, input_ipcmsg->buf + input_ipcmsg->setp, ZPL_MEDIA_MSG_HEADER_SIZE - input_ipcmsg->setp);
        if ((nbyte == 0) || (nbyte == -1))
        {
            zlog_err(MODULE_HAL, "connection closed ipstack_socket [%d]", ipstack_fd(sock));
            zpl_media_proxy_server_client_destroy(client);
            if (proxy_server.mutex)
		        os_mutex_unlock(proxy_server.mutex);  
            return -1;
        }
        if (nbyte != (ssize_t)(ZPL_MEDIA_MSG_HEADER_SIZE - input_ipcmsg->setp))
        {
            /* Try again later. */
            if (proxy_server.t_master)
                client->t_read = eloop_add_read(proxy_server.t_master, zpl_media_proxy_server_read, client, client->sock);
            if (proxy_server.mutex)
		        os_mutex_unlock(proxy_server.mutex);  
            return 0;
        }
        input_ipcmsg->setp = ZPL_MEDIA_MSG_HEADER_SIZE;
    }

    /* Fetch header values */
    hdr->length = ntohs(hdr->length);
    hdr->command = ntohs(hdr->command);

    if (hdr->marker != ZPL_MEDIA_MSG_HEADER_MARKER)
    {
        zlog_err(MODULE_HAL, "%s: ipstack_socket %d mismatch, marker %d",
                 __func__, ipstack_fd(sock), hdr->marker);
        zpl_media_proxy_server_client_destroy(client);
        if (proxy_server.mutex)
		    os_mutex_unlock(proxy_server.mutex);  
        return -1;
    }
    if (hdr->length < ZPL_MEDIA_MSG_HEADER_SIZE)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u is less than header size %u",
                  __func__, ipstack_fd(sock), hdr->length, (zpl_uint32)ZPL_MEDIA_MSG_HEADER_SIZE);
        zpl_media_proxy_server_client_destroy(client);
        if (proxy_server.mutex)
		    os_mutex_unlock(proxy_server.mutex);  
        return -1;
    }
    if (hdr->length > input_ipcmsg->length_max)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
                  __func__, ipstack_fd(sock), hdr->length, (u_long)(input_ipcmsg->length_max));
        zpl_media_proxy_server_client_destroy(client);
        if (proxy_server.mutex)
		    os_mutex_unlock(proxy_server.mutex);  
        return -1;
    }

    /* Read rest of data. */
    if (input_ipcmsg->setp < hdr->length)
    {
        ssize_t nbyte;
        if (((nbyte = ipstack_read(sock, input_ipcmsg->buf + input_ipcmsg->setp,
                                   hdr->length - input_ipcmsg->setp)) == 0) ||
            (nbyte == -1))
        {
            zlog_err(MODULE_HAL, "connection closed [%d] when reading zebra data", ipstack_fd(sock));
            zpl_media_proxy_server_client_destroy(client);
            if (proxy_server.mutex)
		        os_mutex_unlock(proxy_server.mutex);  
            return -1;
        }
        if (nbyte != (ssize_t)(hdr->length - input_ipcmsg->setp))
        {
            /* Try again later. */
            if (proxy_server.t_master)
                client->t_read = eloop_add_read(proxy_server.t_master, zpl_media_proxy_server_read, client, client->sock);
             if (proxy_server.mutex)
		        os_mutex_unlock(proxy_server.mutex);  
            return 0;
        }
        input_ipcmsg->setp += hdr->length;
    }

    hdr->length -= ZPL_MEDIA_MSG_HEADER_SIZE;

    input_ipcmsg->getp = ZPL_MEDIA_MSG_HEADER_SIZE;
    client->channel = hdr->channel; 
    client->level = hdr->level;
    switch (ZPL_MEDIA_CMD_GET(hdr->command))
    {
    case ZPL_MEDIA_MSG_REGISTER:
        client->drop = 1;
        zpl_media_proxy_read_hander(client, hdr->command, input_ipcmsg->buf + input_ipcmsg->getp, hdr->length);
        client->drop = 0;
        break;
    case ZPL_MEDIA_MSG_DATA:
        break;
    case ZPL_MEDIA_MSG_CAPTURE:
        break;
    case ZPL_MEDIA_MSG_SET_CMD:
        client->drop = 1;
        zpl_media_proxy_read_hander(client, hdr->command, input_ipcmsg->buf + input_ipcmsg->getp, hdr->length);
        client->drop = 0;
        break;
    case ZPL_MEDIA_MSG_GET_CMD:
        client->drop = 1;
        zpl_media_proxy_read_hander(client, hdr->command, input_ipcmsg->buf + input_ipcmsg->getp, hdr->length);
        client->drop = 0;
        break;
    case ZPL_MEDIA_MSG_ACK:
        break;
    case ZPL_MEDIA_MSG_GET_ACK:
        break;
    default:
        zlog_warn(MODULE_HAL, "Server Recv unknown command %d", ZPL_MEDIA_CMD_GET(hdr->command));
        break;
    }

    if (proxy_server.t_master)
        client->t_read = eloop_add_read(proxy_server.t_master, zpl_media_proxy_server_read, client, client->sock);
    input_ipcmsg->setp = input_ipcmsg->getp = 0;    
    if (proxy_server.mutex)
		os_mutex_unlock(proxy_server.mutex);  
    return 0;
}



static int mediaProxyTask(void* argv)
{   
    host_waitting_loadconfig();
	proxy_server.t_master = eloop_master_module_create(MODULE_MEDIA_PROXY);
    if(proxy_server.initalition && proxy_server.t_master)
    {
        eloop_mainloop(proxy_server.t_master);
    }
    return OK;
}


int zpl_media_proxy_task_init(void)
{
    if(proxy_server.initalition && proxy_server.t_master)
    {
		proxy_server.t_taskid = os_task_create("mediaProxyTask", OS_TASK_DEFAULT_PRIORITY,
								 0, mediaProxyTask, NULL, OS_TASK_DEFAULT_STACK*8);
        return OK;
    }
    return OK;
}

int zpl_media_proxy_task_exit(void)
{
    if(proxy_server.initalition && proxy_server.t_master)
    {
        if(proxy_server.t_taskid)
		    os_task_destroy(proxy_server.t_taskid);
	    proxy_server.t_taskid = 0;
    }
    return OK;
}

int zpl_media_proxy_init(void)
{
    struct thread_master *master = eloop_master_module_create(MODULE_MEDIA_PROXY);
    if(master)
        zpl_media_proxy_server_init(master, ZPL_MEDIA_PROXY_PORT);
    return OK;
}

int zpl_media_proxy_exit(void)
{
    zpl_media_proxy_server_destroy();
    return OK;
}


/*
 * pjsua_app_cb.c
 *
 *  Created on: Jun 22, 2019
 *      Author: zhurish
 */
#include "pjsua_app.h"
#include "pjsua_app_cfgapi.h"
#include "pjsua_app_cb.h"
#include "pjapp_app_util.h"

#include "auto_include.h"
#include <zplos_include.h>



int pjapp_sock_cfg_default(struct pjapp_sock_cfg *cfg)
{
	pj_bzero(cfg, sizeof(*cfg));
	return socketpair(AF_UNIX, SOCK_STREAM, 0, cfg->fd);
}


static int pjapp_sock_cfg_send_msg(int index, struct pjapp_sock_cfg *cfg)
{
	return send(cfg->fd[index], cfg->obuf, cfg->olen, 0);
}


static int pjapp_sock_cfg_recv_packet(int index, struct pjapp_sock_cfg *cfg)
{
	int len = 0, ret = 0;
	pj_uint8_t *data;
	struct pjapp_sock_msg_hdr *hdr = (struct pjapp_sock_msg_hdr *)cfg->ibuf;
	int already = cfg->ilen;
	data = cfg->ibuf + cfg->ilen;
	if(already < sizeof(struct pjapp_sock_msg_hdr))
	{
		len = sizeof(struct pjapp_sock_msg_hdr) - already;
		ret = recv(cfg->fd[index], data, len, 0);
		if(ret == len)
		{
			cfg->ilen += ret;
		}
		else if(ret <= 0)
			return -1;
		else if(ret > 0)	
			return 1;	
	}
	len = hdr->len - already;
	if(len)
	{
		ret = recv(cfg->fd[index], data, len, 0);
		if(ret == len)
		{
			cfg->ilen += ret;
			return 0;
		}
		if(ret <= 0)
			return -1;
		return 1;	
	}
	return -1;
}


static int pjapp_sock_cfg_read_msg(int index, struct pjapp_sock_cfg *cfg)
{
	int ret = 0;
	while(1)
	{
		ret = pjapp_sock_cfg_recv_packet(index, cfg);
		if(ret > 0)
			continue;
		else if(ret == 0)
		{
			return 0;
		}	
		else
			return -1;
	}
	return 0;
}

int pjapp_sock_write_result(struct pjapp_sock_cfg *cfg, int ret, char *result)
{
	char *msg ; 
	struct pjapp_sock_msg_hdr *hdr = (struct pjapp_sock_msg_hdr *)cfg->obuf;
	struct pjapp_msg_result *result_msg = (struct pjapp_msg_result*)(cfg->obuf + sizeof(struct pjapp_sock_msg_hdr));
	msg = (char *)(cfg->obuf + sizeof(struct pjapp_sock_msg_hdr) + sizeof(struct pjapp_msg_result));
	if(result)
		strcpy(msg, result);
	result_msg->result = ret;
	result_msg->len = result?strlen(result):0;	
	hdr->cmd = PJAPP_APP_ACK_CMD;
	hdr->len = sizeof(struct pjapp_sock_msg_hdr) + sizeof(struct pjapp_msg_result) + result_msg->len;
	hdr->mask = 0;
	hdr->version = 0;
	return pjapp_sock_cfg_send_msg(1, cfg);
}

int pjapp_sock_read_result(struct pjapp_sock_cfg *cfg, char *result)
{
	struct pjapp_msg_result *result_msg;
	if(pjapp_sock_cfg_read_msg(0, cfg) == 0)
	{
		char *msg ; 
		struct pjapp_sock_msg_hdr *hdr = (struct pjapp_sock_msg_hdr *)cfg->ibuf;
		result_msg = (struct pjapp_msg_result*)(cfg->ibuf + sizeof(struct pjapp_sock_msg_hdr));
		msg = (char *)(cfg->ibuf + sizeof(struct pjapp_sock_msg_hdr) + sizeof(struct pjapp_msg_result));
		if(result_msg->result == 0)
			return 0;
		if(result)
			strncpy(result, msg, result_msg->len);	
		return 	result_msg->result ;
	}
	return -1;
}


int pjapp_sock_write_cmd(struct pjapp_sock_cfg *cfg, int cmd, char *data, int len)
{
	char *msg ; 
	struct pjapp_sock_msg_hdr *hdr = (struct pjapp_sock_msg_hdr *)cfg->obuf;
	msg = (char *)(cfg->obuf + sizeof(struct pjapp_sock_msg_hdr));
	if(data)
		memcpy(msg, data, len);
	hdr->cmd = cmd;
	hdr->len = sizeof(struct pjapp_sock_msg_hdr) + len;
	hdr->mask = 0;
	hdr->version = 0;
	return pjapp_sock_cfg_send_msg(0, cfg);
}

int pjapp_sock_read_cmd(struct pjapp_sock_cfg *cfg)
{
	int ret = 0;
	if(pjapp_sock_cfg_read_msg(1, cfg) == 0)
	{
		char *msg ; 
		struct pjapp_sock_msg_hdr *hdr = (struct pjapp_sock_msg_hdr *)cfg->ibuf;
		msg = (char *)(cfg->ibuf + sizeof(struct pjapp_sock_msg_hdr));
		if(cfg->cmd_hander)
		{
			ret = (cfg->cmd_hander)(cfg, hdr->cmd, msg, hdr->len-sizeof(struct pjapp_sock_msg_hdr));
		}
		return 0 ;
	}
	return -1;
}
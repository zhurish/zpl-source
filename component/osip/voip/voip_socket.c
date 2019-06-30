/*
 * voip_socket.c
 *
 *  Created on: 2019年1月3日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"
#include "os_util.h"
#include "os_socket.h"
#include "os_uci.h"

#include "voip_def.h"
#include "voip_app.h"
#include "voip_socket.h"

static voip_socket_t *voip_socket = NULL;

static int _voip_socket_init(voip_socket_t *vsocket);
static int _voip_socket_exit(voip_socket_t *vsocket);


int voip_socket_module_init(void)
{
	voip_socket = &voip_app->voip_socket;
	zassert(voip_socket != NULL);
	memset(voip_socket, 0, sizeof(voip_socket_t));
	_voip_socket_init(voip_socket);
	return OK;
}

int voip_socket_module_exit(void)
{
	zassert(voip_socket != NULL);
	_voip_socket_exit(voip_socket);
	//voip_sip_ctl_module_exit();
	memset(voip_socket, 0, sizeof(voip_socket_t));
	return OK;
}


static int voip_socket_send_cmd(voip_socket_t *socket, char *data, int len)
{
	voip_socket_hdr hdr;
	zassert(socket != NULL);
	zassert(data != NULL);
	if(socket->sock)
	{
		memset(&hdr, 0, sizeof(hdr));

		hdr.type = 0;
		hdr.magic = 0;
		hdr.len = htons(MIN(len, 256));
		memcpy(hdr.data, data, MIN(len, 256));

		if(write(socket->sock, &hdr, sizeof(hdr)) == sizeof(hdr))
			return OK;

/*		if(write(socket->sock, data, len) == len)
			return OK;*/
	}
	zlog_err(ZLOG_VOIP, "mediastream's poll() returned %s",strerror(errno));
	return ERROR;
}

int voip_socket_quit()
{
	zassert(voip_socket != NULL);
	return voip_socket_send_cmd(voip_socket, "quit", 4);
}

int voip_socket_equalizer(BOOL enable, float frequency, float gain, float width)
{
	char cmd[512];
	zassert(voip_socket != NULL);
	memset(cmd, 0, sizeof(cmd));
	if(frequency == 0.0)
	{
		sprintf(cmd, "eq active %d", enable);
	}
	else
	{
		sprintf(cmd, "eq %f %f %f", frequency, gain, width);
	}
	return voip_socket_send_cmd(voip_socket, cmd, strlen(cmd));
}

int voip_socket_lossrate(int lossrate)
{
	char cmd[512];
	zassert(voip_socket != NULL);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "lossrate %i", lossrate);
	return voip_socket_send_cmd(voip_socket, cmd, strlen(cmd));
}

int voip_socket_bandwidth(int bandwidth)
{
	char cmd[512];
	zassert(voip_socket != NULL);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "bandwidth %i", bandwidth);
	return voip_socket_send_cmd(voip_socket, cmd, strlen(cmd));
}

int voip_socket_task_init()
{
	return OK;
}


int voip_socket_task_exit()
{
	return OK;
}




static int _voip_socket_init(voip_socket_t *vsocket)
{
	zassert(vsocket != NULL);
	zassert(vsocket != NULL);
	if(unix_sockpair_create(TRUE, &vsocket->mdctl, &vsocket->sock) == OK)
	{
		os_set_nonblocking(vsocket->sock);
		os_set_nonblocking(vsocket->mdctl);
		//zlog_debug(ZLOG_VOIP, "Create unix socket for stream controls(%d:%d)", vsocket->mdctl, vsocket->sock);
		return OK;
	}
	zlog_err(ZLOG_VOIP, "Can not create socket sockpair (%s)", strerror(errno));
	return ERROR;
}

static int _voip_socket_exit(voip_socket_t *vsocket)
{
	zassert(vsocket != NULL);
	zassert(vsocket != NULL);
	if(vsocket)
	{
		if(vsocket->mdctl)
		{
			close(vsocket->mdctl);
			vsocket->mdctl = 0;
		}
		if(vsocket->sock)
		{
			close(vsocket->sock);
			vsocket->sock = 0;
		}
		return OK;
	}
	return ERROR;
}

int voip_socket_sync_cmd()
{
	zassert(voip_socket != NULL);
	voip_socket_hdr hdr;
	if(voip_socket->mdctl > 0)
	{
		memset(&hdr, 0, sizeof(hdr));
		hdr.type = 0;
		hdr.magic = 0;
		hdr.len = 256;
		if(write(voip_socket->mdctl, &hdr, sizeof(hdr)) == sizeof(hdr))
			return OK;
	}
	//zlog_err(ZLOG_VOIP, "mediastream's poll() returned %s",strerror(errno));
	return ERROR;
}


int voip_socket_get_readfd()
{
	zassert(voip_socket != NULL);
	return voip_socket->sock;
}

int voip_socket_get_writefd()
{
	zassert(voip_socket != NULL);
	return voip_socket->mdctl;
}




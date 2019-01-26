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

#include "voip_def.h"
#include "voip_task.h"
#include "voip_event.h"
#include "voip_socket.h"
#include "voip_stream.h"
#include "voip_app.h"


voip_socket_t voip_socket;

static int _voip_socket_init(voip_socket_t *vsocket);
static int _voip_socket_exit(voip_socket_t *vsocket);
static int voip_socket_read_eloop(struct eloop *eloop);
static int voip_socket_timer_eloop(struct eloop *eloop);

int voip_socket_module_init(void)
{
	memset(&voip_socket, 0, sizeof(voip_socket));
	if(master_eloop[MODULE_VOIP] == NULL)
		master_eloop[MODULE_VOIP] = eloop_master_module_create(MODULE_VOIP);
	voip_socket.master = master_eloop[MODULE_VOIP];
	_voip_socket_init(&voip_socket);
	//voip_sip_ctl_module_init();
	return OK;
}

int voip_socket_module_exit(void)
{
	_voip_socket_exit(&voip_socket);
	//voip_sip_ctl_module_exit();
	memset(&voip_socket, 0, sizeof(voip_socket));
	return OK;
}


static int voip_socket_send_cmd(voip_socket_t *socket, char *data, int len)
{
	voip_socket_hdr hdr;
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

int voip_socket_quit(voip_socket_t *socket)
{
	return voip_socket_send_cmd(socket, "quit", 4);
}

int voip_socket_equalizer(voip_socket_t *socket, BOOL enable, float frequency, float gain, float width)
{
	char cmd[512];
	memset(cmd, 0, sizeof(cmd));
	if(frequency == 0.0)
	{
		sprintf(cmd, "eq active %d", enable);
	}
	else
	{
		sprintf(cmd, "eq %f %f %f", frequency, gain, width);
	}
	return voip_socket_send_cmd(socket, cmd, strlen(cmd));
}

int voip_socket_lossrate(voip_socket_t *socket, int lossrate)
{
	char cmd[512];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "lossrate %i", lossrate);
	return voip_socket_send_cmd(socket, cmd, strlen(cmd));
}

int voip_socket_bandwidth(voip_socket_t *socket, int bandwidth)
{
	char cmd[512];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "bandwidth %i", bandwidth);
	return voip_socket_send_cmd(socket, cmd, strlen(cmd));
}


/*
 * VOIP Event Task
 */
static int voip_socket_task(voip_socket_t *socket)
{
	//int n = 0;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(!socket->enable)
	{
		os_sleep(1);
/*		n++;
		if(n == 8)
			voip_test();*/
	}
	if(socket->master && !socket->t_time)
		socket->t_read = eloop_add_timer(socket->master, voip_socket_timer_eloop, socket, 10);
	if(socket->master && !socket->t_read)
		socket->t_read = eloop_add_read(socket->master, voip_socket_read_eloop, socket, socket->sock);
	while(socket->enable)
	{
		eloop_start_running(master_eloop[MODULE_VOIP], MODULE_VOIP);
	}
	return OK;
}



int voip_socket_task_init()
{
	if(voip_socket.taskid)
		return OK;
	if(master_eloop[MODULE_VOIP] == NULL)
		master_eloop[MODULE_VOIP] = eloop_master_module_create(MODULE_VOIP);
	voip_socket.master = master_eloop[MODULE_VOIP];
	voip_socket.taskid = os_task_create("voipSocket", OS_TASK_DEFAULT_PRIORITY,
	               0, voip_socket_task, &voip_socket, OS_TASK_DEFAULT_STACK);
	if(voip_socket.taskid)
	{
		voip_socket.enable = TRUE;
		return OK;
	}
	return ERROR;
}


int voip_socket_task_exit()
{
	if(voip_socket.taskid)
	{
		if(os_task_destroy(voip_socket.taskid)==OK)
			voip_socket.taskid = 0;
	}
	return OK;
}

static int voip_socket_timer_eloop(struct eloop *eloop)
{
	voip_socket_t *vsocket = ELOOP_ARG(eloop);
	//zlog_debug(ZLOG_VOIP, "voip_socket_timer_eloop");
	vsocket->t_time = eloop_add_timer(vsocket->master, voip_socket_timer_eloop, vsocket, 5);
	return OK;
}


static int voip_socket_read_eloop(struct eloop *eloop)
{
	voip_socket_t *vsocket = ELOOP_ARG(eloop);
	int sock = ELOOP_FD(eloop);
	//ELOOP_VAL(X)
	vsocket->t_read = NULL;
	memset(vsocket->buf, 0, sizeof(vsocket->buf));

	int len = read(sock, vsocket->buf, sizeof(vsocket->buf));
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				//mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
				return OK;
			}
			else
			{

			}
		}
	}
	else
	{

	}
	vsocket->t_read = eloop_add_read(vsocket->master, voip_socket_read_eloop, vsocket, sock);
	return OK;
}



static int _voip_socket_init(voip_socket_t *vsocket)
{
#if 1
	if(unix_sockpair_create(TRUE, &vsocket->mdctl, &vsocket->sock) == OK)
	{
		os_set_nonblocking(vsocket->sock);
		os_set_nonblocking(vsocket->mdctl);
		if(vsocket->master)
		{
			vsocket->t_time = eloop_add_timer(vsocket->master, voip_socket_timer_eloop, vsocket, 5);
			vsocket->t_read = eloop_add_read(vsocket->master, voip_socket_read_eloop, vsocket, vsocket->sock);
		}
		zlog_debug(ZLOG_VOIP, "Create unix socket for stream controls(%d:%d)", vsocket->mdctl, vsocket->sock);
		return OK;
	}
#else
	int fd = unix_sock_server_create(FALSE, "voipEvent");
	if(fd)
	{
		vsocket->sock = fd;
		os_set_nonblocking(fd);
		if(vsocket->master)
			vsocket->t_read = eloop_add_read(vsocket->master, voip_socket_read_eloop, vsocket, fd);
		//vsocket->t_thread = eloop_add_timer(vsocket->master, x5_b_a_timer_eloop, vsocket, vsocket->interval);
		return OK;
	}
#endif
	return ERROR;
}

static int _voip_socket_exit(voip_socket_t *vsocket)
{
	if(vsocket && vsocket->t_accept)
	{
		eloop_cancel(vsocket->t_accept);
		vsocket->t_accept = NULL;
	}
	if(vsocket && vsocket->t_read)
	{
		eloop_cancel(vsocket->t_read);
		vsocket->t_read = NULL;
	}
	if(vsocket && vsocket->t_write)
	{
		eloop_cancel(vsocket->t_write);
		vsocket->t_write = NULL;
	}
	if(vsocket && vsocket->t_event)
	{
		eloop_cancel(vsocket->t_event);
		vsocket->t_event = NULL;
	}
	if(vsocket && vsocket->t_time)
	{
		eloop_cancel(vsocket->t_time);
		vsocket->t_time = NULL;
	}
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
		memset(vsocket->buf, 0, sizeof(vsocket->buf));
		return OK;
	}
	return ERROR;
}

int voip_socket_sync_cmd()
{
	voip_socket_hdr hdr;
	if(voip_socket.mdctl)
	{
		memset(&hdr, 0, sizeof(hdr));
		hdr.type = 0;
		hdr.magic = 0;
		hdr.len = 256;
		if(write(voip_socket.mdctl, &hdr, sizeof(hdr)) == sizeof(hdr))
			return OK;
	}
	//zlog_err(ZLOG_VOIP, "mediastream's poll() returned %s",strerror(errno));
	return ERROR;
}

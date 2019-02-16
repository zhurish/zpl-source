/*
 * voip_estate_mgt.c
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"

#include "os_util.h"
#include "os_socket.h"

#include "os_tlv.h"

#include "x5_b_a.h"
#include "estate_mgt.h"
#ifdef PL_VOIP_MODULE
#include "voip_dbtest.h"
#endif
voip_estate_mgt_t gestate_mgt;



static int voip_estate_mgt_socket_read_eloop(struct eloop *eloop);



static int voip_estate_mgt_send_msg(voip_estate_mgt_t *estate_mgt)
{
	int len = 0;
/*	if(VOIP_EMGT_DEBUG(SEND))
	{
		zlog_debug(ZLOG_APP, "MSG to %s:%d %d byte (seqnum=%d)", mgt->remote_address,
				mgt->remote_port, mgt->slen, mgt->s_seqnum);
		if(VOIP_EMGT_DEBUG(HEX))
			x5_b_a_hex_debug(mgt, "SEND", 0);
	}*/
	if(estate_mgt->sock)
		len = sock_client_write(estate_mgt->sock, estate_mgt->remote_address,
				estate_mgt->remote_port, estate_mgt->sbuf, estate_mgt->slen);
		//len = write(mgt->w_fd, mgt->sbuf, mgt->slen);

	return len ? OK : ERROR;
}

static int voip_estate_mgt_wait_read_msg(voip_estate_mgt_t *estate_mgt, int timeoutms)
{
	int len = 0, maxfd = 0, num = 0;
	fd_set rfdset;
	FD_ZERO(&rfdset);
	FD_SET(estate_mgt->sock, &rfdset);
	maxfd = estate_mgt->sock;
	memset(estate_mgt->buf, 0, sizeof(estate_mgt->buf));
r_again:
	FD_SET(estate_mgt->sock, &rfdset);
	num = os_select_wait(maxfd + 1, &rfdset, NULL, timeoutms);
	if(num > 0)
	{
		//process_log_debug("start unix_sock_accept %s", progname);
		if(FD_ISSET(estate_mgt->sock, &rfdset))
		{
			FD_CLR(estate_mgt->sock, &rfdset);
			len = read(estate_mgt->sock, estate_mgt->buf, sizeof(estate_mgt->buf));
			if(len)
			{

			}
			else
			{
				if (ERRNO_IO_RETRY(errno))
				{
					//return 0;
					//mgt->t_reset = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
					goto r_again;
				}
				else
				{
					return ERROR;
				}
			}
		}
	}
	else if(num < 0)
	{
		//continue;
		return ERROR;
	}
	else
	{
		//timeout
		return -1;
	}
	return OK;
}




static int voip_estate_mgt_write_and_wait_respone(voip_estate_mgt_t *estate_mgt, int timeoutms)
{
	int rep = 0,ret = 0;
	if(estate_mgt->t_read)
	{
		eloop_cancel(estate_mgt->t_read);
		rep = 1;
	}
	ret = voip_estate_mgt_send_msg(estate_mgt);
	if(ret > 0)
		ret = voip_estate_mgt_wait_read_msg(estate_mgt, timeoutms);
	if(rep)
	{
		estate_mgt->t_read = eloop_add_read(estate_mgt->master,
				voip_estate_mgt_socket_read_eloop, estate_mgt, estate_mgt->sock);
	}
	//V_APP_DEBUG("-----------%s:", __func__);
	return ret;
}




static int voip_estate_mgt_socket_read_eloop(struct eloop *eloop)
{
	voip_estate_mgt_t *estate_mgt = ELOOP_ARG(eloop);
	int sock = ELOOP_FD(eloop);
	//ELOOP_VAL(X)
	estate_mgt->t_read = NULL;
	memset(estate_mgt->buf, 0, sizeof(estate_mgt->buf));

	int len = read(sock, estate_mgt->buf, sizeof(estate_mgt->buf));
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				//mgt->t_reset = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
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
	estate_mgt->t_read = eloop_add_read(estate_mgt->master, voip_estate_mgt_socket_read_eloop,
			estate_mgt, sock);
	return OK;
}




static int voip_estate_mgt_socket_init(voip_estate_mgt_t *estate_mgt)
{
	int fd = sock_create(FALSE);
	if(fd)
	{
		if(estate_mgt->local_port == 0)
			estate_mgt->local_port = X5_B_A_PORT_DEFAULT;
		if(estate_mgt->remote_port == 0)
			estate_mgt->remote_port = X5_B_A_PORT_DEFAULT;
		if(estate_mgt->remote_address == NULL)
		{
			close(fd);
			zlog_err(ZLOG_APP, " X5-B-A module remote address is not setting");
			return ERROR;
		}
		//zlog_debug(ZLOG_APP, "sock_bind %s:%d", mgt->local_address ? mgt->local_address:"any", mgt->local_port);
		if(sock_bind(fd, estate_mgt->local_address, estate_mgt->local_port) == OK)
		{
			//zlog_debug(ZLOG_APP, "sock_connect %s:%d", mgt->remote_address, mgt->remote_port);
/*			if(sock_connect(fd, mgt->remote_address, mgt->remote_port)!= OK)
			{
				close(fd);
				return ERROR;
			}*/
			estate_mgt->sock = fd;
			if(estate_mgt->master)
				estate_mgt->t_read = eloop_add_read(estate_mgt->master,
						voip_estate_mgt_socket_read_eloop, estate_mgt, fd);
			return OK;
		}
	}
	return ERROR;
}

static int voip_estate_mgt_socket_exit(voip_estate_mgt_t *estate_mgt)
{
	if(estate_mgt && estate_mgt->t_read)
	{
		eloop_cancel(estate_mgt->t_read);
		estate_mgt->t_read = NULL;
	}
	if(estate_mgt && estate_mgt->t_write)
	{
		eloop_cancel(estate_mgt->t_write);
		estate_mgt->t_write = NULL;
	}
	if(estate_mgt && estate_mgt->t_event)
	{
		eloop_cancel(estate_mgt->t_event);
		estate_mgt->t_event = NULL;
	}
	if(estate_mgt && estate_mgt->t_time)
	{
		eloop_cancel(estate_mgt->t_time);
		estate_mgt->t_time = NULL;
	}
	if(estate_mgt && estate_mgt->t_reset)
	{
		eloop_cancel(estate_mgt->t_reset);
		estate_mgt->t_reset = NULL;
	}
	if(estate_mgt)
	{
		if(estate_mgt->sock)
			close(estate_mgt->sock);
		memset(estate_mgt->buf, 0, sizeof(estate_mgt->buf));
		return OK;
	}
	return ERROR;
}


static int voip_estate_mgt_reset_eloop(struct eloop *eloop)
{
	voip_estate_mgt_t *estate_mgt = ELOOP_ARG(eloop);
	estate_mgt->t_reset = NULL;
/*	if(VOIP_EMGT_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RESET mgt socket OK");*/
	voip_estate_mgt_socket_exit(estate_mgt);
	voip_estate_mgt_socket_init(estate_mgt);
	return OK;
}


int voip_estate_mgt_init(void)
{
	memset(&gestate_mgt, 0, sizeof(gestate_mgt));
	if(master_eloop[MODULE_APP_START] == NULL)
		gestate_mgt.master = master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	voip_estate_mgt_socket_init(&gestate_mgt);
	return OK;
}

int voip_estate_mgt_exit(void)
{
	voip_estate_mgt_socket_exit(&gestate_mgt);
	memset(&gestate_mgt, 0, sizeof(gestate_mgt));
	return OK;
}

int voip_estate_mgt_start(void)
{
	//memset(&gestate_mgt, 0, sizeof(gestate_mgt));
	if(master_eloop[MODULE_APP_START] == NULL)
		gestate_mgt.master = master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	voip_estate_mgt_socket_init(&gestate_mgt);
	return OK;
}

int voip_estate_mgt_stop(void)
{
	voip_estate_mgt_socket_exit(&gestate_mgt);
	//memset(&gestate_mgt, 0, sizeof(gestate_mgt));
	return OK;
}


int voip_estate_mgt_address_set_api(char *remote)
{
	if(!x5_b_a_mgt)
		return ERROR;
	if(gestate_mgt.remote_address)
		free(gestate_mgt.remote_address);
	if(remote)
		gestate_mgt.remote_address = strdup(remote);
	if(gestate_mgt.t_reset)
	{
		eloop_cancel(gestate_mgt.t_reset);
		gestate_mgt.t_reset = NULL;
	}
	gestate_mgt.t_reset = eloop_add_timer_msec(gestate_mgt.master, voip_estate_mgt_reset_eloop, &gestate_mgt, 100);
	return OK;
}

int voip_estate_mgt_local_address_set_api(char *address)
{
	if(!x5_b_a_mgt)
		return ERROR;
	if(gestate_mgt.local_address)
		free(gestate_mgt.local_address);
	if(address)
		gestate_mgt.local_address = strdup(address);
	if(gestate_mgt.t_reset)
	{
		eloop_cancel(gestate_mgt.t_reset);
		gestate_mgt.t_reset = NULL;
	}
	gestate_mgt.t_reset = eloop_add_timer_msec(gestate_mgt.master, voip_estate_mgt_reset_eloop, &gestate_mgt, 100);
	return OK;
}

int voip_estate_mgt_port_set_api(u_int16 port)
{
	if(!x5_b_a_mgt)
		return ERROR;
	gestate_mgt.remote_port = port ? port : X5_B_A_PORT_DEFAULT;
	if(gestate_mgt.t_reset)
	{
		eloop_cancel(gestate_mgt.t_reset);
		gestate_mgt.t_reset = NULL;
	}
	gestate_mgt.t_reset = eloop_add_timer_msec(gestate_mgt.master, voip_estate_mgt_reset_eloop, &gestate_mgt, 100);
	return OK;
}

int voip_estate_mgt_local_port_set_api(u_int16 port)
{
	if(!x5_b_a_mgt)
		return ERROR;
	gestate_mgt.local_port = port ? port : X5_B_A_PORT_DEFAULT;
	if(gestate_mgt.t_reset)
	{
		eloop_cancel(gestate_mgt.t_reset);
		gestate_mgt.t_reset = NULL;
	}
	gestate_mgt.t_reset = eloop_add_timer_msec(gestate_mgt.master, voip_estate_mgt_reset_eloop, &gestate_mgt, 100);
	return OK;
}


/*
 * manage
 */
/*
int voip_estate_mgt_get_phone_number(x5_b_room_position_t *room, voip_position_room_t *out)
{
#ifdef VOIP_ESTATE_MGT_LOOPBACK
	memset(out, 0, sizeof(voip_position_room_t));
	sprintf(out->phone, "%s", "13922360951");
	V_APP_DEBUG("-----------%s:13922360951", __func__);
	return OK;
#else
	int ret = 0;
	if(voip_dbtest_isenable())
	{
		if(voip_dbtest_getphone(room->data, out->phone)!= OK)
		{
			if(VOIP_EMGT_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "Can not get this room:%s of phonenumber", room->data);
			x5_b_a_call_result_api(x5_b_a_mgt, E_CALL_RESULT_NO_SUCH_ROOM);
			return ERROR;
		}
		if(VOIP_EMGT_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Get this room:%s of phonenumber:%s", room->data, out->phone);
		return OK;
	}
	V_APP_DEBUG("-----------%s:", __func__);
	ret = voip_estate_mgt_write_and_wait_respone(&gestate_mgt, VOIP_ESTATE_MGT_TIMEOUT);
	if(ret != OK)
	{
		if(VOIP_EMGT_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Can not get this room:%s of phonenumber", room->data);
		x5_b_a_call_result_api(x5_b_a_mgt, E_CALL_RESULT_NO_SUCH_ROOM);
		return ERROR;
	}
	if(VOIP_EMGT_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Get this room:%s of phonenumber:%s", room->data, out->phone);
	return OK;
#endif
}
*/

int voip_estate_mgt_get_room_position(voip_estate_mgt_t *estate_mgt, char *position, int len)
{
	return voip_estate_mgt_write_and_wait_respone(&gestate_mgt, VOIP_ESTATE_MGT_TIMEOUT);
}

/*int voip_estate_mgt_get_phone_number(voip_estate_mgt_t *estate_mgt, int timeoutms)
{
	return voip_estate_mgt_write_and_wait_respone(estate_mgt, timeoutms);
}*/

/*int voip_estate_mgt_register(voip_estate_mgt_t *estate_mgt)
{
	return voip_estate_mgt_send_msg(estate_mgt);
}*/

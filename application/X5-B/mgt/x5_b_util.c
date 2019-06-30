/*
 * x5_b_util.c
 *
 *  Created on: 2019年5月20日
 *      Author: DELL
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
#include "os_util.h"
#include "os_socket.h"
#include "eloop.h"
#include "uci_ubus.h"

#include "x5_b_cmd.h"
#include "x5_b_app.h"
#include "x5_b_util.h"



uint16_t Data_CRC16Check ( uint8_t * data, uint16_t leng )  // crc 校验
{
    uint16_t i, j;
    uint16_t crcvalue = 0;
    if ( leng <= 0 )
    {
        return ( 0 );
    }
    for ( j = 0; j < leng; j++ )
    {
        crcvalue = crcvalue ^ ( int ) * data++ << 8;

        for ( i = 0; i < 8; ++i )
        {
            if ( crcvalue & 0x8000 )
            {
                crcvalue = crcvalue << 1 ^ 0x1021;
            }
            else
            {
                crcvalue = crcvalue << 1;
            }
        }
    }
    return crcvalue;
}

int x5b_app_hex_debug(x5b_app_mgt_t *mgt, char *hdr, int rx)
{
	char buf[1200];
	char tmp[16];
	u_int8 *p = NULL;
	int i = 0;
	int len = 0;
	zassert(mgt != NULL);
	zassert(hdr != NULL);
	if(mgt->not_debug == TRUE)
	{
		mgt->not_debug = FALSE;
		return OK;
	}
	if(rx)
	{
		len = (int)mgt->len;
		p = (u_int8 *)mgt->buf;
	}
	else
	{
		if(!mgt->app)
			return 0;
		len = mgt->app->slen;
		p = (u_int8 *)mgt->app->sbuf;
	}
	memset(buf, 0, sizeof(buf));
	for(i = 0; i < MIN(len, 128); i++)
	{
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "0x%02x ", (u_int8)p[i]);
		if(i%6 == 0)
			strcat(buf, " ");
		if(i%12 == 0)
			strcat(buf, "\r\n");
		strcat(buf, tmp);
	}
	zlog_debug(ZLOG_APP, "%s : %s%s", hdr, buf, (len>128) ? "...":" ");
	return OK;
}

int x5b_app_statistics(x5b_app_mgt_t *mgt, int tx, int from)
{
	if(mgt)
	{
		if(from == X5B_APP_MODULE_ID_A)
		{
			if(tx)
				mgt->app_a.statistics.tx_packet += 1;
			else
				mgt->app_a.statistics.rx_packet += 1;
		}
		else if(from == X5B_APP_MODULE_ID_C)
		{
			if(tx)
				mgt->app_c.statistics.tx_packet += 1;
			else
				mgt->app_c.statistics.rx_packet += 1;
		}
	}
	return OK;
}




#ifdef X5B_APP_TCP_ENABLE
static int x5b_app_tcp_connect_init(x5b_app_mgt_t *mgt)
{
	if(mgt->app_c.reg_state && mgt->tcp.w_fd == 0)
	{
		mgt->tcp.w_fd = sock_create(TRUE);
		if(mgt->tcp.w_fd)
		{
			if(sock_connect(mgt->tcp.w_fd, inet_address(mgt->app_c.address), mgt->local_port + 1) == OK)
			{
				zlog_debug(ZLOG_APP, "Connect to %s:%d OK", inet_address(mgt->app_c.address),
						mgt->local_port + 1);
				return OK;
			}
			zlog_err(ZLOG_APP, "Can not Connect to %s:%d(:%s)", inet_address(mgt->app_c.address),
					mgt->local_port + 1, strerror(errno));
			close(mgt->tcp.w_fd);
			mgt->tcp.w_fd = 0;
			return ERROR;
		}
		zlog_err(ZLOG_APP, "Can not Create client TCP socket(:%s)", strerror(errno));
	}
	return ERROR;
}

static int x5b_app_tcp_socket_init(x5b_app_mgt_t *mgt)
{
	int fd = sock_create(TRUE);
	if(fd)
	{
		if(sock_bind(fd, mgt->local_address, mgt->local_port + 1) == OK)
		{
			sock_listen(fd, 3);
			mgt->accept_fd = fd;
			zlog_debug(ZLOG_APP, "========> add read fd=%d", fd);
			x5b_app_event_active(mgt, X5B_TCP_ACCEPT_EV, 0, 0);
			//mgt->accept_thread = eloop_add_read(mgt->master, x5b_app_accept_eloop, mgt, fd);
			return OK;
		}
		else
		{
			zlog_err(ZLOG_APP, "Can not bind TCP socket(:%s)", strerror(errno));
		}
	}
	zlog_err(ZLOG_APP, "Can not Create TCP socket(:%s)", strerror(errno));
	return ERROR;
}
#endif


int x5b_app_socket_init(x5b_app_mgt_t *mgt)
{
	zassert(mgt != NULL);
	if(mgt->r_fd > 0)
		return OK;
	int fd = sock_create(FALSE);
	if(fd)
	{
		if(mgt->local_port == 0)
			mgt->local_port = X5B_APP_PORT_DEFAULT;
		//zlog_debug(ZLOG_APP, "sock_bind %s:%d", mgt->local_address ? mgt->local_address:"any", mgt->local_port);
		if(sock_bind(fd, mgt->local_address, mgt->local_port) == OK)
		{
			mgt->r_fd = fd;
			mgt->w_fd = fd;

			setsockopt_so_recvbuf (fd, 8192);
			setsockopt_so_sendbuf (fd, 8192);

			//zlog_debug(ZLOG_APP, "========> add read fd=%d", fd);
			x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
			//mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, fd);
#ifdef X5B_APP_TCP_ENABLE
			x5b_app_tcp_socket_init(mgt);
#endif
			//mgt->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt, mgt->interval + 5);
			return OK;
		}
		else
		{
			zlog_err(ZLOG_APP, "Can not bind UDP socket(:%s)", strerror(errno));
		}
	}
	zlog_err(ZLOG_APP, "Can not Create UDP socket(:%s)", strerror(errno));
	return ERROR;
}

int x5b_app_socket_exit(x5b_app_mgt_t *mgt)
{
	if(mgt && mgt->r_thread)
	{
		eloop_cancel(mgt->r_thread);
		mgt->r_thread = NULL;
	}
#ifdef X5B_APP_TCP_ENABLE
	if(mgt && mgt->accept_thread)
	{
		eloop_cancel(mgt->accept_thread);
		mgt->accept_thread = NULL;
	}
#endif
	if(mgt && mgt->reset_thread)
	{
		eloop_cancel(mgt->reset_thread);
		mgt->reset_thread = NULL;
	}
#ifdef X5B_APP_TCP_ENABLE
	if(mgt && mgt->tcp.r_thread)
	{
		eloop_cancel(mgt->tcp.r_thread);
		mgt->tcp.r_thread = NULL;
	}
#endif
	if(mgt && mgt->app_a.t_thread)
	{
		eloop_cancel(mgt->app_a.t_thread);
		mgt->app_a.t_thread = NULL;
	}
	if(mgt && mgt->app_c.t_thread)
	{
		eloop_cancel(mgt->app_c.t_thread);
		mgt->app_c.t_thread = NULL;
	}
	if(mgt)
	{
#ifdef X5B_APP_TCP_ENABLE
		if(mgt->tcp.r_fd)
		{
			close(mgt->tcp.r_fd);
			mgt->tcp.r_fd = 0;
		}
		if(mgt->tcp.w_fd)
		{
			close(mgt->tcp.w_fd);
			mgt->tcp.w_fd = 0;
		}
		if(mgt->accept_fd)
			close(mgt->accept_fd);
		mgt->accept_fd = 0;
#endif
		if(mgt->r_fd)
			close(mgt->r_fd);
		mgt->r_fd = 0;
		memset(mgt->buf, 0, sizeof(mgt->buf));
		mgt->r_fd = 0;
		mgt->w_fd = 0;
/*		mgt->state = 0;*/

		mgt->app_a.statistics.tx_packet = 0;
		mgt->app_a.statistics.rx_packet = 0;

		mgt->app_c.statistics.tx_packet = 0;
		mgt->app_c.statistics.rx_packet = 0;
		return OK;
	}
	return ERROR;
}


/**************************************************/
static int x5b_app_reset_eloop(struct eloop *eloop)
{
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	mgt->reset_thread = NULL;
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RESET mgt socket OK");
	x5b_app_socket_exit(mgt);
	x5b_app_socket_init(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

static int x5b_app_timer_eloop(struct eloop *eloop)
{
	zassert(eloop != NULL);
	x5b_app_mgt_node_t *mgt_priv = ELOOP_ARG(eloop);
	zassert(mgt_priv != NULL);
	zassert(mgt_priv->priv != NULL);
	x5b_app_mgt_t *mgt = mgt_priv->priv;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	mgt_priv->t_thread = NULL;

	if(!mgt->app_a.reg_state || !mgt->app_a.id)
	{
		if(mgt->app_a.version)
		{
			free(mgt->app_a.version);
			mgt->app_a.version = NULL;
		}
		if(mgt->app_a.version == NULL)
		{
			mgt->app_a.reg_state = TRUE;
			mgt->app_a.remote_port = X5B_APP_PORT_DEFAULT;
#ifdef X5B_APP_STATIC_IP_ENABLE
			mgt->app_a.address = ntohl(inet_addr(X5B_APP_A_IP_DEFAULT));
#endif
			if(x5b_app_version_request(mgt, E_CMD_TO_A) == OK)
			{
				mgt->app_a.id = X5B_APP_MODULE_ID_A;
				mgt->app_a.reg_state = TRUE;
			}
		}
	}
	if(mgt->X5CM)
	{
		if(!mgt->app_c.reg_state || !mgt->app_c.id)
		{
			if(mgt->app_c.version)
			{
				free(mgt->app_c.version);
				mgt->app_c.version = NULL;
			}
			if(mgt->app_c.version == NULL)
			{
				mgt->app_c.reg_state = TRUE;
				mgt->app_c.remote_port = X5B_APP_PORT_DEFAULT;
#ifdef X5B_APP_STATIC_IP_ENABLE
				mgt->app_c.address = ntohl(inet_addr(X5B_APP_C_IP_DEFAULT));
#endif
				if(x5b_app_version_request(mgt, E_CMD_TO_C) == OK)
				{
					mgt->app_c.id = X5B_APP_MODULE_ID_C;
					mgt->app_c.reg_state = TRUE;
				}
			}
		}
	}

	if(!mgt->time_sync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);

		x5b_app_rtc_request(mgt, E_CMD_TO_A);

		if(mgt->mutex)
			os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
		x5b_app_event_active(mgt, X5B_TIMER_EV, mgt_priv->id, 0);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return OK;
	}
/*	if(mgt->state)
		mgt->state--;*/

	if(mgt_priv->id == X5B_APP_MODULE_ID_A)
	{
		if(mgt_priv->reg_state)
		{
			if(mgt->app_a.version == NULL)
			{
				x5b_app_version_request(mgt, E_CMD_TO_A);
			}
/*			if(mgt->app_a.face_snyc == 0)
			{
				x5b_app_face_id_request(mgt, E_CMD_TO_C);
			}*/
		}

		if(mgt_priv->reg_state && !mgt->upgrade)
			x5b_app_keepalive_send(mgt, 0, E_CMD_TO_A);
		//x5b_app_send_msg(mgt);
	}
	else if(mgt_priv->id == X5B_APP_MODULE_ID_C)
	{
		if(mgt_priv->reg_state)
		{
			if(mgt->app_c.version == NULL)
			{
				x5b_app_version_request(mgt, E_CMD_TO_C);
			}
/*			if(mgt->app_c.face_snyc == 0)
			{
				//x5b_app_face_id_request(mgt, E_CMD_TO_C);
			}*/
		}

		if(mgt_priv->reg_state && !mgt->upgrade)
			x5b_app_keepalive_send(mgt, 0, E_CMD_TO_C);
		//x5b_app_send_msg(mgt);
	}
	mgt_priv->keep_cnt--;
	if(mgt_priv->keep_cnt == 0)
	{
		if(mgt_priv->id == X5B_APP_MODULE_ID_A)
			;//zlog_debug(ZLOG_APP, "===================%s A Module keepalive timeout clear state", __func__);
		else if(mgt_priv->id == X5B_APP_MODULE_ID_C)
			;//zlog_debug(ZLOG_APP, "===================%s C Module keepalive timeout clear state", __func__);
	}
/*	if(!X5B_APP_MGT_STATE_OK(mgt_priv))
	{
		mgt_priv->reg_state = FALSE;
		mgt_priv->address = 0;
		mgt_priv->id = 0;
		mgt_priv->msg_sync = FALSE;
		zlog_debug(ZLOG_APP, "===================%s keepalive timeout clear state", __func__);
	}
	if(!mgt_priv->reg_state)
		x5b_app_AC_state_save(mgt);*/
/*	if(mgt_priv->reg_state)
		mgt_priv->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt_priv, mgt_priv->interval);*/
	//else if(mgt_priv->id == X5B_APP_MODULE_ID_C)
	x5b_app_event_active(mgt, X5B_TIMER_EV, mgt_priv->id, 0);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}


static int x5b_app_read_eloop(struct eloop *eloop)
{
	int sock_len, len = 0;
	//struct sockaddr_in from;
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	int sock = ELOOP_FD(eloop);

	//zlog_debug(ZLOG_APP, "========> x5b_app_read_eloop read fd=%d", mgt->r_fd);

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	if(mgt->upgrade)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return OK;
	}
	//ELOOP_VAL(X)
	mgt->r_thread = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));
	//if(X5_B_ESP32_DEBUG(EVENT))
	//	zlog_debug(ZLOG_APP, "RECV mgt on socket");
	//memset(&from, 0, sizeof(from));
	sock_len = sizeof(struct sockaddr_in);
	len = recvfrom(sock, mgt->buf, sizeof(mgt->buf), 0, &mgt->from, &sock_len);
/*	zlog_debug(ZLOG_APP, "MSG from %s:%d %d byte", inet_address(ntohl(from.sin_addr.s_addr)),
			ntohs(from.sin_port), len);*/
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				zlog_err(ZLOG_APP, "RECV mgt on socket (%s)", strerror(errno));
				mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5b_app_reset_eloop, mgt, 100);
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
		}
	}
	else
	{
		if(len > X5B_APP_BUF_DEFAULT)
		{
			zlog_err(ZLOG_APP, "Recv buf size is too big on socket (%d byte)", len);
			if(mgt->r_thread == NULL)
				x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		}
		mgt->len = len;
		if(X5_B_ESP32_DEBUG(RECV))
		{
			zlog_debug(ZLOG_APP, "MSG from %s:%d %d byte", inet_address(ntohl(mgt->from.sin_addr.s_addr)),
					ntohs(mgt->from.sin_port), mgt->len);

			if(X5_B_ESP32_DEBUG(HEX))
				x5b_app_hex_debug(mgt, "RECV", 1);
		}
		if(ntohl(mgt->from.sin_addr.s_addr) == mgt->app_a.address)
		{
			mgt->fromid = X5B_APP_MODULE_ID_A;
			x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_A);
		}
		else if(ntohl(mgt->from.sin_addr.s_addr) == mgt->app_c.address)
		{
			mgt->fromid = X5B_APP_MODULE_ID_C;
			x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_C);
		}
		//x5b_app_read_handle(mgt);
		//if(mgt->mutex)
		//	os_mutex_unlock(mgt->mutex);
		x5b_app_read_handle(mgt);
		//if(mgt->mutex)
		//	os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	}
//	zlog_debug(ZLOG_APP, "========> add read fd=%d", sock);
	if(mgt->r_thread == NULL)
		x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
		//mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, sock);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

#ifdef X5B_APP_TCP_ENABLE
static int x5b_app_accept_eloop(struct eloop *eloop)
{
	struct sockaddr_in client;
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	int accept_sock = ELOOP_FD(eloop);
	mgt->accept_thread = NULL;
	if(mgt->tcp.r_fd)
		close(mgt->tcp.r_fd);
	mgt->tcp.r_fd = sock_accept (accept_sock, &client);
	//mgt->tcp.w_fd = mgt->tcp.r_fd;
	zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
	x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
	//mgt->tcp.r_thread = eloop_add_read(mgt->master, x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);
	zlog_debug(ZLOG_APP, "========> add read fd=%d", accept_sock);
	x5b_app_event_active(mgt, X5B_TCP_ACCEPT_EV, 0, 0);
	//mgt->accept_thread = eloop_add_read(mgt->master, x5b_app_accept_eloop, mgt, accept_sock);
	return OK;
}

static int x5b_app_tcp_read_eloop(struct eloop *eloop)
{
	int len;
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	int sock = ELOOP_FD(eloop);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_hdr_t *hdr = (x5b_app_hdr_t *) mgt->tcp.hdr_buf;
	mgt->tcp.r_thread = NULL;

	if (X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RECV mgt on TCP socket");

	if (mgt->tcp.hdr_len == 0)
	{
		len = ip_read(sock, mgt->tcp.hdr_buf, sizeof(x5b_app_hdr_t));
		if (len == sizeof(x5b_app_hdr_t))
		{
			mgt->tcp.len = ntohl(hdr->total_len) + 2;
			mgt->tcp.hdr_len = sizeof(x5b_app_hdr_t);
		}
		else if (len > 0)
		{
			mgt->tcp.hdr_len = len;
			zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
/*			mgt->tcp.r_thread = eloop_add_read(mgt->master,
					x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);*/
			x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		} else
		{
			if (ERRNO_IO_RETRY(errno))
			{
				zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
		/*		mgt->tcp.r_thread = eloop_add_read(mgt->master,
						x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);*/
				x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
			else
			{
				close(mgt->tcp.r_fd);
					mgt->tcp.r_fd = 0;
/*				close(mgt->tcp.w_fd);
					mgt->tcp.w_fd = 0;*/
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
		}
		if (mgt->tcp.len < sizeof(mgt->buf))
			mgt->tcp.buf = mgt->buf;
		else
		{
			mgt->tcp.buf = XMALLOC(MTYPE_TMP, mgt->tcp.len + 16);
			if (mgt->tcp.buf == NULL)
			{
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
		}
	}
	else
	{
		if (mgt->tcp.hdr_len < sizeof(x5b_app_hdr_t))
		{
			len = ip_read(sock, mgt->tcp.hdr_buf + mgt->tcp.hdr_len,
					sizeof(x5b_app_hdr_t) - mgt->tcp.hdr_len);
			if (len == sizeof(x5b_app_hdr_t) - mgt->tcp.hdr_len)
			{
				mgt->tcp.len = ntohl(hdr->total_len) + 2;
				mgt->tcp.hdr_len = sizeof(x5b_app_hdr_t);
			}
			else if (len > 0)
			{
				mgt->tcp.hdr_len = len;
				zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
				/*mgt->tcp.r_thread = eloop_add_read(mgt->master,
						x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);*/
				x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				return OK;
			}
			else
			{
				if (ERRNO_IO_RETRY(errno))
				{
					zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
					/*mgt->tcp.r_thread = eloop_add_read(mgt->master,
							x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);*/
					x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
					if(mgt->mutex)
						os_mutex_unlock(mgt->mutex);
					return OK;
				}
				else
				{
					close(mgt->tcp.r_fd);
						mgt->tcp.r_fd = 0;
/*					close(mgt->tcp.w_fd);
						mgt->tcp.w_fd = 0;*/
					if(mgt->mutex)
						os_mutex_unlock(mgt->mutex);
					return OK;
				}
			}
			if (mgt->tcp.len < sizeof(mgt->buf))
				mgt->tcp.buf = mgt->buf;
			else
			{
				mgt->tcp.buf = XMALLOC(MTYPE_TMP, mgt->tcp.len + 16);
				if (mgt->tcp.buf == NULL)
				{
					if(mgt->mutex)
						os_mutex_unlock(mgt->mutex);
					return OK;
				}
			}
		}
	}
	zassert(mgt->tcp.buf != NULL);
	len = ip_read(sock, mgt->tcp.buf + mgt->tcp.offset,
			mgt->tcp.len - mgt->tcp.offset);
	if (len > 0)
	{
		mgt->tcp.offset += len;
		if (mgt->tcp.offset == mgt->tcp.len)
		{
			mgt->len = mgt->tcp.len;
			if (X5_B_ESP32_DEBUG(RECV))
			{
				zlog_debug(ZLOG_APP, "MSG TCP from %d byte", mgt->len);

				if (X5_B_ESP32_DEBUG(HEX))
					x5b_app_hex_debug(mgt, "RECV", 1);
			}
			mgt->tcp_r = TRUE;
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			x5b_app_read_handle(mgt);
			if(mgt->mutex)
				os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
		}
		zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
		x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
		/*mgt->tcp.r_thread = eloop_add_read(mgt->master, x5b_app_tcp_read_eloop,
				mgt, mgt->tcp.r_fd);*/
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return OK;
	}
	else
	{
		if (ERRNO_IO_RETRY(errno))
		{
			zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
			/*mgt->tcp.r_thread = eloop_add_read(mgt->master,
					x5b_app_tcp_read_eloop, mgt, mgt->tcp.r_fd);*/
			x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		}
		else
		{
			close(mgt->tcp.r_fd);
				mgt->tcp.r_fd = 0;
/*			close(mgt->tcp.w_fd);
				mgt->tcp.w_fd = 0;*/
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
			return OK;
		}
	}
	zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->tcp.r_fd);
	x5b_app_event_active(mgt, X5B_TCP_READ_EV, 0, 0);
/*	mgt->tcp.r_thread = eloop_add_read(mgt->master, x5b_app_tcp_read_eloop, mgt,
			mgt->tcp.r_fd);*/
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}
#endif


int x5b_app_event_active(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who, int value)
{
	zassert(mgt != NULL);
	switch(ev)
	{
		case X5B_TIMER_EV:
			if(who == X5B_APP_MODULE_ID_A)
			{
				if(mgt->app_a.reg_state && mgt->master)
					mgt->app_a.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_a, mgt->app_a.interval);
			}
			else if(who == X5B_APP_MODULE_ID_C)
			{
				if(mgt->app_c.reg_state && mgt->master)
					mgt->app_c.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_c, mgt->app_c.interval);
			}
			break;
		case X5B_READ_EV:
			if(mgt->master && mgt->r_fd)
				mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, mgt->r_fd);
			break;
		case X5B_WRITE_EV:
			break;
		case X5B_RESET_EV:
			if(mgt->master)
				mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5b_app_reset_eloop, mgt, 100);
			break;
		case X5B_REPORT_EV:
			break;
#ifdef X5B_APP_TCP_ENABLE
		case X5B_TCP_ACCEPT_EV:
			if(mgt->master && mgt->accept_fd)
				mgt->accept_thread = eloop_add_read(mgt->master, x5b_app_accept_eloop, mgt, mgt->accept_fd);
			break;
		case X5B_TCP_READ_EV:
			if(mgt->master && mgt->tcp.r_fd)
				mgt->tcp.r_thread = eloop_add_read(mgt->master, x5b_app_tcp_read_eloop, mgt,
					mgt->tcp.r_fd);
			break;
		case X5B_TCP_WRITE_EV:
			break;
#endif
	}
	return OK;
}

int x5b_app_event_inactive(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who)
{
	zassert(mgt != NULL);
	switch(ev)
	{
		case X5B_TIMER_EV:
			if(who == X5B_APP_MODULE_ID_A)
			{
				if(mgt->app_a.t_thread)
				{
					eloop_cancel(mgt->app_a.t_thread);
					mgt->app_a.t_thread = NULL;
				}
			}
			else if(who == X5B_APP_MODULE_ID_C)
			{
				if(mgt->app_c.t_thread)
				{
					eloop_cancel(mgt->app_c.t_thread);
					mgt->app_c.t_thread = NULL;
				}
			}
			break;
		case X5B_READ_EV:
			if(mgt->r_thread)
			{
				eloop_cancel(mgt->r_thread);
				mgt->r_thread = NULL;
			}
			break;
		case X5B_WRITE_EV:
			break;
		case X5B_RESET_EV:
			if(mgt->reset_thread)
			{
				eloop_cancel(mgt->reset_thread);
				mgt->reset_thread = NULL;
			}
			break;
		case X5B_REPORT_EV:
			break;
#ifdef X5B_APP_TCP_ENABLE
		case X5B_TCP_ACCEPT_EV:
			if(mgt->accept_thread)
			{
				eloop_cancel(mgt->accept_thread);
				mgt->accept_thread = NULL;
			}
			break;
		case X5B_TCP_READ_EV:
			if(mgt->accept_thrtcp.r_threadead)
			{
				eloop_cancel(mgt->tcp.r_thread);
				mgt->tcp.r_thread = NULL;
			}
			break;
		case X5B_TCP_WRITE_EV:
			break;
#endif
	}
	return OK;
}


/******************************************************************************/
int x5b_app_read_chk_handle(x5b_app_mgt_t *mgt)
{
	int len = 0;//, offset = 0;
	u_int16 crc1 = 0;
	u_int16 *crc = NULL;
	zassert(mgt != NULL);
#ifdef X5B_APP_TCP_ENABLE
	if(mgt->tcp_r)
	{
		zassert(mgt->tcp.buf != NULL);
		x5b_app_hdr_t *hdr = mgt->tcp.hdr_buf;
/*		if(ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2!= mgt->len)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "TOTAL len is not same to msg len(%d != %d)",
						ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2, mgt->len);
			return ERROR;
		}*/
		crc1 =  Data_CRC16Check (mgt->tcp.buf,  mgt->len - 2);
		crc = (u_int16 *)&mgt->buf[mgt->len - 2];
		len = mgt->tcp.len - 2;
		//offset += 0;
		mgt->seqnum = hdr->seqnum;
	}
	else
#endif
	{
		x5b_app_hdr_t *hdr = mgt->buf;
		//x5b_app_statistics(mgt, 0);
		if(ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2!= mgt->len)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "TOTAL len is not same to msg len(%d != %d)",
						ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2, mgt->len);
			return ERROR;
		}
		crc1 =  Data_CRC16Check (mgt->buf,  mgt->len - 2);
		crc = (u_int16 *)&mgt->buf[mgt->len - 2];
	/*	if(*crc != htons(crc1))
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "CRC CHECK (%x != %x)", crc, crc1);
			return ERROR;
		}*/
		len = ntohl(hdr->total_len)/* - 2*/;
		if(len > X5B_APP_BUF_DEFAULT)
		{
			zlog_err(ZLOG_APP, "TLV buf size is too big on socket (%d byte)", len);
			return ERROR;
		}
		//offset += sizeof(x5b_app_hdr_t);
		mgt->seqnum = hdr->seqnum;
	}
	return len;
}


static int x5b_app_read_ack_handle(x5b_app_mgt_t *mgt, char *output, int outlen)
{
	int len = 0, offset = 0, ack = 0;
	os_tlv_t tlv;
	char add_tmp[64];
	zassert(mgt != NULL);
#ifdef X5B_APP_TCP_ENABLE
	if(mgt->tcp_r)
	{
		offset += 0;
	}
	else
#endif
	{
		offset += sizeof(x5b_app_hdr_t);
	}
	if(!mgt->app)
	{
		zlog_err(ZLOG_APP, "send module is null");
		return ERROR;
	}
	len = x5b_app_read_chk_handle(mgt);

	while(len > 0)
	{
		memset(&tlv, 0, sizeof(os_tlv_t));
#ifdef X5B_APP_TCP_ENABLE
		if(mgt->tcp_r)
			offset += os_tlv_get(mgt->tcp.buf + offset, &tlv);
		else
#endif
			offset += os_tlv_get(mgt->buf + offset, &tlv);
		if(tlv.len >= X5B_APP_TLV_DEFAULT)
			continue;
/*		if(E_CMD_FROM_C(tlv.tag))
		{
			return ERROR;
		}*/
		if(mgt->upgrade && output) /* Ymodem数据应答 */
		{
			if(E_CMD_TYPE_GET(tlv.tag) == E_CMD_UPDATE)
			{
				if(E_CMD_GET(tlv.tag) == E_CMD_UPDATE_DATA)
				{
					//zlog_debug(ZLOG_APP, "E_CMD_UPDATE_DATA");
					if(output)
					{
						//memcpy(buf, tlv.val.pval, MIN(l, tlv.len));
						output[0] = tlv.val.pval[0];//tlv.val.val8;
					}
					//memcpy(mgt->up_buf, tlv.val.pval, MIN(sizeof(mgt->up_buf), tlv.len));
					mgt->up_buf[0] = tlv.val.pval[0];//tlv.val.val8;
					mgt->up_buf_len = tlv.len;
					ack = OK;
					break;
				}
			}
		}
		else/* TLV数据应答 */
		{
			if(E_CMD_TYPE_GET(tlv.tag) == E_CMD_BASE)
			{
				if(E_CMD_GET(tlv.tag) == E_CMD_ACK)
				{
					//zlog_debug(ZLOG_APP, "E_CMD_ACK : seqnum 0x%02x", tlv.val.val8);
					mgt->ack_seqnum = tlv.val.pval[0];//tlv.val.val8;
					//os_tlv_get_byte
					if(mgt->sync_ack)
					{
						//mgt->sync_ack = FALSE;
						if(mgt->ack_seqnum == mgt->app->seqnum)
						{
							if(X5_B_ESP32_DEBUG(EVENT))
								zlog_debug(ZLOG_APP, "ACK msg (seqnum=%d) OK", mgt->app->seqnum);
							mgt->ack_seqnum = 0;
							ack = OK;
							break;
						}
						else
						{
							//if(X5_B_ESP32_DEBUG(EVENT))
							memset(add_tmp, 0, sizeof(add_tmp));
							sprintf(add_tmp, sizeof(add_tmp), "%s", inet_address(mgt->app->address));
							zlog_err(ZLOG_APP, "ACK msg (send seqnum=%d(%s) not same recv seqnum=%d(%s)) ERROR",
										 mgt->app->seqnum, add_tmp,
										 mgt->ack_seqnum, inet_address(ntohl(mgt->from.sin_addr.s_addr)));
							ack = ERROR;
							break;
						}
					}
					ack = ERROR;
					break;
				}
			}
		}
		len -= offset;
	}
#ifdef X5B_APP_TCP_ENABLE
	if(mgt->tcp_r)
	{
		mgt->tcp_r = FALSE;
		mgt->tcp.offset = 0;
		mgt->tcp.hdr_len = 0;
		mgt->tcp.len = 0;
		memset(mgt->tcp.hdr_buf, 0, sizeof(mgt->tcp.hdr_buf));
		if((mgt->tcp.len >= sizeof(mgt->buf)) && mgt->tcp.buf)
			XFREE(MTYPE_TMP, mgt->tcp.buf);
		mgt->tcp.buf = NULL;
	}
#endif
	return ack;
}

static int x5b_app_read_msg(int fd, x5b_app_mgt_t *mgt, char *output, int outlen)
{
	int sock_len, len = 0;
	zassert(mgt != NULL);
	memset(mgt->buf, 0, sizeof(mgt->buf));
	sock_len = sizeof(struct sockaddr_in);

	len = recvfrom(fd, mgt->buf, sizeof(mgt->buf), 0, &mgt->from, &sock_len);
	if (len <= 0)
	{
		zlog_debug(ZLOG_APP, "recvfrom:%s", strerror(errno));
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				return ERROR;
			}
		}
	}
	else
	{
		mgt->len = len;
		if(X5_B_ESP32_DEBUG(RECV))
		{
			zlog_debug(ZLOG_APP, "MSG from %s:%d %d byte", inet_address(ntohl(mgt->from.sin_addr.s_addr)),
					ntohs(mgt->from.sin_port), mgt->len);

			if(X5_B_ESP32_DEBUG(HEX))
				x5b_app_hex_debug(mgt, "RECV", 1);
		}
		//send to A and get respone from C
		if((ntohl(mgt->from.sin_addr.s_addr) == mgt->app_c.address) &&(mgt->app->address == mgt->app_a.address) )
		{
			return OS_TRY_AGAIN;
		}
		else if((ntohl(mgt->from.sin_addr.s_addr) == mgt->app_a.address) &&(mgt->app->address == mgt->app_c.address) )
		{
			return OS_TRY_AGAIN;
		}
		len = x5b_app_read_ack_handle(mgt, output, outlen);
		if(len == OK)
		{
			return OK;
		}
		else
			return ERROR;
	}
	return OK;
}

int x5b_app_read_msg_timeout(x5b_app_mgt_t *app, int timeout_ms, char *output, int outlen)
{
	int ret = 0, maxfd = 0;
	fd_set rfdset;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;

	zassert(mgt != NULL);

    FD_ZERO( &rfdset );
#ifdef X5B_APP_TCP_ENABLE
    if(mgt->tcp_r)
    {
        FD_SET( mgt->tcp.r_fd, &rfdset );
        maxfd = mgt->tcp.r_fd + 1;
    }
    else
#endif
    {
        FD_SET( mgt->r_fd, &rfdset );
        maxfd = mgt->r_fd + 1;
    }
try_again:
	ret = os_select_wait(maxfd, &rfdset, NULL, timeout_ms);
	if(ret > 0)
	{
#ifdef X5B_APP_TCP_ENABLE
	    if(mgt->tcp_r)
	    {
	    	if (FD_ISSET( mgt->tcp.r_fd, &rfdset))
	    	{
	    		ret = x5b_app_read_msg(mgt->tcp.r_fd, mgt, output, outlen);
	    		if(ret == OS_TRY_AGAIN)
	    			goto try_again;
	    		return ret;
	    	}
	    }
	    else
#endif
	    {
	    	if (FD_ISSET( mgt->r_fd, &rfdset))
	    	{
	    		ret = x5b_app_read_msg(mgt->r_fd, mgt, output, outlen);
	    		if(ret == OS_TRY_AGAIN)
	    			goto try_again;
	    		return ret;
	    	}
	    }
	}
	else if(ret == OS_TIMEOUT)
	{
        zlog_debug(ZLOG_APP, "wait timeout from:%s", inet_address(mgt->app->address));
        return OS_TIMEOUT;
	}
	zlog_debug(ZLOG_APP, "wait from %s error:%s", inet_address(mgt->app->address), strerror(errno));
	return ERROR;
}








int x5b_app_send_msg_without_ack(x5b_app_mgt_t *mgt)
{
	int len = 0;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	zassert(mgt->app->address != 0);
	{
		if(X5_B_ESP32_DEBUG(SEND))
		{
			zlog_debug(ZLOG_APP, "MSG to %s:%d %d byte (seqnum=%d)", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen, mgt->app->seqnum);
			if(X5_B_ESP32_DEBUG(HEX))
				x5b_app_hex_debug(mgt, "SEND", 0);
		}
		if(mgt->w_fd)
			len = sock_client_write(mgt->w_fd, inet_address(mgt->app->address), mgt->app->remote_port,
					mgt->app->sbuf, mgt->app->slen);
	}
	x5b_app_statistics(mgt, 1, mgt->app->id);
	mgt->app->seqnum++;
	return len ? OK : ERROR;
}


int x5b_app_send_msg(x5b_app_mgt_t *mgt)
{
	int len = 0;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	zassert(mgt->app->address != 0);
#ifdef X5B_APP_TCP_ENABLE
	if(mgt->tcp_r)
	{
		if(tcp_sock_state(mgt->tcp.w_fd) != TCP_ESTABLISHED)
		{
			close(mgt->tcp.w_fd);
			mgt->tcp.w_fd = 0;
			x5b_app_tcp_connect_init(mgt);
		}
		if(mgt->tcp.w_fd)
			len = os_write_timeout(mgt->tcp.w_fd,
					mgt->app->sbuf, mgt->app->slen, 2000);
		if(len == ERROR)
		{
			if(!ERRNO_IO_RETRY(errno))
			{
				close(mgt->tcp.w_fd);
				mgt->tcp.w_fd = 0;
				x5b_app_tcp_connect_init(mgt);
			}
			mgt->app->seqnum++;
			return ERROR;
		}
		if(X5_B_ESP32_DEBUG(SEND))
		{
			zlog_debug(ZLOG_APP, "MSG to %s:%d %d byte (seqnum=%d)", inet_address(mgt->app->address),
					mgt->local_port + 1, mgt->app->slen, mgt->app->seqnum);
			if(X5_B_ESP32_DEBUG(HEX))
				x5b_app_hex_debug(mgt, "SEND", 0);
		}
	}
	else
#endif
	{
		if(mgt->app->slen >= X5B_APP_BUF_DEFAULT)
		{
			zlog_err(ZLOG_APP, "MSG(to %s:%d) Size is too big", inet_address(mgt->app->address),
					mgt->app->remote_port);
			return ERROR;
		}
		if(X5_B_ESP32_DEBUG(SEND))
		{
			zlog_debug(ZLOG_APP, "MSG to %s:%d %d byte (seqnum=%d)", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen, mgt->app->seqnum);
			if(X5_B_ESP32_DEBUG(HEX))
				x5b_app_hex_debug(mgt, "SEND", 0);
		}
		if(mgt->w_fd)
			len = sock_client_write(mgt->w_fd, inet_address(mgt->app->address), mgt->app->remote_port,
					mgt->app->sbuf, mgt->app->slen);
	}
	x5b_app_statistics(mgt, 1, mgt->app->id);
	if(mgt->sync_ack)
	{
		//zlog_debug(ZLOG_APP, "----wait ack" );
#ifdef X5B_APP_TCP_ENABLE
		if(!mgt->tcp_r)
#endif
		{
			int ret = 0;
			if(mgt->r_thread)
			{
				eloop_cancel(mgt->r_thread);
				mgt->r_thread = NULL;
			}
			//zlog_debug(ZLOG_APP, "----wait ack 1" );
			ret = x5b_app_read_msg_timeout(mgt, X5B_APP_WAITING_TIMEOUT, NULL, 0);
			if(!mgt->upgrade && mgt->r_thread == NULL)
			{
				//zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->r_fd);
				x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
				//mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, mgt->r_fd);
			}
			mgt->app->seqnum++;
			return ret;
		}
		mgt->app->seqnum++;
		return OK;
	}
	mgt->app->seqnum++;
	return len ? OK : ERROR;
}

/*************************************************************/











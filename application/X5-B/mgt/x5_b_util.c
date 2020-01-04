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







int x5b_app_socket_init(x5b_app_mgt_t *mgt)
{
	zassert(mgt != NULL);
	if(mgt->r_fd > 0)
		return OK;
	int fd = sock_create(FALSE);
	if(fd)
	{
		if(mgt->local_port == 0)
			mgt->local_port = X5B_APP_LOCAL_PORT_DEFAULT;
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
	if(mgt && mgt->t_thread)
	{
		eloop_cancel(mgt->t_thread);
		mgt->t_thread = NULL;
	}

	if(mgt && mgt->reset_thread)
	{
		eloop_cancel(mgt->reset_thread);
		mgt->reset_thread = NULL;
	}

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
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	//return 0;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt->t_thread = NULL;
#if 0
#ifdef X5B_APP_TIMESYNC_C
	if(!mgt->time_sync && mgt->app_c.reg_state)
#else
	if(!mgt->time_sync && mgt->app_a.reg_state)
#endif
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
#ifdef X5B_APP_TIMESYNC_C
		if(x5b_app_mode_X5CM())
			x5b_app_rtc_request(mgt, E_CMD_TO_C);
		else
#endif
			x5b_app_rtc_request(mgt, E_CMD_TO_A);
		if(mgt->mutex)
			os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	}
#endif
#if 1
	if(mgt->app_a.reg_state)
	{
/*		if(strlen(mgt->app_a.version) != 0)
		{
			memset(mgt->app_a.version, 0, sizeof(mgt->app_a.version));
		}*/
		if(strlen(mgt->app_a.version) <= 0 &&
				mgt->app_a.reg_state)
		{
			x5b_app_version_request(mgt, E_CMD_TO_A);
		}
	}

	if(x5b_app_mode_X5CM() && mgt->app_c.reg_state)
	{
/*		if(strlen(mgt->app_c.version) != 0)
		{
			memset(mgt->app_c.version, 0, sizeof(mgt->app_c.version));
		}*/
		if(strlen(mgt->app_c.version) <= 0 &&
				mgt->app_c.reg_state)
		{
			x5b_app_version_request(mgt, E_CMD_TO_C);
		}
	}
#endif
	x5b_app_event_active(mgt, X5B_TIMER_EV, 0, 0);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

#define X5B_APP_KEEPALIVE_SAME_ONE 1

static int x5b_app_keepalive_eloop(struct eloop *eloop)
{
#if (X5B_APP_KEEPALIVE_SAME_ONE == 0)
	zassert(eloop != NULL);
	x5b_app_mgt_node_t *mgt_priv = ELOOP_ARG(eloop);
	zassert(mgt_priv != NULL);
	zassert(mgt_priv->priv != NULL);
	x5b_app_mgt_t *mgt = mgt_priv->priv;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt_priv->t_thread = NULL;

	if(mgt_priv->id == X5B_APP_MODULE_ID_A)
	{
		if(mgt_priv->reg_state && !mgt->upgrade)
			x5b_app_keepalive_send(mgt, 0, E_CMD_TO_A);
	}
	else if(mgt_priv->id == X5B_APP_MODULE_ID_C)
	{
		if(mgt_priv->reg_state && !mgt->upgrade)
			x5b_app_keepalive_send(mgt, 0, E_CMD_TO_C);
	}
	mgt_priv->keep_cnt--;
	if(mgt_priv->keep_cnt == 0)
	{
		if(mgt_priv->id == X5B_APP_MODULE_ID_A)
			;//zlog_debug(ZLOG_APP, "===================%s A Module keepalive timeout clear state", __func__);
		else if(mgt_priv->id == X5B_APP_MODULE_ID_C)
			;//zlog_debug(ZLOG_APP, "===================%s C Module keepalive timeout clear state", __func__);
	}

	x5b_app_event_active(mgt, X5B_KEEPALIVE_EV, mgt_priv->id, 0);
#else
	zassert(eloop != NULL);
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	mgt->k_thread = NULL;

	if(mgt->app_a.reg_state && !mgt->upgrade)
	{
		x5b_app_keepalive_send(mgt, 0, E_CMD_TO_A);
		mgt->app_a.keep_cnt--;
	}
	if(x5b_app_mode_X5CM())
	{
		if(mgt->app_c.reg_state && !mgt->upgrade)
		{
			x5b_app_keepalive_send(mgt, 0, E_CMD_TO_C);
			mgt->app_c.keep_cnt--;
		}
	}
	if(mgt->app_a.keep_cnt == 0)
	{
		x5b_app_event_inactive(mgt, X5B_KEEPALIVE_EV, 0);

		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return OK;
	}
	if(x5b_app_mode_X5CM())
	{
		if(mgt->app_c.keep_cnt == 0)
		{
			x5b_app_event_inactive(mgt, X5B_KEEPALIVE_EV, 0);

			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
		}
	}
	x5b_app_event_active(mgt, X5B_KEEPALIVE_EV, 0, 0);
#endif
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

		if(!x5b_app_mode_X5CM() && ntohl(mgt->from.sin_addr.s_addr)==mgt->app_c.address)
		{
			if(mgt->r_thread == NULL)
				x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return OK;
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




int x5b_app_event_active(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who, int value)
{
	zassert(mgt != NULL);
	switch(ev)
	{
		case X5B_KEEPALIVE_EV:
#if (X5B_APP_KEEPALIVE_SAME_ONE == 0)
			if(who == X5B_APP_MODULE_ID_A)
			{
				if(mgt->app_a.reg_state && mgt->master)
					mgt->app_a.t_thread = eloop_add_timer(mgt->master, x5b_app_keepalive_eloop, &mgt->app_a, mgt->app_a.interval);
			}
			else if(who == X5B_APP_MODULE_ID_C)
			{
				if(mgt->app_c.reg_state && mgt->master)
					mgt->app_c.t_thread = eloop_add_timer(mgt->master, x5b_app_keepalive_eloop, &mgt->app_c, mgt->app_c.interval);
			}
#else
			if(x5b_app_mode_X5CM())
			{
				if(mgt->app_a.reg_state &&
						mgt->app_c.reg_state &&
						mgt->master &&
						!mgt->k_thread)
					mgt->k_thread = eloop_add_timer(mgt->master, x5b_app_keepalive_eloop, mgt, mgt->app_a.interval);
			}
			else
			{
				if(mgt->app_a.reg_state &&
						mgt->master &&
						!mgt->k_thread)
					mgt->k_thread = eloop_add_timer(mgt->master, x5b_app_keepalive_eloop, mgt, mgt->app_a.interval);
			}
#endif
			break;

		case X5B_TIMER_EV:
			if(mgt->master && !mgt->t_thread)
			{
				if(!mgt->time_sync)
					mgt->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt, X5B_APP_INTERVAL_DEFAULT);
				else
				{
					if(mgt->app_a.reg_state)
					{
						if(strlen(mgt->app_a.version) <= 0 &&
								mgt->app_a.reg_state)
						{
							mgt->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt, X5B_APP_INTERVAL_DEFAULT);
						}
					}

					if(x5b_app_mode_X5CM() && mgt->app_c.reg_state)
					{
						if(strlen(mgt->app_c.version) <= 0 &&
								mgt->app_c.reg_state)
						{
							if(!mgt->t_thread)
								mgt->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt, X5B_APP_INTERVAL_DEFAULT);
						}
					}
					if(!mgt->t_thread)
						mgt->t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, mgt, X5B_APP_INTERVAL_DEFAULT * 6);
				}
			}
			break;
		case X5B_READ_EV:
			if(mgt->master && mgt->r_fd  && !mgt->r_thread)
				mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, mgt->r_fd);
			break;
		case X5B_WRITE_EV:
			break;
		case X5B_RESET_EV:
			if(mgt->master  && !mgt->reset_thread)
				mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5b_app_reset_eloop, mgt, 100);
			break;
		case X5B_REPORT_EV:
			break;
	}
	return OK;
}

int x5b_app_event_inactive(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who)
{
	zassert(mgt != NULL);
	switch(ev)
	{
		case X5B_KEEPALIVE_EV:
#if (X5B_APP_KEEPALIVE_SAME_ONE == 0)
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
#else
			if(mgt->k_thread)
			{
				eloop_cancel(mgt->k_thread);
				mgt->k_thread = NULL;
			}
#endif
			break;
		case X5B_TIMER_EV:
			if(mgt->t_thread)
			{
				eloop_cancel(mgt->t_thread);
				mgt->t_thread = NULL;
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
	{
		x5b_app_hdr_t *hdr = mgt->buf;
		//x5b_app_statistics(mgt, 0);
		if(ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2!= mgt->len)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "TOTAL len is not same to msg len(%d != %d)",
						(int)(ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2), mgt->len);
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

		if(!x5b_app_mode_X5CM() && ntohl(mgt->from.sin_addr.s_addr)==mgt->app_c.address)
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

    {
        FD_SET( mgt->r_fd, &rfdset );
        maxfd = mgt->r_fd + 1;
    }
try_again:
	ret = os_select_wait(maxfd, &rfdset, NULL, timeout_ms);
	if(ret > 0)
	{

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
		if(mgt->app)
		{
			int offset = 0;
			os_tlv_t *tlv;
			//x5b_app_hdr_t *hdr = mgt->app->sbuf;
			offset += sizeof(x5b_app_hdr_t);
			tlv = (os_tlv_t *)(mgt->app->sbuf + offset);
			zlog_debug(ZLOG_APP, "CMD 0x%04x (seqnum:%d) wait timeout from:%s",
					E_CMD_GET(ntohl(tlv->tag)), mgt->app->seqnum, inet_address(mgt->app->address));
		}
		else
		{
			zlog_debug(ZLOG_APP, "wait timeout from:%s",inet_address(mgt->app->address));
		}
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
		{
			int ret = 0;
			if(mgt->r_thread)
			{
				eloop_cancel(mgt->r_thread);
				mgt->r_thread = NULL;
			}
			//zlog_debug(ZLOG_APP, "----wait ack 1" );
			if(mgt->wait_timeout)
				ret = x5b_app_read_msg_timeout(mgt, mgt->wait_timeout, NULL, 0);
			else
				ret = x5b_app_read_msg_timeout(mgt, X5B_APP_WAITING_TIMEOUT, NULL, 0);
			if(!mgt->upgrade && mgt->r_thread == NULL)
			{
				//zlog_debug(ZLOG_APP, "========> add read fd=%d", mgt->r_fd);
				x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
				//mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, mgt->r_fd);
			}
			mgt->wait_timeout = 0;
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











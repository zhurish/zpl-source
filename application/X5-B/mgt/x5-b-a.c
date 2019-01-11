/*
 * hello.c
 *
 *  Created on: 2018��12��18��
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
#include "os_tlv.h"

#include "x5_b_a.h"
#include "x5_b_ctl.h"


x5_b_a_mgt_t *x5_b_a_mgt = NULL;


static int x5_b_a_socket_init(x5_b_a_mgt_t *mgt);
static int x5_b_a_socket_exit(x5_b_a_mgt_t *mgt);


uint16_t Data_CRC16Check ( uint8_t * data, uint16_t leng )  // crc ��λ��ǰ
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

static int x5_b_a_hex_debug(x5_b_a_mgt_t *mgt, char *hdr, int rx)
{
	char buf[1200];
	char tmp[16];
	u_int8 *p = NULL;
	int i = 0;
	int len = (int)(rx ? mgt->len:mgt->slen);
	p = (u_int8 *)(rx ? mgt->buf:mgt->sbuf);
	memset(buf, 0, sizeof(buf));
	for(i = 0; i < len; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "0x%02x ", p[i]);
		if(i%6 == 0)
			strcat(buf, " ");
		if(i%12 == 0)
			strcat(buf, "\n");
		strcat(buf, tmp);
	}
	zlog_debug(ZLOG_APP, "%s : %s", hdr, buf);
	return OK;
}

static int x5_b_a_statistics(x5_b_a_mgt_t *mgt, int tx)
{
	if(mgt)
	{
		if(tx)
			mgt->statistics.tx_packet += 1;
		else
			mgt->statistics.rx_packet += 1;
	}
	return OK;
}

static int x5_b_a_send_msg(x5_b_a_mgt_t *mgt)
{
	int len = 0;
	if(X5_B_ESP32_DEBUG(SEND))
	{
		zlog_debug(ZLOG_APP, "MSG to %s:%d %d byte (seqnum=%d)", mgt->remote_address,
				mgt->remote_port, mgt->slen, mgt->s_seqnum);
		if(X5_B_ESP32_DEBUG(HEX))
			x5_b_a_hex_debug(mgt, "SEND", 0);
	}
/*	if(mgt->state != OK)
	{
		zlog_debug(ZLOG_APP, " Remote %s:%d is not online", mgt->remote_address, mgt->remote_port);
		return ERROR;
	}*/
	if(mgt->w_fd)
		len = sock_client_write(mgt->w_fd, mgt->remote_address, mgt->remote_port, mgt->sbuf, mgt->slen);
		//len = write(mgt->w_fd, mgt->sbuf, mgt->slen);
	x5_b_a_statistics(mgt, 1);
	return len ? OK : ERROR;
}


static int x5_b_a_hdr_make(x5_b_a_mgt_t *mgt)
{
	x5_b_a_hdr_t *hdr = (x5_b_a_hdr_t *)mgt->sbuf;
	memset(mgt->sbuf, 0, sizeof(mgt->sbuf));
	hdr->makr = X5_B_A_HDR_MAKR;
	hdr->total_len = 0;//;
	hdr->seqnum = (mgt->s_seqnum++);
	mgt->offset = sizeof(x5_b_a_hdr_t);
	return OK;
}

static int x5_b_a_crc_setup(char *buf, u_int16 crc)
{
	u_int16 *scrc = (u_int16 *)buf;
	*scrc = htons(crc);
	return OK;
}

static int x5_b_a_crc_make(x5_b_a_mgt_t *mgt)
{
	//u_int16 *crc = NULL;//(u_int16 *)&mgt->sbuf[0];
	u_int16	crc1 = 0;
	x5_b_a_hdr_t *hdr = (x5_b_a_hdr_t *)mgt->sbuf;
	mgt->slen = mgt->offset;
	hdr->total_len = htonl(mgt->slen - sizeof(x5_b_a_hdr_t));
	/*
	 * 所有报文以前计算校验和
	 */
	crc1 =  Data_CRC16Check (mgt->sbuf,  mgt->slen);
	//zlog_debug(ZLOG_APP, "%s (mgt->slen=%d)", __func__, mgt->slen);

	//*crc = (u_int16 *)(mgt->sbuf + mgt->slen);
	//crc =  Data_CRC16Check (mgt->sbuf + sizeof(x5_b_a_hdr_t),  hdr->total_len);
	x5_b_a_crc_setup(mgt->sbuf + mgt->slen, crc1);
	//*crc = htons(crc1);
	mgt->slen += 2;
	//zlog_debug(ZLOG_APP, "%s (crc=0x%x)", __func__, crc1);
/*	mgt->sbuf[mgt->slen] = crc>>8;
	mgt->slen++;
	mgt->sbuf[mgt->slen] = crc & 0xff;
	mgt->slen++;*/
	return mgt->slen;
}

static int x5_b_a_ack_make(x5_b_a_mgt_t *mgt, int seqnum)
{
	u_int32 val = (u_int32)seqnum;
	int len = os_tlv_set_integer(mgt->sbuf + mgt->offset, E_CMD_ACK, E_CMD_ACK_LEN, &val);
	mgt->offset += len;
	return (len);
}

static int x5_b_a_open_result_make(x5_b_a_mgt_t *mgt, int res)
{
	u_int32 val = (u_int32)res;
	int len = os_tlv_set_integer(mgt->sbuf + mgt->offset, E_CMD_RESULT, E_CMD_RESULT_LEN, &val);
	mgt->offset += len;
	//zlog_debug(ZLOG_APP, "%s ", __func__);
	return (len);
}

static int x5_b_a_call_result_make(x5_b_a_mgt_t *mgt, int res)
{
	u_int32 val = (u_int32)res;
	int len = os_tlv_set_integer(mgt->sbuf + mgt->offset, E_CMD_CALL_RESULT, E_CMD_CALL_RESULT_LEN, &val);
	mgt->offset += len;
	//zlog_debug(ZLOG_APP, "%s ", __func__);
	return (len);
}

static int x5_b_a_keepalive_make(x5_b_a_mgt_t *mgt, int res)
{
	u_int32 val = (u_int32)res;
	int len = os_tlv_set_integer(mgt->sbuf + mgt->offset, E_CMD_KEEPALIVE, E_CMD_KEEPALIVE_LEN, &val);
	mgt->offset += len;
	//zlog_debug(ZLOG_APP, "%s ", __func__);
	return (len);
}

static int x5_b_a_ack_send(x5_b_a_mgt_t *mgt, int seqnum)
{
	x5_b_a_hdr_make(mgt);
	x5_b_a_ack_make(mgt, seqnum);
	x5_b_a_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "ACK MSG to %s:%d %d byte(seqnum=%d)", mgt->remote_address,
				mgt->remote_port, mgt->slen, seqnum);
	return x5_b_a_send_msg(mgt);
}

static int x5_b_a_open_result_send(x5_b_a_mgt_t *mgt, int res)
{
	x5_b_a_hdr_make(mgt);
	x5_b_a_open_result_make(mgt, res);
	x5_b_a_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "OPEN RES MSG to %s:%d %d byte", mgt->remote_address,
				mgt->remote_port, mgt->slen);
	return x5_b_a_send_msg(mgt);
}

int x5_b_a_open_result_api(x5_b_a_mgt_t *mgt, int res)
{
	return x5_b_a_open_result_send(mgt, res);
}

static int x5_b_a_call_result_send(x5_b_a_mgt_t *mgt, int res)
{
	x5_b_a_hdr_make(mgt);
	x5_b_a_call_result_make(mgt, res);
	x5_b_a_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CALL RES MSG to %s:%d %d byte", mgt->remote_address,
				mgt->remote_port, mgt->slen);
	return x5_b_a_send_msg(mgt);
}

int x5_b_a_call_result_api(x5_b_a_mgt_t *mgt, int res)
{
	return x5_b_a_call_result_send(mgt, res);
}

static int x5_b_a_read_tlv_handle(x5_b_a_mgt_t *mgt, os_tlv_t *tlv)
{
	int ret = ERROR;
	//zlog_warn(ZLOG_APP, "x5_b_a_read_tlv_handle");
	switch(tlv->tag)
	{
	case E_CMD_KEEPALIVE:
		if(tlv->len != E_CMD_ACK_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "ACK msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			ret = OK;
			mgt->state ++;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "ACK msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	case E_CMD_ACK:
		if(tlv->len != E_CMD_ACK_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "ACK msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "ACK msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	case E_CMD_RESULT:
		if(tlv->len != E_CMD_RESULT_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "OPEN RESULT msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			x5_b_a_ack_send(mgt, mgt->seqnum);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
			{
				if(tlv->val.val32 == E_OPEN_RESULT_OK)
					zlog_debug(ZLOG_APP, "Door is open successfully");
				else
					zlog_debug(ZLOG_APP, "Door is open failure ");
			}
			//zlog_debug(ZLOG_APP, "OPEN RESULT msg (seqnum=%d) OK", mgt->s_seqnum);
		}
		break;
	case E_CMD_START_CALLING:
/*		if(tlv->len != E_CMD_START_CALLING_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "START CALLING msg (seqnum=%d) len error", mgt->seqnum);
		}
		else*/
		{
			//7e 00 00 00 00 0c 00 00 00 03 00 00 00 04 32 35 32 35 4d 54  2525
			//zlog_debug(ZLOG_APP, "START CALLING tlv->len:%d", tlv->len);
			int i = 0;
			memset(&mgt->room, 0, sizeof(mgt->room));
			for(i = 0; i < tlv->len; i++)
			{
				//mgt->room.data[i] = tlv->val.pval[i] - 0x30;
				mgt->room.data[i] = tlv->val.pval[i];
			}
			//memcpy(&mgt->room, tlv->val.pval, tlv->len);
			//zlog_debug(ZLOG_APP, "START CALLING msg (seqnum=%d) %s OK", mgt->seqnum, tlv->val.pval);
			x5_b_a_ack_send(mgt, mgt->seqnum);
			x5_b_start_call(TRUE, &mgt->room);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "START CALLING msg (seqnum=%d) room=%s OK", mgt->seqnum, mgt->room.data);
		}
		break;
	case E_CMD_STOP_CALLING:
		if(tlv->len != E_CMD_STOP_CALLING_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "STOP CALLING msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			x5_b_a_ack_send(mgt, mgt->seqnum);
			x5_b_start_call(FALSE, &mgt->room);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "STOP CALLING msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	case E_CMD_ON_LINE:
		if(tlv->len != E_CMD_ON_LINE_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "ON LINE msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "ON LINE msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
/*
	case E_CMD_CALL_RESULT:
		if(tlv->len != E_CMD_CALL_RESULT_LEN)
		{
			zlog_warn(ZLOG_APP, "CALL RESULT msg (seqnum=%d) len error", mgt->s_seqnum);
		}
		else
		{
			ret = OK;
			zlog_warn(ZLOG_APP, "CALL RESULT msg (seqnum=%d) OK", mgt->s_seqnum);
		}
		break;
*/

	case E_CMD_DOOR_TYPE:
		if(tlv->len != E_CMD_DOOR_TYPE_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "DOOR TYPE msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			x5_b_a_ack_send(mgt, mgt->seqnum);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "DOOR TYPE msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	case E_CMD_KEY:
		if(tlv->len != E_CMD_KEY_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "KEY msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			mgt->keyval.keyval[mgt->keyval.keynum++] = tlv->val.val32;
			x5_b_a_ack_send(mgt, mgt->seqnum);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "KEY msg (seqnum=%d) OK  keyval=%x", mgt->seqnum, tlv->val.val32);
		}
		break;
	case E_CMD_FACTORY_MODE:
		if(tlv->len != E_CMD_FACTORY_MODE_LEN)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "FACTORY MODE msg (seqnum=%d) len error", mgt->seqnum);
		}
		else
		{
			memcpy(&mgt->fact, tlv->val.pval, tlv->len);
			ret = x5_b_x5_b_factory_set(&mgt->fact);
			x5_b_a_ack_send(mgt, mgt->seqnum);
			ret = OK;
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "FACTORY MODE msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	default:
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = %d (seqnum=%d)", tlv->tag, mgt->seqnum);
		break;
	}
	return ret;
}


static int x5_b_a_read_handle(x5_b_a_mgt_t *mgt)
{
	int len = 0, offset = 0;
	u_int16 crc1 = 0;
	u_int16 *crc = NULL;
	os_tlv_t tlv;
	x5_b_a_hdr_t *hdr = mgt->buf;
	x5_b_a_statistics(mgt, 0);
	if(ntohl(hdr->total_len) + 8 != mgt->len)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TOTAL len is not same to msg len(%d != %d)", ntohl(hdr->total_len) + 2, mgt->len);
		return ERROR;
	}
	crc1 =  Data_CRC16Check (mgt->buf,  mgt->len - 2);
	crc = (u_int16 *)&mgt->buf[mgt->len - 2];
	if(*crc != htons(crc1))
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "CRC CHECK (%d != %d)", crc, crc1);
		return ERROR;
	}
	len = ntohl(hdr->total_len) - 4;
	offset += sizeof(x5_b_a_hdr_t);
	mgt->seqnum = hdr->seqnum;
	while(len > 0)
	{
		//zlog_warn(ZLOG_APP, "os_tlv_get");
		offset += os_tlv_get(mgt->buf + offset, &tlv);
		x5_b_a_read_tlv_handle(mgt, &tlv);
		len -= offset;
	}
	return OK;
}

static int x5_b_a_reset_eloop(struct eloop *eloop)
{
	x5_b_a_mgt_t *mgt = ELOOP_ARG(eloop);
	mgt->reset_thread = NULL;
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RESET mgt socket OK");
	x5_b_a_socket_exit(mgt);
	x5_b_a_socket_init(mgt);
	mgt->state = 0;
	return OK;
}

static int x5_b_a_timer_eloop(struct eloop *eloop)
{
	x5_b_a_mgt_t *mgt = ELOOP_ARG(eloop);
	mgt->t_thread = NULL;
	if(mgt->state)
		mgt->state--;
	//zlog_debug(ZLOG_APP, "x5_b_a_timer_eloop OK");
	//if(mgt->state > 0)
	{
		x5_b_a_hdr_make(mgt);
		x5_b_a_keepalive_make(mgt, 0);
		x5_b_a_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "KEEPALIVE MSG to %s:%d %d byte", mgt->remote_address,
						mgt->remote_port, mgt->slen);
		x5_b_a_send_msg(mgt);
	}
	mgt->t_thread = eloop_add_timer(mgt->master, x5_b_a_timer_eloop, mgt, mgt->interval);
	return OK;
}


static int x5_b_a_read_eloop(struct eloop *eloop)
{
	x5_b_a_mgt_t *mgt = ELOOP_ARG(eloop);
	int sock = ELOOP_FD(eloop);
	//ELOOP_VAL(X)
	mgt->r_thread = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RECV mgt on socket");
	int len = read(sock, mgt->buf, sizeof(mgt->buf));
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				//return 0;
				mgt->reset_thread = eloop_add_timer_msec(mgt->master, x5_b_a_reset_eloop, mgt, 100);
				return OK;
			}
		}
	}
	else
	{
		mgt->len = len;
		if(X5_B_ESP32_DEBUG(RECV))
		{
			zlog_debug(ZLOG_APP, "MSG from %s:%d %d byte", mgt->remote_address,
					mgt->remote_port, mgt->len);
			if(X5_B_ESP32_DEBUG(HEX))
				x5_b_a_hex_debug(mgt, "RECV", 1);
		}
		x5_b_a_read_handle(mgt);
	}
	mgt->r_thread = eloop_add_read(mgt->master, x5_b_a_read_eloop, mgt, sock);
	return OK;
}



static int x5_b_a_mgt_task(void *argv)
{
	module_setup_task(MODULE_APP_START, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}

	eloop_start_running(master_eloop[MODULE_APP_START], MODULE_APP_START);
	return OK;
}


static int x5_b_a_task_init (x5_b_a_mgt_t *mgt)
{
	if(master_eloop[MODULE_APP_START] == NULL)
		master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);
	//master_thread[MODULE_TELNET] = thread_master_module_create(MODULE_TELNET);
	mgt->task_id = os_task_create("appmgtTask", OS_TASK_DEFAULT_PRIORITY,
	               0, x5_b_a_mgt_task, mgt, OS_TASK_DEFAULT_STACK);
	if(mgt->task_id)
		return OK;
	return ERROR;
}



static int x5_b_a_socket_init(x5_b_a_mgt_t *mgt)
{
	int fd = sock_create(FALSE);
	if(fd)
	{
		if(mgt->local_port == 0)
			mgt->local_port = X5_B_A_PORT_DEFAULT;
		if(mgt->remote_port == 0)
			mgt->remote_port = X5_B_A_PORT_DEFAULT;
		if(mgt->remote_address == NULL)
		{
			close(fd);
			zlog_err(ZLOG_APP, " X5-B-A module remote address is not setting");
			return ERROR;
		}
		//zlog_debug(ZLOG_APP, "sock_bind %s:%d", mgt->local_address ? mgt->local_address:"any", mgt->local_port);
		if(sock_bind(fd, mgt->local_address, mgt->local_port) == OK)
		{
			//zlog_debug(ZLOG_APP, "sock_connect %s:%d", mgt->remote_address, mgt->remote_port);
/*			if(sock_connect(fd, mgt->remote_address, mgt->remote_port)!= OK)
			{
				close(fd);
				return ERROR;
			}*/
			mgt->r_fd = fd;
			mgt->w_fd = fd;
			mgt->state = 0;
			mgt->r_thread = eloop_add_read(mgt->master, x5_b_a_read_eloop, mgt, fd);
			mgt->t_thread = eloop_add_timer(mgt->master, x5_b_a_timer_eloop, mgt, mgt->interval + 5);
			return OK;
		}
	}
	return ERROR;
}

static int x5_b_a_socket_exit(x5_b_a_mgt_t *mgt)
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
	if(mgt && mgt->w_thread)
	{
		eloop_cancel(mgt->w_thread);
		mgt->w_thread = NULL;
	}
	if(mgt)
	{
		if(mgt->r_fd)
			close(mgt->r_fd);
		memset(mgt->buf, 0, sizeof(mgt->buf));
		mgt->r_fd = 0;
		mgt->w_fd = 0;
		mgt->state = 0;
		mgt->statistics.tx_packet = 0;
		mgt->statistics.rx_packet = 0;
		return OK;
	}
	return ERROR;
}



int x5_b_a_address_set_api(char *remote)
{
	if(!x5_b_a_mgt)
		return ERROR;
	if(x5_b_a_mgt->remote_address)
		free(x5_b_a_mgt->remote_address);
	if(remote)
		x5_b_a_mgt->remote_address = strdup(remote);
	if(x5_b_a_mgt->reset_thread)
	{
		eloop_cancel(x5_b_a_mgt->reset_thread);
		x5_b_a_mgt->reset_thread = NULL;
	}
	x5_b_a_mgt->reset_thread = eloop_add_timer_msec(x5_b_a_mgt->master, x5_b_a_reset_eloop, x5_b_a_mgt, 100);
	return OK;
}

int x5_b_a_local_address_set_api(char *address)
{
	if(!x5_b_a_mgt)
		return ERROR;
	if(x5_b_a_mgt->local_address)
		free(x5_b_a_mgt->local_address);
	if(address)
		x5_b_a_mgt->local_address = strdup(address);
	if(x5_b_a_mgt->reset_thread)
	{
		eloop_cancel(x5_b_a_mgt->reset_thread);
		x5_b_a_mgt->reset_thread = NULL;
	}
	x5_b_a_mgt->reset_thread = eloop_add_timer_msec(x5_b_a_mgt->master, x5_b_a_reset_eloop, x5_b_a_mgt, 100);
	return OK;
}

int x5_b_a_port_set_api(u_int16 port)
{
	if(!x5_b_a_mgt)
		return ERROR;
	x5_b_a_mgt->remote_port = port ? port : X5_B_A_PORT_DEFAULT;
	if(x5_b_a_mgt->reset_thread)
	{
		eloop_cancel(x5_b_a_mgt->reset_thread);
		x5_b_a_mgt->reset_thread = NULL;
	}
	x5_b_a_mgt->reset_thread = eloop_add_timer_msec(x5_b_a_mgt->master, x5_b_a_reset_eloop, x5_b_a_mgt, 100);
	return OK;
}

int x5_b_a_local_port_set_api(u_int16 port)
{
	if(!x5_b_a_mgt)
		return ERROR;
	x5_b_a_mgt->local_port = port ? port : X5_B_A_PORT_DEFAULT;
	if(x5_b_a_mgt->reset_thread)
	{
		eloop_cancel(x5_b_a_mgt->reset_thread);
		x5_b_a_mgt->reset_thread = NULL;
	}
	x5_b_a_mgt->reset_thread = eloop_add_timer_msec(x5_b_a_mgt->master, x5_b_a_reset_eloop, x5_b_a_mgt, 100);
	return OK;
}

void * x5_b_a_app_tmp()
{
	return x5_b_a_mgt;
}

int x5_b_a_app_free()
{
	x5_b_a_module_exit();
	x5_b_a_mgt->t_thread = eloop_add_timer(x5_b_a_mgt->master, x5_b_a_timer_eloop, x5_b_a_mgt, 60);
	return OK;
}

int x5_b_a_module_init(char *remote, u_int16 port)
{
	if(x5_b_a_mgt == NULL)
	{
		x5_b_a_mgt = malloc(sizeof(x5_b_a_mgt_t));
		memset(x5_b_a_mgt, 0, sizeof(x5_b_a_mgt_t));

		if(master_eloop[MODULE_APP_START] == NULL)
			master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

		x5_b_a_mgt->master = master_eloop[MODULE_APP_START];

		x5_b_a_mgt->local_address = NULL;
		x5_b_a_mgt->local_port = X5_B_A_PORT_DEFAULT;
		if(remote)
			x5_b_a_mgt->remote_address = strdup(remote);
		else
			x5_b_a_mgt->remote_address = strdup("171.167.189.2");
		x5_b_a_mgt->remote_port = port ? port : X5_B_A_PORT_DEFAULT;

		x5_b_a_mgt->interval = X5_B_A_INTERVAL_DEFAULT;
		voip_estate_mgt_init();
	}
	//x5_b_a_socket_init(x5_b_a_mgt);
	return OK;
}


int x5_b_a_module_exit()
{
	if(x5_b_a_mgt)
	{
		x5_b_a_socket_exit(x5_b_a_mgt);
		if(x5_b_a_mgt->local_address)
			free(x5_b_a_mgt->local_address);
		if(x5_b_a_mgt->remote_address)
			free(x5_b_a_mgt->remote_address);
		voip_estate_mgt_exit();
	}
	return OK;
}


int x5_b_a_module_task_init()
{
	if(x5_b_a_mgt != NULL)
	{
		x5_b_a_task_init(x5_b_a_mgt);
		x5_b_a_socket_init(x5_b_a_mgt);
		voip_estate_mgt_start();
	}
	return OK;
}


int x5_b_a_module_task_exit()
{
	if(x5_b_a_mgt)
		x5_b_a_socket_exit(x5_b_a_mgt);
	voip_estate_mgt_stop();
	return OK;
}

int x5_b_a_show_config(struct vty *vty)
{
	x5_b_a_mgt_t *mgt = x5_b_a_mgt;
	if(mgt)
	{
		if(mgt->local_address && mgt->local_port && mgt->local_port != X5_B_A_PORT_DEFAULT)
		{
			vty_out(vty, " ip esp local address %s port %d %s", mgt->local_address, mgt->local_port, VTY_NEWLINE);
		}
		else if(mgt->local_address)
			vty_out(vty, " ip esp local address %s%s", mgt->local_address, VTY_NEWLINE);
		else if(mgt->local_port && mgt->local_port != X5_B_A_PORT_DEFAULT)
			vty_out(vty, " ip esp local port %d%s", mgt->local_port, VTY_NEWLINE);

		if(mgt->remote_address && mgt->remote_port && mgt->remote_port != X5_B_A_PORT_DEFAULT)
		{
			vty_out(vty, " ip esp address %s port %d %s", mgt->remote_address, mgt->remote_port, VTY_NEWLINE);
		}
		else if(mgt->remote_address)
			vty_out(vty, " ip esp address %s%s", mgt->remote_address, VTY_NEWLINE);

		if(mgt->interval != X5_B_A_INTERVAL_DEFAULT)
			vty_out(vty, " x5-b-a keepalive interval %d %s", mgt->interval, VTY_NEWLINE);
	}
	return OK;
}

int x5_b_a_show_debug(struct vty *vty)
{
	x5_b_a_mgt_t *mgt = x5_b_a_mgt;
	if(mgt)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			vty_out(vty, " debug esp event %s", VTY_NEWLINE);

		if(X5_B_ESP32_DEBUG(SEND))
			vty_out(vty, " debug esp send %s", VTY_NEWLINE);
		if(X5_B_ESP32_DEBUG(RECV))
			vty_out(vty, " debug esp recv %s", VTY_NEWLINE);

		if(X5_B_ESP32_DEBUG(HEX))
			vty_out(vty, " debug esp hex %s", VTY_NEWLINE);
	}
	return OK;
}

int x5_b_a_show_state(struct vty *vty)
{
	x5_b_a_mgt_t *mgt = x5_b_a_mgt;
	if(mgt)
	{
		vty_out(vty, " local address           : %s %s", mgt->local_address ? mgt->local_address:"any", VTY_NEWLINE);
		vty_out(vty, " remote address          : %s %s", mgt->remote_address ? mgt->remote_address:"any", VTY_NEWLINE);
		vty_out(vty, " local port local        : %d %s", mgt->local_port, VTY_NEWLINE);
		vty_out(vty, " remote port             : %d %s", mgt->remote_port, VTY_NEWLINE);
		vty_out(vty, " keepalive state         : %d %s", mgt->state, VTY_NEWLINE);
		vty_out(vty, " keepalive interval      : %d %s", mgt->interval, VTY_NEWLINE);
		vty_out(vty, " send seqnum             : %d %s", mgt->s_seqnum, VTY_NEWLINE);
		vty_out(vty, " recv seqnum             : %d %s", mgt->seqnum, VTY_NEWLINE);

		vty_out(vty, " send packet             : %d %s", mgt->statistics.rx_packet, VTY_NEWLINE);
		vty_out(vty, " recv packet             : %d %s", mgt->statistics.tx_packet, VTY_NEWLINE);
	}
	return OK;
}


#ifdef X5_B_A_DEBUG

int call_recv_test()
{
	u_int8 buf[] = {0x7e,0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
			0x04,0x32,0x35,0x32,0x35,0x4d,0x54};
	if(x5_b_a_mgt)
	{
		x5_b_a_mgt->len = sizeof(buf);
		memcpy(x5_b_a_mgt->buf, buf, sizeof(buf));
		x5_b_a_read_handle(x5_b_a_mgt);
	}
	return OK;
}
int call_result_test(int res)
{
	if(x5_b_a_mgt)
		x5_b_a_call_result_api(x5_b_a_mgt, res);
	return OK;
}
int open_result_test(int res)
{
	if(x5_b_a_mgt)
		x5_b_a_open_result_api(x5_b_a_mgt, res);
	return OK;
}
#endif

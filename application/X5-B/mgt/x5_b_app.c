/*
 * x5b_app.c
 *
 *  Created on: 2019年3月22日
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
#include "x5_b_json.h"
#include "x5_b_ctl.h"
#include "x5b_dbase.h"
#include "web_x5b.h"
#include "x5_b_util.h"
#ifdef PL_PJSIP_MODULE
#include "pjsip_app_api.h"
#endif
x5b_app_mgt_t *x5b_app_mgt = NULL;

static int x5b_app_report_start(x5b_app_mgt_t *mgt, int interval);


int x5b_app_hdr_make(x5b_app_mgt_t *mgt)
{
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	x5b_app_hdr_t *hdr = (x5b_app_hdr_t *)mgt->app->sbuf;
	memset(mgt->app->sbuf, 0, sizeof(mgt->app->sbuf));
	hdr->makr = X5B_APP_HDR_MAKR;
	hdr->total_len = 0;//;
	hdr->seqnum = (mgt->app->seqnum);
	hdr->m_seqnum = ~hdr->seqnum;
	mgt->app->offset = sizeof(x5b_app_hdr_t);
	return OK;
}

static int x5b_app_crc_setup(char *buf, u_int16 crc)
{
	u_int16 *scrc = (u_int16 *)buf;
	*scrc = htons(crc);
	return OK;
}

int x5b_app_crc_make(x5b_app_mgt_t *mgt)
{
	u_int16	crc1 = 0;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	x5b_app_hdr_t *hdr = (x5b_app_hdr_t *)mgt->app->sbuf;
	mgt->app->slen = mgt->app->offset;
	hdr->total_len = htonl(mgt->app->slen - sizeof(x5b_app_hdr_t));
	/*
	 * 所有报文以前计算校验和
	 */
	crc1 =  Data_CRC16Check (mgt->app->sbuf,  mgt->app->slen);
	x5b_app_crc_setup(mgt->app->sbuf + mgt->app->slen, crc1);
	mgt->app->slen += 2;

	return mgt->app->slen;
}

static int x5b_app_ack_make(x5b_app_mgt_t *mgt, u_int8 seqnum)
{
	u_int8 val = (u_int8)seqnum;
	zassert(mgt != NULL);
	zassert(mgt->app != NULL);
	int len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_ACK), E_CMD_ACK_LEN, &val);
	mgt->app->offset += len;
	return (len);
}

int x5b_app_make_update(x5b_app_mgt_t *mgt, int to)
{
	zassert(mgt != NULL);
	if(to == E_CMD_TO_AUTO)
	{
		if(mgt->X5CM)
		{
			mgt->app = &mgt->app_c;
		}
		else
		{
			mgt->app = &mgt->app_a;
		}
	}
	else if(to == E_CMD_TO_A)
	{
		mgt->app = &mgt->app_a;
	}
	else if(to == E_CMD_TO_B)
	{

	}
	else if(to == E_CMD_TO_C)
	{
		mgt->app = &mgt->app_c;
	}
	zassert(mgt->app != NULL);
	mgt->regsync = FALSE;
	if(mgt->X5CM)
	{
		if(mgt->app_c.reg_state && mgt->app_a.reg_state)
			mgt->regsync = TRUE;
		else
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			{
				if(!mgt->app_a.reg_state && !mgt->app_c.reg_state)
				{
					zlog_warn(ZLOG_APP, "A/C module is Not Register");
				}
				else if(mgt->app_a.reg_state && !mgt->app_c.reg_state)
				{
					zlog_warn(ZLOG_APP, "C module is Not Register");
				}
				else if(!mgt->app_a.reg_state && mgt->app_c.reg_state)
				{
					zlog_warn(ZLOG_APP, "A module is Not Register");
				}
			}
		}
	}
	else
	{
		if(mgt->app_a.reg_state)
			mgt->regsync = TRUE;
		else
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			{
				zlog_warn(ZLOG_APP, "A module is Not Register");
			}
		}
	}
	return OK;
}



/*
 * Base CMD
 */
static int x5b_app_ack_api(x5b_app_mgt_t *app, u_int8 seqnum, int to)
{
	int ret = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "ACK MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		return ERROR;
	}*/

	x5b_app_hdr_make(mgt);
	x5b_app_ack_make(mgt, seqnum);
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "ACK MSG to %s:%d %d byte(seqnum=%d)", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen, seqnum);
	//ret = x5b_app_send_msg(mgt);
	ret = x5b_app_send_msg_without_ack(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ret;
}

/* send open door CMD by '#' signal */
int x5b_app_open_door_api(x5b_app_mgt_t *app, int res, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_OPEN), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "OPEN CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/* register ACK */
static int x5b_app_register_ok_api(x5b_app_mgt_t *mgt, int to, int havepayload)
{
	u_int8 val[16] = {1, 2, 3, 4, 5, 6, 7, 8};
	int len = 0;
	zassert(mgt != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	//x5b_app_local_mac_address_get(val);

	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Register OK CMD MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);

	len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_REGISTER_OK), E_CMD_REGISTER_OK_LEN, val);
	mgt->app->offset += len;
#ifndef X5B_APP_STATIC_IP_ENABLE
	if(havepayload)
	{
		u_int32 addr = 0;
		if(mgt->X5CM)
		{
			if(to == E_CMD_TO_A && mgt->app_c.address)
			{
				addr = mgt->app_c.address;
			}
			else if(to == E_CMD_TO_C && mgt->app_a.address)
			{
				addr = mgt->app_a.address;
			}
		}
		else
		{
			if(to == E_CMD_TO_A && mgt->app_a.address)
			{
				addr = mgt->app_a.address;
				x5b_app_local_address_get(&addr);
			}
		}
/*		if(addr <= 0)
			addr=inet_addr("192.168.2.1");*/
		if(addr != 0)
		{
			len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_IP_ADDRESS), E_CMD_IP_ADDRESS_LEN, &addr);
			mgt->app->offset += len;
		}
	}
#endif
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Register ACK MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}


int x5b_app_keepalive_send(x5b_app_mgt_t *mgt, int res, int to)
{
	int len = 0, zone = 0;
	//int timesp = 0;
	static int aaaa = 0;
	zassert(mgt != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}

	if(aaaa < 5)
	{
		aaaa++;
		zlog_debug(ZLOG_APP, "-------------keepalive---------to %s", inet_address(mgt->app->address));
	}

	x5b_app_hdr_make(mgt);

	if(to == E_CMD_TO_C || to == E_CMD_TO_A)
	{
		u_int8 tibuf[5];
		int *tisp = (int *)(tibuf + 1);
		*tisp = htonl(os_time(NULL));

		zone = x5b_app_timezone_offset_api(NULL);
		if(zone == 0)
		{
			tibuf[0] = (zone);
			*tisp = htonl(os_time(NULL)/* - OS_SEC_HOU_V(abs(zone)) */);
		}
		else if(zone < 0)
		{
			tibuf[0] = abs(zone);
			*tisp = htonl(os_time(NULL)/* - OS_SEC_HOU_V(abs(zone)) */);
		}
		else
		{
			tibuf[0] = 12 + zone;
			*tisp = htonl(os_time(NULL)/* + OS_SEC_HOU_V((zone))*/ );
		}
		if((aaaa  >= 100)||(aaaa  < 5))
		{
			int *timesp = tisp;
			zlog_debug(ZLOG_APP, "keepalive timezone=%d system time :%d(%s) to other module",
					   tibuf[0], ntohl(*timesp), os_time_fmt("/", ntohl(*timesp)));
			aaaa = 6;
		}
		aaaa++;
		//0(UTC或者Etc/GMT) 东 1-12(Etc/GMT-%d) 西13-24(Etc/GMT+%d)
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_KEEPALIVE), 5, tibuf);

	}
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
/*	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Keepalive MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);*/
	//mgt->not_debug = TRUE;
	//len = x5b_app_send_msg(mgt);
	len = x5b_app_send_msg_without_ack(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}




/*
 * Request RTC
 */
int x5b_app_rtc_request(x5b_app_mgt_t *app, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "RTC Request MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_GET_RTC_TIME), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "RTC Request MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/*
 * respone cardid(send card id to A module)
 */
int x5b_app_cardid_respone(x5b_app_mgt_t *app, u_int8 *cardid, int clen, int to)
{
	int len = 0;
	open_cardid_respone respone;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(cardid != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "CardID Respone MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	memset(&respone, 0, sizeof(open_cardid_respone));
	if(cardid)
	{
		memcpy(respone.ID, cardid, clen);
		respone.clen = clen;
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_CARD_RESPONE),
				sizeof(open_cardid_respone), &respone);
	}
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CardID Respone MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

int x5b_app_version_request(x5b_app_mgt_t *app, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "VER Request MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REQ_VERSION), 0);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "VER Request MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}


int x5b_app_wiggins_setting(x5b_app_mgt_t *app, int wiggins, int to)
{
	int len = 0;
	u_int8 val = wiggins & 0xff;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "wiggins MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_WIGGINS), 1, &val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Wiggins MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

//向A/C模块通告IP地址
int x5b_app_IP_address_api(x5b_app_mgt_t *app, u_int32 address, int to)
{
	u_int32 val = (u_int32)address;
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "IP Address MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_integer(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_IP_ADDRESS), sizeof(u_int32), &val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "IP Address MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/*
 * Call Result CMD
 */
int x5b_app_call_result_api(x5b_app_mgt_t *app, int res, int inde, int to)
{
	u_int8 val[2];
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "CALL Result MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	x5b_app_hdr_make(mgt);
	val[0] = (res & 0xff);
	val[1] = (inde & 0xff);
	len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_CALL, E_CMD_CALL_RESULT), E_CMD_CALL_RESULT_LEN, val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "CALL Result MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/*
 * network port link status change report
 */
int x5b_app_network_port_status_api(x5b_app_mgt_t *app, int res, int to)
{
	u_int8 val = res & 0xff;
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Network Port Status MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_NETSTATUS), 1, &val);
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Network Port Status MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/* network info request ack */
static int x5b_app_network_information_ack(x5b_app_mgt_t *mgt, int to)
{
	x5b_app_netinfo_t netinfo;
	int len = 0;
	zassert(mgt != NULL);
	//zassert(mgt->app != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	memset(&netinfo, 0, sizeof(x5b_app_netinfo_t));
	if(x5b_app_local_network_info_get(&netinfo) == OK)
	{
		x5b_app_make_update(mgt, to);
		if(!mgt->app->reg_state)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
/*			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);*/
			return ERROR;
		}
		if(mgt->app->address == 0)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Network Info MSG Can not send, Unknown Remote IP Address");
/*			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);*/
			return ERROR;
		}
/*		if(!mgt->regsync)
		{
			return ERROR;
		}*/
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_ACK_STATUS), sizeof(netinfo), &netinfo);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Network Info MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return len;
	}
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ERROR;
}

/* phone register status change report */
int x5b_app_register_status_api(x5b_app_mgt_t *app, int res, int to)
{
#ifdef PL_OPENWRT_UCI
	x5b_app_register_ack_t state;
	int len = 0,rlen = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	memset(&state, 0, sizeof(state));
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Register Status MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
#ifdef PL_OSIP_MODULE
	if(voip_sip_multiuser_get_api())
	{
		voip_sip_local_number_get_api(state.phone, FALSE);
		rlen = (1 + strlen(state.phone));
		state.reg_state = voip_sip_main_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;

		voip_sip_local_number_get_api(state.phone, TRUE);
		rlen = (1 + strlen(state.phone));
		state.reg_state = voip_sip_stanby_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
	else
	{
		voip_sip_local_number_get_api(state.phone, FALSE);
		rlen = (1 + strlen(state.phone));
		state.reg_state = voip_sip_main_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
#endif
#ifdef PL_PJSIP_MODULE
	if(pl_pjsip_multiuser_get_api())
	{
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;
		//state.reg_state = voip_sip_main_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;

		pl_pjsip_username_get_api(state.phone, NULL, TRUE);
		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
	else
	{
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
#endif
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Register Status MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
#else
	return OK;
#endif
}

/* sip info request ack */
static int x5b_app_register_information_ack(x5b_app_mgt_t *mgt, int to)
{
	x5b_app_phone_register_ack_t reginfo;
	int len = 0;
	zassert(mgt != NULL);
	memset(&reginfo, 0, sizeof(x5b_app_phone_register_ack_t));
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Register Info MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		return ERROR;
	}*/
	if(x5b_app_local_register_info_get(&reginfo) == OK)
	{
		x5b_app_hdr_make(mgt);
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_ACK_REGISTER),
				sizeof(x5b_app_phone_register_ack_t), &reginfo);
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Register Info MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return len;
	}
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return ERROR;
}


/* A 模块开门参数设置 */
int x5b_app_open_option(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	//ConfiglockType *card = (ConfiglockType *)info;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		return ERROR;
	}
	if(info)
	{
		x5b_app_hdr_make(mgt);
		if(to == E_CMD_TO_A)
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_OPEN_OPTION), sizeof(ConfiglockType), info);
		else if(to == E_CMD_TO_C)
		{
			u_int8 open_type = mgt->opentype;
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_OPEN_OPTION), sizeof(u_int8), &open_type);
		}
		mgt->app->offset += len;
		x5b_app_crc_make(mgt);
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "OPEN OPTION MSG to %s:%d %d byte", inet_address(mgt->app->address),
					mgt->app->remote_port, mgt->app->slen);
		len = x5b_app_send_msg(mgt);
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return len;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return ERROR;
}

/**********************************************/
int x5b_app_reboot_request(x5b_app_mgt_t *app, int to, BOOL hwreset)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Reboot/Reset Request MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	if(hwreset == TRUE)
	{
		len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_RESET_REQ), 0);

		zlog_warn(ZLOG_APP, "Reset TLV :%x", E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_RESET_REQ));
	}
	else
	{
		len = os_tlv_set_zero(mgt->app->sbuf + mgt->app->offset,
			E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_REBOOT_REQ), 0);

		zlog_warn(ZLOG_APP, "Reboot TLV :%x", E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_REBOOT_REQ));
	}
	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Reboot/Reset Request MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

/**********************************************/


/*
 *A 模块制卡  make card
 */
int x5b_app_add_card(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	permiListType *card  = (permiListType *)info;
	x5b_app_mgt_t *mgt = app;
	//make_face_card_t *inputcard = (make_face_card_t *)info;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
/*
	memset(&card, 0, sizeof(card));

	if(strlen(inputcard->cardid))
	{
		card_id_string_to_hex(inputcard->cardid, strlen(inputcard->cardid), card.ID);
	}
	card.start_time = htonl(inputcard->start_date);
	card.stop_time = htonl(inputcard->stop_date);

	if(strlen(inputcard->cardtype))
	{
		if(strstr(inputcard->cardtype,"Blacklist"))
			card.status = 1;
		else if(strstr(inputcard->cardtype,"Whitelist"))
			card.status = 2;
	}
	zlog_debug(ZLOG_APP, "===================%s -> type=%d", __func__, card.status);
*/

	x5b_app_hdr_make(mgt);
	if(to == E_CMD_TO_A)
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_MAKE_CARD), sizeof(permiListType), card);

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MAKE Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}

int x5b_app_delete_card(x5b_app_mgt_t *app, void *info, int to)
{
	int len = 0;
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	zassert(info != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "OPEN CMD MSG Can not send, Unknown Remote IP Address");
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}
	x5b_app_hdr_make(mgt);

	if(to == E_CMD_TO_A)
	{
		s_int8     cardid[APP_CARD_ID_MAX + 1];
		u_int8     ID[8];
		memset(cardid, 0, sizeof(cardid));
		memcpy(cardid, info, APP_CARD_ID_MAX);
		if(strlen(cardid) <= 16)
		{
			card_id_string_to_hex(cardid, strlen(cardid), ID);
		}
		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_SET, E_CMD_DELETE_CARD), sizeof(ID), ID);
	}

	mgt->app->offset += len;
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Delete Card CMD MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return len;
}


/*************************************************************************/
/*
 * recv register CMD
 */
static int x5b_app_read_register_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);

	if(E_CMD_FROM_A(tlv->tag))//from A module
	{
		u_int32	*rtc = NULL;
		if(tlv->len != 8)
		{
			return OK;
		}
		mgt->app_a.reg_state = TRUE;
		//mgt->app_a.remote_address = strdup(inet_address(ntohl(mgt->from.sin_addr.s_addr)));
#ifndef X5B_APP_STATIC_IP_ENABLE
		if(mgt->app_a.remote_port == 0)
			mgt->app_a.remote_port = ntohs(mgt->from.sin_port);

		if(mgt->app_a.address == 0)
			x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_A);
		mgt->app_a.address = ntohl(mgt->from.sin_addr.s_addr);
#else
		x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_A);
#endif
		rtc = (u_int32 *)tlv->val.pval;
		mgt->app_a.id = ntohl(*rtc);//set Module ID
//		zlog_debug(ZLOG_APP, "======================== id=%x", mgt->app_a.id);
		rtc = (u_int32 *)(tlv->val.pval + sizeof(u_int32));

		//zlog_debug(ZLOG_APP, "===========get stm32 times:->%d", ntohl(*rtc));
		//if(mgt->time_sync == FALSE)
		{
			if(x5b_app_rtc_tm_set(ntohl(*rtc)) != OK)//set local time by RTC Time
			{
				x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);//TVL ack
				mgt->app_a.reg_state = FALSE;
				mgt->app_a.id = 0;
				return OK;
			}
			mgt->time_sync = TRUE;
		}

		x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);//TVL ack
		if(mgt->app_a.t_thread)
		{
			eloop_cancel(mgt->app_a.t_thread);
			mgt->app_a.t_thread = NULL;
		}

		x5b_app_event_active(mgt, X5B_TIMER_EV, X5B_APP_MODULE_ID_A, 0);
		//mgt->app_a.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_a, mgt->app_a.interval);

		//if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Register ID:%x", mgt->app_a.id);

		if(mgt->X5CM)
		{
			if(mgt->app_c.reg_state)//when C module is register OK, send register OK result
			{
				zlog_debug(ZLOG_APP, "======================== X5CM");
				//if(mgt->app_a.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
					//mgt->app_a.msg_sync = TRUE;
					//zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
				}
				//if(mgt->app_c.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
					//mgt->app_c.msg_sync = TRUE;
					//zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				}
/*				if(mgt->report_event)
					x5b_app_report_start(mgt, 2);*/
			}
		}
		else
		{
			zlog_debug(ZLOG_APP, "======================== X5BM");
			//if(mgt->app_a.msg_sync == FALSE)
			{
				//zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
				x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
				//mgt->app_a.msg_sync = TRUE;
			}
		}
		if(mgt->app_a.version == NULL)
		{
			os_msleep(500);
			zlog_debug(ZLOG_APP, "version request to: %x", mgt->app_a.id);
			x5b_app_version_request(mgt, E_CMD_TO_A);
		}
/*		if(mgt->report_event)
			x5b_app_report_start(mgt, 2);*/
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		x5b_app_wiggins_setting(mgt, mgt->app_a.wiggins, E_CMD_TO_A);
		if(mgt->mutex)
			os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
		//x5b_app_AC_state_save(mgt);
	}
	else if(E_CMD_FROM_C(tlv->tag))
	{
/*
		zlog_debug(ZLOG_APP, "TLV T:%x", tlv->tag);
		zlog_debug(ZLOG_APP, "TLV L:%x", tlv->len);
		zlog_debug(ZLOG_APP, "TLV V:%x", tlv->val.val32);
*/
		mgt->app_c.reg_state = TRUE;
#ifndef X5B_APP_STATIC_IP_ENABLE
		//mgt->app_c.remote_address = strdup(inet_address(ntohl(mgt->from.sin_addr.s_addr)));
		if(mgt->app_c.remote_port == 0)
			mgt->app_c.remote_port = ntohs(mgt->from.sin_port);
		if(mgt->app_c.address == 0)
			x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_C);
		mgt->app_c.address = ntohl(mgt->from.sin_addr.s_addr);
#else
		x5b_app_statistics(mgt, 0, X5B_APP_MODULE_ID_C);
#endif
		//mgt->app_c.id = tlv->val.val32;
		os_tlv_get_integer(tlv, &mgt->app_c.id);
		mgt->app_c.id = ntohl(mgt->app_c.id);

		x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);

		if(mgt->app_c.t_thread)
		{
			eloop_cancel(mgt->app_c.t_thread);
			mgt->app_c.t_thread = NULL;
		}
		x5b_app_event_active(mgt, X5B_TIMER_EV, X5B_APP_MODULE_ID_C, 0);
		//mgt->app_c.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_c, mgt->app_c.interval);

		//zlog_debug(ZLOG_APP, "======================== id=%x", mgt->app_c.id);

		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Register ID:%x", mgt->app_c.id);

		if(mgt->X5CM)
		{
			if(mgt->app_a.reg_state)//when A module is register OK, send register OK result
			{
				//zlog_debug(ZLOG_APP, "======================== X5CM");
				//if(mgt->app_a.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
					//mgt->app_a.msg_sync = TRUE;
					//zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
				}
				//if(mgt->app_c.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
					//mgt->app_c.msg_sync = TRUE;
					//zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				}
			}
		}
		else
		{
			//zlog_debug(ZLOG_APP, "======================== X5BM");
			//if(mgt->app_c.msg_sync == FALSE)
			{
				//zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
				//mgt->app_c.msg_sync = TRUE;
			}
		}
		if(mgt->app_c.version == NULL)
		{
			os_msleep(500);
			x5b_app_version_request(mgt, E_CMD_TO_C);
		}

		if(mgt->report_event)
			x5b_app_report_start(mgt, 2);

		x5b_app_AC_state_save(mgt);
#ifdef X5B_APP_TCP_ENABLE
		x5b_app_tcp_connect_init(mgt);//connect to C module
#endif
	}
	return OK;
}

/* 房间号鉴权 应答 */
static int x5b_app_authentication_ack_api(x5b_app_mgt_t *mgt, voip_dbase_t *dbtest, int res, int to)
{
	int len = 0;
	x5b_app_room_auth_ack_t ack;
	zassert(mgt != NULL);
	zassert(dbtest != NULL);
	//zassert(tlv->val.pval != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	x5b_app_make_update(mgt, to);
	if(!mgt->app->reg_state)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Remote is Not Register");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
	if(mgt->app->address == 0)
	{
		if(X5_B_ESP32_DEBUG(EVENT))
		zlog_warn(ZLOG_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
/*		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);*/
		return ERROR;
	}
/*	if(!mgt->regsync)
	{
		if(mgt->mutex)
			os_mutex_unlock(mgt->mutex);
		return ERROR;
	}*/
	x5b_app_hdr_make(mgt);
	if(res == 0)
	{
		ack.building = dbtest->building;
		ack.unit = dbtest->unit;
		memcpy(ack.room_number, dbtest->room_number, sizeof(dbtest->room_number));
		ack.result = res;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_ROOM_AUTH_ACK),
				sizeof(x5b_app_room_auth_t) + 1, &ack);
		mgt->app->offset += len;
	}
	else
	{
		ack.building = dbtest->building;
		ack.unit = dbtest->unit;
		//ack.room_number = htons(dbtest->room_number);
		memcpy(ack.room_number, dbtest->room_number, sizeof(dbtest->room_number));
		ack.result = res;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_BASE, E_CMD_ROOM_AUTH_ACK),
				sizeof(x5b_app_room_auth_t) + 1, &ack);
		mgt->app->offset += len;
	}
	x5b_app_crc_make(mgt);
	if(X5_B_ESP32_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "Auth ACK MSG to %s:%d %d byte", inet_address(mgt->app->address),
				mgt->app->remote_port, mgt->app->slen);
	len = x5b_app_send_msg(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
	return len;
}

/* 接收到房间号鉴权请求 */
static int x5b_app_read_authentication_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	voip_dbase_t *dbtest = NULL;
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	x5b_app_room_auth_t *room = tlv->val.pval;
	//room->room_number = atoi(room->room_number);

	dbtest = voip_dbase_lookup_by_room(room->building, room->unit, atoi(room->room_number));
	if(dbtest)
	{
		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_authentication_ack_api(mgt, dbtest, 0, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_authentication_ack_api(mgt, dbtest, 0, E_CMD_TO_C);
	}
	else
	{
		voip_dbase_t ack;
		ack.building = room->building;
		ack.unit = room->unit;
		//ack.room_number = htons(dbtest->room_number);
		memcpy(ack.room_number, room->room_number, sizeof(room->room_number));

		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_authentication_ack_api(mgt, &ack, 1, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_authentication_ack_api(mgt, &ack, 1, E_CMD_TO_C);
	}
	return OK;
}

static int x5b_app_read_base_cmd_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	if(E_CMD_TYPE_GET(tlv->tag) != E_CMD_BASE)
		return ERROR;
	switch(E_CMD_GET(tlv->tag))
	{
	case E_CMD_ACK:				//应答指令
/*		if(mgt->mutex)
			os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
/*		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "ACK msg (seqnum=%d) OK", mgt->seqnum);*/
		mgt->ack_seqnum = tlv->val.pval[0];//tlv->val.val8;
		if(mgt->sync_ack)
		{
			//mgt->sync_ack = FALSE;
			if(mgt->ack_seqnum == mgt->app->seqnum)
			{
				if(X5_B_ESP32_DEBUG(EVENT))
					zlog_debug(ZLOG_APP, "ACK msg (seqnum=%d) OK", mgt->app->seqnum);
				mgt->ack_seqnum = 0;
/*				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);*/
				return OK;
			}
			else
			{
				if(X5_B_ESP32_DEBUG(EVENT))
					zlog_debug(ZLOG_APP, "ACK msg (send seqnum=%d not same recv seqnum=%d) ERROR", mgt->seqnum, mgt->ack_seqnum);
/*				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);*/
				return ERROR;
			}
		}
		break;
	case E_CMD_REGISTER:				//开机注册 A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Register msg (seqnum=%d)", mgt->seqnum);
		x5b_app_read_register_tlv(mgt, tlv);
		break;
	case E_CMD_REGISTER_OK:				//开机注册成功 B->A/C
		break;
	case E_CMD_OPEN:				//开门信令 B->A
		break;
	case E_CMD_KEEPALIVE:					//keepalive
		if(E_CMD_FROM_A(tlv->tag))
		{
			mgt->app_a.keep_cnt++;
			//if(X5_B_ESP32_DEBUG(EVENT))
			//	zlog_debug(ZLOG_APP, "KeepAlive msg from A module");
		}
		else if(E_CMD_FROM_C(tlv->tag))
		{
			mgt->app_c.keep_cnt++;
			//if(X5_B_ESP32_DEBUG(EVENT))
			//	zlog_debug(ZLOG_APP, "KeepAlive msg from C module");
		}
		break;
	case E_CMD_ROOM_AUTH:					//authentication
		x5b_app_read_authentication_tlv(mgt, tlv);
		break;
	case E_CMD_ROOM_AUTH_ACK:					//authentication ack
		break;

	default:
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = 0x%x (seqnum=%d)", tlv->tag, mgt->seqnum);
		break;
	}
	return OK;
}


static int x5b_app_read_set_cmd_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	if(E_CMD_TYPE_GET(tlv->tag) != E_CMD_SET)
		return ERROR;
	switch(E_CMD_GET(tlv->tag))
	{
	case E_CMD_FACTORY_MODE:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Factory Mode msg (seqnum=%d)", mgt->seqnum);
/*
		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);
*/
		{
			x5b_app_factory_t data;
			memcpy(&data, tlv->val.pval, tlv->len);
			x5b_app_factory_set(&data);
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_debug(ZLOG_APP, "FACTORY MODE msg (seqnum=%d) OK", mgt->seqnum);
		}
		break;
	case E_CMD_ACK_RTC_TIME:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "RTC Time msg (seqnum=%d)", mgt->seqnum);
		{
			u_int32 *rtc_value = (u_int32 *)tlv->val.pval;
			if(x5b_app_rtc_tm_set(ntohl(rtc_value)) == OK)
			{
				mgt->time_sync = TRUE;
				mgt->app_a.reg_state = TRUE;
				mgt->app_a.id = X5B_APP_MODULE_ID_A;
			}
		}
		break;

	case E_CMD_MAKE_CARD:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "CARD Json Make Card msg (seqnum=%d)", mgt->seqnum);
		//x5b_app_req_card_tlv_json(mgt, tlv);
		break;

	case E_CMD_FACE_ACK:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Face Img info msg (seqnum=%d)", mgt->seqnum);
		//x5b_app_face_id_respone(mgt, tlv);
		break;
	case E_CMD_FACE_ACK_RES:				//
		//if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Face Img info msg (seqnum=%d)", mgt->seqnum);
		x5b_app_face_load_respone(mgt, tlv);
		break;
	case E_CMD_SIP_OPT:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "SIP info Load msg (seqnum=%d)", mgt->seqnum);
		x5b_app_sip_load_respone(mgt, tlv);
		break;

	default:
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = 0x%x (seqnum=%d)", tlv->tag, mgt->seqnum);
		break;
	}
	return OK;
}

static int x5b_app_read_call_cmd_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	if(E_CMD_TYPE_GET(tlv->tag) != E_CMD_CALL)
		return ERROR;
	switch(E_CMD_GET(tlv->tag))
	{
	case E_CMD_START_CALL:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Start Call msg (seqnum=%d)", mgt->seqnum);
/*		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);*/
		{
			x5b_app_call_from_t call_info;
			x5b_app_call_t call;
			memcpy(&call_info, tlv->val.pval, tlv->len);

			call.building = (call_info.building);
			call.unit = (call_info.unit);
			call.room_number = atoi(call_info.room_number);

			zlog_debug(ZLOG_APP, " ======== Call (room_number=%d)", call.room_number);

			if(voip_dbase_lookup_by_room(call.building, call.unit, call.room_number) == NULL)
			{
				if(mgt->mutex)
					os_mutex_unlock(mgt->mutex);
				if(E_CMD_FROM_A(tlv->tag))
					x5b_app_call_result_api(mgt, E_CALL_RESULT_NO_SEARCH, 0, E_CMD_TO_A);
				else if(E_CMD_FROM_C(tlv->tag))
					x5b_app_call_result_api(mgt, E_CALL_RESULT_NO_SEARCH, 0, E_CMD_TO_C);
				if(mgt->mutex)
					os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
			}
			else
			{
				//call.room_number = ntohs(call.room_number);
				x5b_app_start_call(TRUE, &call);
			}
		}
		break;
	case E_CMD_STOP_CALL:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Stop Call msg (seqnum=%d)", mgt->seqnum);

/*		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);*/

		{
			x5b_app_stop_call(FALSE, NULL);
		}
		break;
	case E_CMD_START_CALL_OPT:				//
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Start Call msg (seqnum=%d)", mgt->seqnum);
		x5b_app_call_respone(mgt, tlv);
		break;

	default:
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = 0x%x (seqnum=%d)", tlv->tag, mgt->seqnum);
		break;
	}
	return OK;
}

static int x5b_app_read_status_cmd_tlv(x5b_app_mgt_t *mgt, os_tlv_t *tlv)
{
	zassert(mgt != NULL);
	zassert(tlv != NULL);
	zassert(tlv->val.pval != NULL);
	if(E_CMD_TYPE_GET(tlv->tag) != E_CMD_STATUS)
		return ERROR;
	switch(E_CMD_GET(tlv->tag))
	{
	case E_CMD_REQ_STATUS:				//查询网络接口信息A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Request Network Status msg (seqnum=%d)", mgt->seqnum);
		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_network_information_ack(mgt, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_network_information_ack(mgt, E_CMD_TO_C);
		break;
	case E_CMD_REQ_REGISTER:			//查询注册信息A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Request Register Status msg (seqnum=%d)", mgt->seqnum);

		if(E_CMD_FROM_A(tlv->tag))
			x5b_app_register_information_ack(mgt, E_CMD_TO_A);
		else if(E_CMD_FROM_C(tlv->tag))
			x5b_app_register_information_ack(mgt, E_CMD_TO_C);

		break;

	case E_CMD_REQ_VERSION:			//查询注册信息A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Request Version msg (seqnum=%d)", mgt->seqnum);
		break;
	case E_CMD_ACK_VERSION:			//查询注册信息A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Recv Version info msg (seqnum=%d) ver:%s\r\n", mgt->seqnum, tlv->len? tlv->val.pval:" ");
		if(tlv->len && tlv->len < X5B_APP_TLV_DEFAULT)
		{
			char ver[512];
			memset(ver, 0, sizeof(ver));
			memcpy(ver, tlv->val.pval, MIN(tlv->len, sizeof(ver)));

			if(E_CMD_FROM_A(tlv->tag))
			{
				zlog_debug(ZLOG_APP, "Recv A module Version ver:%s\r\n", ver);
				if(mgt->app_a.version)
					free(mgt->app_a.version);
				mgt->app_a.version = strdup(ver);
				os_write_file("/tmp/app/.a-ver", mgt->app_a.version, tlv->len);
			}
			else if(E_CMD_FROM_C(tlv->tag))
			{
				zlog_debug(ZLOG_APP, "Recv C module Version ver:%s\r\n", ver);

				if(mgt->app_c.version)
					free(mgt->app_c.version);
				mgt->app_c.version = strdup(ver);
				os_write_file("/tmp/app/.c-ver", mgt->app_c.version, tlv->len);
			}
		}
		break;
	case E_CMD_OPEN_LOG:			//log信息A/C->B
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "Recv LOG info msg (seqnum=%d)", mgt->seqnum);
		if(tlv->len)
		{
			if(E_CMD_FROM_A(tlv->tag))
			{
				//x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);
				x5b_app_a_thlog_log(tlv->val.pval);
			}
#ifdef PL_VOIP_MODULE
			else if(E_CMD_FROM_C(tlv->tag))
				voip_thlog_log2("%s", tlv->val.pval);
#endif
		}
		break;
	default:
		if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "TAG HDR = 0x%x (seqnum=%d)", tlv->tag, mgt->seqnum);
		break;
	}
	return OK;
}


int x5b_app_read_handle(x5b_app_mgt_t *mgt)
{
	int len = 0, offset = 0, ack = 0;
	os_tlv_t tlv;
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
	zassert(mgt != NULL);
/*	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);*/
	len = x5b_app_read_chk_handle(mgt);
/*	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);*/
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

		if(ack == 0 &&
				(E_CMD_GET(tlv.tag) != E_CMD_REGISTER) &&
				(E_CMD_GET(tlv.tag) != E_CMD_ACK) &&
				(E_CMD_GET(tlv.tag) != E_CMD_KEEPALIVE) )
		{
			if(E_CMD_FROM_A(tlv.tag))
				x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_A);
			else if(E_CMD_FROM_C(tlv.tag))
				x5b_app_ack_api(mgt, mgt->seqnum, E_CMD_TO_C);
			ack = 1;
		}

		switch(E_CMD_TYPE_GET(tlv.tag))
		{
		case E_CMD_BASE:
			x5b_app_read_base_cmd_tlv(mgt, &tlv);
			break;
		case E_CMD_SET:
			x5b_app_read_set_cmd_tlv(mgt, &tlv);
			break;
		case E_CMD_CALL:
			x5b_app_read_call_cmd_tlv(mgt, &tlv);
			break;
		case E_CMD_STATUS:
			x5b_app_read_status_cmd_tlv(mgt, &tlv);
			break;
		default:
			if(X5_B_ESP32_DEBUG(EVENT))
				zlog_warn(ZLOG_APP, "TAG HDR = 0x%x(0x%x) (seqnum=%d)", tlv.tag, E_CMD_TYPE_GET(tlv.tag), mgt->seqnum);
			break;
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
	return OK;
}

static int x5b_app_state_load(x5b_app_mgt_t *mgt)
{
	return OK;
	zassert(mgt != NULL);
	x5b_app_AC_state_load(mgt);

	if(mgt->app_a.address && mgt->app_a.reg_state)
	{
		if(mgt->X5CM)
		{
			if(mgt->app_c.reg_state)//when C module is register OK, send register OK result
			{
				zlog_debug(ZLOG_APP, "======================== X5CM");
				//if(mgt->app_a.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
					//mgt->app_a.msg_sync = TRUE;
					zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
					os_msleep(500);
					x5b_app_rtc_request(mgt, E_CMD_TO_A);
				}
				//if(mgt->app_c.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
				//	mgt->app_c.msg_sync = TRUE;
					zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				}
			}
		}
		else
		{
			zlog_debug(ZLOG_APP, "======================== X5BM");
			//if(mgt->app_a.msg_sync == FALSE)
			{
				zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
				x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
				//mgt->app_a.msg_sync = TRUE;
				os_msleep(500);
				x5b_app_rtc_request(mgt, E_CMD_TO_A);
			}
		}

		if(mgt->app_a.t_thread)
		{
			eloop_cancel(mgt->app_a.t_thread);
			mgt->app_a.t_thread = NULL;
		}
		x5b_app_event_active(mgt, X5B_TIMER_EV, X5B_APP_MODULE_ID_A, 0);
		//mgt->app_a.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_a, mgt->app_a.interval);
	}

	if(mgt->app_c.address && mgt->app_c.reg_state)
	{
		if(mgt->X5CM)
		{
			if(mgt->app_a.reg_state)//when C module is register OK, send register OK result
			{
				zlog_debug(ZLOG_APP, "======================== X5CM");
				//if(mgt->app_a.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_A, 1);
					//mgt->app_a.msg_sync = TRUE;
					zlog_debug(ZLOG_APP, "======================== X5BM Register TO A");
					os_msleep(500);
					x5b_app_rtc_request(mgt, E_CMD_TO_A);
				}
				//if(mgt->app_c.msg_sync == FALSE)
				{
					x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
					//mgt->app_c.msg_sync = TRUE;
					zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				}
			}
		}
		else
		{
			zlog_debug(ZLOG_APP, "======================== X5BM");
			//if(mgt->app_c.msg_sync == FALSE)
			{
				zlog_debug(ZLOG_APP, "======================== X5BM Register TO C");
				x5b_app_register_ok_api(mgt, E_CMD_TO_C, 1);
				//mgt->app_c.msg_sync = TRUE;
				os_msleep(500);
				x5b_app_rtc_request(mgt, E_CMD_TO_C);
			}
		}

		if(mgt->app_c.t_thread)
		{
			eloop_cancel(mgt->app_c.t_thread);
			mgt->app_c.t_thread = NULL;
		}
		x5b_app_event_active(mgt, X5B_TIMER_EV, X5B_APP_MODULE_ID_C, 0);
		//mgt->app_c.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_c, mgt->app_c.interval);
	}
	return OK;
}

static int x5b_app_report_eloop(struct eloop *eloop)
{
	int len = 0, to_cmd = 0, send_cnt = 2;
	zassert(eloop != NULL);
	x5b_app_netinfo_t netinfo;
#ifdef PL_OPENWRT_UCI
	x5b_app_register_ack_t state;
#endif
	x5b_app_phone_register_ack_t reginfo;
	x5b_app_mgt_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);
	mgt->report_thread = NULL;
	mgt->report_event = 0;
	if(!mgt->X5CM)
		send_cnt = 1;
	while(send_cnt--)
	{
		if(mgt->X5CM && send_cnt == 1)
			to_cmd = E_CMD_TO_C;
		else
			to_cmd = E_CMD_TO_A;

		x5b_app_make_update(mgt, to_cmd);
		if(!mgt->app->reg_state)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Remote is Not Register");
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}
		if(mgt->app->address == 0)
		{
			if(X5_B_ESP32_DEBUG(EVENT))
			zlog_warn(ZLOG_APP, "Keepalive MSG Can not send, Unknown Remote IP Address");
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}
/*		if(!mgt->regsync)
		{
			if(mgt->mutex)
				os_mutex_unlock(mgt->mutex);
			return ERROR;
		}*/
		x5b_app_hdr_make(mgt);
#ifdef BUILD_OPENWRT
		if (x5b_app_network_port_status_get(mgt) == OK)
		{
			u_int8 val = 0;
			if(mgt->wan_state.address)
				val = E_CMD_NETWORK_STATE_UP;
			else
			{
				if(mgt->wan_state.link_phy == E_CMD_NETWORK_STATE_PHY_UP)
					val = E_CMD_NETWORK_STATE_PHY_UP;
				else
					val = E_CMD_NETWORK_STATE_PHY_DOWN;
			}
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_NETSTATUS), 1, &val);
			mgt->app->offset += len;
		}
#endif
#ifdef PL_OPENWRT_UCI
#ifdef PL_OSIP_MODULE
		if(voip_sip_multiuser_get_api())
		{
			int rlen = 0;
			voip_sip_local_number_get_api(&state.phone, FALSE);
			rlen = (1 + strlen(&state.phone));
			state.reg_state = voip_sip_main_regstate() & 0xff;

			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
			mgt->app->offset += len;

			voip_sip_local_number_get_api(&state.phone, TRUE);
			rlen = (1 + strlen(&state.phone));
			state.reg_state = voip_sip_stanby_regstate() & 0xff;

			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
			mgt->app->offset += len;
		}
		else
		{
			int rlen = 0;
			voip_sip_local_number_get_api(&state.phone, FALSE);
			rlen = (1 + strlen(&state.phone));
			state.reg_state = voip_sip_main_regstate() & 0xff;

			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
			mgt->app->offset += len;
		}
#endif
#ifdef PL_PJSIP_MODULE
	if(pl_pjsip_multiuser_get_api())
	{
		int rlen = 0;
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;
		//state.reg_state = voip_sip_main_regstate() & 0xff;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;

		pl_pjsip_username_get_api(state.phone, NULL, TRUE);
		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
	else
	{
		int rlen = 0;
		pl_pjsip_username_get_api(state.phone, NULL, FALSE);

		rlen = (1 + strlen(state.phone));
		if(pl_pjsip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			state.reg_state = 1;
		else
			state.reg_state = 0;

		len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
				E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_REG_STATUS), rlen, &state);
		mgt->app->offset += len;
	}
#endif
#endif

		memset(&netinfo, 0, sizeof(x5b_app_netinfo_t));
		if(x5b_app_local_network_info_get(&netinfo) == OK)
		{
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_ACK_STATUS), sizeof(netinfo), &netinfo);
			mgt->app->offset += len;
		}

		memset(&reginfo, 0, sizeof(x5b_app_phone_register_ack_t));
		if(x5b_app_local_register_info_get(&reginfo) == OK)
		{
			len = os_tlv_set_octet(mgt->app->sbuf + mgt->app->offset,
					E_CMD_MAKE(E_CMD_MODULE_B, E_CMD_STATUS, E_CMD_ACK_REGISTER),
					sizeof(x5b_app_phone_register_ack_t), &reginfo);
			mgt->app->offset += len;
		}

		x5b_app_crc_make(mgt);
		x5b_app_send_msg(mgt);
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return OK;
}

static int x5b_app_report_start(x5b_app_mgt_t *mgt, int interval)
{
	zassert(mgt != NULL);
	if(mgt && mgt->report_thread)
	{
		eloop_cancel(mgt->report_thread);
		mgt->report_thread = NULL;
		mgt->report_thread = eloop_add_timer(mgt->master, x5b_app_report_eloop, mgt, interval);
	}
	return OK;
}


static int x5b_app_update_mode_enable_a(x5b_app_mgt_t *app)
{
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;
	zassert(mgt != NULL);
	if(mgt->upgrade == TRUE)
		return OK;
	mgt->upgrade = TRUE;
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
	return OK;
}

static int x5b_app_update_mode_disable_a(x5b_app_mgt_t *app)
{
	x5b_app_mgt_t *mgt = app;
	if(app == NULL)
		mgt = x5b_app_mgt;

	zassert(mgt != NULL);

	if(mgt->upgrade != TRUE)
		return OK;
	if(mgt && !mgt->r_thread && mgt->r_fd > 0)
	{
		x5b_app_event_active(mgt, X5B_READ_EV, 0, 0);
		//mgt->r_thread = eloop_add_read(mgt->master, x5b_app_read_eloop, mgt, mgt->r_fd);
	}
	if(mgt->app_c.reg_state)
		x5b_app_event_active(mgt, X5B_TIMER_EV, X5B_APP_MODULE_ID_C, 0);
		//mgt->app_c.t_thread = eloop_add_timer(mgt->master, x5b_app_timer_eloop, &mgt->app_c, mgt->app_c.interval);

	//mgt->sync_ack = FALSE;
	mgt->app_a.id = 0;
	mgt->app_a.reg_state = FALSE;
	//mgt->app_a.state = 0;			//>= 3: OK, else ERROR;
	mgt->app_a.keep_cnt = X5B_APP_INTERVAL_CNT_DEFAULT;
#ifndef X5B_APP_STATIC_IP_ENABLE
	mgt->app_a.address = 0;
#endif
	mgt->upgrade = FALSE;
	//mgt->app_a.remote_port = 0;
	zlog_debug(ZLOG_APP, "----x5b_app_update_mode_disable_a OK" );
	return OK;
}

int x5b_app_update_mode_enable(x5b_app_mgt_t *app, BOOL enable, int to)
{
	if(to == E_CMD_TO_A)
	{
		if(enable)
			return x5b_app_update_mode_enable_a(app);
		else
			return x5b_app_update_mode_disable_a(app);
	}
	return ERROR;
}


int x5b_app_local_address_set_api(char *address)
{
	if(!x5b_app_mgt)
		return ERROR;
	if(x5b_app_mgt->mutex)
		os_mutex_lock(x5b_app_mgt->mutex, OS_WAIT_FOREVER);
	if(x5b_app_mgt->local_address)
		free(x5b_app_mgt->local_address);
	if(address)
		x5b_app_mgt->local_address = strdup(address);
	if(x5b_app_mgt->reset_thread)
	{
		eloop_cancel(x5b_app_mgt->reset_thread);
		x5b_app_mgt->reset_thread = NULL;
	}
	x5b_app_event_active(x5b_app_mgt, X5B_RESET_EV, 0, 0);
	//x5b_app_mgt->reset_thread = eloop_add_timer_msec(x5b_app_mgt->master, x5b_app_reset_eloop, x5b_app_mgt, 100);
	if(x5b_app_mgt->mutex)
		os_mutex_unlock(x5b_app_mgt->mutex);
	return OK;
}

int x5b_app_local_port_set_api(u_int16 port)
{
	if(!x5b_app_mgt)
		return ERROR;
	if(x5b_app_mgt->mutex)
		os_mutex_lock(x5b_app_mgt->mutex, OS_WAIT_FOREVER);
	x5b_app_mgt->local_port = port ? port : X5B_APP_PORT_DEFAULT;
	if(x5b_app_mgt->reset_thread)
	{
		eloop_cancel(x5b_app_mgt->reset_thread);
		x5b_app_mgt->reset_thread = NULL;
	}
	x5b_app_event_active(x5b_app_mgt, X5B_RESET_EV, 0, 0);
	//x5b_app_mgt->reset_thread = eloop_add_timer_msec(x5b_app_mgt->master, x5b_app_reset_eloop, x5b_app_mgt, 100);
	if(x5b_app_mgt->mutex)
		os_mutex_unlock(x5b_app_mgt->mutex);
	return OK;
}

int x5b_app_port_set_api(int to, u_int16 port)
{
	if(!x5b_app_mgt)
		return ERROR;
	if(x5b_app_mgt->mutex)
		os_mutex_lock(x5b_app_mgt->mutex, OS_WAIT_FOREVER);
	if(to == E_CMD_TO_A)
		x5b_app_mgt->app_a.remote_port = port ? port : X5B_APP_PORT_DEFAULT;
	if(to == E_CMD_TO_C)
		x5b_app_mgt->app_c.remote_port = port ? port : X5B_APP_PORT_DEFAULT;
	if(x5b_app_mgt->mutex)
		os_mutex_unlock(x5b_app_mgt->mutex);
	return OK;
}

int x5b_app_interval_set_api(int to, u_int8 interval)
{
	if(!x5b_app_mgt)
		return ERROR;
	if(x5b_app_mgt->mutex)
		os_mutex_lock(x5b_app_mgt->mutex, OS_WAIT_FOREVER);
	if(to == E_CMD_TO_A)
		x5b_app_mgt->app_a.interval = interval ? interval : X5B_APP_INTERVAL_DEFAULT;
	if(to == E_CMD_TO_C)
		x5b_app_mgt->app_c.interval = interval ? interval : X5B_APP_INTERVAL_DEFAULT;
	if(x5b_app_mgt->mutex)
		os_mutex_unlock(x5b_app_mgt->mutex);
	return OK;
}

BOOL x5b_app_mode_X5CM()
{
	if(!x5b_app_mgt)
		return FALSE;
	return x5b_app_mgt->X5CM;
}

int x5b_app_open_mode()
{
	if(!x5b_app_mgt)
		return OPEN_NONE;
	return x5b_app_mgt->opentype;
}

int x5b_app_customizer()
{
	if(!x5b_app_mgt)
		return CUSTOMIZER_NONE;
	return x5b_app_mgt->customizer;
}
/*static int x5b_app_mode_set_type(x5b_app_mgt_t *mgt)
{
#ifdef PL_OPENWRT_UCI
	char tmp[128];
	int ret = ERROR;
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.type", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strstr(tmp, "BM"))
			mgt->X5CM = FALSE;
		else if(strstr(tmp, "CM"))
			mgt->X5CM = TRUE;
		else
			mgt->X5CM = FALSE;
	}
#else
	mgt->X5CM = FALSE;
#endif
	return OK;
}*/

static int x5b_app_mode_load(x5b_app_mgt_t *mgt)
{
#ifdef PL_OPENWRT_UCI
	char tmp[128];
	int ret = ERROR;
	zassert(mgt != NULL);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("product.global.type", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strstr(tmp, "BM"))
			mgt->X5CM = FALSE;
		else if(strstr(tmp, "CM"))
			mgt->X5CM = TRUE;
		else
			mgt->X5CM = FALSE;
	}
	zlog_debug(ZLOG_APP, "===========%s %s X5CM=%d", __func__, tmp, mgt->X5CM);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("openconfig.open.opentype", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strcmp(tmp, "FaceAndCard") == 0)
			mgt->opentype = OPEN_FACE_AND_CARD;
		else if(strcmp(tmp, "FaceOrCard") == 0)
			mgt->opentype = OPEN_FACE_OR_CARD;
		else if(strcmp(tmp, "Card") == 0)
			mgt->opentype = OPEN_CARD;
		else if(strcmp(tmp, "Face") == 0)
			mgt->opentype = OPEN_FACE;
		else
			mgt->opentype = OPEN_NONE;
	}
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("openconfig.open.wiggins", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strstr(tmp, "26-Bit"))
			mgt->app_a.wiggins = 26;
		else if(strstr(tmp, "34-Bit"))
			mgt->app_a.wiggins = 34;
		else if(strstr(tmp, "66-Bit"))
			mgt->app_a.wiggins = 66;
	}
	mgt->customizer = CUSTOMIZER_NONE;
	memset(tmp, 0, sizeof(tmp));
	ret |= os_uci_get_string("product.global.customizer", tmp);
	if(ret == OK && strlen(tmp))
	{
		if(strstr(tmp, "Secom"))
			mgt->customizer = CUSTOMIZER_SECOM;
		else if(strstr(tmp, "Huifu"))
			mgt->customizer = CUSTOMIZER_HUIFU;
		else
			mgt->customizer = CUSTOMIZER_NONE;
	}

#else
	zassert(mgt != NULL);
	mgt->X5CM = FALSE;
#endif
	return OK;
}

int x5b_app_module_init(char *local, u_int16 port)
{
	if(x5b_app_mgt == NULL)
	{
		x5b_app_mgt = malloc(sizeof(x5b_app_mgt_t));
		memset(x5b_app_mgt, 0, sizeof(x5b_app_mgt_t));

		if(master_eloop[MODULE_APP_START] == NULL)
			master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

		x5b_app_mgt->master = master_eloop[MODULE_APP_START];

		x5b_app_mgt->mutex = os_mutex_init();

		if(local)
			x5b_app_mgt->local_address = strdup(local);
		else
			x5b_app_mgt->local_address = NULL;
		x5b_app_mgt->local_port = X5B_APP_PORT_DEFAULT;

		x5b_app_mgt->app_a.interval = X5B_APP_INTERVAL_DEFAULT;
		x5b_app_mgt->app_c.interval = X5B_APP_INTERVAL_DEFAULT;
		x5b_app_mgt->app_c.keep_cnt = X5B_APP_INTERVAL_CNT_DEFAULT;
		x5b_app_mgt->app_c.keep_cnt = X5B_APP_INTERVAL_CNT_DEFAULT;
		x5b_app_mgt->app_a.remote_port = X5B_APP_PORT_DEFAULT;
		x5b_app_mgt->app_c.remote_port = X5B_APP_PORT_DEFAULT;
#ifdef X5B_APP_STATIC_IP_ENABLE
		x5b_app_mgt->app_a.address = ntohl(inet_addr(X5B_APP_A_IP_DEFAULT));
		x5b_app_mgt->app_c.address = ntohl(inet_addr(X5B_APP_C_IP_DEFAULT));
#endif
		x5b_app_mgt->app_a.priv = x5b_app_mgt;
		x5b_app_mgt->app_c.priv = x5b_app_mgt;
#ifdef X5B_APP_TCP_ENABLE
		x5b_app_mgt->accept_fd = 0;
		x5b_app_mgt->accept_thread = NULL;

		memset(&x5b_app_mgt->tcp, 0, sizeof(x5b_app_mgt_tcp_t));
		x5b_app_mgt->tcp_r = FALSE;
#endif
		x5b_app_mgt->sync_ack = TRUE;
		x5b_app_socket_init(x5b_app_mgt);
		x5b_app_mode_load(x5b_app_mgt);
		x5b_app_network_event_init(x5b_app_mgt);
		x5b_app_mgt->debug = X5_B_ESP32_DEBUG_TIME | X5_B_ESP32_DEBUG_EVENT;

#ifdef PL_OPENWRT_UCI
		uci_ubus_cb_install(x5_b_ubus_uci_update_cb);
#endif
	}
	x5b_user_load();
	return OK;
}

int x5b_app_module_exit()
{
	if(x5b_app_mgt)
	{
		x5b_app_network_event_exit(x5b_app_mgt);
		x5b_app_socket_exit(x5b_app_mgt);
		if(x5b_app_mgt->local_address)
			free(x5b_app_mgt->local_address);
		if(x5b_app_mgt->mutex)
		{
			os_mutex_exit(x5b_app_mgt->mutex);
			x5b_app_mgt->mutex = NULL;
		}
		free(x5b_app_mgt);
		x5b_app_mgt = NULL;
	}
	return OK;
}

static int x5b_app_mgt_task(void *argv)
{
	zassert(argv != NULL);
	x5b_app_mgt_t *mgt = (x5b_app_mgt_t *)argv;
	zassert(mgt != NULL);
	module_setup_task(MODULE_APP_START, os_task_id_self());
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	if(!mgt->enable)
	{
		os_sleep(5);
	}
	x5b_app_state_load(mgt);
	eloop_start_running(master_eloop[MODULE_APP_START], MODULE_APP_START);
	return OK;
}


static int x5b_app_task_init (x5b_app_mgt_t *mgt)
{
	zassert(mgt != NULL);
	if(master_eloop[MODULE_APP_START] == NULL)
		master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);

	mgt->enable = TRUE;
	mgt->task_id = os_task_create("appmgtTask", OS_TASK_DEFAULT_PRIORITY,
	               0, x5b_app_mgt_task, mgt, OS_TASK_DEFAULT_STACK);
	if(mgt->task_id)
		return OK;
	return ERROR;
}


int x5b_app_debug_proc()
{
	if(x5b_app_mgt)
	{
		x5b_app_mgt->enable = TRUE;
		x5b_app_mgt->task_id = 1;
		x5b_app_mgt_task(x5b_app_mgt);
	}
	return OK;
}

int x5b_app_module_task_init()
{
	if(x5b_app_mgt != NULL)
	{
		x5b_app_socket_init(x5b_app_mgt);
		x5b_app_task_init(x5b_app_mgt);
	}
	return OK;
}


int x5b_app_module_task_exit()
{
	if(x5b_app_mgt)
		x5b_app_socket_exit(x5b_app_mgt);
	return OK;
}


/*
 * for CLI
 */
void * x5b_app_tmp()
{
	return x5b_app_mgt;
}

int x5b_app_free()
{
	x5b_app_module_exit();
	//x5b_app_mgt->t_thread = eloop_add_timer(x5b_app_mgt->master, x5b_app_timer_eloop, x5b_app_mgt, 60);
	return OK;
}


int x5b_app_show_config(struct vty *vty)
{
	x5b_app_mgt_t *mgt = x5b_app_mgt;
	if(mgt)
	{
		if(mgt->local_address && mgt->local_port && mgt->local_port != X5B_APP_PORT_DEFAULT)
		{
			vty_out(vty, " ip esp local address %s port %d %s", mgt->local_address, mgt->local_port, VTY_NEWLINE);
		}
		else if(mgt->local_address)
			vty_out(vty, " ip esp local address %s%s", mgt->local_address, VTY_NEWLINE);
		else if(mgt->local_port && mgt->local_port != X5B_APP_PORT_DEFAULT)
			vty_out(vty, " ip esp local port %d%s", mgt->local_port, VTY_NEWLINE);

		if(mgt->app_a.remote_port && mgt->app_a.remote_port != X5B_APP_PORT_DEFAULT)
		{
			vty_out(vty, " ip esp toa port %d %s", mgt->app_a.remote_port, VTY_NEWLINE);
		}
		if(mgt->app_a.interval && mgt->app_a.interval != X5B_APP_INTERVAL_DEFAULT)
			vty_out(vty, " ip esp toa keepalive-interval %d %s", mgt->app_a.interval, VTY_NEWLINE);
		if(mgt->X5CM)
		{
			if(mgt->app_c.remote_port && mgt->app_c.remote_port != X5B_APP_PORT_DEFAULT)
			{
				vty_out(vty, " ip esp toc port %d %s", mgt->app_c.remote_port, VTY_NEWLINE);
			}
			if(mgt->app_c.interval && mgt->app_c.interval != X5B_APP_INTERVAL_DEFAULT)
				vty_out(vty, " ip esp toc keepalive-interval %d %s", mgt->app_c.interval, VTY_NEWLINE);
		}
	}
	return OK;
}

int x5b_app_show_debug(struct vty *vty)
{
	x5b_app_mgt_t *mgt = x5b_app_mgt;
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

int x5b_app_show_state(struct vty *vty)
{
	x5b_app_mgt_t *mgt = x5b_app_mgt;
	if(mgt)
	{
		vty_out(vty, "x5b type                 : %s %s", mgt->X5CM ? "X5CM":"X5BM", VTY_NEWLINE);
		vty_out(vty, " local address           : %s %s", mgt->local_address ? mgt->local_address:"any", VTY_NEWLINE);
		vty_out(vty, " local port local        : %d %s", mgt->local_port, VTY_NEWLINE);

		vty_out(vty, " A address               : %s %s", mgt->app_a.address ? inet_address(mgt->app_a.address):"any", VTY_NEWLINE);
		vty_out(vty, " A port                  : %d %s", mgt->app_a.remote_port, VTY_NEWLINE);
		vty_out(vty, " A keep cnt              : %d %s", mgt->app_a.keep_cnt, VTY_NEWLINE);
		vty_out(vty, " A ID                    : 0x%x %s", mgt->app_a.id, VTY_NEWLINE);
		vty_out(vty, " A Reg state             : %s %s", mgt->app_a.reg_state ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " A interval              : %d %s", mgt->app_a.interval, VTY_NEWLINE);
		vty_out(vty, " A send seqnum           : %d %s", mgt->app_a.seqnum, VTY_NEWLINE);
		vty_out(vty, " A module version        : %s %s", mgt->app_a.version, VTY_NEWLINE);
		if(mgt->X5CM)
		{
			vty_out(vty, " C address               : %s %s", mgt->app_c.address ? inet_address(mgt->app_c.address):"any", VTY_NEWLINE);
			vty_out(vty, " C port                  : %d %s", mgt->app_c.remote_port, VTY_NEWLINE);
			vty_out(vty, " C keep cnt              : %d %s", mgt->app_c.keep_cnt, VTY_NEWLINE);
			vty_out(vty, " C ID                    : 0x%x %s", mgt->app_c.id, VTY_NEWLINE);
			vty_out(vty, " C Reg state             : %s %s", mgt->app_c.reg_state ? "TRUE":"FALSE", VTY_NEWLINE);
			vty_out(vty, " C interval              : %d %s", mgt->app_c.interval, VTY_NEWLINE);
			vty_out(vty, " C send seqnum           : %d %s", mgt->app_c.seqnum, VTY_NEWLINE);
			vty_out(vty, " C module version        : %s %s", mgt->app_c.version, VTY_NEWLINE);
		}
	}
	return OK;
}

/*
swconfig dev switch0 show | grep port:
        link: port:0 link:up speed:100baseT full-duplex
        link: port:1 link:down
        link: port:2 link:down
        link: port:3 link:down
        link: port:4 link:down
        link: port:5 link:down
        link: port:6 link:up speed:1000baseT full-duplex

swconfig dev switch0 show | grep port: | awk -F : '{print $3 " " $4}' | awk '{print $1 "-" $3}'
0-up
1-down
2-down
3-down
4-down
5-down
6-up

static int x5b_app_c_enable_load(x5_b_a_mgt_t *mgt)
{
	int cnt = 0;
	char buf[512];
	super_system("swconfig dev switch0 show | grep link:up > /tmp/phy_status");
	FILE *fp = fopen("/tmp/phy_status", "w+");
	if(fp)
	{
		os_memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			if(strstr(buf, "link:up"))
			{
				cnt++;
			}
		}
		fclose(fp);
		if(cnt >= 4)
			mgt->enable = FALSE;
		return OK;
	}
	return ERROR;
}*/

#ifdef X5B_APP_TEST_DEBUG
int x5b_app_test_register(int to, int p)
{
	if(to == 	E_CMD_TO_A)
	{
		x5b_app_mgt->app_a.id = X5B_APP_MODULE_ID_A;
		x5b_app_mgt->app_a.reg_state = TRUE;
		x5b_app_mgt->app_a.keep_cnt = 3;			//>= 3: OK, else ERROR;
		x5b_app_mgt->app_a.interval = 3;
		x5b_app_mgt->app_a.address = ntohl(inet_addr("192.168.3.222"));
		//x5b_app_mgt->app_a.remote_address = strdup("192.168.3.222");
		x5b_app_mgt->app_a.remote_port = 9527;
		x5b_app_mgt->app_a.seqnum = 1;

	}
	if(to == 	E_CMD_TO_C)
	{
		x5b_app_mgt->app_c.id = X5B_APP_MODULE_ID_C;
		x5b_app_mgt->app_c.reg_state = TRUE;
		x5b_app_mgt->app_c.keep_cnt = 3;			//>= 3: OK, else ERROR;
		x5b_app_mgt->app_c.interval = 3;
		x5b_app_mgt->app_c.address = ntohl(inet_addr("192.168.3.222"));
		//x5b_app_mgt->app_c.remote_address = strdup("192.168.3.222");
		x5b_app_mgt->app_c.remote_port = 9527;
		x5b_app_mgt->app_c.seqnum = 1;
	}
	return x5b_app_register_ok_api(x5b_app_mgt, to, p);
}
int x5b_app_test_call(u_int16 num)
{
	u_int16 *crc = (u_int16 *)(x5b_app_mgt->buf + sizeof(x5b_app_hdr_t) + 8 + sizeof(x5b_app_call_t));
	x5b_app_hdr_t *hdr = (x5b_app_hdr_t *)x5b_app_mgt->buf;
	os_tlv_t *tlv = (os_tlv_t *)(x5b_app_mgt->buf + sizeof(x5b_app_hdr_t));
	x5b_app_call_t *call = (x5b_app_call_t *)(x5b_app_mgt->buf + sizeof(x5b_app_hdr_t) + 8);
	memset(x5b_app_mgt->buf, 0, sizeof(x5b_app_mgt->buf));
	hdr->makr = X5B_APP_HDR_MAKR;
	hdr->total_len = htonl(sizeof(x5b_app_call_t) + 8);//;
	hdr->seqnum = (3);
	hdr->m_seqnum = ~hdr->seqnum;
	tlv->tag = htonl(E_CMD_MAKE(E_CMD_MODULE_A, E_CMD_CALL, E_CMD_START_CALL));
	tlv->len = htonl(sizeof(x5b_app_call_t));
	call->building = 0;
	call->unit = 0;
	call->room_number = htons(num);

	*crc =  Data_CRC16Check (x5b_app_mgt->buf,  ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t));
	*crc = htons(*crc);
	x5b_app_mgt->len = ntohl(hdr->total_len) + sizeof(x5b_app_hdr_t) + 2;
	if(x5b_app_mgt->mutex)
		os_mutex_unlock(x5b_app_mgt->mutex);
	x5b_app_read_handle(x5b_app_mgt);
	if(x5b_app_mgt->mutex)
		os_mutex_lock(x5b_app_mgt->mutex, OS_WAIT_FOREVER);
	return OK;
}
#endif

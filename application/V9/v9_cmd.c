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

#include "application.h"

#define APP_SLIPNET_QUEUE_SIZE			1460
#pragma pack(1)
typedef struct app_slipnet_data_s {
#define APP_SLIPNET_TYPE_DATA	1
#define APP_SLIPNET_TYPE_CMD	2
	u_int8 type;
	u_int16 len;
	u_int8 data[APP_SLIPNET_QUEUE_SIZE];
}app_slipnet_data_t;
#pragma pack()


static int v9_app_rtc_tm_set(int timesp)
{
	return os_time_set_api(timesp);
}

static int v9_cmd_make_hdr(v9_serial_t *mgt)
{
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->sbuf;
	mgt->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = mgt->seqnum;
	hdr->id = mgt->id;
	return OK;
}

int v9_cmd_send_ack(v9_serial_t *mgt, u_int8 status)
{
	zassert(mgt != NULL);
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->sbuf;
	app_cmd_ack_t *ack = (app_cmd_ack_t *)(mgt->sbuf + sizeof(app_cmd_hdr_t));
	memset(mgt->sbuf, 0, sizeof(mgt->sbuf));
	v9_cmd_make_hdr(mgt);

	ack->cmd = htons(V9_APP_CMD_ACK);
	ack->status = status;
	mgt->slen += sizeof(app_cmd_ack_t);
	hdr->len = htons(sizeof(app_cmd_ack_t));
	//v9_app_hex_debug(mgt, "SEND", FALSE);
	if(mgt->tty)
		return tty_com_slip_write (mgt->tty, mgt->sbuf, mgt->slen);
	return (ERROR);
}




int v9_cmd_handle_keepalive(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	app_cmd_keepalive_t *ack = (app_cmd_keepalive_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	memset(mgt->sbuf, 0, sizeof(mgt->sbuf));
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->sbuf;
	app_cmd_status_ack_t *state = (app_cmd_status_ack_t *)(mgt->sbuf + sizeof(app_cmd_hdr_t));

	if(v9_app_rtc_tm_set(ntohl(ack->timesp)) == OK)
	{
		state->synctime = TRUE;		//时间是否已经同步
	}
	else
		state->synctime = FALSE;		//时间是否已经同步
	if(mgt->id == 0 && hdr->id != 0)
	{
		mgt->id = hdr->id;
		//if(V9_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "MSG KEEPALIVE -> SET ID = %d", hdr->id);
	}

	v9_cmd_make_hdr(mgt);
	state->cmd = htons(V9_APP_CMD_KEEPALIVE/*V9_APP_CMD_GET_STATUS*/);
	hdr->len = htons(sizeof(app_cmd_status_ack_t));

	state->temp = 56;			//温度
	state->status = 1;			//状态
	state->vch = 2;			//处理视频路数
	v9_cpu_load(&state->cpuload);		//CPU负载（百分比）
	state->cpuload = htons(state->cpuload);
	state->memtotal = htonl(1024*2);		//内存(M)
	state->memload = 65;		//内存占用（百分比）

	v9_memory_load(&state->memtotal, &state->memload);
	state->memtotal = htonl(state->memtotal);

	state->disktatol1 = htonl(1024*1024);		//硬盘(M)
	state->diskload1 = 15;		//硬盘占用（百分比）
	state->disktatol2 = htonl(1024*1024);		//硬盘(M)
	state->diskload2 = 10;		//硬盘占用（百分比）

	v9_disk_load("/mnt/sda1", &state->disktatol1, NULL, &state->diskload1);
	state->disktatol1 = htonl(state->disktatol1);

	v9_disk_load("/mnt", &state->disktatol2, NULL, &state->diskload2);
	state->disktatol2 = htonl(state->disktatol2);

	mgt->slen += sizeof(app_cmd_status_ack_t);

	if(mgt->tty)
		return tty_com_slip_write (mgt->tty, mgt->sbuf, mgt->slen);
	return (ERROR);
}


int v9_cmd_handle_reboot(v9_serial_t *mgt)
{
	int ret = 0;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	app_cmd_reboot_t *ack = (app_cmd_reboot_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(ZLOG_APP, "MSG REBOOT/RESET/SHUTDOWN %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(ack->cmd == htons(V9_APP_CMD_REBOOT))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "MSG REBOOT -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		super_system("reboot -f");
	}
	else if(ack->cmd == htons(V9_APP_CMD_RESET))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "MSG RESET -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		super_system("jffs2reset -y && reboot -f");
	}
	else if(ack->cmd == htons(V9_APP_CMD_SHUTDOWN))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(ZLOG_APP, "MSG SHUTDOWN -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		super_system("poweroff -d 1");
	}
	else
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	return ret;
}


int v9_cmd_handle_autoip(v9_serial_t *mgt)
{
	int ret = 0;
	u_int32 address = APP_BOARD_ADDRESS_PREFIX;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	app_cmd_autoip_t *ack = (app_cmd_autoip_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(ZLOG_APP, "MSG AUTOIP %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MSG AUTOIP -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);

	if(ack->cmd == htons(V9_APP_CMD_AUTOIP))
	{
		switch(hdr->id)
		{
		case APP_BOARD_MAIN:
			address += APP_BOARD_ADDRESS_MAIN;
			break;
		case APP_BOARD_CALCU_1:
			address += APP_BOARD_CALCU_1;
			break;
		case APP_BOARD_CALCU_2:
			address += APP_BOARD_CALCU_2;
			break;
		case APP_BOARD_CALCU_3:
			address += APP_BOARD_CALCU_3;
			break;
		case APP_BOARD_CALCU_4:
			address += APP_BOARD_CALCU_4;
			break;
		default:
			address = 0;
			ret = v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
			break;
		}
		if(address)
		{
			//TODO setup ip address
			ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		}
	}
	else
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	return ret;
}

int v9_cmd_handle_startup(v9_serial_t *mgt)
{
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	app_cmd_startup_t *ack = (app_cmd_startup_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(ZLOG_APP, "MSG STARTUP %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MSG STARTUP(%d) -> id=%d(id=%d) ACK seqnum = %d", ack->status, hdr->id, mgt->id, hdr->seqnum);

	//if(v9_video_board_active(ack->status, TRUE) == OK)
	//{
		return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
/*	}
	else
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);*/
}


int v9_cmd_handle_route(v9_serial_t *mgt)
{
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	app_cmd_route_t *ack = (app_cmd_route_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(ZLOG_APP, "MSG ROUTE %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MSG ROUTE -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
#ifdef PL_OPENWRT_UCI
	if(ack->address == 0 && ack->port == 1)//DHCP inport
	{
		os_uci_set_string("network.wan.proto", "dhcp");
		os_uci_del("network", "wan", "ipaddr", NULL);
		os_uci_del("network", "wan", "netmask", NULL);
		os_uci_del("network", "wan", "gateway", NULL);
		os_uci_del("network", "wan", "dns", NULL);
	}
	if(ack->address != 0 && ack->port == 1)//static inport
	{
		os_uci_set_string("network.wan.proto", "static");
		os_uci_set_string("network.wan.ipaddr", inet_address(ntohl(ack->address)));
		os_uci_set_string("network.wan.netmask", inet_address(ntohl(ack->netmask)));
		if(ack->gateway)
			os_uci_set_string("network.wan.gateway", inet_address(ntohl(ack->gateway)));
		if(ack->dns)
			os_uci_set_string("network.wan.dns", inet_address(ntohl(ack->dns)));
	}

	if(ack->address == 0 && ack->port == 2)//DHCP outport
	{
		os_uci_set_string("network.wan2.proto", "dhcp");
		os_uci_del("network", "wan2", "ipaddr", NULL);
		os_uci_del("network", "wan2", "netmask", NULL);
		os_uci_del("network", "wan2", "gateway", NULL);
		os_uci_del("network", "wan2", "dns", NULL);
	}
	if(ack->address != 0 && ack->port == 2)//static outport
	{
		os_uci_set_string("network.wan2.proto", "static");
		os_uci_set_string("network.wan2.ipaddr", inet_address(ntohl(ack->address)));
		os_uci_set_string("network.wan2.netmask", inet_address(ntohl(ack->netmask)));
		if(ack->gateway)
			os_uci_set_string("network.wan2.gateway", inet_address(ntohl(ack->gateway)));
		if(ack->dns)
			os_uci_set_string("network.wan2.dns", inet_address(ntohl(ack->dns)));
	}
	os_uci_save_config("network");
#endif
	return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
}



int v9_cmd_handle_board(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	v9_address_t	rboard;
	v9_address_t	*board = (v9_address_t *)(mgt->buf + sizeof(app_cmd_hdr_t) + 2);
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(ZLOG_APP, "MSG BOARD %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(ZLOG_APP, "MSG BOARD -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);

	if(v9_board_lookup(board->id))
	{
		//zlog_debug(ZLOG_APP, "Board is already setup and update info");
		memcpy(&rboard, board, sizeof(v9_address_t));
		rboard.ip = ntohl(board->ip);				//IP地址
		rboard.cpuload = ntohs(board->cpuload);
		rboard.memtotal = ntohl(board->memtotal);		//内存(M)
		rboard.disktatol1 = ntohl(board->disktatol1);		//硬盘(M)
		rboard.disktatol2 = ntohl(board->disktatol2);		//硬盘(M)

		v9_board_update_board(board->id, &rboard);
	}
	else
	{
		//zlog_debug(ZLOG_APP, "Board is not setup and add board info");
		memcpy(&rboard, board, sizeof(v9_address_t));
		rboard.ip = ntohl(board->ip);				//IP地址
		rboard.cpuload = ntohs(board->cpuload);
		rboard.memtotal = ntohl(board->memtotal);		//内存(M)
		rboard.disktatol1 = ntohl(board->disktatol1);		//硬盘(M)
		rboard.disktatol2 = ntohl(board->disktatol2);		//硬盘(M)

		v9_board_update_board(board->id, &rboard);
	}
	return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
}

int v9_cmd_get(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	app_cmd_reboot_t *ack = (app_cmd_reboot_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	mgt->seqnum = hdr->seqnum;
	return ntohs(ack->cmd);
}



int v9_cmd_sync_time_to_rtc(v9_serial_t *mgt, u_int32 timesp)
{
	int ret = 0;
    struct tm *p_tm = NULL;
    //struct tm tm_new;
    u_int32 ptimesp = timesp;
	zassert(mgt != NULL);
	app_slipnet_data_t slipdata;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)slipdata.data;
	app_cmd_rtc_t *rtc = (app_cmd_rtc_t *)(slipdata.data + sizeof(app_cmd_hdr_t));

	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	memset(slipdata.data, 0, sizeof(slipdata.data));

	slipdata.type = APP_SLIPNET_TYPE_CMD;

	mgt->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = mgt->seqnum;

	rtc->cmd = htons(V9_CMD_SYNC_TIME);
	rtc->zone = 8;

    p_tm = localtime(&ptimesp);

	//zlog_info(ZLOG_APP, "%s: %d/%d/%d %d:%d:%d", __func__, p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
	//		  p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

	rtc->ptm.tm_year = htonl(p_tm->tm_year + 1900);
	rtc->ptm.tm_mon = htonl(p_tm->tm_mon + 1);
	rtc->ptm.tm_mday = htonl(p_tm->tm_mday);
	rtc->ptm.tm_hour = htonl(p_tm->tm_hour);
	rtc->ptm.tm_min = htonl(p_tm->tm_min);
	rtc->ptm.tm_sec = htonl(p_tm->tm_sec);

	mgt->slen += sizeof(app_cmd_rtc_t);
	hdr->len = htons(sizeof(app_cmd_rtc_t));

	slipdata.len = htons(mgt->slen + 3);

	if(mgt->slipnet)
		ret = tty_com_slip_write (mgt->slipnet, &slipdata, ntohs(slipdata.len));

	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return (ret);
}

int v9_cmd_sync_time_test(void)
{
	u_int32 timesp = os_time(NULL);
	zlog_debug(ZLOG_APP, "---------%s--------- %d", __func__, timesp);
	v9_cmd_sync_time_to_rtc(v9_serial, timesp);
	return OK;
}

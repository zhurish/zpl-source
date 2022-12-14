#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "tty_com.h"

#include "v9_device.h"
#include "v9_util.h"
#include "v9_video.h"
#include "v9_serial.h"
#include "v9_slipnet.h"
#include "v9_cmd.h"

#include "v9_video_disk.h"
#include "v9_user_db.h"
#include "v9_video_db.h"

#include "v9_board.h"
#include "v9_video_sdk.h"
#include "v9_video_user.h"
#include "v9_video_board.h"
#include "v9_video_api.h"

#ifdef ZPL_WEBGUI_MODULE
#include "web_util.h"
#endif


app_cmd_device_t bios_device;
static int biso_finsh = 0;

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

int v9_cmd_send_ack(v9_serial_t *mgt, zpl_uint8 status)
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

	if(mgt->tty)
		return tty_com_write (mgt->tty, mgt->sbuf, mgt->slen);
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
		state->synctime = zpl_true;		//时间是否已经同步
	}
	else
		state->synctime = zpl_false;		//时间是否已经同步
	if(mgt->id == 0 && hdr->id != 0)
	{
		if(hdr->id != APP_BOARD_MAIN)
		{
			return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
		}
		mgt->id = hdr->id;
		if(V9_APP_DEBUG(EVENT))
			zlog_warn(MODULE_APP, "MSG KEEPALIVE -> SET ID = %d", hdr->id);
	}

	v9_cmd_make_hdr(mgt);
	state->cmd = htons(V9_APP_CMD_KEEPALIVE/*V9_APP_CMD_GET_STATUS*/);
	hdr->len = htons(sizeof(app_cmd_status_ack_t));

	state->temp = 56;			//温度
	state->status = 1;			//状态
	state->vch = 0;			//处理视频路数
	v9_cpu_load(&state->cpuload);		//CPU负载（百分比）
	state->cpuload = htons(state->cpuload);
	state->memtotal = 0;		//内存(M)
	state->memload = 0;		//内存占用（百分比）

	v9_memory_load(&state->memtotal, &state->memload);
	state->memtotal = htonl(state->memtotal);

	state->disktatol1 = 0;		//硬盘(M)
	state->diskload1 = 0;		//硬盘占用（百分比）
	state->disktatol2 = 0;		//硬盘(M)
	state->diskload2 = 0;		//硬盘占用（百分比）

	v9_disk_load("/mnt/diska1", &state->disktatol1, NULL, &state->diskload1);
	state->disktatol1 = htonl(state->disktatol1);
	if(v9_video_disk_count() == 2)
	{
		v9_disk_load("/mnt/diskb1", &state->disktatol2, NULL, &state->diskload2);
		state->disktatol2 = htonl(state->disktatol2);
	}
	mgt->slen += sizeof(app_cmd_status_ack_t);

	if(mgt->tty)
		return tty_com_write (mgt->tty, mgt->sbuf, mgt->slen);
	return (ERROR);
}


int v9_cmd_handle_reboot(v9_serial_t *mgt)
{
	int ret = 0;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	app_cmd_reboot_t *ack = (app_cmd_reboot_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG REBOOT/RESET/SHUTDOWN %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(ack->cmd == htons(V9_APP_CMD_REBOOT))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "MSG REBOOT -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		v9_app_module_exit();
		super_system("reboot -f");
	}
	else if(ack->cmd == htons(V9_APP_CMD_RESET))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "MSG RESET -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		v9_app_module_exit();
		super_system("jffs2reset -y && reboot -f");
	}
	else if(ack->cmd == htons(V9_APP_CMD_SHUTDOWN))
	{
		if(V9_APP_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "MSG SHUTDOWN -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
		v9_app_module_exit();
		super_system("poweroff -d 1");
	}
	else
		ret = v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	return ret;
}


int v9_cmd_handle_autoip(v9_serial_t *mgt)
{
	int ret = 0;
	zpl_uint32 address = APP_BOARD_ADDRESS_PREFIX;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)mgt->buf;
	app_cmd_autoip_t *ack = (app_cmd_autoip_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG AUTOIP %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "MSG AUTOIP -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);

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
		zlog_warn(MODULE_APP, "MSG STARTUP %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "MSG STARTUP(%d) -> id=%d(id=%d) ACK seqnum = %d", ack->status, hdr->id, mgt->id, hdr->seqnum);

	//if(v9_video_board_active(ack->status, zpl_true) == OK)
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
		zlog_warn(MODULE_APP, "MSG ROUTE %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "MSG ROUTE -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);
#ifdef ZPL_OPENWRT_UCI
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
	v9_board_t	rboard;
	v9_board_t	*board = (v9_board_t *)(mgt->buf + sizeof(app_cmd_hdr_t) + 2);
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG BOARD %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	//if(V9_APP_DEBUG(EVENT))
	//	zlog_debug(MODULE_APP, "MSG BOARD Status -> id=%d ACK seqnum = %d", board->id, hdr->seqnum);
		//zlog_debug(MODULE_APP, "MSG BOARD -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);

	if(v9_board_lookup(board->id))
	{
		memcpy(&rboard, board, sizeof(v9_board_t));
		rboard.ip = ntohl(board->ip);				//IP地址
		rboard.cpuload = ntohs(board->cpuload);
		rboard.memtotal = ntohl(board->memtotal);		//内存(M)
		rboard.disktatol1 = ntohl(board->disktatol1);		//硬盘(M)
		rboard.disktatol2 = ntohl(board->disktatol2);		//硬盘(M)
		v9_video_board_lock();
		v9_board_update_board(board->id, &rboard);
		v9_video_board_unlock();
	}
	else
	{
		memcpy(&rboard, board, sizeof(v9_board_t));
		rboard.ip = ntohl(board->ip);				//IP地址
		rboard.cpuload = ntohs(board->cpuload);
		rboard.memtotal = ntohl(board->memtotal);		//内存(M)
		rboard.disktatol1 = ntohl(board->disktatol1);		//硬盘(M)
		rboard.disktatol2 = ntohl(board->disktatol2);		//硬盘(M)
		v9_video_board_lock();
		v9_board_update_board(board->id, &rboard);
		v9_video_board_unlock();
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

int v9_cmd_handle_device(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	app_cmd_device_t *device = (app_cmd_device_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG Device %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	v9_video_board_lock();
	if(bios_device.cmd == 0)
	{
		memset(&bios_device, '\0', sizeof(bios_device));
		bios_device.cmd = 0;				//IP地址
		strcpy(bios_device.devicename, device->devicename);
		strcpy(bios_device.deviceid, device->deviceid);
		strcpy(bios_device.serialno, device->serialno);
		strcpy(bios_device.manufacturer, device->manufacturer);
		strcpy(bios_device.kervel_version, device->kervel_version);
		strcpy(bios_device.app_version, device->app_version);
		strcpy(bios_device.buildtime, device->buildtime);
/*
		zlog_warn(MODULE_APP,"%s: devicename=%s", __func__, bios_device.devicename);
		zlog_warn(MODULE_APP,"%s: deviceid=%s", __func__, bios_device.deviceid);
		zlog_warn(MODULE_APP,"%s: serialno=%s", __func__, bios_device.serialno);
		zlog_warn(MODULE_APP,"%s: manufacturer=%s", __func__, bios_device.manufacturer);
		zlog_warn(MODULE_APP,"%s: kervel_version=%s", __func__, bios_device.kervel_version);
		zlog_warn(MODULE_APP,"%s: buildtime=%s", __func__, bios_device.buildtime);
		zlog_warn(MODULE_APP,"%s: app_version=%s", __func__, bios_device.app_version);*/
	}
	v9_video_board_unlock();
	return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
}

int v9_web_device_info(char *buf)
{
	zpl_uint32 offset = 0;
	sprintf (buf + offset, "\"serialno\":\"%s\",", bios_device.serialno);

	offset = strlen(buf);
	sprintf (buf + offset, "\"deviceid\":\"%s\",", bios_device.deviceid);

	offset = strlen(buf);
	sprintf (buf + offset, "\"manufacturer\":\"%s\",", bios_device.manufacturer);

	offset = strlen(buf);
	sprintf (buf + offset, "\"buildtime\":\"%s\",", bios_device.buildtime);

	offset = strlen(buf);
	sprintf (buf + offset, "\"biosversion\":\"%s-%s\",",
			 bios_device.kervel_version,
			 bios_device.app_version);

	offset = strlen(buf);
	return offset;
}

int v9_cmd_handle_pass_reset(v9_serial_t *mgt)
{
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	app_cmd_status_t *ack = (app_cmd_status_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG PASS RESET %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "MSG PASS RESET(%d) -> id=%d(id=%d) ACK seqnum = %d", ack->cmd, hdr->id, mgt->id, hdr->seqnum);
#ifdef ZPL_WEBGUI_MODULE
	if(webs_username_password_update(NULL, WEB_LOGIN_USERNAME, WEB_LOGIN_PASSWORD) == OK)
	{
		//if(V9_APP_DEBUG(EVENT))
			zlog_debug(MODULE_APP, "Reset Webs Login User Password Successful");
		return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
	}
	else
	{
		zlog_debug(MODULE_APP, "Reset Webs Login User Password Failed");
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
#else
	return v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
#endif
}

#ifdef V9_SLIPNET_ENABLE
int v9_cmd_sync_time_to_rtc(v9_serial_t *mgt, zpl_uint32 timesp)
{
	int ret = 0;
    //struct tm p_tm;
    zpl_uint32 ptimesp = timesp;
	zassert(mgt != NULL);
	app_slipnet_data_t slipdata;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)slipdata.data;
	app_cmd_rtc_t *rtc = (app_cmd_rtc_t *)(slipdata.data + sizeof(app_cmd_hdr_t));

	if(!mgt->slipnet)
		return OK;
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	memset(slipdata.data, 0, sizeof(slipdata.data));

	slipdata.type = APP_SLIPNET_TYPE_CMD;

	mgt->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = mgt->seqnum;

	rtc->cmd = htons(V9_APP_CMD_SYNC_TIME);
	rtc->zone = 8;

/*    if(os_tmtime_get (OS_TMTIME_UTC, ptimesp, &p_tm) != OK)
    {
    	if(mgt->mutex)
    		os_mutex_unlock(mgt->mutex);
    	return ERROR;
    }*/
	rtc->timesp = htonl(ptimesp);

	mgt->slen += sizeof(app_cmd_rtc_t);
	hdr->len = htons(sizeof(app_cmd_rtc_t));

	slipdata.len = htons(mgt->slen + 3);
#ifdef V9_SLIPNET_UDP
	if(mgt->slipnet && mgt->slipnet->fd)
		ret = sock_client_write(mgt->slipnet->fd, mgt->slipnet->devname,
								V9_SLIPNET_UDPSRV_PORT, &slipdata, ntohs(slipdata.len));
#else
	if(mgt->slipnet)
		ret = tty_com_write (mgt->slipnet, &slipdata, ntohs(slipdata.len));
#endif
	if(ret > OK)
	{
		mgt->sntp_sync = zpl_true;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return (ret);
}


int v9_cmd_sync_time_test(void)
{
	zpl_uint32 timesp = os_time(NULL);//返回UTC时间
	//zlog_debug(MODULE_APP, "---------%s--------- %d", __func__, timesp);
	v9_cmd_sync_time_to_rtc(v9_serial, timesp);
	return OK;
}

int v9_cmd_web_reboot()
{
	int ret = 0;
	zassert(v9_serial != NULL);
	app_slipnet_data_t slipdata;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)slipdata.data;
	app_cmd_reboot_t *rtc = (app_cmd_reboot_t *)(slipdata.data + sizeof(app_cmd_hdr_t));

	if(!v9_serial->slipnet)
		return OK;
	if(v9_serial->mutex)
		os_mutex_lock(v9_serial->mutex, OS_WAIT_FOREVER);

	memset(slipdata.data, 0, sizeof(slipdata.data));

	slipdata.type = APP_SLIPNET_TYPE_CMD;

	v9_serial->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = v9_serial->seqnum;

	rtc->cmd = htons(V9_APP_CMD_REBOOT);

	//len = app_cmd_makeup(APP_CMD_REBOOT, buf);

	v9_serial->slen += sizeof(app_cmd_reboot_t);
	hdr->len = htons(sizeof(app_cmd_reboot_t));

	slipdata.len = htons(v9_serial->slen + 3);
#ifdef V9_SLIPNET_UDP
	if(v9_serial->slipnet && v9_serial->slipnet->fd)
		ret = sock_client_write(v9_serial->slipnet->fd, v9_serial->slipnet->devname,
								V9_SLIPNET_UDPSRV_PORT, &slipdata, ntohs(slipdata.len));
#else
	if(v9_serial->slipnet)
		ret = tty_com_write (v9_serial->slipnet, &slipdata, ntohs(slipdata.len));
#endif
	if(v9_serial->mutex)
		os_mutex_unlock(v9_serial->mutex);
	return (ret);
}

int v9_cmd_sync_led(v9_serial_t *mgt, zpl_uint32 led, int status)
{
	int ret = 0;
	zassert(mgt != NULL);
	app_slipnet_data_t slipdata;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)slipdata.data;
	app_cmd_led_t *rtc = (app_cmd_led_t *)(slipdata.data + sizeof(app_cmd_hdr_t));

	if(!mgt->slipnet)
	{
		//printf("--------%s---------:if(!mgt->slipnet)\r\n", __func__);
		return OK;
	}
	//printf("--------%s---------:\r\n", __func__);
	if(mgt->mutex)
		os_mutex_lock(mgt->mutex, OS_WAIT_FOREVER);

	memset(slipdata.data, 0, sizeof(slipdata.data));

	slipdata.type = APP_SLIPNET_TYPE_CMD;

	mgt->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = mgt->seqnum;

	rtc->cmd = htons(V9_APP_CMD_SYNC_LED);
	rtc->zone = status;
	rtc->timesp = htonl(led);

	mgt->slen += sizeof(app_cmd_led_t);
	hdr->len = htons(sizeof(app_cmd_led_t));

	slipdata.len = htons(mgt->slen + 3);
#ifdef V9_SLIPNET_UDP
	if(mgt->slipnet && mgt->slipnet->fd)
		ret = sock_client_write(mgt->slipnet->fd, mgt->slipnet->devname,
								V9_SLIPNET_UDPSRV_PORT, &slipdata, ntohs(slipdata.len));
#else
	if(mgt->slipnet)
		ret = tty_com_write (mgt->slipnet, &slipdata, ntohs(slipdata.len));
#endif
	if(ret > OK)
	{
		mgt->sntp_sync = zpl_true;
	}
	if(mgt->mutex)
		os_mutex_unlock(mgt->mutex);
	return (ret);
}

int v9_cmd_update_bios(zpl_uint32 state)
{
	int ret = 0;
	zassert(v9_serial != NULL);
	app_slipnet_data_t slipdata;
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)slipdata.data;
	app_cmd_bios_t *bios = (app_cmd_bios_t *)(slipdata.data + sizeof(app_cmd_hdr_t));

	if(!v9_serial->slipnet)
		return OK;
	if(v9_serial->mutex)
		os_mutex_lock(v9_serial->mutex, OS_WAIT_FOREVER);

	memset(slipdata.data, 0, sizeof(slipdata.data));

	slipdata.type = APP_SLIPNET_TYPE_CMD;

	v9_serial->slen = sizeof(app_cmd_hdr_t);
	hdr->seqnum = v9_serial->seqnum;

	bios->cmd = htons(V9_APP_CMD_DOWNLOAD_OTA);
	bios->filelen = htons(state);
	v9_serial->slen += sizeof(app_cmd_bios_t);
	hdr->len = htons(sizeof(app_cmd_bios_t));

	slipdata.len = htons(v9_serial->slen + 3);

	biso_finsh = 0;

#ifdef V9_SLIPNET_UDP
	if(v9_serial->slipnet && v9_serial->slipnet->fd)
		ret = sock_client_write(v9_serial->slipnet->fd, v9_serial->slipnet->devname,
								V9_SLIPNET_UDPSRV_PORT, &slipdata, ntohs(slipdata.len));
#else
	if(v9_serial->slipnet)
		ret = tty_com_write (v9_serial->slipnet, &slipdata, ntohs(slipdata.len));
#endif
	if(v9_serial->mutex)
		os_mutex_unlock(v9_serial->mutex);
	return (ret);
}


int v9_cmd_update_bios_ack(v9_serial_t *mgt)
{
	zassert(mgt != NULL);
	app_cmd_bios_t	*bios = (app_cmd_bios_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG BOARD %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	v9_cmd_send_ack (mgt, V9_APP_ACK_OK);
	if(bios->filelen == 0)
	{
		biso_finsh = 1;
		return OK;
	}
	else
	{
		biso_finsh = 2;
		return ERROR;
	}
}

int v9_cmd_update_bios_finsh()
{
	while(biso_finsh == 0)
	{
		os_msleep(1);
	}
	return biso_finsh;
}

#else
int v9_cmd_handle_sntp_sync(v9_serial_t *mgt)
{
	app_cmd_hdr_t *hdr = (app_cmd_hdr_t *)(mgt->buf);
	//app_cmd_rtc_t *ack = (app_cmd_rtc_t *)(mgt->buf + sizeof(app_cmd_hdr_t));
	app_cmd_hdr_t *shdr = (app_cmd_hdr_t *)mgt->sbuf;
	app_cmd_rtc_t *rtc = (app_cmd_rtc_t *)(mgt->sbuf + sizeof(app_cmd_hdr_t));

	if(mgt->id == 0 || mgt->id != hdr->id)
	{
		zlog_warn(MODULE_APP, "MSG SNTP SYNC %d = %d", mgt->id, hdr->id);
		return v9_cmd_send_ack (mgt, V9_APP_ACK_ERROR);
	}
	if(V9_APP_DEBUG(EVENT))
		zlog_debug(MODULE_APP, "MSG SNTP SYNC -> id=%d(id=%d) ACK seqnum = %d", hdr->id, mgt->id, hdr->seqnum);

	v9_cmd_make_hdr(mgt);
	rtc->cmd = htons(V9_APP_CMD_SYNC_TIME/*V9_APP_CMD_GET_STATUS*/);
	shdr->len = htons(sizeof(app_cmd_rtc_t));

	mgt->slen = sizeof(app_cmd_hdr_t);
	shdr->seqnum = mgt->seqnum;

	rtc->cmd = htons(V9_APP_CMD_SYNC_TIME);
	if(mgt->timer_sync == 1)
	{
		rtc->zone = 8;
		rtc->timesp = htonl(os_time(NULL));
		mgt->timer_sync = 0;
	}
	else
	{
		rtc->zone = 0;
		rtc->timesp = 0;
	}
	mgt->slen += sizeof(app_cmd_rtc_t);

	if(mgt->tty)
		return tty_com_write (mgt->tty, mgt->sbuf, mgt->slen);
	return (ERROR);
}
#endif /* V9_SLIPNET_ENABLE */

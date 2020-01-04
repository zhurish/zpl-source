/*
 * web_boardcard.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#define HAS_BOOL 1
#include "zebra.h"
#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

//#ifdef PL_APP_MODULE
#include "application.h"

static int web_board_tbl(Webs *wp, char *path, char *query)
{
	int i = 0;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	for (i = 0; i < v9_board.b_max; i++)
	{
		if (v9_board.board[i] != NULL && v9_board.board[i]->use == TRUE)
		{
			if (v9_board.board[i]->online == TRUE)
			{
				if(wp->iValue > 0)
					websWrite(wp, "%s", ",");

				websWrite(wp, "{\"ID\":\"%d\", \"power\":%s, \"running\":%s,"
					"\"temp\":%d, \"VCH\":%d, \"memory\":\"%d%%\", \"cpu\":\"%d.%d%%\"}",
					v9_board.board[i]->id,
					v9_board.board[i]->online ? "true":"false",
					v9_board.board[i]->active ? "true":"false",
					v9_board.board[i]->temp,
					v9_video_board_get_vch(v9_board.board[i]->id),//v9_board.board[i]->vch,
					v9_board.board[i]->memload,
					(v9_board.board[i]->cpuload>>8)&0xff,
					(v9_board.board[i]->cpuload)&0xff);
				wp->iValue++;
			}
		}
	}
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}


static int web_board_sdk_detail_one(Webs *wp, v9_video_sdk_t *sdk)
{
	if(sdk)
	{
		zlog_debug(ZLOG_APP, "web_board_sdk_detail_one: sdk->getstate=%d", sdk->getstate);
	}
	if(sdk && sdk->device && sdk->getstate)
	{
#ifdef V9_VIDEO_SDK_API
		char datatype[64];
		memset(datatype, 0, sizeof(datatype));
		ST_SDKDeviceInfo *device = (ST_SDKDeviceInfo *)sdk->device;

		websWrite(wp,
				"{\"response\":\"%s\", \"devname\":\"%s\", \"devid\":\"%s\",",
				"OK", device->szDeviceName, device->szDeviceId);

		if(device->nDeviceType == EAIS_DEVICE_TYPE_UNKNOW)
			websWrite(wp,"\"type\":\"%s\", ","未知类型");
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_SNAP)
			websWrite(wp,"\"type\":\"%s\", ","抓拍类型");
		else if(device->nDeviceType == EAIS_DEVICE_TYPE_RECOGNIZE)
			websWrite(wp,"\"type\":\"%s\", ","识别类型");
		else
			websWrite(wp,"\"type\":\"%s\", ","未知类型");

		if(sdk->mode == EAIS_SOLUTION_TYPE_SNAP)
			websWrite(wp,"\"mode\":\"%s\", ","抓拍类型");
		else if(sdk->mode == EAIS_SOLUTION_TYPE_HELMET)
			websWrite(wp,"\"mode\":\"%s\", ","安全帽类型");
		else if(sdk->mode == EAIS_SOLUTION_TYPE_RECOGNITION)
			websWrite(wp,"\"mode\":\"%s\", ","识别类型");
		else
			websWrite(wp,"\"mode\":\"%s\", ","未知类型");

		websWrite(wp,"\"ip\":\"%s\", \"netmask\":\"%s\", \"gateway\":\"%s\",",
				  device->szDeviceIP, device->szSubnetMask, device->szGatewayAddr);

/*
		vty_out (vty, " Service Port        : %d%s", device->nDeviceSerPort, VTY_NEWLINE);// 设备服务端口
		vty_out (vty, " Update Port         : %d%s", device->nDeviceUpdatePort, VTY_NEWLINE);// 设备升级服务端口号
		vty_out (vty, " HTTP Port           : %d%s", device->nDeviceHttpPort, VTY_NEWLINE);// HTTP服务端口号
		vty_out (vty, " RTSP Port           : %d%s", device->nDeviceRTSPPort, VTY_NEWLINE);// RTSP服务端口号
*/

		websWrite(wp,"\"product\":\"%s\", \"productid\":\"%s\", \"model\":\"%s\",",
				  device->szManufacturerName, device->szManufacturerId, device->szProductModel);

		websWrite(wp,"\"SN\":\"%s\", \"swv\":\"%s\", \"hwv\":\"%s\",",
				  device->szSN, device->szSoftWareInfo, device->szHardWareInfo);

		websWrite(wp,"\"CoreSN\":\"%s\", \"WorkMode\":\"%s\"}",
				  device->szCoreSN, (device->nWorkMode)? "老化模式":"正常模式");
		return OK;
#else
		return ERROR;
#endif
	}
	return ERROR;
}


static int web_board_detail_one(Webs *wp, char *path, char *query, int type)
{
	//int ret = 0;
	char *strID = NULL;
	u_int8 id = 0;
	v9_video_sdk_t * sdk = NULL;
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		return ERROR;
	}
	id = atoi(strID);
	sdk = v9_video_sdk_lookup(id);
	zlog_debug(ZLOG_APP, "web_board_detail_one: ID=%d(%s)", id, strID);

	if(sdk)
	{

		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		web_board_sdk_detail_one(wp, sdk);
	/*	websWrite(wp,
				"{\"response\":\"%s\", \"serial\":\"%s\", \"hostname\":\"%s\", \"mac\":\"%s\"}",
				"OK", serial, host_name_get(), inet_ethernet(sysmac));*/
		websDone(wp);
		return OK;
	}
	zlog_debug(ZLOG_APP, "web_board_detail_one: v9_video_sdk_lookup");
	return ERROR;
}

static int web_board_detail(Webs *wp, void *p)
{
	return web_board_detail_one(wp, NULL, NULL, 0);
}

/*static int web_board_detail(Webs *wp, char *path, char *query)
{
	return web_board_detail_one(wp, NULL, NULL, 0);
}*/

int web_boardcard_app(void)
{
	websFormDefine("boardcard-tbl", web_board_tbl);
	web_button_add_hook("boardcard", "detail", web_board_detail, NULL);
	//websFormDefine("boardcard-detail", web_board_detail);
	//websFormDefine("allvideostream", web_board_all_stream_detail);
	return 0;
}
//#endif

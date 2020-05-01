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
	for (i = 0; i < V9_APP_BOARD_MAX; i++)
	{
		if (v9_video_board[i].board.use == TRUE && v9_video_board[i].id != APP_BOARD_MAIN)
		{
			//if (v9_video_board[i].board.online == TRUE)
			{
				if(wp->iValue > 0)
					websWrite(wp, "%s", ",");

				websWrite(wp, "{\"ID\":\"%d\", \"power\":%s, \"running\":%s,"
					"\"temp\":%d, \"VCH\":%d, \"memory\":\"%d%%\", \"cpu\":\"%d.%d%%\"}",
					V9_APP_BOARD_HW_ID(v9_video_board[i].id),
					v9_video_board[i].board.online ? "true":"false",
					v9_video_board[i].board.active ? "true":"false",
					v9_video_board[i].board.temp,
					v9_video_board_get_vch(v9_video_board[i].id),
					v9_video_board[i].board.memload,
					(v9_video_board[i].board.cpuload>>8)&0xff,
					(v9_video_board[i].board.cpuload)&0xff);
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


static int web_board_detail(Webs *wp, void *p)
{
	char *strID = NULL;
	u_int8 id = 0;
	v9_video_sdk_t * sdk = NULL;
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_WEB, "Can not Get Board ID Value");
		return ERROR;
	}
	id = atoi(strID);
	sdk = v9_video_sdk_lookup(V9_APP_BOARD_CALCU_ID(id));
	_WEB_DBG_TRAP( "web_board_detail_one: ID=%d(%s)", id, strID);

	if(sdk)
	{
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		web_board_sdk_detail_one(wp, sdk);

		websDone(wp);
		return OK;
	}
	_WEB_DBG_TRAP( "web_board_detail_one: v9_video_sdk_lookup");
	return ERROR;
}


int web_boardcard_app(void)
{
	websFormDefine("boardcard-tbl", web_board_tbl);
	web_button_add_hook("boardcard", "detail", web_board_detail, NULL);
	return 0;
}
//#endif

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
#include <sys/statfs.h>
#include <sys/vfs.h>

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

//#ifdef PL_APP_MODULE
#include "application.h"

static int web_board_tbl(Webs *wp, char *path, char *query)
{
	int i = 0;
	web_assert(wp);
	web_assert(v9_video_board);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	v9_video_board_lock();
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
					(v9_video_board[i].board.active && v9_video_board[i].sdk.login) ? "true":"false",
					v9_video_board[i].board.temp,
					v9_video_board_get_vch(v9_video_board[i].id),
					v9_video_board[i].board.memload,
					(v9_video_board[i].board.cpuload>>8)&0xff,
					(v9_video_board[i].board.cpuload)&0xff);
				wp->iValue++;
			}
		}
	}
	v9_video_board_unlock();
	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}


static int web_board_sdk_detail_one(Webs *wp, v9_video_sdk_t *sdk)
{
	web_assert(wp);
	web_assert(sdk);
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
	web_assert(wp);
	strID = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strID)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get Board ID Value");
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



static int web_board_diskinfo_tbl(Webs *wp, char *path, char *query)
{
	char tmp[128];
	u_int32 disktatol1 = 0;
	u_int32 diskuse1 = 0;
	u_int32 diskload1 = 0;
	u_int glen = 0, tlen = 0,  mlen = 0;
	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;

	v9_disk_load("/mnt/diska1", &disktatol1, &diskuse1, &diskload1);
	diskuse1 <<= 20;
	//disktatol1 <<= 20;
	mlen = (disktatol1) & 0X000003FF;
	glen = (disktatol1>>10) & 0X000003FF;
	tlen = (disktatol1>>20) & 0X000003FF;

	memset(tmp, 0, sizeof(tmp));
	if(tlen > 0)
	{
		snprintf(tmp, sizeof(tmp), "%d.%02d T",tlen, glen);
	}
	else if(glen > 0)
	{
		snprintf(tmp, sizeof(tmp), "%d.%02d G",glen, mlen);
	}
	else
		snprintf(tmp, sizeof(tmp), "%d M",disktatol1);

	if(wp->iValue > 0)
		websWrite(wp, "%s", ",");
	websWrite(wp, "{\"id\":%d, \"capacity\":\"%s\", \"use\":\"%s\", \"load\":\"%d%%\"}",
		1,
		tmp,
		os_file_size_string(diskuse1),
		diskload1);
	wp->iValue++;



	disktatol1 = 0;
	diskuse1 = 0;
	diskload1 = 0;

	v9_disk_load("/mnt/diskb1", &disktatol1, &diskuse1, &diskload1);
	diskuse1 <<= 20;
	//disktatol1 <<= 20;

	mlen = (disktatol1) & 0X000003FF;
	glen = (disktatol1>>10) & 0X000003FF;
	tlen = (disktatol1>>20) & 0X000003FF;

	memset(tmp, 0, sizeof(tmp));
	if(tlen > 0)
	{
		snprintf(tmp, sizeof(tmp), "%d.%02d T",tlen, glen);
	}
	else if(glen > 0)
	{
		snprintf(tmp, sizeof(tmp), "%d.%02d G",glen, mlen);
	}
	else
		snprintf(tmp, sizeof(tmp), "%d M",disktatol1);

	if(wp->iValue > 0)
		websWrite(wp, "%s", ",");
	websWrite(wp, "{\"id\":%d, \"capacity\":\"%s\", \"use\":\"%s\", \"load\":\"%d%%\"}",
		2,
		tmp,
		os_file_size_string(diskuse1),
		diskload1);
	wp->iValue++;


	wp->iValue = 0;
	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}

static int web_board_product_setget(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if (strstr(strval, "GET"))
	{
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
		websWrite(wp,"{\"response\":\"%s\", \"keepday\":%d}", "OK", v9_video_disk_keep_day_get());
		websDone(wp);
		return OK;
	}
	if (strstr(strval, "SET"))
	{
		strval = webs_get_var(wp, T("keepday"), T(""));
		if (NULL == strval)
		{
			return web_return_text_plain(wp, ERROR);
		}
		if(v9_video_disk_keep_day_set(atoi(strval)) == OK)
		{
			return web_return_text_plain(wp, OK);
		}
	}
	return web_return_text_plain(wp, ERROR);
}


int web_boardcard_app(void)
{
	websFormDefine("boardcard-tbl", web_board_tbl);
	web_button_add_hook("boardcard", "detail", web_board_detail, NULL);
	websFormDefine("diskinfo", web_board_diskinfo_tbl);
	websFormDefine("product", web_board_product_setget);
	return 0;
}
//#endif

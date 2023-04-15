/*
 * zpl_media_log.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"

//zpl_uint32   _video_debug = 0xffffffff;

zpl_uint32 __video_debug[ ZPL_MEDIA_HARDADAP_MAX];

int zpl_media_debugmsg_init(void)
{
	zpl_uint32 i = 0;
	for(i = 0; i <  ZPL_MEDIA_HARDADAP_MAX; i++)
	{
		__video_debug[i] = 0xffffffff;
	}
	return 0;
}


int zpl_bufdata_detail_debug(zpl_uint8 *buf, zpl_uint32 len)
{
	zpl_uint32 i = 0;
	zpl_uint8 buftmp[1024];
	memset(buftmp, 0, sizeof(buftmp));
	for(i = 0; i < len; i++)
	{
		strcat(buftmp, itoa(buf[i], 16));
		strcat(buftmp, " ");
		if((i+1)%8 == 0)
			strcat(buftmp, "   ");
		if((i+1)%16 == 0)
		{
			zlog_debug(MODULE_MEDIA, "%s", buftmp);
			memset(buftmp, 0, sizeof(buftmp));
		}
	}
	return OK;
}
/*
 * zpl_media_resources.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_resources.h"

zpl_media_halres_t _halres;

zpl_uint32 zpl_video_halres_get_flag(int grp, int id, int type)
{
    zpl_uint32 res = 0;
    switch (type)
    {
    case ZPL_VIDHAL_INDEX_DEV:
        res = _halres.vdev_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_VIPIPE:
        res = _halres.vpipe_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_INPUTCHN:
        res = _halres.vchn_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_VPSSGRP:
        res = _halres.vpssgrp_halres[grp].flag;
        break;
    case ZPL_VIDHAL_INDEX_VPSSCHN:
        res = _halres.vpsschn_halres[grp][id].flag;
        break;
    case ZPL_VIDHAL_INDEX_VENCCHN:
        res = _halres.venc_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_VODEV:
        res = _halres.vdev_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_VOCHN:
        res = _halres.vdev_halres[id].flag;
        break;
    case ZPL_VIDHAL_INDEX_CAPTURE_VENCCHN:
        res = _halres.venc_halres[id].flag;
        break;
    default:
        break;
    }
    return res;
}

zpl_uint32 zpl_video_halres_set_flag(int grp, int id, int flag, int type)
{
    zpl_uint32 res;
    switch (type)
    {
    case ZPL_VIDHAL_INDEX_DEV:
        res = _halres.vdev_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VIPIPE:
        res = _halres.vpipe_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_INPUTCHN:
        res = _halres.vchn_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VPSSGRP:
        res = _halres.vpssgrp_halres[grp].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VPSSCHN:
        res = _halres.vpsschn_halres[grp][id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VENCCHN:
        res = _halres.venc_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VODEV:
        res = _halres.vdev_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_VOCHN:
        res = _halres.vdev_halres[id].flag = flag;
        break;
    case ZPL_VIDHAL_INDEX_CAPTURE_VENCCHN:
        res = _halres.venc_halres[id].flag = flag;
        break;
    default:
        break;
    }
    return res;
}

zpl_media_hwres_t * zpl_video_resources_get(int chn, int indx, int type)
{
    int i = 0;
    for(i = 0; i < ZPL_MEDIA_HALRES_NUM; i++)
    {
        if(_halres.halres[i].use && _halres.halres[i].channel == chn && _halres.halres[i].chnindex == indx)
        {
            if(type == -1)
                return &_halres.halres[i];
        }
    }
    return NULL;
}

static int zpl_media_hwres_json_read_obj(cJSON *obj, zpl_media_hwres_t *hwres)
{
	if (obj && hwres)
	{
		if (cJSON_GetObjectItem(obj, "channel")) 
			hwres->channel = cJSON_GetObjectItemIntValue(obj, "channel");
		if (cJSON_GetObjectItem(obj, "chnindex")) 
			hwres->chnindex = cJSON_GetObjectItemIntValue(obj, "chnindex");
		if (cJSON_GetObjectItem(obj, "vencchn")) 
			hwres->vencchn = cJSON_GetObjectItemIntValue(obj, "vencchn");
		if (cJSON_GetObjectItem(obj, "capvencchn")) 
			hwres->capvencchn = cJSON_GetObjectItemIntValue(obj, "capvencchn");
		if (cJSON_GetObjectItem(obj, "vpssgrp")) 
			hwres->vpssgrp = cJSON_GetObjectItemIntValue(obj, "vpssgrp");
		if (cJSON_GetObjectItem(obj, "vpsschn")) 
			hwres->vpsschn = cJSON_GetObjectItemIntValue(obj, "vpsschn");
		if (cJSON_GetObjectItem(obj, "inputchn")) 
			hwres->inputchn = cJSON_GetObjectItemIntValue(obj, "inputchn");
		if (cJSON_GetObjectItem(obj, "inputpipe")) 
			hwres->inputpipe = cJSON_GetObjectItemIntValue(obj, "inputpipe");
		if (cJSON_GetObjectItem(obj, "devid")) 
			hwres->devid = cJSON_GetObjectItemIntValue(obj, "devid");
		if (cJSON_GetObjectItem(obj, "mipidev")) 
			hwres->mipidev = cJSON_GetObjectItemIntValue(obj, "mipidev");
		if (cJSON_GetObjectItem(obj, "sensordev")) 
			hwres->sensordev = cJSON_GetObjectItemIntValue(obj, "sensordev");
		if (cJSON_GetObjectItem(obj, "sensortype")) 
			hwres->sensortype = cJSON_GetObjectItemIntValue(obj, "sensortype");
		if (cJSON_GetObjectItem(obj, "ispdev")) 
			hwres->ispdev = cJSON_GetObjectItemIntValue(obj, "ispdev");

		if (cJSON_GetObjectItem(obj, "inputchn_hwconnect")) 
			hwres->inputchn_hwconnect = cJSON_GetObjectItemBoolValue(obj, "inputchn_hwconnect");
		if (cJSON_GetObjectItem(obj, "vpsschn_hwconnect")) 
			hwres->vpsschn_hwconnect = cJSON_GetObjectItemBoolValue(obj, "vpsschn_hwconnect");
            
        zm_msg_debug("hwres load media channel :%d/%d", hwres->channel, hwres->chnindex);
        zm_msg_debug(" venc channel            :%d", hwres->vencchn);
        zm_msg_debug(" capture venc channel    :%d", hwres->capvencchn);
        zm_msg_debug(" vpss group              :%d", hwres->vpssgrp);
        zm_msg_debug(" vpss group channel      :%d", hwres->vpsschn);
        zm_msg_debug(" input channel           :%d", hwres->inputchn);
        zm_msg_debug(" input vpipe             :%d", hwres->inputpipe);
        zm_msg_debug(" input devid             :%d", hwres->devid);
        zm_msg_debug(" input mipidev           :%d", hwres->mipidev);
        zm_msg_debug(" input sensortype        :%d", hwres->sensortype);
        zm_msg_debug(" input sensordev         :%d", hwres->sensordev);
        zm_msg_debug(" input ispdev            :%d", hwres->ispdev);
        zm_msg_debug(" input hwbind            :%d", hwres->inputchn_hwconnect);
        zm_msg_debug(" vpss hwbind             :%d", hwres->vpsschn_hwconnect);
        
        hwres->use = 1;    
		return OK;
	}
	return ERROR;
}

static int zpl_media_hwres_json_load(cJSON *obj, zpl_media_hwres_t *hwres, int num)
{
	cJSON *tmpobjarray = NULL;
	cJSON *tmpobj_item = NULL;
	tmpobjarray = cJSON_GetObjectItem(obj, "hwres");
	if (tmpobjarray)
	{
		int i = 0, j = 0;
		int table_cnt = cJSON_GetArraySize(tmpobjarray);
		for (i = 0; i < table_cnt && i < 8; i++)
		{
			tmpobj_item = cJSON_GetArrayItem(tmpobjarray, i);
			if (tmpobj_item)
			{
				if (zpl_media_hwres_json_read_obj(tmpobj_item, &hwres[j++]) != OK)
					return ERROR;	
			}
		}
		return OK;
	}
	return ERROR;
}



int zpl_media_hwres_load(char *filename)
{
    cJSON *tmpobj = NULL;
    cJSON *pItem = NULL;
	int file_size = 0;
	char *buffer = NULL;
    memset(&_halres, 0, sizeof(zpl_media_halres_t));
	if (os_file_access(filename) != OK)
	{
		return ERROR;
	}
	file_size = (int)os_file_size(filename);
	buffer = (char *)malloc(file_size + 1);
	if(!buffer)
	{
		return ERROR;
	}
	memset(buffer, 0, file_size + 1);
	if(os_read_file(filename, buffer, file_size) != OK)
	{
		goto on_error;
	}
	tmpobj = cJSON_Parse(buffer);
	if(tmpobj)
	{
		
		pItem = cJSON_GetObjectItem (tmpobj, "mediahwres");
		if (pItem)
		{
			zpl_media_hwres_json_load(pItem, _halres.halres, ZPL_MEDIA_HALRES_NUM);
			cJSON_Delete(tmpobj);
		}
	}
	if (buffer != NULL)
		free(buffer);
	return OK;

on_error:
	if (buffer != NULL)
		free(buffer);
	return ERROR;
}

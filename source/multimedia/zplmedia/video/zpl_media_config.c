/*
 * zpl_media_config.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

#if 0
static int
zpl_video_region_config_json_read_obj(cJSON *obj, zpl_video_hwregion_t *region)
{
	if (obj && region)
	{
		/*
		if (cJSON_GetObjectItem(obj, "rgn_type")) 
			region->rgn_type = cJSON_GetObjectItemIntValue(obj, "rgn_type");
		if (cJSON_GetObjectItem(obj, "rgn_handle")) 
			region->rgn_handle = cJSON_GetObjectItemIntValue(obj, "rgn_handle");
		if (cJSON_GetObjectItem(obj, "rgn_chn")) 
			region->rgn_chn = cJSON_GetObjectItemIntValue(obj, "rgn_chn");

		if (cJSON_GetObjectItem(obj, "rgn_rect.x")) 
			region->rgn_rect.x = cJSON_GetObjectItemIntValue(obj, "rgn_rect.x");
		if (cJSON_GetObjectItem(obj, "rgn_rect.y")) 
			region->rgn_rect.y = cJSON_GetObjectItemIntValue(obj, "rgn_rect.y");
		if (cJSON_GetObjectItem(obj, "rgn_rect.width")) 
			region->rgn_rect.width = cJSON_GetObjectItemIntValue(obj, "rgn_rect.width");
		if (cJSON_GetObjectItem(obj, "rgn_rect.height")) 
			region->rgn_rect.height = cJSON_GetObjectItemIntValue(obj, "rgn_rect.height");
		if (cJSON_GetObjectItem(obj, "bg_color")) 
			region->bg_color = cJSON_GetObjectItemIntValue(obj, "bg_color");
		if (cJSON_GetObjectItem(obj, "fg_alpha")) 
			region->fg_alpha = cJSON_GetObjectItemIntValue(obj, "fg_alpha");
		if (cJSON_GetObjectItem(obj, "bg_alpha")) 
			region->bg_alpha = cJSON_GetObjectItemIntValue(obj, "bg_alpha");
		if (cJSON_GetObjectItem(obj, "rgn_layer")) 
			region->rgn_layer = cJSON_GetObjectItemIntValue(obj, "rgn_layer");
*/
		return OK;
	}
	return ERROR;
}

static int
zpl_video_region_config_json_write_obj(cJSON *obj, zpl_video_hwregion_t *region)
{
	if (obj && region)
	{
		/*
		cJSON_AddItemToObject(obj, "rgn_type", cJSON_CreateNumber(region->rgn_type));
		cJSON_AddItemToObject(obj, "rgn_handle", cJSON_CreateNumber(region->rgn_handle));
		cJSON_AddItemToObject(obj, "rgn_chn", cJSON_CreateNumber(region->rgn_chn));
		cJSON_AddItemToObject(obj, "rgn_rect.x", cJSON_CreateNumber(region->rgn_rect.x));
		cJSON_AddItemToObject(obj, "rgn_rect.y", cJSON_CreateNumber(region->rgn_rect.y));
		cJSON_AddItemToObject(obj, "rgn_rect.width", cJSON_CreateNumber(region->rgn_rect.width));
		cJSON_AddItemToObject(obj, "rgn_rect.height", cJSON_CreateNumber(region->rgn_rect.height));
		cJSON_AddItemToObject(obj, "bg_color", cJSON_CreateNumber(region->bg_color));
		cJSON_AddItemToObject(obj, "fg_alpha", cJSON_CreateNumber(region->fg_alpha));
		cJSON_AddItemToObject(obj, "bg_alpha", cJSON_CreateNumber(region->bg_alpha));
		cJSON_AddItemToObject(obj, "rgn_layer", cJSON_CreateNumber(region->rgn_layer));
*/
/*
		ZPL_JSON_WRITE_INT(region, obj, rgn_type);
		ZPL_JSON_WRITE_INT(region, obj, rgn_handle);
		ZPL_JSON_WRITE_INT(region, obj, rgn_chn);
		ZPL_JSON_WRITE_INT(region, obj, rgn_rect.x);
		ZPL_JSON_WRITE_INT(region, obj, rgn_rect.y);
		ZPL_JSON_WRITE_INT(region, obj, rgn_rect.width);
		ZPL_JSON_WRITE_INT(region, obj, rgn_rect.height);

		ZPL_JSON_WRITE_INT(region, obj, bg_color);
		ZPL_JSON_WRITE_INT(region, obj, fg_alpha);
		ZPL_JSON_WRITE_INT(region, obj, bg_alpha);
		ZPL_JSON_WRITE_INT(region, obj, rgn_layer);
*/
		return OK;
	}
	return ERROR;
}
#endif
static int
zpl_video_encode_config_json_read_obj(cJSON *obj, zpl_video_codec_t *encode)
{
	if (obj && encode)
	{
		/*
		ZPL_JSON_READ_INT(encode, obj, format);
		ZPL_JSON_READ_INT(encode, obj, vidsize.width);
		ZPL_JSON_READ_INT(encode, obj, vidsize.height);
		ZPL_JSON_READ_INT(encode, obj, codectype);

		ZPL_JSON_READ_INT(encode, obj, framerate);
		ZPL_JSON_READ_INT(encode, obj, bitrate);
		ZPL_JSON_READ_INT(encode, obj, profile);
		ZPL_JSON_READ_INT(encode, obj, bitrate_type);

		ZPL_JSON_READ_INT(encode, obj, ikey_rate);
		ZPL_JSON_READ_INT(encode, obj, enRcMode);
		ZPL_JSON_READ_INT(encode, obj, gopmode);
		*/
		if (cJSON_GetObjectItem(obj, "format")) 
			encode->format = cJSON_GetObjectItemIntValue(obj, "format");
		if (cJSON_GetObjectItem(obj, "vidsize.width")) 
			encode->vidsize.width = cJSON_GetObjectItemIntValue(obj, "vidsize.width");
		if (cJSON_GetObjectItem(obj, "vidsize.height")) 
			encode->vidsize.height = cJSON_GetObjectItemIntValue(obj, "vidsize.height");
		if (cJSON_GetObjectItem(obj, "codectype")) 
			encode->codectype = cJSON_GetObjectItemIntValue(obj, "codectype");
		if (cJSON_GetObjectItem(obj, "framerate")) 
			encode->framerate = cJSON_GetObjectItemIntValue(obj, "framerate");
		if (cJSON_GetObjectItem(obj, "bitrate")) 
			encode->bitrate = cJSON_GetObjectItemIntValue(obj, "bitrate");
		if (cJSON_GetObjectItem(obj, "profile")) 
			encode->profile = cJSON_GetObjectItemIntValue(obj, "profile");
		if (cJSON_GetObjectItem(obj, "bitrate_type")) 
			encode->bitrate_type = cJSON_GetObjectItemIntValue(obj, "bitrate_type");

		if (cJSON_GetObjectItem(obj, "ikey_rate")) 
			encode->ikey_rate = cJSON_GetObjectItemIntValue(obj, "ikey_rate");
		if (cJSON_GetObjectItem(obj, "enRcMode")) 
			encode->enRcMode = cJSON_GetObjectItemIntValue(obj, "enRcMode");
		if (cJSON_GetObjectItem(obj, "gopmode")) 
			encode->gopmode = cJSON_GetObjectItemIntValue(obj, "gopmode");

		return OK;
	}
	return ERROR;
}

static int
zpl_video_encode_config_json_write_obj(cJSON *obj, zpl_video_codec_t *encode)
{
	if (obj && encode)
	{
		/*ZPL_JSON_WRITE_INT(encode, obj, format);
		ZPL_JSON_WRITE_INT(encode, obj, vidsize.width);
		ZPL_JSON_WRITE_INT(encode, obj, vidsize.height);
		ZPL_JSON_WRITE_INT(encode, obj, codectype);

		ZPL_JSON_WRITE_INT(encode, obj, framerate);
		ZPL_JSON_WRITE_INT(encode, obj, bitrate);
		ZPL_JSON_WRITE_INT(encode, obj, profile);
		ZPL_JSON_WRITE_INT(encode, obj, bitrate_type);

		ZPL_JSON_WRITE_INT(encode, obj, ikey_rate);
		ZPL_JSON_WRITE_INT(encode, obj, enRcMode);
		ZPL_JSON_WRITE_INT(encode, obj, gopmode);*/
		cJSON_AddItemToObject(obj, "format", cJSON_CreateNumber(encode->format));
		cJSON_AddItemToObject(obj, "vidsize.width", cJSON_CreateNumber(encode->vidsize.width));
		cJSON_AddItemToObject(obj, "vidsize.height", cJSON_CreateNumber(encode->vidsize.height));
		cJSON_AddItemToObject(obj, "codectype", cJSON_CreateNumber(encode->codectype));
		cJSON_AddItemToObject(obj, "framerate", cJSON_CreateNumber(encode->framerate));
		cJSON_AddItemToObject(obj, "bitrate", cJSON_CreateNumber(encode->bitrate));
		cJSON_AddItemToObject(obj, "profile", cJSON_CreateNumber(encode->profile));
		cJSON_AddItemToObject(obj, "bitrate_type", cJSON_CreateNumber(encode->bitrate_type));
		cJSON_AddItemToObject(obj, "ikey_rate", cJSON_CreateNumber(encode->ikey_rate));
		cJSON_AddItemToObject(obj, "enRcMode", cJSON_CreateNumber(encode->enRcMode));
		cJSON_AddItemToObject(obj, "gopmode", cJSON_CreateNumber(encode->gopmode));
		return OK;
	}
	return ERROR;
}

static int
zpl_video_media_channel_config_json_read_obj(cJSON *obj, zpl_media_channel_t *ua)
{
	if (obj && ua)
	{
		/*
		ZPL_JSON_READ_INT(ua, obj, channel);
		ZPL_JSON_READ_INT(ua, obj, channel_index);
		ZPL_JSON_READ_INT(ua, obj, channel_type);
		*/
		if (cJSON_GetObjectItem(obj, "channel")) 
			ua->channel = cJSON_GetObjectItemIntValue(obj, "channel");
		if (cJSON_GetObjectItem(obj, "channel_index")) 
			ua->channel_index = cJSON_GetObjectItemIntValue(obj, "channel_index");
		if (cJSON_GetObjectItem(obj, "media_type")) 
			ua->media_type = cJSON_GetObjectItemIntValue(obj, "media_type");

		cJSON *tmpobj = cJSON_GetObjectItem(obj, "video_codec");
		if (tmpobj)
		{
			//if (ua->media_param.video_media.codec)
			{
				if (zpl_video_encode_config_json_read_obj(tmpobj, &ua->media_param.video_media.codec) != OK)
				{
					return ERROR;
				}
			}
		}
		tmpobj = cJSON_GetObjectItem(obj, "vdregion");
		if (tmpobj)
		{
			//if (zpl_video_region_config_json_read_obj(tmpobj, ua->vdregion) != OK)
			{
				return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

static int
zpl_video_media_channel_config_json_write_obj(cJSON *obj, zpl_media_channel_t *ua)
{
	if (obj && ua)
	{
		/*ZPL_JSON_WRITE_INT(ua, obj, channel);
		ZPL_JSON_WRITE_INT(ua, obj, channel_index);
		ZPL_JSON_WRITE_INT(ua, obj, channel_type);*/
		cJSON_AddItemToObject(obj, "channel", cJSON_CreateNumber(ua->channel));
		cJSON_AddItemToObject(obj, "channel_index", cJSON_CreateNumber(ua->channel_index));
		cJSON_AddItemToObject(obj, "media_type", cJSON_CreateNumber(ua->media_type));
		cJSON *tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			//if (ua->video_codec)
			{
				if (zpl_video_encode_config_json_write_obj(tmpobj, &ua->media_param.video_media.codec) != OK)
				{
					cJSON_Delete(tmpobj);
					return ERROR;
				}
				cJSON_AddItemToObject(obj, "video_codec", tmpobj);
	
				tmpobj = NULL;
			}
		}
		tmpobj = cJSON_CreateObject();
		if (tmpobj)
		{
			//if (ua->video_codec)
			{
				//if (zpl_video_region_config_json_write_obj(tmpobj, ua->vdregion) != OK)
				{
					cJSON_Delete(tmpobj);
					return ERROR;
				}
				cJSON_AddItemToObject(obj, "vdregion", tmpobj);
	
				tmpobj = NULL;
			}
		}
		return OK;
	}
	return ERROR;
}





static int zpl_video_media_channel_json_load(cJSON *obj)
{
	cJSON *tmpobjarray = NULL;
	cJSON *tmpobj_item = NULL;
	tmpobjarray = cJSON_GetObjectItem(obj, "channel");
	if (tmpobjarray)
	{
		int i = 0;
		int table_cnt = cJSON_GetArraySize(tmpobjarray);
		for (i = 0; i < table_cnt && i < 8; i++)
		{
			tmpobj_item = cJSON_GetArrayItem(tmpobjarray, i);
			if (tmpobj_item)
			{
				zpl_media_channel_t *t = os_malloc(sizeof(zpl_media_channel_t));
				//t->video_codec = os_malloc(sizeof(zpl_video_codec_t));
				if (zpl_video_media_channel_config_json_read_obj(tmpobj_item, t) != OK)
					return ERROR;
				zpl_media_channel_load(t);	
			}
		}
		return OK;
	}
	return ERROR;
}

static int zpl_video_media_channel_json_write_one(zpl_media_channel_t *t, cJSON *obj)
{
	cJSON *tmpobj_item = NULL;
	tmpobj_item = cJSON_CreateObject();
	if (tmpobj_item)
	{
		if (zpl_video_media_channel_config_json_write_obj(tmpobj_item,t) != OK)
		{
			cJSON_Delete(tmpobj_item);
			return ERROR;
		}
		cJSON_AddItemToArray(obj, tmpobj_item);
		tmpobj_item = NULL;
		return OK;
	}
	return ERROR;
}

static int zpl_video_media_channel_json_write(cJSON *obj)
{
	cJSON *tmpobjarray = NULL;
	tmpobjarray = cJSON_CreateArray();
	if (tmpobjarray)
	{
		cJSON_AddItemToObject(obj, "channel", tmpobjarray);
		zpl_media_channel_foreach(zpl_video_media_channel_json_write_one, tmpobjarray);
		return OK;
	}
	return ERROR;
}

int zpl_media_config_load(char *filename)
{
	if (os_file_access(filename) != OK)
	{
		printf("pjsip_config_load :%s is not exist.\r\n", filename);
		return ERROR;
	}
	int file_size = (int)os_file_size(filename);
	char *buffer = (char *)malloc(file_size + 1);
	if(!buffer)
	{
		//printf("pjsip_config_load : can not malloc buffer(%d byte)\r\n", file_size);
		return ERROR;
	}
	memset(buffer, 0, file_size + 1);
	if(os_read_file(filename, buffer, file_size) != OK)
	{
		//printf("pjsip_config_load : can not read buffer(%d byte)\r\n", file_size);
		goto on_error;
	}
	cJSON *tmpobj = cJSON_Parse(buffer);
	if(tmpobj)
	{
		cJSON *pItem = cJSON_GetObjectItem (tmpobj, "media-config");
		if (pItem)
		{
			if (zpl_video_media_channel_json_load(pItem) != OK)
			{
				cJSON_Delete(tmpobj);
				if (buffer != NULL)
					free(buffer);
				return ERROR;
			}
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

int zpl_media_config_write(char *filename)
{
	int wrsize = 0;
	cJSON *tmpobj = cJSON_CreateObject();
	cJSON* pRoot = cJSON_CreateObject();
	if (tmpobj && pRoot)
	{
		cJSON_AddItemToObject(pRoot, "media-config", tmpobj);

		if (zpl_video_media_channel_json_write(tmpobj) != OK)
		{
			cJSON_Delete(tmpobj);
			return ERROR;
		}

		char *szJSON = NULL; //cJSON_Print(tmpobj);
		//printf("pjsip_config_json_write_obj end\r\n");

		//printf("pjsip_config_write cJSON_Print\r\n");
		szJSON = cJSON_Print(pRoot);
		if (szJSON)
		{
			wrsize = strlen(szJSON);
			//printf("pjsip_config_write szJSON:%s\r\n", szJSON);

			if(os_write_file(filename, szJSON, wrsize) != OK)
			{
				cJSON_Delete(pRoot);
				cjson_free(szJSON);
				remove(filename);
				return ERROR;
			}
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
		return OK;
	}
	return ERROR;
}

/*
 * zpl_media_area.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"


static int zpl_media_channel_area_update_start(zpl_media_area_t * area, ZPL_MEDIA_OSD_TYPE_E type, zpl_bool bactive);

static zpl_media_area_t * _media_channel_areaget(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type)
{
    zpl_media_area_t * area = NULL;
	if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
	{
		int i = 0;
		for(i = 0; i< ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
		{
			if(chn->media_param.video_media.m_areas[i] && chn->media_param.video_media.m_areas[i]->areatype == type)
			{
				area = chn->media_param.video_media.m_areas[i];
				break;
			}
		}
	}	  
    return area;  
}

static int zpl_media_osd_bitrate_string(zpl_media_channel_t *chn, char *osdbuf)
{
    int n = 0;
    zpl_media_video_encode_t *encode = NULL;
    char *bitrate_typestr[] = {"none", "CBR", "VBR", "ABR"};
    char *enRcMode_typestr[] = {"CBR", "RC VBR", "AVBR", "QVBR", "CVBR", "QPMAP", "FIXQP"};
    char *gopmode_typestr[] = {"NORMALP", "DUALP", "SMARTP", "ADVSMARTP", "BIPREDB", "LOWDELAYB", "BUTT"};
    ZPL_MEDIA_CHANNEL_LOCK(chn);
    if (chn && chn->media_type == ZPL_MEDIA_VIDEO)
    {
        encode = (zpl_media_video_encode_t *)chn->media_param.video_media.halparam;
        if(encode)
        {
            //sprintf(osdbuf,  " size              : %dx%d%s", chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height, VTY_NEWLINE);
            n += sprintf(osdbuf + n,  " Codec %s %s fps %d\n", zpl_media_codec_name(chn->media_param.video_media.codec.codectype),
                zpl_media_format_name(chn->media_param.video_media.codec.format),
                chn->media_param.video_media.codec.framerate);
            n += sprintf(osdbuf + n,  " Bitrate %s %d kbps RcMode %s gop %s", bitrate_typestr[chn->media_param.video_media.codec.bitrate_type],
                chn->media_param.video_media.codec.bitrate,
                enRcMode_typestr[chn->media_param.video_media.codec.enRcMode],
                gopmode_typestr[chn->media_param.video_media.codec.gopmode]);
        }
        else
        {
            n += sprintf(osdbuf + n,  " Codec video.H265 1080P(1920*1080) fps 30\n");
            n += sprintf(osdbuf + n,  " Bitrate CBR 51200 kbps RcMode CBR gop ADVSMARTP"); 
        }
    }
    else if (chn->media_type == ZPL_MEDIA_AUDIO)
    {
        // sprintf(osdbuf,  "%-4d  %-4d %-6s %-8d %dx%d%s", chn->channel, chn->channel_index, media_typestr[chn->media_type],
        //		chn->media_param.video_media.halparam ? ((zpl_audio_encode_t *)chn->media_param.audio_media.halparam)->venc_channel : -1,
        //		chn->media_param.video_media.codec.vidsize.width, chn->media_param.video_media.codec.vidsize.height, VTY_NEWLINE);
    }
    ZPL_MEDIA_CHANNEL_LOCK(chn);
    return n;
}

static int zpl_media_area_osd_default(zpl_media_area_t *mareas, ZPL_MEDIA_OSD_TYPE_E osd, zpl_bool force)
{
    if (mareas)
    {
        int count = 0;
        char osdbuf[1024];
        zpl_time_t lt;
        memset(osdbuf, 0, sizeof(osdbuf));
        if (mareas->media_area.osd[osd].bactive || force)
        {
            switch (osd)
            {
            case ZPL_MEDIA_OSD_CHANNAL: // 通道信息
                if(mareas->mchn)
                    count = snprintf(osdbuf, sizeof(osdbuf), "channel %d/%s", mareas->mchn->channel, 
			                        (mareas->mchn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub");
                else
                    count = snprintf(osdbuf, sizeof(osdbuf), "channel 0/main");   
                break;
            case ZPL_MEDIA_OSD_DATETIME: //
                lt = os_time (&lt);
                count = snprintf(osdbuf, sizeof(osdbuf), "%s", os_time_fmt ("-", lt));
                break;
            case ZPL_MEDIA_OSD_BITRATE: // 码率信息
                count = zpl_media_osd_bitrate_string(mareas->mchn, osdbuf);
                //if(count)
                //    count = snprintf(osdbuf, sizeof(osdbuf), "%s", osdbuf);
                break;
            case ZPL_MEDIA_OSD_LABEL: // 标签信息
                if(mareas->media_area.osd[osd].m_keystr)
                    count = snprintf(osdbuf, sizeof(osdbuf), "%s", mareas->media_area.osd[osd].m_keystr);
                break;
            case ZPL_MEDIA_OSD_OTHER: // 其他信息
                if(mareas->media_area.osd[osd].m_keystr)
                    count = snprintf(osdbuf, sizeof(osdbuf), "%s", mareas->media_area.osd[osd].m_keystr);
                break;
            }
        }
        if (count)
        {
            zpl_media_text_show(mareas->media_area.osd[osd].m_text, zpl_false, OSD_RGB_BLACK, osdbuf);
            if (ZPL_MEDIA_DEBUG(REGION, DETAIL))
                zm_msg_debug("area osd default string:%s", osdbuf);
            if(mareas->media_area.osd[osd].m_point.x == 0 || mareas->media_area.osd[osd].m_point.y == 0)
            {
                if(osd > ZPL_MEDIA_OSD_CHANNAL)
                {
                    mareas->media_area.osd[osd].m_point.x = mareas->media_area.osd[osd-1].m_point.x;
                    mareas->media_area.osd[osd].m_point.y += mareas->media_area.osd[osd-1].m_point.y + 10 + mareas->media_area.osd[osd].m_bitmap.u32Height;
                }
                else
                {
                    mareas->media_area.osd[osd].m_point.x = 60;   
                    mareas->media_area.osd[osd].m_point.y = 60;    
                } 
            } 
            if (ZPL_MEDIA_DEBUG(REGION, DETAIL))
                zm_msg_debug("area osd default string rect [%d,%d %d,%d]", 
                    mareas->media_area.osd[osd].m_point.x, 
                    mareas->media_area.osd[osd].m_point.y, 
                    mareas->media_area.osd[osd].m_bitmap.u32Width,
                    mareas->media_area.osd[osd].m_bitmap.u32Height);            
        }
        return OK;
    }
    zm_msg_error("area osd load default keystring fail, area is null");
    return ERROR;
}


zpl_media_area_t *zpl_media_channel_area_create(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type)
{
    int i = 0;
    zpl_media_area_t *area = malloc(sizeof(zpl_media_area_t));
    if(area)
    {
        memset(area, 0, sizeof(zpl_media_area_t));
        area->areaid = (zpl_uint32)area;
        area->areatype = type;
        area->mchn = chn;
        if(area->areatype == ZPL_MEDIA_AREA_OSD)
        {
            for(i = ZPL_MEDIA_OSD_CHANNAL; i <= ZPL_MEDIA_OSD_OTHER; i++)
            {
                area->media_area.osd[i].m_text = zpl_media_text_create(&area->media_area.osd[i].m_bitmap, NULL); //OSD字符
                if(area->media_area.osd[i].m_text == NULL)
                {
                    zm_msg_error("area osd create text fail");
                    goto error_ret;
                }
                zpl_media_text_attr(area->media_area.osd[i].m_text, ZPL_FONT_SIZE_32X32, OSD_RGB_BLACK, OSD_RGB_WHITE, 2, 2);
                area->media_area.osd[i].bactive = zpl_false;
                area->media_area.osd[i].fg_alpha = OSD_FGALPHA_DEFAULT;
                area->media_area.osd[i].bg_alpha = OSD_BGALPHA_DEFAULT; 
                zpl_media_area_osd_default(area, i, zpl_true);
            }
        }
    }
    return area;
error_ret:
    for(i = ZPL_MEDIA_OSD_CHANNAL; i <= ZPL_MEDIA_OSD_OTHER; i++)
    {
        if(area->media_area.osd[i].m_text != NULL)
        {
            zpl_media_text_destroy(area->media_area.osd[i].m_text);
            area->media_area.osd[i].m_text = NULL;
        } 
        if(area->media_area.osd[i].m_hwregion)
        {
            zpl_media_video_hwregion_destroy(area->media_area.osd[i].m_hwregion);
            area->media_area.osd[i].m_hwregion = NULL;
        }
    }
    if(area)
    {
        free(area);
        area = NULL;
    }
    return area;
}

int zpl_media_channel_area_destroy(zpl_media_area_t * area)
{
    if(area)
    {
        int i = 0;
        if(area->areatype == ZPL_MEDIA_AREA_OSD)
        {
            for(i = ZPL_MEDIA_OSD_CHANNAL; i <= ZPL_MEDIA_OSD_OTHER; i++)
            {
                if(area->media_area.osd[i].m_text != NULL)
                {
                    zpl_media_text_destroy(area->media_area.osd[i].m_text);
                    area->media_area.osd[i].m_text = NULL;
                } 
                if(area->media_area.osd[i].m_hwregion)
                {
                    zpl_media_video_hwregion_destroy(area->media_area.osd[i].m_hwregion);
                    area->media_area.osd[i].m_hwregion = NULL;
                }
            }
        }
        free(area);
        area = NULL;
    }
    return OK;
}


static int zpl_media_area_osd_attachtochannel(zpl_media_area_t *area, int osd, zpl_media_channel_t *chn, zpl_bool attach)
{
    int ret = 0;
    zpl_rect_t m_rect;
    zpl_media_video_hwregion_t *hwregion = NULL;
    hwregion = area->media_area.osd[osd].m_hwregion;
    if(hwregion == NULL)
    {
        if( area->media_area.osd[osd].m_bitmap.u32Width == 0 || 
            area->media_area.osd[osd].m_bitmap.u32Height == 0 ||
            area->media_area.osd[osd].m_bitmap.pData == NULL)
        {
            zm_msg_error("area osd %d bitmap is null");
            return ERROR;
            /*if(area->media_area.osd[osd].m_bitmap.u32Width == 0)
                area->media_area.osd[osd].m_bitmap.u32Width = 100;
            if(area->media_area.osd[osd].m_bitmap.u32Height == 0)
                area->media_area.osd[osd].m_bitmap.u32Height = 32;*/
        }
        m_rect.x = area->media_area.osd[osd].m_point.x;
        m_rect.y = area->media_area.osd[osd].m_point.y;
        m_rect.width = area->media_area.osd[osd].m_bitmap.u32Width;
        m_rect.height = area->media_area.osd[osd].m_bitmap.u32Height;
        hwregion = zpl_media_video_hwregion_create(area, m_rect);     //区域硬件资源
        if(hwregion == NULL)
        {
            zm_msg_error("area osd hwregion create fail");
            return ERROR;
        }
        area->media_area.osd[osd].m_hwregion = hwregion;
        if(area->media_area.osd[osd].m_text)
	        hwregion->bg_color = area->media_area.osd[osd].m_text->m_font.m_bgcolor;		//区域背景颜色 
	    hwregion->fg_alpha = area->media_area.osd[osd].fg_alpha;       //透明度
	    hwregion->bg_alpha = area->media_area.osd[osd].bg_alpha;       //背景透明度
	    hwregion->rgn_layer = 0;      //层次[0-7]
    }
    if(area->media_area.osd[osd].m_text)
	    hwregion->bg_color = area->media_area.osd[osd].m_text->m_font.m_bgcolor;		//区域背景颜色 
	hwregion->fg_alpha = area->media_area.osd[osd].fg_alpha;       //透明度
	hwregion->bg_alpha = area->media_area.osd[osd].bg_alpha;       //背景透明度
	hwregion->rgn_layer = 0;      //层次[0-7]    

    m_rect.x = area->media_area.osd[osd].m_point.x;
    m_rect.y = area->media_area.osd[osd].m_point.y;
    m_rect.width = area->media_area.osd[osd].m_bitmap.u32Width;
    m_rect.height = area->media_area.osd[osd].m_bitmap.u32Height;

    zm_msg_debug("area osd attachtochannel rect [%d,%d %d,%d]", 
                area->media_area.osd[osd].m_point.x, 
                area->media_area.osd[osd].m_point.y, 
                area->media_area.osd[osd].m_bitmap.u32Width,
                area->media_area.osd[osd].m_bitmap.u32Height);

    ret = zpl_media_video_hwregion_attachtochannel(hwregion, chn, m_rect, attach);
    return ret;
}


int zpl_media_channel_area_osd_active(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E osd, zpl_bool bactive)
{
    int ret = ERROR;
    int dx = 0, dy = 0;
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area == NULL || osd < ZPL_MEDIA_OSD_CHANNAL || osd >= ZPL_MEDIA_OSD_MAX)
    {
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        zm_msg_error("area osd active fail, area is null or osd type %d invaled", osd);
        return ERROR;
    }   
    if(area && area->media_area.osd[osd].m_text && osd >= ZPL_MEDIA_OSD_CHANNAL && osd < ZPL_MEDIA_OSD_MAX)
    {
        if((osd == ZPL_MEDIA_OSD_LABEL || osd == ZPL_MEDIA_OSD_OTHER) && area->media_area.osd[osd].m_keystr == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            zm_msg_error("area osd label or other (%d) set attr keystr is null", osd);
            return ERROR;
        }
        area->media_area.osd[osd].bactive = bactive;
        ret = zpl_media_area_osd_attachtochannel(area,  osd, chn, bactive);
    }
    if(ret != OK)
    {
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        zm_msg_error("area osd active fail, can not attach to channel");
        return ERROR;
    }

    dx = area->media_area.osd[osd].m_point.x;
    dy = area->media_area.osd[osd].m_point.y;
    zpl_media_channel_area_update_start(area, osd, bactive);
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);   
    if(bactive)
        ret = zpl_media_channel_area_osd_show(chn, osd, dx, dy); 
    return ret;  
}




int zpl_media_channel_area_osd_attr(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E osd, zpl_uint32 pixel)
{
    int ret = ERROR;
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area == NULL || osd < ZPL_MEDIA_OSD_CHANNAL || osd >= ZPL_MEDIA_OSD_MAX)
    {
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        zm_msg_error("area osd set attr fail, area is null or osd type %d invaled", osd);
        return ERROR;
    }
    ret = zpl_media_text_attr(area->media_area.osd[osd].m_text,  pixel, area->media_area.osd[osd].m_text->m_font.m_fontcolor, 
        area->media_area.osd[osd].m_text->m_font.m_bgcolor, 
        area->media_area.osd[osd].m_text->m_font.m_fontsplit, 
        area->media_area.osd[osd].m_text->m_font.m_fontline);
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);    
    return ret;    
}




int zpl_media_channel_area_osd_show(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E osd, zpl_int32 x, zpl_int32 y)
{
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area == NULL || osd < ZPL_MEDIA_OSD_CHANNAL || osd >= ZPL_MEDIA_OSD_MAX)
    {
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        zm_msg_error("area osd set attr fail, area is null or osd type %d invaled", osd);
        return ERROR;
    }
    if(area && area->media_area.osd[osd].m_text && area->media_area.osd[osd].m_hwregion && area->media_area.osd[osd].bactive)
    {
        int ret = 0;
        zpl_rect_t m_rect;
        zpl_media_video_hwregion_t *hwregion = NULL;
        if((osd == ZPL_MEDIA_OSD_LABEL || osd == ZPL_MEDIA_OSD_OTHER) && area->media_area.osd[osd].m_keystr == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            zm_msg_error("area osd label or other (%d) set attr keystr is null", osd);
            return ERROR;
        }
        ret = zpl_media_area_osd_default(area, osd, zpl_false);
        if(ret != OK)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return ERROR;
        }
        if( area->media_area.osd[osd].m_bitmap.u32Width == 0 || 
            area->media_area.osd[osd].m_bitmap.u32Height == 0 ||
            area->media_area.osd[osd].m_bitmap.pData == NULL)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return ERROR;
        }
        m_rect.x = x;
        m_rect.y = y;
        m_rect.width = area->media_area.osd[osd].m_bitmap.u32Width;
        m_rect.height = area->media_area.osd[osd].m_bitmap.u32Height;
        hwregion = area->media_area.osd[osd].m_hwregion;
	    hwregion->bg_color = area->media_area.osd[osd].m_text->m_font.m_bgcolor;		//区域背景颜色 

	    hwregion->fg_alpha = area->media_area.osd[osd].fg_alpha;       //Alpha 位为 1 的像素点的透明度 0-128;越小越透明
	    hwregion->bg_alpha = area->media_area.osd[osd].bg_alpha;       //Alpha 位为 0 的像素点的透明度 0-128;越小越透明
	    
        hwregion->rgn_layer = 0;      //层次[0-7]

        if (ZPL_MEDIA_DEBUG(REGION, DETAIL))
            zm_msg_debug("area osd show rect [%d,%d %d,%d]", 
                m_rect.x, 
                m_rect.y, 
                m_rect.width,
                m_rect.height);

        ret = zpl_media_video_hwregion_set_bitmap(hwregion, m_rect, &area->media_area.osd[osd].m_bitmap);
        if(ret == OK)
        {
            ret = zpl_media_video_hwregion_show(hwregion, m_rect, zpl_true);
            if(ret != OK)
            {
                zm_msg_error("area osd hwregion show fail");    
            }
        }
        else
        {
            zm_msg_error("area osd set bitmap fail");
        }
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        return ret;
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
    return ERROR;
}


int zpl_media_channel_areaosd_keystr(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, char * keystr)
{
	int ret = ERROR;
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area)
    {
        if(area == NULL || type < ZPL_MEDIA_OSD_LABEL || type > ZPL_MEDIA_OSD_OTHER)
        {
            zm_msg_error("area osd set keystring fail, area is null or osd type %d invaled", type);
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return ERROR;
        }
        if(area && area->media_area.osd[type].m_text && type >= ZPL_MEDIA_OSD_LABEL && type <= ZPL_MEDIA_OSD_OTHER)
        {
            if(area->media_area.osd[type].m_keystr)
            {
                free(area->media_area.osd[type].m_keystr);
                area->media_area.osd[type].m_keystr = NULL;
            }
            if(keystr)
            {
                area->media_area.osd[type].m_keystr = strdup(keystr);
                zpl_media_area_osd_default(area, type, zpl_true);
                zpl_media_channel_area_osd_show(chn, type, area->media_area.osd[type].m_point.x, area->media_area.osd[type].m_point.y);
            }
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return ret;	
}


int zpl_media_channel_area_osd_alpha(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, int alpha, int fg_alpha)
{
	int ret = ERROR;
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area)
    {
        if(area == NULL || type < ZPL_MEDIA_OSD_CHANNAL || type >= ZPL_MEDIA_OSD_MAX)
        {
            zm_msg_error("area osd set alpha fail, area is null or osd type %d invaled", type);
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return ERROR;
        }
        if(area && area->media_area.osd[type].m_text && type >= ZPL_MEDIA_OSD_CHANNAL && type < ZPL_MEDIA_OSD_MAX)
        {
            area->media_area.osd[type].bg_alpha = alpha;
            area->media_area.osd[type].fg_alpha = fg_alpha;
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            if(area->media_area.osd[type].bactive)
                zpl_media_channel_area_osd_show(chn, type, area->media_area.osd[type].m_point.x, area->media_area.osd[type].m_point.y);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return ret;	
}

int zpl_media_channel_area_osd_point(zpl_media_channel_t *chn, ZPL_MEDIA_OSD_TYPE_E type, zpl_point_t m_point)
{
	int ret = ERROR;
    zpl_media_area_t * area = NULL;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
    area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
    if(area)
    {
        if(area == NULL || type < ZPL_MEDIA_OSD_CHANNAL || type >= ZPL_MEDIA_OSD_MAX)
        {
            zm_msg_error("area osd set point fail, area is null or osd type %d invaled", type);
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            return ERROR;
        }
        if(area && area->media_area.osd[type].m_text && type >= ZPL_MEDIA_OSD_CHANNAL && type < ZPL_MEDIA_OSD_MAX)
        {
            area->media_area.osd[type].m_point = m_point;
            ZPL_MEDIA_CHANNEL_UNLOCK(chn);
            if(area->media_area.osd[type].bactive)
                zpl_media_channel_area_osd_show(chn, type, area->media_area.osd[type].m_point.x, area->media_area.osd[type].m_point.y);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return ret;	
}

zpl_media_area_t * zpl_media_channel_area_lookup(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type)
{
	ZPL_MEDIA_CHANNEL_LOCK(chn);
	if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
	{
		int i = 0;
		for(i = 0; i< ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
		{
			if(chn->media_param.video_media.m_areas[i] && chn->media_param.video_media.m_areas[i]->areatype == type)
			{
				ZPL_MEDIA_CHANNEL_UNLOCK(chn);
				return chn->media_param.video_media.m_areas[i];
			}
		}
	}	
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return NULL;	
}

int zpl_media_channel_area_add(zpl_media_channel_t *chn, zpl_media_area_t *area)
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
	if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
	{
		int i = 0;
		for(i = 0; i< ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
		{
			if(chn->media_param.video_media.m_areas[i]  == NULL)
			{
				chn->media_param.video_media.m_areas[i] = area;
				ret = OK;
				break;
			}
		}
	}	
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return ret;	
}

int zpl_media_channel_area_del(zpl_media_channel_t *chn, ZPL_MEDIA_AREA_E type)
{
	int ret = ERROR;
	ZPL_MEDIA_CHANNEL_LOCK(chn);
	if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
	{
		int i = 0;
		for(i = 0; i< ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
		{
			if(chn->media_param.video_media.m_areas[i] && chn->media_param.video_media.m_areas[i]->areatype == type)
			{
				ret = OK;
				chn->media_param.video_media.m_areas[i] = NULL;
				break;
			}
		}
	}	
    ZPL_MEDIA_CHANNEL_UNLOCK(chn);
	return ret;	
}


int zpl_media_channel_area_destroy_all(void *chn)
{
    zpl_uint32 i = 0;
    zpl_media_channel_t *channel = chn;
    if(channel && channel->media_param.video_media.enable)
    {
        for(i = 0; i < ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
        {
            if(channel->media_param.video_media.m_areas[i])
            {
                zpl_media_channel_area_destroy(channel->media_param.video_media.m_areas[i]);
                channel->media_param.video_media.m_areas[i] = NULL;
            }
        }
    }
    return OK;
}

int zpl_media_channel_area_default(void *chn)
{
    zpl_media_channel_t *channel = chn;
    if(channel && channel->media_param.video_media.enable)
    {
        channel->media_param.video_media.m_areas[0] = zpl_media_channel_area_create(chn, ZPL_MEDIA_AREA_OSD);
        if(channel->media_param.video_media.m_areas[0])
        {
            zpl_media_channel_area_osd_active(channel, ZPL_MEDIA_OSD_CHANNAL, zpl_true);
            zpl_media_channel_area_osd_active(channel, ZPL_MEDIA_OSD_DATETIME, zpl_true);
            zpl_media_channel_area_osd_active(channel, ZPL_MEDIA_OSD_BITRATE, zpl_true);
            zpl_media_channel_area_osd_active(channel, ZPL_MEDIA_OSD_LABEL, zpl_true);
        }
        return OK;
    }
    return ERROR;
}

static int zpl_media_channel_area_update_thread(struct thread *e)
{
    zpl_media_channel_t *chn = THREAD_ARG(e);
    if(chn)
    {
        int dx = 0, dy = 0, bx = 0, by = 0, active = 0;
        zpl_media_area_t * area = NULL;
        struct thread_master *master = NULL;
	    ZPL_MEDIA_CHANNEL_LOCK(chn);
        area = _media_channel_areaget(chn, ZPL_MEDIA_AREA_OSD);
        if(area)
        {
            if(area->t_timer)
                master = ((struct thread *)area->t_timer)->master;
            area->t_timer = NULL;
            dx = area->media_area.osd[ZPL_MEDIA_OSD_DATETIME].m_point.x;
            dy = area->media_area.osd[ZPL_MEDIA_OSD_DATETIME].m_point.y;
            bx = area->media_area.osd[ZPL_MEDIA_OSD_BITRATE].m_point.x;
            by = area->media_area.osd[ZPL_MEDIA_OSD_BITRATE].m_point.y;
        }
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
        if(area)
        {
            if(area->media_area.osd[ZPL_MEDIA_OSD_DATETIME].bactive)
            {
                active = 1;
                zpl_media_channel_area_osd_show(chn, ZPL_MEDIA_OSD_DATETIME, dx, dy);
            }
            if(area->media_area.osd[ZPL_MEDIA_OSD_BITRATE].bactive)
            {
                active = 1;
                zpl_media_channel_area_osd_show(chn, ZPL_MEDIA_OSD_BITRATE, bx, by);
            }
            if(active && master)
                area->t_timer = thread_add_timer_msec(master, zpl_media_channel_area_update_thread, area->mchn, 1000);    
        }
    }
    return OK;
}

static int zpl_media_channel_area_update_start(zpl_media_area_t * area, ZPL_MEDIA_OSD_TYPE_E type, zpl_bool bactive)
{
    if (area)
	{
        if(area->t_timer)
            THREAD_TIMER_OFF(area->t_timer);
        if(bactive)    
		    area->t_timer = thread_add_timer_msec(_media_global.mthreadpool[area->mchn->channel].t_master, zpl_media_channel_area_update_thread, area->mchn, 1000);
        return OK;    
	}
    return ERROR;
}

/*
media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) (enable|disable)
media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) bgalpha <0-128> fgalpha <5-110>
media channel <0-1> (main|sub) osd (label|other) keystr .LINE
media channel <0-1> (main|sub) osd (channel|datetime|bitrate|label|other) x <1-4096> y <1-4096>
*/
int zpl_media_channel_area_config(zpl_media_channel_t *chn, struct vty *vty)
{
	if(chn)
	{
        char *osdtype[] = {"none", "channel","datetime","bitrate","label","other"};
        ZPL_MEDIA_CHANNEL_LOCK(chn);
        if(chn && chn->media_type == ZPL_MEDIA_VIDEO)
        {
            int j = 0, i = 0;
            zpl_media_area_t * area = NULL;
            for(j = 0; j< ZPL_MEDIA_AREA_CHANNEL_MAX; j++)
            {
                if(chn->media_param.video_media.m_areas[j] && chn->media_param.video_media.m_areas[j]->areatype == ZPL_MEDIA_AREA_OSD)
                {
                    area = chn->media_param.video_media.m_areas[j];
                    for(i = ZPL_MEDIA_OSD_CHANNAL; i <= ZPL_MEDIA_OSD_OTHER; i++)
                    {
                        if(area->media_area.osd[i].bactive)
                        {
                            if(i == ZPL_MEDIA_OSD_OTHER && area->media_area.osd[ZPL_MEDIA_OSD_OTHER].m_keystr)
                            {
		                        vty_out(vty, " media channel %d %s osd %s keystr %s %s", chn->channel, 
			                        (chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
                                    osdtype[i], area->media_area.osd[ZPL_MEDIA_OSD_OTHER].m_keystr, VTY_NEWLINE);
                            }
                            else if(i == ZPL_MEDIA_OSD_LABEL && area->media_area.osd[ZPL_MEDIA_OSD_LABEL].m_keystr)
                            {
		                        vty_out(vty, " media channel %d %s osd %s keystr %s %s", chn->channel, 
			                        (chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
                                    osdtype[i], area->media_area.osd[ZPL_MEDIA_OSD_LABEL].m_keystr, VTY_NEWLINE);
                            }
                            
		                    vty_out(vty, " media channel %d %s osd %s enable%s", chn->channel, 
			                        (chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", 
                                    osdtype[i], VTY_NEWLINE);

		                    vty_out(vty, " media channel %d %s osd %s x %d y %d%s", chn->channel, 
			                        (chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", osdtype[i], 
                                    area->media_area.osd[i].m_point.x,
                                    area->media_area.osd[i].m_point.y,
                                    VTY_NEWLINE);

		                    vty_out(vty, " media channel %d %s osd %s bgalpha %d fgalpha %d%s", chn->channel, 
			                        (chn->channel_index==ZPL_MEDIA_CHANNEL_TYPE_MAIN)?"main":"sub", osdtype[i], 
                                    area->media_area.osd[i].bg_alpha,area->media_area.osd[i].fg_alpha,
                                    VTY_NEWLINE);
                        } 
                    } 
                }
            }
        }	
        ZPL_MEDIA_CHANNEL_UNLOCK(chn);
    }
    return OK;
}








#include "zpl_media_bmp.h"

int zpl_media_text_bitmap_text(void)
{
    zpl_media_bitmap_t    m_bitmap;
    //FT_Init_FreeType(&_freetype_library); /* initialize library */
    //FT_New_Face(_freetype_library, "/home/zhurish/Downloads/tftpboot/fontawesome-webfont.ttf", 0, &_gl_freetype_face); 
    //zpl_media_text_t *text = zpl_media_text_create("/home/zhurish/Downloads/tftpboot/fontawesome-webfont.ttf" );
    //zpl_media_text_t *text = zpl_media_text_create("/home/zhurish/Downloads/tftpboot/FontAwesome.ttf" );
    zpl_media_text_t *text = zpl_media_text_create(&m_bitmap, "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf" );


    if(text)
    {
        zpl_media_text_attr(text, ZPL_FONT_SIZE_16X16, 0x0000, 0xffff, 2, 2);

        //zpl_media_text_auto_size(text, "ABC", ZPL_FONT_SIZE_16X16, &m_rect);
        zpl_media_text_show(text, zpl_false, 0x0000, "ABC\nde");

        UGL_BMP_ID *bmp = uglBitMapCreate("/home/zhurish/Downloads/tftpboot/abc.bmp", m_bitmap.u32Width, m_bitmap.u32Height, BMP_BITCOUNT_16);
        uglBitMapWrite(bmp, m_bitmap.pData, m_bitmap.u32len);//
        uglBitMapClose(bmp);//

        zpl_media_text_destroy(text);
    }
    return 0;
}
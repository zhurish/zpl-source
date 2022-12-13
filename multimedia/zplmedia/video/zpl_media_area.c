/*
 * zpl_media_area.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_area.h"
#include "zpl_media_channel.h"

zpl_media_area_t *zpl_media_area_create(ZPL_MEDIA_AREA_E type)
{
    zpl_media_area_t *area = malloc(sizeof(zpl_media_area_t));
    if(area)
    {
        memset(area, 0, sizeof(zpl_media_area_t));
        area->areaid = (zpl_uint32)area;
        area->areatype = type;
        area->b_rect = zpl_true;
        area->osd_type = ZPL_MEDIA_OSD_NONE;
        //area->media_area.m_rect;
        //area->media_area.m_multarea;
        if(area->areatype == ZPL_MEDIA_AREA_OSD)
        {
            area->m_text = zpl_media_text_bitmap_create(NULL); //OSD字符
            if(area->m_text == NULL)
            {
                free(area);
                area = NULL;
            }    
        }
        //area->m_hwregion;     //区域硬件资源
    }
    return area;
}

int zpl_media_area_destroy(zpl_media_area_t * area)
{
    if(area)
    {
        if(area->m_text)
        {
            zpl_media_text_bitmap_destroy(area->m_text);
            area->m_text = NULL;
        }
        if(area->m_hwregion)
        {
            zpl_media_text_bitmap_destroy(area->m_hwregion);
            area->m_hwregion = NULL;
        }
        free(area);
        area = NULL;
    }
    return OK;
}


int zpl_media_area_rectsize(zpl_media_area_t * area, zpl_rect_t rect)
{
    if(area)
    {
        area->b_rect = zpl_true;
        area->media_area.m_rect.x = rect.x;
        area->media_area.m_rect.y = rect.y;
        area->media_area.m_rect.width = rect.width;
        area->media_area.m_rect.height = rect.height;
        return OK;
    }
    return ERROR;
}


int zpl_media_area_multrectsize(zpl_media_area_t * area, zpl_multarea_t *rect)
{
    if(area)
    {
        area->b_rect = zpl_true;
        memcpy(&area->media_area.m_multarea, rect, sizeof(zpl_multarea_t));
        return OK;
    }
    return ERROR;
}


int zpl_media_area_osd_attr(zpl_media_area_t * area, ZPL_MEDIA_OSD_TYPE_E osd, zpl_uint32 bgcolor)
{
    if(area &&  area->m_text)
    {
        area->b_rect = zpl_true;
        area->osd_type = osd;
        return zpl_media_text_bitmap_attr(area->m_text, bgcolor);
    }
    return ERROR;
}

int zpl_media_area_osd_show(zpl_media_area_t * area, zpl_bool bshow, zpl_char *osdstring, zpl_uint32 pixel,
                               zpl_uint32 color, zpl_bool bold)
{
    if(area &&  area->m_text)
    {
        area->bactive = bshow;
        return zpl_media_text_bitmap_show(area->m_text, bshow, osdstring, pixel, color, bold);
    }
    return ERROR;
}



int zpl_media_area_active(zpl_media_area_t * area, zpl_bool bactive)
{
    if(area &&  area->m_text)
    {
        area->bactive = bactive;
        return OK;
    }
    return ERROR;
}


int zpl_media_area_destroy_all(void *chn)
{
    zpl_uint32 i = 0;
    zpl_media_channel_t *channel = chn;
    if(channel && channel->video_media.enable)
    {
        for(i = 0; i < ZPL_MEDIA_AREA_CHANNEL_MAX; i++)
        {
            if(channel->video_media.m_areas[i])
            {
                zpl_media_area_destroy(channel->video_media.m_areas[i]);
                channel->video_media.m_areas[i] = NULL;
            }
        }
    }
    return OK;
}

int zpl_media_area_channel_default(void *chn)
{
    zpl_media_channel_t *channel = chn;
    if(channel && channel->video_media.enable)
    {
        channel->video_media.m_areas[0] = zpl_media_area_create(ZPL_MEDIA_AREA_OSD);
        if(channel->video_media.m_areas[0])
        {
            zpl_media_text_bitmap_auto_size(channel->video_media.m_areas[0], "channel", 
                ZPL_FONT_SIZE_16X16, &channel->video_media.m_areas[0]->media_area.m_rect);
   
            zpl_media_area_osd_attr(channel->video_media.m_areas[0], ZPL_MEDIA_OSD_CHANNAL, 0xffff);
            zpl_media_area_osd_show(channel->video_media.m_areas[0], zpl_true, "channel", 16, 0x0000, zpl_true);
            //area->m_text->m_rect.width;
            //area->m_text->m_rect.height;
            //zpl_media_area_rectsize(channel->video_media.m_areas[0], channel->video_media.m_areas[0]->m_text->m_rect);
            zpl_media_area_active(channel->video_media.m_areas[0], zpl_true);
        }
        channel->video_media.m_areas[1] = zpl_media_area_create(ZPL_MEDIA_AREA_OSD);
        if(channel->video_media.m_areas[1])
        {
            zpl_media_text_bitmap_auto_size(channel->video_media.m_areas[1], "time:2021/10/19 11:13:33", 
                ZPL_FONT_SIZE_16X16, &channel->video_media.m_areas[1]->media_area.m_rect);
            //zpl_media_area_rectsize(channel->video_media.m_areas[1], channel->video_media.m_areas[0]->m_text->m_rect);
            zpl_media_area_osd_attr(channel->video_media.m_areas[1], ZPL_MEDIA_OSD_DATETIME, 0xffff);
            zpl_media_area_osd_show(channel->video_media.m_areas[1], zpl_true, "time:2021/10/19 11:13:33", 16, 0x0000, zpl_true);
            zpl_media_area_active(channel->video_media.m_areas[1], zpl_true);
        }
        channel->video_media.m_areas[2] = zpl_media_area_create(ZPL_MEDIA_AREA_OSD);
        if(channel->video_media.m_areas[2])
        {
            zpl_media_text_bitmap_auto_size(channel->video_media.m_areas[2], "channel", 
                ZPL_FONT_SIZE_16X16, &channel->video_media.m_areas[1]->media_area.m_rect);
            //zpl_media_area_rectsize(channel->video_media.m_areas[2], channel->video_media.m_areas[0]->m_text->m_rect);
            zpl_media_area_osd_attr(channel->video_media.m_areas[2], ZPL_MEDIA_OSD_BITRATE, 0xffff);
            zpl_media_area_osd_show(channel->video_media.m_areas[2], zpl_true, "channel", 16, 0x0000, zpl_true);
            zpl_media_area_active(channel->video_media.m_areas[2], zpl_true);
        }
        return OK;
    }
    return ERROR;
}
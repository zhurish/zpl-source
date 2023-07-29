/*
 * zpl_video_region.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"


#ifndef RGN_HANDLE_MAX
#define RGN_HANDLE_MAX 128
#endif

static int _lrng_handle_res[RGN_HANDLE_MAX];

static int zvid_rng_handle_res_get(void)
{
	int i = 0;
	for(i = 0; i < RGN_HANDLE_MAX; i++)
	{
		if(_lrng_handle_res[i] == 0)
		{
			_lrng_handle_res[i] = 1;
			return i;
		}
	}
	return -1;
}

static int zvid_rng_handle_res_free(int v)
{
	if(v >= 0 && v < RGN_HANDLE_MAX && _lrng_handle_res[v])
		_lrng_handle_res[v] = 0;
	return 0;
}

zpl_media_video_hwregion_t * zpl_media_video_hwregion_create(zpl_media_area_t *area, zpl_rect_t m_rect)
{
	zpl_media_video_hwregion_t *t = os_malloc(sizeof(zpl_media_video_hwregion_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_media_video_hwregion_t));
		switch(area->areatype)
		{
			case ZPL_MEDIA_AREA_MOSAIC:
			t->rgn_type = ZPL_MOSAIC_RGN;
			break;
			case ZPL_MEDIA_AREA_INTERESTED:
			t->rgn_type = ZPL_OVERLAY_RGN;
			break;
			case ZPL_MEDIA_AREA_OVERLAY:
			t->rgn_type = ZPL_OVERLAY_RGN;
			break;
			case ZPL_MEDIA_AREA_OSD:
			t->rgn_type = ZPL_OVERLAY_RGN;
			break;
			default:
			break;
		}
		t->rgn_handle = zvid_rng_handle_res_get();
		t->area = area;		    //
		//t->rgn_type;       //
		//t->rgn_handle;     //描述符
		//t->rgn_chn;        //通道号
		t->bg_color = 0xffff;		//区域背景颜色 
		t->fg_alpha = 100;       //透明度
		t->bg_alpha = 10;       //背景透明度
		t->rgn_layer = 0;      //层次[0-7]
		t->m_rect = m_rect; 
		if(zpl_vidhal_region_create(t) == 0)
			return t;
		else
		{
            zm_msg_error("media channel create region fail");			
			zvid_rng_handle_res_free(t->rgn_handle);
			free(t);
		}	
	}
	return NULL;
}


int zpl_media_video_hwregion_destroy(zpl_media_video_hwregion_t *hwregion)
{
	if(hwregion)
	{
		zpl_vidhal_region_destroy(hwregion);
		zvid_rng_handle_res_free(hwregion->rgn_handle);
		os_free(hwregion);	
	}
	return OK;
}

int zpl_media_video_hwregion_attachtochannel(zpl_media_video_hwregion_t *hwregion, zpl_media_channel_t *chn, zpl_rect_t m_rect, zpl_bool attach)
{
	int ret = 0;
	char *keystr = "encode";
	ZPL_MEDIA_CHANNEL_LOCK(chn);
	zpl_media_video_encode_t *video_encode = NULL;
	zpl_media_video_vpsschn_t *video_vpsschn = NULL;
	video_encode = chn->media_param.video_media.halparam;
	if (video_encode)
	{
		video_vpsschn = video_encode->source_input;
	}
	if (video_vpsschn == NULL || video_encode == NULL)
	{
		ZPL_MEDIA_CHANNEL_LOCK(chn);
		zm_msg_error("media channel bind to region fail, vpss or venc is null");	
		return ERROR;
	}
	switch (hwregion->rgn_type)
	{
	case ZPL_MOSAIC_RGN:
		#ifdef ZPL_HISIMPP_MODULE
		hwregion->rgn_chn.modId = HI_ID_VPSS;
		#endif
		hwregion->rgn_chn.devId = video_vpsschn->vpss_group;
		hwregion->rgn_chn.chnId = 0;
		keystr = "vpss group";
		break;
	case ZPL_OVERLAY_RGN:
	#ifdef ZPL_HISIMPP_MODULE
		hwregion->rgn_chn.modId = HI_ID_VENC;
		#endif
		hwregion->rgn_chn.devId = 0;
		hwregion->rgn_chn.chnId = video_encode->venc_channel;
		keystr = "encode channel";
		break;
	case ZPL_OVERLAYEX_RGN:
		#ifdef ZPL_HISIMPP_MODULE
		hwregion->rgn_chn.modId = HI_ID_VPSS;
		#endif
		hwregion->rgn_chn.devId = video_vpsschn->vpss_group;
		hwregion->rgn_chn.chnId = video_vpsschn->vpss_channel;
		keystr = "vpss channel";
		break;
	case ZPL_COVER_RGN:
		#ifdef ZPL_HISIMPP_MODULE
		hwregion->rgn_chn.modId = HI_ID_VPSS;
		#endif
		hwregion->rgn_chn.devId = video_vpsschn->vpss_group;
		hwregion->rgn_chn.chnId = 0;
		keystr = "vpss group";
		break;
	case ZPL_COVEREX_RGN:
		#ifdef ZPL_HISIMPP_MODULE
		hwregion->rgn_chn.modId = HI_ID_VPSS;
		#endif
		hwregion->rgn_chn.devId = video_vpsschn->vpss_group;
		hwregion->rgn_chn.chnId = video_vpsschn->vpss_channel;
		keystr = "vpss channel";
		break;
	default:
		break;
	}
	zm_msg_debug("media channel area region attach to %s %d/%d, rect %dx%d,xy %d/%d", keystr, hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId,
		m_rect.width, m_rect.height, m_rect.x, m_rect.y);	

	if(attach && (hwregion->m_rect.height != m_rect.height || hwregion->m_rect.width != m_rect.width))
	{
		hwregion->m_rect = m_rect;
		ret = zpl_vidhal_region_attachtochannel(hwregion, zpl_false);
		if(ret != OK)
		{
			zm_msg_error("media channel area unattach to %s %d/%d fail", keystr, hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			ZPL_MEDIA_CHANNEL_LOCK(chn);
			return ret;
		}
		ret = zpl_vidhal_region_update(hwregion);
		if(ret != OK)
		{
			zm_msg_error("media channel area rect update to %s %d/%d fail", keystr, hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			ZPL_MEDIA_CHANNEL_LOCK(chn);
			return ret;
		}
	}
	hwregion->m_rect = m_rect;
	ret = zpl_vidhal_region_attachtochannel(hwregion, attach);
	if(ret != OK)
	{
        zm_msg_error("media channel area attach to %s %d/%d fail", keystr, hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);			
	}
	ZPL_MEDIA_CHANNEL_LOCK(chn);
	return ret;
}

int zpl_media_video_hwregion_show(zpl_media_video_hwregion_t *hwregion, zpl_rect_t m_rect, zpl_bool show)
{
	int ret = ERROR;
	if((hwregion->m_rect.height != m_rect.height || hwregion->m_rect.width != m_rect.width))
	{
		hwregion->m_rect = m_rect;
		ret = zpl_vidhal_region_attachtochannel(hwregion, zpl_false);
		if(ret != OK)
		{
			zm_msg_error("media channel area unattach to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			return ret;
		}
		ret = zpl_vidhal_region_update(hwregion);
		if(ret != OK)
		{
			zm_msg_error("media channel area rect update to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			return ret;
		}
		ret = zpl_vidhal_region_attachtochannel(hwregion, zpl_true);
		if(ret != OK)
		{
			zm_msg_error("media channel area attach to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);			
		}
	}
	hwregion->m_rect = m_rect;
	return zpl_vidhal_region_channel_show(hwregion,  show);
}

int zpl_media_video_hwregion_set_bitmap(zpl_media_video_hwregion_t *hwregion, zpl_rect_t m_rect, zpl_media_bitmap_t *pstBitmap)
{
	if((hwregion->m_rect.height != m_rect.height || hwregion->m_rect.width != m_rect.width))
	{
		int ret = 0;
		hwregion->m_rect = m_rect;
		ret = zpl_vidhal_region_attachtochannel(hwregion, zpl_false);
		if(ret != OK)
		{
			zm_msg_error("media channel area unattach to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			return ret;
		}
		ret = zpl_vidhal_region_update(hwregion);
		if(ret != OK)
		{
			zm_msg_error("media channel area rect update to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);	
			return ret;
		}
		ret = zpl_vidhal_region_attachtochannel(hwregion, zpl_true);
		if(ret != OK)
		{
			zm_msg_error("media channel area attach to %d/%d fail", hwregion->rgn_chn.devId, hwregion->rgn_chn.chnId);			
		}
	}
	hwregion->m_rect = m_rect;
	return zpl_vidhal_region_set_bitmap(hwregion, pstBitmap);
}

int zpl_media_video_hwregion_update_canvas(zpl_media_video_hwregion_t *hwregion, zpl_rect_t m_rect)
{
	//zpl_media_video_hwregion_t *hwregion = area->media_area.osd[osd].m_hwregion;
	return zpl_vidhal_region_update_canvas(hwregion);
}



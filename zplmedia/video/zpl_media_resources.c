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



static zpl_video_resources_t _hal_resources[] = 
{
    /*channel channel_index*/
    {
        0,       0,     1920,   1080,
                 /*flag[]*/
                  {   1,    1,     1,  0x21,     1,     0x01,     0,    0},
                /*videv vipipe vichn vpssgrp vpsschn vencchn vodev vochn*/
        .halres = {   0,    0,     0,    0,     0,       0,     -1,   -1}
    },
    /*channel channel_index*/
    {
        0,       1,     1080,   720,
                 /*flag[]*/
                  {   0,    0,     0,   0x20,     1,    0x01,     0,    0},
                 /*videv vipipe vichn vpssgrp vpsschn vencchn vodev vochn*/
        .halres = {   0,    0,     0,    0,      1,      1,     -1,   -1}
    },
};




/*
int zpl_video_resources_init()
{
    zpl_uint32 i = 0;
    for(i = 0; i < array_size(_hal_resources); i++)
    {
        if(_hal_resources[i].channel  >= 0 && 
            _hal_resources[i].channel_index  >= 0)
        {
           zpl_media_channel_add(_hal_resources[i].channel, _hal_resources[i].channel_index);
        }
    }
    return 0;
}
*/



zpl_uint32 zpl_video_resources_get_flag(zpl_int32 channel, 
    zpl_int32 channel_index, ZPL_VIDHAL_INDEX_E index)
{
    zpl_uint32 i = 0;
    for(i = 0; i < array_size(_hal_resources); i++)
    {
        if(_hal_resources[i].channel == channel && 
            _hal_resources[i].channel_index == channel_index)
        {
            return (_hal_resources[i].flag[index]);
        }
    }
    return 0;
}

int zpl_video_resources_get(zpl_int32 channel, 
    zpl_int32 channel_index, ZPL_VIDHAL_INDEX_E index)
{
    zpl_uint32 i = 0;
    zpl_int32 res = -1;
    for(i = 0; i < array_size(_hal_resources); i++)
    {
        if(_hal_resources[i].channel == channel && 
            _hal_resources[i].channel_index == channel_index)
        {
            switch(index)
            {
            case ZPL_VIDHAL_INDEX_DEV:
                res = _hal_resources[i].halres.input_dev;
                break;    
            case ZPL_VIDHAL_INDEX_VIPIPE:
                res = _hal_resources[i].halres.input_pipe;
                break;     
            case ZPL_VIDHAL_INDEX_VICHN:
                res = _hal_resources[i].halres.input_chn;
                break;     
            case ZPL_VIDHAL_INDEX_VPSSGRP:
                res = _hal_resources[i].halres.vpss_group;
                break; 
            case ZPL_VIDHAL_INDEX_VPSSCHN:
                res = _hal_resources[i].halres.vpss_channel;
                break;     
            case ZPL_VIDHAL_INDEX_VENCCHN:
                res = _hal_resources[i].halres.venc_channel;
                break;     
            case ZPL_VIDHAL_INDEX_VODEV:
                res = _hal_resources[i].halres.hdmi_dev;
                break;      
            case ZPL_VIDHAL_INDEX_VOCHN:
                res = _hal_resources[i].halres.hdmi_chn;
                break; 
            default:
                break;
            }
            return res;
        }
    }
    return res;
}


int zpl_video_resources_get_pipe(zpl_int32 *vipipe)
{
    zpl_uint32 i = 0, j = 0;
    zpl_int32 tmppipe = -1;
    for(i = 0; i < array_size(_hal_resources); i++)
    {
        if(_hal_resources[i].halres.input_pipe >= 0)
        {
            if(tmppipe != _hal_resources[i].halres.input_pipe)
            {
                tmppipe = vipipe[j] = _hal_resources[i].halres.input_pipe;
                j++;
            }
        }
    }
    return j;
}

const char *zpl_video_resstring_get(ZPL_VIDEO_RES_FLAG_E e)
{
    static char buf[8];
    memset(buf, 0, sizeof(buf));
    if(VIDHAL_RES_FLAG_CHECK(e, CREATE))
        strcat(buf,"C");
    if(VIDHAL_RES_FLAG_CHECK(e, INIT))
        strcat(buf,"I");
    if(VIDHAL_RES_FLAG_CHECK(e, SRCBIND))
        strcat(buf,"S");
    if(VIDHAL_RES_FLAG_CHECK(e, DSTBIND))
        strcat(buf,"D");
    if(VIDHAL_RES_FLAG_CHECK(e, START))
        strcat(buf,"R");
    if(strlen(buf))
        return buf;
    return "N";        
}


int zpl_video_resources_show(void *p)
{
    zpl_uint32 i = 0;
    for(i = 0; i < array_size(_hal_resources); i++)
    {
        zpl_media_debugmsg_debug("channel(%d/%d) ", _hal_resources[i].channel, _hal_resources[i].channel_index);
        zpl_media_debugmsg_debug(" input_dev=%d input_pipe=%d ", _hal_resources[i].halres.input_dev, _hal_resources[i].halres.input_pipe);
        zpl_media_debugmsg_debug(" input_chn=%d vpss_group=%d ", _hal_resources[i].halres.input_chn, _hal_resources[i].halres.vpss_group);
        zpl_media_debugmsg_debug(" vpss_channel=%d venc_channel=%d ", _hal_resources[i].halres.vpss_channel, _hal_resources[i].halres.venc_channel);
        zpl_media_debugmsg_debug(" hdmi_dev=%d hdmi_chn=%d ", _hal_resources[i].halres.hdmi_dev, _hal_resources[i].halres.hdmi_chn);
    }
    return 0;
}
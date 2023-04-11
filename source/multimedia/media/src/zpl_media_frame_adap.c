#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "nal-h264.h"
#include "nal-hevc.h"



static struct zpl_media_frame_adap _frame_adap_tbl[] =
{
    {"H264", ZPL_VIDEO_CODEC_H264, nal_h264_get_frame, NULL},
    {"H265", ZPL_VIDEO_CODEC_H265, nal_h264_get_frame, NULL},
};


media_get_frame_hander * zpl_media_adap_get_frame_get(uint32_t id)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_frame_adap_tbl)/sizeof(_frame_adap_tbl[0]); i++)
    {
        if(_frame_adap_tbl[i].id == id)
        {
            return _frame_adap_tbl[i].get_frame;
        }
    }
    return NULL;
}

media_put_frame_hander * zpl_media_adap_put_frame_get(uint32_t id)
{
    uint32_t i = 0;
    for(i = 0; i < sizeof(_frame_adap_tbl)/sizeof(_frame_adap_tbl[0]); i++)
    {
        if(_frame_adap_tbl[i].id == id)
        {
            return _frame_adap_tbl[i].put_frame;
        }
    }
    return NULL;
}
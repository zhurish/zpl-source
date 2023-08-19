#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_syshal.h"


int zpl_media_system_bind(zpl_media_syschn_t src, zpl_media_syschn_t dst)
{
    int ret = 0;
    switch(src.modId)
    {
        case ZPL_MEDIA_GLOAL_VIDEO_DEV:
        switch(dst.modId)
        {
            
        }
        break;
    case ZPL_MEDIA_GLOAL_VIDEO_INPUT:
        break;    //输入
    case ZPL_MEDIA_GLOAL_VIDEO_VPSS:
        break;    //处理
    case ZPL_MEDIA_GLOAL_VIDEO_ENCODE:
        break;    //编码
    case ZPL_MEDIA_GLOAL_VIDEO_DECODE:
        break;    //解码
    case ZPL_MEDIA_GLOAL_VIDEO_OUTPUT:
        break;    //输出

    case ZPL_MEDIA_GLOAL_AUDIO:
        break;
    }
    return ret;
}

int zpl_media_system_unbind(zpl_media_syschn_t src, zpl_media_syschn_t dst)
{
    int ret = 0;
    return ret;
}

int zpl_media_system_init(void)
{
    zpl_syshal_vbmem_init();
    return OK;
}
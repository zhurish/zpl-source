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

static zpl_video_devres_t _hal_dev_resources[] = 
{
    //snsdev, mipmdev, snstype, enWDRMode, bMultiPipe, SnapPipe, bDoublePipe, s32BusId
    {   0,       0,       0,       0,         0,           0,        0,          0},
    {   1,       1,       0,       0,         0,           0,        0,          1},
};


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

zpl_uint32 zpl_video_devres_get(ZPL_MEDIA_CHANNEL_E channel, zpl_video_devres_t *info)
{
    if(channel == 0 || channel == 1)
    {
        memcpy(info, &_hal_dev_resources[channel], sizeof(zpl_video_devres_t));
        return OK;
    }
    return ERROR;
}



int zpl_video_resources_get_pipe(zpl_int32 *vipipe)
{
    vipipe[0] = 0;
    return 1;
}


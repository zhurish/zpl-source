/*
 * zpl_vidhal_venc.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_venc.h"

#ifdef ZPL_HISIMPP_MODULE
#include "zpl_hal_hisi.h"

static int zpl_vidhal_venc_CloseReEncode(zpl_int32 vencchn)
{
    zpl_int32 s32Ret;
    VENC_RC_PARAM_S stRcParam;
    VENC_CHN_ATTR_S stChnAttr;

    s32Ret = HI_MPI_VENC_GetChnAttr(vencchn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Get Attr failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_GetRcParam(vencchn, &stRcParam);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d)  Get RC Param failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    if (VENC_RC_MODE_H264CBR == stChnAttr.stRcAttr.enRcMode)
    {
        stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = 0;
    }
    else if (VENC_RC_MODE_H264VBR == stChnAttr.stRcAttr.enRcMode)
    {
        stRcParam.stParamH264Vbr.s32MaxReEncodeTimes = 0;
    }
    else if (VENC_RC_MODE_H265CBR == stChnAttr.stRcAttr.enRcMode)
    {
        stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = 0;
    }
    else if (VENC_RC_MODE_H265VBR == stChnAttr.stRcAttr.enRcMode)
    {
        stRcParam.stParamH264Vbr.s32MaxReEncodeTimes = 0;
    }
    else
    {
        return HI_SUCCESS;
    }
    s32Ret = HI_MPI_VENC_SetRcParam(vencchn, &stRcParam);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Set RC Param failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static int zpl_codec_build_hwgop_attr(ZPL_VENC_GOP_MODE_E enGopMode,VENC_GOP_ATTR_S *pstGopAttr)
{
    switch(enGopMode)
    {
        case ZPL_VENC_GOPMODE_NORMALP:
            pstGopAttr->enGopMode  = VENC_GOPMODE_NORMALP;
            pstGopAttr->stNormalP.s32IPQpDelta = 2;
        break;
        case ZPL_VENC_GOPMODE_SMARTP:
            pstGopAttr->enGopMode  = VENC_GOPMODE_SMARTP;
            pstGopAttr->stSmartP.s32BgQpDelta  = 4;
            pstGopAttr->stSmartP.s32ViQpDelta  = 2;
            pstGopAttr->stSmartP.u32BgInterval =  90;
        break;

        case ZPL_VENC_GOPMODE_DUALP:
            pstGopAttr->enGopMode  = VENC_GOPMODE_DUALP;
            pstGopAttr->stDualP.s32IPQpDelta  = 4;
            pstGopAttr->stDualP.s32SPQpDelta  = 2;
            pstGopAttr->stDualP.u32SPInterval = 3;
        break;

        case ZPL_VENC_GOPMODE_BIPREDB:
            pstGopAttr->enGopMode  = VENC_GOPMODE_BIPREDB;
            pstGopAttr->stBipredB.s32BQpDelta  = -2;
            pstGopAttr->stBipredB.s32IPQpDelta = 3;
            pstGopAttr->stBipredB.u32BFrmNum   = 2;
        break;

        default:
            if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                zpl_media_debugmsg_err("not support the gop mode !\n");
            return HI_FAILURE;
        break;
    }
    return HI_SUCCESS;
}

static int zpl_codec_build_hwh264(zpl_video_encode_t *venc, VENC_CHN_ATTR_S *stVencChnAttr, zpl_bool bRcnRefShareBuf)
{
    //zpl_int32 s32Ret;
    //VENC_CHN_ATTR_S stVencChnAttr;
    zpl_uint32 u32Gop = venc->pCodec->ikey_rate;
    zpl_uint32 u32StatTime;
    VENC_GOP_ATTR_S stGopAttr;
    zpl_codec_build_hwgop_attr(venc->pCodec->gopmode, &stGopAttr);
    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr->stVencAttr.enType = PT_H264;//venc->pCodec->enctype;
    stVencChnAttr->stVencAttr.u32MaxPicWidth = venc->pCodec->vidsize.width;
    stVencChnAttr->stVencAttr.u32MaxPicHeight = venc->pCodec->vidsize.height;
    stVencChnAttr->stVencAttr.u32PicWidth = venc->pCodec->vidsize.width;                                   /*the picture width*/
    stVencChnAttr->stVencAttr.u32PicHeight = venc->pCodec->vidsize.height;                                 /*the picture height*/
    stVencChnAttr->stVencAttr.u32BufSize = venc->pCodec->vidsize.width * venc->pCodec->vidsize.height * 2; /*stream buffer size*/
    stVencChnAttr->stVencAttr.u32Profile = venc->pCodec->profile;
    stVencChnAttr->stVencAttr.bByFrame = HI_TRUE; /*get stream mode is slice mode or frame mode?*/
    if (VENC_GOPMODE_SMARTP == stGopAttr.enGopMode)
    {
        u32StatTime = stGopAttr.stSmartP.u32BgInterval/u32Gop;
    }
    else
    {
        u32StatTime = 1;
    }
    if (ZPL_VENC_RC_CBR == venc->pCodec->enRcMode)
    {
        VENC_H264_CBR_S stH264Cbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        stH264Cbr.u32Gop = u32Gop;                            /*the interval of IFrame*/
        stH264Cbr.u32StatTime = u32StatTime;                  /* stream rate statics time(s) */
        stH264Cbr.u32SrcFrameRate = venc->pCodec->framerate;  /* input (vi) frame rate */
        stH264Cbr.fr32DstFrameRate = venc->pCodec->framerate; /* target frame rate */
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH264Cbr.u32BitRate = 1024 * 3 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH264Cbr.u32BitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH264Cbr.u32BitRate = 1024 * 4 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH264Cbr.u32BitRate = 1024 * 8 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH264Cbr.u32BitRate = 1024 * 12 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH264Cbr.u32BitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH264Cbr.u32BitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            break;
        }

        memcpy(&stVencChnAttr->stRcAttr.stH264Cbr, &stH264Cbr, sizeof(VENC_H264_CBR_S));
    }
    else if (ZPL_VENC_RC_FIXQP == venc->pCodec->enRcMode)
    {
        VENC_H264_FIXQP_S stH264FixQp;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
        stH264FixQp.u32Gop = 30;
        stH264FixQp.u32SrcFrameRate = venc->pCodec->framerate;
        stH264FixQp.fr32DstFrameRate = venc->pCodec->framerate;
        stH264FixQp.u32IQp = 25;
        stH264FixQp.u32PQp = 30;
        stH264FixQp.u32BQp = 32;
        memcpy(&stVencChnAttr->stRcAttr.stH264FixQp, &stH264FixQp, sizeof(VENC_H264_FIXQP_S));
    }
    else if (ZPL_VENC_RC_VBR == venc->pCodec->enRcMode)
    {
        VENC_H264_VBR_S stH264Vbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
        stH264Vbr.u32Gop = u32Gop;
        stH264Vbr.u32StatTime = u32StatTime;
        stH264Vbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH264Vbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_360P:
            stH264Vbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_720P:
            stH264Vbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH264Vbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH264Vbr.u32MaxBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH264Vbr.u32MaxBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH264Vbr.u32MaxBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH264Vbr.u32MaxBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH264Vbr.u32MaxBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH264Vbr, &stH264Vbr, sizeof(VENC_H264_VBR_S));
    }
    else if (ZPL_VENC_RC_AVBR == venc->pCodec->enRcMode)
    {
        VENC_H264_VBR_S stH264AVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
        stH264AVbr.u32Gop = u32Gop;
        stH264AVbr.u32StatTime = u32StatTime;
        stH264AVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH264AVbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_360P:
            stH264AVbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_720P:
            stH264AVbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH264AVbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH264AVbr.u32MaxBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH264AVbr.u32MaxBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH264AVbr.u32MaxBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH264AVbr.u32MaxBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH264AVbr.u32MaxBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH264AVbr, &stH264AVbr, sizeof(VENC_H264_AVBR_S));
    }
    else if (ZPL_VENC_RC_QVBR == venc->pCodec->enRcMode)
    {
        VENC_H264_QVBR_S stH264QVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QVBR;
        stH264QVbr.u32Gop = u32Gop;
        stH264QVbr.u32StatTime = u32StatTime;
        stH264QVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH264QVbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_360P:
            stH264QVbr.u32TargetBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_720P:
            stH264QVbr.u32TargetBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH264QVbr.u32TargetBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH264QVbr.u32TargetBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH264QVbr.u32TargetBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH264QVbr.u32TargetBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH264QVbr.u32TargetBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH264QVbr.u32TargetBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH264QVbr, &stH264QVbr, sizeof(VENC_H264_QVBR_S));
    }
    else if (ZPL_VENC_RC_CVBR == venc->pCodec->enRcMode)
    {
        VENC_H264_CVBR_S stH264CVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CVBR;
        stH264CVbr.u32Gop = u32Gop;
        stH264CVbr.u32StatTime = u32StatTime;
        stH264CVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH264CVbr.fr32DstFrameRate = venc->pCodec->framerate;
        stH264CVbr.u32LongTermStatTime = 1;
        stH264CVbr.u32ShortTermStatTime = u32StatTime;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH264CVbr.u32MaxBitRate = 1024 * 3 + 1024 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 512;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH264CVbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH264CVbr.u32MaxBitRate = 1024 * 4 + 3072 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024 * 2;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH264CVbr.u32MaxBitRate = 1024 * 8 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024 * 3;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH264CVbr.u32MaxBitRate = 1024 * 12 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024 * 4;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH264CVbr.u32MaxBitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024 * 6;
            break;
        default:
            stH264CVbr.u32MaxBitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMaxBitrate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            stH264CVbr.u32LongTermMinBitrate = 1024 * 5;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH264CVbr, &stH264CVbr, sizeof(VENC_H264_CVBR_S));
    }
    else if (ZPL_VENC_RC_QPMAP == venc->pCodec->enRcMode)
    {
        VENC_H264_QPMAP_S stH264QpMap;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QPMAP;
        stH264QpMap.u32Gop = u32Gop;
        stH264QpMap.u32StatTime = u32StatTime;
        stH264QpMap.u32SrcFrameRate = venc->pCodec->framerate;
        stH264QpMap.fr32DstFrameRate = venc->pCodec->framerate;
        memcpy(&stVencChnAttr->stRcAttr.stH264QpMap, &stH264QpMap, sizeof(VENC_H264_QPMAP_S));
    }
    else
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err("%s,%d,enRcMode(%d) not support\n", __FUNCTION__, __LINE__, venc->pCodec->enRcMode);
        return HI_FAILURE;
    }
    stVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = bRcnRefShareBuf;
	memcpy(&stVencChnAttr->stGopAttr, &stGopAttr, sizeof(VENC_GOP_ATTR_S));
    return HI_SUCCESS;
}
static int zpl_codec_build_hwh265(zpl_video_encode_t *venc, VENC_CHN_ATTR_S *stVencChnAttr, zpl_bool bRcnRefShareBuf)
{
    //zpl_int32 s32Ret;
    //VENC_CHN_ATTR_S stVencChnAttr;
    zpl_uint32 u32Gop = venc->pCodec->ikey_rate;
    zpl_uint32 u32StatTime;
    VENC_GOP_ATTR_S stGopAttr;
    zpl_codec_build_hwgop_attr(venc->pCodec->gopmode, &stGopAttr);
    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr->stVencAttr.enType = PT_H265;//venc->pCodec->enctype;
    stVencChnAttr->stVencAttr.u32MaxPicWidth = venc->pCodec->vidsize.width;
    stVencChnAttr->stVencAttr.u32MaxPicHeight = venc->pCodec->vidsize.height;
    stVencChnAttr->stVencAttr.u32PicWidth = venc->pCodec->vidsize.width;                                   /*the picture width*/
    stVencChnAttr->stVencAttr.u32PicHeight = venc->pCodec->vidsize.height;                                 /*the picture height*/
    stVencChnAttr->stVencAttr.u32BufSize = venc->pCodec->vidsize.width * venc->pCodec->vidsize.height * 2; /*stream buffer size*/
    stVencChnAttr->stVencAttr.u32Profile = venc->pCodec->profile;
    stVencChnAttr->stVencAttr.bByFrame = HI_TRUE; /*get stream mode is slice mode or frame mode?*/
    if (VENC_GOPMODE_SMARTP == stGopAttr.enGopMode)
    {
        u32StatTime = stGopAttr.stSmartP.u32BgInterval/u32Gop;
    }
    else
    {
        u32StatTime = 1;
    }
    if (ZPL_VENC_RC_CBR == venc->pCodec->enRcMode)
    {
        VENC_H265_CBR_S stH265Cbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
        stH265Cbr.u32Gop = u32Gop;
        stH265Cbr.u32StatTime = u32StatTime;                  /* stream rate statics time(s) */
        stH265Cbr.u32SrcFrameRate = venc->pCodec->framerate;  /* input (vi) frame rate */
        stH265Cbr.fr32DstFrameRate = venc->pCodec->framerate; /* target frame rate */
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH265Cbr.u32BitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH265Cbr.u32BitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH265Cbr.u32BitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH265Cbr.u32BitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH265Cbr.u32BitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH265Cbr.u32BitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH265Cbr.u32BitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH265Cbr, &stH265Cbr, sizeof(VENC_H265_CBR_S));
    }
    else if (ZPL_VENC_RC_FIXQP == venc->pCodec->enRcMode)
    {
        VENC_H265_FIXQP_S stH265FixQp;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
        stH265FixQp.u32Gop = 30;
        stH265FixQp.u32SrcFrameRate = venc->pCodec->framerate;
        stH265FixQp.fr32DstFrameRate = venc->pCodec->framerate;
        stH265FixQp.u32IQp = 25;
        stH265FixQp.u32PQp = 30;
        stH265FixQp.u32BQp = 32;
        memcpy(&stVencChnAttr->stRcAttr.stH265FixQp, &stH265FixQp, sizeof(VENC_H265_FIXQP_S));
    }
    else if (ZPL_VENC_RC_VBR == venc->pCodec->enRcMode)
    {
        VENC_H265_VBR_S stH265Vbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
        stH265Vbr.u32Gop = u32Gop;
        stH265Vbr.u32StatTime = u32StatTime;
        stH265Vbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH265Vbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH265Vbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH265Vbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH265Vbr.u32MaxBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH265Vbr.u32MaxBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH265Vbr.u32MaxBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH265Vbr.u32MaxBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH265Vbr.u32MaxBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH265Vbr, &stH265Vbr, sizeof(VENC_H265_VBR_S));
    }
    else if (ZPL_VENC_RC_AVBR == venc->pCodec->enRcMode)
    {
        VENC_H265_AVBR_S stH265AVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;
        stH265AVbr.u32Gop = u32Gop;
        stH265AVbr.u32StatTime = u32StatTime;
        stH265AVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH265AVbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH265AVbr.u32MaxBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH265AVbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH265AVbr.u32MaxBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH265AVbr.u32MaxBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH265AVbr.u32MaxBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH265AVbr.u32MaxBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH265AVbr.u32MaxBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH265AVbr, &stH265AVbr, sizeof(VENC_H265_AVBR_S));
    }
    else if (ZPL_VENC_RC_QVBR == venc->pCodec->enRcMode)
    {
        VENC_H265_QVBR_S stH265QVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QVBR;
        stH265QVbr.u32Gop = u32Gop;
        stH265QVbr.u32StatTime = u32StatTime;
        stH265QVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH265QVbr.fr32DstFrameRate = venc->pCodec->framerate;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH265QVbr.u32TargetBitRate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH265QVbr.u32TargetBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH265QVbr.u32TargetBitRate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH265QVbr.u32TargetBitRate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH265QVbr.u32TargetBitRate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH265QVbr.u32TargetBitRate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            break;
        default:
            stH265QVbr.u32TargetBitRate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH265QVbr, &stH265QVbr, sizeof(VENC_H265_QVBR_S));
    }
    else if (ZPL_VENC_RC_CVBR == venc->pCodec->enRcMode)
    {
        VENC_H265_CVBR_S stH265CVbr;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CVBR;
        stH265CVbr.u32Gop = u32Gop;
        stH265CVbr.u32StatTime = u32StatTime;
        stH265CVbr.u32SrcFrameRate = venc->pCodec->framerate;
        stH265CVbr.fr32DstFrameRate = venc->pCodec->framerate;
        stH265CVbr.u32LongTermStatTime = 1;
        stH265CVbr.u32ShortTermStatTime = u32StatTime;
        switch (venc->pCodec->format)
        {
        case ZPL_VIDEO_FORMAT_720P:
            stH265CVbr.u32MaxBitRate = 1024 * 3 + 1024 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 2 + 1024 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 512;
            break;
        case ZPL_VIDEO_FORMAT_1080P:
            stH265CVbr.u32MaxBitRate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 2 + 2048 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024;
            break;
        case ZPL_VIDEO_FORMAT_2592x1944:
            stH265CVbr.u32MaxBitRate = 1024 * 4 + 3072 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 3 + 3072 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024 * 2;
            break;
        case ZPL_VIDEO_FORMAT_3840x2160:
            stH265CVbr.u32MaxBitRate = 1024 * 8 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 5 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024 * 3;
            break;
        case ZPL_VIDEO_FORMAT_4000x3000:
            stH265CVbr.u32MaxBitRate = 1024 * 12 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 10 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024 * 4;
            break;
        case ZPL_VIDEO_FORMAT_7680x4320:
            stH265CVbr.u32MaxBitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 20 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024 * 6;
            break;
        default:
            stH265CVbr.u32MaxBitRate = 1024 * 24 + 5120 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMaxBitrate = 1024 * 15 + 2048 * venc->pCodec->framerate / 30;
            stH265CVbr.u32LongTermMinBitrate = 1024 * 5;
            break;
        }
        memcpy(&stVencChnAttr->stRcAttr.stH265CVbr, &stH265CVbr, sizeof(VENC_H265_CVBR_S));
    }
    else if (ZPL_VENC_RC_QPMAP == venc->pCodec->enRcMode)
    {
        VENC_H265_QPMAP_S stH265QpMap;

        stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QPMAP;
        stH265QpMap.u32Gop = u32Gop;
        stH265QpMap.u32StatTime = u32StatTime;
        stH265QpMap.u32SrcFrameRate = venc->pCodec->framerate;
        stH265QpMap.fr32DstFrameRate = venc->pCodec->framerate;
        stH265QpMap.enQpMapMode = VENC_RC_QPMAP_MODE_MEANQP;
        memcpy(&stVencChnAttr->stRcAttr.stH265QpMap, &stH265QpMap, sizeof(VENC_H265_QPMAP_S));
    }
    else
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err("%s,%d,enRcMode(%d) not support\n", __FUNCTION__, __LINE__, venc->pCodec->enRcMode);
        return HI_FAILURE;
    }
    stVencChnAttr->stVencAttr.stAttrH265e.bRcnRefShareBuf = bRcnRefShareBuf;
	memcpy(&stVencChnAttr->stGopAttr,&stGopAttr, sizeof(VENC_GOP_ATTR_S));
    return HI_SUCCESS;
}

static int zpl_codec_build_hwmjpeg(zpl_video_encode_t *venc, PAYLOAD_TYPE_E enType, 
    VENC_CHN_ATTR_S *stVencChnAttr, zpl_bool bRcnRefShareBuf)
{
    //zpl_int32 s32Ret;
    //VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_JPEG_S stJpegAttr;
    zpl_uint32 u32StatTime;
    zpl_uint32 u32Gop = venc->pCodec->ikey_rate;
    VENC_GOP_ATTR_S stGopAttr;
    zpl_codec_build_hwgop_attr(venc->pCodec->gopmode, &stGopAttr);
    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr->stVencAttr.enType = venc->pCodec->enctype;
    stVencChnAttr->stVencAttr.u32MaxPicWidth = venc->pCodec->vidsize.width;
    stVencChnAttr->stVencAttr.u32MaxPicHeight = venc->pCodec->vidsize.height;
    stVencChnAttr->stVencAttr.u32PicWidth = venc->pCodec->vidsize.width;                                   /*the picture width*/
    stVencChnAttr->stVencAttr.u32PicHeight = venc->pCodec->vidsize.height;                                 /*the picture height*/
    stVencChnAttr->stVencAttr.u32BufSize = venc->pCodec->vidsize.width * venc->pCodec->vidsize.height * 2; /*stream buffer size*/
    stVencChnAttr->stVencAttr.u32Profile = venc->pCodec->profile;
    stVencChnAttr->stVencAttr.bByFrame = HI_TRUE; /*get stream mode is slice mode or frame mode?*/

    if (VENC_GOPMODE_SMARTP == stGopAttr.enGopMode)
    {
        u32StatTime = stGopAttr.stSmartP.u32BgInterval / u32Gop;
    }
    else
    {
        u32StatTime = 1;
    }

    if (enType == PT_MJPEG)
    {
        if (ZPL_VENC_RC_FIXQP == venc->pCodec->enRcMode)
        {
            VENC_MJPEG_FIXQP_S stMjpegeFixQp;

            stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
            stMjpegeFixQp.u32Qfactor = 95;
            stMjpegeFixQp.u32SrcFrameRate = venc->pCodec->framerate;
            stMjpegeFixQp.fr32DstFrameRate = venc->pCodec->framerate;

            memcpy(&stVencChnAttr->stRcAttr.stMjpegFixQp, &stMjpegeFixQp, sizeof(VENC_MJPEG_FIXQP_S));
        }
        else if (ZPL_VENC_RC_CBR == venc->pCodec->enRcMode)
        {
            VENC_MJPEG_CBR_S stMjpegeCbr;

            stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
            stMjpegeCbr.u32StatTime = u32StatTime;
            stMjpegeCbr.u32SrcFrameRate = venc->pCodec->framerate;
            stMjpegeCbr.fr32DstFrameRate = venc->pCodec->framerate;
            switch (venc->pCodec->format)
            {
            case ZPL_VIDEO_FORMAT_360P:
                stMjpegeCbr.u32BitRate = 1024 * 3 + 1024 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_720P:
                stMjpegeCbr.u32BitRate = 1024 * 5 + 1024 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_1080P:
                stMjpegeCbr.u32BitRate = 1024 * 8 + 2048 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_2592x1944:
                stMjpegeCbr.u32BitRate = 1024 * 20 + 3072 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_3840x2160:
                stMjpegeCbr.u32BitRate = 1024 * 25 + 5120 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_4000x3000:
                stMjpegeCbr.u32BitRate = 1024 * 30 + 5120 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_7680x4320:
                stMjpegeCbr.u32BitRate = 1024 * 40 + 5120 * venc->pCodec->framerate / 30;
                break;
            default:
                stMjpegeCbr.u32BitRate = 1024 * 20 + 2048 * venc->pCodec->framerate / 30;
                break;
            }

            memcpy(&stVencChnAttr->stRcAttr.stMjpegCbr, &stMjpegeCbr, sizeof(VENC_MJPEG_CBR_S));
        }
        else if ((ZPL_VENC_RC_VBR == venc->pCodec->enRcMode) || (ZPL_VENC_RC_AVBR == venc->pCodec->enRcMode) ||
                 (ZPL_VENC_RC_QVBR == venc->pCodec->enRcMode) || (ZPL_VENC_RC_CVBR == venc->pCodec->enRcMode))
        {
            VENC_MJPEG_VBR_S stMjpegVbr;

            if (ZPL_VENC_RC_AVBR == venc->pCodec->enRcMode)
            {
                if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                    zpl_media_debugmsg_err("Mjpege not support AVBR, so change rcmode to VBR!\n");
            }

            stVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
            stMjpegVbr.u32StatTime = u32StatTime;
            stMjpegVbr.u32SrcFrameRate = venc->pCodec->framerate;
            stMjpegVbr.fr32DstFrameRate = 5;

            switch (venc->pCodec->format)
            {
            case ZPL_VIDEO_FORMAT_360P:
                stMjpegVbr.u32MaxBitRate = 1024 * 3 + 1024 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_720P:
                stMjpegVbr.u32MaxBitRate = 1024 * 5 + 1024 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_1080P:
                stMjpegVbr.u32MaxBitRate = 1024 * 8 + 2048 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_2592x1944:
                stMjpegVbr.u32MaxBitRate = 1024 * 20 + 3072 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_3840x2160:
                stMjpegVbr.u32MaxBitRate = 1024 * 25 + 5120 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_4000x3000:
                stMjpegVbr.u32MaxBitRate = 1024 * 30 + 5120 * venc->pCodec->framerate / 30;
                break;
            case ZPL_VIDEO_FORMAT_7680x4320:
                stMjpegVbr.u32MaxBitRate = 1024 * 40 + 5120 * venc->pCodec->framerate / 30;
                break;
            default:
                stMjpegVbr.u32MaxBitRate = 1024 * 20 + 2048 * venc->pCodec->framerate / 30;
                break;
            }

            memcpy(&stVencChnAttr->stRcAttr.stMjpegVbr, &stMjpegVbr, sizeof(VENC_MJPEG_VBR_S));
        }
        else
        {
            if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                zpl_media_debugmsg_err("cann't support other mode(%d) in this version!\n", venc->pCodec->enRcMode);
            return HI_FAILURE;
        }
    }
    else if (enType == PT_JPEG)
    {
        stJpegAttr.bSupportDCF = HI_FALSE;
        stJpegAttr.stMPFCfg.u8LargeThumbNailNum = 0;
        stJpegAttr.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
        memcpy(&stVencChnAttr->stVencAttr.stAttrJpege, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));
    }

    if (PT_MJPEG == enType || PT_JPEG == enType)
    {
        stVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
        stVencChnAttr->stGopAttr.stNormalP.s32IPQpDelta = 0;
    }
    else
    {
        memcpy(&stVencChnAttr->stGopAttr, &stGopAttr, sizeof(VENC_GOP_ATTR_S));
        if ((VENC_GOPMODE_BIPREDB == stGopAttr.enGopMode) && (PT_H264 == enType))
        {
            if (0 == stVencChnAttr->stVencAttr.u32Profile)
            {
                stVencChnAttr->stVencAttr.u32Profile = 1;

                if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                    zpl_media_debugmsg_err("H.264 base profile not support BIPREDB, so change profile to main profile!\n");
            }
        }

        if ((VENC_RC_MODE_H264QPMAP == stVencChnAttr->stRcAttr.enRcMode) || (VENC_RC_MODE_H265QPMAP == stVencChnAttr->stRcAttr.enRcMode))
        {
            if (VENC_GOPMODE_ADVSMARTP == stGopAttr.enGopMode)
            {
                stVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_SMARTP;

                if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                    zpl_media_debugmsg_err("advsmartp not support QPMAP, so change gopmode to smartp!\n");
            }
        }
    }
	memcpy(&stVencChnAttr->stGopAttr, &stGopAttr, sizeof(VENC_GOP_ATTR_S));
    return HI_SUCCESS;
}
#endif



int zpl_vidhal_venc_create(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_PARAM_MOD_S stModParam;
    zpl_bool bRcnRefShareBuf = zpl_true;
    if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H264)
        s32Ret = zpl_codec_build_hwh264(venc, &stVencChnAttr,  bRcnRefShareBuf);
    else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H265)
        s32Ret = zpl_codec_build_hwh265(venc, &stVencChnAttr,  bRcnRefShareBuf);
    else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_MJPEG)
        s32Ret = zpl_codec_build_hwmjpeg(venc, PT_MJPEG,  &stVencChnAttr, bRcnRefShareBuf);
    else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_JPEG)
        s32Ret = zpl_codec_build_hwmjpeg(venc, PT_JPEG, &stVencChnAttr, bRcnRefShareBuf);
    if(s32Ret != OK)
        return ERROR;
    if(HI_MPI_VENC_GetModParam(&stModParam) == HI_SUCCESS)    
    {
        if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H264)
        {
            stModParam.stH264eModParam.u32OneStreamBuffer = venc->pCodec->packetization_mode;
        }
        else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H265)
        {
            stModParam.stH265eModParam.u32OneStreamBuffer = venc->pCodec->packetization_mode;
        }
        else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_MJPEG)
        {
            stModParam.stJpegeModParam.u32OneStreamBuffer = venc->pCodec->packetization_mode;
        }
        else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_JPEG)
        {
            stModParam.stJpegeModParam.u32OneStreamBuffer = venc->pCodec->packetization_mode;
        }
        if(HI_MPI_VENC_SetModParam(&stModParam) != HI_SUCCESS)  
            return ERROR;
    }
    else
        return ERROR;

    s32Ret = HI_MPI_VENC_CreateChn(venc->venc_channel, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Create failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    s32Ret = zpl_vidhal_venc_CloseReEncode(venc->venc_channel);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) CloseReEncode failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        HI_MPI_VENC_DestroyChn(venc->venc_channel);
        return s32Ret;
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_venc_reset(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = HI_MPI_VENC_ResetChn(venc->venc_channel);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Reset failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_venc_destroy(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE

    int s32Ret = HI_MPI_VENC_DestroyChn(venc->venc_channel);

    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Destroy failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return s32Ret;
#else
    return OK;
#endif
}

int zpl_vidhal_venc_update_fd(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    if(venc->venc_channel >= 0)
    {
        ipstack_type(venc->vencfd) = IPSTACK_OS;
        ipstack_fd(venc->vencfd) = HI_MPI_VENC_GetFd(venc->venc_channel);
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
			zpl_media_debugmsg_debug(" video venc channel %d fd %d\n", venc->venc_channel, venc->vencfd);
        return OK;
    }
    return ERROR;
#else
    return OK;
#endif
}

int zpl_vidhal_venc_start(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    VENC_RECV_PIC_PARAM_S stRecvParam;
    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    stRecvParam.s32RecvPicNum = -1;
    s32Ret = HI_MPI_VENC_StartRecvFrame(venc->venc_channel, &stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Start failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    zpl_media_debugmsg_err(" ==========VENC Channel (%d) zpl_vidhal_venc_start", venc->venc_channel);
    venc->vencfd = HI_MPI_VENC_GetFd(venc->venc_channel);
    return HI_SUCCESS;
#else
    return OK;
#endif
}

int zpl_vidhal_venc_stop(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_VENC_StopRecvFrame(venc->venc_channel);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Stop failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    HI_MPI_VENC_CloseFd(venc->venc_channel);
    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
* funciton : Start snap
******************************************************************************/

int zpl_vidhal_venc_snap_start(zpl_int32 vencchn, zpl_video_size_t *pstSize, zpl_bool bSupportDCF)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    /******************************************
     step 1:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVencAttr.enType = PT_JPEG;
    stVencChnAttr.stVencAttr.u32Profile = 0;
    stVencChnAttr.stVencAttr.u32MaxPicWidth = pstSize->width;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = pstSize->height;
    stVencChnAttr.stVencAttr.u32PicWidth = pstSize->width;
    stVencChnAttr.stVencAttr.u32PicHeight = pstSize->height;
    stVencChnAttr.stVencAttr.u32BufSize = pstSize->width * pstSize->height * 2;
    stVencChnAttr.stVencAttr.bByFrame = HI_TRUE; /*get stream mode is field mode  or frame mode*/
    stVencChnAttr.stVencAttr.stAttrJpege.bSupportDCF = bSupportDCF;
    //stVencChnAttr.stVencAttr.stAttrJpege.bSupportXMP = HI_FALSE;
    stVencChnAttr.stVencAttr.stAttrJpege.stMPFCfg.u8LargeThumbNailNum = 0;
    stVencChnAttr.stVencAttr.stAttrJpege.enReceiveMode = VENC_PIC_RECEIVE_SINGLE;

    s32Ret = HI_MPI_VENC_CreateChn(vencchn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Create failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
* funciton : Stop snap
******************************************************************************/
int zpl_vidhal_venc_snap_stop(zpl_int32 vencchn)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_VENC_StopRecvFrame(vencchn);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Stop failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_DestroyChn(vencchn);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Destroy failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
* funciton : snap process
******************************************************************************/

int zpl_vidhal_venc_snap_process(zpl_int32 vencchn, zpl_int32 SnapCnt)
{
#ifdef ZPL_HISIMPP_MODULE
    struct timeval TimeoutVal;
    fd_set read_fds;
    zpl_int32 s32VencFd;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    zpl_int32 s32Ret;
    VENC_RECV_PIC_PARAM_S stRecvParam;
    zpl_uint32 i;

    /******************************************
     step 2:  Start Recv Venc Pictures
    ******************************************/
    stRecvParam.s32RecvPicNum = SnapCnt;
    s32Ret = HI_MPI_VENC_StartRecvFrame(vencchn, &stRecvParam);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Start failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    /******************************************
     step 3:  recv picture
    ******************************************/
    s32VencFd = HI_MPI_VENC_GetFd(vencchn);
    if (s32VencFd < 0)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Get FD failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    for (i = 0; i < SnapCnt; i++)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32VencFd, &read_fds);
        TimeoutVal.tv_sec = 10;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            zpl_media_debugmsg_err("snap select failed!\n");
            return HI_FAILURE;
        }
        else if (0 == s32Ret)
        {
            zpl_media_debugmsg_err("snap time out!\n");
            return HI_FAILURE;
        }
        else
        {
            if (FD_ISSET(s32VencFd, &read_fds))
            {
                s32Ret = HI_MPI_VENC_QueryStatus(vencchn, &stStat);
                if (s32Ret != HI_SUCCESS)
                {
                    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                        zpl_media_debugmsg_err(" VENC Channel (%d) Query Status failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
                    return HI_FAILURE;
                }
                /*******************************************************
                suggest to check both u32CurPacks and u32LeftStreamFrames at the same time,for example:
                 if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
                 {                zpl_media_debugmsg_err("NOTE: Current  frame is NULL!\n");
                    return HI_SUCCESS;
                 }
                 *******************************************************/
                if (0 == stStat.u32CurPacks)
                {
                    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                        zpl_media_debugmsg_err("NOTE: Current  frame is NULL!\n");
                    return HI_SUCCESS;
                }
                stStream.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                if (NULL == stStream.pstPack)
                {
                    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                        zpl_media_debugmsg_err("malloc memory failed!\n");
                    return HI_FAILURE;
                }
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = HI_MPI_VENC_GetStream(vencchn, &stStream, -1);
                if (HI_SUCCESS != s32Ret)
                {
                    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                        zpl_media_debugmsg_err(" VENC Channel (%d) Get Stream failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));

                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    return HI_FAILURE;
                }
                s32Ret = HI_MPI_VENC_ReleaseStream(vencchn, &stStream);
                if (HI_SUCCESS != s32Ret)
                {
                    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
                        zpl_media_debugmsg_err(" VENC Channel (%d) Release Stream failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));

                    free(stStream.pstPack);
                    stStream.pstPack = NULL;

                    return HI_FAILURE;
                }
                free(stStream.pstPack);
                stStream.pstPack = NULL;
            }
        }
    }
    /******************************************
     step 4:  stop recv picture
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvFrame(vencchn);
    if (s32Ret != HI_SUCCESS)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Stop failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}




int zpl_vidhal_venc_request_IDR(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
    {
        zpl_media_debugmsg_debug(" VENC Channel (%d) Request IDR", venc->venc_channel);
    }
    s32Ret = HI_MPI_VENC_RequestIDR(venc->venc_channel, zpl_true);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Request IDR failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return ERROR;
#endif
}

int zpl_vidhal_venc_enable_IDR(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc, zpl_bool bEnableIDR)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
    {
        zpl_media_debugmsg_debug(" VENC Channel (%d) Enable IDR", venc->venc_channel);
    }
    s32Ret = HI_MPI_VENC_EnableIDR(venc->venc_channel, bEnableIDR);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Enable IDR failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return ERROR;
#endif
}

#ifdef ZPL_HISIMPP_MODULE
static int zpl_vidhal_venc_frame_code(zpl_video_encode_t *venc, VENC_DATA_TYPE_U enctype)
{
    int enc = 0;
    if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H264)
    {
        if(enctype.enH264EType == H264E_NALU_BSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_BSLICE;
        else if(enctype.enH264EType == H264E_NALU_PSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_PSLICE;
        else if(enctype.enH264EType == H264E_NALU_ISLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_ISLICE;
        else if(enctype.enH264EType == H264E_NALU_IDRSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_IDRSLICE;
        else if(enctype.enH264EType == H264E_NALU_SEI)
            enc = ZPL_VIDEO_FRAME_TYPE_SEI;
         else if(enctype.enH264EType == H264E_NALU_SPS)
            enc = ZPL_VIDEO_FRAME_TYPE_SPS;
        else if(enctype.enH264EType == H264E_NALU_PPS)
            enc = ZPL_VIDEO_FRAME_TYPE_PPS;
    }
    else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_H265)
    {
        if(enctype.enH265EType == H265E_NALU_BSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_BSLICE;
        else if(enctype.enH265EType == H265E_NALU_PSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_PSLICE;
        else if(enctype.enH265EType == H265E_NALU_ISLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_ISLICE;
        else if(enctype.enH265EType == H265E_NALU_IDRSLICE)
            enc = ZPL_VIDEO_FRAME_TYPE_IDRSLICE;
        else if(enctype.enH265EType == H265E_NALU_SEI)
            enc = ZPL_VIDEO_FRAME_TYPE_SEI;
         else if(enctype.enH265EType == H265E_NALU_SPS)
            enc = ZPL_VIDEO_FRAME_TYPE_SPS;
        else if(enctype.enH265EType == H265E_NALU_PPS)
            enc = ZPL_VIDEO_FRAME_TYPE_PPS;
        else if(enctype.enH265EType == H265E_NALU_VPS)
            enc = ZPL_VIDEO_FRAME_TYPE_VPS;
    }
    else if(venc->pCodec->enctype == ZPL_VIDEO_CODEC_MJPEG)
    {
        if(enctype.enJPEGEType == JPEGE_PACK_ECS)
            enc = ZPL_VIDEO_FRAME_TYPE_ECS;
        else if(enctype.enJPEGEType == JPEGE_PACK_APP)
            enc = ZPL_VIDEO_FRAME_TYPE_APP;
        else if(enctype.enJPEGEType == JPEGE_PACK_VDO)
            enc = ZPL_VIDEO_FRAME_TYPE_VDO;
        else if(enctype.enJPEGEType == JPEGE_PACK_PIC)
            enc = ZPL_VIDEO_FRAME_TYPE_PIC;
        else if(enctype.enJPEGEType == JPEGE_PACK_DCF)
            enc = ZPL_VIDEO_FRAME_TYPE_DCF;
        else if(enctype.enJPEGEType == JPEGE_PACK_DCF_PIC)
            enc = ZPL_VIDEO_FRAME_TYPE_DCF_PIC;
    }
    return enc;
}
#endif

int zpl_vidhal_venc_frame_recvfrom(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret, i;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    VENC_PACK_S venc_pack[16];
    zpl_uint32 packsize = 0;
    zpl_uint32 offset = 0;
    zpl_media_buffer_data_t * bufdata = NULL;
    zpl_video_assert(venc);
    zpl_video_assert(venc->buffer_queue);
    memset(&stStat, 0, sizeof(stStat));
    memset(&stStream, 0, sizeof(stStream));
    memset(&venc_pack, 0, sizeof(venc_pack));
    /*******************************************************
     step 1 : query how many packs in one-frame stream.
    *******************************************************/
    s32Ret = HI_MPI_VENC_QueryStatus(venc->venc_channel, &stStat);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Query Status failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        //DP(APP_ERR, MODULE_ENC, "HI_MPI_VENC_Query chn[%d] failed with %#x!\n", vencchn, s32Ret);
        return ERROR;
    }

    /*******************************************************
     step 2 : malloc corresponding number of pack nodes.
    *******************************************************/
    stStream.pstPack = venc_pack;
    /*******************************************************
     step 3 : call mpi to get one-frame stream
    *******************************************************/
    stStream.u32PackCount = stStat.u32CurPacks ? 16:stStat.u32CurPacks;
    s32Ret = HI_MPI_VENC_GetStream(venc->venc_channel, &stStream, -1);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Get Stream failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        //DP(APP_ERR, MODULE_ENC, "HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
        return ERROR;
    }

//    if (venc && venc->venc_frame_handle)
//        (venc->venc_frame_handle)(&stStream, 0);
    /*******************************************************
     step 4 : save frame to buff
    *******************************************************/
    for (i = 0; i < (zpl_int32)(stStream.u32PackCount); i++)
    {
        packsize += (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(venc->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		venc->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, RECV) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		{
			zpl_media_debugmsg_debug(" VENC Channel (%d) Get Stream Count=%d Total Size=%d", venc->venc_channel, stStream.u32PackCount, packsize);
		}
	}
	venc->dbg_recv_count++;
#endif

    bufdata = zpl_media_buffer_data_malloc(venc->buffer_queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, packsize);
    if(bufdata && bufdata->buffer_data && bufdata->buffer_maxsize >= packsize)
    {
        bufdata->buffer_type = ZPL_MEDIA_VIDEO;     //音频视频
        bufdata->buffer_timetick = stStream.pstPack->u64PTS / 1000U;    //时间戳 毫秒
        //bufdata->buffer_seq  = stStream.u32Seq;                        //序列号 底层序列号
        bufdata->buffer_len = packsize;             //当前缓存帧的长度

        for (i = 0; i < (zpl_int32)(stStream.u32PackCount); i++)
        {
            packsize = (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
            memcpy(bufdata->buffer_data + offset, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, packsize);
            offset += packsize;
            
            if ((stStream.pstPack[i].DataType.enH264EType == H264E_NALU_IDRSLICE) || (stStream.pstPack[i].DataType.enH265EType == H265E_NALU_IDRSLICE))
            {
                bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_KEY;
            }
            else if((stStream.pstPack[i].DataType.enH264EType == H264E_NALU_PSLICE) || (stStream.pstPack[i].DataType.enH265EType == H265E_NALU_PSLICE))
            {
                bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_PSLICE;
            }
            else
            {
                bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_BSLICE;
            }
            bufdata->buffer_key = zpl_vidhal_venc_frame_code(venc, stStream.pstPack[i].DataType);
        }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
		if(venc->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
		{
			venc->dbg_recv_count = 0;
			if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, RECV) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
			{
				zpl_media_debugmsg_debug(" VENC Channel (%d) Add Stream To buffer_queue Total Size=%d", venc->venc_channel, packsize);
			}
		}
		venc->dbg_recv_count++;
#endif

		zpl_media_buffer_add(venc->buffer_queue, bufdata);
        //zpl_media_buffer_enqueue(venc->buffer_queue, bufdata);
    }
    /*******************************************************
     step 5 : release stream
    *******************************************************/
    s32Ret = HI_MPI_VENC_ReleaseStream(venc->venc_channel, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Release Stream failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    return HI_SUCCESS;
#else
    return ERROR;
#endif
}

int zpl_vidhal_venc_frame_recvfrom_one(zpl_int32 channel, ZPL_MEDIA_CHANNEL_INDEX_E channel_index, zpl_video_encode_t *venc)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret, i;
    VENC_CHN_STATUS_S stStat;
    VENC_STREAM_S stStream;
    VENC_PACK_S venc_pack[16];
    zpl_uint32 packsize = 0;
    #ifndef ZPL_VENC_READ_DEBUG
    zpl_uint32 offset = 0;
    zpl_media_buffer_data_t * bufdata = NULL;
    #endif
    memset(&stStat, 0, sizeof(stStat));
    memset(&stStream, 0, sizeof(stStream));
    memset(&venc_pack, 0, sizeof(venc_pack));

    zpl_video_assert(venc);
    zpl_video_assert(venc->buffer_queue);
    
    /*******************************************************
     step 1 : query how many packs in one-frame stream.
    *******************************************************/
    s32Ret = HI_MPI_VENC_QueryStatus(venc->venc_channel, &stStat);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Query Status failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        //DP(APP_ERR, MODULE_ENC, "HI_MPI_VENC_Query chn[%d] failed with %#x!\n", vencchn, s32Ret);
        return ERROR;
    }
	if (0 == stStat.u32CurPacks)
	{
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Query Status no CurPacks(%d)", venc->venc_channel, stStat.u32CurPacks);
        return ERROR;
	}
    /*******************************************************
     step 2 : malloc corresponding number of pack nodes.
    *******************************************************/
    stStream.pstPack = venc_pack;
    /*******************************************************
     step 3 : call mpi to get one-frame stream
    *******************************************************/
    stStream.u32PackCount = (stStat.u32CurPacks>16) ? 16:stStat.u32CurPacks;
	
    s32Ret = HI_MPI_VENC_GetStream(venc->venc_channel, &stStream, -1);
    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" VENC Channel (%d) Get Stream failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
        //DP(APP_ERR, MODULE_ENC, "HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
        return ERROR;
    }
    //zpl_media_debugmsg_debug(" ======================VENC Channel (%d) Get Stream Count=%d", venc->venc_channel, stStream.u32PackCount);
    /*******************************************************
     step 4 : save frame to buff
    *******************************************************/
	packsize = 0;
    for (i = 0; i < (zpl_int32)(stStream.u32PackCount); i++)
    {
        packsize += (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
    }
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
	if(venc->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
	{
		venc->dbg_recv_count = 0;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, RECV) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		{
			zpl_media_debugmsg_debug(" VENC Channel (%d) Get Stream Count=%d Total Size=%d", venc->venc_channel, stStream.u32PackCount, packsize);
		}
	}
	venc->dbg_recv_count++;
#endif

	packsize = 0;
	for (i = 0; i < (zpl_int32)(stStream.u32PackCount); i++)
	{
        packsize = (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
        zpl_uint8 *pbuf = stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset;
/*        
        if (packsize >= 3 && pbuf[0] == 0x00 && pbuf[1] == 0x00 && pbuf[2] == 0x01) {
            pbuf += 3;
            packsize -= 3;
        }
        if (packsize >= 4 && pbuf[0] == 0x00 && pbuf[1] == 0x00 && pbuf[2] == 0x00 && pbuf[3] == 0x01) {
            pbuf += 4;
            packsize -= 4;
        }
*/
#ifdef ZPL_VENC_READ_DEBUG 
		if(venc->mem_buf && venc->mem_size && venc->mem_size >= packsize)       
    	{
			//packsize = (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
            //if(stStream.pstPack[i].pu8Addr && (stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset))
			//    memcpy(venc->mem_buf, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, packsize);
            if(stStream.pstPack[i].pu8Addr && (pbuf))
			    memcpy(venc->mem_buf + offset, pbuf, packsize);
        }       
#else/* ZPL_VENC_READ_DEBUG */

		bufdata = zpl_media_buffer_data_malloc(venc->buffer_queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, packsize);
        offset = 0;
		if(bufdata != NULL
            && bufdata->buffer_data != NULL 
            && bufdata->buffer_maxsize >= packsize)
		{
            bufdata->buffer_type = ZPL_MEDIA_VIDEO;     //音频视频
            bufdata->buffer_timetick = stStream.pstPack->u64PTS / 1000U;    //时间戳 毫秒
            //bufdata->buffer_seq = stStream.u32Seq;               //序列号 底层序列号
            bufdata->buffer_len = packsize;             //当前缓存帧的长度

			//packsize = (stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
            //if(stStream.pstPack[i].pu8Addr && (stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset))
			//    memcpy(bufdata->buffer_data + offset, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, packsize);
			
            if(stStream.pstPack[i].pu8Addr && (pbuf))
			    memcpy(bufdata->buffer_data + offset, pbuf, packsize);
            //offset += packsize;
			//bufdata->buffer_data = stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset;
			if ((stStream.pstPack[i].DataType.enH264EType == H264E_NALU_IDRSLICE) || 
                (stStream.pstPack[i].DataType.enH265EType == H265E_NALU_IDRSLICE))
			{
				bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_KEY;
			}
			else if((stStream.pstPack[i].DataType.enH264EType == H264E_NALU_PSLICE) || 
                (stStream.pstPack[i].DataType.enH265EType == H265E_NALU_PSLICE))
			{
				bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_PSLICE;
			}
			else
			{
				bufdata->buffer_key = ZPL_VIDEO_FRAME_TYPE_BSLICE;
			}
            bufdata->buffer_key = zpl_vidhal_venc_frame_code(venc, stStream.pstPack[i].DataType);
            
	#ifdef ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL
			if(venc->dbg_recv_count == ZPL_VIDEO_VIDHAL_DEBUG_RECV_DETAIL)
			{
				venc->dbg_recv_count = 0;
				if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, RECV) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
				{
					zpl_media_debugmsg_debug(" VENC Channel (%d) Add Stream To buffer_queue Total Size=%d", venc->venc_channel, packsize);
				}
			}
			venc->dbg_recv_count++;
	#endif
			zpl_media_buffer_add(venc->buffer_queue, bufdata);
			//zpl_media_buffer_enqueue(venc->buffer_queue, bufdata);
            //zpl_media_buffer_add_and_distribute(venc->buffer_queue, bufdata);     
		}
#endif /* ZPL_VENC_READ_DEBUG */
	}
	/*******************************************************
	 step 5 : release stream
	*******************************************************/
	s32Ret = HI_MPI_VENC_ReleaseStream(venc->venc_channel, &stStream);
	if (HI_SUCCESS != s32Ret)
	{
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
			zpl_media_debugmsg_err(" VENC Channel (%d) Release Stream failed(%s)", venc->venc_channel, zpl_syshal_strerror(s32Ret));
		return ERROR;
	}
	
    return HI_SUCCESS;
#else
    return ERROR;
#endif
}

int zpl_vidhal_venc_frame_sendto(zpl_video_encode_t *venc, zpl_int32 id, zpl_int32 vencchn, void *p, zpl_int32 s32MilliSec)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_VENC_SendFrame(vencchn, p, s32MilliSec);
#ifdef ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL
	if(venc->dbg_send_count == ZPL_VIDEO_VIDHAL_DEBUG_SEND_DETAIL)
	{
		venc->dbg_send_count = 0;
		if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE) && ZPL_MEDIA_DEBUG(ENCODE, SEND) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
		{
			zpl_media_debugmsg_debug(" Frame sendto VENC Channel (%d)", vencchn);
		}
	}
	venc->dbg_send_count++;
#endif

    if (HI_SUCCESS != s32Ret)
    {
        if(ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zpl_media_debugmsg_err(" Frame sendto VENC Channel (%d) failed(%s)", vencchn, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return ERROR;
#endif
}

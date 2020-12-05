/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtsp_server.h
** 
** RTSP server
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>
#include <cstring>

#include <stdint.h>
#include <stdio.h>

#include "h264Encoder.hpp"

h264Encoder::h264Encoder()
{
}

h264Encoder::~h264Encoder()
{
    h264EncoderDestroy();
}

int h264Encoder::h264EncoderSetup(int width, int height, int fmt, int fps)
{
    this->v_width = width;
    this->v_height = height;
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_setup(width, height, fmt, fps);
#endif
#ifdef PL_LIBX264_MODULE
    return h264_encoder_setup(width, height, fmt, fps);
#endif
}

int h264Encoder::h264EncoderInput(char *frame, int len)
{
#ifdef PL_LIBX264_MODULE
    return h264_encoder_input(frame, len);
#endif
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_input(frame, len);
#endif
}

int h264Encoder::h264EncoderOutput(char *frame, int len)
{
#ifdef PL_LIBX264_MODULE
    return h264_encoder_output(frame, len);
#endif
#ifdef PL_OPENH264_MODULE
    return openh264_encoder_output(frame, len);
#endif
}

unsigned char *h264Encoder::h264EncoderOutput()
{
#ifdef PL_LIBX264_MODULE
    return m_h264_nal->p_payload;
#endif
#ifdef PL_OPENH264_MODULE
    return (unsigned char *)m_out_frame_payload;
#endif
}

int h264Encoder::h264EncoderOutputSize(bool clear)
{
    int ret = 0;

    ret = (int)m_out_frame_size;
    if (clear)
        m_out_frame_size = 0;
    return ret;
}

int h264Encoder::h264EncoderDestroy()
{
#ifdef PL_LIBX264_MODULE
    if (m_h264)
        x264_encoder_close(m_h264);
    x264_picture_clean(&m_pic_in);
#endif
#ifdef PL_OPENH264_MODULE
    openh264_encoder_destroy();
#endif
    return 0;
}

#ifdef PL_LIBX264_MODULE
int h264Encoder::h264_encoder_setup(int width, int height, int fmt, int fps)
{
    int _x264_profile_index = 0;
    x264_param_default(&m_param);
    /* Get default params for preset/tuning */
    if (x264_param_default_preset(&m_param, "medium", NULL) < 0)
        return -1;

    /* Configure non-default params */
    //param.i_bitdepth = 8;
    m_param.i_csp = fmt; //X264_CSP_I420;
    m_param.i_width = width;
    m_param.i_height = height;
    //m_param.b_vfr_input = 0;
    //m_param.b_repeat_headers = 1;
    //m_param.b_annexb = 1;
    m_param.i_fps_num = fps;
    //m_param.i_fps_dep = 1;
    //m_param.i_keyint_max = 10;
    //m_param.rc.i_bitrate = 1200;
    //m_param.rc.i_rc_method = X264_RC_ABR;

    m_plane_2 = m_plane_1 = m_plane_0 = m_frame_size = width * height;

    if (i_csp == X264_CSP_I444)
    {
        m_frame_size += (m_frame_size << 1);
        _x264_profile_index = 5;
    }
    else if (i_csp == X264_CSP_I422)
    {
        //plane_2 = plane_1 = (plane_0 >> 1);
        m_frame_size = (m_frame_size * 2);
        _x264_profile_index = 4;
    }
    else //if(i_csp == X264_CSP_I420)
    {
        //frame_size += (frame_size/4)*2;
        //frame_size += (frame_size >> 1);
        m_plane_2 = m_plane_1 = (m_plane_0 >> 2);
        m_frame_size = (m_frame_size * 3 / 2);
        _x264_profile_index = 2;
    }
    //m_frame_size = m_plane_0 + m_plane_1 + m_plane_2;
    /* Apply profile restrictions. */
    if (x264_param_apply_profile(&m_param, x264_profile_names[_x264_profile_index] /*"high422"*/) < 0)
        return -1;
    //x264_picture_init( &m_pic_in);
    if (x264_picture_alloc(&m_pic_in, m_param.i_csp, m_param.i_width, m_param.i_height) < 0)
        return -1;
    m_h264 = x264_encoder_open(&m_param);
    if (!m_h264)
        return -1;
    return 0;
}

int h264Encoder::h264_encoder_input(char *frame, int len)
{
    int i_nal = 0;
    x264_picture_t pic_out;
    x264_picture_init(&pic_out);
    std::cout << "====================== frame-len:" << len << "frame-size:" << frame_size << std::endl;
    if (len == m_frame_size)
    {
        /* Read input frame */
        if (m_param.i_csp == X264_CSP_I422)
        {
            int i = 0;
            int y_i = 0, u_i = 0, v_i = 0;
            for (i = 0; i < len;)
            {
                m_pic_in.img.plane[0][y_i++] = frame[i];
                i++;
                m_pic_in.img.plane[1][u_i++] = frame[i];
                i++;
                m_pic_in.img.plane[0][y_i++] = frame[i];
                i++;
                m_pic_in.img.plane[2][v_i++] = frame[i];
                i++;
            }
        }
        else
        {
            memcpy(m_pic_in.img.plane[0], frame, m_plane_0);
            memcpy(m_pic_in.img.plane[1], frame + m_plane_0, m_plane_1);
            memcpy(m_pic_in.img.plane[2], frame + m_plane_0 + m_plane_1, m_plane_2);
        }
        std::cout << "====================== x264_encoder_encode" << std::endl;
        m_out_frame_size = x264_encoder_encode(m_h264, &m_h264_nal, &i_nal, &m_pic_in, &pic_out);
        if (m_out_frame_size < 0)
            return -1;
        else if (m_out_frame_size)
        {
            //if( !fwrite( nal->p_payload, out_frame_size, 1, stdout ) )
            //    goto fail;
            m_pic_in.i_pts++;
            return m_out_frame_size;
        }
    }
    else
    {
        /* Flush delayed frames */
        while (x264_encoder_delayed_frames(m_h264))
        {
            m_out_frame_size = x264_encoder_encode(m_h264, &m_h264_nal, &i_nal, NULL, &pic_out);
            if (m_out_frame_size < 0)
                return -1;
            else if (m_out_frame_size)
            {
                //encode_data.clear();
                //if( !fwrite( nal->p_payload, out_frame_size, 1, stdout ) )
                //    goto fail;
                return m_out_frame_size;
            }
        }
    }
    return -1;
}

int h264Encoder::h264_encoder_output(char *frame, int len)
{
    if (m_out_frame_size > 0)
    {
        int ret = (m_out_frame_size > len) ? len : m_out_frame_size;
        memmove(frame, m_h264_nal->p_payload, ret);
        m_out_frame_size = 0;
        return ret;
    }
    return -1;
}
#endif

#ifdef PL_OPENH264_MODULE
struct SLayerPEncCtx
{
    unsigned int iDLayerQp;
    SSliceArgument sSliceArgument;
};

int h264Encoder::openh264_encoder_destroy()
{
    if (m_out_frame_payload)
        delete[] m_out_frame_payload;
    if (m_encoder)
        WelsDestroySVCEncoder(m_encoder);
    return 0;
}

int h264Encoder::openh264_encoder_setup(int width, int height, int fmt, int fps)
{
    /* encoder allocation */
    int rc = WelsCreateSVCEncoder(&m_encoder);
    if (rc != 0)
        return -1;
    SEncParamExt eprm;
    SSpatialLayerConfig *elayer = &eprm.sSpatialLayers[0];
    SLayerPEncCtx elayer_ctx;
    SDecodingParam sDecParam = {0};
    SliceModeEnum eSliceMode = SM_SINGLE_SLICE; //eprm.sSpatialLayers[0].sSliceArgument.uiSliceMode;
    /*
     * Encoder
     */
    /* Init encoder parameters */
    m_encoder->GetDefaultParams(&eprm);
    eSliceMode = eprm.sSpatialLayers[0].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

    eprm.iComplexityMode = MEDIUM_COMPLEXITY;
    eprm.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
    eprm.iPicWidth = v_width;
    eprm.iUsageType = CAMERA_VIDEO_REAL_TIME;
    eprm.iPicHeight = v_height;
    eprm.fMaxFrameRate = fps;
    eprm.iTemporalLayerNum = 1;
    eprm.uiIntraPeriod = 0;               /* I-Frame interval in frames */
    eprm.eSpsPpsIdStrategy = CONSTANT_ID; //INCREASING_ID;
#if OPENH264_VER_AT_LEAST(1, 4)
    eprm.eSpsPpsIdStrategy = CONSTANT_ID;
#else
    //eprm.bEnableSpsPpsIdAddition = 0;
#endif
    eprm.bEnableFrameCroppingFlag = true;
    eprm.iLoopFilterDisableIdc = 0;
    eprm.iLoopFilterAlphaC0Offset = 0;
    eprm.iLoopFilterBetaOffset = 0;
    eprm.iMultipleThreadIdc = 1;
    //eprm.bEnableRc			= 1;
    eprm.iTargetBitrate = 5000000; //param->enc_fmt.det.vid.avg_bps;
    eprm.bEnableFrameSkip = 1;
    eprm.bEnableDenoise = 0;
    eprm.bEnableSceneChangeDetect = 1;
    eprm.bEnableBackgroundDetection = 1;
    eprm.bEnableAdaptiveQuant = 1;
    eprm.bEnableLongTermReference = 0;
    eprm.iLtrMarkPeriod = 30;
    eprm.bPrefixNalAddingCtrl = false;
    eprm.iSpatialLayerNum = 1;
    //eprm.uiMaxNalSize			= param->enc_mtu;

    memset(&elayer_ctx, 0, sizeof(SLayerPEncCtx));
    elayer_ctx.iDLayerQp = 24;
    elayer_ctx.sSliceArgument.uiSliceMode = SM_SINGLE_SLICE; //SM_SIZELIMITED_SLICE;

    elayer_ctx.sSliceArgument.uiSliceNum = 1;
    elayer_ctx.sSliceArgument.uiSliceMbNum[0] = 960;
    elayer_ctx.sSliceArgument.uiSliceMbNum[1] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[2] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[3] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[4] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[5] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[6] = 0;
    elayer_ctx.sSliceArgument.uiSliceMbNum[7] = 0;

    eprm.iEntropyCodingModeFlag = 1;
    if (eSliceMode != SM_SINGLE_SLICE && eSliceMode != SM_SIZELIMITED_SLICE) //SM_SIZELIMITED_SLICE don't support multi-thread now
        eprm.iMultipleThreadIdc = 2;

    if (eSliceMode == SM_SIZELIMITED_SLICE)
    {
        eprm.sSpatialLayers[0].sSliceArgument.uiSliceSizeConstraint = 600;
        eprm.uiMaxNalSize = 1500;
        eprm.iMultipleThreadIdc = 4;
        eprm.bUseLoadBalancing = false;
    }
    if (eSliceMode == SM_FIXEDSLCNUM_SLICE)
    {
        eprm.sSpatialLayers[0].sSliceArgument.uiSliceNum = 4;
        eprm.iMultipleThreadIdc = 4;
        eprm.bUseLoadBalancing = false;
    }
    if (eprm.iEntropyCodingModeFlag)
    {
        eprm.sSpatialLayers[0].uiProfileIdc = PRO_MAIN;
    }
    elayer->iVideoWidth = eprm.iPicWidth;
    elayer->iVideoHeight = eprm.iPicHeight;
    elayer->fFrameRate = eprm.fMaxFrameRate;
    elayer->uiProfileIdc = eprm.sSpatialLayers[0].uiProfileIdc;
    elayer->iSpatialBitrate = eprm.iTargetBitrate;
    elayer->iDLayerQp = elayer_ctx.iDLayerQp;
    elayer->sSliceArgument.uiSliceMode = elayer_ctx.sSliceArgument.uiSliceMode;

    memcpy(&elayer->sSliceArgument,
           &elayer_ctx.sSliceArgument,
           sizeof(SSliceArgument));
    memcpy(&elayer->sSliceArgument.uiSliceMbNum[0],
           &elayer_ctx.sSliceArgument.uiSliceMbNum[0],
           sizeof(elayer_ctx.sSliceArgument.uiSliceMbNum));

    /* Init input picture */
    m_esrc_pic.iColorFormat = fmt;
    //esrc_pic.iColorFormat	= videoFormatI420;
    m_esrc_pic.uiTimeStamp = 0;
    m_esrc_pic.iPicWidth = eprm.iPicWidth;
    m_esrc_pic.iPicHeight = eprm.iPicHeight;
    m_esrc_pic.iStride[0] = m_esrc_pic.iPicWidth;
    if (fmt == videoFormatI420)
        m_esrc_pic.iStride[1] = m_esrc_pic.iStride[2] = m_esrc_pic.iStride[0] >> 1;

    //enc_input_size = esrc_pic.iPicWidth *esrc_pic.iPicHeight * 3 >> 1;

    /* Initialize encoder */
    rc = m_encoder->InitializeExt(&eprm);
    if (rc != cmResultSuccess)
    {
        printf("SVC encoder Initialize failed, rc=%d", rc);
        return -1;
    }

    int videoFormat = m_esrc_pic.iColorFormat;
    rc = m_encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
    if (rc != cmResultSuccess)
    {
        printf("SVC encoder SetOption videoFormat failed, "
               "rc=%d",
               rc);
        return -1;
    }
    return 0;
}

int h264Encoder::openh264_encoder_input(char *frame, int len)
{
    int rc = 0;
    //if (opt && opt->force_keyframe) {
    //enc->ForceIntraFrame(true);
    //}
    m_esrc_pic.iPicWidth = v_width;
    m_esrc_pic.iPicHeight = v_height;
    m_esrc_pic.pData[0] = (unsigned char *)frame;
    if (m_esrc_pic.iColorFormat == videoFormatI420)
    {
        m_esrc_pic.pData[1] = m_esrc_pic.pData[0] + (m_esrc_pic.iPicWidth * m_esrc_pic.iPicHeight);
        m_esrc_pic.pData[2] = m_esrc_pic.pData[1] + (m_esrc_pic.iPicWidth * m_esrc_pic.iPicHeight >> 2);
    }
    memset(&m_sfbsinfo, 0, sizeof(SFrameBSInfo));
    rc = m_encoder->EncodeFrame(&m_esrc_pic, &m_sfbsinfo);
    if (rc != cmResultSuccess)
    {
        printf("EncodeFrame() error, ret: %d", rc);
        return -1;
    }

    if (m_sfbsinfo.eFrameType == videoFrameTypeSkip)
    {
        /*
	    output->size = 0;
	    output->type = PJMEDIA_FRAME_TYPE_NONE;
	    output->timestamp = input->timestamp;
        */
        return 0;
    }

    //ets = input->timestamp;
    m_ilayer = 0;
    int layer_size[MAX_LAYER_NUM_OF_FRAME] = {0};
    //enc_frame_size = enc_processed = 0;

    for (m_ilayer = 0; m_ilayer < m_sfbsinfo.iLayerNum; m_ilayer++)
    {
        for (int i = 0; i < m_sfbsinfo.sLayerInfo[m_ilayer].iNalCount; i++)
            layer_size[m_ilayer] += m_sfbsinfo.sLayerInfo[m_ilayer].pNalLengthInByte[i];
        m_out_frame_size += layer_size[m_ilayer];
    }
    /*
    if ((ret = ff_alloc_packet2(avctx, avpkt, size, size))) {
        av_log(avctx, AV_LOG_ERROR, "Error getting output packet\n");
        return ret;
    }
    */
    if (m_out_frame_payload == nullptr)
        m_out_frame_payload = new char[m_out_frame_size + 1];
    m_out_frame_size = 0;
    for (m_ilayer = 0; m_ilayer < m_sfbsinfo.iLayerNum; m_ilayer++)
    {
        memcpy(m_out_frame_payload + m_out_frame_size, m_sfbsinfo.sLayerInfo[m_ilayer].pBsBuf, layer_size[m_ilayer]);
        m_out_frame_size += layer_size[m_ilayer];
    }
    //avpkt->pts = frame->pts;
    //if ( m_sfbsinfo.eFrameType == videoFrameTypeIDR)
    //    avpkt->flags |= AV_PKT_FLAG_KEY;
    //*got_packet = 1;
    return 0;
    /*
    if (whole) 
    {
        SLayerBSInfo* pLayerBsInfo;
        unsigned i = 0;

        // Find which layer with biggest payload 
        ilayer = 0;
         m_out_frame_size =  m_sfbsinfo.sLayerInfo[0].pNalLengthInByte[0];
        for (i=0; i < (unsigned) m_sfbsinfo.iLayerNum; ++i) 
        {
            unsigned j;
            pLayerBsInfo = &bsi.sLayerInfo[i];
            for (j=0; j < (unsigned)pLayerBsInfo->iNalCount; ++j) 
            {
                if (pLayerBsInfo->pNalLengthInByte[j] > (int) m_out_frame_size) 
                {
                     m_out_frame_size = pLayerBsInfo->pNalLengthInByte[j];
                     m_ilayer = i;
                }
            }
        }

        pLayerBsInfo = & m_sfbsinfo.sLayerInfo[ m_ilayer];
        if (pLayerBsInfo == NULL) 
        {
            //output->size = 0;
            //output->type = PJMEDIA_FRAME_TYPE_NONE;
            return 0;
        }

        payload = pLayerBsInfo->pBsBuf;
         m_out_frame_size = 0;
        for (int inal = pLayerBsInfo->iNalCount - 1; inal >= 0; --inal) {
             m_out_frame_size += pLayerBsInfo->pNalLengthInByte[inal];
        }

        //if ( m_out_frame_size > out_size)
        //    return -1;//PJMEDIA_CODEC_EFRMTOOSHORT;
        
        //output->type = PJMEDIA_FRAME_TYPE_VIDEO;
        //output->size = out_frame_size;
        //output->timestamp = input->timestamp;
        //pj_memcpy(output->buf, payload,  m_out_frame_size);
        
        return 0;
    }
    */
    return -1;
}

int h264Encoder::openh264_encoder_output(char *frame, int len)
{
    if (m_out_frame_size > 0 && m_out_frame_payload != nullptr)
    {
        int ret = (m_out_frame_size > len) ? len : m_out_frame_size;
        memmove(frame, m_out_frame_payload, ret);
        m_out_frame_size = 0;
        return ret;
    }
    return -1;
}
#endif

#if 0
/**
 * 最简单的基于X264的视频编码器
 * Simplest X264 Encoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序可以YUV格式的像素数据编码为H.264码流，是最简单的
 * 基于libx264的视频编码器
 *
 * This software encode YUV data to H.264 bitstream.
 * It's the simplest encoder example based on libx264.
 */
#include <stdio.h>
#include <stdlib.h>

#include "stdint.h"

#if defined(__cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif
 
 
int main(int argc, char** argv)
{
 
         int ret;
         int y_size;
         int i,j;
 
         //FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv444p.yuv", "rb");
         FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");
 
         FILE* fp_dst = fopen("cuc_ieschool.h264", "wb");
        
         //Encode 50 frame
         //if set 0, encode all frame
         int frame_num=50;
         int csp=X264_CSP_I420;
         int width=640,height=360;
 
         int iNal   = 0;
         x264_nal_t* pNals = NULL;
         x264_t* pHandle   = NULL;
         x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
        
         //Check
         if(fp_src==NULL||fp_dst==NULL){
                   printf("Error open files.\n");
                   return -1;
         }
 
         x264_param_default(pParam);
         pParam->i_width   = width;
         pParam->i_height  = height;
         /*
         //Param
         pParam->i_log_level  = X264_LOG_DEBUG;
         pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
         pParam->i_frame_total = 0;
         pParam->i_keyint_max = 10;
         pParam->i_bframe  = 5;
         pParam->b_open_gop  = 0;
         pParam->i_bframe_pyramid = 0;
         pParam->rc.i_qp_constant=0;
         pParam->rc.i_qp_max=0;
         pParam->rc.i_qp_min=0;
         pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
         pParam->i_fps_den  = 1;
         pParam->i_fps_num  = 25;
         pParam->i_timebase_den = pParam->i_fps_num;
         pParam->i_timebase_num = pParam->i_fps_den;
         */
         pParam->i_csp=csp;
         x264_param_apply_profile(pParam, x264_profile_names[5]);
        
         pHandle = x264_encoder_open(pParam);
   
         x264_picture_init(pPic_out);
         x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);
 
         //ret = x264_encoder_headers(pHandle, &pNals, &iNal);
 
         y_size = pParam->i_width * pParam->i_height;
         //detect frame number
         if(frame_num==0){
                   fseek(fp_src,0,SEEK_END);
                   switch(csp){
                   case X264_CSP_I444:frame_num=ftell(fp_src)/(y_size*3);break;
                   case X264_CSP_I420:frame_num=ftell(fp_src)/(y_size*3/2);break;
                   default:printf("Colorspace Not Support.\n");return -1;
                   }
                   fseek(fp_src,0,SEEK_SET);
         }
        
         //Loop to Encode
         for( i=0;i<frame_num;i++){
                   switch(csp){
                   case X264_CSP_I444:{
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y
                            fread(pPic_in->img.plane[1],y_size,1,fp_src);         //U
                            fread(pPic_in->img.plane[2],y_size,1,fp_src);         //V
                            break;}
                   case X264_CSP_I420:{
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y
                            fread(pPic_in->img.plane[1],y_size/4,1,fp_src);     //U
                            fread(pPic_in->img.plane[2],y_size/4,1,fp_src);     //V
                            break;}
                   default:{
                            printf("Colorspace Not Support.\n");
                            return -1;}
                   }
                   pPic_in->i_pts = i;
 
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
                   if (ret< 0){
                            printf("Error.\n");
                            return -1;
                   }
 
                   printf("Succeed encode frame: %5d\n",i);
 
                   for ( j = 0; j < iNal; ++j){
                             fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
                   }
         }
         i=0;
         //flush encoder
         while(1){
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
                   if(ret==0){
                            break;
                   }
                   printf("Flush 1 frame.\n");
                   for (j = 0; j < iNal; ++j){
                            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
                   }
                   i++;
         }
         x264_picture_clean(pPic_in);
         x264_encoder_close(pHandle);
         pHandle = NULL;
 
         free(pPic_in);
         free(pPic_out);
         free(pParam);
 
         fclose(fp_src);
         fclose(fp_dst);
 
         return 0;
}
#endif
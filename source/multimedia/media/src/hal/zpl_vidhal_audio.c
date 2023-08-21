/**
 * @file     zpl_audhal_audio_codec.c
 * @brief     : Description
 * @author   zhurish (zhurish@163.com)
 * @version  1.0
 * @date     2023-08-13
 *
 * @copyright   Copyright (c) 2023 {author}({email}).Co.Ltd. All rights reserved.
 *
 */
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"


#define ACODEC_FILE     "/dev/acodec"
/******************************************************************************
 * function : Start Ai
 ******************************************************************************/
static int zpl_audhal_aichn_create(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AI_EnableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_EnableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    if(audio->clock_rate != audio->encode.codec.clock_rate)
    {
        audio->bResample = zpl_true;
        audio->resample_rate = audio->encode.codec.clock_rate;
        s32Ret = HI_MPI_AI_EnableReSmp(audio->devid, audio->channel, audio->encode.codec.clock_rate);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_EnableReSmp(%d,%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aichn_VqeAttr(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if(audio->output)
    {
        AI_TALKVQE_CONFIG_S stVqeConfig;
        memset(&stVqeConfig, 0, sizeof(AI_TALKVQE_CONFIG_S));
        s32Ret = HI_MPI_AI_GetTalkVqeAttr(audio->devid, audio->channel, &stVqeConfig);

        stVqeConfig.u32OpenMask = AI_TALKVQE_MASK_HPF | AI_TALKVQE_MASK_ANR ;//| AI_TALKVQE_MASK_AGC ;
        stVqeConfig.s32WorkSampleRate = audio->clock_rate;
        stVqeConfig.s32FrameSample = audio->max_frame_size;
        stVqeConfig.enWorkstate = VQE_WORKSTATE_COMMON ;

        //去低频,表现为轰轰不舒适声音
        stVqeConfig.stHpfCfg.bUsrMode = HI_TRUE;
        stVqeConfig.stHpfCfg.enHpfFreq = AUDIO_HPF_FREQ_120;//建议120或150

        //回声抵消
        stVqeConfig.stAecCfg.bUsrMode = HI_TRUE;
        stVqeConfig.stAecCfg.s8CngMode = 1;//舒适噪音模式
        /*stVqeConfig.stAecCfg.s8NearAllPassEnergy = 1;//判断是否无光泽传输的远端能量阈值:
        stVqeConfig.stAecCfg.s8NearCleanSupEnergy = 2;//近端信号强制复位的能量门限:
        stVqeConfig.stAecCfg.s16DTHnlSortQTh   =   16384;
        stVqeConfig.stAecCfg.s16EchoBandLow  = 10;//语音处理band1，低频参数，
        stVqeConfig.stAecCfg.s16EchoBandHigh = 25;//41;//语音处理band1，高频参数，
        stVqeConfig.stAecCfg.s16EchoBandLow2 = 28;//47;
        stVqeConfig.stAecCfg.s16EchoBandHigh2 = 35;//63;*/

    //    HI_S16 s16ERLBand[6] = {4, 6, 36, 49, 50, 51};
    //    HI_S16 s16ERL[7] = {7, 10, 16, 10, 18, 18, 18};
    //    memcpy(stVqeConfig.stAecCfg.s16ERLBand,s16ERLBand,sizeof(s16ERLBand));
    //    memcpy(stVqeConfig.stAecCfg.s16ERL,s16ERL,sizeof(s16ERL));
    //    stVqeConfig.stAecCfg.s16VioceProtectFreqL = 3;
    //    stVqeConfig.stAecCfg.s16VioceProtectFreqL1 = 6;


        //去除外界噪音
        stVqeConfig.stAnrCfg.bUsrMode = HI_TRUE;
        stVqeConfig.stAnrCfg.s16NrIntensity = 15;//[0~25]越大降噪力度越高,损伤越高.
        stVqeConfig.stAnrCfg.s16NoiseDbThr = 45;//[30~60]越大，检测力度越弱,声音更平滑
        stVqeConfig.stAnrCfg.s8SpProSwitch = 1;//[0/1]是否开启对音乐细节检测,喧闹场景不建议开

        //AGC 更多是放大输入源的声音
        stVqeConfig.stAgcCfg.bUsrMode    = HI_FALSE;

    #if 0
        stVqeConfig.stAgcCfg.s8TargetLevel = -2;
        stVqeConfig.stAgcCfg.s8NoiseFloor = -40;
        stVqeConfig.stAgcCfg.s8MaxGain = 30;
        stVqeConfig.stAgcCfg.s8AdjustSpeed = 10;
        stVqeConfig.stAgcCfg.s8ImproveSNR = 2;//上限6db
        stVqeConfig.stAgcCfg.s8UseHighPassFilt = 3;
        stVqeConfig.stAgcCfg.s8OutputMode = 0;
        stVqeConfig.stAgcCfg.s16NoiseSupSwitch = 1;//开启噪声抑制
    #endif
        s32Ret = HI_MPI_AI_DisableVqe(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_DisableVqe(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_AI_SetTalkVqeAttr(audio->devid, audio->channel, audio->output->devid, audio->output->channel, &stVqeConfig);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_SetTalkVqeAttr(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        s32Ret = HI_MPI_AI_EnableVqe(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_EnableVqe(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = HI_MPI_AI_DisableVqe(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_DisableVqe(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aichn_start(zpl_audio_input_t *audio, int start)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if(start)
    {
        s32Ret = HI_MPI_AI_EnableChn(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_EnableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        return zpl_audhal_aichn_VqeAttr(audio);
    }
    else
        s32Ret = HI_MPI_AI_DisableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_DisableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
 * function : Stop Ai
 ******************************************************************************/
static int zpl_audhal_aichn_destroy(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if (HI_TRUE == audio->bResample)
    {
        s32Ret = HI_MPI_AI_DisableReSmp(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AI_DisableReSmp failed with %#x(%s)\n", __FUNCTION__, s32Ret, zpl_syshal_strerror(s32Ret));
            return s32Ret;
        }
    }
    s32Ret = HI_MPI_AI_DisableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_DisableChn failed with %#x(%s)\n", __FUNCTION__, s32Ret, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }

    return HI_SUCCESS;
#else
    return OK;
#endif
}
static int zpl_audhal_aichn_update_fd(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    if (audio->channel >= 0 && audio->fd != ZPL_SOCKET_INVALID)
    {
        ipstack_type(audio->fd) = IPSTACK_OS;
        ipstack_fd(audio->fd) = HI_MPI_AI_GetFd(audio->devid, audio->channel);
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio channel %d fd %d\n", audio->channel, ipstack_fd(audio->fd));
        return OK;
    }
    return OK;
#else
    return OK;
#endif
}
static int zpl_audhal_aichn_frame_handle(zpl_audio_input_t *audio, char *p, char *p2, int ti)
{
#ifdef ZPL_HISIMPP_MODULE    
    int s32Ret = 0;
    if(audio->hwbind == ZPL_MEDIA_CONNECT_SW)
    {
        //zm_msg_debug(" audio channel %d frame sendto encode %d \n", audio->channel, audio->encode.channel);
        s32Ret = HI_MPI_AENC_SendFrame(audio->encode.channel, p, p2);
    }
    if(audio->hw_connect_out == ZPL_MEDIA_CONNECT_SW && audio->output)
    {
        //zm_msg_debug(" audio channel %d frame sendto ao %d/%d \n", audio->channel, audio->output->devid, audio->output->channel);
        s32Ret = HI_MPI_AO_SendFrame(audio->output->devid, audio->output->channel, p, 1000);
    }
    return s32Ret;
#else
    return OK;
#endif
}

static int zpl_audhal_aichn_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S   stAecFrm;

    memset(&stFrame, 0, sizeof(stFrame));
    zpl_video_assert(audio);
    /* get stream from audio chn */
    s32Ret = HI_MPI_AI_GetFrame(audio->devid, audio->channel, &stFrame, &stAecFrm, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio get audio channel %d stream failed with:%s\n", audio->channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    if(audio->input_frame_handle)
        (audio->input_frame_handle)(audio, &stFrame, &stAecFrm, 1000);
    //zm_msg_debug(" audio channel %d frame read \n", audio->channel);
    /*if(audio->hwbind == ZPL_MEDIA_CONNECT_SW)
    {
        //zm_msg_debug(" audio channel %d frame sendto encode %d \n", audio->channel, audio->encode.channel);
        s32Ret = HI_MPI_AENC_SendFrame(audio->encode.channel, &stFrame, &stAecFrm);
    }
    if(audio->hw_connect_out == ZPL_MEDIA_CONNECT_SW && audio->output)
    {
        //zm_msg_debug(" audio channel %d frame sendto ao %d/%d \n", audio->channel, audio->output->devid, audio->output->channel);
        s32Ret = HI_MPI_AO_SendFrame(audio->output->devid, audio->output->channel, &stFrame, 1000);
    }*/
    //s32Ret = zpl_media_channel_skbuffer_frame_put(media_channel, ZPL_MEDIA_AUDIO, ZPL_MEDIA_FRAME_DATA_INPUT,
    //                                           0, stFrame.u64TimeStamp, (HI_S8*)stFrame.u64VirAddr[0], stFrame.u32Len); /// 1000U
    /* finally you must release the stream */
    s32Ret = HI_MPI_AI_ReleaseFrame(audio->devid, audio->channel, &stFrame, &stAecFrm);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio channel %d release stream failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    return 1;
#else
    return OK;
#endif
}

static int zpl_audhal_aidev_start(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    AIO_ATTR_S stAioAttr;
    AIO_ATTR_S *pstAioAttr = &stAioAttr;

    stAioAttr.enSamplerate   = audio->clock_rate;
    stAioAttr.enBitwidth     = audio->bits_per_sample;//AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = (audio->channel_cnt==2)?AUDIO_SOUND_MODE_STEREO:AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = audio->encode.codec.framerate;//缓存帧数量

    stAioAttr.u32PtNumPerFrm = audio->max_frame_size;//(u32PtNumPerFrm*1000)/ enSamplerate>=10
    stAioAttr.u32ChnCnt      = audio->channel_cnt;
    stAioAttr.u32ClkSel      = 0;
    stAioAttr.enI2sType      = AIO_I2STYPE_INNERCODEC;

    s32Ret = HI_MPI_AI_SetPubAttr(audio->devid, pstAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x(%s)\n", __FUNCTION__,
               audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AI_Enable(audio->devid);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_Enable(%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aidev_stop(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    s32Ret = HI_MPI_AI_Disable(audio->devid);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AI_Disable(%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}
/******************************************************************************
 * function : Start Aenc
 ******************************************************************************/
static int zpl_audhal_encode_create(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE

    HI_S32 s32Ret;
    AENC_CHN_ATTR_S stAencAttr;
    AENC_ATTR_ADPCM_S stAdpcmAenc;
    AENC_ATTR_G711_S stAencG711;
    AENC_ATTR_G726_S stAencG726;
    AENC_ATTR_LPCM_S stAencLpcm;
    AI_CHN_PARAM_S stAiChnPara;
    switch (audio->encode.codec.codectype)
    {
    case RTP_MEDIA_PAYLOAD_G711A:
        stAencAttr.enType = PT_G711A;
        break;
    case RTP_MEDIA_PAYLOAD_G711U:
        stAencAttr.enType = PT_G711U;
        break;
    case RTP_MEDIA_PAYLOAD_PCMA:
        stAencAttr.enType = PT_ADPCMA;
        break;
    case RTP_MEDIA_PAYLOAD_G726:
        stAencAttr.enType = PT_G726;
        break;
    case RTP_MEDIA_PAYLOAD_LPCM:
        stAencAttr.enType = PT_LPCM;
        break;
    }
    /* set AENC chn attr */
    stAencAttr.u32BufSize = audio->encode.codec.framerate;//音频编码缓存大小;
    stAencAttr.u32PtNumPerFrm = audio->encode.codec.max_frame_size; // 音频编码协议对应的帧长;

    if (PT_ADPCMA == stAencAttr.enType)
    {
        stAencAttr.pValue = &stAdpcmAenc;
        stAdpcmAenc.enADPCMType = ADPCM_TYPE_DVI4;
    }
    else if (PT_G711A == stAencAttr.enType || PT_G711U == stAencAttr.enType)
    {
        stAencAttr.pValue = &stAencG711;
    }
    else if (PT_G726 == stAencAttr.enType)
    {
        stAencAttr.pValue = &stAencG726;
        stAencG726.enG726bps = MEDIA_G726_40K;
    }
    else if (PT_LPCM == stAencAttr.enType)
    {
        stAencAttr.pValue = &stAencLpcm;
    }
    else
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("invalid audio payload type:%d\n", stAencAttr.enType);
        return HI_FAILURE;
    }

    /* create audio chn*/
    s32Ret = HI_MPI_AENC_CreateChn(audio->encode.channel, &stAencAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("HI_MPI_AENC_CreateChn(%d) failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    
    s32Ret = HI_MPI_AI_GetChnParam(audio->encode.devid, audio->encode.channel, &stAiChnPara);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("HI_MPI_AI_GetChnParam(%d) failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    stAiChnPara.u32UsrFrmDepth = audio->encode.codec.framerate/4;
    if(stAiChnPara.u32UsrFrmDepth > 30)
        stAiChnPara.u32UsrFrmDepth = 8;
    s32Ret = HI_MPI_AI_SetChnParam(audio->encode.devid, audio->encode.channel, &stAiChnPara);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("HI_MPI_AI_SetChnParam(%d) u32UsrFrmDepth=%d failed with:%s\n", audio->encode.channel, stAiChnPara.u32UsrFrmDepth, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
 * function : Stop Aenc
 ******************************************************************************/
static int zpl_audhal_encode_destroy(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AENC_DestroyChn(audio->encode.channel);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("HI_MPI_AENC_DestroyChn(%d) failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_encode_update_fd(zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    if (audio->encode.channel >= 0 && audio->encode.fd != ZPL_SOCKET_INVALID)
    {
        ipstack_type(audio->encode.fd) = IPSTACK_OS;
        ipstack_fd(audio->encode.fd) = HI_MPI_AENC_GetFd(audio->encode.channel);
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio encode channel %d fd %d\n", audio->encode.channel, ipstack_fd(audio->encode.fd));
        return OK;
    }
    return OK;
#else
    return OK;
#endif
}

static int zpl_audhal_encode_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    int s32Ret = 0;
    AUDIO_STREAM_S stStream;
    zpl_audio_frame_hdr_t *framehdr = NULL;
    memset(&stStream, 0, sizeof(stStream));
    zpl_video_assert(audio);
    /* get stream from audio chn */
    s32Ret = HI_MPI_AENC_GetStream(audio->encode.channel, &stStream, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio get audio channel %d stream failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    //zm_msg_debug(" audio encode channel %d frame read \n", audio->encode.channel);
    framehdr = (zpl_audio_frame_hdr_t *)stStream.pStream;

    //zm_msg_debug(" audio encode channel %d frame read %d  %d(0x%02x 0x%02x 0x%02x 0x%02x)\n", 
    //    audio->encode.channel, framehdr->len, stStream.u32Len, stStream.pStream[0], stStream.pStream[1],stStream.pStream[2],stStream.pStream[3]);

    s32Ret = zpl_media_channel_skbuffer_frame_put(media_channel, ZPL_MEDIA_AUDIO, ZPL_MEDIA_FRAME_DATA_ENCODE,
                                               0, stStream.u64TimeStamp, (char*)(stStream.pStream+4), framehdr->len); /// 1000U
    
    /* finally you must release the stream */
    s32Ret = HI_MPI_AENC_ReleaseStream(audio->encode.channel, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio release audio channel %d stream failed with:%s\n", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    return 1;
#else
    return OK;
#endif
}

static int zpl_audhal_encode_frame_sendto(zpl_audio_input_t *audio, void *p, void *p2)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_AENC_SendFrame(audio->encode.channel, p, p2);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err(" Frame sendto AENC Channel (%d) failed(%s)", audio->encode.channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}
/*
7.9.1 gpio number 计算
在 hi3516dv300 soc 内部，io 的命名是 gpion_m，那么 gpio number 的计算
gpio_num=n * 8 + m
那么 SPK_ON 的 gpio number 就是 83
echo 83 > /sys/class/gpio/export # 导出 gpio 83
echo out > /sys/class/gpio/gpio83/direction # 配置 gpio 为输出功能
echo 1 > /sys/class/gpio/gpio83/value # 配置 gpio 输出高电平，0 为低电平

himm 0x111F0030 0X000004F1
himm 0x120DA400 0X08
himm 0x120DA000 0X08

echo 83 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio83/direction
echo 1 > /sys/class/gpio/gpio83/value

echo 84 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio84/direction
echo 0 > /sys/class/gpio/gpio84/value
*/
/*********************************************************/

static int zpl_audhal_decode_create(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_ATTR_ADPCM_S stAdpcm;
    ADEC_ATTR_G711_S stAdecG711;
    ADEC_ATTR_G726_S stAdecG726;
    ADEC_ATTR_LPCM_S stAdecLpcm;
    switch (audio->decode.codec.codectype)
    {
    case RTP_MEDIA_PAYLOAD_G711A:
        stAdecAttr.enType = PT_G711A;
        break;
    case RTP_MEDIA_PAYLOAD_G711U:
        stAdecAttr.enType = PT_G711U;
        break;
    case RTP_MEDIA_PAYLOAD_PCMA:
        stAdecAttr.enType = PT_ADPCMA;
        break;
    case RTP_MEDIA_PAYLOAD_G726:
        stAdecAttr.enType = PT_G726;
        break;
    case RTP_MEDIA_PAYLOAD_LPCM:
        stAdecAttr.enType = PT_LPCM;
        break;
    case RTP_MEDIA_PAYLOAD_MP3:
        stAdecAttr.enType = PT_MP3;
        break;
    }
    stAdecAttr.u32BufSize = 20;
    stAdecAttr.enMode = ADEC_MODE_PACK; // ADEC_MODE_STREAM;/* propose use pack mode in your app */

    if (PT_ADPCMA == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdpcm;
        stAdpcm.enADPCMType = ADPCM_TYPE_DVI4;
    }
    else if (PT_G711A == stAdecAttr.enType || PT_G711U == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG711;
    }
    else if (PT_G726 == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG726;
        stAdecG726.enG726bps = MEDIA_G726_40K;
    }
    else if (PT_LPCM == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecLpcm;
        stAdecAttr.enMode = ADEC_MODE_PACK; /* lpcm must use pack mode */
    }
    else
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("invalid audio payload type:%d\n", stAdecAttr.enType);
        return HI_FAILURE;
    }

    /* create adec chn*/
    s32Ret = HI_MPI_ADEC_CreateChn(audio->decode.channel, &stAdecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("HI_MPI_ADEC_CreateChn(%d) failed with:%s\n", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_decode_destroy(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;

    s32Ret = HI_MPI_ADEC_DestroyChn(audio->decode.channel);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err("HI_MPI_ADEC_DestroyChn(%d) failed with:%s\n", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_decode_frame_recvfrom(void *media_channel, zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret = 0;
    AUDIO_FRAME_INFO_S stStream;

    memset(&stStream, 0, sizeof(stStream));
    zpl_video_assert(audio);

    s32Ret = HI_MPI_ADEC_GetFrame(audio->decode.channel, &stStream, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio get adec channel %d stream failed with:%s\n", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    //s32Ret = zpl_media_channel_skbuffer_frame_put(media_channel, ZPL_MEDIA_AUDIO, ZPL_MEDIA_FRAME_DATA_ENCODE,
    //                                           0, stStream.u64TimeStamp, stStream.pStream, stStream.u32Len); /// 1000U
    /* finally you must release the stream */
    s32Ret = HI_MPI_ADEC_ReleaseFrame(audio->decode.channel, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, DETAIL))
            zm_msg_debug(" audio release adec channel %d stream failed with:%s\n", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return ERROR;
    }
    return 1;
#else
    return OK;
#endif
}

static int zpl_audhal_decode_frame_sendto(zpl_audio_output_t *audio, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_ADEC_SendStream(audio->decode.channel, p, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err(" Frame sendto adec Channel (%d) failed(%s)", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_decode_frame_finsh(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_ADEC_SendEndOfStream(audio->decode.channel, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err(" Frame finsh adec Channel (%d) failed(%s)", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}



/******************************************************************************
 * function : Start Ao
 ******************************************************************************/
static int zpl_audhal_aochn_create(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AO_EnableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_EnableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    if (audio->clock_rate != audio->decode.codec.clock_rate)
    {
        audio->bResample = zpl_true;
        s32Ret = HI_MPI_AO_DisableReSmp(audio->devid, audio->channel);
        s32Ret |= HI_MPI_AO_EnableReSmp(audio->devid, audio->channel, audio->clock_rate);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
    }

    s32Ret = HI_MPI_AO_EnableChn(audio->devid, AO_SYSCHN_CHNID);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_EnableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    return HI_SUCCESS;
#else
    return OK;
#endif
}

/******************************************************************************
 * function : Stop Ao
 ******************************************************************************/
static int zpl_audhal_aochn_destroy(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if (HI_TRUE == audio->bResample)
    {
        s32Ret = HI_MPI_AO_DisableReSmp(audio->devid, audio->channel);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AO_DisableReSmp failed with %#x(%s)\n", __FUNCTION__, s32Ret, zpl_syshal_strerror(s32Ret));
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AO_DisableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_DisableChn failed with %#x(%s)\n", __FUNCTION__, s32Ret, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }

    s32Ret = HI_MPI_AO_DisableChn(audio->devid, AO_SYSCHN_CHNID);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_DisableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return s32Ret;
    }

    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aochn_VqeAttr(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if(audio)
    {
        AO_VQE_CONFIG_S stVqeConfig;
        memset(&stVqeConfig, 0, sizeof(AO_VQE_CONFIG_S));
        s32Ret = HI_MPI_AO_GetVqeAttr(audio->devid, audio->channel, &stVqeConfig);
        stVqeConfig.u32OpenMask = AO_VQE_MASK_HPF | AO_VQE_MASK_ANR | AO_VQE_MASK_AGC |AO_VQE_MASK_EQ;
        stVqeConfig.s32WorkSampleRate = audio->clock_rate;
        stVqeConfig.s32FrameSample = audio->max_frame_size;
        stVqeConfig.enWorkstate = VQE_WORKSTATE_COMMON ;

        //去低频,表现为轰轰不舒适声音
        stVqeConfig.stHpfCfg.bUsrMode = HI_TRUE;
        stVqeConfig.stHpfCfg.enHpfFreq = AUDIO_HPF_FREQ_120;


        //去除外界噪音
        stVqeConfig.stAnrCfg.bUsrMode = HI_TRUE;
        stVqeConfig.stAnrCfg.s16NrIntensity = 15;//[0~25]越大降噪力度越高,损伤越高.
        stVqeConfig.stAnrCfg.s16NoiseDbThr = 45;//[30~60]越大，检测力度越弱,声音更平滑
        stVqeConfig.stAnrCfg.s8SpProSwitch = 1;//[0/1]是否开启对音乐细节检测,喧闹场景不建议开

        //AGC 更多是放大输入源的声音

        stVqeConfig.stAgcCfg.s8TargetLevel = -2;
        stVqeConfig.stAgcCfg.s8NoiseFloor = -40;
        stVqeConfig.stAgcCfg.s8MaxGain = 15;
        stVqeConfig.stAgcCfg.s8AdjustSpeed = 10;
        stVqeConfig.stAgcCfg.s8ImproveSNR = 2;//上限6db
        stVqeConfig.stAgcCfg.s8UseHighPassFilt = 0;
        stVqeConfig.stAgcCfg.s8OutputMode = 0;
        stVqeConfig.stAgcCfg.s16NoiseSupSwitch = 1;//开启噪声抑制

        HI_MPI_AO_DisableVqe(audio->devid, audio->channel);
        s32Ret = HI_MPI_AO_SetVqeAttr(audio->devid, audio->channel, &stVqeConfig);
        if (HI_SUCCESS != s32Ret)
        {
            zm_msg_err("%s: HI_MPI_AO_SetVqeAttr(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
            return HI_FAILURE;
        }
        HI_MPI_AO_EnableVqe(audio->devid, audio->channel);
    }
    else
    {
        HI_MPI_AO_DisableVqe(audio->devid, audio->channel);
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aochn_start(zpl_audio_output_t *audio, int start)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    if(start)
    {
        s32Ret = HI_MPI_AO_EnableChn(audio->devid, audio->channel);
        HI_MPI_AO_EnableVqe(audio->devid, audio->channel);
        zpl_audhal_aochn_VqeAttr(audio);
    }
    else
        s32Ret = HI_MPI_AO_DisableChn(audio->devid, audio->channel);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_DisableChn(%d) failed with %#x(%s)\n", __FUNCTION__, audio->channel, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}


static int zpl_audhal_aochn_frame_sendto(zpl_audio_output_t *audio, void *p)
{
#ifdef ZPL_HISIMPP_MODULE
    zpl_int32 s32Ret;
    s32Ret = HI_MPI_AO_SendFrame(audio->devid, audio->channel, p, 1000);
    //s32Ret = HI_MPI_ADEC_SendStream(audio->decode.channel, p, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        if (ZPL_MEDIA_DEBUG(ENCODE, EVENT) && ZPL_MEDIA_DEBUG(ENCODE, HARDWARE))
            zm_msg_err(" Frame sendto audio output Channel (%d) failed(%s)", audio->decode.channel, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}
static int zpl_audhal_aodev_start(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    AIO_ATTR_S stAioAttr;
    AIO_ATTR_S *pstAioAttr = &stAioAttr;

    stAioAttr.enSamplerate   = audio->clock_rate;
    stAioAttr.enBitwidth     = audio->bits_per_sample;//AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = (audio->channel_cnt==2)?AUDIO_SOUND_MODE_STEREO:AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = audio->decode.codec.framerate;
    stAioAttr.u32PtNumPerFrm = audio->max_frame_size;
    stAioAttr.u32ChnCnt      = audio->channel_cnt;
    stAioAttr.u32ClkSel      = 0;
    stAioAttr.enI2sType      = AIO_I2STYPE_INNERCODEC;

    s32Ret = HI_MPI_AO_SetPubAttr(audio->devid, pstAioAttr);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x(%s)\n", __FUNCTION__,
               audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AO_Enable(audio->devid);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_Enable(%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}

static int zpl_audhal_aodev_stop(zpl_audio_output_t *audio)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    s32Ret = HI_MPI_AO_Disable(audio->devid);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_Disable(%d) failed with %#x(%s)\n", __FUNCTION__, audio->devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}


/******************************************************************************************/
int zpl_audhal_audio_input_create(zpl_audio_input_t *audio)
{
    int ret = 0;
    ret = zpl_audhal_aidev_start(audio);
    if(ret == OK)
    {
        ret = zpl_audhal_aichn_create(audio);
        if(ret == OK)
        {
            if(audio->encode.bEnable)
                return zpl_audhal_encode_create(audio);
            else
                return OK;   
        }
        else
            return ERROR;
    }
    return ERROR;    
}

int zpl_audhal_audio_input_destroy(zpl_audio_input_t *audio)
{
    int ret = 0;
    if(audio->encode.bEnable)
        ret = zpl_audhal_encode_destroy(audio);
    if(ret == OK)
    {
        ret = zpl_audhal_aichn_destroy(audio);
        ret |= zpl_audhal_aidev_stop(audio);
        return ret;
    }
    return ERROR;   
}

int zpl_audhal_audio_input_start(zpl_audio_input_t *audio)
{
    return zpl_audhal_aichn_start(audio, 1);
}
int zpl_audhal_audio_input_stop(zpl_audio_input_t *audio)
{
    return zpl_audhal_aichn_start(audio, 0);
}
int zpl_audhal_audio_input_update_fd(zpl_audio_input_t *audio)
{
    int ret = 0;
    ret = zpl_audhal_encode_update_fd(audio);  
    ret = zpl_audhal_aichn_update_fd(audio);
    return ret;    
}

int zpl_audhal_audio_input_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio)
{
    int ret = 0;
    ret = zpl_audhal_aichn_frame_recvfrom(media_channel, audio);
    return ret; 
}
int zpl_audhal_audio_encode_frame_recvfrom(void *media_channel, zpl_audio_input_t *audio)
{
    int ret = 0;
    ret = zpl_audhal_encode_frame_recvfrom(media_channel, audio);
    return ret; 
}

int zpl_audhal_audio_encode_frame_sendto(void *media_channel, zpl_audio_input_t *audio, void *p)/*向编码单元发送数据*/
{
    return zpl_audhal_encode_frame_sendto(audio, p, NULL);
}

int zpl_audhal_audio_frame_forward_hander(zpl_audio_input_t *audio, char *p, char *p2, int ti)/*向编码单元发送数据*/
{
    return zpl_audhal_aichn_frame_handle(audio, p, p2, ti);
}

int zpl_audhal_audio_output_create(zpl_audio_output_t *audio)
{
    int ret = 0;
    ret = zpl_audhal_aodev_start(audio);
    if(ret == OK)
    {
        ret = zpl_audhal_aochn_create(audio);
        if(ret == OK)
        {
            if(audio->decode.bEnable)
                return zpl_audhal_decode_create(audio);
            else
                return OK;    
        }
        else
            return ERROR;
    }
    return ERROR;     
}
int zpl_audhal_audio_output_destroy(zpl_audio_output_t *audio)
{
    int ret = 0;
    if(audio->decode.bEnable)
        ret = zpl_audhal_decode_destroy(audio);
    if(ret == OK)
    {
        ret = zpl_audhal_aochn_destroy(audio);
        ret |= zpl_audhal_aodev_stop(audio);
        return ret;
    }
    return ERROR; 
}
int zpl_audhal_audio_output_start(zpl_audio_output_t *audio)
{
    return zpl_audhal_aochn_start(audio, 1);
}
int zpl_audhal_audio_output_stop(zpl_audio_output_t *audio)
{
    return zpl_audhal_aochn_start(audio, 0);
}
int zpl_audhal_audio_decode_finsh(zpl_audio_output_t *audio)
{
    return zpl_audhal_decode_frame_finsh(audio);
}
int zpl_audhal_audio_output_frame_sendto(void *media_channel, zpl_audio_output_t *audio, void *p)/*向解码单元发送数据*/
{
    return zpl_audhal_aochn_frame_sendto(audio, p);
}
int zpl_audhal_audio_decode_frame_sendto(void *media_channel, zpl_audio_output_t *audio, void *p)/*向解码单元发送数据*/
{
    return zpl_audhal_decode_frame_sendto(audio, p);
}
int zpl_audhal_audio_decode_frame_recv(void *media_channel, zpl_audio_output_t *audio, void *p)/*向解码单元读取数据*/
{
    return zpl_audhal_decode_frame_recvfrom(audio, p);
}

int zpl_audhal_audio_output_volume(zpl_media_audio_channel_t *audio, int val)
{
#ifdef ZPL_HISIMPP_MODULE
    HI_S32 s32Ret;
    HI_S32 voleme = val-94;
    s32Ret = HI_MPI_AO_SetVolume(audio->audio_param.output.devid, voleme);
    if (HI_SUCCESS != s32Ret)
    {
        zm_msg_err("%s: HI_MPI_AO_SetVolume(%d) failed with %#x(%s)\n", __FUNCTION__, audio->audio_param.output.devid, s32Ret, zpl_syshal_strerror(s32Ret));
        return HI_FAILURE;
    }
    return HI_SUCCESS;
#else
    return OK;
#endif
}
/******************************************************************************************/
/*********************************************************/
int zpl_audhal_audio_codec_clock_rate(zpl_media_audio_channel_t *audio, int enSample)
{
    #ifdef ZPL_HISIMPP_MODULE
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    ACODEC_FS_E i2s_fs_sel = 0;
    int iAcodecInputVol = 0;
    ACODEC_MIXER_E input_mode = 0;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        zm_msg_err("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
    {
        zm_msg_err("Reset audio codec error\n");
    }

    switch (enSample)
    {
        case AUDIO_SAMPLE_RATE_8000:
            i2s_fs_sel = ACODEC_FS_8000;
            break;

        case AUDIO_SAMPLE_RATE_16000:
            i2s_fs_sel = ACODEC_FS_16000;
            break;

        case AUDIO_SAMPLE_RATE_32000:
            i2s_fs_sel = ACODEC_FS_32000;
            break;

        case AUDIO_SAMPLE_RATE_11025:
            i2s_fs_sel = ACODEC_FS_11025;
            break;

        case AUDIO_SAMPLE_RATE_22050:
            i2s_fs_sel = ACODEC_FS_22050;
            break;

        case AUDIO_SAMPLE_RATE_44100:
            i2s_fs_sel = ACODEC_FS_44100;
            break;

        case AUDIO_SAMPLE_RATE_12000:
            i2s_fs_sel = ACODEC_FS_12000;
            break;

        case AUDIO_SAMPLE_RATE_24000:
            i2s_fs_sel = ACODEC_FS_24000;
            break;

        case AUDIO_SAMPLE_RATE_48000:
            i2s_fs_sel = ACODEC_FS_48000;
            break;

        case AUDIO_SAMPLE_RATE_64000:
            i2s_fs_sel = ACODEC_FS_64000;
            break;

        case AUDIO_SAMPLE_RATE_96000:
            i2s_fs_sel = ACODEC_FS_96000;
            break;

        default:
            zm_msg_err("%s: not support enSample:%d\n", __FUNCTION__, enSample);
            close(fdAcodec);
            return ret;
            break;
    }

    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
    {
        zm_msg_err("%s: set acodec sample rate failed\n", __FUNCTION__);
        ret = HI_FAILURE;
    }

    input_mode = ACODEC_MIXER_IN1;
    if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &input_mode))
    {
        zm_msg_err("%s: select acodec input_mode failed\n", __FUNCTION__);
        ret = HI_FAILURE;
    }
    close(fdAcodec);
    return ret;
#else
    return OK;
#endif
}

int zpl_audhal_audio_codec_input_volume(zpl_media_audio_channel_t *audio, int val)
{
    #ifdef ZPL_HISIMPP_MODULE
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    int iAcodecInputVol = val;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        zm_msg_err("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }

    if (1) /* should be 1 when micin */
    {
        /******************************************************************************************
        The input volume range is [-87, +86]. Both the analog gain and digital gain are adjusted.
        A larger value indicates higher volume.
        For example, the value 86 indicates the maximum volume of 86 dB,
        and the value -87 indicates the minimum volume (muted status).
        The volume adjustment takes effect simultaneously in the audio-left and audio-right channels.
        The recommended volume range is [+10, +56].
        Within this range, the noises are lowest because only the analog gain is adjusted,
        and the voice quality can be guaranteed.
        *******************************************************************************************/
        if (ioctl(fdAcodec, ACODEC_SET_INPUT_VOL, &iAcodecInputVol))
        {
            zm_msg_err("%s: audio codec set input valume  failed\n", __FUNCTION__);
            return HI_FAILURE;
        }

    }
    close(fdAcodec);
    return ret;
#else
    return OK;
#endif
}

int zpl_audhal_audio_codec_output_volume(zpl_media_audio_channel_t *audio, int val)
{
    #ifdef ZPL_HISIMPP_MODULE
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    int iAcodecOutputVol = val;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        zm_msg_err("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_SET_OUTPUT_VOL, &iAcodecOutputVol))
    {
        zm_msg_err("audio codec set output valume error\n");
    }
    close(fdAcodec);
    return ret;
#else
    return OK;
#endif
}
int zpl_audhal_audio_codec_mic_gain_val(zpl_media_audio_channel_t *audio, int val)
{
    #ifdef ZPL_HISIMPP_MODULE
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    int iAcodecVol = val;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        zm_msg_err("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &iAcodecVol))
    {
        zm_msg_err("audio codec set gain micl error\n");
    }
    if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &iAcodecVol))
    {
        zm_msg_err("audio codec set gain micr error\n");
    }
    close(fdAcodec);
    return ret;
#else
    return OK;
#endif
}
int zpl_audhal_audio_codec_boost_val(zpl_media_audio_channel_t *audio, int val)
{
    #ifdef ZPL_HISIMPP_MODULE
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    int iAcodecVol = val;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0)
    {
        zm_msg_err("%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_ENABLE_BOOSTL, &iAcodecVol))
    {
        zm_msg_err("audio codec set boostl error\n");
    }
    if (ioctl(fdAcodec, ACODEC_ENABLE_BOOSTR, &iAcodecVol))
    {
        zm_msg_err("audio codec set boostr error\n");
    }
    close(fdAcodec);
    return ret;
#else
    return OK;
#endif
}


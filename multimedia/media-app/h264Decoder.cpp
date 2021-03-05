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

#include "h264Decoder.hpp"

h264Decoder::h264Decoder():videoDecoder()
{
}

h264Decoder::~h264Decoder()
{
    videoDecoderDestroy();
}

int h264Decoder::videoDecoderSetup(const int width, const int height, const int fmt, const int fps)
{
    this->m_width = width;
    this->m_height = height;
    this->m_fmt = fmt;
    this->m_fps = fps;
#ifdef PL_OPENH264_MODULE
    return openh264_decoder_setup(width, height, fmt, fps);
#endif
}

int h264Decoder::videoDecoderInput(const ospl_uint8 *frame, const int len)
{
#ifdef PL_OPENH264_MODULE
    return openh264_decoder_input(frame, len);
#endif
}

int h264Decoder::videoDecoderOutput(ospl_uint8 *frame, const int len)
{
#ifdef PL_OPENH264_MODULE
    return openh264_decoder_output(frame, len);
#endif
}

ospl_uint8 *h264Decoder::videoDecoderOutput()
{
#ifdef PL_OPENH264_MODULE
    return (ospl_uint8 *)m_out_frame_payload;
#endif
}

int h264Decoder::videoDecoderOutputSize(const bool clear)
{
    int ret = 0;
    ret = (int)m_out_size;
    if (clear)
        m_out_size = 0;
    return ret;
}

int h264Decoder::videoDecoderDestroy()
{
#ifdef PL_OPENH264_MODULE
    openh264_decoder_destroy();
#endif
    return 0;
}

#ifdef PL_OPENH264_MODULE

int h264Decoder::openh264_decoder_destroy()
{
    if (m_decoder)
        WelsDestroyDecoder(m_decoder);
    return 0;
}

int h264Decoder::openh264_decoder_setup(const int width, const int height, const int fmt, const int fps)
{
    /* decoder allocation */
    SDecodingParam param = {0};
    if (WelsCreateDecoder(&m_decoder))
    {
        printf("Unable to create decoder\n");
        return -1;
    }

    /*
     * Decoder
     */
#if !OPENH264_VER_AT_LEAST(1, 6)
    //param.eOutputColorFormat = videoFormatI420;
#endif
    param.sVideoProperty.size = sizeof(param.sVideoProperty);
    param.uiTargetDqLayer = (ospl_uint8)-1;
    param.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

    //param.eEcActiveIdc       = ERROR_CON_DISABLE;
    //param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

    if (m_decoder->Initialize(&param) != cmResultSuccess)
    {
        printf("Initialize failed\n");
        return -1;
    }

    return 0;
}

int h264Decoder::openh264_write_yuv(ospl_uint8 *buf,
                                    const int dst_len,
                                    const ospl_uint8 *pData[3],
                                    const int *iStride,
                                    const int iWidth,
                                    const int iHeight)
{
    unsigned req_size;
    ospl_uint8 *dst = buf;
    ospl_uint8 *max = dst + dst_len;
    int i;
    ospl_uint8 *pPtr = NULL;
    int miWidth = 0;
    int miHeight = 0;
    req_size = (iWidth * iHeight) + (iWidth / 2 * iHeight / 2) +
               (iWidth / 2 * iHeight / 2);
    if (dst_len < req_size)
        return -1;

    pPtr = (ospl_uint8 *)pData[0];
    for (i = 0; i < iHeight && (dst + iWidth < max); i++)
    {
        memcpy(dst, pPtr, iWidth);
        pPtr += iStride[0];
        dst += iWidth;
    }

    if (i < iHeight)
        return -1;

    miHeight = iHeight / 2;
    miWidth = iWidth / 2;
    pPtr = (ospl_uint8 *)pData[1];
    for (i = 0; i < miHeight && (dst + miWidth <= max); i++)
    {
        memcpy(dst, pPtr, miWidth);
        pPtr += iStride[1];
        dst += miWidth;
    }

    if (i < miHeight)
        return -1;

    pPtr = (ospl_uint8 *)pData[2];
    for (i = 0; i < miHeight && (dst + miWidth <= max); i++)
    {
        memcpy(dst, pPtr, miWidth);
        pPtr += iStride[1];
        dst += miWidth;
    }

    if (i < miHeight)
        return -1;

    return dst - buf;
}

int h264Decoder::openh264_got_decoded_frame(
    const ospl_uint8 *pData[3],
    const SBufferInfo *sDstBufInfo,
    uint64_t *timestamp)
{
    ospl_uint8 *pDst[3] = {NULL};

    pDst[0] = (ospl_uint8 *)pData[0];
    pDst[1] = (ospl_uint8 *)pData[1];
    pDst[2] = (ospl_uint8 *)pData[2];

    /* Do not reset size as it may already contain frame
    output->size = 0;
    */

    if (!pDst[0] || !pDst[1] || !pDst[2])
    {
        return -1;
    }

    int iStride[2];
    int iWidth = sDstBufInfo->UsrData.sSystemBuffer.iWidth;
    int iHeight = sDstBufInfo->UsrData.sSystemBuffer.iHeight;

    iStride[0] = sDstBufInfo->UsrData.sSystemBuffer.iStride[0];
    iStride[1] = sDstBufInfo->UsrData.sSystemBuffer.iStride[1];

    int len = openh264_write_yuv((ospl_uint8*)m_out_frame_payload, (const int)m_out_size,
                                 (const ospl_uint8 **)pDst, (const int *)iStride, (const int)iWidth, (const int)iHeight);
    if (len > 0)
    {
        //output->timestamp = *timestamp;
        //output->size = len;
        //output->type = PJMEDIA_FRAME_TYPE_VIDEO;
        return len;
    }
    else
    {
        /* buffer is damaged, reset size */
        //output->size = 0;
        return -1;
    }
    return 0;
}

int h264Decoder::openh264_decoder_input(const ospl_uint8 *frame, const int len)
{
    SBufferInfo info = {0};
    ospl_uint8 *ptrs[3];
    int ret, linesize[3];
    DECODING_STATE state;

    if (!frame)
    {
#if OPENH264_VER_AT_LEAST(1, 9)
        int end_of_stream = 1;
        m_decoder->SetOption(m_decoder, DECODER_OPTION_END_OF_STREAM, &end_of_stream);
        state = m_decoder->FlushFrame(m_decoder, ptrs, &info);
#else
        return 0;
#endif
    }
    else
    {
        //info.uiInBsTimeStamp = avpkt->pts;
#if OPENH264_VER_AT_LEAST(1, 4)
        // Contrary to the name, DecodeFrameNoDelay actually does buffering
        // and reordering of frames, and is the recommended decoding entry
        // point since 1.4. This is essential for successfully decoding
        // B-frames.
        state = m_decoder->DecodeFrameNoDelay(m_decoder, frame, len, ptrs, &info);
#else
        state = m_decoder->DecodeFrame2((const ospl_uint8*)frame, len, ptrs, &info);
#endif
    }
    if (state != dsErrorFree)
    {
        printf("DecodeFrame failed\n");
        return -1;
    }
    if (info.iBufferStatus != 1)
    {
        printf("No frame produced\n");
        return len;
    }
    if (info.iBufferStatus == 1)
    {
        uint64_t ptimestamp;
        /* May overwrite existing frame but that's ok. */
        int status = openh264_got_decoded_frame((const ospl_uint8**)ptrs,
                                            (const SBufferInfo *)&info,
                                            (uint64_t *)&ptimestamp);
        if(status > 0)
            return status;                                    
        //has_frame = (status==PJ_SUCCESS && output->size != 0);
    }
    return 0;
}

int h264Decoder::openh264_decoder_output(ospl_uint8 *frame, const int len)
{
    if (m_out_size > 0 && m_out_frame_payload != nullptr)
    {
        int ret = (m_out_size > len) ? len : m_out_size;
        memmove(frame, m_out_frame_payload, ret);
        m_out_size = 0;
        return ret;
    }
    return -1;
}
#endif

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

#include "ffmpegDevice.hpp"

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/imgutils.h"
};

ffmpegDevice::ffmpegDevice()
{
}

int ffmpegDevice::ffmpegDeviceInit(int width, int height, int fps)
{
    this->v_width = width;
    this->v_height = height;
    this->v_fps = fps;
    av_register_all();
    avformat_network_init();
    m_AVInputCtx = avformat_alloc_context();
    //Register Device
    avdevice_register_all();
    return 0;
}

int ffmpegDevice::ffmpegDeviceOpen(const char *inputFormat, const char *ul)
{
    int ret = 0;
    char args[512];
    AVDictionary *options = NULL;
    snprintf(args, sizeof(args), "%d", this->v_fps);
    av_dict_set(&options, "framerate", args, 0);
    snprintf(args, sizeof(args), "%dx%d", this->v_width, this->v_height);
    av_dict_set(&options, "video_size", args, 0);
    //av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "rtbufsize", "40M", 0);

    AVInputFormat *m_AVInputFormat = av_find_input_format(/*"video4linux2"*/ inputFormat);
    ret = avformat_open_input(&m_AVInputCtx, ul /*"/dev/video0"*/, m_AVInputFormat, &options);
    if (ret != 0)
    {
        av_strerror(ret, args, sizeof(args));
        av_dict_free(&options);
        std::cout << "ffmpegDeviceOpen Couldn't open input stream->"
                  << args << std::endl;
        return -1;
    }
    av_dict_free(&options);
    // Initializes the video decoder
    if (avformat_find_stream_info(m_AVInputCtx, NULL) < 0)
    {
        std::cout << "ffmpegDeviceOpen Cannot find stream info" << std::endl;
        return -1;
    }
    return 0;
}

int ffmpegDevice::ffmpegDeviceDecoderOpen()
{
    int i = 0;
    for (i = 0; i < m_AVInputCtx->nb_streams; i++)
    {
        if (m_AVInputCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }
    if (videoindex == -1)
    {
        std::cout << "ffmpegDeviceOpen Couldn't find a video stream.（没有找到视频流）\n"
                  << std::endl;
        return -1;
    }
    //编解码上下文
    m_DecoderCodecCtx = m_AVInputCtx->streams[videoindex]->codec;
    //查找解码器
    m_DecoderCodec = avcodec_find_decoder(m_DecoderCodecCtx->codec_id);

    if (m_DecoderCodec == NULL)
    {
        std::cout << "ffmpegDeviceOpen Codec not found.\n"
                  << std::endl;
        return -1;
    }
    if (avcodec_open2(m_DecoderCodecCtx, m_DecoderCodec, NULL) < 0)
    {
        std::cout << "ffmpegDeviceOpen Could not open codec.（无法打开解码器）\n"
                  << std::endl;
        return -1;
    }
    return 0;
}

int ffmpegDevice::ffmpegDeviceReady()
{
    /*    
    //prepare before decode and encode
    dec_pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
	//camera data may has a pix fmt of RGB or sth else,convert it to YUV420
    m_ImgConvertCtx = sws_getContext(m_AVInputCtx->streams[videoindex]->codec->width, m_AVInputCtx->streams[videoindex]->codec->height,
        m_AVInputCtx->streams[videoindex]->codec->pix_fmt, m_EncoderCodecCtx->width, m_EncoderCodecCtx->height, AV_PIX_FMT_YUV422P, SWS_BICUBIC, NULL, NULL, NULL);
    
    //Initialize the buffer to store YUV frames to be encoded.
	pFrameYUV = av_frame_alloc();
    zpl_uint8 *out_buffer = (zpl_uint8 *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV422P, m_EncoderCodecCtx->width, m_EncoderCodecCtx->height));
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV422P, m_EncoderCodecCtx->width, m_EncoderCodecCtx->height);
*/

    //申请AVFrame，用于原始视频
    m_raw_frame = av_frame_alloc();
    //申请AVFrame，用于yuv视频
    pFrameYUV = av_frame_alloc();
    //分配内存，用于图像格式转换
    zpl_uint8 *out_buffer = (zpl_uint8 *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV422P, m_DecoderCodecCtx->width, m_DecoderCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV422P, m_DecoderCodecCtx->width, m_DecoderCodecCtx->height, 1);
    dec_pkt = (AVPacket *)av_malloc(sizeof(AVPacket));

    m_ImgConvertCtx = sws_getContext(m_DecoderCodecCtx->width, m_DecoderCodecCtx->height,
                                     m_DecoderCodecCtx->pix_fmt, m_DecoderCodecCtx->width, m_DecoderCodecCtx->height,
                                     AV_PIX_FMT_YUV422P, SWS_BICUBIC, NULL, NULL, NULL);

    m_raw_frame->format = m_DecoderCodecCtx->pix_fmt;
    m_raw_frame->width = m_DecoderCodecCtx->width;
    m_raw_frame->height = m_DecoderCodecCtx->height;

    return 0;
}

int ffmpegDevice::ffmpegDeviceGetFrame()
{
    int ret = 0;
    if ((ret = av_read_frame(m_AVInputCtx, dec_pkt)) >= 0)
    {
        if (dec_pkt->stream_index == videoindex)
        {
            if (m_raw_frame == nullptr)
                m_raw_frame = av_frame_alloc();
            //std::cout <<" ffmpegDeviceGetFrame : " << ret << std::endl;
            return ret;
        }
    }
    std::cout << " ffmpegDeviceGetFrame : -1" << std::endl;
    return -1;
}

int ffmpegDevice::ffmpegDeviceDecoderFrame()
{
    int dec_got_frame = 0;
    int ret = avcodec_decode_video2(m_AVInputCtx->streams[dec_pkt->stream_index]->codec, m_raw_frame,
                                    &dec_got_frame, dec_pkt);

    //int ret = avcodec_decode_video2(m_DecoderCodecCtx, m_raw_frame, &dec_got_frame, dec_pkt);

    if (ret < 0)
    {
        av_frame_free(&m_raw_frame);
        m_raw_frame = nullptr;
        av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
        std::cout << " ffmpegDeviceDecoderFrame : Decoding failed" << std::endl;
        return -1;
    }
    if (dec_got_frame)
    {
        sws_scale(m_ImgConvertCtx, (const zpl_uint8 *const *)m_raw_frame->data, m_raw_frame->linesize, 0,
                  m_DecoderCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
        pFrameYUV->width = m_DecoderCodecCtx->width;
        pFrameYUV->height = m_DecoderCodecCtx->height;
        pFrameYUV->format = AV_PIX_FMT_YUV422P;
        /*
        y_size = pCodecCtx->width*pCodecCtx->height;
		fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
		fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
		fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
        */
        return 0;
    }
    std::cout << " ffmpegDeviceDecoderFrame : sws_scale failed " << std::endl;
    return -1;
}

int ffmpegDevice::ffmpegDeviceDecoderFrameFinish()
{
    int dec_got_frame = 0;
    //flush decoder
    //FIX: Flush Frames remained in Codec
    while (true)
    {
        if (!(m_DecoderCodec->capabilities & AV_CODEC_CAP_DELAY))
            return 0;

        int ret = avcodec_decode_video2(m_DecoderCodecCtx, m_raw_frame, &dec_got_frame, dec_pkt);
        if (ret < 0)
        {
            break;
        }
        if (!dec_got_frame)
        {
            break;
        }

        sws_scale(m_ImgConvertCtx, (const zpl_uint8 *const *)m_raw_frame->data, m_raw_frame->linesize, 0, m_DecoderCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);

        //pFrameYUV->width = m_raw_frame->width;
        //pFrameYUV->height = m_raw_frame->height;
        //pFrameYUV->format = AV_PIX_FMT_YUV422P;
        /*
		int y_size = m_DecoderCodecCtx->width*m_DecoderCodecCtx->height;
		memmove(pFrameYUV->data[0], m_raw_frame->data, y_size);    //Y 
		memmove(pFrameYUV->data[1], m_raw_frame->data, y_size / 4);  //U
		memmove(pFrameYUV->data[2], m_raw_frame->data, y_size / 4);  //V*/
        printf("Flush Decoder: Succeed to decode 1 frame!\n");
    }
    return 0;
}

int ffmpegDevice::ffmpegDeviceDestroy()
{
    //if(dec_pkt)
    //    av_free_packet(dec_pkt);

    if (m_ImgConvertCtx)
        sws_freeContext(m_ImgConvertCtx);

    av_frame_free(&pFrameYUV);
    av_frame_free(&m_raw_frame);

    avcodec_close(m_DecoderCodecCtx);

    avformat_close_input(&m_AVInputCtx);

    //Clean
    //if (m_video_st)
    {
        //avcodec_close(m_video_st->codec);
        //av_free(pFrame);
        //av_free(picture_buf);
    }
    return 0;
}

#if 0
void Transcoder::initializeEncoder() {

        LOG(DEBUG) << "Initialize HEVC encoder";

        // allocate format context for an output format (null - no output file)
        int statCode = avformat_alloc_output_context2(&encoderContext.formatContext, nullptr, "null", nullptr);
        assert(statCode >= 0);

        encoderContext.codec = avcodec_find_encoder_by_name("libx265");
        assert(encoderContext.codec);

        // create new video output stream (dummy)
        encoderContext.videoStream = avformat_new_stream(encoderContext.formatContext, encoderContext.codec);
        assert(encoderContext.videoStream);
        encoderContext.videoStream->id = encoderContext.formatContext->nb_streams - 1;

        // create codec context (for each codec new codec context)
        encoderContext.codecContext = avcodec_alloc_context3(encoderContext.codec);
        assert(encoderContext.codecContext);

        // set up parameters
        encoderContext.codecContext->width = static_cast<int>(config.getOutputParams().getWidth());
        encoderContext.codecContext->height = static_cast<int>(config.getOutputParams().getHeight());

        encoderContext.codecContext->profile = FF_PROFILE_HEVC_MAIN;

        encoderContext.codecContext->time_base = (AVRational) {1, static_cast<int>(config.getOutputParams().getFrameRate().first)};
        encoderContext.codecContext->framerate = (AVRational) {static_cast<int>(config.getOutputParams().getFrameRate().first), 1};

        // set encoder's pixel format (it is advised to use yuv420p)
        encoderContext.codecContext->pix_fmt = encoderPixFormat;

        if (encoderContext.formatContext->flags & AVFMT_GLOBALHEADER) {
            encoderContext.codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        // copy encoder parameters to the video stream parameters
        avcodec_parameters_from_context(encoderContext.videoStream->codecpar, encoderContext.codecContext);

        AVDictionary *options = nullptr;

        // the faster you get, the less compression is achieved
        av_dict_set(&options, "preset", config.getEncoderParams().getPreset().c_str(), 0);

        // optimization for fast encoding and low latency streaming
        av_dict_set(&options, "tune", config.getEncoderParams().getTune().c_str(), 0);

        av_dict_set(&options, "b", lirs::utils::to_string_with_prefix(config.getEncoderParams().getBitrate(), "K").data(), 0);

        char x265_params[128];

        sprintf(x265_params, "vbv-maxrate=%d:vbv-bufsize=%d",
                config.getEncoderParams().getBitrate(), config.getEncoderParams().getVbvBufSize());

        LOG(INFO) << x265_params;

        // set additional codec options
        av_opt_set(encoderContext.codecContext->priv_data, "x265-params", x265_params, 0);

        // open the output format to use given codec
        statCode = avcodec_open2(encoderContext.codecContext, encoderContext.codec, &options);
        av_dict_free(&options);
        assert(statCode == 0);

        // initializes time base automatically
        statCode = avformat_write_header(encoderContext.formatContext, nullptr);
        assert(statCode >= 0);

        // report info to the console
        av_dump_format(encoderContext.formatContext, encoderContext.videoStream->index, "null", 1);

        // allocate encoding packet
        encodingPacket = av_packet_alloc();
        av_init_packet(encodingPacket);

    }
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
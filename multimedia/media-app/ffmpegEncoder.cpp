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

#include "ffmpegEncoder.hpp"

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

ffmpegEncoder::ffmpegEncoder()
{
}

int ffmpegEncoder::ffmpegEncoderInit(int width, int height, int fmt, int fps)
{
    this->m_width = width;
    this->m_height = height;
    this->m_fmt = fmt;
    this->m_fps = fps;
	return 0;
}

int ffmpegEncoder::ffmpegEncoderOpen(int enc /*, std::function <int(void*, int)> Callback*/)
{
	//onEencoderCallback = std::move(Callback);
	m_AVOutputCtx = avformat_alloc_context();
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
	const char *out_file = "output.h264";
	//猜测类型,返回输出类型。
	m_OutputFmt = av_guess_format(NULL, out_file, NULL);
#else
	m_OutputFmt = av_guess_format("h264", NULL, NULL);
#endif
	m_AVOutputCtx->oformat = m_OutputFmt;
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
	//打开FFmpeg的输入输出文件,成功之后创建的AVIOContext结构体。
	if (avio_open2(&m_AVOutputCtx->pb, out_file, AVIO_FLAG_READ_WRITE, NULL, NULL) < 0)
	{
		//Failed to open output file
		std::cout << "ffmpegEncoderOpen Failed to open output file\n"
				  << std::endl;
		return -1;
	}
#endif
	//AV_CODEC_ID_H264
	//除了以下方法，另外还可以使用avcodec_find_encoder_by_name()来获取AVCodec
	m_Codec = avcodec_find_encoder(m_OutputFmt->video_codec); //获取编码器
	if (!m_Codec)
	{
		//cannot find encoder
		std::cout << "ffmpegEncoderOpen cannot find encoder\n"
				  << std::endl;
		return -1;
	}

	m_CodecCtx = avcodec_alloc_context3(m_Codec); //申请AVCodecContext，并初始化。
	if (!m_CodecCtx)
	{
		//failed get AVCodecContext
		std::cout << "ffmpegEncoderOpen Failed to get AVCodecContext \n"
				  << std::endl;
		return -1;
	}

	/* 创建输出码流的AVStream */
	m_video_st = avformat_new_stream(m_AVOutputCtx, 0);
	m_video_st->time_base.num = 1;
	m_video_st->time_base.den = this->m_fps;
	if (m_video_st == NULL)
	{
		return -1;
	}

	//Param that must set
	m_CodecCtx = m_video_st->codec;
	//pCodecCtx->codec_id =AV_CODEC_ID_HEVC;  //H265
	m_CodecCtx->codec_id = AV_CODEC_ID_H264;
	m_CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	//openh264 -> AV_PIX_FMT_YUV420P
	//libx264 -> AV_PIX_FMT_YUV422P
#ifdef PL_LIBX264_MODULE
	m_CodecCtx->pix_fmt = AV_PIX_FMT_YUV422P;//AV_PIX_FMT_YUYV422;//AV_PIX_FMT_YUV422P;
#endif
#ifdef PL_OPENH264_MODULE
	m_CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//AV_PIX_FMT_YUYV422;//AV_PIX_FMT_YUV422P;
#endif
	m_CodecCtx->width = this->m_width;
	m_CodecCtx->height = this->m_height;
	m_CodecCtx->b_frame_strategy = true;
	/*
	码率
	bit_rate/-bt tolerance 设置视频码率容忍度kbit/s （固定误差）
	rc_max_rate/-maxrate bitrate设置最大视频码率容忍度 （可变误差）
	rc_min_rate/-minrate bitreate 设置最小视频码率容忍度（可变误差）
	rc_buffer_size/-bufsize size 设置码率控制缓冲区大小
	如何设置固定码率编码 ?
	bit_rate是平均码率，不一定能控制住
	c->bit_rate = 400000;
	c->rc_max_rate = 400000;
	c->rc_min_rate = 400000;
	提示  [libx264 @ 00c70be0] VBV maxrate specified, but no bufsize, ignored
	再设置  c->rc_buffer_size = 200000;  即可。如此控制后编码质量明显差了。
	*/
	m_CodecCtx->bit_rate = 400000; //采样码率越大，目标文件越大
	//pCodecCtx->bit_rate_tolerance = 8000000; // 码率误差，允许的误差越大，视频越小
	//两个I帧之间的间隔
	m_CodecCtx->gop_size = 15;

	//编码帧率，每秒多少帧。下面表示1秒25帧
	m_CodecCtx->time_base.num = 1;
	m_CodecCtx->time_base.den = this->m_fps;

	//最小的量化因子
	m_CodecCtx->qmin = 10;
	//最大的量化因子
	m_CodecCtx->qmax = 30;

	//最大B帧数
	m_CodecCtx->max_b_frames = 3;

	// Set Option
	AVDictionary *param = 0;
	//H.264
	if (m_CodecCtx->codec_id == AV_CODEC_ID_H264)
	{
		std::cout << "--------------- AV_CODEC_ID_H264 ----------------\n"
				  << std::endl;
		/*
		preset的参数主要调节编码速度和质量的平衡，有ultrafast、superfast、veryfast、faster、fast、medium、slow、slower、veryslow、placebo这10个选项，从快到慢。
		*/
		av_dict_set(&param, "preset", "fast", 0);
		/* 
		tune的参数主要配合视频类型和视觉优化的参数。
		tune的值有： film：  电影、真人类型；
		animation：  动画；
		grain：      需要保留大量的grain时用；
		stillimage：  静态图像编码时使用；
		psnr：      为提高psnr做了优化的参数；
		ssim：      为提高ssim做了优化的参数；
		fastdecode： 可以快速解码的参数；
		zerolatency：零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码。
		*/
		av_dict_set(&param, "tune", "zerolatency", 0);
		/*
		画质,分别是baseline, extended, main, high
		1、Baseline Profile：基本画质。支持I/P 帧，只支持无交错（Progressive）和CAVLC；
		2、Extended profile：进阶画质。支持I/P/B/SP/SI 帧，只支持无交错（Progressive）和CAVLC；(用的少)
		3、Main profile：主流画质。提供I/P/B 帧，支持无交错（Progressive）和交错（Interlaced）， 也支持CAVLC 和CABAC 的支持；
		4、High profile：高级画质。在main Profile 的基础上增加了8x8内部预测、自定义量化、 无损视频编码和更多的YUV 格式；
　　	H.264 Baseline profile、Extended profile和Main profile都是针对8位样本数据、4:2:0格式(YUV)的视频序列。在相同配置情况下，
		High profile（HP）可以比Main profile（MP）降低10%的码率。 根据应用领域的不同，Baseline profile多应用于实时通信领域，
		Main profile多应用于流媒体领域，High profile则多应用于广电和存储领域。
		*/
		//av_dict_set(¶m, "profile", "main", 0);
	}
	else if (m_CodecCtx->codec_id == AV_CODEC_ID_H265 || m_CodecCtx->codec_id == AV_CODEC_ID_HEVC)
	{
		av_dict_set(&param, "preset", "fast", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
	//Output Info-----------------------------
	std::cout << "--------------- out_file Information ----------------\n"
			  << std::endl;
	//手工调试函数，输出tbn、tbc、tbr、PAR、DAR的含义
	av_dump_format(m_AVOutputCtx, 0, out_file, 1); //最后一个参数，如果是输出文件时，该值为1；如果是输入文件时，该值为0
	std::cout << "-----------------------------------------------------\n"
			  << std::endl;
#endif
	/* 查找编码器 */
	/*	m_Codec = avcodec_find_encoder(m_CodecCtx->codec_id);
	if (!m_Codec) 
	{
		std::cout <<"Can not find encoder! \n"<< std::endl;
		return -1;
	}
 */
	/* 打开编码器 */
	if (avcodec_open2(m_CodecCtx, m_Codec, &param) < 0)
	{
		std::cout << "Failed to open output video encoder! (jie码器打开失败！)\n"
				  << std::endl;
		return -1;
	}

	//pFrame = av_frame_alloc();
	int picture_size = avpicture_get_size(m_CodecCtx->pix_fmt, m_CodecCtx->width, m_CodecCtx->height);
	//picture_buf = (uint8_t *)av_malloc(picture_size);
	//avpicture_fill((AVPicture *)pFrame, picture_buf, m_CodecCtx->pix_fmt, m_CodecCtx->width, m_CodecCtx->height);
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
	//Write File Header
	/* 写文件头（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS） */
	avformat_write_header(m_AVOutputCtx, NULL);
#endif
	/* Allocate the payload of a packet and initialize its fields with default values.  */
	//av_new_packet(&enc_pkt, picture_size);

	enc_count = 1;
	//y_size = pCodecCtx->width * pCodecCtx->height;

	return 0;
}

int ffmpegEncoder::ffmpegEncoderFrame(AVFrame *input, AVPacket *out)
{
	int enc_got_frame = 0;
	//enc_pkt.data = NULL;
	//enc_pkt.size = 0;
	//AVRational time_base_q = { 1, AV_TIME_BASE };
	//av_init_packet(&enc_pkt);
	//PTS
	input->pts = enc_count * (m_video_st->time_base.den) / ((m_video_st->time_base.num) * this->m_fps);

	int ret = avcodec_encode_video2(m_CodecCtx, out, input, &enc_got_frame);
	if (enc_got_frame >= 1)
	{

		framecnt++;
		//printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt, out->size);

		out->stream_index = m_video_st->index;
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
		/* 将编码后的视频码流写入文件 */
		ret = av_write_frame(m_AVOutputCtx, out);
		/*statusCode = encode(encoderContext.codecContext, convertedFrame, encodingPacket);
        if (statusCode >= 0) {
            // new encoded data is available (one NALU)
            if (onEncodedDataCallback) {
                NALU_START_CODE_BYTES_NUMBER=4
                onEncodedDataCallback(std::vector<uint8_t>(encodingPacket->data + NALU_START_CODE_BYTES_NUMBER,
                                                         encodingPacket->data + encodingPacket->size));
            }
        }*/
#else
		//if(onEencoderCallback)
		//    (onEencoderCallback)(out->data, out->size);
#endif
		//av_free_packet(&enc_pkt);
		enc_count++;
		return 0;
	}
	enc_count++;
	std::cout << " ffmpegEncoderFrame : avcodec_encode_video2 failed ret:" << ret << " enc_got_frame:" << enc_got_frame << std::endl;
	return -1;
}

int ffmpegEncoder::ffmpegEncoderFrameFinish()
{
	AVPacket enc_pkt;
	int enc_got_frame = 0;
	//flush decoder
	if (!(m_AVOutputCtx->streams[0]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
	{
		return 0;
	}
	while (true)
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		int ret = avcodec_encode_video2(m_AVOutputCtx->streams[0]->codec, &enc_pkt, NULL, &enc_got_frame);
		av_frame_free(NULL);
		if (ret < 0)
		{
			break;
		}
		if (!enc_got_frame)
		{
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
		/* mux encoded frame */
		ret = av_write_frame(m_AVOutputCtx, &enc_pkt);
		if (ret < 0)
		{
			break;
		}
#endif
	}
	av_free_packet(&enc_pkt);
	av_write_trailer(m_AVOutputCtx);
	return 0;
}

int ffmpegEncoder::ffmpegEncoderDestroy()
{
	//av_free_packet(&enc_pkt);
	avcodec_close(m_CodecCtx);

	//Clean
	if (m_video_st)
	{
		avcodec_close(m_video_st->codec);
		//av_free(pFrame);
		//av_free(picture_buf);
	}
#ifdef FFMPEG_ENCODE_OUTPUT_FILE
	avio_close(m_AVOutputCtx->pb);
#endif
	avformat_free_context(m_AVOutputCtx);

	return 0;
}

/*

*/
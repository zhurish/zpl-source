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
#include <liveMedia/liveMedia.hh>
#include "ffmpegDevice.hpp"
#include "ffmpegEncoder.hpp"
#include "ffmpegSource.hpp"

ffmpegSource::ffmpegSource()
{
	init_flag = good_frame = false;
	ffmpeg_Device = new ffmpegDevice();
	ffmpeg_Encode = new ffmpegEncoder();
}

int ffmpegSource::ffmpegSourceInit()
{
	if (init_flag == true)
		return 0;
	int ret = ffmpeg_Device->ffmpegDeviceInit(640, 480, 30);
	if (ret != 0)
		return ret;
	ret = ffmpeg_Device->ffmpegDeviceOpen("video4linux2", "/dev/video0");
	if (ret != 0)
		return ret;
	ret = ffmpeg_Device->ffmpegDeviceDecoderOpen();
	if (ret != 0)
		return ret;
	ret = ffmpeg_Encode->ffmpegEncoderInit(640, 480, 30);
	if (ret != 0)
		return ret;
	ret = ffmpeg_Encode->ffmpegEncoderOpen(0);
	if (ret != 0)
		return ret;

	ret = ffmpeg_Device->ffmpegDeviceReady();
	if (ret != 0)
		return ret;
	init_flag = true;
	return 0;
}

// overide FramedSource
int ffmpegSource::doGetFrameData(void *p, int size)
{
	if (good_frame == true && ffmpeg_Encode != nullptr)
	{
		memmove(p, enc_pkt.data, size);
		return 0;
	}
	return -1;
}

void ffmpegSource::doGetFrameDataFree()
{
	av_packet_unref(&enc_pkt);
	av_free_packet(&enc_pkt);
}

int ffmpegSource::doGetFrameDataSize()
{
	if (good_frame == true && ffmpeg_Encode != nullptr)
	{
		return enc_pkt.size;
	}
	return 0;
}

int ffmpegSource::doGetFrame()
{
	if (ffmpeg_Device != nullptr && ffmpeg_Encode != nullptr)
	{
		int ret = ffmpeg_Device->ffmpegDeviceGetFrame();
		if (ret < 0)
			return 0;
		ret = ffmpeg_Device->ffmpegDeviceDecoderFrame();
		if (ret != 0)
			return 0;

		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		if (ffmpeg_Encode->ffmpegEncoderFrame(ffmpeg_Device->pFrameYUV, &enc_pkt) == 0)
		{
			good_frame = true;
		}
		else
			good_frame = false;
	}
	return 0;
}

int ffmpegSource::ffmpegSourceDestroy()
{
	if (ffmpeg_Encode)
	{
		ffmpeg_Encode->ffmpegEncoderFrameFinish();
		ffmpeg_Encode->ffmpegEncoderDestroy();
		delete ffmpeg_Encode;
	}
	if (ffmpeg_Device)
	{
		ffmpeg_Device->ffmpegDeviceDecoderFrameFinish();
		ffmpeg_Device->ffmpegDeviceDestroy();
		delete ffmpeg_Device;
	}
	return 0;
}

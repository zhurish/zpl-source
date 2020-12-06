/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2020 Live Networks, Inc.  All rights reserved.
// Framed File Sources
// Implementation
#include <sstream>
#include <iostream>

extern "C"
{
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "device/v4l2_driver.h"
};

#include "videoEncoder.hpp"
#include "v4l2Device.hpp"
////////// v4l2Device //////////

v4l2Device::v4l2Device(int width, int height, int fps, int fmt)
{
  this->m_width = width;
  this->m_height = height;
  this->m_fmt = V4L2_PIX_FMT_YUYV;
  this->m_fps = fps;
  m_videoEncoder = nullptr;
  obuf.start = nullptr;
  obuf.length = 0;
}

v4l2Device::~v4l2Device()
{
  if (obuf.start != nullptr)
  {
    delete[] obuf.start;
    obuf.start = nullptr;
  }
  if (video_drv.fd)
  {
    v4l2_free_bufs(&video_drv);
    v4l2_close(&video_drv);
  }
}

int v4l2Device::v4l2DeviceTryOpen(char *device)
{
  int ret = 0;
  struct v4l2_format fmt;
  if (v4l2_open(device, 1, &video_drv) < 0)
  {
    std::cout << "Faile open " << device << std::endl;
    return -1;
  }
  if (v4l2_get_capabilities(&video_drv) < 0)
  {
    perror("v4l2_get_capabilities");
    v4l2_close(&video_drv);
    return -1;
  }
  /*
  V4L2_PIX_FMT_UYVY V4L2_PIX_FMT_H264 V4L2_PIX_FMT_H264_NO_SC V4L2_PIX_FMT_H264_MVC V4L2_PIX_FMT_MPEG4
  V4L2_PIX_FMT_YUV422M V4L2_PIX_FMT_YVU422M V4L2_PIX_FMT_YUV422P
  */
  if (v4l2_enum_fmt(&video_drv, V4L2_BUF_TYPE_VIDEO_CAPTURE) < 0)
  {
    perror("enum_fmt_cap");
    v4l2_close(&video_drv);
    return -1;
  }

  if (v4l2_get_parm(&video_drv) < 0)
  {
    perror("get_parm");
    v4l2_close(&video_drv);
    return -1;
  }
  v4l2_close(&video_drv);
  return ret;
}

int v4l2Device::v4l2DeviceOpen(char *device)
{
  int ret = 0;
  struct v4l2_format fmt;
  if (v4l2_open(device, 1, &video_drv) < 0)
  {
    std::cout << "Faile open " << device << std::endl;
    return -1;
  }
  /*
  if (v4l2_gettryset_fmt_cap (&video_drv, V4L2_SET, &fmt, m_width, m_height, m_fmt, V4L2_FIELD_ANY))
  {
		perror("set_input");
    v4l2_close(&video_drv);
    return -1;
	}
  */
  if (v4l2_set_fmt_cap(&video_drv, &fmt, m_width, m_height, m_fmt, V4L2_FIELD_ANY))
  {
    perror("set_input");
    v4l2_close(&video_drv);
    return -1;
  }
  ret = v4l2_mmap_bufs(&video_drv, 2);
  if (ret != 0)
  {
    v4l2_close(&video_drv);
    return -1;
  }
  return ret;
}

int v4l2Device::v4l2DeviceStart(videoEncoder *encoder)
{
  this->m_videoEncoder = encoder;
  if (video_drv.fd)
  {
    obuf.length = video_drv.bufs[0].length;
    obuf.start = new char[obuf.length + 1];
    return v4l2_start_streaming(&video_drv);
  }
  return -1;
}

int v4l2Device::v4l2DeviceStop()
{
  if (video_drv.fd)
  {
    return v4l2_stop_streaming(&video_drv);
  }
  return -1;
}

int v4l2Device::v4l2DeviceStartCapture(struct v4l2_t_buf *buf, int timeout)
{
  fd_set fds;
  struct timeval tv;
  int r = 0;
  if (video_drv.fd)
  {
    FD_ZERO(&fds);
    FD_SET(video_drv.fd, &fds);
    /* Timeout. */
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    while (1)
    {
      r = select(video_drv.fd + 1, &fds, NULL, NULL, &tv);
      if (-1 == r)
      {
        if (EINTR == errno)
          continue;
        perror("select");
        return -1;
      }
      if (0 == r)
      {
        fprintf(stderr, "select timeout\n");
        return -1;
      }
      r = v4l2_read_outbuf(&video_drv, buf);
      //r = v4l2_rcvbuf(&video_drv, recebe_buffer);
      if (r > 0)
        break;
    }
  }
  return r;
}

int v4l2Device::v4l2DeviceStartCapture(FramedQueue *m_queue)
{
  if (m_queue != nullptr)
  {
    if (!m_queue->FramedQueueIsStart())
    {
      return -1;
    }
  }
  if (m_queue != nullptr)
  {
    m_queue->FramedQueueWait();
    int ret = v4l2DeviceStartCapture(&obuf, 500);
    if (ret > 0)
    {
      if (m_queue->FramedQueueDataIsFull())
        m_queue->FramedQueueDataFlush();
      if (m_videoEncoder != nullptr)
      {
        //h264Core->h264Encoder_setup(m_width, m_height, X264_CSP_I422);
        /*
          FILE *fp = fopen("./aaa.yuv", "ab");
          fwrite(obuf.start, ret, 1, fp);
          fclose(fp);
          */
        ret = m_videoEncoder->videoEncoderInput((char *)obuf.start, ret, false);
        if (ret > 0)
          m_queue->FramedQueueDataPut((char *)m_videoEncoder->videoEncoderOutput(), m_videoEncoder->videoEncoderOutputSize(true));
      }
      else
        m_queue->FramedQueueDataPut((char *)obuf.start, ret);
    }
  }
  return 0;
}
/*
int v4l2Device::recebe_buffer (struct v4l2_buffer *v4l2_buf, struct v4l2_t_buf *buf)
{
  //memcpy(aa, buf->start, v4l2_buf->bytesused);
	return v4l2_buf->bytesused;
}
*/

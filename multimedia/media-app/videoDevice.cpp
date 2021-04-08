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

#include "h264Encoder.hpp"
#include "videoDevice.hpp"
////////// videoDevice //////////


videoDevice::videoDevice(int width, int height, int fps, int fmt) {
	this->m_width = width;	
	this->m_height = height;
  this->m_fmt = V4L2_PIX_FMT_YUYV;
 	this->m_fps = fps;
  h264Core = nullptr;
  h264Core = new h264Encoder();
  obuf.start = nullptr;
  obuf.length = 0;
}

videoDevice::~videoDevice(){
  if(obuf.start != nullptr)
  {
    delete [] obuf.start;
    obuf.start = nullptr;
  }
  if(video_drv.fd)
  {
    v4l2_free_bufs(&video_drv);
    v4l2_close(&video_drv);
  }
  if(h264Core != nullptr)
  {
    h264Core->h264EncoderDestroy();
    delete h264Core;
    h264Core = nullptr;
  }
}

int videoDevice::videoDeviceTryOpen(char *device)
{
  int ret = 0;
  struct v4l2_format fmt;
	if (v4l2_open (device, 1, &video_drv)<0) {
		std::cout << "Faile open " << device << std::endl;
		return -1;
	}
  if (v4l2_get_capabilities (&video_drv) < 0)
  {
		perror("v4l2_get_capabilities");
    v4l2_close(&video_drv);
    return -1;
	}
  /*
  V4L2_PIX_FMT_UYVY V4L2_PIX_FMT_H264 V4L2_PIX_FMT_H264_NO_SC V4L2_PIX_FMT_H264_MVC V4L2_PIX_FMT_MPEG4
  V4L2_PIX_FMT_YUV422M V4L2_PIX_FMT_YVU422M V4L2_PIX_FMT_YUV422P
  */
	if (v4l2_enum_fmt (&video_drv,V4L2_BUF_TYPE_VIDEO_CAPTURE)<0) {
		perror("enum_fmt_cap");
    v4l2_close(&video_drv);
    return -1;
	}

	if (v4l2_get_parm (&video_drv)<0) {
		perror("get_parm");
    v4l2_close(&video_drv);
    return -1;
	}
  v4l2_close(&video_drv);
  return ret;
}

int videoDevice::videoDeviceOpen(char *device)
{
  int ret = 0;
  struct v4l2_format fmt;
	if (v4l2_open (device, 1, &video_drv)<0) {
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
  if (v4l2_set_fmt_cap (&video_drv, &fmt, m_width, m_height, m_fmt, V4L2_FIELD_ANY))
  {
		perror("set_input");
    v4l2_close(&video_drv);
    return -1;
	}
	ret = v4l2_mmap_bufs(&video_drv, 2);
  if(ret != 0)
  {
    v4l2_close(&video_drv);
    return -1;
  }
  if(h264Core != nullptr)
  { 
#ifdef PL_LIBX264_MODULE
    h264Core->h264EncoderSetup(m_width, m_height, X264_CSP_I422, this->m_fps);
#endif
#ifdef PL_OPENH264_MODULE
    h264Core->h264EncoderSetup(m_width, m_height, videoFormatI420, this->m_fps);
#endif
  }
  return ret;
}

int videoDevice::videoDeviceStart()
{
  if(video_drv.fd)
  {
    obuf.length = video_drv.bufs[0].length;
    obuf.start = new char [obuf.length + 1];
    return v4l2_start_streaming(&video_drv);
  }  
  return -1;
}

int videoDevice::videoDeviceStop()
{
  if(video_drv.fd)
  {
    return v4l2_stop_streaming(&video_drv);
  } 
  return -1;
}

int videoDevice::videoDeviceStartCapture(struct v4l2_t_buf *buf, int timeout)
{
		fd_set fds;
		struct timeval tv;
		int r = 0;
    if(video_drv.fd)
    {
      FD_ZERO (&fds);
      FD_SET (video_drv.fd, &fds);
      /* Timeout. */
      tv.tv_sec = timeout/1000;
      tv.tv_usec = (timeout%1000)*1000;
      while(1)
      {
        r = select (video_drv.fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) 
        {
          if (EINTR == errno)
            continue;
          perror ("select");
          return -1;
        }
        if (0 == r) 
        {
          fprintf (stderr, "select timeout\n");
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


int videoDevice::videoDeviceStartCapture(FramedQueue *m_queue)
{
    if(m_queue != nullptr)
    {
      if(!m_queue->FramedQueueIsStart ())
      {
        return -1;
      }
    }
    if(m_queue != nullptr)
    {
      m_queue->FramedQueueWait();
      int ret = videoDeviceStartCapture(&obuf, 500);
      if(ret > 0)
      {
        if(m_queue->FramedQueueDataIsFull())
          m_queue->FramedQueueDataFlush();
        if(h264Core != nullptr)
        { 
          //h264Core->h264Encoder_setup(m_width, m_height, X264_CSP_I422);
          /*
          FILE *fp = fopen("./aaa.yuv", "ab");
          fwrite(obuf.start, ret, 1, fp);
          fclose(fp);
          */
          ret = h264Core->h264EncoderInput((char *)obuf.start, ret);
          if(ret > 0)
            m_queue->FramedQueueDataPut((char *)h264Core->h264EncoderOutput(), h264Core->h264EncoderOutputSize(true));
          
        }
        else
          m_queue->FramedQueueDataPut((char *)obuf.start, ret);
      }
    }
  return 0;
}
/*
int videoDevice::recebe_buffer (struct v4l2_buffer *v4l2_buf, struct v4l2_t_buf *buf)
{
  //memcpy(aa, buf->start, v4l2_buf->bytesused);
	return v4l2_buf->bytesused;
}
*/

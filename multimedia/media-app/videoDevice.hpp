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
// Framed fd Sources
// C++ header

#ifndef __VIDEO_DEVICE_HPP__
#define __VIDEO_DEVICE_HPP__


extern "C"
{
#include "device/v4l2_driver.h"
};

#include "h264Encoder.hpp"
#include "FramedQueue.hpp"

class videoDevice {
public:
  videoDevice(int width, int height, int fps, int fmt);
  virtual ~videoDevice();
  int videoDeviceTryOpen(char *device);
  int videoDeviceOpen(char *device);
  int videoDeviceStart();
  int videoDeviceStop();
  int videoDeviceStartCapture(struct v4l2_t_buf *buf, int timeout);
  int videoDeviceStartCapture(FramedQueue *m_queue);
  //static int recebe_buffer (struct v4l2_buffer *v4l2_buf, struct v4l2_t_buf *buf);
private:
  //int v4l2_read_capture(struct v4l2_driver *drv);
private:
  h264Encoder *h264Core = nullptr;
  struct v4l2_driver video_drv;
  struct v4l2_t_buf obuf;
	int m_width;	
	int m_height;
  int m_fmt;
 	int m_fps;
	int m_fps_sec;
	int m_size;

};

#endif

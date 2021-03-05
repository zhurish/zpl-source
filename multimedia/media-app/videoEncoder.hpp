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

#ifndef __VIDEO_ENCODER_HPP__
#define __VIDEO_ENCODER_HPP__
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif
class videoEncoder
{
public:
    videoEncoder()
    {
        this->m_width = 0;
        this->m_height = 0;
        this->m_fmt = 0;
        this->m_fps = 0;
        this->m_out_size = 0;
        this->m_out_buf = nullptr;
    }
    virtual ~videoEncoder() = default;
    virtual int videoEncoderSetup(const int width, const int height, const int fmt, const int fps) = 0;
    /*{
	      this->m_width;	
          this->m_height;
          this->m_fmt;
          this->m_fps;
          return 0;
    }
    */
    virtual int videoEncoderInput(const ospl_uint8 *frame, const int len, bool keyframe) = 0;
    virtual int videoEncoderOutput(ospl_uint8 *frame, const int len) = 0;
    virtual ospl_uint8 *videoEncoderOutput() = 0;
    virtual int videoEncoderOutputSize(const bool clear = true) = 0;
    virtual int videoEncoderDestroy() = 0;
public:
    int m_width = 0;
    int m_height = 0;
    int m_fmt = 0;
    int m_fps = 0;
    int m_out_size = 0;
    char *m_out_buf = nullptr;
};


#endif

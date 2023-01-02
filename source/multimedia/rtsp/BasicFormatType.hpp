
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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H265 Elementary Stream video file.
// C++ header

#ifndef _BASIC_FORMAT_TYPE_HH
#define _BASIC_FORMAT_TYPE_HH
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

class BasicFormatType {
public:
  static const int BASIC_FORMAT_TYPE_AC3 = 1;
  static const int BASIC_FORMAT_TYPE_ADTS = 2;
  static const int BASIC_FORMAT_TYPE_AMR = 3;
  static const int BASIC_FORMAT_TYPE_AVI = 4;
  static const int BASIC_FORMAT_TYPE_GSM = 5;

  static const int BASIC_FORMAT_TYPE_DW = 6;

  static const int BASIC_FORMAT_TYPE_H261 = 7;
  static const int BASIC_FORMAT_TYPE_H263 = 8;
  static const int BASIC_FORMAT_TYPE_H264 = 9;
  static const int BASIC_FORMAT_TYPE_H265 = 10;

  static const int BASIC_FORMAT_TYPE_HLS = 11;
  static const int BASIC_FORMAT_TYPE_JPEG2000 = 12;
  static const int BASIC_FORMAT_TYPE_MKV = 13;
  static const int BASIC_FORMAT_TYPE_MP3 = 14;
  static const int BASIC_FORMAT_TYPE_MPEG1 = 15;
  static const int BASIC_FORMAT_TYPE_MPEG2 = 16;

  static const int BASIC_FORMAT_TYPE_MPEG4 = 17;
  static const int BASIC_FORMAT_TYPE_OGG = 18;
  static const int BASIC_FORMAT_TYPE_VP8 = 19;

  static const int BASIC_FORMAT_TYPE_VP9 = 20;
  static const int BASIC_FORMAT_TYPE_WAV = 21;
  static const int BASIC_FORMAT_TYPE_MJPG = 22;
  static const int BASIC_FORMAT_TYPE_RAW = 23;
  
};

#endif

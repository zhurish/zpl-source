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
// Copyright (c) 1996-2024 Live Networks, Inc.  All rights reserved.
// Framed Sources
// C++ header

#ifndef _FRAMED_LIVE_SOURCE_H_
#define _FRAMED_LIVE_SOURCE_H_

#ifndef _NET_COMMON_H
#include "NetCommon.h"
#endif
#ifndef _MEDIA_SOURCE_HH
#include "MediaSource.hh"
#endif
#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

class FramedLiveSource: public FramedSource {
public:
  static FramedLiveSource *createNew(UsageEnvironment &env, void *);
  // redefined virtual functions
  virtual unsigned maxFrameSize() const;

protected:
  FramedLiveSource(UsageEnvironment &env, void *);
  virtual ~FramedLiveSource();

private:
  virtual void doGetNextFrame();
  void  *frame_queue;
};

#endif

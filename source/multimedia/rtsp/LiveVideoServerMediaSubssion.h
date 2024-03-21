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

#ifndef _LIVE_VIDEO_SERVER_MEDIA_SUBSSION_H_
#define _LIVE_VIDEO_SERVER_MEDIA_SUBSSION_H_

#include "OnDemandServerMediaSubsession.hh"

class LiveVideoServerMediaSubssion : public OnDemandServerMediaSubsession
{

public:
  static LiveVideoServerMediaSubssion *createNew(UsageEnvironment &env, Boolean reuseFirstSource, int codec, void *);


protected:
  LiveVideoServerMediaSubssion(UsageEnvironment &env, Boolean reuseFirstSource, int codec, void *);
  ~LiveVideoServerMediaSubssion();

protected: // redefined virtual functions
  FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
  RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                            unsigned char rtpPayloadTypeIfDynamic,
                            FramedSource *inputSource);
private:

  void *frame_queue = NULL;
  int m_codec;
};

#endif

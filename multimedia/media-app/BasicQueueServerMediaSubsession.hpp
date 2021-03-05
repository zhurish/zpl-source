
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

#ifndef _BASIC_QUEUE_SERVER_MEDIA_SUBSESSION_HH
#define _BASIC_QUEUE_SERVER_MEDIA_SUBSESSION_HH

#include "FramedQueue.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif
class BasicQueueServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
  static BasicQueueServerMediaSubsession*
    createNew(UsageEnvironment& env, FramedQueue *queue, int format);
  static BasicQueueServerMediaSubsession*
    createNew(UsageEnvironment& env, int fd, int format);
  static BasicQueueServerMediaSubsession*
    createNew(UsageEnvironment& env, FramedSource *source, int format);

protected:
  BasicQueueServerMediaSubsession(UsageEnvironment& env, FramedSource *source, FramedQueue *queue, int fd, int format);
  // called only by createNew();
  virtual ~BasicQueueServerMediaSubsession();

protected: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
				    ospl_uint8 rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);
private:

  FramedSource* BasicQueueCreateStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate, FramedSource* videoES, int format);
  RTPSink* BasicQueueCreateNewRTPSink(Groupsock* rtpGroupsock,
				    ospl_uint8 rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource, int format);

  FramedSource* m_streamSource = nullptr;
  FramedQueue *m_Queue = nullptr;
  int m_format = 0;
  int m_inputfd = 0;
};

#endif

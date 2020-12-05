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

#ifndef _FRAMED_QUEUE_SOURCE_HH
#define _FRAMED_QUEUE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif
#include "FramedQueue.hpp"

class FramedQueueSource: public FramedSource {
public:
  static FramedQueueSource* createNew(UsageEnvironment& env, FramedQueue *queue);

  virtual ~FramedQueueSource();

private:
  // called only by createNew()
  FramedQueueSource(UsageEnvironment& env, FramedQueue *queue);
      
  static void FramedQueueSourceReadableHandler(FramedQueueSource* source, int mask);
  void doFramedQueueSourceReadHandler(int fd);

  Boolean handleReadFramedQueue(unsigned char* buffer, unsigned bufferMaxSize,
			      unsigned& bytesRead);

            
private: // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

private:
  EventTriggerId eventTriggerId;
  FramedQueue *m_Queue = nullptr;

  Boolean fHaveStartedReading;
};

#endif

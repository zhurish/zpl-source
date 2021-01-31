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
#include "FramedQueueSource.hpp"
#include "FramedQueue.hpp"
////////// FramedQueueSource //////////

FramedQueueSource *FramedQueueSource::createNew(UsageEnvironment &env,
                                                FramedQueue *queue)
{
  return new FramedQueueSource(env, queue);
}

FramedQueueSource::FramedQueueSource(UsageEnvironment &env, FramedQueue *queue)
    : FramedSource(env), fHaveStartedReading(False), m_Queue(queue)
{
  if (eventTriggerId == 0)
  {
    if (m_Queue)
      eventTriggerId = envir().taskScheduler().createEventTrigger(m_Queue->FramedQueueEventTriggerHandler);
  }
  if (eventTriggerId)
  {
    if (m_Queue)
      envir().taskScheduler().triggerEvent(eventTriggerId, m_Queue);
  }
}

FramedQueueSource::~FramedQueueSource()
{
  if (eventTriggerId)
  {
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
  }
  eventTriggerId = 0;
}

void FramedQueueSource::doGetNextFrame()
{
  if (!fHaveStartedReading)
  {
    if (m_Queue && m_Queue->FramedQueueDataFd())
    {
      envir().taskScheduler().turnOnBackgroundReadHandling(m_Queue->FramedQueueDataFd(),
                                                           (TaskScheduler::BackgroundHandlerProc *)&FramedQueueSourceReadableHandler, this);
    }
    else
    {
      doFramedQueueSourceReadHandler(0);
    }
    fHaveStartedReading = True;
  }
}

void FramedQueueSource::doStopGettingFrames()
{
  if (m_Queue && m_Queue->FramedQueueDataFd())
    envir().taskScheduler().turnOffBackgroundReadHandling(m_Queue->FramedQueueDataFd());
  fHaveStartedReading = False;
}

void FramedQueueSource::FramedQueueSourceReadableHandler(FramedQueueSource *source, int mask)
{
  if (source && source->m_Queue)
    source->doFramedQueueSourceReadHandler(source->m_Queue->FramedQueueDataFd());
  //std::cout << "=========================FramedQueueSource::FramedQueueSourceReadableHandler "<< std::endl;
}

void FramedQueueSource::doFramedQueueSourceReadHandler(int fd)
{

  if (!isCurrentlyAwaitingData())
    return; // we're not ready for the data yet

  // Read the packet into our desired destination:
  if (m_Queue)
  {
    m_Queue->FramedQueueDataSizeGet();
    if (!handleReadFramedQueue(fTo, fMaxSize, fFrameSize))
      return;
  }
  else
    return;
  fNumTruncatedBytes += fMaxSize - fFrameSize;
  // Tell our client that we have new data:
  gettimeofday(&fPresentationTime, NULL);
  if (m_Queue && !m_Queue->FramedQueueDataFd())
  {
    // Switch to another task, and inform the reader that he has data:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *)FramedSource::afterGetting, this);
  }
  else
    afterGetting(this); // we're preceded by a net read; no infinite recursion
}

Boolean FramedQueueSource::handleReadFramedQueue(unsigned char *buffer, unsigned bufferMaxSize,
                                                 unsigned &bytesRead)
{

  if (m_Queue)
  {
    m_Queue->FramedQueueDataReady();
    if (m_Queue->FramedQueueDataIsEmpty())
    {
      m_Queue->FramedQueuePost();
      return False;
    }
    bytesRead = m_Queue->FramedQueueDataGet((unsigned char  *)buffer, bufferMaxSize);
    if (bytesRead > 0)
    {
      m_Queue->FramedQueuePost();
      return True;
    }
    m_Queue->FramedQueuePost();
  }
  return False;
}

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
// A simple UDP source, where every UDP payload is a complete frame
// Implementation

#include "BasicFdSource.hpp"
#include <GroupsockHelper.hh>

BasicFdSource *BasicFdSource::createNew(UsageEnvironment &env,
                                        int fd)
{
  return new BasicFdSource(env, fd);
}

BasicFdSource::BasicFdSource(UsageEnvironment &env, int fd)
    : FramedSource(env), input_fd(fd), fHaveStartedReading(False)
{
  // Try to use a large receive buffer (in the OS):
  //increaseReceiveBufferTo(env, input_fd, 50*1024);

  // Make the socket non-blocking, even though it will be read from only asynchronously, when packets arrive.
  // The reason for this is that, in some OSs, reads on a blocking socket can (allegedly) sometimes block,
  // even if the socket was previously reported (e.g., by "select()") as having data available.
  // (This can supposedly happen if the UDP checksum fails, for example.)
  makeSocketNonBlocking(input_fd);
}

BasicFdSource::~BasicFdSource()
{
  envir().taskScheduler().turnOffBackgroundReadHandling(input_fd);
}

void BasicFdSource::doGetNextFrame()
{
  if (!fHaveStartedReading)
  {
    // Await incoming packets:
    envir().taskScheduler().turnOnBackgroundReadHandling(input_fd,
                                                         (TaskScheduler::BackgroundHandlerProc *)&BasicFdReadableHandler, this);
    fHaveStartedReading = True;
  }
}

void BasicFdSource::doStopGettingFrames()
{
  envir().taskScheduler().turnOffBackgroundReadHandling(input_fd);
  fHaveStartedReading = False;
}

void BasicFdSource::BasicFdReadableHandler(BasicFdSource *source, int /*mask*/)
{
  source->BasicFdReadFrom();
}

void BasicFdSource::BasicFdReadFrom()
{
  if (!isCurrentlyAwaitingData())
    return; // we're not ready for the data yet

  // Read the packet into our desired destination:
  //struct sockaddr_in fromAddress;
  //if (!fInputGS->handleRead(fTo, fMaxSize, fFrameSize, fromAddress)) return;
  if (!handleReadFd(fTo, fMaxSize, fFrameSize))
    return;
  // Tell our client that we have new data:
  gettimeofday(&fPresentationTime, NULL);
  afterGetting(this); // we're preceded by a net read; no infinite recursion
}

Boolean BasicFdSource::doReadFromFd()
{

  int numBytes = read(input_fd, fTo, fMaxSize);
  if (numBytes < 0)
  {
    return False;
  }
  fFrameSize = numBytes;
  return True;
  //FramedSource::afterGetting(this);
}

Boolean BasicFdSource::handleReadFd(unsigned char *buffer, unsigned bufferMaxSize,
                                    unsigned &bytesRead)
{
  // Read data from the socket, and relay it across any attached tunnels
  //##### later make this code more general - independent of tunnels
  bytesRead = 0;
  int maxBytesToRead = bufferMaxSize; // - TunnelEncapsulationTrailerMaxSize;
  /*int numBytes = readSocket(env(), socketNum(),
			    buffer, maxBytesToRead, fromAddressAndPort);*/
  int numBytes = read(input_fd, buffer, maxBytesToRead);
  if (numBytes < 0)
  {
    return False;
  }

  // We'll handle this data.
  // Also write it (with the encapsulation trailer) to each member,
  // unless the packet was originally sent by us to begin with.
  bytesRead = numBytes;
  return True;
}
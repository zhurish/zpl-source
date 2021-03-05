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
// C++ header

#ifndef _BASIC_FD_SOURCE_HH
#define _BASIC_FD_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif
#ifndef _GROUPSOCK_HH
#include "Groupsock.hh"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif

class BasicFdSource: public FramedSource {
public:
  static BasicFdSource* createNew(UsageEnvironment& env, int fd);

  virtual ~BasicFdSource();

private:
  BasicFdSource(UsageEnvironment& env, int fd);
      // called only by createNew()
  static void BasicFdReadableHandler(BasicFdSource* source, int mask);
  void BasicFdReadFrom();
  Boolean handleReadFd(ospl_uint8* buffer, unsigned bufferMaxSize,
			      unsigned& bytesRead);
  Boolean doReadFromFd();
            
private: // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();

private:
  int input_fd;
  Boolean fHaveStartedReading;
};

#endif

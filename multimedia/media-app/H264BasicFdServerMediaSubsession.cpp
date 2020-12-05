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
// on demand, from a H264 video file.
// Implementation

#include <liveMedia/liveMedia.hh>
#include "H264VideoRTPSink.hh"
#include "BasicFdSource.hpp"
#include "H264VideoStreamFramer.hh"
#include "H264BasicFdServerMediaSubsession.hpp"

H264BasicFdServerMediaSubsession *
H264BasicFdServerMediaSubsession::createNew(UsageEnvironment &env, int fd)
{
  return new H264BasicFdServerMediaSubsession(env, fd);
}

H264BasicFdServerMediaSubsession ::H264BasicFdServerMediaSubsession(UsageEnvironment &env, int fd)
    : OnDemandServerMediaSubsession(env, True /*reuseFirstSource*/),
      input_fd(fd)
{
}

H264BasicFdServerMediaSubsession::
    ~H264BasicFdServerMediaSubsession()
{
}

FramedSource *H264BasicFdServerMediaSubsession ::createNewStreamSource(unsigned /* clientSessionId*/, unsigned &estBitrate)
{
  estBitrate = 5000; // kbps, estimate

  FramedSource *transportStreamSource;
  transportStreamSource = BasicFdSource::createNew(envir(), input_fd);

  return H264VideoStreamFramer::createNew(envir(), transportStreamSource);
}

RTPSink *H264BasicFdServerMediaSubsession ::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource)
{
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  //return SimpleRTPSink::createNew(envir(), rtpGroupsock,
  //			  33, 90000, "video", "H264",
  //			  1, True, False /*no 'M' bit*/);
}
/*
SimpleRTPSink::SimpleRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
			     unsigned char rtpPayloadFormat,
			     unsigned rtpTimestampFrequency,
			     char const* sdpMediaTypeString,
			     char const* rtpPayloadFormatName,
			     unsigned numChannels,
			     Boolean allowMultipleFramesPerPacket,
			     Boolean doNormalMBitRule)*/
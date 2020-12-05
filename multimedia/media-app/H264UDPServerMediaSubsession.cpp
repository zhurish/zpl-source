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
#include "BasicUDPSource.hh"
#include "SimpleRTPSource.hh"
#include "SimpleRTPSink.hh"
#include "GroupsockHelper.hh"

#include "H264VideoStreamFramer.hh"
#include "H264UDPServerMediaSubsession.hpp"

H264UDPServerMediaSubsession *
H264UDPServerMediaSubsession::createNew(UsageEnvironment &env,
                                        char const *inputAddressStr, Port const &inputPort, Boolean inputStreamIsRawUDP)
{
  return new H264UDPServerMediaSubsession(env, inputAddressStr, inputPort, inputStreamIsRawUDP);
}

H264UDPServerMediaSubsession ::H264UDPServerMediaSubsession(UsageEnvironment &env,
                                                            char const *inputAddressStr, Port const &inputPort, Boolean inputStreamIsRawUDP)
    : OnDemandServerMediaSubsession(env, True /*reuseFirstSource*/),
      fInputPort(inputPort), fInputGroupsock(NULL), fInputStreamIsRawUDP(inputStreamIsRawUDP)
{
  fInputAddressStr = strDup(inputAddressStr);
}

H264UDPServerMediaSubsession::
    ~H264UDPServerMediaSubsession()
{
  delete fInputGroupsock;
  delete[](char *) fInputAddressStr;
}

FramedSource *H264UDPServerMediaSubsession ::createNewStreamSource(unsigned /* clientSessionId*/, unsigned &estBitrate)
{
  estBitrate = 5000; // kbps, estimate

  if (fInputGroupsock == NULL)
  {
    // Create a 'groupsock' object for receiving the input stream:
    struct in_addr inputAddress;
    inputAddress.s_addr = fInputAddressStr == NULL ? 0 : our_inet_addr(fInputAddressStr);
    fInputGroupsock = new Groupsock(envir(), inputAddress, fInputPort, 255);
  }

  FramedSource *transportStreamSource;
  if (fInputStreamIsRawUDP)
  {
    transportStreamSource = BasicUDPSource::createNew(envir(), fInputGroupsock);
  }
  else
  {
    transportStreamSource = SimpleRTPSource::createNew(envir(), fInputGroupsock, 33, 90000, "video/H264", 0, False /*no 'M' bit*/);
  }
  return H264VideoStreamFramer::createNew(envir(), transportStreamSource);
}

RTPSink *H264UDPServerMediaSubsession ::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource)
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
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
#include <sstream>
#include <iostream>
#include <liveMedia/liveMedia.hh>
#include "H264VideoRTPSink.hh"
#include "BasicFdSource.hpp"
#include "FramedQueueSource.hpp"
#include "H264VideoStreamFramer.hh"
#include "BasicQueueServerMediaSubsession.hpp"
#include "BasicFormatType.hpp"

BasicQueueServerMediaSubsession *
BasicQueueServerMediaSubsession::createNew(UsageEnvironment &env, FramedQueue *queue, int format)
{
	return new BasicQueueServerMediaSubsession(env, nullptr, queue, 0, format);
}

BasicQueueServerMediaSubsession *
BasicQueueServerMediaSubsession::createNew(UsageEnvironment &env, int fd, int format)
{
	return new BasicQueueServerMediaSubsession(env, nullptr, nullptr, fd, format);
}

BasicQueueServerMediaSubsession *
BasicQueueServerMediaSubsession::createNew(UsageEnvironment &env, FramedSource *source, int format)
{
	return new BasicQueueServerMediaSubsession(env, source, nullptr, 0, format);
}
BasicQueueServerMediaSubsession::BasicQueueServerMediaSubsession(UsageEnvironment &env,
																 FramedSource *source, FramedQueue *queue, int fd, int format)
	: OnDemandServerMediaSubsession(env, True /*reuseFirstSource*/),
	  m_streamSource(source), m_Queue(queue), m_format(format), m_inputfd(fd)
{
}

BasicQueueServerMediaSubsession::~BasicQueueServerMediaSubsession()
{
}

FramedSource *BasicQueueServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate)
{

	//estBitrate = 5000; // kbps, estimate
	FramedSource *transportStreamSource = nullptr;
	if (m_Queue != nullptr)
		transportStreamSource = FramedQueueSource::createNew(envir(), m_Queue);
	else if (m_inputfd > 0)
		transportStreamSource = BasicFdSource::createNew(envir(), m_inputfd);
	else if (m_streamSource != nullptr)
		transportStreamSource = m_streamSource;
	if (transportStreamSource == nullptr)
		return nullptr;
	return BasicQueueCreateStreamSource(clientSessionId,
										estBitrate, transportStreamSource, m_format);
	//return H264VideoStreamFramer::createNew(envir(), transportStreamSource);
}

RTPSink *BasicQueueServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock, ospl_uint8 rtpPayloadTypeIfDynamic, FramedSource *inputSource)
{

	return BasicQueueCreateNewRTPSink(rtpGroupsock,
									  rtpPayloadTypeIfDynamic, inputSource, m_format);
	//return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	//return SimpleRTPSink::createNew(envir(), rtpGroupsock,
	//			  33, 90000, "video", "H264",
	//			  1, True, False /*no 'M' bit*/);
}

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------

FramedSource *BasicQueueServerMediaSubsession::BasicQueueCreateStreamSource(unsigned clientSessionId,
																			unsigned &estBitrate, FramedSource *videoES, int format)
{
	FramedSource *source = NULL;
	if (format == BasicFormatType::BASIC_FORMAT_TYPE_MPEG2)
	{
		estBitrate = 5000; // kbps, estimate
		source = MPEG2TransportStreamFramer::createNew(envir(), videoES);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_MPEG4)
	{
		estBitrate = 500; // kbps, estimate
		source = MPEG4VideoStreamFramer::createNew(envir(), videoES);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_AC3)
	{
		estBitrate = 48; // kbps, estimate
		source = AC3AudioStreamFramer::createNew(envir(), videoES);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H263)
	{
		estBitrate = 500; // kbps, estimate
		source = H263plusVideoStreamFramer::createNew(envir(), videoES);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H264)
	{
		estBitrate = 500; // kbps, estimate
		std::cout << "H264VideoStreamFramer::createNew" << std::endl;
		source = H264VideoStreamFramer::createNew(envir(), videoES);
	}
#if LIVEMEDIA_LIBRARY_VERSION_INT > 1414454400
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H265)
	{
		estBitrate = 500; // kbps, estimate
		source = H265VideoStreamFramer::createNew(envir(), videoES);
	}
#endif
	else
	{
		source = videoES;
	}
	return source;
}

RTPSink *BasicQueueServerMediaSubsession::BasicQueueCreateNewRTPSink(Groupsock *rtpGroupsock,
																	 ospl_uint8 rtpPayloadTypeIfDynamic, FramedSource *inputSource, int format)
{
	RTPSink *videoSink = NULL;
	if (format == BasicFormatType::BASIC_FORMAT_TYPE_MPEG2)
	{
		videoSink = SimpleRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, 90000, "video", "MP2T", 1, True, False);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H264)
	{
		std::cout << "H264VideoRTPSink::createNew" << std::endl;
		videoSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_VP8)
	{
		videoSink = VP8VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
#if LIVEMEDIA_LIBRARY_VERSION_INT > 1414454400
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_VP9)
	{
		videoSink = VP9VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H265)
	{
		videoSink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
#endif
	/*else if (format.find("audio/L16") == 0)
	{
		std::istringstream is(format);
		std::string dummy;
		getline(is, dummy, '/');	
		getline(is, dummy, '/');	
		std::string sampleRate("44100");
		getline(is, sampleRate, '/');	
		std::string channels("2");
		getline(is, channels);	
		videoSink = SimpleRTPSink::createNew(env, rtpGroupsock,rtpPayloadTypeIfDynamic, atoi(sampleRate.c_str()), "audio", "L16", atoi(channels.c_str()), True, False); 
	}*/
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_AC3)
	{
		AC3AudioStreamFramer *audioSource = (AC3AudioStreamFramer *)inputSource;
		videoSink = AC3AudioRTPSink::createNew(envir(), rtpGroupsock,
											   rtpPayloadTypeIfDynamic,
											   audioSource->samplingRate());
	}
	else if (format == BasicFormatType::BASIC_FORMAT_TYPE_H263)
	{
		videoSink = H263plusVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	}
	return videoSink;
}

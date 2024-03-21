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
// Framed Live Sources
// Implementation

 
#include "LiveVideoServerMediaSubssion.h"
#include "FramedLiveSource.h"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "H264VideoRTPSink.hh"
#include "H265VideoRTPSink.hh"
#include <liveMedia.hh>
#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"


LiveVideoServerMediaSubssion* LiveVideoServerMediaSubssion::createNew(UsageEnvironment& env, Boolean reuseFirstSource, int codec, void *queue)
{
	return new LiveVideoServerMediaSubssion(env, reuseFirstSource, codec, queue);
}
 
LiveVideoServerMediaSubssion::LiveVideoServerMediaSubssion(UsageEnvironment& env,Boolean reuseFirstSource, int codec, void *queue)
: OnDemandServerMediaSubsession(env,reuseFirstSource)
{
	m_codec = codec;
	frame_queue = queue;
}
 
LiveVideoServerMediaSubssion::~LiveVideoServerMediaSubssion()
{
	frame_queue = NULL;	
}


FramedSource* LiveVideoServerMediaSubssion::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	//创建视频源,参照H264VideoFileServerMediaSubsession
	estBitrate = 500; // kbps, estimate
	FramedLiveSource* liveSource = FramedLiveSource::createNew(envir(), (zpl_skbqueue_t*)frame_queue);
	if (liveSource == NULL)
	{
		return NULL;
	}
	// Create a framer for the Video Elementary Stream:
#if 0
	// Create a framer for the Video Elementary Stream:
	//return H264VideoStreamFramer::createNew(envir(), liveSource);
#else 
	//不需要parse, 直接就是完整的一帧
	if(m_codec == ZPL_VIDEO_CODEC_H264)
		return H264VideoStreamDiscreteFramer::createNew(envir(), liveSource);
	else if(m_codec == ZPL_VIDEO_CODEC_H265)
		return H265VideoStreamDiscreteFramer::createNew(envir(), liveSource);
	/*	
	else if(m_codec == ZPL_VIDEO_CODEC_AAC)
		return ADTSAudioFileServerMediaSubsession::createNew(envir(), liveSource, True);
	else if(m_codec == ZPL_AUDIO_CODEC_AMR)
		return AMRAudioFileServerMediaSubsession::createNew(envir(), liveSource, True);
	else if(m_codec == ZPL_AUDIO_CODEC_G711A)
		return uLawFromPCMAudioSource::createNew(envir(), liveSource, True);

	else if(m_codec == ZPL_AUDIO_CODEC_G711A)
		return H265VideoStreamDiscreteFramer::createNew(envir(), liveSource);
	else if(m_codec == ZPL_AUDIO_CODEC_G711U)
		return uLawFromPCMAudioSource::createNew(envir(), liveSource);
	else if(m_codec == ZPL_AUDIO_CODEC_G726)
		return H265VideoStreamDiscreteFramer::createNew(envir(), liveSource);
	else if(m_codec == ZPL_AUDIO_CODEC_LPCM)
		return H265VideoStreamDiscreteFramer::createNew(envir(), liveSource);*/
#endif  
	return NULL;
}
 
RTPSink* LiveVideoServerMediaSubssion
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
	if(m_codec == ZPL_VIDEO_CODEC_H264)			
  		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	else if(m_codec == ZPL_VIDEO_CODEC_H265)			
  		return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	return NULL;
}
 
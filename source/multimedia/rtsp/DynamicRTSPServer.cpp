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
// Copyright (c) 1996-2024, Live Networks, Inc.  All rights reserved
// A subclass of "RTSPServer" that creates "ServerMediaSession"s on demand,
// based on whether or not the specified stream name exists as a file
// Implementation

#include "DynamicRTSPServer.hh"
#include <liveMedia.hh>
#include <string.h>
#include "LiveVideoServerMediaSubssion.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

DynamicRTSPServer *
DynamicRTSPServer::createNew(UsageEnvironment &env, Port ourPort,
                             UserAuthenticationDatabase *authDatabase,
                             unsigned reclamationTestSeconds)
{
  int ourSocketIPv4 = setUpOurSocket(env, ourPort, AF_INET);
  int ourSocketIPv6 = setUpOurSocket(env, ourPort, AF_INET6);
  if (ourSocketIPv4 < 0 && ourSocketIPv6 < 0)
    return NULL;

  return new DynamicRTSPServer(env, ourSocketIPv4, ourSocketIPv6, ourPort,
                               authDatabase, reclamationTestSeconds);
}

DynamicRTSPServer::DynamicRTSPServer(UsageEnvironment &env, int ourSocketIPv4, int ourSocketIPv6,
                                     Port ourPort,
                                     UserAuthenticationDatabase *authDatabase, unsigned reclamationTestSeconds)
    : RTSPServer(env, ourSocketIPv4, ourSocketIPv6, ourPort, authDatabase, reclamationTestSeconds)
{
  _url_channel = -1;
  _url_level = -1;
  _url_multcast = 0;
  _url_tls = 0;
  _url_overtcp = 0;
  _url_filename = NULL; 
}

DynamicRTSPServer::~DynamicRTSPServer()
{
  _url_channel = -1;
  _url_level = -1;
  _url_multcast = 0;
  _url_tls = 0;
  _url_overtcp = 0;
  if(_url_filename)
    free(_url_filename);
  _url_filename = NULL;
}
int DynamicRTSPServer::DynamicRTSPServerBaseDir(char *dir)
{
  envir().UsageEnvironmentBaseDirSet(dir);
  return 0;
}

/*
* rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1
* rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&rtsp
* rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&multcast
* rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&tls
* rtsp://user:pass@192.168.1.1/media/channel=1&level=1
* rtsp://192.168.1.1:9988/media/channel=1&level=1
* rtsp://192.168.1.1:9988/media.h264&multcast
* rtsp://192.168.1.1:9988/media.h264
*/
void DynamicRTSPServer::DynamicRTSPServerParaURL(const char *url)
{
   char *p, *brk;
   char filename[512];
   _url_channel = -1;
   _url_level = -1;
   _url_multcast = 0;
   _url_tls = 0;
   _url_overtcp = 0;
   if(_url_filename)
    free(_url_filename);
   _url_filename = NULL;
   p = strstr((char*)url, "media");
   if(p)
       p+=6; 
   if(p)
   {
       brk = strstr(p, "channel");
       if(brk && strstr(p, "level"))
       {
           sscanf(brk, "channel=%d", &_url_channel);
           p = brk + 8;
           brk = strstr(p, "level");
           if(brk)
           {
               sscanf(brk, "level=%d", &_url_level);
               p = brk + 6;
           }
       }
       else if(!strstr(p, "channel") && !strstr(p, "level"))
       {
           _url_channel = _url_level = -1;
           memset(filename, 0, sizeof(filename));
           if(strstr(p, "&"))
           {
               sscanf(p, "%[^&]", filename);
           }
           else
               strcpy(filename, p);
           if(strlen(filename))
               _url_filename = strdup(filename);   
       }
       if(strstr(p, "&rtsp")||strstr(p, "&tcp"))
       {
           _url_overtcp = True;
       }
       else if(strstr(p, "&multcast"))
       {
           _url_multcast = True;
       }
       else if(strstr(p, "&tls"))
       {
           _url_tls = True;
       }
       else
       {
 
       }
   } 
  return ;
}



int DynamicRTSPServer::DynamicRTSPServerMediaAdd(const int channel, int level, const char *streamName)
{
    char const *descriptionString = "Session streamed by \"Live555 RTSPServer\"";
    // To make the second and subsequent client for each stream reuse the same
    // input stream as the first client (rather than playing the file from the
    // start for each client), change the following "False" to "True":
    // 同一个文件每一个客户端使用同时视频（而不是每一个客户端都是重头开始）
    Boolean reuseFirstSource = True;
    // To stream *only* MPEG-1 or 2 video "I" frames
    // (e.g., to reduce network bandwidth),
    // change the following "False" to "True":
    //Boolean iFramesOnly = False;
    zpl_media_channel_t *chninfo = zpl_media_channel_lookup(channel, level);
    if(chninfo && zpl_media_channel_isvideo(channel, level) == OK)
    {
      zpl_video_codec_t vcodec;
      if(zpl_media_channel_video_codec_get(channel, level, &vcodec) == OK)
      {
        OutPacketBuffer::maxSize = 100000;
        ServerMediaSession *sms = ServerMediaSession::createNew(envir(), streamName, streamName, descriptionString);
        sms->addSubsession(LiveVideoServerMediaSubssion::createNew(envir(), reuseFirstSource, vcodec.codectype, chninfo->rtsp_param.param));
        addServerMediaSession(sms);
      }
    }
    else if(chninfo && zpl_media_channel_isaudio(channel, level) == OK)
    {
      zpl_audio_codec_t acodec;
      if(zpl_media_channel_audio_codec_get(channel, level, &acodec) == OK)
      {
        ServerMediaSession *sms = ServerMediaSession::createNew(envir(), streamName, streamName, descriptionString);
        sms->addSubsession(LiveVideoServerMediaSubssion::createNew(envir(), reuseFirstSource, acodec.codectype, chninfo->rtsp_param.param));
        addServerMediaSession(sms);
      }
    }
    return 0;
}

void DynamicRTSPServer::lookupServerMediaSession(char const *streamName,
                                                  lookupServerMediaSessionCompletionFunc *completionFunc,
                                                  void *completionClientData,
                                                  Boolean isFirstLookupInSession)
{
  // First, check whether the specified "streamName" exists as a local file:
  char *filename = (char*)streamName;
  FILE *fid = NULL;
  Boolean fileExists = False;
  Boolean smsExists = False;

  DynamicRTSPServerParaURL(streamName);

  envir() << "streamName:" << streamName << " urlfile:" << _url_filename <<"\n";

  if(_url_channel >= 0 && _url_level >= 0)
  {
    if(zpl_media_channel_lookup(_url_channel, _url_level))
      fileExists = True;
  }
  if (strstr(streamName, "media"))
  {
    if(streamName[0] == '/')
      filename = (char*)(streamName + 7);
    else
      filename = (char*)(streamName + 6);  

    std::string stfilename = filename;
    std::string baseDirName = envir().baseDir;
    std::string baseFileName = envir().baseDir?(baseDirName + "/" + stfilename):("./" + stfilename);
    envir() << "baseFileName:" << baseFileName.c_str() << "\n";

    fid = fopen(baseFileName.c_str(), "rb");
    fileExists = fid != NULL;
  }
  envir() << "fileExists:" << fileExists << "\n";
  // Next, check whether we already have a "ServerMediaSession" for this file:
  ServerMediaSession *sms = getServerMediaSession(streamName);
  smsExists = sms != NULL;
  envir() << "smsExists:" << smsExists << "\n";

  // Handle the four possibilities for "fileExists" and "smsExists":
  if (!fileExists)
  {
    if (smsExists)
    {
      // "sms" was created for a file that no longer exists. Remove it:
      removeServerMediaSession(sms);
    }
    sms = NULL;
  }
  else
  {
    if (smsExists && isFirstLookupInSession)
    {
      if(_url_channel >= 0 && _url_level >= 0)
      {
        removeServerMediaSession(sms);
        DynamicRTSPServerMediaAdd(_url_channel, _url_level, streamName);
      }
    }
    if(fid)
      fclose(fid);
  }

  if (completionFunc != NULL)
  {
    (*completionFunc)(completionClientData, sms);
  }
}

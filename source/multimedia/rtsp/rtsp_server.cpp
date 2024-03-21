/**
 * @file      : rtsp_server.cpp
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-18
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#include <sstream>
#include <iostream>

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include "DynamicRTSPServer.hh"

#include "rtsp_server.h"



struct rtsp_srv_s
{
    char baseDir[256];
    pthread_mutex_t mutex;

    char m_stop;
    TaskScheduler *m_scheduler;
    UsageEnvironment *m_env;

    UserAuthenticationDatabase *m_auth_db;
    DynamicRTSPServer *m_rtsp_server;
};

// Special code for handling Matroska files:
struct MatroskaDemuxCreationState
{
  MatroskaFileServerDemux *demux;
  char watchVariable;
};

// Special code for handling Ogg files:
struct OggDemuxCreationState
{
  OggFileServerDemux *demux;
  char watchVariable;
};

#ifdef __cplusplus
extern "C"
{
#endif




// Special code for handling Matroska files:
static void onMatroskaDemuxCreation(MatroskaFileServerDemux *newDemux, void *clientData)
{
  MatroskaDemuxCreationState *creationState = (MatroskaDemuxCreationState *)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}
// END Special code for handling Matroska files:

// Special code for handling Ogg files:
static void onOggDemuxCreation(OggFileServerDemux *newDemux, void *clientData)
{
  OggDemuxCreationState *creationState = (OggDemuxCreationState *)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}
// END Special code for handling Ogg files:


rtsp_srv_t *rtsp_server_create(char *dir, int (*logcb)(const char *fmt, ...))
{
    rtsp_srv_t *info = (rtsp_srv_t *)malloc(sizeof(rtsp_srv_t));
    if (info)
    {
        memset(info, 0, sizeof(rtsp_srv_t));
        strcpy(info->baseDir, dir);
        info->m_stop = 0;
        info->m_auth_db = NULL;
        info->m_scheduler = BasicTaskScheduler::createNew();
        info->m_env = BasicUsageEnvironment::createNew(*info->m_scheduler);
        info->m_env->UsageEnvironmentLogSet(logcb);
        // OutPacketBuffer::maxSize = 60000; default
        // OutPacketBuffer::maxSize = 300000;//159458
        return info;
    }
    return NULL;
}

int rtsp_server_destroy(rtsp_srv_t *info)
{
    if (info->m_rtsp_server != NULL)
        Medium::close(info->m_rtsp_server);
    if (info->m_env != NULL)
    {
        info->m_env->reclaim();
        if (info->m_scheduler != NULL)
            delete info->m_scheduler;
    }
    free(info);
    return 0;
}

int rtsp_server_start(rtsp_srv_t *info, int port, unsigned int reclamationTestSeconds)
{
    if (info->m_rtsp_server == NULL)
        info->m_rtsp_server = DynamicRTSPServer::createNew(*info->m_env, port, info->m_auth_db, reclamationTestSeconds);
    if (info->m_rtsp_server != NULL)
    {
        rtsp_server_subsession_create(info, "h264", "media/test.264", "test.264");
        rtsp_server_subsession_create(info, "h264", "media/0-0-1970-01-01-00-48-39-video.H264", "0-0-1970-01-01-00-48-39-video.H264");
        // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
        // Try first with the default HTTP port (80), and then with the alternative HTTP
        // port numbers (8000 and 8080).
        if (strlen(info->baseDir))
            info->m_rtsp_server->DynamicRTSPServerBaseDir(info->baseDir);
        // m_rtsp_server->setLocalIPAddress("127.0.0.1", False);
        // m_env->UsageEnvironmentStart("127.0.0.1", True);
        if (info->m_rtsp_server->setUpTunnelingOverHTTP(80) || info->m_rtsp_server->setUpTunnelingOverHTTP(8000) || info->m_rtsp_server->setUpTunnelingOverHTTP(8080))
        {
            *info->m_env << "(We use port " << info->m_rtsp_server->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling).)\n";
        }
        else
        {
            *info->m_env << "(RTSP-over-HTTP tunneling is not available.)\n";
        }

        *info->m_env << "LIVE555 Media Server\n";
        *info->m_env << "\tversion " << LIVEMEDIA_LIBRARY_VERSION_STRING
                     << " (LIVE555 Streaming Media library version "
                     << LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";

        *info->m_env << "Play streams from this server using the URL\n";
        if (weHaveAnIPv4Address(*info->m_env))
        {
            char *rtspURLPrefix = info->m_rtsp_server->ipv4rtspURLPrefix();
            *info->m_env << "\t" << rtspURLPrefix << "<filename>\n";
            delete[] rtspURLPrefix;
            if (weHaveAnIPv6Address(*info->m_env))
                *info->m_env << "or\n";
        }
        if (weHaveAnIPv6Address(*info->m_env))
        {
            char *rtspURLPrefix = info->m_rtsp_server->ipv6rtspURLPrefix();
            *info->m_env << "\t" << rtspURLPrefix << "<filename>\n";
            delete[] rtspURLPrefix;
        }
        *info->m_env << "where <filename> is a file present in the current directory.\n";

        *info->m_env << "Each file's type is inferred from its name suffix:\n";
        *info->m_env << "\t\".264\" => a H.264 Video Elementary Stream file\n";
        *info->m_env << "\t\".265\" => a H.265 Video Elementary Stream file\n";
        *info->m_env << "\t\".aac\" => an AAC Audio (ADTS format) file\n";
        *info->m_env << "\t\".ac3\" => an AC-3 Audio file\n";
        *info->m_env << "\t\".amr\" => an AMR Audio file\n";
        *info->m_env << "\t\".dv\" => a DV Video file\n";
        *info->m_env << "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
        *info->m_env << "\t\".mkv\" => a Matroska audio+video+(optional)subtitles file\n";
        *info->m_env << "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
        *info->m_env << "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
        *info->m_env << "\t\".ogg\" or \".ogv\" or \".opus\" => an Ogg audio and/or video file\n";
        *info->m_env << "\t\".ts\" => a MPEG Transport Stream file\n";
        *info->m_env << "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
        *info->m_env << "\t\".vob\" => a VOB (MPEG-2 video with AC-3 audio) file\n";
        *info->m_env << "\t\".wav\" => a WAV Audio file\n";
        *info->m_env << "\t\".webm\" => a WebM audio(Vorbis)+video(VP8) file\n";
        *info->m_env << "See http://www.live555.com/mediaServer/ for additional documentation.\n";

        return 0;
    }
    return -1;
}

int rtsp_server_event_loop(rtsp_srv_t *info)
{
    info->m_env->taskScheduler().doEventLoop(&info->m_stop);
    return 0;
}
int rtsp_server_event_loop_interval(rtsp_srv_t *info, unsigned int maxDelayTime)
{
    info->m_env->taskScheduler().doEventLoop(&info->m_stop, maxDelayTime);
    return 0;
}
int rtsp_server_add_username(rtsp_srv_t *info, const char *username, const char *password)
{
    if (info->m_auth_db == NULL)
    {
        info->m_auth_db = new UserAuthenticationDatabase;
    }
    if (info->m_auth_db != NULL)
    {
        info->m_auth_db->addUserRecord(username, password); // replace these with real strings
        return 0;
    }
    return -1;
}

int rtsp_server_add_session(rtsp_srv_t *info, const char *sessionName, void *subSession)
{
    ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, sessionName);
    if (sms)
    {
        sms->addSubsession((ServerMediaSubsession *)subSession);
        info->m_rtsp_server->addServerMediaSession(sms);
        char *url = info->m_rtsp_server->rtspURL(sms);
        *info->m_env << "Play this stream using the URL: " << url << "\n";
        return 0;
    }
    return -1;
}

int rtsp_server_delete_session(rtsp_srv_t *info, const char *sessionName, void *subSession)
{
    ServerMediaSession *resultSession = NULL;
    ServerMediaSession::lookupByName(*info->m_env, sessionName, resultSession);
    if (resultSession)
    {
        if (resultSession->numSubsessions() == 0)
        {
            resultSession->deleteAllSubsessions();
            info->m_rtsp_server->removeServerMediaSession(resultSession);
            // delete resultSession;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    return -1;
}

int rtsp_server_update_session(rtsp_srv_t *info, const char *sessionName, void *subSession)
{
    ServerMediaSession *resultSession = NULL;
    ServerMediaSession::lookupByName(*info->m_env, sessionName, resultSession);
    if (resultSession)
    {
        info->m_rtsp_server->removeServerMediaSession(resultSession);
        resultSession->deleteAllSubsessions();
        // delete resultSession;
    }
    resultSession = ServerMediaSession::createNew(*info->m_env, sessionName);
    if (resultSession)
    {
        resultSession->addSubsession((ServerMediaSubsession *)subSession);
        info->m_rtsp_server->addServerMediaSession(resultSession);
        char *url = info->m_rtsp_server->rtspURL(resultSession);
        *info->m_env << "Play this stream using the URL: " << url << "\n";
        return 0;
    }
    return -1;
}

//添加一个音视频流
int rtsp_server_subsession_create(rtsp_srv_t *info, const char *codecname, const char *streamName, const char *inputFileName)
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
    Boolean iFramesOnly = False;
    // sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True));
    if (strcasecmp(codecname, "H264") == 0)
    {
        OutPacketBuffer::maxSize = 100000;
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    else if (strcasecmp(codecname, "H265") == 0)
    {
        OutPacketBuffer::maxSize = 100000;
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    else if (strcasecmp(codecname, "mpeg4") == 0)
    {
        // MPEG-4 video
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    // A MPEG-1 or 2 audio+video program stream
    else if (strcasecmp(codecname, "mpg") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        MPEG1or2FileServerDemux *demux = MPEG1or2FileServerDemux::createNew(*info->m_env, inputFileName, True);
        sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
        sms->addSubsession(demux->newAudioServerMediaSubsession());
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    else if (strcasecmp(codecname, "mpeg2") == 0)
    {
        // A MPEG-1 or 2 video elementary stream
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(MPEG1or2VideoFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True, iFramesOnly));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    else if (strcasecmp(codecname, "mp3") == 0)
    {
        // A MP3 audio stream (actually, any MPEG-1 or 2 audio file will work):
        // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
        // #define STREAM_USING_ADUS 1
        // To also reorder ADUs before streaming, uncomment the following:
        // #define INTERLEAVE_ADUS 1
        // (For more information about ADUs and interleaving,
        //  see <http://www.live555.com/rtp-mp3/>)
        Boolean useADUs = False;
        Interleaving *interleaving = NULL;
#ifdef STREAM_USING_ADUS
        useADUs = True;
#ifdef INTERLEAVE_ADUS
        unsigned char interleaveCycle[] = {0, 2, 1, 3}; // or choose your own...
        unsigned const interleaveCycleSize = (sizeof interleaveCycle) / (sizeof(unsigned char));
        interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True, useADUs, interleaving));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    else if (strcasecmp(codecname, "pcm") == 0)
    {
        // A WAV audio stream
        // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
        // change the following to True:
        Boolean convertToULaw = False;
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(MPEG1or2VideoFileServerMediaSubsession::createNew(*info->m_env, inputFileName, True, convertToULaw));
        info->m_rtsp_server->addServerMediaSession(sms);
    }
    // An AMR audio stream:
    else if (strcasecmp(codecname, "amr") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(AMRAudioFileServerMediaSubsession ::createNew(*info->m_env, inputFileName, True));
        info->m_rtsp_server->addServerMediaSession(sms);
    }

    // A 'VOB' file (e.g., from an unencrypted DVD):
    else if (strcasecmp(codecname, "mpeg3-ac") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        // Note: VOB files are MPEG-2 Program Stream files, but using AC-3 audio
        MPEG1or2FileServerDemux *demux = MPEG1or2FileServerDemux::createNew(*info->m_env, inputFileName, reuseFirstSource);
        sms->addSubsession(demux->newVideoServerMediaSubsession(iFramesOnly));
        sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
        info->m_rtsp_server->addServerMediaSession(sms);
    }

    // A MPEG-2 Transport Stream:
    else if (strcasecmp(codecname, "mpeg-2") == 0)
    {
        // char const* streamName = "mpeg2TransportStreamTest";
        // char const* inputFileName = "test.ts";
        char const *indexFileName = "test.tsx";
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName);
        sms->addSubsession(MPEG2TransportFileServerMediaSubsession ::createNew(*info->m_env, inputFileName, indexFileName, reuseFirstSource));
        info->m_rtsp_server->addServerMediaSession(sms);

        // announceStream(info->m_rtsp_server, sms, streamName, inputFileName);
    }

    // An AAC audio stream (ADTS-format file):
    else if (strcasecmp(codecname, "aac") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName, descriptionString);
        sms->addSubsession(ADTSAudioFileServerMediaSubsession ::createNew(*info->m_env, inputFileName, reuseFirstSource));
        info->m_rtsp_server->addServerMediaSession(sms);
    }

    // A DV video stream:
    else if (strcasecmp(codecname, "dv") == 0)
    {
        // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
        OutPacketBuffer::maxSize = 300000;
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                descriptionString);
        sms->addSubsession(DVVideoFileServerMediaSubsession ::createNew(*info->m_env, inputFileName, reuseFirstSource));
        info->m_rtsp_server->addServerMediaSession(sms);
    }

    // A AC3 video elementary stream:
    else if (strcasecmp(codecname, "ac3") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                descriptionString);

        sms->addSubsession(AC3AudioFileServerMediaSubsession ::createNew(*info->m_env, inputFileName, reuseFirstSource));

        info->m_rtsp_server->addServerMediaSession(sms);
    }

    // A Matroska ('.mkv') file, with video+audio+subtitle streams:
    else if (strcasecmp(codecname, "mkv") == 0)
    {
        // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
        OutPacketBuffer::maxSize = 300000; // allow for some possibly large VP8 or VP9 frames
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                "Matroska video+audio+(optional)subtitles");
        // Create a Matroska file server demultiplexor for the specified file.
        // (We enter the event loop to wait for this to complete.)
        MatroskaDemuxCreationState creationState;
        creationState.watchVariable = 0;
        MatroskaFileServerDemux::createNew(*info->m_env, inputFileName, onMatroskaDemuxCreation, &creationState);
        info->m_env->taskScheduler().doEventLoop(&creationState.watchVariable);
        Boolean sessionHasTracks = False;
        ServerMediaSubsession *smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL)
        {
            sms->addSubsession(smss);
            sessionHasTracks = True;
        }
        if (sessionHasTracks)
        {
            info->m_rtsp_server->addServerMediaSession(sms);
        }        
    }

    // A WebM ('.webm') file, with video(VP8)+audio(Vorbis) streams:
    // (Note: ".webm' files are special types of Matroska files, so we use the same code as the Matroska ('.mkv') file code above.)
    else if (strcasecmp(codecname, "vp8") == 0)
    {
        // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
        OutPacketBuffer::maxSize = 300000; // allow for some possibly large VP8 or VP9 frames
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                descriptionString);
        MatroskaDemuxCreationState creationState;
        creationState.watchVariable = 0;
        MatroskaFileServerDemux::createNew(*info->m_env, inputFileName, onMatroskaDemuxCreation, &creationState);
        info->m_env->taskScheduler().doEventLoop(&creationState.watchVariable);
        Boolean sessionHasTracks = False;
        ServerMediaSubsession *smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL)
        {
            sms->addSubsession(smss);
            sessionHasTracks = True;
        }
        if (sessionHasTracks)
        {
            info->m_rtsp_server->addServerMediaSession(sms);
        }  
    }

    // An Ogg ('.ogg') file, with video and/or audio streams:
    else if (strcasecmp(codecname, "ogg") == 0)
    {
        // Assumed to be an Ogg file
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                    "Ogg video and/or audio");
        // Create a Ogg file server demultiplexor for the specified file.
        // (We enter the event loop to wait for this to complete.)
        OggDemuxCreationState creationState;
        creationState.watchVariable = 0;
        Boolean sessionHasTracks = False;
        OggFileServerDemux::createNew(*info->m_env, inputFileName, onOggDemuxCreation, &creationState);
        info->m_env->taskScheduler().doEventLoop(&creationState.watchVariable);

        ServerMediaSubsession *smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL)
        {
        sms->addSubsession(smss);
        sessionHasTracks = True;
        }
        if (sessionHasTracks)
        {
            info->m_rtsp_server->addServerMediaSession(sms);
        }
    }

    // An Opus ('.opus') audio file:
    // (Note: ".opus' files are special types of Ogg files, so we use the same code as the Ogg ('.ogg') file code above.)
    else if (strcasecmp(codecname, "opus") == 0)
    {
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                    descriptionString);
        // Create a Ogg file server demultiplexor for the specified file.
        // (We enter the event loop to wait for this to complete.)
        OggDemuxCreationState creationState;
        creationState.watchVariable = 0;
        Boolean sessionHasTracks = False;
        OggFileServerDemux::createNew(*info->m_env, inputFileName, onOggDemuxCreation, &creationState);
        info->m_env->taskScheduler().doEventLoop(&creationState.watchVariable);

        ServerMediaSubsession *smss;
        while ((smss = creationState.demux->newServerMediaSubsession()) != NULL)
        {
        sms->addSubsession(smss);
        sessionHasTracks = True;
        }
        if (sessionHasTracks)
        {
            info->m_rtsp_server->addServerMediaSession(sms);
        }
    }

    // A MPEG-2 Transport Stream, coming from a live UDP (raw-UDP or RTP/UDP) source:
    else if (strcasecmp(codecname, "mpeg-2-udp") == 0)
    {
        char const *streamName = "mpeg2TransportStreamFromUDPSourceTest";
        char const *inputAddressStr = "239.255.42.42";
        // This causes the server to take its input from the stream sent by the "testMPEG2TransportStreamer" demo application.
        // (Note: If the input UDP source is unicast rather than multicast, then change this to NULL.)
        portNumBits const inputPortNum = 1234;
        // This causes the server to take its input from the stream sent by the "testMPEG2TransportStreamer" demo application.
        Boolean const inputStreamIsRawUDP = False;
        ServerMediaSession *sms = ServerMediaSession::createNew(*info->m_env, streamName, streamName,
                                                                descriptionString);
        sms->addSubsession(MPEG2TransportUDPServerMediaSubsession ::createNew(*info->m_env, inputAddressStr, inputPortNum, inputStreamIsRawUDP));
        info->m_rtsp_server->addServerMediaSession(sms);

        *info->m_env << "\n\"" << streamName << "\" stream, from a UDP Transport Stream input source \n\t(";
        if (inputAddressStr != NULL)
        {
            *info->m_env << "IP multicast address " << inputAddressStr << ",";
        }
        else
        {
            *info->m_env << "unicast;";
        }
        *info->m_env << " port " << inputPortNum << ")\n";
    }
    return -1;
}

#ifdef __cplusplus
};
#endif
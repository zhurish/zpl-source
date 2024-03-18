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

#ifdef __cplusplus
extern "C"
{
#endif

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
            // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
            // Try first with the default HTTP port (80), and then with the alternative HTTP
            // port numbers (8000 and 8080).
            if(strlen(info->baseDir))
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

    int rtsp_server_event_loop_running(rtsp_srv_t *info)
    {
        info->m_env->taskScheduler().doEventLoop(&info->m_stop);
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

#ifdef __cplusplus
};
#endif
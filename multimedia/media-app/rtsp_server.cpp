/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtsp_server.h
** 
** RTSP server
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>

#include <liveMedia/liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "rtsp_server.hpp"

#ifndef RTSP_SERVER_EXTERN
rtsp_server::rtsp_server()
    : m_stop(0)
{
}

#else
rtsp_server *
rtsp_server::createNew(UsageEnvironment &env, Port ourPort,
                       UserAuthenticationDatabase *authDatabase,
                       unsigned reclamationTestSeconds)
{
    int ourSocket = setUpOurSocket(env, ourPort);
    if (ourSocket == -1)
        return NULL;

    return new rtsp_server(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

rtsp_server::rtsp_server(UsageEnvironment &env, int ourSocket,
                         Port ourPort,
                         UserAuthenticationDatabase *authDatabase, unsigned reclamationTestSeconds)
    : RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds)
{
    m_stop = 0;
    m_env = &env;	
    //m_rtsp_server = nullptr;
    m_auth_db = authDatabase;
    reuseSource = False;
}
#endif

rtsp_server::~rtsp_server()
{
    if (m_rtsp_server != nullptr)
        Medium::close(m_rtsp_server);
    if (m_env != nullptr)
    {
        TaskScheduler *scheduler = &(m_env->taskScheduler());
        m_env->reclaim();
        if (scheduler != nullptr)
            delete scheduler;
    }
}

int rtsp_server::rtsp_server_initalize()
{
#ifndef RTSP_SERVER_EXTERN
    if (m_env == nullptr)
    {
        TaskScheduler *scheduler = BasicTaskScheduler::createNew();
        if (scheduler != nullptr)
        {
            m_env = BasicUsageEnvironment::createNew(*scheduler);
        }
    }
#endif
    if (m_env != nullptr)
        return 0;
    return -1;
}

int rtsp_server::rtsp_server_add_username(const std::string &username, const std::string &password)
{
    if (m_auth_db == nullptr)
    {
        m_auth_db = new UserAuthenticationDatabase;
    }
    if (m_auth_db != nullptr)
    {
        m_auth_db->addUserRecord(username.c_str(), password.c_str()); // replace these with real strings
        return 0;
    }
    return -1;
}

int rtsp_server::rtsp_server_start(Port ourPort, ospl_uint32 reclamationTestSeconds)
{
    /*int ourSocket = RTSPServer::setUpOurSocket(m_env, ourPort);
    if (ourSocket == -1) 
        return -1;*/
    //if(m_auth_db != nullptr)
    if (m_rtsp_server == nullptr)
        m_rtsp_server = RTSPServer::createNew(*m_env, ourPort, m_auth_db, reclamationTestSeconds);
    if (m_rtsp_server != nullptr)
        return 0;
    return -1;
}

int rtsp_server::rtsp_server_tunneling_over_HTTP(int rtspOverHTTPPort)
{
    if (!rtsp_server_available())
        return -1;
    if (m_rtsp_server->setUpTunnelingOverHTTP(rtspOverHTTPPort)
        /*|| m_rtsp_server->setUpTunnelingOverHTTP(8000) || m_rtsp_server->setUpTunnelingOverHTTP(8080)*/)
    {
        *m_env << "(We use port " << m_rtsp_server->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
    }
    else
    {
        *m_env << "(RTSP-over-HTTP tunneling is not available.)\n";
    }
    return 0;
}

Boolean rtsp_server::rtsp_server_available()
{
    if ((m_env != nullptr) && (m_rtsp_server != nullptr))
        return True;
    std::cout << "rtsp_server_available " << std::endl;
    return False;
}

int rtsp_server::rtsp_server_event_loop_running(char *stop)
{
    if (!rtsp_server_available())
        return -1;
    m_env->taskScheduler().doEventLoop(stop);
    return 0;
}

int rtsp_server::rtsp_server_event_loop_running()
{
    if (!rtsp_server_available())
        return -1;
    m_env->taskScheduler().doEventLoop();
    return 0;
}

int rtsp_server::rtsp_server_event_loop_stop()
{
    m_stop = 1;
    return 0;
}

UsageEnvironment *rtsp_server::rtsp_server_env()
{
    return m_env;
}

std::string rtsp_server::rtsp_server_get_result_msg()
{
    std::string result("UsageEnvironment not exists");
    if (m_env)
    {
        result = m_env->getResultMsg();
    }
    return result;
}

ServerMediaSession *rtsp_server::rtsp_server_add_session_node(const std::string &sessionName, const std::string &fileName)
{
    std::string extension = fileName.substr(fileName.find_last_of('.'));
    //if (extension.)
    //    return nullptr;
    ServerMediaSession *sms = ServerMediaSession::createNew(*m_env, sessionName.c_str());
    if (sms)
    {
        if (extension == ".aac")
        {
            // Assumed to be an AAC Audio (ADTS format) file:
            sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".amr")
        {
            // Assumed to be an AMR Audio file:
            sms->addSubsession(AMRAudioFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".ac3")
        {
            // Assumed to be an AC-3 Audio file:
            sms->addSubsession(AC3AudioFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".m4e")
        {
            // Assumed to be a MPEG-4 Video Elementary Stream file:
            sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".264")
        {
            // Assumed to be a H.264 Video Elementary Stream file:
            //OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.264 frames
            sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".265")
        {
            // Assumed to be a H.265 Video Elementary Stream file:
            //OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.265 frames
            sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        }
        else if (extension == ".mp3")
        {
            // Assumed to be a MPEG-1 or 2 Audio file:
            Boolean useADUs = False;
            Interleaving *interleaving = nullptr;
#ifdef STREAM_USING_ADUS
            useADUs = True;
#ifdef INTERLEAVE_ADUS
            ospl_uint8 interleaveCycle[] = {0, 2, 1, 3}; // or choose your own...
            unsigned const interleaveCycleSize = (sizeof interleaveCycle) / (sizeof(ospl_uint8));
            interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
            sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource, useADUs, interleaving));
        }
        else if (extension == ".mpg")
        {
            // Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
            MPEG1or2FileServerDemux *demux = MPEG1or2FileServerDemux::createNew(*m_env, fileName.c_str(), reuseSource);
            sms->addSubsession(demux->newVideoServerMediaSubsession());
            sms->addSubsession(demux->newAudioServerMediaSubsession());
        }
        else if (extension == ".vob")
        {
            // Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
            MPEG1or2FileServerDemux *demux = MPEG1or2FileServerDemux::createNew(*m_env, fileName.c_str(), reuseSource);
            sms->addSubsession(demux->newVideoServerMediaSubsession());
            sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
        }
        else if (extension == ".ts")
        {
            // Assumed to be a MPEG Transport Stream file:
            // Use an index file name that's the same as the TS file name, except with ".tsx":
            unsigned indexFileNameLen = strlen(fileName.c_str()) + 2; // allow for trailing "x\0"
            char *indexFileName = new char[indexFileNameLen];
            sprintf(indexFileName, "%sx", fileName.c_str());
            sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), indexFileName, reuseSource));
            delete[] indexFileName;
        }
        else if (extension == ".wav")
        {
            // Assumed to be a WAV Audio file:
            Boolean convertToULaw = False;
            sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource, convertToULaw));
        } //else if (extension == ".dv") {
        // Assumed to be a DV Video file
        // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
        //OutPacketBuffer::maxSize = 300000;
        //sms->addSubsession(DVVideoFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseSource));
        //}
        //sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*m_env, fileName.c_str(), reuseFirstSource));
        m_rtsp_server->addServerMediaSession(sms);
    }
    return sms;
}

ServerMediaSession *rtsp_server::rtsp_server_add_session_node(const std::string &sessionName, ServerMediaSubsession *subSession)
{
    if (!rtsp_server_available())
        return nullptr;
    ServerMediaSession *sms = ServerMediaSession::createNew(*m_env, sessionName.c_str());
    if (sms)
    {
        sms->addSubsession(subSession);
        m_rtsp_server->addServerMediaSession(sms);
        char *url = m_rtsp_server->rtspURL(sms);
        return sms;
    }
    return nullptr;
}

int rtsp_server::rtsp_server_add_session(const std::string &sessionName, const std::string &fileName)
{
    if (!rtsp_server_available())
        return -1;
    ServerMediaSession *sms = rtsp_server_add_session_node(sessionName, fileName);
    if (sms)
    {
        char *url = m_rtsp_server->rtspURL(sms);
        return 0;
    }
    return -1;
}

int rtsp_server::rtsp_server_add_session(const std::string &sessionName, ServerMediaSubsession *subSession)
{
    if (!rtsp_server_available())
        return -1;
    ServerMediaSession *sms = ServerMediaSession::createNew(*m_env, sessionName.c_str());
    if (sms)
    {
        sms->addSubsession(subSession);
        m_rtsp_server->addServerMediaSession(sms);
        char *url = m_rtsp_server->rtspURL(sms);
        return 0;
    }
    return -1;
}

#ifdef RTSP_SERVER_EXTERN
ServerMediaSession *rtsp_server::lookupServerMediaSession(char const *streamName, Boolean isFirstLookupInSession)
{
    // First, check whether the specified "streamName" exists as a local file:
    FILE *fid = fopen(streamName, "rb");
    Boolean const fileExists = fid != NULL;

    // Next, check whether we already have a "ServerMediaSession" for this file:
    ServerMediaSession *sms = RTSPServer::lookupServerMediaSession(streamName);
    Boolean const smsExists = sms != NULL;

    // Handle the four possibilities for "fileExists" and "smsExists":
    if (!fileExists)
    {
        if (smsExists)
        {
            // "sms" was created for a file that no longer exists. Remove it:
            removeServerMediaSession(sms);
            sms = NULL;
        }

        return NULL;
    }
    else
    {
        if (smsExists && isFirstLookupInSession)
        {
            // Remove the existing "ServerMediaSession" and create a new one, in case the underlying
            // file has changed in some way:
            removeServerMediaSession(sms);
            sms = NULL;
        }

        if (sms == NULL)
        {
            sms = rtsp_server_add_session_node(streamName, streamName);
            addServerMediaSession(sms);
        }
        fclose(fid);
        return sms;
    }
}
#endif

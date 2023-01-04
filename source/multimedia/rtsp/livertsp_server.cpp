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

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "livertsp_server.hpp"
#include <GroupsockHelper.hh> 
#include "livertsp_server.h"

static livertsp_server *_rtsp_server = NULL;



livertsp_server::livertsp_server()
    : m_stop(0)
{
    m_stop = 1;
    m_scheduler = NULL;
    m_env = NULL;

    m_auth_db = NULL;
    reuseSource = False;
    m_livertsp_server = NULL;
    //OutPacketBuffer::maxSize = 60000; default
    //OutPacketBuffer::maxSize = 300000;//159458
}


livertsp_server::~livertsp_server()
{
    if (m_livertsp_server != NULL)
        Medium::close(m_livertsp_server);
    if (m_env != NULL)
    {
        m_env->reclaim();
        if (m_scheduler != NULL)
            delete m_scheduler;
    }
}

int livertsp_server::livertsp_server_initalize()
{
    if (m_env == NULL)
    {
        if(m_scheduler == NULL)
            m_scheduler = BasicTaskScheduler::createNew();
        if (m_scheduler != NULL)
        {
            m_env = BasicUsageEnvironment::createNew(*m_scheduler);
        }
    }
    if (m_env != NULL)
    {
        return 0;
    }
    return -1;
}

int livertsp_server::livertsp_server_initalize(int (*logcb)(const char *fmt,...))
{
    if (m_env == NULL)
    {
        if(m_scheduler == NULL)
            m_scheduler = BasicTaskScheduler::createNew();
        if (m_scheduler != NULL)
        {
            m_env = BasicUsageEnvironment::createNew(*m_scheduler);
        }
    }
    if (m_env != NULL)
    {
        if(logcb)
            m_env->UsageEnvironmentLogSet(logcb);
        return 0;
    }
    return -1;
}


int livertsp_server::livertsp_server_add_username(const char *username, const char *password)
{
    if (m_auth_db == NULL)
    {
        m_auth_db = new UserAuthenticationDatabase;
    }
    if (m_auth_db != NULL)
    {
        m_auth_db->addUserRecord(username, password); // replace these with real strings
        return 0;
    }
    return -1;
}

int livertsp_server::livertsp_server_start(Port ourPort, unsigned int reclamationTestSeconds)
{
    if (m_livertsp_server == NULL)
        m_livertsp_server = DynamicRTSPServer::createNew(*m_env, ourPort, m_auth_db, reclamationTestSeconds);
    if (m_livertsp_server != NULL)
    {
        m_livertsp_server->setLocalIPAddress("127.0.0.1", False);
        m_env->UsageEnvironmentStart("127.0.0.1", True);
        return 0;
    }
    return -1;
}

int livertsp_server::livertsp_server_tunneling_over_HTTP(int rtspOverHTTPPort)
{
    if (!livertsp_server_available())
        return -1;
    if (m_livertsp_server->setUpTunnelingOverHTTP(rtspOverHTTPPort)
            /*|| m_livertsp_server->setUpTunnelingOverHTTP(8000) || m_livertsp_server->setUpTunnelingOverHTTP(8080)*/)
    {
        *m_env << "(We use port " << m_livertsp_server->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
    }
    else
    {
        *m_env << "(RTSP-over-HTTP tunneling is not available.)\n";
    }
    return 0;
}

Boolean livertsp_server::livertsp_server_available()
{
    if ((m_env != NULL) && (m_livertsp_server != NULL))
        return True;
    std::cout << "livertsp_server_available " << std::endl;
    return False;
}

int livertsp_server::livertsp_server_event_loop_running(char *stop)
{
    if (!livertsp_server_available())
        return -1;
    m_env->taskScheduler().doEventLoop(stop);
    return 0;
}

int livertsp_server::livertsp_server_event_loop_running(char *stop, unsigned delay)
{
    if (!livertsp_server_available())
        return -1;
    m_env->taskScheduler().doEventLoop(stop, delay);
    return 0;
}

int livertsp_server::livertsp_server_event_loop_running()
{
    if (!livertsp_server_available())
        return -1;
   *m_env << "LIVE555 Media Server\n";
  *m_env << "\tversion " << "aaaa"
       << " (LIVE555 Streaming Media library version "
       << "sadada" << ").\n";

  *m_env << "Play streams from this server using the URL\n";
  if (weHaveAnIPv4Address(*m_env)) {
    char* rtspURLPrefix = m_livertsp_server->ipv4rtspURLPrefix();
    *m_env << "\t" << rtspURLPrefix << "<filename>\n";
    delete[] rtspURLPrefix;
    if (weHaveAnIPv6Address(*m_env)) *m_env << "or\n";
  }
  if (weHaveAnIPv6Address(*m_env)) {
    char* rtspURLPrefix = m_livertsp_server->ipv6rtspURLPrefix();
    *m_env << "\t" << rtspURLPrefix << "<filename>\n";
    delete[] rtspURLPrefix;
  }
    m_env->taskScheduler().doEventLoop();
    return 0;
}

int livertsp_server::livertsp_server_event_loop_stop()
{
    m_stop = 1;
    return 0;
}

UsageEnvironment *livertsp_server::livertsp_server_env()
{
    return m_env;
}

std::string livertsp_server::livertsp_server_get_result_msg()
{
    std::string result("UsageEnvironment not exists");
    if (m_env)
    {
        result = m_env->getResultMsg();
    }
    return result;
}


int livertsp_server::livertsp_server_add_session(const char *sessionName, ServerMediaSubsession *subSession)
{
    if (!livertsp_server_available())
        return -1;
    ServerMediaSession *sms = ServerMediaSession::createNew(*m_env, sessionName);
    if (sms)
    {
        sms->addSubsession(subSession);
        m_livertsp_server->addServerMediaSession(sms);
        char *url = m_livertsp_server->rtspURL(sms);
        *m_env << "Play this stream using the URL: " << url << "\n";
        return 0;
    }
    return -1;
}

int livertsp_server::livertsp_server_delete_session(const char *sessionName, ServerMediaSubsession *subSession)
{
    if (!livertsp_server_available())
        return -1;
    ServerMediaSession *resultSession = NULL;
    ServerMediaSession::lookupByName(*m_env, sessionName, resultSession);
    if(resultSession)
    {
        if(resultSession->numSubsessions() == 0)
        {
            resultSession->deleteAllSubsessions();
            m_livertsp_server->removeServerMediaSession(resultSession);
            //delete resultSession;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    return -1;
}

int livertsp_server::livertsp_server_update_session(const char *sessionName, ServerMediaSubsession *subSession)
{
    if (!livertsp_server_available())
        return -1;
    ServerMediaSession *resultSession = NULL;
    ServerMediaSession::lookupByName(*m_env, sessionName, resultSession);
    if(resultSession)
    {
        m_livertsp_server->removeServerMediaSession(resultSession);
        resultSession->deleteAllSubsessions();
        //delete resultSession;
    }
    resultSession = ServerMediaSession::createNew(*m_env, sessionName);
    if (resultSession)
    {
        resultSession->addSubsession(subSession);
        m_livertsp_server->addServerMediaSession(resultSession);
        char *url = m_livertsp_server->rtspURL(resultSession);
        *m_env << "Play this stream using the URL: " << url << "\n";
        return 0;
    }
    return -1;
}

void livertsp_server::livertsp_server_control_create(int sock) {
    m_env->taskScheduler().setBackgroundHandling(sock, SOCKET_READABLE,
                                                  livertsp_server_control_handler, NULL);
}

void livertsp_server::livertsp_server_control_handler(void* instance, int /*mask*/) {
  //livertsp_server* connection = (livertsp_server*)instance;
  //connection->incomingRequestHandler();
}

int livertsp_server_loop(void*p)
{
    if(_rtsp_server)
    {
        _rtsp_server->livertsp_server_event_loop_running();
    }
    return 0;
}


int livertsp_server_init(int port, char *base, int (*logcb)(const char *fmt,...))
{
    _rtsp_server = new livertsp_server ();
    if(_rtsp_server)
    {
        if(base)
            UsageEnvironmentBaseDir(base);
        if(_rtsp_server->livertsp_server_initalize(logcb) == 0)
         return _rtsp_server->livertsp_server_start(8554);
    }
    return -1;
}

int livertsp_server_exit(void)
{
    if(_rtsp_server)
        delete _rtsp_server;
    return 0;
}


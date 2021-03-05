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
#ifndef __RTSP_SERVER_APP_HPP__
#define __RTSP_SERVER_APP_HPP__

//#pragma once
#include <iostream>
#include <fstream>
#include <list>
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif
// live555
//#include <liveMedia/liveMedia.hh>
//#include <BasicUsageEnvironment/BasicUsageEnvironment.hh>
//#include <groupsock/GroupsockHelper.hh>

//#define RTSP_SERVER_EXTERN

#ifdef RTSP_SERVER_EXTERN
class rtsp_server: public RTSPServer {
#else
class rtsp_server {
#endif    
    public:
#ifdef RTSP_SERVER_EXTERN
        static rtsp_server*rtsp_server::createNew(UsageEnvironment& env, Port ourPort,
			     UserAuthenticationDatabase* authDatabase,
			     unsigned reclamationTestSeconds = 65);
    protected:
        rtsp_server(UsageEnvironment& env, int ourSocket,
				     Port ourPort,
				     UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds = 65);
        
        virtual ~rtsp_server();
#else  
        rtsp_server();
        virtual ~rtsp_server();
#endif
    public:
        int rtsp_server_initalize();
        int rtsp_server_start(Port ourPort, ospl_uint32 reclamationTestSeconds = 65);
        int rtsp_server_tunneling_over_HTTP(int rtspOverHTTPPort);

        int rtsp_server_event_loop_running(char * stop);
        int rtsp_server_event_loop_running();
        int rtsp_server_event_loop_stop();

        Boolean rtsp_server_available() ;
        UsageEnvironment* rtsp_server_env();
        std::string rtsp_server_get_result_msg();


        int rtsp_server_add_username(const std::string & username, const std::string & password);

        int rtsp_server_add_session(const std::string & sessionName, const std::string & fileName);

        int rtsp_server_add_session(const std::string & sessionName, ServerMediaSubsession* subSession);
        
    private:
        ServerMediaSession* rtsp_server_add_session_node(const std::string & sessionName, const std::string & fileName);
        ServerMediaSession* rtsp_server_add_session_node(const std::string & sessionName, ServerMediaSubsession* subSession);
#ifdef RTSP_SERVER_EXTERN
    protected: // redefined virtual functions
        virtual ServerMediaSession* lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
#endif
    protected:
        char                m_stop = 0;
        //TaskScheduler*      scheduler = nullptr;
        UsageEnvironment*   m_env = nullptr;	
        RTSPServer*         m_rtsp_server = nullptr;
        UserAuthenticationDatabase* m_auth_db = nullptr;
        Boolean             reuseSource = False;
};


#endif /* __RTSP_SERVER_APP_HPP__ */
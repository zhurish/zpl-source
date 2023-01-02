/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** livertsp_server.h
** 
** RTSP server
**
** -------------------------------------------------------------------------*/
#ifndef __LIVERTSP_SERVER_HPP__
#define __LIVERTSP_SERVER_HPP__

//#pragma once
#include <iostream>
#include <fstream>
#include <list>
#ifdef __cplusplus
extern "C" {
#endif
//#include "zpl_type.h"
#ifdef __cplusplus
}
#endif
// live555
#include <liveMedia.hh>
#include "DynamicRTSPServer.hh"


class livertsp_server {

    public: 
        livertsp_server();
        virtual ~livertsp_server();
    public:
        int livertsp_server_initalize();
        int livertsp_server_initalize(int (*logcb)(const char *fmt,...));
        int livertsp_server_start(Port ourPort, unsigned int reclamationTestSeconds = 65);
        int livertsp_server_tunneling_over_HTTP(int rtspOverHTTPPort);

        int livertsp_server_event_loop_running(char *stop, unsigned delay);
        int livertsp_server_event_loop_running(char * stop);
        int livertsp_server_event_loop_running();
        int livertsp_server_event_loop_stop();

        Boolean livertsp_server_available() ;
        UsageEnvironment* livertsp_server_env();
        std::string livertsp_server_get_result_msg();


        int livertsp_server_add_username(const char *username, const char * password);

        int livertsp_server_add_session(const char * sessionName, ServerMediaSubsession* subSession);

        int livertsp_server_delete_session(const char *sessionName, ServerMediaSubsession *subSession);
        int livertsp_server_update_session(const char *sessionName, ServerMediaSubsession *subSession);

        void livertsp_server_control_create(int sock);

        static void livertsp_server_control_handler(void *instance, int /*mask*/);

    protected:
        char                m_stop = 0;
        TaskScheduler*      m_scheduler = NULL;
        UsageEnvironment*   m_env = NULL;	

        UserAuthenticationDatabase* m_auth_db = NULL;
        Boolean             reuseSource = False;
public:
        DynamicRTSPServer*      m_livertsp_server = NULL;
};


#endif /* __LIVERTSP_SERVER_HPP__ */

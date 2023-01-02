/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** livertsp_client.h
** 
** RTSP client
**
** -------------------------------------------------------------------------*/
#ifndef __LIVERTSP_CLIENT_HPP__
#define __LIVERTSP_CLIENT_HPP__
#include <iostream>
#include <vector>
#include <list>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#ifdef __cplusplus
extern "C" {
#endif
//#include "zpl_type.h"
#ifdef __cplusplus
}
#endif

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE      300000//100000
// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME     0
// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP          False
#define DEFAULT_FRAME_BUFFER_SIZE           250000
//#define DEBUG_PRINT_NPT 1

class livertsp_client;
class RTSPClientSession;


// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class RTSPClientSinkSession: public MediaSink {
public:
    static RTSPClientSinkSession* createNew(UsageEnvironment &env, MediaSubsession &subsession,void *queue, int fd, char const *streamId,
                                            char const* sPropParameterSetsStr);

    virtual ~RTSPClientSinkSession();

private:
    RTSPClientSinkSession(UsageEnvironment& env, MediaSubsession& subsession, void *queue, int fd, char const *streamId,
                          char const* sPropParameterSetsStr);

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime, unsigned durationInMicroseconds);

    int rtspcPropParameterGet(char const* sPropParameterSetsStr);

private:
    void handleFrame(u_int8_t *buff, unsigned frameSize, int head);

    virtual Boolean continuePlaying();

protected:
    //int           fHNumber = 0;

    //char          *fFmtpSDPLine = NULL;
    u_int8_t* fVPS = NULL; unsigned fVPSSize = 0;
    u_int8_t* fSPS = NULL; unsigned fSPSSize = 0;
    u_int8_t* fPPS = NULL; unsigned fPPSSize = 0;

private:
    UsageEnvironment  *m_env = NULL;
    MediaSubsession   *m_fSubsession = NULL;
    u_int8_t          *m_fReceiveBuffer = NULL;
    char              *m_fStreamId = NULL;

    void  *m_Queue = NULL;
    int               m_fd = 0;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "rtspcStreamState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "rtspcStreamState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "rtspcStreamState" field to the subclass:

class RTSPClientSubMediaSession {
public:
    static RTSPClientSubMediaSession* createNew(MediaSubsession *sub);
    virtual ~RTSPClientSubMediaSession();

protected:
    RTSPClientSubMediaSession(MediaSubsession *sub);

public:
    MediaSubsession     *m_subsession = NULL;
};

class RTSPClientSession: public RTSPClient {
public:
    static RTSPClientSession* createNew(UsageEnvironment& env, livertsp_client *client, char const* rtspURL,
                                        int verbosityLevel = 0,
                                        char const* applicationName = NULL,
                                        portNumBits tunnelOverHTTPPortNum = 0);
    virtual ~RTSPClientSession();

protected:
    RTSPClientSession(UsageEnvironment& env, livertsp_client *client, char const* rtspURL,
                      int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

public:
    livertsp_client         *m_livertsp_client = NULL;
    UsageEnvironment    *m_env = NULL;
};


class livertsp_client
{
public:
    livertsp_client(const char *name="rtspClient");
    virtual ~livertsp_client();

    int livertsp_client_initalize();

    int livertsp_client_create(char const* rtspURL, int loglevel);
    int livertsp_client_create(void *queue, char const* rtspURL, int loglevel);
    int livertsp_client_create(int fd, char const *rtspURL, int loglevel);
    int livertsp_client_event_loop_running(char *stop, unsigned delay);
    int livertsp_client_event_loop_running(char * stop);
    int livertsp_client_event_loop_running();
    int livertsp_client_event_loop_stop();
    UsageEnvironment* livertsp_client_env();
    std::string livertsp_client_get_result_msg();

private:
    Boolean livertsp_client_available();

public:
    TaskScheduler       *m_scheduler = NULL;
    UsageEnvironment    *m_env = NULL;
    MediaSession        *m_session = NULL;
    RTSPClientSession   *m_rtsp = NULL;

    void    *m_Queue = NULL;
    int                 m_fd = 0;

    MediaSubsessionIterator *m_iter = NULL;
    MediaSubsession     *m_subsession = NULL;
    std::list<RTSPClientSubMediaSession*> lst_sub_session;

    Boolean             b_get_param = False;   /* Does the server support GET_PARAMETER */
    Boolean             b_paused = False;      /* Are we paused? */
    Boolean             b_error = False;
    int                 m_result_code = 0;
    char                *m_result_string = NULL;
    char                *m_sdpDescription = NULL;
    double              m_duration = 0.0;
    TaskToken           m_streamTimerTask = NULL;

private:
    unsigned            m_rtspClientCount = 0;

    Boolean             b_force_mcast = False;
    Boolean             b_multicast = False;   /* if one of the tracks is multicasted */
    Boolean             b_no_data = False;     /* if we never received any data */
    int                 i_no_data_ti = 0;  /* consecutive number of TaskInterrupt */
    Boolean             b_rtsp_tcp = False;
    char                event_rtsp = 0;
    char                event_data = 0;

    char                m_stop = 0;

    char                *m_prog_name = NULL;
};

#endif /* __LIVERTSP_CLIENT_HPP__ */

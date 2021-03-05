/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtsp_client.h
** 
** RTSP client
**
** -------------------------------------------------------------------------*/
#ifndef __RTSP_CLIENT_APP_HPP__
#define __RTSP_CLIENT_APP_HPP__

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "FramedQueue.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "ospl_type.h"
#ifdef __cplusplus
}
#endif

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000
// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False



// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class rtspcStreamState {
public:
  rtspcStreamState();
  virtual ~rtspcStreamState();

public:
  MediaSubsessionIterator* m_iter = nullptr;
  MediaSession* m_session = nullptr;
  MediaSubsession* m_subsession = nullptr;
  TaskToken m_streamTimerTask;
  double m_duration;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class rtspcDummySink: public MediaSink {
public:
  static rtspcDummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

private:
  rtspcDummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
  virtual ~rtspcDummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  void handleFrame();
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* m_fReceiveBuffer = nullptr;
  MediaSubsession& m_fSubsession;
  char* m_fStreamId = nullptr;
//private:
//  FramedQueue *m_Queue = nullptr;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "rtspcStreamState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "rtspcStreamState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "rtspcStreamState" field to the subclass:

class rtspcSession: public RTSPClient {
public:
  static rtspcSession* createNew(UsageEnvironment& env, FramedQueue *queue, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
  rtspcSession(UsageEnvironment& env, FramedQueue *queue, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~rtspcSession();

public:
  rtspcStreamState m_rtspcState;
private:
  FramedQueue *m_Queue = nullptr;
};

class rtsp_client
{
public:
    rtsp_client(char *name="rtspClient");
    virtual ~rtsp_client();

    int rtsp_client_initalize();

    int rtsp_client_create(char const* rtspURL, int loglevel);
    int rtsp_client_create(FramedQueue *queue, char const* rtspURL, int loglevel);

    int rtsp_client_event_loop_running(char * stop);
    int rtsp_client_event_loop_running();
    int rtsp_client_event_loop_stop();
    UsageEnvironment* rtsp_client_env();
    std::string rtsp_client_get_result_msg();
    unsigned m_rtspClientCount = 0;

private:
    Boolean rtsp_client_available();
    char                m_stop = 0;
    UsageEnvironment*   m_env = nullptr;
    char * m_prog_name = nullptr;
};

#endif /* __RTSP_CLIENT_APP_HPP__ */
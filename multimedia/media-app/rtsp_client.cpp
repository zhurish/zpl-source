/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtsp_client.h
** 
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>

extern "C"
{
#include <stdio.h>
#include <string.h>
#include <unistd.h>
};

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "rtsp_client.hpp"

// Forward function definitions:

// RTSP 'response handlers':
static void rtsp_client_AfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString);
static void rtsp_client_AfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString);
static void rtsp_client_AfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString);

// Other event handler functions:
static void rtsp_client_Subsession_AfterPlaying(void *clientData); // called when a stream's subsession (e.g., audio or video substream) ends
static void rtsp_client_Subsession_ByeHandler(void *clientData, char const *reason);
// called when a RTCP "BYE" is received for a subsession
static void rtsp_client_StreamTimerHandler(void *clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
static void rtsp_client_NextSubsession_Setup(RTSPClient *rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
static void rtsp_client_ShutdownStream(RTSPClient *rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const RTSPClient &rtspClient)
{
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const MediaSubsession &subsession)
{
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

// Implementation of the RTSP 'response handlers':

static void rtsp_client_AfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString)
{
  do
  {
    UsageEnvironment &env = rtspClient->envir();                                 // alias
    rtspcStreamState &m_rtspcState = ((rtspcSession *)rtspClient)->m_rtspcState; // alias

    if (resultCode != 0)
    {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char *const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n"
        << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    m_rtspcState.m_session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (m_rtspcState.m_session == NULL)
    {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    }
    else if (!m_rtspcState.m_session->hasSubsessions())
    {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    m_rtspcState.m_iter = new MediaSubsessionIterator(*m_rtspcState.m_session);
    rtsp_client_NextSubsession_Setup(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  rtsp_client_ShutdownStream(rtspClient);
}

static void rtsp_client_NextSubsession_Setup(RTSPClient *rtspClient)
{
  UsageEnvironment &env = rtspClient->envir();                                 // alias
  rtspcStreamState &m_rtspcState = ((rtspcSession *)rtspClient)->m_rtspcState; // alias

  m_rtspcState.m_subsession = m_rtspcState.m_iter->next();
  if (m_rtspcState.m_subsession != NULL)
  {
    if (!m_rtspcState.m_subsession->initiate())
    {
      env << *rtspClient << "Failed to initiate the \"" << *m_rtspcState.m_subsession << "\" subsession: " << env.getResultMsg() << "\n";
      rtsp_client_NextSubsession_Setup(rtspClient); // give up on this subsession; go to the next one
    }
    else
    {
      env << *rtspClient << "Initiated the \"" << *m_rtspcState.m_subsession << "\" subsession (";
      if (m_rtspcState.m_subsession->rtcpIsMuxed())
      {
        env << "client port " << m_rtspcState.m_subsession->clientPortNum();
      }
      else
      {
        env << "client ports " << m_rtspcState.m_subsession->clientPortNum() << "-" << m_rtspcState.m_subsession->clientPortNum() + 1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*m_rtspcState.m_subsession, rtsp_client_AfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (m_rtspcState.m_session->absStartTime() != NULL)
  {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*m_rtspcState.m_session, rtsp_client_AfterPLAY, m_rtspcState.m_session->absStartTime(), m_rtspcState.m_session->absEndTime());
  }
  else
  {
    m_rtspcState.m_duration = m_rtspcState.m_session->playEndTime() - m_rtspcState.m_session->playStartTime();
    rtspClient->sendPlayCommand(*m_rtspcState.m_session, rtsp_client_AfterPLAY);
  }
}

static void rtsp_client_AfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString)
{
  do
  {
    UsageEnvironment &env = rtspClient->envir();                                 // alias
    rtspcStreamState &m_rtspcState = ((rtspcSession *)rtspClient)->m_rtspcState; // alias

    if (resultCode != 0)
    {
      env << *rtspClient << "Failed to set up the \"" << *m_rtspcState.m_subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *m_rtspcState.m_subsession << "\" subsession (";
    if (m_rtspcState.m_subsession->rtcpIsMuxed())
    {
      env << "client port " << m_rtspcState.m_subsession->clientPortNum();
    }
    else
    {
      env << "client ports " << m_rtspcState.m_subsession->clientPortNum() << "-" << m_rtspcState.m_subsession->clientPortNum() + 1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    m_rtspcState.m_subsession->sink = rtspcDummySink::createNew(env, *m_rtspcState.m_subsession, rtspClient->url());
    // perhaps use your own custom "MediaSink" subclass instead
    if (m_rtspcState.m_subsession->sink == NULL)
    {
      env << *rtspClient << "Failed to create a data sink for the \"" << *m_rtspcState.m_subsession
          << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *m_rtspcState.m_subsession << "\" subsession\n";
    m_rtspcState.m_subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
    m_rtspcState.m_subsession->sink->startPlaying(*(m_rtspcState.m_subsession->readSource()),
                                                  rtsp_client_Subsession_AfterPlaying, m_rtspcState.m_subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (m_rtspcState.m_subsession->rtcpInstance() != NULL)
    {
      m_rtspcState.m_subsession->rtcpInstance()->setByeWithReasonHandler(rtsp_client_Subsession_ByeHandler, m_rtspcState.m_subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  rtsp_client_NextSubsession_Setup(rtspClient);
}

static void rtsp_client_AfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString)
{
  Boolean success = False;

  do
  {
    UsageEnvironment &env = rtspClient->envir();                                 // alias
    rtspcStreamState &m_rtspcState = ((rtspcSession *)rtspClient)->m_rtspcState; // alias

    if (resultCode != 0)
    {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some zpl_int16er value.)
    if (m_rtspcState.m_duration > 0)
    {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      m_rtspcState.m_duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(m_rtspcState.m_duration * 1000000);
      m_rtspcState.m_streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)rtsp_client_StreamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (m_rtspcState.m_duration > 0)
    {
      env << " (for up to " << m_rtspcState.m_duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success)
  {
    // An unrecoverable error occurred with this stream.
    rtsp_client_ShutdownStream(rtspClient);
  }
}

// Implementation of the other event handlers:

static void rtsp_client_Subsession_AfterPlaying(void *clientData)
{
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession &session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL)
  {
    if (subsession->sink != NULL)
      return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  rtsp_client_ShutdownStream(rtspClient);
}

static void rtsp_client_Subsession_ByeHandler(void *clientData, char const *reason)
{
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)subsession->miscPtr;
  UsageEnvironment &env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\"";
  if (reason != NULL)
  {
    env << " (reason:\"" << reason << "\")";
    delete[](char *) reason;
  }
  env << " on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  rtsp_client_Subsession_AfterPlaying(subsession);
}

static void rtsp_client_StreamTimerHandler(void *clientData)
{
  rtspcSession *rtspClient = (rtspcSession *)clientData;
  rtspcStreamState &m_rtspcState = rtspClient->m_rtspcState; // alias

  m_rtspcState.m_streamTimerTask = NULL;

  // Shut down the stream:
  rtsp_client_ShutdownStream(rtspClient);
}

static void rtsp_client_ShutdownStream(RTSPClient *rtspClient, int exitCode)
{
  UsageEnvironment &env = rtspClient->envir();                                 // alias
  rtspcStreamState &m_rtspcState = ((rtspcSession *)rtspClient)->m_rtspcState; // alias

  // First, check whether any subsessions have still to be closed:
  if (m_rtspcState.m_session != NULL)
  {
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*m_rtspcState.m_session);
    MediaSubsession *subsession;

    while ((subsession = iter.next()) != NULL)
    {
      if (subsession->sink != NULL)
      {
        Medium::close(subsession->sink);
        subsession->sink = NULL;

        if (subsession->rtcpInstance() != NULL)
        {
          subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
        }

        someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive)
    {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*m_rtspcState.m_session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
  // Note that this will also cause this stream's "rtspcStreamState" structure to get reclaimed.

  //if (--rtspClientCount == 0) {
  // The final stream has ended, so exit the application now.
  // (Of course, if you're embedding this code into your own application, you might want to comment this out,
  // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
  //exit(exitCode);
  //}
}

// Implementation of "rtspcSession":

rtspcSession *rtspcSession::createNew(UsageEnvironment &env, FramedQueue *queue, char const *rtspURL,
                                      int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
{
  return new rtspcSession(env, queue, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

rtspcSession::rtspcSession(UsageEnvironment &env, FramedQueue *queue, char const *rtspURL,
                           int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
  m_Queue = queue;
}

rtspcSession::~rtspcSession()
{
}

// Implementation of "rtspcStreamState":

rtspcStreamState::rtspcStreamState()
    : m_iter(NULL), m_session(NULL), m_subsession(NULL), m_streamTimerTask(NULL), m_duration(0.0)
{
}

rtspcStreamState::~rtspcStreamState()
{
  delete m_iter;
  if (m_session != NULL)
  {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment &env = m_session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(m_streamTimerTask);
    Medium::close(m_session);
  }
}

// Implementation of "rtspcDummySink":

rtspcDummySink *rtspcDummySink::createNew(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId)
{
  return new rtspcDummySink(env, subsession, streamId);
}

rtspcDummySink::rtspcDummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId)
    : MediaSink(env),
      m_fSubsession(subsession)
{
  m_fStreamId = strDup(streamId);
  m_fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

rtspcDummySink::~rtspcDummySink()
{
  delete[] m_fReceiveBuffer;
  delete[] m_fStreamId;
}

void rtspcDummySink::afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                       struct timeval presentationTime, unsigned durationInMicroseconds)
{
  rtspcDummySink *sink = (rtspcDummySink *)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void rtspcDummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                       struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
  // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (m_fStreamId != NULL)
    envir() << "Stream \"" << m_fStreamId << "\"; ";
  envir() << m_fSubsession.mediumName() << "/" << m_fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0)
    envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6 + 1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
  if (m_fSubsession.rtpSource() != NULL && !m_fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP())
  {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#ifdef DEBUG_PRINT_NPT
  envir() << "\tNPT: " << m_fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
  handleFrame();
  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean rtspcDummySink::continuePlaying()
{
  if (fSource == NULL)
    return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(m_fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

void rtspcDummySink::handleFrame()
{
  //todo one frame
  //Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 24 bytes.	Presentation time: 1607010703.757182
  /*
  m_fStreamId: "rtsp://127.0.0.1:5454/cam/";
  m_fSubsession.mediumName(): video 
  m_fSubsession.codecName(): H264 
  frameSize : 24
  */
  if (!strcmp(m_fSubsession.mediumName(), "video"))
  {
    /*
     if(firstFrame)
     {
         zpl_uint32 num;
         SPropRecord *sps = parseSPropParameterSets(m_fSubsession.fmtp_spropparametersets(), num);
         // For H.264 video stream, we use a special sink that insert start_codes:
         struct timeval tv= {0,0};
         zpl_uint8 start_code[4] = {0x00, 0x00, 0x00, 0x01};
         FILE *fp = fopen("test.264", "a+b");
         if(fp)
         {
             fwrite(start_code, 4, 1, fp);
             fwrite(sps[0].sPropBytes, sps[0].sPropLength, 1, fp);
             fwrite(start_code, 4, 1, fp);
             fwrite(sps[1].sPropBytes, sps[1].sPropLength, 1, fp);
             fclose(fp);
             fp = NULL;
         }
         delete [] sps;
         firstFrame = False;
     }
     char *pbuf = (char *)fReceiveBuffer;
     char head[4] = {0x00, 0x00, 0x00, 0x01};
     FILE *fp = fopen("test.264", "a+b");
     if(fp)
     {
         fwrite(head, 4, 1, fp);
         fwrite(fReceiveBuffer, frameSize, 1, fp);
         fclose(fp);
         fp = NULL;
     }
     */
  }
}

rtsp_client::rtsp_client(char *name)
{
  m_stop = 0;
  m_rtspClientCount = 0;
  if (m_prog_name)
    free(m_prog_name);
  if (name)
    m_prog_name = strdup(name);
}

rtsp_client::~rtsp_client()
{
  if (m_prog_name)
    free(m_prog_name);
  if (m_env != NULL)
  {
    TaskScheduler *scheduler = &(m_env->taskScheduler());
    m_env->reclaim();
    if (scheduler != NULL)
      delete scheduler;
  }
}

int rtsp_client::rtsp_client_initalize()
{
  if (m_env == NULL)
  {
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    if (scheduler != NULL)
    {
      m_env = BasicUsageEnvironment::createNew(*scheduler);
    }
  }
  if (m_env != NULL)
    return 0;
  return -1;
}

int rtsp_client::rtsp_client_create(char const *rtspURL, int loglevel)
{
  if (!rtsp_client_available())
    return -1;
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  rtspcSession *rtspClient = rtspcSession::createNew(*m_env, NULL, rtspURL, loglevel, m_prog_name);
  if (rtspClient == NULL)
  {
    *m_env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << m_env->getResultMsg() << "\n";
    return -1;
  }

  ++m_rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(rtsp_client_AfterDESCRIBE);
  return 0;
}

int rtsp_client::rtsp_client_create(FramedQueue *queue, char const *rtspURL, int loglevel)
{
  if (!rtsp_client_available())
    return -1;
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  rtspcSession *rtspClient = rtspcSession::createNew(*m_env, queue, rtspURL, loglevel, m_prog_name);
  if (rtspClient == NULL)
  {
    *m_env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << m_env->getResultMsg() << "\n";
    return -1;
  }

  ++m_rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(rtsp_client_AfterDESCRIBE);
  return 0;
}

Boolean rtsp_client::rtsp_client_available()
{
  if ((m_env != NULL))
    return True;
  std::cout << "rtsp_client_available " << std::endl;
  return False;
}

int rtsp_client::rtsp_client_event_loop_running(char *stop)
{
  if (!rtsp_client_available())
    return -1;
  m_env->taskScheduler().doEventLoop(stop);
  return 0;
}

int rtsp_client::rtsp_client_event_loop_running()
{
  if (!rtsp_client_available())
    return -1;
  m_env->taskScheduler().doEventLoop();
  return 0;
}

int rtsp_client::rtsp_client_event_loop_stop()
{
  m_stop = 1;
  return 0;
}

UsageEnvironment *rtsp_client::rtsp_client_env()
{
  return m_env;
}

std::string rtsp_client::rtsp_client_get_result_msg()
{
  std::string result("UsageEnvironment not exists");
  if (m_env)
  {
    result = m_env->getResultMsg();
  }
  return result;
}

/*
[URL:"rtsp://127.0.0.1:5454/cam/"]: Started playing session...
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 24 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 6 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 686 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 19851 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 6170 bytes.	Presentation time: 1607010703.797182
*/

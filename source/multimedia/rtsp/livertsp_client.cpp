/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** livertsp_client.h
** 
**
** -------------------------------------------------------------------------*/
#include <sstream>
#include <iostream>

extern "C"
{
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
};

#include <liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include <GroupsockHelper.hh>
#include "livertsp_client.hpp"

// Forward function definitions:

// RTSP 'response handlers':
static void livertsp_client_AfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString);
static void livertsp_client_AfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString);
static void livertsp_client_AfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString);

// Other event handler functions:
static void livertsp_client_Subsession_AfterPlaying(void *clientData); // called when a stream's subsession (e.g., audio or video substream) ends
static void livertsp_client_Subsession_ByeHandler(void *clientData, char const *reason);
// called when a RTCP "BYE" is received for a subsession
static void livertsp_client_StreamTimerHandler(void *clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
static void livertsp_client_NextSubsession_Setup(RTSPClient *rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
static void livertsp_client_ShutdownStream(RTSPClient *rtspClient, int exitCode = 1);

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


static void livertsp_client_subsession_setup(RTSPClientSession *client, MediaSession *m_ms, MediaSubsessionIterator* m_iter)
{
    int i_receive_buffer = DEFAULT_FRAME_BUFFER_SIZE;
    unsigned const thresh = 200000;
    //RTSPClientSubMediaSession
    //MediaSubsession *m_subsession = m_iter->next();
    MediaSubsession *m_subsession = client->m_livertsp_client->m_iter->next();
    if (m_subsession != NULL)
    {
        *client->m_env << "=================livertsp_client_subsession_setup :" << m_subsession->mediumName() << "/" << m_subsession->codecName() <<"\n";

        if (!m_subsession->initiate())
        {
            *client->m_env << "=================initiate :" << m_subsession->mediumName() << "/" << m_subsession->codecName() <<"\n";

            *client->m_env  << "Failed to initiate the \"" << *m_subsession << "\" subsession: " << client->m_env->getResultMsg() << "\n";
            livertsp_client_subsession_setup(client, m_ms, m_iter); // give up on this subsession; go to the next one
        }
        else
        {
            *client->m_env  << "Initiated the \"" << *m_subsession << "\" subsession (";
            if (m_subsession->rtcpIsMuxed())
            {
                *client->m_env << "client port " << m_subsession->clientPortNum();
            }
            else
            {
                *client->m_env << "client ports " << m_subsession->clientPortNum() << "-" << m_subsession->clientPortNum() + 1;
            }
            *client->m_env << ")\n";
            /* Value taken from mplayer */
            if( !strcmp( m_subsession->mediumName(), "audio" ) )
            {
                i_receive_buffer = 100000;
                *client->m_env << "=================audio :" << m_subsession->mediumName() << "/" << m_subsession->codecName() <<"\n";
            }
            else if( !strcmp( m_subsession->mediumName(), "video" ) )
            {
                i_receive_buffer = 2000000;
            }
            if( m_subsession->rtpSource() != NULL && m_subsession->readSource() != NULL)
            {
                int fd = m_subsession->rtpSource()->RTPgs()->socketNum();

                /* Increase the buffer size */
                if( i_receive_buffer > 0 )
                    increaseReceiveBufferTo(*client->m_env, fd, i_receive_buffer );

                /* Increase the RTP reorder timebuffer just a bit */
                m_subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);
                //client->m_livertsp_client->lst_sub_session.push_back(RTSPClientSubMediaSession::createNew(m_subsession));
                // Continue setting up this subsession, by sending a RTSP "SETUP" command:
                //client->sendSetupCommand(*m_subsession, livertsp_client_AfterSETUP, False, REQUEST_STREAMING_OVER_TCP);

            //m_subsession->thisPtr = m_subsession;
            client->m_livertsp_client->lst_sub_session.push_back(RTSPClientSubMediaSession::createNew(m_subsession));
            client->sendSetupCommand(*m_subsession, livertsp_client_AfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
            }
        }
        return;
    }


    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (m_ms->absStartTime() != NULL)
    {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        client->sendPlayCommand(*m_ms, livertsp_client_AfterPLAY, m_ms->absStartTime(), m_ms->absEndTime());
    }
    else
    {
        //m_rtspc->m_duration = m_ms->playEndTime() - m_ms->playStartTime();
        client->sendPlayCommand(*m_ms, livertsp_client_AfterPLAY);
    }
}

static void livertsp_client_AfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    do
    {
        UsageEnvironment &env = rtspClient->envir();                                 // alias
        livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client; // alias

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
        m_rtspc->m_session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (m_rtspc->m_session == NULL)
        {
            env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
            break;
        }
        else if (!m_rtspc->m_session->hasSubsessions())
        {
            env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }

        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        m_rtspc->m_iter = new MediaSubsessionIterator(*m_rtspc->m_session);


        //livertsp_client_subsession_setup(((RTSPClientSession *)rtspClient), m_rtspc->m_session, m_rtspc->m_iter);

        livertsp_client_NextSubsession_Setup(rtspClient);
        return;
    } while (0);

    // An unrecoverable error occurred with this stream.
    livertsp_client_ShutdownStream(rtspClient);
}

#if 1
static void livertsp_client_NextSubsession_Setup(RTSPClient *rtspClient)
{
    UsageEnvironment &env = rtspClient->envir();                                 // alias
    livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client; // alias
    int i_receive_buffer = DEFAULT_FRAME_BUFFER_SIZE;
    unsigned const thresh = 200000;
    m_rtspc->m_subsession = m_rtspc->m_iter->next();
    if (m_rtspc->m_subsession != NULL)
    {
        if (!m_rtspc->m_subsession->initiate())
        {
            env << *rtspClient << "Failed to initiate the \"" << *m_rtspc->m_subsession << "\" subsession: " << env.getResultMsg() << "\n";
            livertsp_client_NextSubsession_Setup(rtspClient); // give up on this subsession; go to the next one
        }
        else
        {
            env << *rtspClient << "Initiated the \"" << *m_rtspc->m_subsession << "\" subsession (";
            if (m_rtspc->m_subsession->rtcpIsMuxed())
            {
                env << "client port " << m_rtspc->m_subsession->clientPortNum();
            }
            else
            {
                env << "client ports " << m_rtspc->m_subsession->clientPortNum() << "-" << m_rtspc->m_subsession->clientPortNum() + 1;
            }
            env << ")\n";
            /* Value taken from mplayer */
            if( !strcmp( m_rtspc->m_subsession->mediumName(), "audio" ) )
                i_receive_buffer = 100000;
            else if( !strcmp( m_rtspc->m_subsession->mediumName(), "video" ) )
            {
                i_receive_buffer = 2000000;
            }
            if( m_rtspc->m_subsession->rtpSource() != NULL )
            {
                int fd = m_rtspc->m_subsession->rtpSource()->RTPgs()->socketNum();

                /* Increase the buffer size */
                if( i_receive_buffer > 0 )
                    increaseReceiveBufferTo(env, fd, i_receive_buffer );

                /* Increase the RTP reorder timebuffer just a bit */
                m_rtspc->m_subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);
            }
            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*m_rtspc->m_subsession, livertsp_client_AfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
        }
        return;
    }

    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (m_rtspc->m_session->absStartTime() != NULL)
    {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        rtspClient->sendPlayCommand(*m_rtspc->m_session, livertsp_client_AfterPLAY, m_rtspc->m_session->absStartTime(), m_rtspc->m_session->absEndTime());
    }
    else
    {
        m_rtspc->m_duration = m_rtspc->m_session->playEndTime() - m_rtspc->m_session->playStartTime();
        rtspClient->sendPlayCommand(*m_rtspc->m_session, livertsp_client_AfterPLAY);
    }
}
#endif

#if 0
static void livertsp_client_AfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    UsageEnvironment &env = rtspClient->envir();
    livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client;// alias

    MediaSubsessionIterator iter(*m_rtspc->m_session);
    MediaSubsession *subsession;

    while ((subsession = iter.next()) != NULL && subsession->thisPtr == subsession)
    {
        if(subsession->readSource())
        {
            env << *rtspClient << "Set up the \"" << *subsession << "\" subsession (";
            if (subsession->rtcpIsMuxed())
            {
                env << "client port " << subsession->clientPortNum();
            }
            else
            {
                env << "client ports " << subsession->clientPortNum() << "-" << subsession->clientPortNum() + 1;
            }
            env << ")\n";

            subsession->sink = RTSPClientSinkSession::createNew(env, *subsession, m_rtspc->m_Queue,
                                                                m_rtspc->m_fd, rtspClient->url(),
                                                                subsession->fmtp_spropparametersets());
            // perhaps use your own custom "MediaSink" subclass instead
            if (subsession->sink == NULL)
            {
                env << *rtspClient << "Failed to create a data sink for the \"" << *subsession
                    << "\" subsession: " << env.getResultMsg() << "\n";
                break;
            }

            env << *rtspClient << "Created a data sink for the \"" << *subsession << "\" subsession\n";
            subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
            subsession->sink->startPlaying(*(subsession->readSource()),
                                           livertsp_client_Subsession_AfterPlaying, subsession);
            // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
            if (subsession->rtcpInstance() != NULL)
            {
                subsession->rtcpInstance()->setByeWithReasonHandler(livertsp_client_Subsession_ByeHandler, subsession);
            }
        }
    }
    delete[] resultString;

    // Set up the next subsession, if any:

    livertsp_client_subsession_setup(((RTSPClientSession *)rtspClient), m_rtspc->m_session, &iter);

}
#else
static void livertsp_client_AfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    do
    {
        UsageEnvironment &env = rtspClient->envir();                                 // alias
        livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client;// alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to set up the \"" << *m_rtspc->m_subsession << "\" subsession: " << resultString << "\n";
            break;
        }

        env << *rtspClient << "Set up the \"" << *m_rtspc->m_subsession << "\" subsession (";
        if (m_rtspc->m_subsession->rtcpIsMuxed())
        {
            env << "client port " << m_rtspc->m_subsession->clientPortNum();
        }
        else
        {
            env << "client ports " << m_rtspc->m_subsession->clientPortNum() << "-" << m_rtspc->m_subsession->clientPortNum() + 1;
        }
        env << ")\n";

        // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
        // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
        // after we've sent a RTSP "PLAY" command.)

        m_rtspc->m_subsession->sink = RTSPClientSinkSession::createNew(env, *m_rtspc->m_subsession, m_rtspc->m_Queue,
                                                                       m_rtspc->m_fd, rtspClient->url(),
                                                                       m_rtspc->m_subsession->fmtp_spropparametersets());
        // perhaps use your own custom "MediaSink" subclass instead
        if (m_rtspc->m_subsession->sink == NULL)
        {
            env << *rtspClient << "Failed to create a data sink for the \"" << *m_rtspc->m_subsession
                << "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }

        env << *rtspClient << "Created a data sink for the \"" << *m_rtspc->m_subsession << "\" subsession\n";
        m_rtspc->m_subsession->miscPtr = rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
        m_rtspc->m_subsession->sink->startPlaying(*(m_rtspc->m_subsession->readSource()),
                                                  livertsp_client_Subsession_AfterPlaying, m_rtspc->m_subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (m_rtspc->m_subsession->rtcpInstance() != NULL)
        {
            m_rtspc->m_subsession->rtcpInstance()->setByeWithReasonHandler(livertsp_client_Subsession_ByeHandler, m_rtspc->m_subsession);
        }
    } while (0);
    delete[] resultString;

    // Set up the next subsession, if any:
    livertsp_client_NextSubsession_Setup(rtspClient);
}
#endif
static void livertsp_client_AfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString)
{
    Boolean success = False;

    do
    {
        UsageEnvironment &env = rtspClient->envir();                                 // alias
        livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client; // alias

        if (resultCode != 0)
        {
            env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some zpl_int16er value.)
        if (m_rtspc->m_duration > 0)
        {
            unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            m_rtspc->m_duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(m_rtspc->m_duration * 1000000);
            m_rtspc->m_streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)livertsp_client_StreamTimerHandler, rtspClient);
        }

        env << *rtspClient << "Started playing session";
        if (m_rtspc->m_duration > 0)
        {
            env << " (for up to " << m_rtspc->m_duration << " seconds)";
        }
        env << "...\n";

        success = True;
    } while (0);
    delete[] resultString;

    if (!success)
    {
        // An unrecoverable error occurred with this stream.
        livertsp_client_ShutdownStream(rtspClient);
    }
}

// Implementation of the other event handlers:

static void livertsp_client_Subsession_AfterPlaying(void *clientData)
{
    RTSPClientSubMediaSession *sub = NULL;
    MediaSubsession *subsession = (MediaSubsession *)clientData;
    RTSPClient *rtspClient = (RTSPClient *)(subsession->miscPtr);
    livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client;
    // Begin by closing this subsession's stream:
    Medium::close(subsession->sink);
    subsession->sink = NULL;

    std::list<RTSPClientSubMediaSession*>::iterator v = m_rtspc->lst_sub_session.begin();
    while( v != m_rtspc->lst_sub_session.end())
    {
        sub = *v;
        if(sub)
        {
            if(sub->m_subsession == subsession)
                break;
        }
        v++;
    }
    if(sub)
        m_rtspc->lst_sub_session.remove(sub);
    // Next, check whether *all* subsessions' streams have now been closed:
    MediaSession &session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != NULL)
    {
        if (subsession->sink != NULL)
            return; // this subsession is still active
    }

    // All subsessions' streams have now been closed, so shutdown the client:
    livertsp_client_ShutdownStream(rtspClient);
}

static void livertsp_client_Subsession_ByeHandler(void *clientData, char const *reason)
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
    livertsp_client_Subsession_AfterPlaying(subsession);
}

static void livertsp_client_StreamTimerHandler(void *clientData)
{
    RTSPClientSession *rtspClient = (RTSPClientSession *)clientData;
    //livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client; // alias

    //m_rtspc->m_streamTimerTask = NULL;

    // Shut down the stream:
    livertsp_client_ShutdownStream(rtspClient);
}

static void livertsp_client_ShutdownStream(RTSPClient *rtspClient, int exitCode)
{
#if 1
    Boolean someSubsessionsWereActive = False;
    RTSPClientSession * rtspSession = static_cast<RTSPClientSession *> (rtspClient);
    livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client;
    if(m_rtspc && m_rtspc->lst_sub_session.size())
    {
        std::list<RTSPClientSubMediaSession*>::iterator sub = m_rtspc->lst_sub_session.begin();
        while( sub != m_rtspc->lst_sub_session.end())
        {
            RTSPClientSubMediaSession *subsession = *sub;
            if (subsession != NULL && subsession->m_subsession != NULL && subsession->m_subsession->sink != NULL)
            {
                Medium::close(subsession->m_subsession->sink);
                subsession->m_subsession->sink = NULL;

                if (subsession->m_subsession->rtcpInstance() != NULL)
                {
                    subsession->m_subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                }

                someSubsessionsWereActive = True;
            }
            if(subsession != NULL)
                m_rtspc->lst_sub_session.remove(subsession);
        }
        if (someSubsessionsWereActive)
        {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
            // Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(*m_rtspc->m_session, NULL);
        }
        *rtspSession->m_env << *rtspClient << "Closing the stream.\n";
        Medium::close(rtspClient);
    }
#else
    UsageEnvironment &env = rtspClient->envir();                                 // alias
    livertsp_client *m_rtspc = ((RTSPClientSession *)rtspClient)->m_livertsp_client; // alias

    // First, check whether any subsessions have still to be closed:
    if (m_rtspc->m_session != NULL)
    {
        Boolean someSubsessionsWereActive = False;
        MediaSubsessionIterator iter(*m_rtspc->m_session);
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
            rtspClient->sendTeardownCommand(*m_rtspc->m_session, NULL);
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
#endif
}





// Implementation of "RTSPClientSinkSession":

RTSPClientSinkSession* RTSPClientSinkSession::createNew(UsageEnvironment &env, MediaSubsession &subsession,void *queue, int fd, char const *streamId,
                                                        char const* sPropParameterSetsStr) {

    RTSPClientSinkSession* result
            = new RTSPClientSinkSession(env, subsession, queue, fd, streamId, sPropParameterSetsStr);
    return result;
}

RTSPClientSinkSession::RTSPClientSinkSession(UsageEnvironment &env, MediaSubsession &subsession, void *queue, int fd, char const *streamId,
                                             char const* sPropParameterSetsStr)
    : MediaSink(env),
      m_fSubsession(&subsession)
{
    m_env = &env;
    m_fStreamId = strDup(streamId);
    m_fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    m_fd = fd;
    m_Queue = queue;

    rtspcPropParameterGet(sPropParameterSetsStr);
}

RTSPClientSinkSession::~RTSPClientSinkSession()
{
    if(m_fReceiveBuffer != NULL)
    {
        delete[] m_fReceiveBuffer;
        m_fReceiveBuffer = NULL;
    }
    if(m_fStreamId != NULL)
    {
        delete[] m_fStreamId;
        m_fStreamId = NULL;
    }
    if(fVPS != NULL)
    {
        delete[] fVPS;
        fVPS = NULL;
    }
    if(fSPS != NULL)
    {
        delete[] fSPS;
        fSPS = NULL;
    }
    if(fPPS != NULL)
    {
        delete[] fPPS;
        fPPS = NULL;
    }
}

int RTSPClientSinkSession::rtspcPropParameterGet(char const* sPropParameterSetsStr)
{
    u_int8_t* vps = NULL; unsigned vpsSize = 0;
    u_int8_t* sps = NULL; unsigned spsSize = 0;
    u_int8_t* pps = NULL; unsigned ppsSize = 0;
    if(sPropParameterSetsStr == NULL)
        return 0;
    unsigned numSPropRecords = 0;
    SPropRecord* sPropRecords = parseSPropParameterSets(sPropParameterSetsStr, numSPropRecords);
    if(sPropRecords != NULL)
    {
        for (unsigned i = 0; i < numSPropRecords; ++i) {
            if (sPropRecords[i].sPropLength == 0) continue; // bad data
            u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0])&0x1F;
            if (nal_unit_type == 7/*SPS*/) {
                sps = sPropRecords[i].sPropBytes;
                spsSize = sPropRecords[i].sPropLength;
            } else if (nal_unit_type == 8/*PPS*/) {
                pps = sPropRecords[i].sPropBytes;
                ppsSize = sPropRecords[i].sPropLength;
            }
        }

        if(fVPS != NULL)
        {
            delete[] fVPS;
            fVPSSize = 0;
            fVPS = NULL;
        }
        if (vps != NULL && vpsSize) {
            fVPS = new u_int8_t[vpsSize];
            if(fVPS != NULL)
            {
                memmove(fVPS, vps, vpsSize);
                fVPSSize = vpsSize;
            }
        }

        if(fSPS != NULL)
        {
            delete[] fSPS;
            fSPSSize = 0;
            fSPS = NULL;
        }
        if (vps != NULL && spsSize) {
            fSPS = new u_int8_t[spsSize];
            if(fSPS != NULL)
            {
                memmove(fSPS, sps, spsSize);
                fSPSSize = spsSize;
            }
        }

        if(fPPS != NULL)
        {
            delete[] fPPS;
            fPPSSize = 0;
            fPPS = NULL;
        }
        if (vps != NULL && ppsSize) {
            fPPS = new u_int8_t[ppsSize];
            if(fPPS != NULL)
            {
                memmove(fPPS, pps, ppsSize);
                fPPSSize = ppsSize;
            }
        }
        delete[] sPropRecords;
    }
    return 0;
}

void RTSPClientSinkSession::afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                              struct timeval presentationTime, unsigned durationInMicroseconds)
{
    RTSPClientSinkSession *sink = (RTSPClientSinkSession *)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void RTSPClientSinkSession::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                              struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
    // We've just received a frame of data.  (Optionally) print out information about it:
#if DEBUG_PRINT_EACH_RECEIVED_FRAME == 1
    if (m_fStreamId != NULL)
        envir() << "Stream \"" << m_fStreamId << "\"; ";
    envir() << m_fSubsession->mediumName() << "/" << m_fSubsession->codecName() << ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0)
        envir() << " (with " << numTruncatedBytes << " bytes truncated)";
    char uSecsStr[6 + 1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
    if (m_fSubsession->rtpSource() != NULL && !m_fSubsession->rtpSource()->hasBeenSynchronizedUsingRTCP())
    {
        envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << m_fSubsession->getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif
    /* No data sent. Always in sync then */

    handleFrame(m_fReceiveBuffer, frameSize,  1);
    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean RTSPClientSinkSession::continuePlaying()
{
    if (fSource == NULL)
        return False; // sanity check (should not happen)

    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(m_fReceiveBuffer+4, DUMMY_SINK_RECEIVE_BUFFER_SIZE-4,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}

void RTSPClientSinkSession::handleFrame(u_int8_t *buff, unsigned frameSize, int head)
{
    //todo one frame
    //Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 24 bytes.	Presentation time: 1607010703.757182
    /*
    m_fStreamId: "rtsp://127.0.0.1:5454/cam/";
    m_fSubsession.mediumName(): video
    m_fSubsession.codecName(): H264
    frameSize : 24
    */
    int packetlen = frameSize;
    int offset = 0;
    if (strcmp(m_fSubsession->mediumName(), "audio") == 0)
    {
        //*m_env << "========================Stream '" << m_fStreamId << "' audio\n";
        /*if(!lst_data_queue_isready(m_Queue))
        {
            lst_data_queue_setready(m_Queue, 1);
            lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_AUDIO, buff + offset, packetlen);
        }
        else
            lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_AUDIO, buff + offset, packetlen);*/
    }
    if (strcmp(m_fSubsession->mediumName(), "video") == 0)
    {
        //*m_env << "========================Stream '" << m_fStreamId << "' video\n";
        if (strcmp(m_fSubsession->codecName(), "H264") == 0)
        {

            if(head)
            {
                buff[0] = 0x00;
                buff[1] = 0x00;
                buff[2] = 0x00;
                buff[3] = 0x01;
                packetlen += 4;
                offset = 0;
            }
            else
            {
                offset = 4;
                packetlen = frameSize;
            }
            if(m_Queue)
            {
                rtspcPropParameterGet(m_fSubsession->fmtp_spropparametersets());
                // For H.264 video stream, we use a special sink that adds 'start codes',
                // and (at the start) the SPS and PPS NAL units:
                /*if(!lst_data_queue_get_fSProp(m_Queue))
                {
                    lst_data_queue_set_fSPropParam(m_Queue, fVPS, fVPSSize,
                                                   fSPS, fSPSSize,
                                                   fPPS, fPPSSize);
                }*/
                /*
            live555在传输h264流时省略了起始码，若需要存储h264码流的朋友并需要能使用vlc播放加入起始码即可。
                        起始码：0x00 0x00 0x00 0x01
                        （注意：0x01  在高地址）
            */
               /* if(!lst_data_queue_isready(m_Queue))
                {
                    lst_data_queue_setready(m_Queue, 1);
                    lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_VIDEO, buff + offset, packetlen);
                }
                else
                    lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_VIDEO, buff + offset, packetlen);*/
            }
        }
        if (strcmp(m_fSubsession->codecName(), "H265") == 0)
        {
            // For H.265 video stream, we use a special sink that adds 'start codes',
            // and (at the start) the VPS, SPS, and PPS NAL units:
            /*
            m_fSubsession.fmtp_spropvps(),
            m_fSubsession.fmtp_spropsps(),
            m_fSubsession.fmtp_sproppps(),
            */
            if(m_Queue)
            {
                /*if(!lst_data_queue_get_fSProp(m_Queue))
                {
                    lst_data_queue_set_fSPropParam(m_Queue, fVPS, fVPSSize,
                                                   fSPS, fSPSSize,
                                                   fPPS, fPPSSize);
                }
                if(!lst_data_queue_isready(m_Queue))
                {
                    lst_data_queue_setready(m_Queue, 1);
                    lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_VIDEO, buff + offset, packetlen);
                }
                else
                    lst_data_queue_enqueue(m_Queue, LST_DATA_TYPE_VIDEO, buff + offset, packetlen);*/
            }
        }
    }

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



// Implementation of "RTSPClientSubMediaSession":

RTSPClientSubMediaSession *RTSPClientSubMediaSession::createNew(MediaSubsession *sub)
{
    return new RTSPClientSubMediaSession(sub);
}

RTSPClientSubMediaSession::RTSPClientSubMediaSession(MediaSubsession *sub)
{
    m_subsession = sub;
}
RTSPClientSubMediaSession::~RTSPClientSubMediaSession()
{
}

// Implementation of "RTSPClientSession":

RTSPClientSession *RTSPClientSession::createNew(UsageEnvironment &env, livertsp_client *client, char const *rtspURL,
                                                int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
{
    return new RTSPClientSession(env, client, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

RTSPClientSession::RTSPClientSession(UsageEnvironment &env, livertsp_client *client, char const *rtspURL,
                                     int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1)
{
    m_env = &env;
    m_livertsp_client = client;
}


RTSPClientSession::~RTSPClientSession()
{
}

// Implementation of "livertsp_client":
livertsp_client::livertsp_client(const char *name)
{
    m_stop = 0;
    m_rtspClientCount = 0;
    if (m_prog_name)
        free(m_prog_name);
    if (name)
        m_prog_name = strdup(name);
}

livertsp_client::~livertsp_client()
{
    if(m_rtsp)
    {
        delete m_rtsp;
        m_rtsp = NULL;
    }
    if (m_prog_name)
        free(m_prog_name);
    if (m_env != NULL)
    {
        m_env->reclaim();
        m_env = NULL;
    }
    if (m_scheduler != NULL)
    {
        delete m_scheduler;
        m_scheduler = NULL;
    }
}

int livertsp_client::livertsp_client_initalize()
{
    if (m_env == NULL)
    {
        m_scheduler = BasicTaskScheduler::createNew();
        if (m_scheduler != NULL)
        {
            m_env = BasicUsageEnvironment::createNew(*m_scheduler);
        }
    }
    if (m_env != NULL)
        return 0;
    return -1;
}

int livertsp_client::livertsp_client_create(char const *rtspURL, int loglevel)
{
    Authenticator authenticator;
    if (!livertsp_client_available())
        return -1;
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    m_rtsp = RTSPClientSession::createNew(*m_env, this, rtspURL, loglevel, m_prog_name);
    if (m_rtsp == NULL)
    {
        *m_env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << m_env->getResultMsg() << "\n";
        return -1;
    }

    ++m_rtspClientCount;

    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    m_rtsp->sendDescribeCommand(livertsp_client_AfterDESCRIBE);
    return 0;
}

int livertsp_client::livertsp_client_create(void *queue, char const *rtspURL, int loglevel)
{
    Authenticator authenticator;
    if (!livertsp_client_available())
        return -1;
    m_Queue = queue;
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    m_rtsp = RTSPClientSession::createNew(*m_env, this, rtspURL, loglevel, m_prog_name);
    if (m_rtsp == NULL)
    {
        *m_env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << m_env->getResultMsg() << "\n";
        return -1;
    }

    ++m_rtspClientCount;
    //rtspc_vector.push_back(rtspClient);
    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    m_rtsp->sendDescribeCommand(livertsp_client_AfterDESCRIBE);
    //authenticator.setUsernameAndPassword( psz_user, psz_pwd );
    //m_rtsp->sendOptionsCommand( &livertsp_client_AfterOPTIONS, &authenticator);
    return 0;
}
#if 0
int livertsp_client::livertsp_client_create(int fd, char const *rtspURL, int loglevel)
{
    Authenticator authenticator;
    if (!livertsp_client_available())
        return -1;
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    m_rtsp = RTSPClientSession::createNew(*m_env, this, rtspURL, loglevel, m_prog_name);
    if (m_rtsp == NULL)
    {
        *m_env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << m_env->getResultMsg() << "\n";
        return -1;
    }

    ++m_rtspClientCount;
    //rtspc_vector.push_back(rtspClient);
    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    m_rtsp->sendDescribeCommand(livertsp_client_AfterDESCRIBE);
    //authenticator.setUsernameAndPassword( psz_user, psz_pwd );
    return 0;
}
#endif

Boolean livertsp_client::livertsp_client_available()
{
    if ((m_env != NULL))
        return True;
    std::cout << "livertsp_client_available " << std::endl;
    return False;
}

int livertsp_client::livertsp_client_event_loop_running(char *stop)
{
    if (!livertsp_client_available())
        return -1;
    m_env->taskScheduler().doEventLoop(stop);
    return 0;
}
int livertsp_client::livertsp_client_event_loop_running(char *stop, unsigned delay)
{
    if (!livertsp_client_available())
        return -1;
    m_env->taskScheduler().doEventLoop(stop, delay);
    return 0;
}

int livertsp_client::livertsp_client_event_loop_running()
{
    if (!livertsp_client_available())
        return -1;
    m_env->taskScheduler().doEventLoop();
    return 0;
}

int livertsp_client::livertsp_client_event_loop_stop()
{
    m_stop = 1;
    return 0;
}

UsageEnvironment *livertsp_client::livertsp_client_env()
{
    return m_env;
}

std::string livertsp_client::livertsp_client_get_result_msg()
{
    std::string result("UsageEnvironment not exists");
    if (m_env)
    {
        result = m_env->getResultMsg();
    }
    return result;
}

/*
    // 显示 vec 的原始大小
   cout << "vector size = " << vec.size() << endl;

   // 推入 5 个值到向量中
   for(i = 0; i < 5; i++){
      vec.push_back(i);
   }
   // 使用迭代器 iterator 访问值
   vector<int>::iterator v = vec.begin();
   while( v != vec.end()) {
      cout << "value of v = " << *v << endl;
      v++;
   }
    for (std::vector<pjsipBuddy *>::iterator it = buddyList.begin();
         it != buddyList.end(); ++it)
    {
        if (*it == &buddy)
        {
            buddyList.erase(it);
            break;
        }
    }
[URL:"rtsp://127.0.0.1:5454/cam/"]: Started playing session...
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 24 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 6 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 686 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 19851 bytes.	Presentation time: 1607010703.757182
Stream "rtsp://127.0.0.1:5454/cam/"; video/H264:	Received 6170 bytes.	Presentation time: 1607010703.797182
*/

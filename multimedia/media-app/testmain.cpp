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
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <liveMedia/liveMedia.hh>
#include "BasicUsageEnvironment.hh"
#include "rtsp_server.hpp"
#include "v4l2Device.hpp"

#include "BasicFormatType.hpp"

#include "liveMediaUtil/rtsp_server_wrapper.h"
#include "rtsp_client.hpp"

#ifdef ZPL_FFMPEG_MODULE
#include "ffmpegDevice.hpp"
#include "ffmpegSource.hpp"
#endif
#ifdef ZPL_OPENH264_MODULE
#include "h264Encoder.hpp"
#endif

static int sig_en = 0;
static void sigterm_handler(int sig)
{
    sig_en = 1;
}

static lst_data_queue_t *m_queue = NULL;

static void *encoder_task(void *a)
{
    int ret = 0;
    printf("start encoder_task\n");
    //return 0;

#ifdef ZPL_OPENH264_MODULE
    v4l2Device *devi = new v4l2Device(640, 480, 25, videoFormatYUY2 /*videoFormatI420*/);
#endif
#if defined(ZPL_OPENH264_MODULE) || defined(ZPL_LIBX264_MODULE)
    if (devi)
    {
        if (devi->v4l2DeviceTryOpen("/dev/video0") < 0)
        {
            delete devi;
            return 0;
        }
        devi->v4l2DeviceOpen("/dev/video0");
        devi->v4l2DeviceStart(nullptr);
        while (1)
        {
            devi->v4l2DeviceStartCapture(m_queue);
            if (sig_en == 1)
            {
                //udp_write("127.0.0.1", 9696, (char *)"ffmpeg_source->enc_pkt.data", 5);
                break;
            }
        }
        devi->v4l2DeviceStop();
    }
#endif
    pthread_exit(NULL);
}

static char rtsp_srv_running = 5;

int main(int argc, char **argv)
{
    int ret = 0;
    struct rtsp_server_t *rtsp_srv = nullptr; //new rtsp_server();
    pthread_t epid;
    signal(SIGINT, sigterm_handler);  /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */
#ifdef SIGXCPU
    signal(SIGXCPU, sigterm_handler);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN); /* Broken pipe (POSIX). */
#endif

    m_queue = lst_data_queue_create(64);
    if (!m_queue)
    {
        return 0;
    }
    //#ifdef FFMPEG_ENCODE_OUTPUT_FILE
    printf("static pthread_create\n");
    pthread_create(&epid, NULL, encoder_task, 0);
    //#endif

    if (rtsp_srv == NULL)
    {
        rtsp_srv = rtsp_server_warp_create();
    }
    if (rtsp_srv)
    {
        rtsp_server_warp_start(rtsp_srv, 5454);

        //struct rtsp_server_session_t *m_sub_session = rtsp_server_warp_sub_session_queue(rtsp_server_warp_env(rtsp_srv), m_queue, RTSP_FORMAT_TYPE_H264);
        struct rtsp_server_session_t *m_sub_session = rtsp_server_warp_sub_session_queue(rtsp_server_warp_env(rtsp_srv), m_queue, RTSP_FORMAT_TYPE_MJPG);
        
        if (m_sub_session)
        {
            rtsp_server_warp_add_session(rtsp_srv, "0/0", m_sub_session);
            rtsp_server_warp_tunneling_over_http(rtsp_srv, 8888);
            while (1)
            {
                rtsp_server_warp_event_loop_running(rtsp_srv);
                if (rtsp_srv_running == 0)
                    rtsp_server_warp_event_loop_running_delay(rtsp_srv, &rtsp_srv_running, 1000);
                else if (rtsp_srv_running == 2)
                    break;
                else
                {
                    sleep(1);
                }
            }
        }
    }

    if (epid)
        pthread_join(epid, NULL);
}

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

#include "H264UDPServerMediaSubsession.hpp"
#include "H264BasicFdServerMediaSubsession.hpp"
#include "BasicFormatType.hpp"
#include "FramedQueue.hpp"
#include "BasicQueueServerMediaSubsession.hpp"

#include "rtsp_client.hpp"

#ifdef PL_FFMPEG_MODULE
#include "ffmpegDevice.hpp"
#include "ffmpegSource.hpp"
#endif
#ifdef PL_OPENH264_MODULE
#include "h264Encoder.hpp"
#endif



static int sig_en = 0;
static void sigterm_handler(int sig)
{
    sig_en = 1;
}


FramedQueue *m_queue = nullptr;
static int udp_close();
static int udp_write(char *ipaddress, int port, char *buf, int len);
static int srv_udp_close(int );
static int srv_udp_create(int);
static pthread_t  epid = 0;


static void * encoder_task(void *a)
{
    int ret = 0;
    printf("start encoder_task\n");
    //return 0;
#ifdef PL_FFMPEG_MODULE    
    ffmpegSource *ffmpeg_source = new ffmpegSource();
    if(ffmpeg_source)
    {
        printf("start encoder_task ffmpegSourceInit\n");
        ret = ffmpeg_source->ffmpegSourceInit();
        if(ret == 0)
        {
            int getsize = 0;
            while(1)
            {
                
                if(m_queue)
                {
                    if(!m_queue->FramedQueueIsStart ())
                    {
                        sleep(1);
                        continue;
                    }
                }
                if(m_queue)
                {
                    m_queue->FramedQueueWait();
                }
                
                //printf("============================== to encode and send size:%d\n", getsize); 
                if(ffmpeg_source->doGetFrame() == 0)
                {
                    getsize = ffmpeg_source->doGetFrameDataSize();
                    if(getsize > 0)
                    {
                        //printf("==============================Succeed to encode and send size:%d\n", getsize); 
                        if(m_queue)
                        {
                            if(m_queue->FramedQueueDataIsFull())
                                m_queue->FramedQueueDataFlush();
                            m_queue->FramedQueueDataPut((unsigned char *)ffmpeg_source->enc_pkt.data, getsize);
                            //udp_write("127.0.0.1", 9696, (char *)&getsize, 4);
                        }
                        else
                        {
                            udp_write("127.0.0.1", 9696, (char *)ffmpeg_source->enc_pkt.data, getsize);
                        }
                        //ffmpeg_source->doGetFrameData(fTo, fFrameSize);
                        ffmpeg_source->doGetFrameDataFree();
                        //gettimeofday(&fPresentationTime, NULL);
                        //ffmpeg_source->doGetFrameDataFree();
                        //return 0;
                    }
                }
                if(sig_en == 1)
                    break;
            }
        }
        udp_close();
        ffmpeg_source->ffmpegSourceDestroy();
        delete ffmpeg_source;
    }
#endif
    sleep(2);
#ifdef PL_LIBX264_MODULE
    v4l2Device *devi = new v4l2Device(640, 480, 25, X264_CSP_I422); 
#endif
#ifdef PL_OPENH264_MODULE
    v4l2Device *devi = new v4l2Device(640, 480, 25, videoFormatYUY2/*videoFormatI420*/); 
#endif
#if defined(PL_OPENH264_MODULE)||defined(PL_LIBX264_MODULE)
    if(devi)
    {
        if(devi->v4l2DeviceTryOpen("/dev/video0") < 0)
        {
            delete devi;
            return 0;
        }
        devi->v4l2DeviceOpen("/dev/video0");
        devi->v4l2DeviceStart(nullptr);
        while(1)
        { 
            devi->v4l2DeviceStartCapture(m_queue);
            if(sig_en == 1)
            {
                udp_write("127.0.0.1", 9696, (char *)"ffmpeg_source->enc_pkt.data", 5);
                break;
            }
        }
        devi->v4l2DeviceStop();
    }
#endif
    udp_close();
    pthread_exit(NULL);
}



int main(int argc, char** argv)
{
    int ret = 0;
    rtsp_server *tmp = nullptr;//new rtsp_server();
    rtsp_client *ctmp = nullptr;//new rtsp_server();

    signal(SIGINT , sigterm_handler); /* Interrupt (ANSI).    */
    signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */
#ifdef SIGXCPU
    signal(SIGXCPU, sigterm_handler);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN); /* Broken pipe (POSIX). */
#endif
    ///openh264_test();
    //return 0;
    if(strstr(argv[1], "client"))
    {
        ctmp = new rtsp_client();
        if(ctmp)
        {
            ctmp->rtsp_client_initalize();
            ctmp->rtsp_client_create("rtsp://127.0.0.1:5454/cam", 1);
            ctmp->rtsp_client_event_loop_running();
            ctmp->rtsp_client_event_loop_stop();
        }
    }
    int srv_fd = srv_udp_create(9696);
    m_queue = new FramedQueue(FramedQueue::FRAMED_QUEUE_RING);
    if(m_queue)
    {
        if(m_queue->FramedQueueInit(80*2048, true) != 0)
        {
            delete m_queue;
            return 0;
        }
    }
//#ifdef FFMPEG_ENCODE_OUTPUT_FILE
    printf("static pthread_create\n"); 
    pthread_create(&epid, NULL, encoder_task, 0);
//#endif

    tmp = new rtsp_server();
    tmp->rtsp_server_initalize();
    tmp->rtsp_server_start(5454);
    //tmp->rtsp_server_add_username(const std::string & username, const std::string & password);
    tmp->rtsp_server_add_session("test", "test.264");

    //tmp->rtsp_server_add_session("cam", H264UDPServerMediaSubsession::createNew(*tmp->rtsp_server_env(), "127.0.0.1", 9696));
    //tmp->rtsp_server_add_session("cam", H264BasicFdServerMediaSubsession::createNew(*tmp->rtsp_server_env(), srv_fd));
    tmp->rtsp_server_add_session("cam", BasicQueueServerMediaSubsession::createNew(*tmp->rtsp_server_env(), 
            m_queue, BasicFormatType::BASIC_FORMAT_TYPE_H264));

    tmp->rtsp_server_tunneling_over_HTTP(8888);

    tmp->rtsp_server_event_loop_running();

    srv_udp_close(srv_fd);
    if(epid)
        pthread_join(epid, NULL);
}



static int sock_create(bool tcp)
{
	int rc = 0;//, ret;
	//struct sockaddr_in serv;
	//int flag = 1;

	/* socket creation */
	rc = socket(AF_INET, tcp ? SOCK_STREAM : SOCK_DGRAM, tcp ? IPPROTO_TCP:IPPROTO_UDP);
	if (rc < 0)
	{
		fprintf(stderr, "cannot open socket\n");
		return -1;
	}
	return rc;
}

static int sock_bind(int sock, char *ipaddress, int port)
{
	struct sockaddr_in serv;
	int ret = 0;//, flag = 1;
	/* bind local server port */
	serv.sin_family = AF_INET;
	if (ipaddress)
		serv.sin_addr.s_addr = inet_addr(ipaddress);
	else
		serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = bind(sock, (struct sockaddr *) &serv, sizeof(serv));
	if (ret < 0)
	{
		fprintf(stderr, "cannot bind port number %d(%s) \n", port,
				strerror(errno));
		return -1;;
	}
	return 0;
}



static int sock_client_write(int fd, char *ipaddress, int port, char *buf, int len)
{
	int ret = -1;
	if (buf && ipaddress)
	{
		struct sockaddr_in serv;
		/* bind local server port */
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = inet_addr(ipaddress);
		serv.sin_port = htons(port);

		ret = sendto(fd, buf, len, 0, (struct sockaddr *) &serv, sizeof(serv));
		if (ret < 0)
		{
			fprintf(stderr, "cannot sendto to %s:%d(%s) \n", ipaddress, port,
					strerror(errno));
			return -1;;
		}
		return ret;
	}
	return ret;
}

static int fd = 0;
static int fdok = 0;

static int srv_udp_close(int srv_fd)
{
    if(srv_fd)
    {
        close(srv_fd);
        srv_fd = 0;
    }
    return 0;
}
static int srv_udp_create(int port)
{
    int srv_fd = sock_create(false);
    if(sock_bind(srv_fd, NULL, port)==0)
    {
        return srv_fd;
    }
    return -1;
}
static int udp_close()
{
    if(fd && fdok)
    {
        close(fd);
        fd = fdok = 0;
    }
    return 0;
}

static int udp_write(char *ipaddress, int port, char *buf, int len)
{
    if(fd == 0)
    {
        fd = sock_create(false);
        if(sock_bind(fd, NULL, 9695)==0)
        {
            fdok = 1;
        }
    }
    if(fd && fdok)
        sock_client_write(fd, ipaddress,  port,  buf, len);
    return 0;
}



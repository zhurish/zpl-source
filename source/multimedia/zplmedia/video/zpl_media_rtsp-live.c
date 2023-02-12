/*
 * zpl_media_rtsp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"

#include "zpl_media.h"
#include "zpl_media_internal.h"

#include "liveMediaUtil/rtsp_server_wrapper.h"
//#include "liveMediaUtil/bufferQueue.h"


static struct rtsp_server_t *rtsp_srv = NULL;//new rtsp_server();
static char rtsp_srv_running = 5;
static zpl_taskid_t rtsp_srv_taskid = 0;


extern int ortp_create_init();
extern int ortp_create_exit();
extern int ortp_create_send(char *buffer, int len);

#define SHARE_BUF_SIZE (256*1024)
typedef struct userSHARE_BUF_S
{
//    int written; // 作为一个标志，非0：表示可读，0：表示可写
    int u32BufSize;
    char videobuf[SHARE_BUF_SIZE]; // 记录写入 和 读取 的文本
}shared_use_st;

static int  HisiPutH264DataToShareMem(char *pstStream, int len, shared_use_st *shared)
{
    memset(shared, 0, sizeof(shared_use_st));
    memcpy(shared->videobuf,pstStream, len);
    shared->u32BufSize = len;
     return 0;
}

//static zpl_uint32 dbg_count = 0;

static int zpl_video_frame_info(zpl_skbuffer_t *bufdata)
{
    /*
    zpl_media_debugmsg_info("  frame  ID                     =%d", bufdata->ID);
    zpl_media_debugmsg_info("  frame  timetick               =%u", bufdata->timetick);
    zpl_media_debugmsg_info("  frame  frame_seq              =%u", bufdata->frame_seq);
    zpl_media_debugmsg_info("  frame  frame_key              =%d", bufdata->frame_key);
    zpl_media_debugmsg_info("  frame  max_size               =%u", bufdata->max_size);
    zpl_media_debugmsg_info("  frame  frame_len              =%u", bufdata->frame_len);
    */
    return 0;
}
//#define TEST
extern sem_t* sem_r;
extern sem_t* sem_w;
extern char *m_pFrameBuffer;

static int zpl_media_rtsp_write_frame(zpl_media_channel_t *mediachn, zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    lst_data_queue_t *m_queue = (lst_data_queue_t *)pVoidUser;
    if(mediachn == NULL)
        return 0;
    zpl_media_video_encode_t *video_encode = (zpl_media_video_encode_t *)mediachn->halparam;
	//zpl_media_debugmsg_warn(" =====================zpl_media_rtsp_write_frame");
    if(m_queue)
    {
        /*if(sem_r == NULL)
        // 打开/创建读信号量,对读进程来说，非0时有数据可读
            sem_r = sem_open("./sem_r",O_CREAT, 0666,0);
        // 打开/创建读信号量,对读进程来说，读完之后应该设置该信号量非0，让写进程可写
	    if(sem_w == NULL)
            sem_w = sem_open("./sem_w",O_CREAT, 0666,1);*/
    #ifdef TEST
	if(sem_r == NULL)
	{
		sem_r = (sem_t*)malloc(sizeof(sem_t));
		sem_init(sem_r, 0, 0);
	}
	if(sem_w == NULL)
	{
		sem_w = (sem_t*)malloc(sizeof(sem_t));
		sem_init(sem_w, 0, 1);
	}
       sem_wait(sem_w);
       if(m_pFrameBuffer)
        HisiPutH264DataToShareMem(bufdata->frame_data, bufdata->frame_len, (shared_use_st*)m_pFrameBuffer);
       sem_post(sem_r);
       #endif
        if(bufdata->frame_key == ZPL_VIDEO_FRAME_TYPE_KEY 
        /*|| bufdata->frame_key == ZPL_VIDEO_FRAME_TYPE_P*/)
        {
            zpl_video_frame_info(bufdata);
        }
        else
        {
            if(video_encode && mediachn)
            {
                #ifdef ZPL_HISIMPP_HWDEBUG
                if(video_encode->dbg_send_count == 300)
                {
                    video_encode->dbg_send_count = 0;
                    zpl_video_frame_info(bufdata);
                    zpl_media_video_encode_request_IDR(video_encode);
                }
                video_encode->dbg_send_count++;
                #endif
            }
        }
		
        if(!lst_data_queue_isready (m_queue))
        {
            lst_data_queue_setready(m_queue, true);
            return OK;
        }
        //ortp_create_send(bufdata->frame_data, bufdata->frame_len);
        //if(rtsp_framed_queue_data_isfull(m_queue))
        //    rtsp_framed_queue_data_flush(m_queue);
        lst_data_queue_enqueue(m_queue, LST_DATA_TYPE_VIDEO, (zpl_uint8 *)bufdata->frame_data, bufdata->frame_len);
    }
    return OK;
}

static int zpl_media_rtsp_user(void *up, void *p)
{
    struct vty_user *user = (struct vty_user *)up;
    if(user && p != NULL && strlen(user->username) && user->password)
    {
        ;//rtsp_server_warp_add_username(p, user->username, user->password);
    }
    return 0;
}

/*
static int zpl_media_rtsp_logcb(char*fmt,...)
{
    va_list args;
	va_start(args, fmt);
    //fprintf(stdout, "============");
	fprintf(stdout, fmt, args);
	va_end(args);
    fflush(stdout);
    return OK;
}
*/

static int zpl_media_rtsp_task(void* argv)
{
    //int ret = 0;
	host_waitting_loadconfig();
    os_sleep(1);
    return OK;
    #ifdef TEST
    h264aamain() ;
    #endif
    if(rtsp_srv)
    {
        //rtsp_server_warp_logset(rtsp_srv, zpl_media_rtsp_logcb);
        vty_user_foreach(zpl_media_rtsp_user, rtsp_srv);
        
        //rtsp_server_warp_add_session_filename(rtsp_srv, "0/3", "/home/zhurish/workspace/planform/output.264");
        rtsp_srv_running = 0;
        rtsp_server_warp_tunneling_over_http(rtsp_srv, 8888);
        while(OS_TASK_TRUE())
        {
            //rtsp_server_warp_event_loop_running(rtsp_srv);
            if(rtsp_srv_running == 0)
                rtsp_server_warp_event_loop_running_delay(rtsp_srv, &rtsp_srv_running, 1000);
            else if(rtsp_srv_running == 2)
                break;    
            else
            {
                sleep(1);
            }    
        }
    }
    return OK;
}


static int zpl_video_encode2rtspfmt(int encode)
{
    switch(encode)
    {
	case ZPL_VIDEO_CODEC_NONE:
	case ZPL_VIDEO_CODEC_RAW:
    return RTSP_FORMAT_TYPE_AVI;
	case ZPL_VIDEO_CODEC_H263P:
    case ZPL_VIDEO_CODEC_H263I:
    return RTSP_FORMAT_TYPE_H263;
	case ZPL_VIDEO_CODEC_FLV1:
    return RTSP_FORMAT_TYPE_MP3;
    case ZPL_VIDEO_CODEC_SVQ1:
    return 0;
    case ZPL_VIDEO_CODEC_SVQ3:
    return 0;
	case ZPL_VIDEO_CODEC_H264:
    return RTSP_FORMAT_TYPE_H264;
	case ZPL_VIDEO_CODEC_H265:
    return RTSP_FORMAT_TYPE_H265;
	case ZPL_VIDEO_CODEC_MJPEG:
    return RTSP_FORMAT_TYPE_MPEG2;
	case ZPL_VIDEO_CODEC_JPEG:
    return RTSP_FORMAT_TYPE_JPEG2000;
	case ZPL_VIDEO_CODEC_MPEG4:
    return RTSP_FORMAT_TYPE_MPEG4;
	case ZPL_VIDEO_CODEC_SVAC1:
    return 0;
    default:
    return 0;
    }
    return 0;
}

int zpl_media_rtsp_url(ZPL_MEDIA_CHANNEL_E channel, 
	ZPL_MEDIA_CHANNEL_TYPE_E type, char *rtspurl)
{
    //sprintf(rtspurl, "%d/%d", channel, type);
    sprintf(rtspurl, "live");
    return OK;
}


int zpl_media_rtsp_init()
{
    #ifdef TEST
    return OK;
    #endif
    if(rtsp_srv == NULL)
    {
        rtsp_srv = rtsp_server_warp_create();
    }
    if(rtsp_srv)
    {
        //ortp_create_init();
        return rtsp_server_warp_start(rtsp_srv, 554);
    }
    return ERROR;
}

int zpl_media_rtsp_exit()
{
    if(rtsp_srv)
    {
        rtsp_srv_running = 2;
        sleep(1);
        rtsp_server_warp_destroy(rtsp_srv);
    }
    return OK;
}

/*
int zpl_media_rtsp_channel_init(ZPL_MEDIA_CHANNEL_E channel, 
	ZPL_MEDIA_CHANNEL_TYPE_E type, int rtspfmt)
{
    if(rtsp_srv)
    {
        lst_data_queue_t *m_queue = lst_data_queue_create(64);
        //lst_data_queue_t *m_queue = rtsp_framed_queue_create(1, 128, true);
        struct rtsp_server_session_t* m_sub_session = NULL;
        if(m_queue)
        {
            m_sub_session = rtsp_server_warp_sub_session_queue(rtsp_server_warp_env(rtsp_srv), m_queue, rtspfmt);
            if(m_sub_session)
            {
                char rtspurl[32];
                memset(rtspurl, 0, sizeof(rtspurl));
                zpl_media_rtsp_url(channel, type, rtspurl);
                rtsp_server_warp_add_session(rtsp_srv, rtspurl, m_sub_session);
                zpl_media_client_add(channel, type, 0, zpl_media_rtsp_write_frame, m_queue);
                zpl_media_client_set_channel(channel, type, zpl_media_channel_lookup(channel, type));
                rtsp_srv_running = 0;
            }
            else
            {
                lst_data_queue_destroy(m_queue);
                return ERROR;
            }
            return OK;
        }
    }
    return ERROR;
}
*/
lst_data_queue_t *m_queue;
int zpl_media_rtsp_channel_add(ZPL_MEDIA_CHANNEL_E channel, 
	ZPL_MEDIA_CHANNEL_TYPE_E type, zpl_int32 encode, void *mediachn)
{
    #ifdef TEST
    m_queue = lst_data_queue_create(64);
    zpl_media_channel_client_add(mediachn, zpl_media_rtsp_write_frame, m_queue);
    return OK;
    #endif
    int rtspfmt = zpl_video_encode2rtspfmt(encode);
    if(rtsp_srv && channel == 0 && type == ZPL_MEDIA_CHANNEL_TYPE_MAIN)
    {
        //lst_data_queue_t *m_queue = lst_data_queue_create(64);
        m_queue = lst_data_queue_create(64);
        //lst_data_queue_t *m_queue = rtsp_framed_queue_create(1, 128, true);
        struct rtsp_server_session_t* m_sub_session = NULL;
        if(m_queue)
        {
            m_sub_session = rtsp_server_warp_sub_session_queue(rtsp_server_warp_env(rtsp_srv), m_queue, rtspfmt);
            if(m_sub_session)
            {
                char rtspurl[32];
                memset(rtspurl, 0, sizeof(rtspurl));
                zpl_media_rtsp_url(channel, type, rtspurl);
                rtsp_server_warp_add_session(rtsp_srv, rtspurl, m_sub_session);
                zpl_media_channel_client_add(mediachn, zpl_media_rtsp_write_frame, m_queue);
                rtsp_srv_running = 0;
            }
            else
            {
                lst_data_queue_destroy(m_queue);
                return ERROR;
            }
            return OK;
        }
    }
    return ERROR;
}


int zpl_media_rtsp_task_init()
{
    if(rtsp_srv_taskid == 0)
        rtsp_srv_taskid = os_task_create("rtspTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_rtsp_task, NULL, OS_TASK_DEFAULT_STACK);
    return OK;
}

int zpl_media_rtsp_task_exit()
{
    if(rtsp_srv_taskid)
        os_task_destroy(rtsp_srv_taskid);
    return OK;
}

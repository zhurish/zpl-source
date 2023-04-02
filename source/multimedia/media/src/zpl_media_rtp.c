/*
 * zpl_media_rtp.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

#ifdef ZPL_LIBORTP_MODULE
#include <ortp/ortp.h>
#endif
#ifdef ZPL_JRTPLIB_MODULE
#include "jrtplib_api.h"
#endif
#include "rtp_payload.h"
#ifdef ZPL_LIBORTP_MODULE
extern void _rtp_session_apply_socket_sizes(RtpSession * session);
#endif
static int zpl_mediartp_ortp_create(zpl_mediartp_session_t *_rtpsession);
static int zpl_mediartp_ortp_destroy(zpl_mediartp_session_t* mysession);

#ifdef ZPL_LIBORTP_MODULE
static zpl_mediartp_scheduler_t _rtp_scheduler;
#endif
static zpl_mediartp_session_adap_t _rtp_media_adap_tbl[] =
{
    {"H264", RTP_MEDIA_PAYLOAD_H264, rtp_payload_send_h264, NULL},
    {"G711A", RTP_MEDIA_PAYLOAD_G711A, rtp_payload_send_g7xx, NULL},
    {"G711U", RTP_MEDIA_PAYLOAD_G711U, rtp_payload_send_g7xx, NULL},

    {"G722", RTP_MEDIA_PAYLOAD_G722, rtp_payload_send_g7xx, NULL},
    {"G726", RTP_MEDIA_PAYLOAD_G726, rtp_payload_send_g7xx, NULL},

    {"G728", RTP_MEDIA_PAYLOAD_G728, rtp_payload_send_g7xx, NULL},
    {"G729", RTP_MEDIA_PAYLOAD_G729, rtp_payload_send_g7xx, NULL},
    {"G729A", RTP_MEDIA_PAYLOAD_G729A, rtp_payload_send_g7xx, NULL},

    {"PCMA", RTP_MEDIA_PAYLOAD_PCMA, rtp_payload_send_g7xx, NULL},
    {"PCMU", RTP_MEDIA_PAYLOAD_PCMU, rtp_payload_send_g7xx, NULL},
};



static int zpl_mediartp_session_adap_rtp_sendto(uint32_t codec, zpl_mediartp_session_t *session, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    zpl_mediartp_session_t   *myrtp_session = session;
  
    if(myrtp_session && myrtp_session->_session)
    {
        for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
        {
            if(_rtp_media_adap_tbl[i].codec == codec)
            {
                if(_rtp_media_adap_tbl[i]._rtp_sendto)
                return (_rtp_media_adap_tbl[i]._rtp_sendto)(myrtp_session->_session, buf, len, myrtp_session->user_timestamp);
            }
        }
    }
    return -1;
}
/*
static int zpl_mediartp_session_adap_rtp_recv(uint32_t codec, zpl_mediartp_session_t *session, uint8_t *buf, uint32_t len, int *havemore)
{
    uint32_t i = 0;
    zpl_mediartp_session_t   *myrtp_session = session;

    for(i = 0; i < sizeof(_rtp_media_adap_tbl)/sizeof(_rtp_media_adap_tbl[0]); i++)
    {
        if(_rtp_media_adap_tbl[i].codec == codec && myrtp_session->_session)
        {
            if(_rtp_media_adap_tbl[i]._rtp_recv)
               return (_rtp_media_adap_tbl[i]._rtp_recv)(myrtp_session->_session, buf, len, myrtp_session->user_timestamp, havemore);
        }
    }
    return -1;
}
*/
static int zpl_mediartp_session_rtp_clone(zpl_media_channel_t *mediachn, 
                          const zpl_skbuffer_t *bufdata,  void *pVoidUser)
{
    int ret = 0;
    zpl_mediartp_session_t *session = pVoidUser;
    zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;
    int channel = -1, level = -1, mtype = -1;
    zpl_skbuffer_t *skb = NULL;
    channel = ZPL_MEDIA_CHANNEL_GET_C(media_header->ID);
    level = ZPL_MEDIA_CHANNEL_GET_I(media_header->ID);
    mtype = ZPL_MEDIA_CHANNEL_GET_T(media_header->ID);   
    if(session->rtp_media_queue)
    {
        skb = zpl_skbuffer_clone(session->rtp_media_queue, bufdata); 
        if(skb)
        {
            ret = zpl_skbqueue_add(session->rtp_media_queue, skb);
            zm_msg_debug("======== rtsp_session_media_rtp_proxy");
        }
    }
    return ret;
}

int zpl_mediartp_session_start(zpl_mediartp_session_t* my_session, zpl_bool start)
{
    int ret = 0;
    if(my_session->_session && my_session->_session)
    {
        if((my_session->mchannel != -1 && my_session->mlevel != -1))
        {
            if(start)
            {
                if(my_session->rtp_media_queue == NULL)
                    my_session->rtp_media_queue = zpl_skbqueue_create(os_name_format("rtpSkbQueue-%d/%d", my_session->mchannel, my_session->mlevel), ZPL_MEDIA_BUFQUEUE_SIZE, zpl_false); 
                if(my_session->_call_index == 0)
                    my_session->_call_index = zpl_media_channel_client_add(my_session->mchannel, my_session->mlevel, zpl_mediartp_session_rtp_clone, my_session);
                if(my_session->_call_index > 0)
                {
                    ret = zpl_media_channel_client_start(my_session->mchannel, my_session->mlevel, my_session->_call_index, zpl_true);
                    return ret;
                }
                else
                    return ERROR;
            }
            else
            {
                if(my_session->_call_index > 0)
                {
                    ret = zpl_media_channel_client_start(my_session->mchannel, my_session->mlevel, my_session->_call_index, zpl_false);
                    if(ret == OK)
                        ret = zpl_media_channel_client_del(my_session->mchannel, my_session->mlevel, my_session->_call_index);
                }
                my_session->_call_index = 0;    
                return ret;      
            }  
        }
        else if(my_session->rtp_media)
        {
            if(start)
                return zpl_media_file_reopen(my_session->rtp_media);
            else
            {
                ret = zpl_media_file_destroy(my_session->rtp_media);
                my_session->rtp_media = NULL;
                return ret;    
            }
        }
    }
    return ERROR;
}

static int zpl_mediartp_session_rtp_sendto(zpl_mediartp_session_t *rtp_session)
{
    int ret = 0;
    if(rtp_session->rtp_media_queue)
    {
        zpl_skbuffer_t * skb = zpl_skbqueue_get(rtp_session->rtp_media_queue);
        if(skb)
        {
            zpl_media_hdr_t *media_header = skb->skb_hdr.other_hdr;
            ret = zpl_mediartp_session_adap_rtp_sendto(media_header->codectype, rtp_session,
                                            ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));
            rtp_session->user_timestamp += rtp_session->timestamp_interval;
            zpl_skbqueue_finsh(rtp_session->rtp_media_queue, skb);
        }
    }
    else if(rtp_session->rtp_media)
    {
        zpl_media_bufcache_t *bufcache = &zpl_media_file_getptr(rtp_session->rtp_media)->bufcache;
        if(zpl_media_file_read(rtp_session->rtp_media, bufcache) > 0)
        {
            int type = zpl_media_file_getptr(rtp_session->rtp_media)->b_audio?ZPL_MEDIA_AUDIO:ZPL_MEDIA_VIDEO;
            int frame_codec = 0;
            if(type == ZPL_MEDIA_VIDEO)
                frame_codec = zpl_media_file_getptr(rtp_session->rtp_media)->filedesc.video.codectype;
            else
                frame_codec = zpl_media_file_getptr(rtp_session->rtp_media)->filedesc.audio.codectype;
            if(bufcache->len) 
            {       
                ret = zpl_mediartp_session_adap_rtp_sendto(frame_codec, rtp_session,
                                                bufcache->data, bufcache->len);
                rtp_session->user_timestamp += rtp_session->timestamp_interval;
            }
        }
    }
    return ret;
}


static int zpl_mediartp_session_rtp_recv(zpl_mediartp_session_t *myrtp_session)
{
    int havemore = 1 , ret = 0, tlen = 0;
    uint8_t pbuffer[MAX_RTP_PAYLOAD_LENGTH + 128];
    while (havemore && myrtp_session->_session)
    {
        #ifdef ZPL_LIBORTP_MODULE
        ret = rtp_session_recv_with_ts(myrtp_session->_session, pbuffer + 4, MAX_RTP_PAYLOAD_LENGTH, myrtp_session->user_timestamp, &havemore);
        if (havemore)
        {
            fprintf(stdout, " ======================= rtsp_session_rtp_recv: havemore=%i\n",havemore);
            fflush(stdout);
        }
        if(ret)
            tlen += ret;
        #endif    
    }
    myrtp_session->recv_user_timestamp += myrtp_session->recv_timestamp_interval;
    return OK;
}

static int media_rtp_task(void *p)
{
    int ret = 0;
	NODE node;
    #ifdef ZPL_LIBORTP_MODULE
    SessionSet r_set;
    SessionSet w_set;
    SessionSet e_set;    
    #endif
    zpl_mediartp_session_t *myrtp_session = NULL;
    zpl_mediartp_scheduler_t *_rtpscheduler = p;
	host_waitting_loadconfig();
	while(_rtpscheduler && OS_TASK_TRUE())
	{
        if(lstCount(&_rtp_scheduler.list) == 0)
        {
            os_sleep(1);
            continue;
        }
        #ifdef ZPL_LIBORTP_MODULE
        session_set_copy(&r_set, &_rtp_scheduler.r_set);
        session_set_copy(&w_set, &_rtp_scheduler.w_set);
        session_set_copy(&e_set, &_rtp_scheduler.e_set);
        
        ret = session_set_select(&r_set, &w_set, &e_set);
        #endif
        if(ret)
        {
            if(_rtp_scheduler.mutex)
                os_mutex_lock(_rtp_scheduler.mutex, OS_WAIT_FOREVER);  
            for(myrtp_session = (zpl_mediartp_session_t *)lstFirst(&_rtp_scheduler.list); myrtp_session != NULL;
                myrtp_session = (zpl_mediartp_session_t *)lstNext(&node))
            {
                node = myrtp_session->node;
                if(myrtp_session) 
                {
                    #ifdef ZPL_LIBORTP_MODULE
                    if(myrtp_session->_session->mode == RTP_SESSION_SENDRECV)
                    {
                        if (session_set_is_set(&w_set, myrtp_session->_session))
                        {
                            zpl_mediartp_session_rtp_sendto(myrtp_session);
                        }
                        if (session_set_is_set(&r_set, myrtp_session->_session))
                        {
                            zpl_mediartp_session_rtp_recv(myrtp_session);
                        }
                    }
                    if(myrtp_session->_session->mode == RTP_SESSION_SENDONLY)
                    {
                        if (session_set_is_set(&w_set, myrtp_session->_session))
                        {
                            zpl_mediartp_session_rtp_sendto(myrtp_session);
                        }
                    }
                    if(myrtp_session->_session->mode == RTP_SESSION_RECVONLY)
                    {
                        if (session_set_is_set(&r_set, myrtp_session->_session))
                        {
                            zpl_mediartp_session_rtp_recv(myrtp_session);
                        }
                    }
                    if (session_set_is_set(&e_set, myrtp_session->_session))
                    {

                    }
                    #endif
                }
            }
            if(_rtp_scheduler.mutex)
                os_mutex_unlock(_rtp_scheduler.mutex);
        }
	}
	return OK;
}

static int zpl_mediartp_sessioncb_free(zpl_mediartp_session_t *data)
{
	if(data)
	{		
        free(data);
	}	
	return OK;
}

int zpl_mediartp_session_scheduler_init(void)
{
    memset(&_rtp_scheduler, 0, sizeof(zpl_mediartp_scheduler_t));
    _rtp_scheduler.mutex = os_mutex_name_create("mrtp-mutex");
    lstInitFree (&_rtp_scheduler.list, zpl_mediartp_sessioncb_free);
    #ifdef ZPL_LIBORTP_MODULE
    ortp_init();
    ortp_set_log_level(7);
    #endif
    #ifdef ZPL_JRTPLIB_MODULE
    jrtp_session_create();
    #endif
	return OK;
}

int zpl_mediartp_session_scheduler_exit(void)
{
    if(_rtp_scheduler.mutex)
        os_mutex_lock(_rtp_scheduler.mutex, OS_WAIT_FOREVER);
    lstFree (&_rtp_scheduler.list);
    if(_rtp_scheduler.mutex)
        os_mutex_unlock(_rtp_scheduler.mutex);
    if(_rtp_scheduler.mutex)
    	os_mutex_destroy(_rtp_scheduler.mutex);
    _rtp_scheduler.mutex = NULL;
    return OK;    
}

int zpl_mediartp_session_scheduler_start(void)
{
    if(_rtp_scheduler.taskid == 0)
        _rtp_scheduler.taskid = os_task_create("mediaRtpScheduler", OS_TASK_DEFAULT_PRIORITY,
	               0, media_rtp_task, &_rtp_scheduler, OS_TASK_DEFAULT_STACK);
    ortp_scheduler_init();
    return OK;               
}

int zpl_mediartp_session_scheduler_stop(void)
{
	if(_rtp_scheduler.taskid)
	{
		if(os_task_destroy(_rtp_scheduler.taskid)==OK)
			_rtp_scheduler.taskid = 0;
	}
    return OK;
}

int zpl_mediartp_session_add(zpl_mediartp_session_t *myrtp_session)
{
    if (myrtp_session)
    {
        if(zpl_mediartp_ortp_create(myrtp_session) != OK)
        {
            return ERROR;
        }
        if (_rtp_scheduler.mutex)
            os_mutex_lock(_rtp_scheduler.mutex, OS_WAIT_FOREVER);
        lstAdd(&_rtp_scheduler.list, myrtp_session);
        #ifdef ZPL_LIBORTP_MODULE
        if ((myrtp_session->_session->flags & (1<<2)))
        {
            if (myrtp_session->_session->mode == RTP_SESSION_SENDRECV)
            {
                session_set_set(&_rtp_scheduler.w_set, myrtp_session->_session);
                session_set_set(&_rtp_scheduler.r_set, myrtp_session->_session);
            }
            if (myrtp_session->_session->mode == RTP_SESSION_SENDONLY)
            {
                session_set_set(&_rtp_scheduler.w_set, myrtp_session->_session);
            }
            if (myrtp_session->_session->mode == RTP_SESSION_RECVONLY)
            {
                session_set_set(&_rtp_scheduler.r_set, myrtp_session->_session);
            }
        }
        #endif
        if (_rtp_scheduler.mutex)
            os_mutex_unlock(_rtp_scheduler.mutex);
        return OK;
    }
    return ERROR;
}

int zpl_mediartp_session_delete(zpl_mediartp_session_t *myrtp_session)
{
    if (!myrtp_session)
        return ERROR;
    if (_rtp_scheduler.mutex)
        os_mutex_lock(_rtp_scheduler.mutex, OS_WAIT_FOREVER);
    lstDelete(&_rtp_scheduler.list, myrtp_session);
    #ifdef ZPL_LIBORTP_MODULE
    if ((myrtp_session->_session->flags & (1<<2)))
    {
        if (myrtp_session->_session->mode == RTP_SESSION_SENDRECV)
        {
            session_set_clr(&_rtp_scheduler.w_set, myrtp_session->_session);
            session_set_clr(&_rtp_scheduler.r_set, myrtp_session->_session);
        }
        if (myrtp_session->_session->mode == RTP_SESSION_SENDONLY)
        {
            session_set_clr(&_rtp_scheduler.w_set, myrtp_session->_session);
        }
        if (myrtp_session->_session->mode == RTP_SESSION_RECVONLY)
        {
            session_set_clr(&_rtp_scheduler.r_set, myrtp_session->_session);
        }
    }
    #endif
    if (_rtp_scheduler.mutex)
        os_mutex_unlock(_rtp_scheduler.mutex);
    return OK;
}

int zpl_mediartp_session_create(zpl_mediartp_session_t** mysession)
{
    zpl_mediartp_session_t* my_session = malloc(sizeof(zpl_mediartp_session_t));
    if(my_session)
    {
        memset(my_session, 0, sizeof(zpl_mediartp_session_t));
        if(mysession)
            *mysession = my_session;
        return OK;    
    }
    return ERROR;
}

int zpl_mediartp_session_destroy(zpl_mediartp_session_t* mysession)
{
    if (!mysession)
        return ERROR;
    if (_rtp_scheduler.mutex)
        os_mutex_lock(_rtp_scheduler.mutex, OS_WAIT_FOREVER);
    if(zpl_mediartp_ortp_destroy(mysession) != OK)
    {
        if (_rtp_scheduler.mutex)
            os_mutex_unlock(_rtp_scheduler.mutex);
        return ERROR;    
    }
    if(mysession->rtp_media_queue)
    {
        zpl_skbqueue_destroy(mysession->rtp_media_queue);
        mysession->rtp_media_queue = NULL;
    }
    if (_rtp_scheduler.mutex)
        os_mutex_unlock(_rtp_scheduler.mutex);
    free(mysession);
    return OK;
}

#ifdef ZPL_LIBORTP_MODULE
static void zpl_mediartp_session_timestamp_jump(RtpSession *session)
{
    //zm_msg_debug("======================rtp_session_timestamp_jump !\n");
    rtp_session_resync(session);
}
static void zpl_mediartp_session_ssrc_changed(RtpSession *session) {
	ortp_message("SSRC change detected !");
	rtp_session_resync(session);
}
#endif

static int zpl_mediartp_ortp_create(zpl_mediartp_session_t *_rtpsession)
{
    int ret = 0;
    bool bcreate = false;
    if (_rtpsession == NULL)
        return ERROR;
    if (_rtpsession->_session)
    {
        bcreate = false;
    }
    else
    {
        #ifdef ZPL_LIBORTP_MODULE
        //_rtpsession->rtpmode = RTP_SESSION_SENDRECV; // RTP_SESSION_SENDONLY;// RTP_SESSION_SENDRECV
        //_rtpsession->rtpmode = RTP_SESSION_RECVONLY;
        if(_rtpsession->b_video)
            _rtpsession->_session = rtp_session_new(RTP_SESSION_SENDRECV);
        else    
            _rtpsession->_session = rtp_session_new(RTP_SESSION_SENDRECV);
        #endif    
        bcreate = true;
    }
    #ifdef ZPL_LIBORTP_MODULE
    if (_rtpsession->_session)
    {
        if (_rtpsession->local_ssrc == 0)
            _rtpsession->local_ssrc = (uint32_t)_rtpsession->_session;
        rtp_session_set_recv_buf_size(_rtpsession->_session, 165530);
        rtp_session_set_send_buf_size(_rtpsession->_session, 65530);

        rtp_session_set_seq_number(_rtpsession->_session, 0);

        rtp_session_set_ssrc(_rtpsession->_session, _rtpsession->local_ssrc);

        rtp_session_set_payload_type(_rtpsession->_session, _rtpsession->payload);

        rtp_session_enable_rtcp(_rtpsession->_session, true);

        rtp_session_signal_connect(_rtpsession->_session, "timestamp_jump", (RtpCallback)zpl_mediartp_session_timestamp_jump, 0);
        rtp_session_signal_connect(_rtpsession->_session, "ssrc_changed", (RtpCallback)zpl_mediartp_session_ssrc_changed, 0);
        rtp_session_set_scheduling_mode(_rtpsession->_session, true);
        rtp_session_set_blocking_mode(_rtpsession->_session, false);

        rtp_session_enable_adaptive_jitter_compensation(_rtpsession->_session, true);
        rtp_session_set_symmetric_rtp(_rtpsession->_session, true);
        rtp_session_set_ssrc_changed_threshold(_rtpsession->_session, 0);
        rtp_session_set_rtcp_report_interval(_rtpsession->_session, 2500); /* At the beginning of the session send more reports. */
        rtp_session_set_multicast_loopback(_rtpsession->_session, true);   /*very useful, specially for testing purposes*/

        ret = rtp_session_set_remote_addr_and_port(_rtpsession->_session,
                                                 _rtpsession->address,
                                                 _rtpsession->rtp_port,
                                                 _rtpsession->rtcp_port);
        if(ret != 0)
            return ERROR;                                     

 
        ret = rtp_session_set_local_addr(_rtpsession->_session, _rtpsession->local_address,
                                   _rtpsession->local_rtp_port,
                                   _rtpsession->local_rtcp_port);       
        if(ret != 0)
            return ERROR; 
        if(strlen(_rtpsession->address))  
        {
            struct in_addr sin_addr;
            sin_addr.s_addr = inet_addr(_rtpsession->address); 
            if (IN_MULTICAST(ntohl(sin_addr.s_addr)))
            {
                rtp_session_enable_rtcp(_rtpsession->_session, false);
                rtp_session_set_multicast_ttl(_rtpsession->_session, 2);
                rtp_session_join_multicast_group(_rtpsession->_session, _rtpsession->address);
            }
        }
        if (_rtpsession->overtcp)
        {
            zpl_socket_t rtp_recv[2] = {0, 0};
            zpl_socket_t rtcp_recv[2] = {0, 0};
            if (ipstack_invalid(_rtpsession->rtp_sock))
            {
                ipstack_socketpair(IPSTACK_OS, AF_UNIX, SOCK_STREAM, 0, rtp_recv);
                if (!ipstack_invalid(rtp_recv[0]) && !ipstack_invalid(rtp_recv[1]))
                {
                    ipstack_set_nonblocking(rtp_recv[0]);
                    ipstack_set_nonblocking(rtp_recv[1]);
                    _rtpsession->rtp_sock = rtp_recv[0];
                }
            }
            if (ipstack_invalid(_rtpsession->rtcp_sock))
            {
                ipstack_socketpair(IPSTACK_OS, AF_UNIX, SOCK_STREAM, 0, rtcp_recv);
                if (!ipstack_invalid(rtcp_recv[0]) && !ipstack_invalid(rtcp_recv[1]))
                {
                    _rtpsession->rtcp_sock = rtcp_recv[0];
                    ipstack_set_nonblocking(rtcp_recv[0]);
                    ipstack_set_nonblocking(rtcp_recv[1]);
                }
            }
            if (!ipstack_invalid(rtp_recv[1]) && !ipstack_invalid(rtcp_recv[1]))
            {
                rtp_session_set_sockets(_rtpsession->_session,
                                        ipstack_fd(rtp_recv[1]), ipstack_fd(rtcp_recv[1]));
                rtp_session_set_overtcp(_rtpsession->_session, true,
                                        _rtpsession->rtp_interleaved,
                                        _rtpsession->rtcp_interleaved);
                _rtp_session_apply_socket_sizes(_rtpsession->_session);
            }
        }
    }
    #endif
    return ret;
}

static int zpl_mediartp_ortp_destroy(zpl_mediartp_session_t* mysession)
{
    if(mysession->_session)
    {
        #ifdef ZPL_LIBORTP_MODULE
        rtp_stats_display(&mysession->_session->stats,"statistics");
        rtp_session_destroy(mysession->_session);
        mysession->_session = NULL;
        #endif
    }
    return OK;
}

int zpl_mediartp_session_tcp_forward(zpl_mediartp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    zpl_socket_t sock = ZPL_SOCKET_INVALID;
    uint8_t  channel = buffer[1];
    if(session->rtp_interleaved == channel)
        sock = session->rtp_sock;
    else if(session->rtcp_interleaved == channel)
        sock = session->rtcp_sock;
    if(!ipstack_invalid(sock))
        return send(ipstack_fd(sock), buffer + 4, (int)len - 4, 0);
    return OK;
}


int zpl_media_channel_multicast_enable(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index, zpl_bool enable, char *address, int rtp_port, char *localaddr)
{
    zpl_media_channel_t *mediachn = NULL;
    zpl_mediartp_session_t *myrtp_session = NULL;
    mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return ERROR;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    // 通道使能录像
    if(enable == zpl_true)
    {
        if(zpl_mediartp_session_create(&myrtp_session) == ERROR)
        {
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return ERROR;
        }
        myrtp_session->rtp_sock = ZPL_SOCKET_INVALID;           //rtp socket
        myrtp_session->rtcp_sock = ZPL_SOCKET_INVALID;          //rtcp socket

        myrtp_session->b_video = zpl_true;
        myrtp_session->overtcp = 0;
        myrtp_session->payload = mediachn->media_param.video_media.codec.codectype;
        
        myrtp_session->rtp_interleaved = -1;
        myrtp_session->rtcp_interleaved = -1;

        myrtp_session->i_trackid = -1;          //视频通道
        myrtp_session->b_issetup = 0;          //视频是否设置
        //myrtp_session->address[24];
        myrtp_session->rtp_port = 65300;
        myrtp_session->rtcp_port = 65301;  // multicast only
        //myrtp_session->local_address[24];
        myrtp_session->local_rtp_port = 65200;     //本地RTP端口
        myrtp_session->local_rtcp_port = 65200;    //本地RTCP端口
        if(address)
            strcpy(myrtp_session->address, address);
        if(localaddr)    
            strcpy(myrtp_session->local_address, localaddr);
        if(rtp_port)
        {
            myrtp_session->rtp_port = rtp_port;
            myrtp_session->rtcp_port = rtp_port+1;  // multicast only 
        } 
        //myrtp_session->framerate;          //帧率
        //myrtp_session->video_height;       //视频的高度
        //myrtp_session->video_width;        //视频的宽度
        
        myrtp_session->t_master = NULL;
        myrtp_session->t_rtp_read = NULL;
        myrtp_session->t_rtcp_read = NULL;

        myrtp_session->mchannel = channel;
        myrtp_session->mlevel = channel_index;
        //myrtp_session->_call_index;       //媒体回调索引, 音视频通道数据发送

        myrtp_session->packetization_mode = 0; //封包解包模式
        myrtp_session->user_timestamp = 0;     //用户时间戳
        //myrtp_session->timestamp_interval; //用户时间戳间隔
        if(mediachn->media_param.video_media.codec.framerate == 0)
            myrtp_session->framerate = 30;
        else    
            myrtp_session->framerate = mediachn->media_param.video_media.codec.framerate;
        if(myrtp_session->framerate)
        {
            //mediachn->media_param.frame_delay_msec = RTP_MEDIA_FRAME_DELAY(1000 / mediachn->media_param.framerate);
            myrtp_session->recv_timestamp_interval = myrtp_session->timestamp_interval = rtp_profile_get_clock_rate(myrtp_session->payload) / myrtp_session->framerate;
        }

        myrtp_session->recv_user_timestamp = 0;     //用户时间戳
        //myrtp_session->recv_timestamp_interval; //用户时间戳间隔
        mediachn->p_mucast.param = myrtp_session;    
        mediachn->p_mucast.enable = enable; 

        zpl_mediartp_session_add(myrtp_session);
        zpl_mediartp_session_start(myrtp_session, zpl_true);

        ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
        return OK;
    }
    else
    {
        mediachn->p_mucast.enable = enable; 
        if(mediachn->p_mucast.param) 
        {
            zpl_mediartp_session_start(mediachn->p_mucast.param, zpl_false);
            zpl_mediartp_session_delete(mediachn->p_mucast.param);
            zpl_mediartp_session_destroy(mediachn->p_mucast.param);
            mediachn->p_mucast.param = NULL;
            ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
            return OK;
        }
    }
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ERROR;
}


zpl_bool zpl_media_channel_multicast_state(ZPL_MEDIA_CHANNEL_E channel, ZPL_MEDIA_CHANNEL_TYPE_E channel_index)
{
    zpl_bool ret = zpl_false;
    zpl_media_channel_t *mediachn = zpl_media_channel_lookup(channel, channel_index);
    if(mediachn == NULL)
        return 0;
    ZPL_MEDIA_CHANNEL_LOCK(mediachn);
    ret = mediachn->p_mucast.enable;
    ZPL_MEDIA_CHANNEL_UNLOCK(mediachn);
    return ret;
}
#ifdef ZPL_LIBORTP_MODULE
static void on_ssrc_changed(RtpSession *session) {
	ortp_message("SSRC change detected !");
	rtp_session_resync(session);
}
static int mmmmmmmmmm_task(void *ppp)
{
    RtpSession *session;
    uint32_t user_ts = 0, user_ts2 = 0;
    char buffer[320], recvbuf[320];
    SessionSet *set;
    SessionSet *rset;
    session = rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(session, 1);
    rtp_session_set_blocking_mode(session, 0);
    rtp_session_set_local_addr(session, "127.0.0.1", 65120, 65121);
    rtp_session_set_payload_type(session, 0);
    rtp_session_set_remote_addr(session, "127.0.0.1", 65320);
    rtp_session_set_ssrc(session, (int)session);
    rtp_session_enable_rtcp(session, TRUE);
    rtp_session_enable_adaptive_jitter_compensation(session, TRUE);
	rtp_session_set_symmetric_rtp(session, TRUE);
	rtp_session_set_ssrc_changed_threshold(session, 0);
	rtp_session_set_rtcp_report_interval(session, 2500); /* At the beginning of the session send more reports. */
	rtp_session_set_multicast_loopback(session, TRUE);   /*very useful, specially for testing purposes*/
	//rtp_session_set_send_ts_offset(session, (uint32_t)ortp_random());
	//rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_TMMBR, TRUE);
    //rtp_session_set_recv_buf_size(session, 256);
	rtp_session_signal_connect(session, "timestamp_jump", (RtpCallback)rtp_session_resync, NULL);
	rtp_session_signal_connect(session, "ssrc_changed", (RtpCallback)on_ssrc_changed, NULL);
    /* create a set */
    set = session_set_new();
    rset = session_set_new();
    sleep(1);
    while (OS_TASK_TRUE())
    {
        int k;
        sleep(1);
        session_set_set(set, session);
        session_set_set(rset, session);
        /* and then suspend the process by selecting() */
        k = session_set_select(rset, set, NULL);
        if (k > 0)
        {
            if (session_set_is_set(rset, session))
            {
                int err, havemore = 1;
                while (havemore)
                {
                    err = rtp_session_recv_with_ts(session, recvbuf, 160, user_ts2, &havemore);
                    if (havemore)
                        printf("warning: havemore=%i!\n", havemore);
                    if (err > 0)
                    {
                        zm_msg_debug("=========aa=============rtp_session_recv_with_ts %d bytes !\n", err);
                    }
                }
                user_ts2 += 160;
            }
          if (set && session_set_is_set(set, session))
            {
                zm_msg_debug("=========aa=============rtp_session_send_with_ts %d  !\n", user_ts);
                rtp_session_send_with_ts(session, buffer, 160, user_ts);
                user_ts += 160;
            }
        }
    }
    rtp_stats_display(&session->stats, "statistics");
    rtp_session_destroy(session);
    session_set_destroy(set);
    return 0;
}

static int mmmaasasa_task(void *ppp)
{
    RtpSession *session;
    uint32_t user_ts = 0, user_ts2 = 0;
    char buffer[320], recvbuf[320];
    SessionSet *set;
    SessionSet *rset;
    session = rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_scheduling_mode(session, 1);
    rtp_session_set_blocking_mode(session, 0);
    rtp_session_set_local_addr(session, "127.0.0.1", 65320, 65321);
    rtp_session_set_payload_type(session, 0);
    rtp_session_set_remote_addr(session, "127.0.0.1", 65120);
    rtp_session_set_ssrc(session, (int)session);
    rtp_session_enable_rtcp(session, TRUE);
    rtp_session_enable_adaptive_jitter_compensation(session, TRUE);
	rtp_session_set_symmetric_rtp(session, TRUE);
	rtp_session_set_ssrc_changed_threshold(session, 0);
	rtp_session_set_rtcp_report_interval(session, 2500); /* At the beginning of the session send more reports. */
	rtp_session_set_multicast_loopback(session, TRUE);   /*very useful, specially for testing purposes*/
	//rtp_session_set_send_ts_offset(session, (uint32_t)ortp_random());
	//rtp_session_enable_avpf_feature(session, ORTP_AVPF_FEATURE_TMMBR, TRUE);
    //rtp_session_set_recv_buf_size(session, 256);
	rtp_session_signal_connect(session, "timestamp_jump", (RtpCallback)rtp_session_resync, NULL);
	rtp_session_signal_connect(session, "ssrc_changed", (RtpCallback)on_ssrc_changed, NULL);

    /* create a set */
    set = session_set_new();
    rset = session_set_new();
    sleep(1);
    while (OS_TASK_TRUE())
    {
        int k;
        sleep(1);
        session_set_set(set, session);
        session_set_set(rset, session);
        /* and then suspend the process by selecting() */
        k = session_set_select(rset, set, NULL);
        if (k > 0)
        {
            if (rset && session_set_is_set(rset, session))
            {
                int err, havemore = 1;
                while (havemore)
                {
                    err = rtp_session_recv_with_ts(session, recvbuf, 160, user_ts2, &havemore);
                    if (havemore)
                        printf("warning: havemore=%i!\n", havemore);
                    if (err > 0)
                    {
                        zm_msg_debug("=========dd=============rtp_session_recv_with_ts %d bytes !\n", err);
                    }
                }
                user_ts2 += 160;
            }
            if (session_set_is_set(set, session))
            {
                zm_msg_debug("=========dd=============rtp_session_send_with_ts %d  !\n", user_ts);
                rtp_session_send_with_ts(session, buffer, 160, user_ts);
                user_ts += 160;
            }
        }
    }
    rtp_stats_display(&session->stats, "statistics");
    rtp_session_destroy(session);
    session_set_destroy(set);
    return 0;
}
#else
static int mmmaasasa_task(void *ppp)
{
    return 0;
}
static int mmmmmmmmmm_task(void *ppp)
{
    return 0;
}
#endif

void rtp_sched_test(void)
{
    
    os_task_create("abcd", OS_TASK_DEFAULT_PRIORITY,
	               0, mmmaasasa_task, NULL, OS_TASK_DEFAULT_STACK);

    os_task_create("bcde", OS_TASK_DEFAULT_PRIORITY,
	               0, mmmmmmmmmm_task, NULL, OS_TASK_DEFAULT_STACK);
}

//#endif /* ZPL_LIBORTP_MODULE */
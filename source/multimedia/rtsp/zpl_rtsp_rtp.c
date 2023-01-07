/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */


#include <ortp/ortp.h>

#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtsp_server.h"

#define rtsp_srv_getptr(m)             (((rtsp_srv_t*)m))


int rtsp_rtp_init(void)
{
    ortp_init();

    return 0;
}

int rtsp_rtp_start(void)
{
    ortp_scheduler_init();
    return 0;
}



int rtsp_rtp_handle_options(rtsp_session_t* session)
{
    return 0;
}

int rtsp_rtp_handle_describe(rtsp_session_t* session)
{
    return 0;
}
#define ortp_socketpair ipstack_socketpair
static int rtsp_rtp_session_srv_setup(rtsp_session_t* session, bool bcreate)
{
    rtp_session_t   *_rtpsession = session->_rtpsession;
    if(_rtpsession == NULL)
        return -1;
    if(_rtpsession->rtp_session)
    {
        if(bcreate == false)
        {
            rtp_session_add_aux_remote_addr_full(_rtpsession->rtp_session,
                                                 session->address,
                                                 _rtpsession->transport.rtp.unicast.rtp_port,
                                                 session->address,
                                                 _rtpsession->transport.rtp.unicast.rtcp_port);
            //ORTP_PUBLIC int rtp_session_del_aux_remote_addr_full(RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port);
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media set remote rtp-port %d rtcp-port %d local rtp-port %d rtcp-port %d payload %d for %d/%d or %s", 
                    _rtpsession->transport.rtp.unicast.rtp_port, _rtpsession->transport.rtp.unicast.rtcp_port,
                    _rtpsession->transport.rtp.unicast.local_rtp_port, _rtpsession->transport.rtp.unicast.local_rtcp_port,
                    _rtpsession->payload, session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
            } 
            return 0;
        }
        else
        {
            if(_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP ||
                    _rtpsession->transport.proto == RTSP_TRANSPORT_RTP_TCP)
            {
                zpl_socket_t rtp_recv[2] = {0, 0};
                zpl_socket_t rtcp_recv[2] = {0, 0};
                if(ipstack_invalid(_rtpsession->rtp_sock))
                {
                    ortp_socketpair(IPSTACK_OS, AF_INET, SOCK_STREAM, IPPROTO_TCP, rtp_recv);
                    if(!ipstack_invalid(rtp_recv[0]) && !ipstack_invalid(rtp_recv[1]))
                    {
                        _rtpsession->rtp_sock = rtp_recv[0];
                    }
                }
                if(ipstack_invalid(_rtpsession->rtcp_sock))
                {
                    ortp_socketpair(IPSTACK_OS, AF_INET, SOCK_STREAM, IPPROTO_TCP, rtcp_recv);
                    if(!ipstack_invalid(rtcp_recv[0]) && !ipstack_invalid(rtcp_recv[1]))
                    {
                        _rtpsession->rtcp_sock = rtcp_recv[0];
                    }
                }
                if(!ipstack_invalid(rtp_recv[1]) && !ipstack_invalid(rtcp_recv[1]))
                {
                    rtp_session_set_sockets(_rtpsession->rtp_session,
                                            ipstack_fd(rtp_recv[1]), ipstack_fd(rtcp_recv[1]));

                    rtp_session_set_overtcp(_rtpsession->rtp_session, true,
                                            _rtpsession->transport.rtp_interleaved,
                                            _rtpsession->transport.rtcp_interleaved);
                }
            }
            else
            {
                if(!ipstack_invalid(_rtpsession->rtp_sock) && !ipstack_invalid(_rtpsession->rtcp_sock))
                {
                    rtp_session_set_sockets(_rtpsession->rtp_session,
                                            ipstack_fd(_rtpsession->rtp_sock),
                                            ipstack_fd(_rtpsession->rtcp_sock));
                }
                else
                {
                    rtp_session_set_local_addr(_rtpsession->rtp_session, NULL,
                                               _rtpsession->transport.rtp.unicast.local_rtp_port,
                                               _rtpsession->transport.rtp.unicast.local_rtcp_port);
                    if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
                    {
                        rtsp_log_debug("RTSP Media set local rtp-port %d rtcp-port %d for %d/%d or %s", 
                            _rtpsession->transport.rtp.unicast.local_rtp_port, _rtpsession->transport.rtp.unicast.local_rtcp_port,
                            session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
                    } 
                }
            }
        }
        return 0;
    }
    return -1;
}

static int rtsp_rtp_session_client_setup(rtsp_session_t* session, bool bcreate)
{
    rtp_session_t   *_rtpsession = session->_rtpsession;
    if(_rtpsession == NULL)
        return -1;

    if(_rtpsession->rtp_session)
    {
        if(_rtpsession->transport.proto == RTSP_TRANSPORT_RTP_RTPOVERRTSP ||
                _rtpsession->transport.proto == RTSP_TRANSPORT_RTP_TCP)
        {
            zpl_socket_t rtp_recv[2] = {0, 0};
            zpl_socket_t rtcp_recv[2] = {0, 0};
            if(ipstack_invalid(_rtpsession->rtp_sock))
            {
                ortp_socketpair(IPSTACK_OS, AF_INET, SOCK_STREAM, IPPROTO_TCP, rtp_recv);
                if(!ipstack_invalid(rtp_recv[0]) && !ipstack_invalid(rtp_recv[1]))
                {
                    _rtpsession->rtp_sock = rtp_recv[0];
                }
            }
            if(ipstack_invalid(_rtpsession->rtcp_sock))
            {
                ortp_socketpair(IPSTACK_OS, AF_INET, SOCK_STREAM, IPPROTO_TCP, rtcp_recv);
                if(!ipstack_invalid(rtcp_recv[0]) && !ipstack_invalid(rtcp_recv[1]))
                {
                    _rtpsession->rtcp_sock = rtcp_recv[0];
                }
            }
            if(!ipstack_invalid(rtp_recv[1]) && !ipstack_invalid(rtcp_recv[1]))
            {
                rtp_session_set_sockets(_rtpsession->rtp_session,
                                        ipstack_fd(rtp_recv[1]), ipstack_fd(rtcp_recv[1]));

                rtp_session_set_overtcp(_rtpsession->rtp_session, true,
                                        _rtpsession->transport.rtp_interleaved,
                                        _rtpsession->transport.rtcp_interleaved);
            }
        }
        else
        {
            //rtp_session_set_connected_mode(_rtpsession->rtp_session, true);
            rtp_session_set_local_addr(_rtpsession->rtp_session, NULL,
                                       _rtpsession->transport.rtp.unicast.local_rtp_port,
                                       _rtpsession->transport.rtp.unicast.local_rtcp_port);
            if(session->parent && RTSP_DEBUG_FLAG(rtsp_srv_getptr(session->parent)->debug, EVENT))
            {
                rtsp_log_debug("RTSP Media set local rtp-port %d rtcp-port %d for %d/%d or %s", 
                    _rtpsession->transport.rtp.unicast.local_rtp_port, _rtpsession->transport.rtp.unicast.local_rtcp_port,
                    session->mchannel, session->mlevel, session->mfilepath?session->mfilepath:"nil");
            } 
        }
        return 0;
    }
    return -1;
}

static void rtp_session_timestamp_jump(RtpSession *session)
{
    rtsp_log_debug("======================rtp_session_timestamp_jump !\n");
}

int rtsp_rtp_handle_setup(rtsp_session_t* session)
{
    int ret = 0;
    bool    bcreate = false;
    rtp_session_t   *_rtpsession = session->_rtpsession;
    if(_rtpsession == NULL)
        return -1;
    if(_rtpsession->rtp_session)
    {
        bcreate = false;
    }
    else
    {
        if(session->bsrv)
            _rtpsession->rtpmode = RTP_SESSION_SENDONLY;
        else
            _rtpsession->rtpmode = RTP_SESSION_RECVONLY;
        //_rtpsession->rtp_session = rtp_session_new(srv ? RTP_SESSION_SENDONLY:RTP_SESSION_RECVONLY);
        //_rtpsession->rtp_session = rtp_session_new(RTP_SESSION_SENDRECV);
        _rtpsession->rtp_session = rtp_session_new(_rtpsession->rtpmode);
        bcreate = true;
    }
    if(_rtpsession->rtp_session)
    {
        if(_rtpsession->local_ssrc == 0)
            _rtpsession->local_ssrc = (uint32_t)_rtpsession->rtp_session;
        //if(!bcreate && _rtpsession->local_ssrc)
        //ORTP_SESSION_LOCK(_rtpsession->rtp_session);
        rtp_session_set_recv_buf_size(_rtpsession->rtp_session, 165530);
        rtp_session_set_send_buf_size(_rtpsession->rtp_session, 65530);

        rtp_session_set_seq_number(_rtpsession->rtp_session, 0);

        rtp_session_set_ssrc(_rtpsession->rtp_session, _rtpsession->local_ssrc);

        rtp_session_set_payload_type(_rtpsession->rtp_session, _rtpsession->payload);
        /*rtp_session_set_symmetric_rtp(_rtpsession->rtp_session, true);
        rtp_session_enable_adaptive_jitter_compensation(_rtpsession->rtp_session, true);
        rtp_session_set_jitter_compensation(_rtpsession->rtp_session, 40);
*/
        //if(session->audio_session.rtpmode == RTP_SESSION_SENDRECV)
            rtp_session_enable_rtcp(_rtpsession->rtp_session, true);
        //else
        //    rtp_session_enable_rtcp(_rtpsession->rtp_session, false);
        //rtp_session_signal_connect(_rtpsession->rtp_session,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
        rtp_session_set_scheduling_mode(_rtpsession->rtp_session, true);
        rtp_session_set_blocking_mode(_rtpsession->rtp_session, true);
        if(session->bsrv)
        {
            rtp_session_set_remote_addr_and_port (_rtpsession->rtp_session,
                                              session->address,
                                              _rtpsession->transport.rtp.unicast.rtp_port,
                                              _rtpsession->transport.rtp.unicast.rtcp_port);
        }
        //ORTP_SESSION_UNLOCK(_rtpsession->rtp_session);                                      
    }
    if(session->bsrv && _rtpsession->rtp_session)
    {
        //ORTP_SESSION_LOCK(_rtpsession->rtp_session);
        rtp_session_signal_connect(_rtpsession->rtp_session,"timestamp_jump",(RtpCallback)rtp_session_timestamp_jump,0);
        rtp_session_signal_connect(_rtpsession->rtp_session,"ssrc_changed",(RtpCallback)rtp_session_reset,0);
        ret = rtsp_rtp_session_srv_setup(session, bcreate);
        //ORTP_SESSION_UNLOCK(_rtpsession->rtp_session);
        return ret;
    }
    else if(_rtpsession->rtp_session)
    {
        //ORTP_SESSION_LOCK(_rtpsession->rtp_session);
        ret = rtsp_rtp_session_client_setup(session, bcreate);
        //ORTP_SESSION_UNLOCK(_rtpsession->rtp_session);
        return ret;
    }
    return -1;
}

int rtsp_rtp_handle_teardown(rtsp_session_t* session)
{
    if(session->audio_session.rtp_session)
    {
        session->audio_session.rtp_state = RTP_SESSION_STATE_CLOSE;
        //rtp_session_destroy(session->audio_session.rtp_session);
        //session->audio_session.rtp_session = NULL;
    }
    if(session->video_session.rtp_session)
    {
        session->video_session.rtp_state = RTP_SESSION_STATE_CLOSE;
        //rtp_session_destroy(session->video_session.rtp_session);
        //session->video_session.rtp_session = NULL;
    }
    return 0;
}

int rtsp_rtp_handle_play(rtsp_session_t* session)
{
    return 0;
}

int rtsp_rtp_handle_pause(rtsp_session_t* session)
{
    if(session->audio_session.rtp_session)
    {
        session->audio_session.rtp_state = RTP_SESSION_STATE_STOP;
        rtsp_media_start(session, false);
    }
    if(session->video_session.rtp_session)
    {
        session->video_session.rtp_state = RTP_SESSION_STATE_STOP;
        rtsp_media_start(session, false);
    }
    return 0;
}

int rtsp_rtp_handle_scale(rtsp_session_t* session)
{
    return 0;
}

int rtsp_rtp_handle_set_parameter(rtsp_session_t* session)
{
    return 0;
}

int rtsp_rtp_handle_get_parameter(rtsp_session_t* session)
{
    return 0;
}


int rtsp_rtp_select(rtsp_session_t* session)
{
    if(session->session_set == NULL)
    {
        session->session_set = session_set_new();
    }
    else
    {
        if(session->audio_session.rtp_session)
            session_set_set(((SessionSet*)session->session_set), (RtpSession *)session->audio_session.rtp_session);
        if(session->video_session.rtp_session)
            session_set_set(((SessionSet*)session->session_set), (RtpSession *)session->video_session.rtp_session);

        int k = session_set_select(session->session_set, NULL, NULL);
        return k;
    }
    // k=;
    // session_set_is_set(set,session[k]);
    //session_set_destroy(set);
    return -1;
}


int rtsp_rtp_tcp_forward(rtsp_session_t* session, const uint8_t *buffer, uint32_t len)
{
    zpl_socket_t sock;
    uint8_t  channel = buffer[1];
    if(session->video_session.transport.rtp_interleaved == channel)
        sock = session->video_session.rtp_sock;
    else if(session->video_session.transport.rtcp_interleaved == channel)
        sock = session->video_session.rtcp_sock;
    else if(session->audio_session.transport.rtp_interleaved == channel)
        sock = session->audio_session.rtp_sock;
    else if(session->audio_session.transport.rtcp_interleaved == channel)
        sock = session->audio_session.rtcp_sock;
    if(sock)
        return rtp_session_tcp_forward(ipstack_fd(sock), buffer + 4, (int)len - 4);
    return 0;
}


int rtsp_rtp_send(rtsp_session_t* session, bool bvideo, const uint8_t *buffer, uint32_t len, uint8_t flags)
{
    int ret = 0;
    uint8_t *pbuffer = (uint8_t *)(buffer);
    if(bvideo && session->video_session.rtp_session)
    {
        //rtsp_log_debug("rtsp_rtp_send===============video");
        //ORTP_SESSION_LOCK(session->video_session.rtp_session);
        ret = rtp_session_send_with_ts(session->video_session.rtp_session,
                                            pbuffer, len,
                                            session->video_session.user_timestamp/*, flags*/);
        //ORTP_SESSION_UNLOCK(session->video_session.rtp_session); 
        return ret;                                   
    }
    if(!bvideo && session->audio_session.rtp_session)
    {
        //ORTP_SESSION_LOCK(session->audio_session.rtp_session);
        ret = rtp_session_send_with_ts(session->audio_session.rtp_session,
                                            pbuffer, len,
                                            session->audio_session.user_timestamp/*, flags*/);
        //ORTP_SESSION_UNLOCK(session->audio_session.rtp_session);   
        return ret;                                 
    }
    return -1;
}
#if 0
/**  发送rtp数据包   
 *   主要用于发送rtp数据包   
 *   @param:  RtpSession *session RTP会话对象的指针   
 *   @param:  const char *buffer 要发送的数据的缓冲区地址   
 *   @param: int len 要发送的数据长度   
 *   @return:  int 实际发送的数据包数目   
 *   @note:     如果要发送的数据包长度大于BYTES_PER_COUNT，本函数内部会进行分包处理   
 */   
int  rtpSend(RtpSession *session, char  *buffer,  int  len)
{  
    int  sendBytes = 0; 
    int status;       
    uint32_t valid_len=len-4;
    unsigned char NALU=buffer[4];
     
    //如果数据小于MAX_RTP_PKT_LENGTH字节，直接发送：单一NAL单元模式
    if(valid_len <= MAX_RTP_PKT_LENGTH)
    {
    	// 如果需要发送的数据长度小于等于阙值，则直接发送  
        sendBytes = rtp_session_send_with_ts(session,
                                             &buffer[4],
                                             valid_len,
                                             g_userts);
    }
    else if (valid_len > MAX_RTP_PKT_LENGTH)
    {
        //切分为很多个包发送，每个包前要对头进行处理，如第一个包
        valid_len -= 1;
        int k=0,l=0;
        k=valid_len/MAX_RTP_PKT_LENGTH;//完整包的个数
        l=valid_len%MAX_RTP_PKT_LENGTH;//最后一包的大小
        int t=0;
        int pos=5;
        if(l!=0)
            k=k+1;     //完整+非完整包的包的总数 = 发送的包的数量

        while(t<k)    //发送序号<实际需要发送的包数量
        {
            if(t<(k-1))       //完整包（非尾包）
            {
-------设置FU_indicator和FU_header
                buffer[pos-2]=(NALU & 0x60)|28;
                //①NALU & 0x60→→FU_indicator的重要位NRI源自NALU_header的NRI
                //②位与28→→确定FU_indicator的类型为FU-A分片
                //③而FU_indicator的首位 禁止位则在①中被清零
                buffer[pos-1]=(NALU & 0x1f);//FU header  只要NALU的低5位
                if(0==t)
                    buffer[pos-1]|=0x80;   				首包则FU_header的首位-开始位置位

-------调用函数发送
                sendBytes = rtp_session_send_with_ts(session,      //会话
                                                     &buffer[pos-2],                   //发送内容
                                                     MAX_RTP_PKT_LENGTH+2,     //发送长度
                                                     g_userts);
                                                     
                t++;
                pos+=MAX_RTP_PKT_LENGTH;   //每次发送都用pos定位到下面要发送的内容的大致位置
            }
            else     //尾包（完整or不完整包）
            {
--------计算尾包的长度
                int iSendLen;
                if(l>0){    //完整or不完整包→→计算尾包长度
                    iSendLen=valid_len-t*MAX_RTP_PKT_LENGTH;
                }else{
                    iSendLen=MAX_RTP_PKT_LENGTH;
				}
-------设置FU_indicator和FU_header
                buffer[pos-2]=(NALU & 0x60)|28;  //FU_indicator设置NRI 并设置类型位FU-A分片
                buffer[pos-1]=(NALU & 0x1f);   //FU_header类型从NALU提取
                buffer[pos-1]|=0x40;      //FU_header的结束位置位→表示是尾包
-------调用函数发送
                sendBytes = rtp_session_send_with_ts(session,    
                                                     &buffer[pos-2],
                                                     iSendLen+2,
                                                     g_userts);
                t++;
            }
        }
    }

    g_userts += DefaultTimestampIncrement;//timestamp increase    
    //时间戳 同一帧内(不同包)时间戳相同
    return  len;
}
#endif


int rtsp_rtp_recv(rtsp_session_t* session, uint8_t *buffer, uint32_t len, bool bvideo, int *more)
{
    int ret = 0;
    if(bvideo && session->video_session.rtp_session)
    {
        //ORTP_SESSION_LOCK(session->video_session.rtp_session);
        ret = rtp_session_recv_with_ts(session->video_session.rtp_session,
                                            buffer, len,
                                            session->video_session.user_timestamp, more);
        //ORTP_SESSION_UNLOCK(session->video_session.rtp_session);
        return ret;
    }
    if(!bvideo && session->audio_session.rtp_session)
    {
        //ORTP_SESSION_LOCK(session->audio_session.rtp_session);
        ret = rtp_session_recv_with_ts(session->audio_session.rtp_session,
                                            buffer, len,
                                            session->audio_session.user_timestamp, more);
        //ORTP_SESSION_UNLOCK(session->audio_session.rtp_session);
        return ret;
    }
    return -1;
}

int rtsp_srvread(rtsp_session_t* rtsp_session)
{
    int flag = 0;
    zpl_skbuffer_t bufdata;
    memset(&bufdata, 0, sizeof(zpl_skbuffer_t));
    while(1)
    {
        printf("====================rtsp_srvread===================\r\n");
        if(rtsp_session && rtsp_rtp_select(rtsp_session))
        {
            printf("====================rtsp_srvread 2 ===================\r\n");
            if(rtsp_session->video_session.rtp_session && rtsp_session->video_session.rtp_state == RTP_SESSION_STATE_START)
            {
                flag = 1;
                if(rtsp_session->session_set &&
                        session_set_is_set(((SessionSet*)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session))
                {
                    bufdata.skb_len = 0;
                    rtsp_media_rtp_recv(rtsp_session, true, &bufdata);
                    session_set_clr(((SessionSet*)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session);
                }
            }

            if(rtsp_session->audio_session.rtp_session && rtsp_session->audio_session.rtp_state == RTP_SESSION_STATE_START)
            {
                flag = 1;
                if(rtsp_session->session_set &&
                        session_set_is_set(((SessionSet*)rtsp_session->session_set), (RtpSession *)rtsp_session->audio_session.rtp_session))
                {
                    bufdata.skb_len = 0;
                    session_set_clr(((SessionSet*)rtsp_session->session_set), (RtpSession *)rtsp_session->video_session.rtp_session);
                    rtsp_media_rtp_recv(rtsp_session, false, &bufdata);
                }
            }
            if(flag == 0)
            {
    #if defined(_WIN32)
                Sleep(10);
    #else
                usleep(10000);
    #endif
            }
        }
    }
    return -1;
}

static unsigned char ascii_tbl[] = {
    /* [字库]：[ASC8x16E字库] [数据排列]:从左到右从上到下 [取模方式]:横向8点左高位 [正负反色]:否 [去掉重复后]共95个字符
                                    [总字符库]：" !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"*/

    /*-- ID:0,字符:" ",ASCII编码:20,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:1,字符:"!",ASCII编码:21,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,

    /*-- ID:2,字符:""",ASCII编码:22,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x66,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:3,字符:"#",ASCII编码:23,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x6C,0x6C,0xFE,0x6C,0x6C,0x6C,0xFE,0x6C,0x6C,0x00,0x00,0x00,

    /*-- ID:4,字符:"$",ASCII编码:24,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x18,0x18,0x18,0x7C,0xC6,0xC2,0xC0,0x7C,0x06,0x86,0xC6,0x7C,0x18,0x18,0x00,0x00,

    /*-- ID:5,字符:"%",ASCII编码:25,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0xC2,0xC6,0x0C,0x18,0x30,0x60,0xC6,0x86,0x00,0x00,0x00,

    /*-- ID:6,字符:"&",ASCII编码:26,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x38,0x6C,0x6C,0x38,0x76,0xDC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,

    /*-- ID:7,字符:"'",ASCII编码:27,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x30,0x30,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:8,字符:"(",ASCII编码:28,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x0C,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0C,0x00,0x00,0x00,

    /*-- ID:9,字符:")",ASCII编码:29,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x30,0x18,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x18,0x30,0x00,0x00,0x00,

    /*-- ID:10,字符:"*",ASCII编码:2A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:11,字符:"+",ASCII编码:2B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:12,字符:",",ASCII编码:2C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00,

    /*-- ID:13,字符:"-",ASCII编码:2D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:14,字符:".",ASCII编码:2E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,

    /*-- ID:15,字符:"/",ASCII编码:2F,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x02,0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00,0x00,0x00,

    /*-- ID:16,字符:"0",ASCII编码:30,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0xCE,0xD6,0xD6,0xE6,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:17,字符:"1",ASCII编码:31,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x18,0x7E,0x00,0x00,0x00,

    /*-- ID:18,字符:"2",ASCII编码:32,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0x06,0x0C,0x18,0x30,0x60,0xC0,0xC6,0xFE,0x00,0x00,0x00,

    /*-- ID:19,字符:"3",ASCII编码:33,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0x06,0x06,0x3C,0x06,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:20,字符:"4",ASCII编码:34,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x0C,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,

    /*-- ID:21,字符:"5",ASCII编码:35,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFE,0xC0,0xC0,0xC0,0xFC,0x0E,0x06,0x06,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:22,字符:"6",ASCII编码:36,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x38,0x60,0xC0,0xC0,0xFC,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:23,字符:"7",ASCII编码:37,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFE,0xC6,0x06,0x06,0x0C,0x18,0x30,0x30,0x30,0x30,0x00,0x00,0x00,

    /*-- ID:24,字符:"8",ASCII编码:38,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:25,字符:"9",ASCII编码:39,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0x7E,0x06,0x06,0x06,0x0C,0x78,0x00,0x00,0x00,

    /*-- ID:26,字符:":",ASCII编码:3A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,

    /*-- ID:27,字符:";",ASCII编码:3B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,0x00,

    /*-- ID:28,字符:"<",ASCII编码:3C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,

    /*-- ID:29,字符:"=",ASCII编码:3D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:30,字符:">",ASCII编码:3E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,

    /*-- ID:31,字符:"?",ASCII编码:3F,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0x0C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,

    /*-- ID:32,字符:"@",ASCII编码:40,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xDE,0xDE,0xDE,0xDC,0xC0,0x7C,0x00,0x00,0x00,

    /*-- ID:33,字符:"A",ASCII编码:41,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,

    /*-- ID:34,字符:"B",ASCII编码:42,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x66,0xFC,0x00,0x00,0x00,

    /*-- ID:35,字符:"C",ASCII编码:43,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xC0,0xC0,0xC2,0x66,0x3C,0x00,0x00,0x00,

    /*-- ID:36,字符:"D",ASCII编码:44,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xF8,0x6C,0x66,0x66,0x66,0x66,0x66,0x66,0x6C,0xF8,0x00,0x00,0x00,

    /*-- ID:37,字符:"E",ASCII编码:45,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,

    /*-- ID:38,字符:"F",ASCII编码:46,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFE,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,

    /*-- ID:39,字符:"G",ASCII编码:47,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x3C,0x66,0xC2,0xC0,0xC0,0xDE,0xC6,0xC6,0x66,0x3A,0x00,0x00,0x00,

    /*-- ID:40,字符:"H",ASCII编码:48,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,

    /*-- ID:41,字符:"I",ASCII编码:49,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,

    /*-- ID:42,字符:"J",ASCII编码:4A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0xCC,0xCC,0x78,0x00,0x00,0x00,

    /*-- ID:43,字符:"K",ASCII编码:4B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xE6,0x66,0x6C,0x6C,0x78,0x78,0x6C,0x66,0x66,0xE6,0x00,0x00,0x00,

    /*-- ID:44,字符:"L",ASCII编码:4C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0x60,0x62,0x66,0xFE,0x00,0x00,0x00,

    /*-- ID:45,字符:"M",ASCII编码:4D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,

    /*-- ID:46,字符:"N",ASCII编码:4E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xE6,0xF6,0xFE,0xDE,0xCE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00,

    /*-- ID:47,字符:"O",ASCII编码:4F,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x38,0x6C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x00,0x00,0x00,

    /*-- ID:48,字符:"P",ASCII编码:50,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x60,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,

    /*-- ID:49,字符:"Q",ASCII编码:51,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,0xDE,0x7C,0x0C,0x0E,0x00,

    /*-- ID:50,字符:"R",ASCII编码:52,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFC,0x66,0x66,0x66,0x7C,0x6C,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,

    /*-- ID:51,字符:"S",ASCII编码:53,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7C,0xC6,0xC6,0x60,0x38,0x0C,0x06,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:52,字符:"T",ASCII编码:54,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x7E,0x7E,0x5A,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,

    /*-- ID:53,字符:"U",ASCII编码:55,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:54,字符:"V",ASCII编码:56,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00,0x00,0x00,

    /*-- ID:55,字符:"W",ASCII编码:57,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xD6,0xD6,0xFE,0x6C,0x6C,0x00,0x00,0x00,

    /*-- ID:56,字符:"X",ASCII编码:58,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xC6,0xC6,0x6C,0x6C,0x38,0x38,0x6C,0x6C,0xC6,0xC6,0x00,0x00,0x00,

    /*-- ID:57,字符:"Y",ASCII编码:59,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,

    /*-- ID:58,字符:"Z",ASCII编码:5A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xFE,0xC6,0x86,0x0C,0x18,0x30,0x60,0xC2,0xC6,0xFE,0x00,0x00,0x00,

    /*-- ID:59,字符:"[",ASCII编码:5B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,

    /*-- ID:60,字符:"\",ASCII编码:5C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x06,0x02,0x00,0x00,0x00,

    /*-- ID:61,字符:"]",ASCII编码:5D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,

    /*-- ID:62,字符:"^",ASCII编码:5E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x10,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:63,字符:"_",ASCII编码:5F,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,

    /*-- ID:64,字符:"`",ASCII编码:60,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x30,0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /*-- ID:65,字符:"a",ASCII编码:61,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,

    /*-- ID:66,字符:"b",ASCII编码:62,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xE0,0x60,0x60,0x78,0x6C,0x66,0x66,0x66,0x66,0xDC,0x00,0x00,0x00,

    /*-- ID:67,字符:"c",ASCII编码:63,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC0,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:68,字符:"d",ASCII编码:64,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x1C,0x0C,0x0C,0x3C,0x6C,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,

    /*-- ID:69,字符:"e",ASCII编码:65,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xFE,0xC0,0xC0,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:70,字符:"f",ASCII编码:66,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x38,0x6C,0x64,0x60,0xF0,0x60,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,

    /*-- ID:71,字符:"g",ASCII编码:67,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,

    /*-- ID:72,字符:"h",ASCII编码:68,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xE0,0x60,0x60,0x6C,0x76,0x66,0x66,0x66,0x66,0xE6,0x00,0x00,0x00,

    /*-- ID:73,字符:"i",ASCII编码:69,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,

    /*-- ID:74,字符:"j",ASCII编码:6A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,

    /*-- ID:75,字符:"k",ASCII编码:6B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0xE0,0x60,0x60,0x66,0x6C,0x78,0x78,0x6C,0x66,0xE6,0x00,0x00,0x00,

    /*-- ID:76,字符:"l",ASCII编码:6C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,

    /*-- ID:77,字符:"m",ASCII编码:6D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xEC,0xFE,0xD6,0xD6,0xD6,0xD6,0xD6,0x00,0x00,0x00,

    /*-- ID:78,字符:"n",ASCII编码:6E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x00,0x00,

    /*-- ID:79,字符:"o",ASCII编码:6F,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:80,字符:"p",ASCII编码:70,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xDC,0x66,0x66,0x66,0x66,0x66,0x7C,0x60,0x60,0xF0,

    /*-- ID:81,字符:"q",ASCII编码:71,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x76,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x1E,

    /*-- ID:82,字符:"r",ASCII编码:72,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xDC,0x76,0x62,0x60,0x60,0x60,0xF0,0x00,0x00,0x00,

    /*-- ID:83,字符:"s",ASCII编码:73,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0xC6,0x60,0x38,0x0C,0xC6,0x7C,0x00,0x00,0x00,

    /*-- ID:84,字符:"t",ASCII编码:74,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x10,0x30,0x30,0xFC,0x30,0x30,0x30,0x30,0x36,0x1C,0x00,0x00,0x00,

    /*-- ID:85,字符:"u",ASCII编码:75,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x76,0x00,0x00,0x00,

    /*-- ID:86,字符:"v",ASCII编码:76,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00,0x00,0x00,

    /*-- ID:87,字符:"w",ASCII编码:77,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xD6,0xD6,0xFE,0x6C,0x00,0x00,0x00,

    /*-- ID:88,字符:"x",ASCII编码:78,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0x6C,0x38,0x38,0x38,0x6C,0xC6,0x00,0x00,0x00,

    /*-- ID:89,字符:"y",ASCII编码:79,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0xC6,0xC6,0xC6,0xC6,0xC6,0x7E,0x06,0x0C,0xF8,

    /*-- ID:90,字符:"z",ASCII编码:7A,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0xCC,0x18,0x30,0x60,0xC6,0xFE,0x00,0x00,0x00,

    /*-- ID:91,字符:"{",ASCII编码:7B,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,

    /*-- ID:92,字符:"|",ASCII编码:7C,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,

    /*-- ID:93,字符:"}",ASCII编码:7D,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00,0x00,

    /*-- ID:94,字符:"~",ASCII编码:7E,对应字:宽x高=8x16,画布:宽W=8 高H=16,共16字节*/
    0x00,0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,
};



static int st_app_osd_point(unsigned short *yuv, int u32Width, int u32Height, ST_Point_T p, int color)
{
    unsigned char *pc = ((unsigned char *)yuv + u32Width*p.u32Y*3 + p.u32X*3);
    *pc = (color>>8) & 0xff;
    *(++pc) = color & 0xff;
    *(++pc) = color & 0xff;
    return 0;
}
//static unsigned char sasadsasads[] = {0x00,0x00,0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0xC6,0xC6,0x00,0x00,0x00};
static void display_font_ascii(char *asc)
{
    int i, j;
    fprintf(stdout, "=================\n");
    for(i=0;i<16;i++)
    {
        for(j=0;j<8;j++)
        {
            /* 逐位相与，为1者打印“*”，否则打印空格 */
            if(asc[i] & (0x80>>j))
                fprintf(stdout, "*");
            else
                fprintf(stdout, "$");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    fprintf(stdout, "=================\n");
    fflush(stdout);
}

static void st_app_osd_font(unsigned short *yuv, int u32Width, int u32Height,
                            int offset, int x, int y, int width, int height, int u32Color)
{
    int i = 0, j = 0;
    int bit = 0;
    uint32_t pFontdataTemp;
    ST_Point_T stPoint;

    //display_font_ascii(sasadsasads);
    display_font_ascii(ascii_tbl + offset);
    for (i = 0; i < height; i ++)
    {
        pFontdataTemp = ascii_tbl[offset+i] & 0xff;

        for (j = 0; j < width; j ++)
        {
            bit = (pFontdataTemp & (0x80 >> j));

            stPoint.u32X = x + j;
            stPoint.u32Y = y + i;

            if (bit)
            {
                st_app_osd_point(yuv,  u32Width, u32Height, stPoint, u32Color);
            }
            else
            {
            }
        }
    }
}



static void st_app_osd_text(unsigned short *yuv, int u32Width, int u32Height,
                            ST_Point_T stPoint, const char *szString, int u32Color)
{
    int i = 0;

    int num = 0, offset = 0;
    ST_Point_T pstPoint;
    if (szString == NULL)
    {
        printf("=========>%s %d\n", __func__, __LINE__);
        return;
    }
    pstPoint.u32Y = stPoint.u32Y;
    pstPoint.u32X = stPoint.u32X;
    num = strlen(szString);
    for (i = 0; i < num; i++)
    {
        if(szString[i] >= 0x20 && szString[i] <= 0x7e)
        {
            offset = szString[i] - 0x20;
            printf("=========>%s %d offset=%d\n", __func__, __LINE__, offset);
            //fontAddr = ascii_tbl + offset;
            st_app_osd_font(yuv, u32Width, u32Height, offset*16,
                            pstPoint.u32X, pstPoint.u32Y, 8, 16, u32Color);
            pstPoint.u32X += 10;
        }
    }
}

int st_app_osd_DrawText(unsigned short *yuv, int u32Width, int u32Height, ST_Point_T stPoint, const char *szString, int u32Color)
{
    st_app_osd_text(yuv,  u32Width, u32Height, stPoint, szString, u32Color);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#ifdef ZPL_LIBRTSP_MODULE

#include <ortp/port.h>
#include <ortp/logging.h>
#include <ortp/ortp_list.h>
#include <ortp/extremum.h>
#include <ortp/rtp_queue.h>
#include <ortp/rtp.h>
#include <ortp/rtcp.h>
#include <ortp/sessionset.h>
#include <ortp/payloadtype.h>
#include <ortp/rtpprofile.h>
#include <ortp/rtpsession_priv.h>
#include <ortp/rtpsession.h>
#include <ortp/telephonyevents.h>
#include <ortp/rtpsignaltable.h>
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
#include "zpl_rtp_h264.h"



#define H264_SPLIT_SIZE(n)  ((n)-4)

static int rtp_send_h264_nalu(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len);

static int rtp_h264_fuhdr(uint8_t *p, uint8_t NALU, uint8_t start, uint8_t end)
{
    H264_NALU_FUHDR *hdr = (H264_NALU_FUHDR*)(p);
    hdr->FU_F = H264_NALU_F(NALU);
    hdr->FU_NRI = H264_NALU_NRI(NALU);
    hdr->FU_Type = NAL_TYPE_FU_A;

    hdr->FU_HDR_S = start;
    hdr->FU_HDR_E = end;
    hdr->FU_HDR_R = 0;
    hdr->FU_HDR_Type = H264_NALU_TYPE(NALU);
    return 0;
}
#ifndef ZPL_LIBRTSP_MODULE
static bool is_nalu3_start(uint8_t *buffer)
{
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static bool is_nalu4_start(uint8_t *buffer)
{
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static int zpl_media_channel_get_nextnalu(uint8_t *bufdata, uint32_t len)
{
    uint32_t pos = 0;
    pos = 0;
    while (pos < len)
    {
        if (is_nalu3_start(&bufdata[pos]))
        {
            return pos;
        }
        else if (is_nalu4_start(&bufdata[pos]))
        {
            return pos;
        }
        else
        {
            pos++;
        }
    }
    return 0; // if file is end
}

static int zpl_media_channel_isnalu(uint8_t *bufdata, H264_NALU_T *nalu)
{
    if (bufdata)
    {
        if (is_nalu4_start(bufdata))
        {
            nalu->hdr_len = 4;
            nalu->len = nalu->hdr_len;
            nalu->forbidden_bit = bufdata[nalu->hdr_len] & 0x80;	 //1 bit
            nalu->nal_idc = bufdata[nalu->hdr_len] & 0x60;		 // 2 bit
            nalu->nal_unit_type = (bufdata[nalu->hdr_len]) & 0x1f; // 5 bit
            return 1;
        }
        else if (is_nalu3_start(bufdata))
        {
            nalu->hdr_len = 3;
            nalu->len = nalu->hdr_len;
            //nalu->len = len - nalu->hdr_len;
            //memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = bufdata[nalu->hdr_len] & 0x80;	 //1 bit
            nalu->nal_idc = bufdata[nalu->hdr_len] & 0x60;		 // 2 bit
            nalu->nal_unit_type = (bufdata[nalu->hdr_len]) & 0x1f; // 5 bit
            return 1;
        }
        //nalu->len++;
    }
    return 0;
}

static int zpl_media_channel_nalu_show(H264_NALU_T *nalu)
{
    if (nalu->nal_unit_type)
    {
        char type_str[20] = {0};
        switch (nalu->nal_unit_type)
        {
        case NALU_TYPE_SLICE:
            sprintf(type_str, "SLICE");
            break;
        case NALU_TYPE_DPA:
            sprintf(type_str, "DPA");
            break;
        case NALU_TYPE_DPB:
            sprintf(type_str, "DPB");
            break;
        case NALU_TYPE_DPC:
            sprintf(type_str, "DPC");
            break;
        case NALU_TYPE_IDR:
            sprintf(type_str, "IDR");
            break;
        case NALU_TYPE_SEI:
            sprintf(type_str, "SEI");
            break;
        case NALU_TYPE_SPS:

            sprintf(type_str, "SPS");
            break;
        case NALU_TYPE_PPS:

            sprintf(type_str, "PPS");
            break;
        case NALU_TYPE_AUD:
            sprintf(type_str, "AUD");
            break;
        case NALU_TYPE_EOSEQ:
            sprintf(type_str, "EOSEQ");
            break;
        case NALU_TYPE_EOSTREAM:
            sprintf(type_str, "EOSTREAM");
            break;
        case NALU_TYPE_FILL:
            sprintf(type_str, "FILL");
            break;
        }
        char idc_str[20] = {0};
        switch (nalu->nal_idc >> 5)
        {
        case NALU_PRIORITY_DISPOSABLE:
            sprintf(idc_str, "DISPOS");
            break;
        case NALU_PRIORITY_LOW:
            sprintf(idc_str, "LOW");
            break;
        case NALU_PRIORITY_HIGH:
            sprintf(idc_str, "HIGH");
            break;
        case NALU_PRIORITY_HIGHEST:
            sprintf(idc_str, "HIGHEST");
            break;
        }
        fprintf(stdout, "-------- NALU Table ------+---------+\n");
        fprintf(stdout, "       IDC |  TYPE |   LEN   |\n");
        fprintf(stdout, "---------+--------+-------+---------+\n");
        fprintf(stdout, " %7s| %6s| %8d|\n", idc_str, type_str, nalu->len-nalu->hdr_len);
        fflush(stdout);
    }
    return 0;
}
#endif


#if 0
static int _h264_file_read_data(zpl_media_file_t *file, zpl_media_bufcache_t *outpacket)
{
    int ret = 0;
    uint32_t packetsize = MAX_RTP_PAYLOAD_LENGTH, packet_offset = 0;
    uint32_t nalulen = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[MAX_RTP_PAYLOAD_LENGTH + 16];
    char tmpdata[8];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    outpacket->len = 0;
    fprintf(stdout, "============_h264_file_read_data=============\n");
    while(1)
    {
        packetsize = MAX_RTP_PAYLOAD_LENGTH - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, file->fp);
        if(ret > 0)
        {
            if (is_nalu4_start(buftmp))
            {
                naluhdr.hdr_len = 4;
                naluhdr.len = naluhdr.hdr_len;
                naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                naluhdr.buf = buftmp;
                file->flags |= NALU_START_FLAG;/* 找到开始的标志 */
                fprintf(stdout, "============_h264_file_read_data==head 4===========\n");
            }
            else if (is_nalu3_start(buftmp))
            {
                naluhdr.hdr_len = 3;
                naluhdr.len = naluhdr.hdr_len;
                naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                naluhdr.buf = buftmp;
                file->flags |= NALU_START_FLAG;/* 找到开始的标志 */
                fprintf(stdout, "============_h264_file_read_data==head 3===========\n");
            }
            if(file->flags & NALU_START_FLAG)
            {
                p = buftmp + naluhdr.len;
                nalulen = zpl_media_channel_get_nextnalu(p, H264_SPLIT_SIZE(ret));
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    zpl_media_bufcache_add(outpacket, buftmp, naluhdr.len);

                    file->offset_len += (naluhdr.len);
                    fseek(file->fp, file->offset_len, SEEK_SET);

                    file->flags = 0;
                    zpl_media_channel_nalu_show(&naluhdr);
                    return outpacket->len;
                }
                else
                {
                    if(ret > 8)
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret - 8);
                        memcpy(tmpdata, buftmp+ret-8, 8);
                        memmove(buftmp, tmpdata, 8);
                        packet_offset = 8;
                    }
                    else
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret);
                        naluhdr.len += ret;
                        file->offset_len = 0;
                        file->flags = 0;
                        fprintf(stdout, "============_h264_file_read_data==end===========\n");
                        zpl_media_channel_nalu_show(&naluhdr);
                        return outpacket->len;
                    }
                }
            }
        }
        else
            break;
    }
    file->offset_len = 0;
    return -1;
}
#endif
/*
 * 多个nalu组成一个包
 */
int _rtp_packet_h264(zpl_skbuffer_t *nalu, uint8_t *out)
{
    uint16_t packetlen = 0;
    uint16_t *nalulen = NULL;
    if(nalu->skb_data && nalu->skb_len)
    {
        nalulen = (uint16_t *)out;
        if (is_nalu4_start(nalu->skb_data))
        {
            packetlen = nalu->skb_len + 2 - 4;
            *nalulen = htons(nalu->skb_len-4);
            memcpy(out, nalu->skb_data + 4, nalu->skb_len - 4);
            nalu->skb_len = 0;
            return packetlen;
        }
        else if (is_nalu3_start(nalu->skb_data))
        {
            packetlen = nalu->skb_len + 2 - 3;
            *nalulen = htons(nalu->skb_len-3);
            memcpy(out, nalu->skb_data + 3, nalu->skb_len - 3);
            nalu->skb_len = 0;
            return packetlen;
        }
        nalu->skb_len = 0;
        return -1;
    }
    return -1;
}




static int rtp_send_h264_nalu(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len)
{
    int ret = 0;
    uint8_t *p = (uint8_t *)buffer;
    uint8_t payload[MAX_RTP_PAYLOAD_LENGTH +16];
    uint32_t plen = len;
    uint8_t NALU = buffer[0];
    H264_NALU_T nalu;
    memset(&nalu, 0, sizeof(H264_NALU_T));
    if(zpl_media_channel_isnaluhdr(p, &nalu))
    {
        nalu.len = len;
        p = (uint8_t *)(buffer + nalu.hdr_len);
        plen = len - nalu.hdr_len;
        NALU = nalu.buf[nalu.hdr_len];
    }
    if(len > MAX_RTP_PAYLOAD_LENGTH)
    {
        int i = 0;
        while(plen)
        {
            if(plen > MAX_RTP_PAYLOAD_LENGTH)
            {
                memcpy(payload + HEADER_SIZE_FU_A, p, MAX_RTP_PAYLOAD_LENGTH);
                rtp_h264_fuhdr(payload, NALU, (i==0)?1:0, 0);

                ret = rtsp_rtp_send(session, true, payload, MAX_RTP_PAYLOAD_LENGTH + HEADER_SIZE_FU_A, 0);
                //ret = rtp_session_send_with_ts(session->video_session.rtp_session,
                //                               payload, MAX_RTP_PAYLOAD_LENGTH + HEADER_SIZE_FU_A,
                //                               session->video_session.user_timestamp, 0);
                p +=  MAX_RTP_PAYLOAD_LENGTH;
                plen -= MAX_RTP_PAYLOAD_LENGTH;
            }
            else
            {
                memcpy(payload + HEADER_SIZE_FU_A, p, plen);
                //p -= 2;
                rtp_h264_fuhdr(payload, NALU, 0, 1);
                ret = rtsp_rtp_send(session, true, payload, plen + HEADER_SIZE_FU_A, 1);
                //ret = rtp_session_send_with_ts(session->video_session.rtp_session,
                //                               payload, plen + HEADER_SIZE_FU_A,
                //                               session->video_session.user_timestamp, 1);
                break;
            }
            i++;
        }
    }
    else
    {
        H264_NALU_FUHDR *hdr = (H264_NALU_FUHDR*)(payload);
        hdr->FU_F = H264_NALU_F(NALU);
        hdr->FU_NRI = H264_NALU_NRI(NALU);
        hdr->FU_Type = H264_NALU_TYPE(NALU);
        memcpy(payload + 1, p, plen);
        ret = rtsp_rtp_send(session, true, payload, plen + 1, 1);
        //ret = rtp_session_send_with_ts(session->video_session.rtp_session,
        //                               p, plen,
        //                               session->video_session.user_timestamp, 1);
    }
    return ret;
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


int _rtp_send_h264(rtsp_session_t *session, int type, const  uint8_t *buffer, uint32_t len)
{
    return rtp_send_h264_nalu(session,  type, buffer,  len);
}





/*
 * Append RTP payload to a H.264 picture bitstream. Note that the only
 * payload format that cares about packet lost is the NAL unit
 * fragmentation format (FU-A/B), so we will only manage the "prev_lost"
 * state for the FU-A/B packets.
 */
static int h264_nalu_unpacket_stap_a(uint8_t *payload,
                                     size_t payload_len, zpl_skbuffer_t *packet)
{
    uint8_t nal_start[4] = {0, 0, 0, 1};
    uint16_t *nalulen = (uint16_t *)payload;
    uint16_t nalusize = ntohs((*nalulen));
    uint8_t *p = payload;
    uint8_t *dst = NULL;
    size_t len = payload_len;
    while(len)
    {
        nalulen = (uint16_t *)p;
        nalusize = ntohs((*nalulen));
        p += 2;
        len -= 2;
        if(packet->skb_data == NULL)
        {
            packet->skb_data = malloc(nalusize + 4);
            packet->skb_len = packet->skb_maxsize = nalusize + 4;
        }
        else
        {
            if(packet->skb_maxsize < (packet->skb_len + nalusize + 4))
            {
                packet->skb_data = realloc(packet->skb_data, nalusize + 4);
                packet->skb_len += nalusize + 4;
                packet->skb_maxsize = packet->skb_len;
            }
        }
        if(packet->skb_data)
        {
            if(dst == NULL)
                dst = packet->skb_data;
            memcpy(dst, nal_start, 4);
            dst += 4;
            memcpy(dst, p, nalusize);
            packet->skb_len += nalusize;
            dst += nalusize;
            p += nalusize;
            len -= nalusize;
        }
    }
    return packet->skb_len;
}


static int h264_nalu_unpacket_fu_a(uint8_t *payload,
                                   size_t payload_len, zpl_skbuffer_t *packet)
{
    uint8_t nal_start[4] = {0, 0, 0, 1};
    H264_NALU_FUHDR *hdr = (H264_NALU_FUHDR*)(payload);
    uint8_t nalu = 0;
    //hdr->FU_F = H264_NALU_TYPE(NALU);
    //hdr->FU_NRI = H264_NALU_NRI(NALU);
    //hdr->FU_Type = NAL_TYPE_FU_A;
    //hdr->FU_HDR_S = (start)?1:0;
    //hdr->FU_HDR_E = end?1:0;
    //hdr->FU_HDR_R = 0;
    //hdr->FU_HDR_Type = H264_NALU_TYPE(NALU);
    nalu = (hdr->FU_F <<7)|(hdr->FU_NRI << 5) | hdr->FU_HDR_Type;
    if(hdr->FU_HDR_S)
    {
        /* Write NAL unit octet */
        zpl_skbuffer_put(packet, nal_start, 4);
        zpl_skbuffer_put(packet, &nalu, 1);
    }
    else
    {
        //payload[1] = nalu;
        zpl_skbuffer_put(packet, payload + 2, payload_len - 2);
    }
    if(hdr->FU_HDR_E)
    {
        return packet->skb_len;
    }
    return 0;
}

int _rtp_unpacket_h264(zpl_skbuffer_t *packet, uint8_t *payload,
                       size_t payload_len)
{
    int ret = 0;
    uint8_t nal_type = 0;

    nal_type = H264_NALU_TYPE(payload[0]);
    switch(nal_type)
    {
    case NAL_TYPE_STAP_A:
        ret = h264_nalu_unpacket_stap_a(payload + 1,
                                        payload_len - 1, packet);
        break;

    case NAL_TYPE_FU_A:
        ret = h264_nalu_unpacket_fu_a(payload, payload_len, packet);
        break;
    default:
        if (nal_type >= NAL_TYPE_SINGLE_NAL_MIN &&
                nal_type <= NAL_TYPE_SINGLE_NAL_MAX)
        {
            uint8_t nal_start[4] = {0, 0, 0, 1};
            ret = zpl_skbuffer_put(packet, nal_start, 4);
            ret = zpl_skbuffer_put(packet, payload, payload_len);
        }
        break;
    }
    return ret;
}




static int _h264_sprop_parameterset_parse(uint8_t *buffer, uint32_t len, zpl_video_extradata_t *extradata)
{
    uint32_t i = 0;
    uint8_t *p = NULL;
    uint8_t *start = buffer;
    uint8_t tmp[4096];
    size_t out_size = 0;
    for (p = buffer; *p != '\0' && i < len; ++p,i++)
    {
        if (*p == ',')
        {
            *p = '\0';
            memset(tmp, 0, sizeof(tmp));
            out_size = av_base64_decode(tmp, (const char*)start, sizeof(tmp));
            if(out_size)
            {
                uint8_t nal_unit_type = (tmp[0])&0x1F;
                if (nal_unit_type == 7/*SPS*/)
                {
                    #ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
                    if(out_size <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
                    #else 
                    extradata->fSPS = malloc(out_size);
                    if(extradata->fSPS)
                    #endif
                    {
                        memset(extradata->fSPS, 0, out_size);
                        memcpy(extradata->fSPS, tmp, out_size);
                        extradata->fSPSSize = out_size;
                    }
                }
                else if (nal_unit_type == 8/*PPS*/)
                {
                    #ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
                    if(out_size <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
                    #else 
                    extradata->fPPS = malloc(out_size);
                    if(extradata->fPPS)
                    #endif
                    {
                        memset(extradata->fPPS, 0, out_size);
                        memcpy(extradata->fPPS, tmp, out_size);
                        extradata->fPPSSize = out_size;
                    }
                }
            }
            start = p + 1;
        }
    }
    return 0;
}

int _rtsp_parse_sdp_h264(rtsp_session_t *session, uint8_t *attrval, uint32_t len)
{
    int format  = 0;
    zpl_video_extradata_t extradata;
    struct sdp_attr_fmtp_h264_t h264;
    rtsp_client_t *client = session->parent;

    memset(&h264, 0, sizeof(struct sdp_attr_fmtp_h264_t));
    sdp_attr_fmtp_h264(attrval, &format, &h264);
    session->_rtpsession->packetization_mode = h264.packetization_mode;


    _h264_sprop_parameterset_parse(h264.sprop_parameter_sets,
                                   h264.sprop_parameter_sets_len, &extradata);
    zpl_media_channel_decode_sps(extradata.fSPS, extradata.fSPSSize,
                                    &client->client_media.video_codec.vidsize.width,
                                    &client->client_media.video_codec.vidsize.height,
                                    &client->client_media.video_codec.framerate);
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
    if(extradata.fPPS)
        free(extradata.fPPS);
    if(extradata.fSPS)
        free(extradata.fSPS);
#endif
    return 0;
}













int _rtsp_build_sdp_h264(rtsp_session_t *session, uint8_t *src, uint32_t len)
{
    uint8_t base64sps[AV_BASE64_DECODE_SIZE(1024)];
    uint8_t base64pps[AV_BASE64_DECODE_SIZE(1024)];

    zpl_video_extradata_t *extradata = NULL;

    int profile = 0, sdplength = 0;


    if(session->rtsp_media)
    {
        extradata = &zpl_media_getptr(session->rtsp_media)->video_media.extradata;

        profile = extradata->profileLevelId;
    }
    else
        return 0;
/*
    if(extradata->fSPSSize == 0 && extradata->fSPS == NULL)
        zpl_media_file_extradata(zpl_media_getptr(session->rtsp_media)->video_media.halparam, &zpl_media_getptr(session->rtsp_media)->video_media.extradata);

    extradata = &zpl_media_getptr(session->rtsp_media)->video_media.extradata;
*/
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
    if(extradata->fSPSSize && extradata->fSPS != NULL)
#else
    if(extradata->fSPSSize)
#endif    
        zpl_media_channel_decode_sps(extradata->fSPS, extradata->fSPSSize,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.width,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.height,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.framerate);

    extradata = &zpl_media_getptr(session->rtsp_media)->video_media.extradata;

    if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264))
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_H264, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H264));
    else
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d H264/90000\r\n", RTP_MEDIA_PAYLOAD_H264);

    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    if(extradata->fPPSSize)
        av_base64_encode(base64pps, sizeof(base64pps), extradata->fPPS, extradata->fPPSSize);
    if(extradata->fSPSSize)
        av_base64_encode(base64sps, sizeof(base64sps), extradata->fSPS, extradata->fSPSSize);

    if (strlen(base64sps))
    {
        if (strlen(base64pps))
        {
            sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=%06x;"
                                                  " sprop-parameter-sets=%s,%s; packetization-mode=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, session->video_session.packetization_mode);
        }
        else
        {
            sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=%06x;"
                                                  " sprop-parameter-sets=%s,%s; packetization-mode=%d;bitrate=%d\r\n",
                                 RTP_MEDIA_PAYLOAD_H264, profile, base64sps, base64pps, session->video_session.packetization_mode, 48000 /*bitrate*/);
        }
    }
    else
    {
        sdplength += sprintf(src + sdplength, "a=fmtp:%d profile-level-id=42E00D; "
                                              "sprop-parameter-sets=Z0LgDdqFAlE=,aM48gA==,aFOPoA==; packetization-mode=%d\r\n",
                             RTP_MEDIA_PAYLOAD_H264, session->video_session.packetization_mode);
    }


    fprintf(stdout, " _rtp_build_sdp_h264 vidsize=%dx%d framerate=%d\r\n",
            zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.width,
            zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.height,
            zpl_media_getptr(session->rtsp_media)->video_media.codec.framerate);
    fflush(stdout);

    return sdplength;
}





int rtp_send_h264_test(void)
{
    int width = 0;
    int height = 0;
    int fps = 0;
    zpl_video_extradata_t extradata;
    char bufdda[] = {0x00,0x00,0x00,0x01,0x67,0x64,0x00,0x28,0xac,0xd9,0x40,0x78,0x02,0x27,0xe5,0xff
        ,0xc0,0x02,0x40,0x02,0x84,0x00,0x00,0x03,0x00,0x04,0x00,0x00,0x03,0x00,0xf0,0x3c
        ,0x60,0xc6,0x58};
    zpl_media_channel_decode_sps(bufdda, sizeof(bufdda), &width, &height, &fps);
    zpl_media_channel_decode_sps(bufdda+4, sizeof(bufdda)-1, &width, &height, &fps);

    zpl_media_file_t * rfile = zpl_media_file_create("/home/zhurish/workspace/working/zpl-source/source/multimedia/zplmedia/out.h264", "r");
    if(rfile)
    {
        rfile->get_frame = zpl_media_file_get_frame_h264;
        memset(&extradata, 0, sizeof(zpl_video_extradata_t));
        zpl_media_file_extradata(rfile, &extradata);

        rfile->filedesc.video.vidsize.width = width;
        rfile->filedesc.video.vidsize.height = height;
        rfile->filedesc.video.enctype = ZPL_VIDEO_CODEC_H264;
        rfile->filedesc.video.format = ZPL_VIDEO_FORMAT_720P;
        rfile->filedesc.video.framerate = fps;		//帧率
        rfile->filedesc.audio.enctype = ZPL_AUDIO_CODEC_NONE;

        zpl_media_file_close(rfile);
        zpl_media_file_destroy(rfile);
    }
    return 0;
}


#endif
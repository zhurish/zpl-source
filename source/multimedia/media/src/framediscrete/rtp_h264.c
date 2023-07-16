
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <stdbool.h>
#ifdef ZPL_JRTPLIB_MODULE 
#include <jrtplib_api.h>
#endif
#include "rtp_h264.h"
#include "rtp_payload.h"


#define H264_SPLIT_SIZE(n)  ((n)-4)


bool rtp_payload_h264_is_nalu3_start(u_int8_t *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 1) {
        return true;
    } else {
        return false;
    }
}

bool rtp_payload_h264_is_nalu4_start(u_int8_t *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
        return true;
    } else {
        return false;
    }
}

int rtp_payload_h264_get_nextnalu(u_int8_t *bufdata, u_int32_t len)
{
    u_int32_t pos = 0;
    pos = 0;
    while (pos < len)
    {
        if (rtp_payload_h264_is_nalu3_start(&bufdata[pos]))
        {
            return pos;
        }
        else if (rtp_payload_h264_is_nalu4_start(&bufdata[pos]))
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
bool rtp_payload_h264_isnaluhdr(u_int8_t *bufdata, RTP_H264_NALU_T *nalu)
{
    if (bufdata)
    {
        if (rtp_payload_h264_is_nalu4_start(bufdata))
        {
            nalu->hdr_len = 4;
            nalu->len = nalu->hdr_len;
            nalu->forbidden_bit = bufdata[nalu->hdr_len] & 0x80;	 //1 bit
            nalu->nal_idc = bufdata[nalu->hdr_len] & 0x60;		 // 2 bit
            nalu->nal_unit_type = (bufdata[nalu->hdr_len]) & 0x1f; // 5 bit
            nalu->buf = bufdata;
            return true;
        }
        else if (rtp_payload_h264_is_nalu3_start(bufdata))
        {
            nalu->hdr_len = 3;
            nalu->len = nalu->hdr_len;
            //nalu->len = len - nalu->hdr_len;
            //memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = bufdata[nalu->hdr_len] & 0x80;	 //1 bit
            nalu->nal_idc = bufdata[nalu->hdr_len] & 0x60;		 // 2 bit
            nalu->nal_unit_type = (bufdata[nalu->hdr_len]) & 0x1f; // 5 bit
            nalu->buf = bufdata;
            return true;
        }
        //nalu->len++;
    }
    return false;
}

static int rtp_payload_h264_hdr_set(u_int8_t *p, u_int8_t NALU, u_int8_t start, u_int8_t end)
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

static int rtp_payload_send_h264_oneframe(void *session, const u_int8_t *buffer, u_int32_t len, int user_ts)
{
    int ret = 0;
    u_int8_t *p = (u_int8_t *)buffer;
    u_int32_t plen = len;
    u_int8_t NALU = buffer[0];
    RTP_H264_NALU_T nalu;
    memset(&nalu, 0, sizeof(RTP_H264_NALU_T));
    if(rtp_payload_h264_isnaluhdr(p, &nalu))
    {
        nalu.len = len;
        p = (u_int8_t *)(buffer + nalu.hdr_len);
        plen = len - nalu.hdr_len;
        NALU = nalu.buf[nalu.hdr_len];
    }
    if(plen > MAX_RTP_PAYLOAD_LENGTH)
    {
        int i = 0;
        while(plen)
        {
            if(plen > MAX_RTP_PAYLOAD_LENGTH)
            {
                rtp_payload_h264_hdr_set(p-2, NALU, (i==0)?1:0, 0);
#ifdef ZPL_JRTPLIB_MODULE 
                ret = jrtp_session_sendto(session, p-2, MAX_RTP_PAYLOAD_LENGTH + HEADER_SIZE_FU_A, 255, 0, user_ts);
#endif
                p +=  MAX_RTP_PAYLOAD_LENGTH;
                plen -= MAX_RTP_PAYLOAD_LENGTH;
            }
            else
            {
                rtp_payload_h264_hdr_set(p-2, NALU, 0, 1);
#ifdef ZPL_JRTPLIB_MODULE 
                ret = jrtp_session_sendto(session, p-2, plen + HEADER_SIZE_FU_A, 255, 1, user_ts);
#endif
                break;
            }
            i++;
        }
    }
    else
    {
#ifdef ZPL_JRTPLIB_MODULE
        if(rtp_payload_h264_is_nalu3_start(p))
            ret = jrtp_session_sendto(session, p+3, plen-3, 255, 0, user_ts);
        else if(rtp_payload_h264_is_nalu4_start(p))
            ret = jrtp_session_sendto(session, p+4, plen-4, 255, 0, user_ts);
        else    
            ret = jrtp_session_sendto(session, p, plen, 255, 0, user_ts); 
#endif
    }
    return ret;
}



int rtp_payload_send_h264(void *session, const u_int8_t *buffer, u_int32_t len, int user_ts)
{
    return rtp_payload_send_h264_oneframe(session, buffer, len,  user_ts);
}
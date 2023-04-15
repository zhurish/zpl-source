#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdbool.h>
#ifdef ZPL_LIBORTP_MODULE 
#include <ortp/ortp.h>
#endif
#include "rtp_h264.h"



#define H264_SPLIT_SIZE(n)  ((n)-4)


bool rtp_payload_h264_is_nalu3_start(uint8_t *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 1) {
        return true;
    } else {
        return false;
    }
}

bool rtp_payload_h264_is_nalu4_start(uint8_t *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
        return true;
    } else {
        return false;
    }
}

int rtp_payload_h264_get_nextnalu(uint8_t *bufdata, uint32_t len)
{
    uint32_t pos = 0;
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
bool rtp_payload_h264_isnaluhdr(uint8_t *bufdata, RTP_H264_NALU_T *nalu)
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

static int rtp_payload_h264_hdr_set(uint8_t *p, uint8_t NALU, uint8_t start, uint8_t end)
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

static int rtp_payload_send_h264_oneframe(void *session, const uint8_t *buffer, uint32_t len, int user_ts)
{
    int ret = 0;
    uint8_t *p = (uint8_t *)buffer;
    uint32_t plen = len;
    uint8_t NALU = buffer[0];
    RTP_H264_NALU_T nalu;
    memset(&nalu, 0, sizeof(RTP_H264_NALU_T));
    if(rtp_payload_h264_isnaluhdr(p, &nalu))
    {
        nalu.len = len;
        p = (uint8_t *)(buffer + nalu.hdr_len);
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
                #ifdef ZPL_LIBORTP_MODULE 
                ret = rtp_session_send_with_ts(session, p-2, MAX_RTP_PAYLOAD_LENGTH + HEADER_SIZE_FU_A, user_ts);
                #endif
                p +=  MAX_RTP_PAYLOAD_LENGTH;
                plen -= MAX_RTP_PAYLOAD_LENGTH;
            }
            else
            {
                rtp_payload_h264_hdr_set(p-2, NALU, 0, 1);
                #ifdef ZPL_LIBORTP_MODULE 
                ret = rtp_session_send_with_ts(session, p-2, plen + HEADER_SIZE_FU_A, user_ts);
                #endif
                break;
            }
            i++;
        }
    }
    else
    {
        #ifdef ZPL_LIBORTP_MODULE 
      /*  zpl_char hexformat[2048];

                memset(hexformat, 0, sizeof(hexformat));
                os_loghex(hexformat, sizeof(hexformat), buffer, len);
                zlog_debug(MODULE_MEDIA, "-------- rtp send Table ------+---------+");
                zlog_debug(MODULE_MEDIA, " hex :\n%s", hexformat);
                zlog_debug(MODULE_MEDIA, "---------+--------+-------+---------+");
*/
        ret = rtp_session_send_with_ts(session, p, plen, user_ts);
        #endif
    }
    return ret;
}



int rtp_payload_send_h264(void *session, const uint8_t *buffer, uint32_t len, int user_ts)
{
    RTP_H264_NALU_T nalu;
    uint8_t *tmpbuf = buffer;
    int data_offset=0, pos = 0;
    return rtp_payload_send_h264_oneframe(session, buffer, len,  user_ts);

    memset(&nalu, 0, sizeof(RTP_H264_NALU_T));
    while(data_offset <(len-4))
    {
        if(rtp_payload_h264_isnaluhdr(tmpbuf, &nalu))
        {
            tmpbuf += nalu.hdr_len;
            data_offset += nalu.hdr_len;
        }
        if(nalu.hdr_len)
        {
            pos = rtp_payload_h264_get_nextnalu(tmpbuf, (len-4)-data_offset);
            if(pos)
            {
                nalu.len += pos;
                //zpl_media_channel_nalu2extradata(&nalu, &chn->media_param.video_media.extradata);
                //memcpy(extradata->fSEI, nalu->buf, nalu->len);
                rtp_payload_send_h264_oneframe(session, nalu.buf, nalu.len,  user_ts);
                memset(&nalu, 0, sizeof(RTP_H264_NALU_T));
                tmpbuf += pos;
                data_offset += pos;
                continue;
            }
            else
            {
                nalu.len += ((len)-data_offset);
                //zpl_media_channel_nalu2extradata(&nalu, &chn->media_param.video_media.extradata);
                //memcpy(extradata->fSEI, nalu->buf, nalu->len);
                rtp_payload_send_h264_oneframe(session, nalu.buf, nalu.len,  user_ts);
                memset(&nalu, 0, sizeof(RTP_H264_NALU_T));
                return 0;
            }
        }
        tmpbuf++;
        data_offset++;
    }
    
    /*while(1)
    {
        pos = rtp_payload_h264_get_nextnalu(p + 4, plen - 4);
        if(pos == 0)
            return rtp_payload_send_h264_oneframe(session, p,  plen,  user_ts);
        else
        {
            rtp_payload_send_h264_oneframe(session, p,  pos + 4,  user_ts);
            p += (pos+4);
            plen -= (pos+4);
        }    
    }*/
    return 0;
}
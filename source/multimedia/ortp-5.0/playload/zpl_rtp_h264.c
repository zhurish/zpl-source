#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "zpl_rtsp_def.h"


#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_server.h"
#include "zpl_rtp_h264.h"



#define H264_SPLIT_SIZE(n)  ((n)+4)

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



static int _h264_file_read_more_data(zpl_media_file_t *file)
{
    int ret = 0;
    if(feof(file->fp) != 0)
    {
        if(file->tmppacket.len <= 0)
            return -1;
    }
    else
    {
        memset(file->tmppacket.data + file->tmppacket.len, 0, file->tmppacket.maxsize - file->tmppacket.len);
        ret = fread(file->tmppacket.data + file->tmppacket.len, 1, file->tmppacket.maxsize - file->tmppacket.len, file->fp);
        if(ret <= 0)
        {
            if(file->tmppacket.len <= 0)
                return -1;
        }
        file->tmppacket.len += ret;
        return file->tmppacket.len;
    }
    return 0;
}

static int _h264_file_data_append_finsh(zpl_media_file_t *file, int packetsize)
{
    file->tmppacket.len = file->tmppacket.len - packetsize;
    if(file->tmppacket.len && file->tmppacket.len < file->tmppacket.maxsize)
        memmove(file->tmppacket.data, file->tmppacket.data + packetsize, file->tmppacket.len);
    return 0;
}

static int _h264_file_read_data(zpl_media_file_t *file, zpl_media_bufcache_t *outpacket)
{
    int finsh = 0;
    uint32_t packetsize = MAX_RTP_PAYLOAD_LENGTH;
    uint32_t nalulen = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    memset(&naluhdr, 0, sizeof(H264_NALU_T));

    while(1)
    {
        if(_h264_file_read_more_data(file) == -1)
            return -1;
        p = file->tmppacket.data;

        if(packetsize > file->tmppacket.len)
            packetsize = file->tmppacket.len;
        else
            packetsize = MAX_RTP_PAYLOAD_LENGTH;

        if(zpl_media_channel_isnaluhdr(p, &naluhdr))
        {
            p+=naluhdr.hdr_len;
            file->flags |= NALU_START_FLAG;
            outpacket->len = 0;
            //zpl_media_channel_nalu_show(&naluhdr);
        }
        nalulen = 0;
        nalulen = zpl_media_channel_get_nextnalu(p, H264_SPLIT_SIZE(packetsize));
        if(nalulen)
        {
            if(file->flags & NALU_START_FLAG)
            {
                if(file->flags & NALU_MAX_FLAG)
                {
                    naluhdr.len += (nalulen);
                    zpl_media_bufcache_add(outpacket, file->tmppacket.data, nalulen);
                    _h264_file_data_append_finsh(file, nalulen);
                    file->flags = 0;
                    //zpl_media_channel_nalu_show(&naluhdr);
                    return outpacket->len;
                }
                else
                {

                    naluhdr.len += nalulen;
                    //packetsize = nalulen + naluhdr.hdr_len;
                    zpl_media_bufcache_add(outpacket, file->tmppacket.data, nalulen + naluhdr.hdr_len);
                    file->flags |= NALU_MAX_FLAG;
                    _h264_file_data_append_finsh(file, nalulen + naluhdr.hdr_len);
                    file->flags = 0;
                    //zpl_media_channel_nalu_show(&naluhdr);
                    return outpacket->len;

                }
            }
            else
            {
                fprintf(stdout, "===========================\n");
            }
        }
        else
        {
            if(packetsize < MAX_RTP_PAYLOAD_LENGTH)
            {
                if((file->flags & NALU_MAX_FLAG))
                {
                    naluhdr.len += file->tmppacket.len;
                    packetsize = file->tmppacket.len;
                    finsh = 1;
                }
                else
                {
                    naluhdr.len = file->tmppacket.len;
                    packetsize = file->tmppacket.len;
                    finsh = 1;
                }
            }
            else
            {
                if((file->flags & NALU_MAX_FLAG))
                {
                    naluhdr.len += MAX_RTP_PAYLOAD_LENGTH;
                    packetsize = MAX_RTP_PAYLOAD_LENGTH;
                }
                else
                {
                    naluhdr.len += MAX_RTP_PAYLOAD_LENGTH;
                    packetsize = MAX_RTP_PAYLOAD_LENGTH + naluhdr.hdr_len;
                    file->flags |= NALU_MAX_FLAG;
                }
            }

            zpl_media_bufcache_add(outpacket, file->tmppacket.data, packetsize);

            _h264_file_data_append_finsh(file, packetsize);
            if(finsh)
            {
                file->flags = 0;
                //zpl_media_channel_nalu_show(&naluhdr);
                return outpacket->len;
            }
        }
    }
    return -1;
}

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


int _h264_file_get_frame(void *fi, void *p)
{
    int ret = 0;
    zpl_media_file_t *file = fi;
    ret = _h264_file_read_data(file, p);
    fprintf(stdout, " rtp_h264_get_frame ret=%d\r\n", ret);
    fflush(stdout);
    return ret;
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
                    extradata->fSPS = malloc(out_size);
                    if(extradata->fSPS)
                    {
                        memset(extradata->fSPS, 0, out_size);
                        memcpy(extradata->fSPS, tmp, out_size);
                        extradata->fSPSSize = out_size;
                    }
                }
                else if (nal_unit_type == 8/*PPS*/)
                {
                    extradata->fPPS = malloc(out_size);
                    if(extradata->fPPS)
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
    zpl_media_channel_decode_spspps(extradata.fSPS, extradata.fSPSSize,
                                    &client->client_media.video_codec.vidsize.width,
                                    &client->client_media.video_codec.vidsize.height,
                                    &client->client_media.video_codec.framerate);
    if(extradata.fPPS)
        free(extradata.fPPS);
    if(extradata.fSPS)
        free(extradata.fSPS);

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
    if(extradata->fSPSSize && extradata->fSPS != NULL)
        zpl_media_channel_decode_spspps(extradata->fSPS, extradata->fSPSSize,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.width,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.vidsize.height,
                                    &zpl_media_getptr(session->rtsp_media)->video_media.codec.framerate);

    extradata = &zpl_media_getptr(session->rtsp_media)->video_media.extradata;

    if (rtsp_rtp_get_rtpmap(RTP_MEDIA_PAYLOAD_H264))
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_H264, rtsp_rtp_get_rtpmap(RTP_MEDIA_PAYLOAD_H264));
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





int rtp_send_h264_test()
{
    zpl_video_extradata_t extradata;
    zpl_media_file_t * rfile = zpl_media_file_create("aa.h264", "r");
    zpl_media_file_t * wfile = zpl_media_file_create("testoutfiledesc.h264", "a+");
    if(rfile && wfile)
    {
        rfile->get_frame = _h264_file_get_frame;
        memset(&extradata, 0, sizeof(zpl_video_extradata_t));
        zpl_media_file_extradata(rfile, &extradata);
        int width = 0;
        int height = 0;
        int fps = 0;
        int ret = 0;
zpl_skbuffer_t packet;
        zpl_media_channel_decode_spspps(extradata.fSPS, extradata.fSPSSize, &width, &height, &fps);
zpl_media_file_open(rfile);
        wfile->filedesc.video.vidsize.width = width;
        wfile->filedesc.video.vidsize.height = height;
        wfile->filedesc.video.enctype = ZPL_VIDEO_CODEC_H264;
        wfile->filedesc.video.format = ZPL_VIDEO_FORMAT_720P;
        wfile->filedesc.video.framerate = fps;		//帧率
        wfile->filedesc.audio.enctype = ZPL_AUDIO_CODEC_NONE;

        zpl_media_file_writehdr(wfile, &wfile->filedesc);

        while(1)
        {
            ret = _h264_file_get_frame(rfile, &packet);
            if(ret > 0)
            {
                ret = zpl_media_file_write_data(wfile, packet.skb_data, packet.skb_len);
                fprintf(stdout, " zpl_media_file_write_data ret=%d\r\n", ret);
            }
            else
                break;
        }
        zpl_media_file_close(rfile);
        zpl_media_file_close(wfile);
        zpl_media_file_destroy(rfile);
        zpl_media_file_destroy(wfile);
    }
    return 0;
}


#endif
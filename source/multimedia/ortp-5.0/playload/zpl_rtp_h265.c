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
#include "zpl_rtp_h265.h"



/*
 * 多个nalu组成一个包
 */
int _h265_data_packet(zpl_skbuffer_t *nalu, uint8_t *out)
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


int _h265_file_get_frame(void *fi, void *p)
{
    int ret = 0;
    return ret;
}

#if 0
uint8_t nal_type = (buf[0] >> 1) & 0x3F;
/*
 * create the HEVC payload header and transmit the buffer as fragmentation units (FU)
 *
 *    0                   1
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |F|   Type    |  LayerId  | TID |
 *   +-------------+-----------------+
 *
 *      F       = 0
 *      Type    = 49 (fragmentation unit (FU))
 *      LayerId = 0
 *      TID     = 1
 */
s->buf[0] = 49 << 1;
s->buf[1] = 1;

/*
 *     create the FU header
 *
 *     0 1 2 3 4 5 6 7
 *    +-+-+-+-+-+-+-+-+
 *    |S|E|  FuType   |
 *    +---------------+
 *
 *       S       = variable
 *       E       = variable
 *       FuType  = NAL unit type
 */
s->buf[2]  = nal_type;
/* set the S bit: mark as start fragment */
s->buf[2] |= 1 << 7;

/* pass the original NAL header */
buf  += 2;
size -= 2;
#endif
static int rtp_h265_fuhdr(uint8_t *p, uint8_t NALU, uint8_t start, uint8_t end)
{
    H265_NALU_FUHDR *hdr = (H265_NALU_FUHDR*)(p);
    hdr->FU_F = 0;
    hdr->Type = NAL5_TYPE_FU_A;
    hdr->TID = 1;
    hdr->LayerId = 0;

    hdr->FU_HDR_S = start;
    hdr->FU_HDR_E = end;
    hdr->FU_Type = H265_NALU_TYPE(NALU);
    return 0;
}

static int rtp_send_h265_nalu(rtsp_session_t *session, int type, const uint8_t *buffer, uint32_t len)
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
                memcpy(payload + H265_HDR_LEN, p, MAX_RTP_PAYLOAD_LENGTH);
                rtp_h265_fuhdr(payload, NALU, (i==0)?1:0, 0);

                ret = rtsp_rtp_send(session, true, payload, MAX_RTP_PAYLOAD_LENGTH + H265_HDR_LEN, 0);

                p +=  MAX_RTP_PAYLOAD_LENGTH;
                plen -= MAX_RTP_PAYLOAD_LENGTH;
            }
            else
            {
                memcpy(payload + H265_HDR_LEN, p, plen);
                rtp_h265_fuhdr(payload, NALU, 0, 1);
                ret = rtsp_rtp_send(session, true, payload, plen + H265_HDR_LEN, 1);

                break;
            }
            i++;
        }
    }
    else
    {
        ret = rtsp_rtp_send(session, true, p, plen, 1);
    }
    return ret;
}


int _rtp_send_h265(rtsp_session_t *session, int type, const  uint8_t *buffer, uint32_t len)
{
    return rtp_send_h265_nalu(session,  type, buffer,  len);
}





/*
 * Append RTP payload to a H.265 picture bitstream. Note that the only
 * payload format that cares about packet lost is the NAL unit
 * fragmentation format (FU-A/B), so we will only manage the "prev_lost"
 * state for the FU-A/B packets.
 */
static int h265_nalu_unpacket_stap_a(uint8_t *payload,
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


static int h265_nalu_unpacket_fu_a(uint8_t *payload,
                                   size_t payload_len, zpl_skbuffer_t *packet)
{
    uint8_t nal_start[4] = {0, 0, 0, 1};
    H265_NALU_FUHDR *hdr = (H265_NALU_FUHDR*)(payload);
    uint8_t nalu = 0;
    //hdr->FU_F = H264_NALU_TYPE(NALU);
    //hdr->FU_NRI = H264_NALU_NRI(NALU);
    //hdr->FU_Type = NAL_TYPE_FU_A;
    //hdr->FU_HDR_S = (start)?1:0;
    //hdr->FU_HDR_E = end?1:0;
    //hdr->FU_HDR_R = 0;
    //hdr->FU_HDR_Type = H264_NALU_TYPE(NALU);
    //nalu =  hdr->FU_Type;
    nalu = (hdr->FU_Type << 1) | (payload[0] & 0x81); // replace NAL Unit Type Bits
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

int _rtp_unpacket_h265(zpl_skbuffer_t *packet, uint8_t *payload,
                       size_t payload_len)
{
    int ret = 0;
    uint8_t nal_type = 0;
    H265_NALU_FUHDR *hdr = (H265_NALU_FUHDR*)(payload);
    //nal_type = H265_NALU_TYPE(payload[0]);
    nal_type = hdr->Type;
    switch(nal_type)
    {
    case NAL5_TYPE_STAP_A:
        ret = h265_nalu_unpacket_stap_a(payload + 1,
                                        payload_len - 1, packet);
        break;

    case NAL5_TYPE_FU_A:
        ret = h265_nalu_unpacket_fu_a(payload, payload_len, packet);
        break;
    default:
        {
            uint8_t nal_start[4] = {0, 0, 0, 1};
            ret = zpl_skbuffer_put(packet, nal_start, 4);
            ret = zpl_skbuffer_put(packet, payload, payload_len);
        }
        break;
    }
    return ret;
}




static int _h265_sprop_parameterset_parse(uint8_t *buffer, uint32_t len, zpl_video_extradata_t *extradata)
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

int _rtsp_parse_sdp_h265(rtsp_session_t *session, uint8_t *attrval, uint32_t len)
{
    int format  = 0;
    zpl_video_extradata_t extradata;
    struct sdp_attr_fmtp_h265_t h265;
    rtsp_client_t *client = session->parent;

    memset(&h265, 0, sizeof(struct sdp_attr_fmtp_h265_t));
    sdp_attr_fmtp_h265(attrval, &format, &h265);
    //session->_rtpsession->packetization_mode = h265.packetization_mode;


    _h265_sprop_parameterset_parse(h265.sprop_sps,
                                   1, &extradata);

    zpl_media_channel_decode_sps(extradata.fSPS, extradata.fSPSSize,
                                    &client->client_media.video_codec.vidsize.width,
                                    &client->client_media.video_codec.vidsize.height,
                                    &client->client_media.video_codec.framerate);
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE                          
    if(extradata.fPPS)
        free(extradata.fPPS);
    if(extradata.fSPS)
        free(extradata.fSPS);
    if(extradata.fVPS)
        free(extradata.fVPS);
#endif
    return 0;
}









#if 0
int sdp_h265(uint8_t *data, int bytes, const char* proto, unsigned short port, int payload, int frequence, const void* extra, int extra_size)
{
    static const char* pattern =
        "m=video %hu %s %d\n"
        "a=rtpmap:%d H265/90000\n"
        "a=fmtp:%d";

    const uint8_t nalu[] = { 32/*vps*/, 33/*sps*/, 34/*pps*/ };
    const char* sprop[] = { "sprop-vps", "sprop-sps", "sprop-pps" };

    int r, n;
    int i, j, k;
    struct mpeg4_hevc_t hevc;

    assert(90000 == frequence);
    r = mpeg4_hevc_decoder_configuration_record_load((const uint8_t*)extra, extra_size, &hevc);
    if (r < 0)
        return r;

    n = snprintf((char*)data, bytes, pattern, port, proto && *proto ? proto : "RTP/AVP", payload, payload, payload);

    for (i = 0; i < sizeof(nalu) / sizeof(nalu[0]); i++)
    {
        if (i > 0 && n < bytes) data[n++] = ';';
        n += snprintf((char*)data + n, bytes - n, " %s=", sprop[i]);

        for (k = j = 0; j < hevc.numOfArrays; j++)
        {
            assert(hevc.nalu[j].type == ((hevc.nalu[j].data[0] >> 1) & 0x3F));
            if (nalu[i] != hevc.nalu[j].type)
                continue;

            if (n + 1 + hevc.nalu[j].bytes * 2 > bytes)
                return -1; // don't have enough memory

            if (k++ > 0 && n < bytes) data[n++] = ',';
            n += base64_encode((char*)data + n, hevc.nalu[j].data, hevc.nalu[j].bytes);
        }
    }

    if(n < bytes)
        data[n++] = '\n';
    return n;
}
#endif


int _rtsp_build_sdp_h265(rtsp_session_t *session, uint8_t *src, uint32_t len)
{
    uint8_t base64sps[AV_BASE64_DECODE_SIZE(1024)];
    uint8_t base64pps[AV_BASE64_DECODE_SIZE(1024)];
    uint8_t base64vps[AV_BASE64_DECODE_SIZE(1024)];
    uint8_t tmpflag = 0;
    zpl_video_extradata_t extradata;

    int profile = 0, sdplength = 0;
    if (rtsp_media_lookup(session, session->mchannel, session->mlevel, session->mfilepath))
    {
        rtsp_media_extradata_get(session, session->mchannel, session->mlevel, session->mfilepath, &extradata);
        profile = extradata.profileLevelId;
    }
    else
        return 0;

    if (rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H265))
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d %s\r\n", RTP_MEDIA_PAYLOAD_H265, rtp_profile_get_rtpmap(RTP_MEDIA_PAYLOAD_H265));
    else
        sdplength += sprintf(src + sdplength, "a=rtpmap:%d H265/90000\r\n", RTP_MEDIA_PAYLOAD_H265);

    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    memset(base64vps, 0, sizeof(base64vps));
    if(extradata.fPPSSize)
        av_base64_encode(base64pps, sizeof(base64pps), extradata.fPPS, extradata.fPPSSize);
    if(extradata.fSPSSize)
        av_base64_encode(base64sps, sizeof(base64sps), extradata.fSPS, extradata.fSPSSize);
    if(extradata.fVPSSize)
        av_base64_encode(base64vps, sizeof(base64vps), extradata.fVPS, extradata.fVPSSize);

    sdplength += sprintf(src + sdplength, "a=fmtp:%d", RTP_MEDIA_PAYLOAD_H265);

    if (strlen(base64sps))
    {
        sdplength += sprintf(src + sdplength, " sprop-sps=%s", base64pps);
        tmpflag++;
    }
    if (strlen(base64pps))
    {
        sdplength += sprintf(src + sdplength, "%ssprop-pps=%s", tmpflag?" ":";",base64pps);
        tmpflag++;
    }
    if (strlen(base64vps))
    {
        sdplength += sprintf(src + sdplength, "%ssprop-vps=%s", tmpflag?" ":";",base64vps);
        tmpflag++;
    }
    sdplength += sprintf(src + sdplength, "\r\n");
    return sdplength;
}


#if 0
int sdp_mpeg4_es(uint8_t *data, int bytes, const char* proto, unsigned short port, int payload, int frequence, const void* extra, int extra_size)
{
    static const char* pattern =
        "m=video %hu %s %d\n"
        "a=rtpmap:%d MP4V-ES/90000\n"
        "a=fmtp:%d profile-level-id=1;config=";

    int n;

    assert(90000 == frequence);
    n = snprintf((char*)data, bytes, pattern, port, proto && *proto ? proto : "RTP/AVP", payload, payload, payload);

    if (n + extra_size * 2 + 1 > bytes)
        return -1; // // don't have enough memory

    // It is a hexadecimal representation of an octet string that
    // expresses the MPEG-4 Visual configuration information
    n += base16_encode((char*)data + n, extra, extra_size);

    if (n < bytes)
        data[n++] = '\n';
    return n;
}
#endif
#endif
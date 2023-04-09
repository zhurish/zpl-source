/*
 * zpl_media_extradata.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_extradata.h"
#include <math.h>
#include "nal-h264.h"
#include "nal-hevc.h"

bool is_nalu3_start(zpl_uint8 *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 1) {
        return true;
    } else {
        return false;
    }
}

bool is_nalu4_start(zpl_uint8 *buffer) {
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1) {
        return true;
    } else {
        return false;
    }
}

int zpl_media_channel_get_nextnalu(zpl_uint8 *bufdata, zpl_uint32 len)
{
    zpl_uint32 pos = 0;
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

zpl_uint32 zpl_media_channel_get_profileLevelId(zpl_uint8 const* from, zpl_uint32 fromSize, zpl_uint8* to, zpl_uint32 toMaxSize) {
    zpl_uint32 toSize = 0;
    zpl_uint32 i = 0;
    while (i < fromSize && toSize+1 < toMaxSize) {
        if (i+2 < fromSize && from[i] == 0 && from[i+1] == 0 && from[i+2] == 3) {
            to[toSize] = to[toSize+1] = 0;
            toSize += 2;
            i += 3;
        } else {
            to[toSize] = from[i];
            toSize += 1;
            i += 1;
        }
    }
    return toSize;
}

bool zpl_media_channel_isnaluhdr(zpl_uint8 *bufdata, H264_NALU_T *nalu)
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
            nalu->buf = bufdata;
            return true;
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
            nalu->buf = bufdata;
            return true;
        }
        //nalu->len++;
    }
    return false;
}

int zpl_media_channel_nalu2extradata(H264_NALU_T *nalu, zpl_video_extradata_t *extradata)
{
    zpl_uint8 tmp[512];
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE    
    if (extradata->fSPS != NULL && extradata->fPPS != NULL/* && extradata->fSEI != NULL*/)
    {
        //fprintf(stdout, " _zpl_media_file_get_extradata already\r\n");
        fflush(stdout);
        return 1;
    }
#endif    
    if (nalu->nal_unit_type)
    {
        switch (nalu->nal_unit_type)
        {
        case NALU_TYPE_SLICE:
            break;
        case NALU_TYPE_DPA:
            break;
        case NALU_TYPE_DPB:
            break;
        case NALU_TYPE_DPC:
            break;
        case NALU_TYPE_IDR:
            break;
        case NALU_TYPE_SEI:
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE
            if (extradata->fSEI == NULL)
            {
                extradata->fSEI = malloc(nalu->len);
                if (extradata->fSEI)
                {
                    extradata->fSEISize = nalu->len;
                    extradata->fSEIHdrLen = nalu->hdr_len;
                    memcpy(extradata->fSEI, nalu->buf, nalu->len);
                }
            }
#else
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                extradata->fSEISize = nalu->len;
                extradata->fSEIHdrLen = nalu->hdr_len;
                memcpy(extradata->fSEI, nalu->buf, nalu->len);
            }
#endif
            break;
        case NALU_TYPE_SPS:
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                memset(tmp, 0, sizeof(tmp));
                memcpy(extradata->fSPS, nalu->buf, nalu->len);
                extradata->fSPSSize = nalu->len;
                extradata->fSPSHdrLen = nalu->hdr_len;
                if(zpl_media_channel_get_profileLevelId(nalu->buf+nalu->hdr_len, nalu->len - nalu->hdr_len,
                                                            tmp, sizeof(tmp)) >= 4)
                extradata->h264spspps.profileLevelId = (tmp[1]<<16) | (tmp[2]<<8) | tmp[3];
            }
#else        
            if (extradata->fSPS == NULL)
            {
                extradata->fSPS = malloc(nalu->len);
                if (extradata->fSPS)
                {
                    memset(tmp, 0, sizeof(tmp));
                    memcpy(extradata->fSPS, nalu->buf, nalu->len);
                    extradata->fSPSSize = nalu->len;
                    extradata->fSPSHdrLen = nalu->hdr_len;
                    if(zpl_media_channel_get_profileLevelId(nalu->buf + nalu->hdr_len, nalu->len - nalu->hdr_len,
                                                            tmp, sizeof(tmp)) >= 4)
                        extradata->h264spspps.profileLevelId = (tmp[1]<<16) | (tmp[2]<<8) | tmp[3];
                }
            }
#endif
            break;
        case NALU_TYPE_PPS:
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                extradata->fPPSSize = nalu->len;
                extradata->fPPSHdrLen = nalu->hdr_len;
                memcpy(extradata->fPPS, nalu->buf, nalu->len);
            }
#else
            if (extradata->fPPS == NULL)
            {
                extradata->fPPS = malloc(nalu->len);
                if (extradata->fPPS)
                {
                    extradata->fPPSHdrLen = nalu->hdr_len;
                    extradata->fPPSSize = nalu->len;
                    memcpy(extradata->fPPS, nalu->buf, nalu->len);
                }
            }
#endif
            break;
        case NALU_TYPE_AUD:
            break;
        case NALU_TYPE_EOSEQ:
            break;
        case NALU_TYPE_EOSTREAM:
            break;
        case NALU_TYPE_FILL:
            break;
        default:
            return -1;    
        }
    }
    return 0;
}

int zpl_media_channel_nalu_show(H264_NALU_T *nalu)
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
        default:
            sprintf(type_str, "TYPE-%d", nalu->nal_unit_type);
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
        default:
            sprintf(idc_str, "IDC-%d", nalu->nal_idc);
            break;  
        }
        switch (nalu->nal_unit_type)
        {
            case NALU_TYPE_SEI:
            case NALU_TYPE_SPS:
            case NALU_TYPE_PPS:
            zlog_debug(MODULE_ZPLMEDIA, "-------- NALU Table ------+---------+");
            zlog_debug(MODULE_ZPLMEDIA, "       IDC |  TYPE |   LEN   |");
            zlog_debug(MODULE_ZPLMEDIA, "---------+--------+-------+---------+");
            zlog_debug(MODULE_ZPLMEDIA, " %7s| %6s| %8d|", idc_str, type_str, nalu->len-nalu->hdr_len);
            break;
        }
    }
    return 0;
}




/**
 * Analysis H.264 Bitstream
 * @param url    Location of input H.264 bitstream file.
 */
int zpl_media_channel_extradata_import(void *p, zpl_uint8 *Buf, int len)
{
    H264_NALU_T nalu;
    zpl_media_channel_t *chn = (zpl_media_channel_t *)p;
    zpl_uint8 *tmpbuf = Buf;
    int data_offset=0, pos = 0;
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE  
#else
    if(chn && chn->media_param.video_media.extradata.fSPS != NULL && 
        chn->media_param.video_media.extradata.fPPS != NULL /*&& 
        chn->media_param.video_media.extradata.fSEI != NULL*/)
        return 1;
#endif
    memset(&nalu, 0, sizeof(H264_NALU_T));
    while(data_offset <(len-4))
    {
        if(zpl_media_channel_isnaluhdr(tmpbuf, &nalu))
        {
            tmpbuf += nalu.hdr_len;
            data_offset += nalu.hdr_len;
        }
        if(nalu.hdr_len)
        {
            pos = zpl_media_channel_get_nextnalu(tmpbuf, (len-4)-data_offset);
            if(pos)
            {
                nalu.len += pos;
                if(ZPL_MEDIA_DEBUG(ENCODE, BUFDETAIL))
                    zpl_media_channel_nalu_show(&nalu);
                if(chn)
                    zpl_media_channel_nalu2extradata(&nalu, &chn->media_param.video_media.extradata);
                memset(&nalu, 0, sizeof(H264_NALU_T));
                tmpbuf += pos;
                data_offset += pos;
                continue;
            }
            else
            {
                nalu.len += ((len)-data_offset);
                if(ZPL_MEDIA_DEBUG(ENCODE, BUFDETAIL))
                    zpl_media_channel_nalu_show(&nalu);
                if(chn)
                    zpl_media_channel_nalu2extradata(&nalu, &chn->media_param.video_media.extradata);
                memset(&nalu, 0, sizeof(H264_NALU_T));
                return 0;
            }
        }
        tmpbuf++;
        data_offset++;
    }
    return 0;
}


int zpl_media_channel_extradata_get(void *p, zpl_video_extradata_t *extradata)
{
    int n = 0;
    zpl_media_channel_t *chn = (zpl_media_channel_t *)p;
    if(chn && (chn->media_param.video_media.halparam))
    {
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE         
        if(chn->media_param.video_media.extradata.fPPS && extradata->fPPS)
 #endif
        {
			n++;
            memcpy(extradata->fPPS, chn->media_param.video_media.extradata.fPPS, chn->media_param.video_media.extradata.fPPSSize);
            extradata->fPPSSize = chn->media_param.video_media.extradata.fPPSSize;
            extradata->fPPSHdrLen = chn->media_param.video_media.extradata.fPPSHdrLen;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->media_param.video_media.extradata.fVPS && extradata->fVPS)
 #endif
        {
			n++;
            memcpy(extradata->fVPS, chn->media_param.video_media.extradata.fVPS, chn->media_param.video_media.extradata.fVPSSize);
            extradata->fVPSSize = chn->media_param.video_media.extradata.fVPSSize;
            extradata->fVPSHdrLen = chn->media_param.video_media.extradata.fVPSHdrLen;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->media_param.video_media.extradata.fSPS && extradata->fSPS)
 #endif
        {
			n++;
            memcpy(extradata->fSPS, chn->media_param.video_media.extradata.fSPS, chn->media_param.video_media.extradata.fSPSSize);
            extradata->fSPSSize = chn->media_param.video_media.extradata.fSPSSize;
            extradata->fSPSHdrLen = chn->media_param.video_media.extradata.fSPSHdrLen;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->media_param.video_media.extradata.fSEI && extradata->fSEI)
 #endif
        {
			n++;
            memcpy(extradata->fSEI, chn->media_param.video_media.extradata.fSEI, chn->media_param.video_media.extradata.fSEISize);
            extradata->fSEISize = chn->media_param.video_media.extradata.fSEISize;
            extradata->fSEIHdrLen = chn->media_param.video_media.extradata.fSEIHdrLen;
        }
        if(chn->media_param.video_media.extradata.h264spspps.profileLevelId && extradata->h264spspps.profileLevelId)
        {
			n++;
            extradata->h264spspps.profileLevelId = chn->media_param.video_media.extradata.h264spspps.profileLevelId;
        }
		if(n)
        	return OK;
    }
    return ERROR;
}


int zpl_media_channel_extradata_delete(zpl_video_extradata_t *extradata)
{
    if(extradata)
    {
 #ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
        extradata->fPPSSize = 0;
        extradata->fVPSSize = 0;
        extradata->fSPSSize = 0;
        extradata->fSEISize = 0;
        extradata->fPPSHdrLen = 0;
        extradata->fVPSHdrLen = 0;
        extradata->fSPSHdrLen = 0;
        extradata->fSEIHdrLen = 0; 
 #else
        if(extradata->fPPS)
        {
            free(extradata->fPPS);
			extradata->fPPSSize = 0;
        }
        if(extradata->fVPS)
        {
			free(extradata->fVPS);
			extradata->fVPSSize = 0;
        }
        if(extradata->fSPS)
        {
			free(extradata->fSPS);
			extradata->fSPSSize = 0;
        }
        if(extradata->fSEI)
        {
			free(extradata->fSEI);
			extradata->fSEISize = 0;
        }
#endif
        if(extradata->h264spspps.profileLevelId)
        {
            extradata->h264spspps.profileLevelId = 0;
        }
        return OK;
    }
    return ERROR;
}

static void sps_get_profile(int profile_idc, char* profile_str){
    switch(profile_idc){
    case 66:
        strcpy(profile_str, "Baseline");
        break;
    case 77:
        strcpy(profile_str, "Main");
        break;
    case 88:
        strcpy(profile_str, "Extended");
        break;
    case 100:
        strcpy(profile_str, "High(FRExt)");
        break;
    case 110:
        strcpy(profile_str, "High10(FRExt)");
        break;
    case 122:
        strcpy(profile_str, "High4:2:2(FRExt)");
        break;
    case 144:
        strcpy(profile_str, "High4:4:4(FRExt)");
        break;
    default:
        strcpy(profile_str, "Unknown");
    }
}

int zpl_media_channel_decode_sps(const zpl_uint8 *buf, int len, h264_sps_extradata_t *h264_sps)
{
    struct nal_h264_sps aasps;
    char profilestr[64];
    if (nal_h264_read_sps(&aasps, buf, len) >= 0)
    {
        h264_sps->profileLevelId = aasps.profile_idc;
        h264_sps->profile = aasps.profile_idc;
        h264_sps->level = aasps.level_idc;
        h264_sps->vidsize.width = (aasps.pic_width_in_mbs_minus1 + 1) * 16;
        h264_sps->vidsize.height = (aasps.pic_height_in_map_units_minus1 + 1) * 16 * (2 - aasps.frame_mbs_only_flag);
        if (aasps.frame_cropping_flag)
        {
            h264_sps->vidsize.width -= 2 * (aasps.crop_left + aasps.crop_right);
            if (aasps.frame_mbs_only_flag)
                h264_sps->vidsize.height -= 2 * (aasps.crop_top + aasps.crop_bottom);
            else
                h264_sps->vidsize.height -= 4 * (aasps.crop_top + aasps.crop_bottom);
        }
        if(aasps.vui.num_units_in_tick)
            h264_sps->fps = aasps.vui.time_scale / aasps.vui.num_units_in_tick;
        if (aasps.vui.fixed_frame_rate_flag == 0)
            h264_sps->fps /= 2;
        if(h264_sps->fps > 60)
            h264_sps->fps = 30;
        if(h264_sps->fps < 1)
            h264_sps->fps = 16;
        sps_get_profile(h264_sps->profile, profilestr);
        printf("H.264 SPS: -> video size %dx%d, fps %d profile %s(%d) level=%d\r\n",
           h264_sps->vidsize.width, h264_sps->vidsize.height, h264_sps->fps, profilestr, h264_sps->profile, h264_sps->level);    
        return OK;
    }
    return ERROR;
}
#if 0
typedef struct {
 
    zpl_uint8 *data;
    zpl_uint8 *data_end;
    zpl_uint32 cache;
    zpl_uint32 cache_bits;
 
} br_state;
 
#define BR_INIT(data,bytes) {(data), (data)+(bytes), 0, 0}
/*
static void br_init(br_state *br, const zpl_uint8 *data, int bytes)
{
    br->data = data;
    br->data_end = data + bytes;
    br->cache = 0;
    br->cache_bits = 0;
}
*/
#define BR_GET_BYTE(br) (br->data < br->data_end ? *br->data++ : 0xff)
#define BR_EOF(br) ((br)->data >= (br)->data_end)

static zpl_uint32 br_get_bits(br_state *br, zpl_uint32 n)
{
    if (n > 24)
        return (br_get_bits(br, 16) << 16) | br_get_bits(br, n - 16);
    while (br->cache_bits < 24)
    {
        br->cache = (br->cache << 8) | BR_GET_BYTE(br);
        br->cache_bits += 8;
    }
    br->cache_bits -= n;
    return (br->cache >> br->cache_bits) & ((1 << n) - 1);
}

static int br_get_bit(br_state *br)
{
    if (!br->cache_bits)
    {
        br->cache = BR_GET_BYTE(br);
        br->cache_bits = 7;
    }
    else
    {
        br->cache_bits--;
    }
    return (br->cache >> br->cache_bits) & 1;
}

static void br_skip_bit(br_state *br)
{
    if (!br->cache_bits)
    {
        br->cache = BR_GET_BYTE(br);
        br->cache_bits = 7;
    }
    else
    {
        br->cache_bits--;
    }
}

static void br_skip_bits(br_state *br, int n)
{
    if (br->cache_bits >= n)
    {
        br->cache_bits -= n;
    }
    else
    {
        /* drop cached bits */
        n -= br->cache_bits;
        /* drop full bytes */
        br->data += (n >> 3);
        n &= 7;
        /* update cache */
        if (n)
        {
            br->cache = BR_GET_BYTE(br);
            br->cache_bits = 8 - n;
        }
        else
        {
            br->cache_bits = 0;
        }
    }
}



#define br_get_u8(br) br_get_bits(br, 8)
#define br_get_u16(br) ((br_get_bits(br, 8) << 8) | br_get_bits(br, 8))
#define br_get_u32(br) ((br_get_bits(br, 8) << 24) | (br_get_bits(br, 8) << 16) | (br_get_bits(br, 8) << 8) | br_get_bits(br, 8))

static zpl_uint32 br_get_ue_golomb(br_state *br)
{
    int n = 0;
    while (!br_get_bit(br) && n < 32)
        n++;
    return n ? ((1 << n) - 1) + br_get_bits(br, n) : 0;
}

#pragma warning(disable : 4146)

static int32_t br_get_se_golomb(br_state *br)
{
    zpl_uint32 r = br_get_ue_golomb(br) + 1;
    return (r & 1) ? -(r >> 1) : (r >> 1);
}

static void br_skip_golomb(br_state *br)
{
    int n = 0;
    while (!br_get_bit(br) && n < 32)
        n++;
    br_skip_bits(br, n);
}
static int32_t br_skip_expgolomb(br_state *br)
{
    int n = 0;
    unsigned codeStart = 1;
    while (!br_get_bit(br) && n < 32)
    {
        ++n;
        codeStart *= 2;
    }
    return codeStart - 1 + br_get_bits(br, n);
}
#define br_skip_ue_golomb(br) br_skip_golomb(br)
#define br_skip_se_golomb(br) br_skip_golomb(br)




int zpl_media_channel_decode_sps(const zpl_uint8 *buf, int len, int *width,int *height,int *fps)
{
    int profile_idc = 0, pic_order_cnt_type = 0;
    int frame_mbs_only = 0;
    int i = 0, j = 0;
    zpl_h264_sps_data_t sps;
    struct nal_h264_sps aasps;
    char profilestr[64];

    // find sps
    if(len <= 0)
        return -1;
    bool findSPS = false;
    nal_h264_read_sps(&aasps, buf, len);
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0)
    {
        if ((buf[4] & 0x1f) == 7)
        { // start code 0 0 0 1
            len -= 5;
            buf += 5;
            findSPS = true;
        }
    }
    else if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
    { // start code 0 0 1
        if ((buf[3] & 0x1f) == 7)
        {
            len -= 4;
            buf += 4;
            findSPS = true;
        }
    }
    else
    {
        if ((buf[0] & 0x1f) == 7)
        { // no start code 0x67 开头
            len -= 1;
            buf += 1;
            findSPS = true;
        }
    }
    if(findSPS == false)
        return -1;

    br_state br = BR_INIT(buf, len);
    profile_idc = br_get_u8(&br);
    sps.profile = profile_idc;
    /* constraint_set0_flag = br_get_bit(br);    */
    /* constraint_set1_flag = br_get_bit(br);    */
    /* constraint_set2_flag = br_get_bit(br);    */
    /* constraint_set3_flag = br_get_bit(br);    */
    /* reserved             = br_get_bits(br,4); */
    br_skip_bits(&br, 8);
    sps.level = br_get_u8(&br);
    sps_get_profile(profile_idc, profilestr);
    printf("H.264 SPS: profile_idc %s level %d\r\n", profilestr, sps.level);
    //br_skip_bits(&br, 8);
    br_skip_ue_golomb(&br); /* seq_parameter_set_id */
    if (profile_idc >= 100)
    {
        if (br_get_ue_golomb(&br) == 3) /* chroma_format_idc      */
            br_skip_bit(&br);           /* residual_colour_transform_flag */
        br_skip_ue_golomb(&br);         /* bit_depth_luma - 8             */
        br_skip_ue_golomb(&br);         /* bit_depth_chroma - 8           */
        br_skip_bit(&br);               /* transform_bypass               */
        if (br_get_bit(&br))            /* seq_scaling_matrix_present     */
            for (i = 0; i < 8; i++)
                if (br_get_bit(&br))
                {
                    /* seq_scaling_list_present    */
                    int last = 8, next = 8, size = (i < 6) ? 16 : 64;
                    for (j = 0; j < size; j++)
                    {
                        if (next)
                            next = (last + br_get_se_golomb(&br)) & 0xff;
                        last = next ? next : last;
                    }
                }
    }

    br_skip_ue_golomb(&br); /* log2_max_frame_num - 4 */
    pic_order_cnt_type = br_get_ue_golomb(&br);
    if (pic_order_cnt_type == 0)
        br_skip_ue_golomb(&br); /* log2_max_poc_lsb - 4 */
    else if (pic_order_cnt_type == 1)
    {
        br_skip_bit(&br);          /* delta_pic_order_always_zero     */
        br_skip_se_golomb(&br);    /* offset_for_non_ref_pic          */
        br_skip_se_golomb(&br);    /* offset_for_top_to_bottom_field  */
        j = br_get_ue_golomb(&br); /* num_ref_frames_in_pic_order_cnt_cycle */
        for (i = 0; i < j; i++)
            br_skip_se_golomb(&br); /* offset_for_ref_frame[i]         */
    }
    br_skip_ue_golomb(&br); /* ref_frames                      */
    br_skip_bit(&br);       /* gaps_in_frame_num_allowed       */
    sps.vidsize.width /* mbs */ = br_get_ue_golomb(&br) + 1;
    sps.vidsize.height /* mbs */ = br_get_ue_golomb(&br) + 1;
    frame_mbs_only = br_get_bit(&br);
    printf("H.264 SPS: pic_width:  %u mbs\r\n", (zpl_uint32)sps.vidsize.width);
    printf("H.264 SPS: pic_height: %u mbs\r\n", (zpl_uint32)sps.vidsize.height);
    printf("H.264 SPS: frame only flag: %d\r\n", frame_mbs_only);

    sps.vidsize.width *= 16;
    sps.vidsize.height *= 16 * (2 - frame_mbs_only);

    if (!frame_mbs_only)
        if (br_get_bit(&br)) /* mb_adaptive_frame_field_flag */
            printf("H.264 SPS: MBAFF\r\n");
    br_skip_bit(&br); /* direct_8x8_inference_flag    */
    if (br_get_bit(&br))
    {
        /* frame_cropping_flag */
        zpl_uint32 crop_left = br_get_ue_golomb(&br);
        zpl_uint32 crop_right = br_get_ue_golomb(&br);
        zpl_uint32 crop_top = br_get_ue_golomb(&br);
        zpl_uint32 crop_bottom = br_get_ue_golomb(&br);
        printf("H.264 SPS: cropping %d %d %d %d\r\n",
               crop_left, crop_top, crop_right, crop_bottom);

        sps.vidsize.width -= 2 * (crop_left + crop_right);
        if (frame_mbs_only)
            sps.vidsize.height -= 2 * (crop_top + crop_bottom);
        else
            sps.vidsize.height -= 4 * (crop_top + crop_bottom);
    }
    /* VUI parameters */
    sps.pixel_aspect.num = 0;
    if (br_get_bit(&br))
    {
        /* vui_parameters_present flag */
        if (br_get_bit(&br))
        {
            /* aspect_ratio_info_present */
            zpl_uint32 aspect_ratio_idc = br_get_u8(&br);
            printf("H.264 SPS: aspect_ratio_idc %d\r\n", aspect_ratio_idc);

            if (aspect_ratio_idc == 255 /* Extended_SAR */)
            {
                sps.pixel_aspect.num = br_get_u16(&br); /* sar_width */
                sps.pixel_aspect.den = br_get_u16(&br); /* sar_height */
                printf("H.264 SPS: -> sar %dx%d\r\n", sps.pixel_aspect.num, sps.pixel_aspect.den);
            }
            else
            {
                static const zpl_mpeg_rational_t aspect_ratios[] =
                    {
                        /* page 213: */
                        /* 0: unknown */
                        {0, 1},
                        /* 1...16: */
                        {1, 1},
                        {12, 11},
                        {10, 11},
                        {16, 11},
                        {40, 33},
                        {24, 11},
                        {20, 11},
                        {32, 11},
                        {80, 33},
                        {18, 11},
                        {15, 11},
                        {64, 33},
                        {160, 99},
                        {4, 3},
                        {3, 2},
                        {2, 1}
                    };

                if (aspect_ratio_idc < sizeof(aspect_ratios) / sizeof(aspect_ratios[0]))
                {
                    memcpy(&sps.pixel_aspect, &aspect_ratios[aspect_ratio_idc], sizeof(zpl_mpeg_rational_t));
                    printf("H.264 SPS: -> aspect ratio %d / %d\r\n", sps.pixel_aspect.num, sps.pixel_aspect.den);
                }
                else
                {
                    printf("H.264 SPS: aspect_ratio_idc out of range !\r\n");
                }
            }
        }
           //overscan_info_present_flag;
            if (br_get_bit(&br))
            {
                br_skip_bit(&br);
                zpl_uint32 overscan_appropriate_flagu = br_get_u8(&br);
                printf("H.264 SPS: overscan_appropriate_flagu = %d\r\n", overscan_appropriate_flagu);
            }
            zpl_uint32 video_signal_type_present_flag = br_get_bit(&br);
            if(video_signal_type_present_flag)
            {
                br_skip_bits(&br, 4);
                //zpl_uint32 video_format=sps_get_u(3,buf,&StartBit);
                //zpl_uint32 video_full_range_flag=sps_get_u(1,buf,&StartBit);
                zpl_uint32 colour_description_present_flag = br_get_bit(&br);
                if(colour_description_present_flag)
                {
                    br_skip_bits(&br, 24);
                    //zpl_uint32 colour_primaries=sps_get_u(8,buf,&StartBit);
                    //zpl_uint32 transfer_characteristics=sps_get_u(8,buf,&StartBit);
                    //zpl_uint32 matrix_coefficients=sps_get_u(8,buf,&StartBit);
                }
            }
            zpl_uint32 chroma_loc_info_present_flag = br_get_bit(&br);
            if(chroma_loc_info_present_flag)
            {
                br_skip_expgolomb(&br);
                br_skip_expgolomb(&br);
                //zpl_uint32 chroma_sample_loc_type_top_field=sps_get_ue(buf,nLen,&StartBit);
                //zpl_uint32 chroma_sample_loc_type_bottom_field=sps_get_ue(buf,nLen,&StartBit);
            }
            zpl_uint32 timing_info_present_flag = br_get_bit(&br);

            if(timing_info_present_flag)
            {
                zpl_uint32 num_units_in_tick = br_get_u32(&br);
                zpl_uint32 time_scale = br_get_u32(&br);
                if(num_units_in_tick > 0)
                    *fps=time_scale/num_units_in_tick;
                zpl_uint32 fixed_frame_rate_flag = br_get_bit(&br);
                if(fixed_frame_rate_flag)
                {
                    *fps=(*fps)/2;
                }
                printf("H.264 SPS: -> time_scale %d, num_units_in_tick :%d fps=%d\r\n",
                    time_scale, num_units_in_tick, *fps);
            }
    }
    //sps.vidsize.width = width;
    //sps.vidsize.height = height;
    if(width)
        *width = sps.vidsize.width;
    if(height)
        *height = sps.vidsize.height;
    if(fps)
        *fps = 25;

    printf("H.264 SPS: -> video size %dx%d, aspect %d:%d\r\n",
           sps.vidsize.width, sps.vidsize.height, sps.pixel_aspect.num, sps.pixel_aspect.den);
    return 1;
}
#endif




       
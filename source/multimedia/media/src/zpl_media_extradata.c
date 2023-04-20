/*
 * zpl_media_extradata.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "zpl_media.h"
#include "zpl_media_internal.h"
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
                zpl_media_channel_decode_sps(nalu->buf, nalu->len,  &extradata->h264spspps);
                extradata->h264spspps.profileLevelId = zpl_media_channel_get_profileLevelId(nalu->buf + nalu->hdr_len, nalu->len - nalu->hdr_len);
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
                    zpl_media_channel_decode_sps(nalu->buf, nalu->len,  &extradata->h264spspps);  
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
            break;
            case 19:
            zlog_debug(MODULE_MEDIA, "-------- NALU Table ------+---------+");
            zlog_debug(MODULE_MEDIA, "       IDC |  TYPE |   LEN   |");
            zlog_debug(MODULE_MEDIA, "---------+--------+-------+---------+");
            zlog_debug(MODULE_MEDIA, " %7s| %6s| %8d|", idc_str, type_str, nalu->len-nalu->hdr_len);
            break;
        }
    }
    return 0;
}

/*
unsigned removeH264or5EmulationBytes(u_int8_t* to, unsigned toMaxSize,
                                     u_int8_t const* from, unsigned fromSize) {
  unsigned toSize = 0;
  unsigned i = 0;
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
*/
zpl_int32 zpl_media_channel_get_profileLevelId(zpl_uint8 const* from, zpl_uint32 fromSize) 
{
    zpl_uint8 to[1024];
    zpl_uint32 toMaxSize = 1024;
    zpl_uint32 toSize = 0;
    u_int32_t profileLevelId = 0;
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
    if(toSize >=4)
        profileLevelId = (to[1]<<16) | (to[2]<<8) | to[3];
    return profileLevelId;
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
        //if(chn->media_param.video_media.extradata.h264spspps.profileLevelId && extradata->h264spspps.profileLevelId)
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
    memset(&aasps, 0, sizeof(struct nal_h264_sps));
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
        zlog_debug(MODULE_MEDIA, "H.264 SPS: -> video size %dx%d, fps %d profile %s(%d) level=%d\r\n",
           h264_sps->vidsize.width, h264_sps->vidsize.height, h264_sps->fps, profilestr, h264_sps->profile, h264_sps->level);    
        
        fprintf(stdout, "===========H.264 SPS: -> video size %dx%d, fps %d profile %s(%d) level=%d profileLevelId=%x\r\n",
           h264_sps->vidsize.width, h264_sps->vidsize.height, h264_sps->fps, profilestr, h264_sps->profile, 
           h264_sps->level, zpl_media_channel_get_profileLevelId(buf+4, len-4)); 
        
        return OK;
    }
    return ERROR;
}


static const zpl_uint8 packet_bytes[] = {
  0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x2a, 0x96, 0x35, 0x40, 0xf0,
  0x04, 0x4f, 0xcb, 0x37, 0x01, 0x01, 0x01, 0x02
};

int zpl_media_sps_test(void)
{
    h264_sps_extradata_t h264_sps;
    zpl_media_channel_decode_sps(packet_bytes, sizeof(packet_bytes), &h264_sps);
    return 0;
}       
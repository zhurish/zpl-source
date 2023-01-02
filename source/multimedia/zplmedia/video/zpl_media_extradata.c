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
                    memcpy(extradata->fSEI, nalu->buf + nalu->hdr_len, nalu->len);
                }
            }
#else
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                extradata->fSEISize = nalu->len;
                memcpy(extradata->fSEI, nalu->buf + nalu->hdr_len, nalu->len);
            }
#endif
            break;
        case NALU_TYPE_SPS:
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                memset(tmp, 0, sizeof(tmp));
                memcpy(extradata->fSPS, nalu->buf+nalu->hdr_len, nalu->len);
                extradata->fSPSSize = nalu->len;

                if(zpl_media_channel_get_profileLevelId(nalu->buf+nalu->hdr_len, nalu->len,
                                                            tmp, sizeof(tmp)) >= 4)
                extradata->profileLevelId = (tmp[1]<<16) | (tmp[2]<<8) | tmp[3];
            }
#else        
            if (extradata->fSPS == NULL)
            {
                extradata->fSPS = malloc(nalu->len);
                if (extradata->fSPS)
                {
                    memset(tmp, 0, sizeof(tmp));
                    memcpy(extradata->fSPS, nalu->buf+nalu->hdr_len, nalu->len);
                    extradata->fSPSSize = nalu->len;

                    if(zpl_media_channel_get_profileLevelId(nalu->buf+nalu->hdr_len, nalu->len,
                                                            tmp, sizeof(tmp)) >= 4)
                        extradata->profileLevelId = (tmp[1]<<16) | (tmp[2]<<8) | tmp[3];
                }
            }
#endif
            break;
        case NALU_TYPE_PPS:
#ifdef ZPL_VIDEO_EXTRADATA_MAXSIZE
            if (nalu->len <= ZPL_VIDEO_EXTRADATA_MAXSIZE)
            {
                extradata->fPPSSize = nalu->len;
                memcpy(extradata->fPPS, nalu->buf + nalu->hdr_len, nalu->len);
            }
#else
            if (extradata->fPPS == NULL)
            {
                extradata->fPPS = malloc(nalu->len);
                if (extradata->fPPS)
                {
                    extradata->fPPSSize = nalu->len;
                    memcpy(extradata->fPPS, nalu->buf + nalu->hdr_len, nalu->len);
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
        /*
        zlog_debug(MODULE_ZPLMEDIA, "-------- NALU Table ------+---------+");
        zlog_debug(MODULE_ZPLMEDIA, "       IDC |  TYPE |   LEN   |");
        zlog_debug(MODULE_ZPLMEDIA, "---------+--------+-------+---------+");
        zlog_debug(MODULE_ZPLMEDIA, " %7s| %6s| %8d|", idc_str, type_str, nalu->len-nalu->hdr_len);
        */
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
    if(chn && chn->video_media.extradata.fSPS != NULL && 
        chn->video_media.extradata.fPPS != NULL /*&& 
        chn->video_media.extradata.fSEI != NULL*/)
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
                    zpl_media_channel_nalu2extradata(&nalu, &chn->video_media.extradata);
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
                    zpl_media_channel_nalu2extradata(&nalu, &chn->video_media.extradata);
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
    if(chn && (chn->video_media.halparam))
    {
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE         
        if(chn->video_media.extradata.fPPS && extradata->fPPS)
 #endif
        {
			n++;
            memcpy(extradata->fPPS, chn->video_media.extradata.fPPS, chn->video_media.extradata.fPPSSize);
            extradata->fPPSSize = chn->video_media.extradata.fPPSSize;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->video_media.extradata.fVPS && extradata->fVPS)
 #endif
        {
			n++;
            memcpy(extradata->fVPS, chn->video_media.extradata.fVPS, chn->video_media.extradata.fVPSSize);
            extradata->fVPSSize = chn->video_media.extradata.fVPSSize;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->video_media.extradata.fSPS && extradata->fSPS)
 #endif
        {
			n++;
            memcpy(extradata->fSPS, chn->video_media.extradata.fSPS, chn->video_media.extradata.fSPSSize);
            extradata->fSPSSize = chn->video_media.extradata.fSPSSize;
        }
#ifndef ZPL_VIDEO_EXTRADATA_MAXSIZE 
        if(chn->video_media.extradata.fSEI && extradata->fSEI)
 #endif
        {
			n++;
            memcpy(extradata->fSEI, chn->video_media.extradata.fSEI, chn->video_media.extradata.fSEISize);
            extradata->fSEISize = chn->video_media.extradata.fSEISize;
        }
        if(chn->video_media.extradata.profileLevelId && extradata->profileLevelId)
        {
			n++;
            extradata->profileLevelId = chn->video_media.extradata.profileLevelId;
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
        if(extradata->profileLevelId)
        {
            extradata->profileLevelId = 0;
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

static zpl_uint32 sps_get_ue(zpl_uint8 *pBuff, zpl_uint32 nLen, zpl_uint32 *nStartBit)
{
    zpl_uint32 nZeroNum = 0;
    zpl_uint32 i = 0;
    zpl_uint32 dwRet = 0;
    while (*nStartBit < nLen * 8)
    {
        if (pBuff[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
        {
            break;
        }
        nZeroNum++;
        (*nStartBit)++;
    }
    (*nStartBit)++;

    
    for ( i=0; i<nZeroNum; i++)
    {
        dwRet <<= 1;
        if (pBuff[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
        {
            dwRet += 1;
        }
        (*nStartBit)++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}

static int sps_get_se(zpl_uint8 *pBuff, zpl_uint32 nLen, zpl_uint32 *nStartBit)
{
    zpl_uint32 UeVal=sps_get_ue(pBuff,nLen,nStartBit);
    double k=UeVal;
    int nValue=ceil(k/2);
    if (UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}


static zpl_uint32 sps_get_u(zpl_uint32 BitCount,zpl_uint8 * buf,zpl_uint32 *nStartBit)
{
    zpl_uint32 dwRet = 0;
    zpl_uint32 i=0;
    for ( i=0; i<BitCount; i++)
    {
        dwRet <<= 1;
        if (buf[*nStartBit / 8] & (0x80 >> (*nStartBit % 8)))
        {
            dwRet += 1;
        }
        (*nStartBit)++;
    }
    return dwRet;
}

static void de_emulation_prevention(zpl_uint8* buf, zpl_uint32* buf_size)
{
    zpl_uint32 i=0,j=0;
    zpl_uint8* tmp_ptr=NULL;
    zpl_uint32 tmp_buf_size=0;
    int val=0;

    tmp_ptr=buf;
    tmp_buf_size=*buf_size;

    fprintf(stdout, " tmp_buf_size = %d\r\n", tmp_buf_size);
    fflush(stdout);

    for(i=0;i<(tmp_buf_size-2);i++)
    {
        //check for 0x000003
        val=(tmp_ptr[i]^0x00) +(tmp_ptr[i+1]^0x00)+(tmp_ptr[i+2]^0x03);
        if(val==0)
        {
            //kick out 0x03
            for(j=i+2;j<tmp_buf_size-1;j++)
                tmp_ptr[j]=tmp_ptr[j+1];
            //and so we should devrease bufsize
            (*buf_size)--;
        }
    }
}

static bool h264_decode_sps(zpl_uint8 * buf, zpl_uint32 nLen, int *width,int *height,int *fps)
{
    zpl_uint32 StartBit=0;
    zpl_uint32 dddnLen = nLen, i = 0;
    *fps=0;
    de_emulation_prevention(buf,&dddnLen);
    zpl_uint32 residual_colour_transform_flag=0;
    zpl_uint32 log2_max_pic_order_cnt_lsb_minus4=0;
    zpl_uint32 mb_adaptive_frame_field_flag=0;
    zpl_uint32 overscan_appropriate_flagu=0;
    zpl_uint32 timing_info_present_flag = 0;
    //zpl_uint32 forbidden_zero_bit=sps_get_u(1,buf,&StartBit);
    //zpl_uint32 nal_ref_idc=sps_get_u(2,buf,&StartBit);
    zpl_uint32 nal_unit_type=sps_get_u(5,buf,&StartBit);
    if(nal_unit_type==7)
    {
        zpl_uint32 profile_idc=sps_get_u(8,buf,&StartBit);
        //zpl_uint32 constraint_set0_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x80)>>7;
        //zpl_uint32 constraint_set1_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x40)>>6;
        //zpl_uint32 constraint_set2_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x20)>>5;
        //zpl_uint32 constraint_set3_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x10)>>4;
        //zpl_uint32 reserved_zero_4bits=sps_get_u(4,buf,&StartBit);
        //zpl_uint32 level_idc=sps_get_u(8,buf,&StartBit);

        //zpl_uint32 seq_parameter_set_id=sps_get_ue(buf,nLen,&StartBit);

        if( profile_idc == 100 || profile_idc == 110 ||
                profile_idc == 122 || profile_idc == 144 )
        {
            zpl_uint32 chroma_format_idc=sps_get_ue(buf,nLen,&StartBit);
            if( chroma_format_idc == 3 )
                residual_colour_transform_flag=sps_get_u(1,buf,&StartBit);
            //zpl_uint32 bit_depth_luma_minus8=sps_get_ue(buf,nLen,&StartBit);
            //zpl_uint32 bit_depth_chroma_minus8=sps_get_ue(buf,nLen,&StartBit);
            //zpl_uint32 qpprime_y_zero_transform_bypass_flag=sps_get_u(1,buf,&StartBit);
            zpl_uint32 seq_scaling_matrix_present_flag=sps_get_u(1,buf,&StartBit);

            zpl_uint32 seq_scaling_list_present_flag[8];
            if( seq_scaling_matrix_present_flag )
            {
                for(  i = 0; i < 8; i++ ) {
                    seq_scaling_list_present_flag[i]=sps_get_u(1,buf,&StartBit);
                }
            }
        }
        //zpl_uint32 log2_max_frame_num_minus4=sps_get_ue(buf,nLen,&StartBit);
        zpl_uint32 pic_order_cnt_type=sps_get_ue(buf,nLen,&StartBit);
        if( pic_order_cnt_type == 0 )
            log2_max_pic_order_cnt_lsb_minus4=sps_get_ue(buf,nLen,&StartBit);
        else if( pic_order_cnt_type == 1 )
        {
            //zpl_uint32 delta_pic_order_always_zero_flag=sps_get_u(1,buf,&StartBit);
            //int offset_for_non_ref_pic=sps_get_se(buf,nLen,&StartBit);
            //int offset_for_top_to_bottom_field=sps_get_se(buf,nLen,&StartBit);
            zpl_uint32 num_ref_frames_in_pic_order_cnt_cycle=sps_get_ue(buf,nLen,&StartBit);

            int *offset_for_ref_frame=malloc(4*num_ref_frames_in_pic_order_cnt_cycle);
            for(  i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
                offset_for_ref_frame[i]=sps_get_se(buf,nLen,&StartBit);
            free(offset_for_ref_frame);
        }
        //zpl_uint32 num_ref_frames=sps_get_ue(buf,nLen,&StartBit);
        //zpl_uint32 gaps_in_frame_num_value_allowed_flag=sps_get_u(1,buf,&StartBit);
        zpl_uint32 pic_width_in_mbs_minus1=sps_get_ue(buf,nLen,&StartBit);
        zpl_uint32 pic_height_in_map_units_minus1=sps_get_ue(buf,nLen,&StartBit);

        *width=(pic_width_in_mbs_minus1+1)*16;
        *height=(pic_height_in_map_units_minus1+1)*16;

        zpl_uint32 frame_mbs_only_flag=sps_get_u(1,buf,&StartBit);
        if(!frame_mbs_only_flag)
            mb_adaptive_frame_field_flag=sps_get_u(1,buf,&StartBit);

        //zpl_uint32 direct_8x8_inference_flag=sps_get_u(1,buf,&StartBit);
        zpl_uint32 frame_cropping_flag=sps_get_u(1,buf,&StartBit);
        if(frame_cropping_flag)
        {
            /*
            zpl_uint32 frame_crop_left_offset=sps_get_ue(buf,nLen,&StartBit);
            zpl_uint32 frame_crop_right_offset=sps_get_ue(buf,nLen,&StartBit);
            zpl_uint32 frame_crop_top_offset=sps_get_ue(buf,nLen,&StartBit);
            zpl_uint32 frame_crop_bottom_offset=sps_get_ue(buf,nLen,&StartBit);*/
        }
        zpl_uint32 vui_parameter_present_flag=sps_get_u(1,buf,&StartBit);
        if(vui_parameter_present_flag)
        {
            zpl_uint32 aspect_ratio_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(aspect_ratio_info_present_flag)
            {
                zpl_uint32 aspect_ratio_idc=sps_get_u(8,buf,&StartBit);
                if(aspect_ratio_idc==255)
                {
                    //zpl_uint32 sar_width=sps_get_u(16,buf,&StartBit);
                    //zpl_uint32 sar_height=sps_get_u(16,buf,&StartBit);
                }
            }
            zpl_uint32 overscan_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(overscan_info_present_flag)
                overscan_appropriate_flagu=sps_get_u(1,buf,&StartBit);
            zpl_uint32 video_signal_type_present_flag=sps_get_u(1,buf,&StartBit);
            if(video_signal_type_present_flag)
            {
                //zpl_uint32 video_format=sps_get_u(3,buf,&StartBit);
                //zpl_uint32 video_full_range_flag=sps_get_u(1,buf,&StartBit);
                zpl_uint32 colour_description_present_flag=sps_get_u(1,buf,&StartBit);
                if(colour_description_present_flag)
                {
                    //zpl_uint32 colour_primaries=sps_get_u(8,buf,&StartBit);
                    //zpl_uint32 transfer_characteristics=sps_get_u(8,buf,&StartBit);
                    //zpl_uint32 matrix_coefficients=sps_get_u(8,buf,&StartBit);
                }
            }
            zpl_uint32 chroma_loc_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(chroma_loc_info_present_flag)
            {
                //zpl_uint32 chroma_sample_loc_type_top_field=sps_get_ue(buf,nLen,&StartBit);
                //zpl_uint32 chroma_sample_loc_type_bottom_field=sps_get_ue(buf,nLen,&StartBit);
            }
            timing_info_present_flag=sps_get_u(1,buf,&StartBit);

            if(timing_info_present_flag)
            {
                zpl_uint32 num_units_in_tick=sps_get_u(32,buf,&StartBit);
                zpl_uint32 time_scale=sps_get_u(32,buf,&StartBit);
                *fps=time_scale/num_units_in_tick;
                zpl_uint32 fixed_frame_rate_flag=sps_get_u(1,buf,&StartBit);
                if(fixed_frame_rate_flag)
                {
                    *fps=(*fps)/2;
                }
            }
        }

        char profile_str[32] = {0};
        sps_get_profile(profile_idc, &profile_str[0]);

        if(timing_info_present_flag){
            fprintf(stdout,"H.264 SPS: -> video size %dx%d, %d fps, profile(%d) %s\n",
                    *width, *height, *fps, profile_idc, profile_str);
        } else {
            fprintf(stdout,"H.264 SPS: -> video size %dx%d, unknown fps, profile(%d) %s\n",
                    *width, *height, profile_idc, profile_str);
        }
        fflush(stdout);
        return true;
    }
    else
        return false;
}


int zpl_media_channel_decode_spspps(zpl_uint8 *bufdata, zpl_uint32 nLen,int *width,int *height,int *fps)
{
    fprintf(stdout, " 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
            bufdata[0],bufdata[1],bufdata[2],bufdata[3],bufdata[4],bufdata[5]);
    fprintf(stdout, " nLen = %d\r\n", nLen);
    fflush(stdout);
    h264_decode_sps(bufdata,  nLen, width, height, fps);
    return 0;
}


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

#define br_skip_ue_golomb(br) br_skip_golomb(br)
#define br_skip_se_golomb(br) br_skip_golomb(br)

int zpl_media_channel_decode_sps(const zpl_uint8 *buf, int len, int *width,int *height,int *fps)
{
    int profile_idc = 0, pic_order_cnt_type = 0;
    int frame_mbs_only = 0;
    int i = 0, j = 0;
    zpl_h264_sps_data_t sps;
    // find sps
    if(len <= 0)
        return -1;
    bool findSPS = false;
    if (buf[2] == 0)
    {
        if ((buf[4] & 0x1f) == 7)
        { // start code 0 0 0 1
            len -= 5;
            buf += 5;
            findSPS = true;
        }
    }
    else if (buf[2] == 1)
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
    printf("H.264 SPS: profile_idc %d level %d\r\n", profile_idc, sps.level);
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
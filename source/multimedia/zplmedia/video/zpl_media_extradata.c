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

int zpl_media_channel_get_nextnalu(uint8_t *bufdata, uint32_t len)
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

unsigned zpl_media_channel_get_profileLevelId(uint8_t const* from, unsigned fromSize, uint8_t* to, unsigned toMaxSize) {
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

bool zpl_media_channel_isnaluhdr(uint8_t *bufdata, H264_NALU_T *nalu)
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
    if (extradata->fSPS != NULL && extradata->fPPS != NULL/* && extradata->fSEI != NULL*/)
    {
        //fprintf(stdout, " _zpl_media_file_get_extradata already\r\n");
        fflush(stdout);
        return 1;
    }
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
            if (extradata->fSEI == NULL)
            {
                extradata->fSEI = malloc(nalu->len);
                if (extradata->fSEI)
                {
                    extradata->fSEISize = nalu->len;
                    memcpy(extradata->fSEI, nalu->buf + nalu->hdr_len, nalu->len);
                }
            }
            break;
        case NALU_TYPE_SPS:
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
            break;
        case NALU_TYPE_PPS:
            if (extradata->fPPS == NULL)
            {
                extradata->fPPS = malloc(nalu->len);
                if (extradata->fPPS)
                {
                    extradata->fPPSSize = nalu->len;
                    memcpy(extradata->fPPS, nalu->buf + nalu->hdr_len, nalu->len);
                }
            }
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
        zlog_debug(MODULE_ZPLMEDIA, "-------- NALU Table ------+---------+");
        zlog_debug(MODULE_ZPLMEDIA, "       IDC |  TYPE |   LEN   |");
        zlog_debug(MODULE_ZPLMEDIA, "---------+--------+-------+---------+");
        zlog_debug(MODULE_ZPLMEDIA, " %7s| %6s| %8d|", idc_str, type_str, nalu->len-nalu->hdr_len);
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
    if(chn && chn->video_media.extradata.fSPS != NULL && 
        chn->video_media.extradata.fPPS != NULL /*&& 
        chn->video_media.extradata.fSEI != NULL*/)
        return 1;

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
        if(chn->video_media.extradata.fPPS && extradata->fPPS)
        {
			n++;
            memcpy(extradata->fPPS, chn->video_media.extradata.fPPS, chn->video_media.extradata.fPPSSize);
            extradata->fPPSSize = chn->video_media.extradata.fPPSSize;
        }
        if(chn->video_media.extradata.fVPS && extradata->fVPS)
        {
			n++;
            memcpy(extradata->fVPS, chn->video_media.extradata.fVPS, chn->video_media.extradata.fVPSSize);
            extradata->fVPSSize = chn->video_media.extradata.fVPSSize;
        }
        if(chn->video_media.extradata.fSPS && extradata->fSPS)
        {
			n++;
            memcpy(extradata->fSPS, chn->video_media.extradata.fSPS, chn->video_media.extradata.fSPSSize);
            extradata->fSPSSize = chn->video_media.extradata.fSPSSize;
        }
        if(chn->video_media.extradata.fSEI && extradata->fSEI)
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

static uint32_t sps_get_ue(uint8_t *pBuff, uint32_t nLen, uint32_t *nStartBit)
{
    uint32_t nZeroNum = 0;
    uint32_t i = 0;
    uint32_t dwRet = 0;
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

static int sps_get_se(uint8_t *pBuff, uint32_t nLen, uint32_t *nStartBit)
{
    uint32_t UeVal=sps_get_ue(pBuff,nLen,nStartBit);
    double k=UeVal;
    int nValue=ceil(k/2);
    if (UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}


static uint32_t sps_get_u(uint32_t BitCount,uint8_t * buf,uint32_t *nStartBit)
{
    uint32_t dwRet = 0;
    uint32_t i=0;
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

static void de_emulation_prevention(uint8_t* buf, uint32_t* buf_size)
{
    uint32_t i=0,j=0;
    uint8_t* tmp_ptr=NULL;
    uint32_t tmp_buf_size=0;
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

static bool h264_decode_sps(uint8_t * buf, uint32_t nLen, int *width,int *height,int *fps)
{
    uint32_t StartBit=0;
    uint32_t dddnLen = nLen, i = 0;
    *fps=0;
    de_emulation_prevention(buf,&dddnLen);
    uint32_t residual_colour_transform_flag=0;
    uint32_t log2_max_pic_order_cnt_lsb_minus4=0;
    uint32_t mb_adaptive_frame_field_flag=0;
    uint32_t overscan_appropriate_flagu=0;
    uint32_t timing_info_present_flag = 0;
    //uint32_t forbidden_zero_bit=sps_get_u(1,buf,&StartBit);
    //uint32_t nal_ref_idc=sps_get_u(2,buf,&StartBit);
    uint32_t nal_unit_type=sps_get_u(5,buf,&StartBit);
    if(nal_unit_type==7)
    {
        uint32_t profile_idc=sps_get_u(8,buf,&StartBit);
        //uint32_t constraint_set0_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x80)>>7;
        //uint32_t constraint_set1_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x40)>>6;
        //uint32_t constraint_set2_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x20)>>5;
        //uint32_t constraint_set3_flag=sps_get_u(1,buf,&StartBit);//(buf[1] & 0x10)>>4;
        //uint32_t reserved_zero_4bits=sps_get_u(4,buf,&StartBit);
        //uint32_t level_idc=sps_get_u(8,buf,&StartBit);

        //uint32_t seq_parameter_set_id=sps_get_ue(buf,nLen,&StartBit);

        if( profile_idc == 100 || profile_idc == 110 ||
                profile_idc == 122 || profile_idc == 144 )
        {
            uint32_t chroma_format_idc=sps_get_ue(buf,nLen,&StartBit);
            if( chroma_format_idc == 3 )
                residual_colour_transform_flag=sps_get_u(1,buf,&StartBit);
            //uint32_t bit_depth_luma_minus8=sps_get_ue(buf,nLen,&StartBit);
            //uint32_t bit_depth_chroma_minus8=sps_get_ue(buf,nLen,&StartBit);
            //uint32_t qpprime_y_zero_transform_bypass_flag=sps_get_u(1,buf,&StartBit);
            uint32_t seq_scaling_matrix_present_flag=sps_get_u(1,buf,&StartBit);

            uint32_t seq_scaling_list_present_flag[8];
            if( seq_scaling_matrix_present_flag )
            {
                for(  i = 0; i < 8; i++ ) {
                    seq_scaling_list_present_flag[i]=sps_get_u(1,buf,&StartBit);
                }
            }
        }
        //uint32_t log2_max_frame_num_minus4=sps_get_ue(buf,nLen,&StartBit);
        uint32_t pic_order_cnt_type=sps_get_ue(buf,nLen,&StartBit);
        if( pic_order_cnt_type == 0 )
            log2_max_pic_order_cnt_lsb_minus4=sps_get_ue(buf,nLen,&StartBit);
        else if( pic_order_cnt_type == 1 )
        {
            //uint32_t delta_pic_order_always_zero_flag=sps_get_u(1,buf,&StartBit);
            //int offset_for_non_ref_pic=sps_get_se(buf,nLen,&StartBit);
            //int offset_for_top_to_bottom_field=sps_get_se(buf,nLen,&StartBit);
            uint32_t num_ref_frames_in_pic_order_cnt_cycle=sps_get_ue(buf,nLen,&StartBit);

            int *offset_for_ref_frame=malloc(4*num_ref_frames_in_pic_order_cnt_cycle);
            for(  i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
                offset_for_ref_frame[i]=sps_get_se(buf,nLen,&StartBit);
            free(offset_for_ref_frame);
        }
        //uint32_t num_ref_frames=sps_get_ue(buf,nLen,&StartBit);
        //uint32_t gaps_in_frame_num_value_allowed_flag=sps_get_u(1,buf,&StartBit);
        uint32_t pic_width_in_mbs_minus1=sps_get_ue(buf,nLen,&StartBit);
        uint32_t pic_height_in_map_units_minus1=sps_get_ue(buf,nLen,&StartBit);

        *width=(pic_width_in_mbs_minus1+1)*16;
        *height=(pic_height_in_map_units_minus1+1)*16;

        uint32_t frame_mbs_only_flag=sps_get_u(1,buf,&StartBit);
        if(!frame_mbs_only_flag)
            mb_adaptive_frame_field_flag=sps_get_u(1,buf,&StartBit);

        //uint32_t direct_8x8_inference_flag=sps_get_u(1,buf,&StartBit);
        uint32_t frame_cropping_flag=sps_get_u(1,buf,&StartBit);
        if(frame_cropping_flag)
        {
            /*
            uint32_t frame_crop_left_offset=sps_get_ue(buf,nLen,&StartBit);
            uint32_t frame_crop_right_offset=sps_get_ue(buf,nLen,&StartBit);
            uint32_t frame_crop_top_offset=sps_get_ue(buf,nLen,&StartBit);
            uint32_t frame_crop_bottom_offset=sps_get_ue(buf,nLen,&StartBit);*/
        }
        uint32_t vui_parameter_present_flag=sps_get_u(1,buf,&StartBit);
        if(vui_parameter_present_flag)
        {
            uint32_t aspect_ratio_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(aspect_ratio_info_present_flag)
            {
                uint32_t aspect_ratio_idc=sps_get_u(8,buf,&StartBit);
                if(aspect_ratio_idc==255)
                {
                    //uint32_t sar_width=sps_get_u(16,buf,&StartBit);
                    //uint32_t sar_height=sps_get_u(16,buf,&StartBit);
                }
            }
            uint32_t overscan_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(overscan_info_present_flag)
                overscan_appropriate_flagu=sps_get_u(1,buf,&StartBit);
            uint32_t video_signal_type_present_flag=sps_get_u(1,buf,&StartBit);
            if(video_signal_type_present_flag)
            {
                //uint32_t video_format=sps_get_u(3,buf,&StartBit);
                //uint32_t video_full_range_flag=sps_get_u(1,buf,&StartBit);
                uint32_t colour_description_present_flag=sps_get_u(1,buf,&StartBit);
                if(colour_description_present_flag)
                {
                    //uint32_t colour_primaries=sps_get_u(8,buf,&StartBit);
                    //uint32_t transfer_characteristics=sps_get_u(8,buf,&StartBit);
                    //uint32_t matrix_coefficients=sps_get_u(8,buf,&StartBit);
                }
            }
            uint32_t chroma_loc_info_present_flag=sps_get_u(1,buf,&StartBit);
            if(chroma_loc_info_present_flag)
            {
                //uint32_t chroma_sample_loc_type_top_field=sps_get_ue(buf,nLen,&StartBit);
                //uint32_t chroma_sample_loc_type_bottom_field=sps_get_ue(buf,nLen,&StartBit);
            }
            timing_info_present_flag=sps_get_u(1,buf,&StartBit);

            if(timing_info_present_flag)
            {
                uint32_t num_units_in_tick=sps_get_u(32,buf,&StartBit);
                uint32_t time_scale=sps_get_u(32,buf,&StartBit);
                *fps=time_scale/num_units_in_tick;
                uint32_t fixed_frame_rate_flag=sps_get_u(1,buf,&StartBit);
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


int zpl_media_channel_decode_spspps(uint8_t *bufdata, uint32_t nLen,int *width,int *height,int *fps)
{
    fprintf(stdout, " 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
            bufdata[0],bufdata[1],bufdata[2],bufdata[3],bufdata[4],bufdata[5]);
    fprintf(stdout, " nLen = %d\r\n", nLen);
    fflush(stdout);
    h264_decode_sps(bufdata,  nLen, width, height, fps);
    return 0;
}

/**
 * @file      : rtmp-send_h264.c
 * @brief     : Description
 * @author    : zhurish (zhurish@163.com)
 * @version   : 1.0
 * @date      : 2024-03-14
 * 
 * @copyright : Copyright (c) - 2024 zhurish(zhurish@163.com).Co.Ltd. All rights reserved.
 * 
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "log.h"
#include "rtmpsys.h"
#include "rtmplog.h"
#include "rtmp-client-api.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
/**
 * 发送视频的sps和pps信息
 *
 * @param pps 存储视频的pps信息
 * @param pps_len 视频的pps信息长度
 * @param sps 存储视频的pps信息
 * @param sps_len 视频的sps信息长度
 *
 * @成功则返回 1 , 失败则返回0
 */
static int rtmp_send_h264_spspps(rtmp_client_t *rtmp, char *pps, int pps_len, char * sps, int sps_len)
{
	unsigned char  body[4096];
	int i;
	rtmp->_packet.m_body = (char *)body;
	//body = (unsigned char *)rtmp->_packet.m_body;
	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++]   = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++]   = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i],pps,pps_len);
	i +=  pps_len;

	rtmp->_packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
	rtmp->_packet.m_nBodySize = i;
	rtmp->_packet.m_nChannel = 0x04;
	rtmp->_packet.m_nTimeStamp = 0;
	rtmp->_packet.m_hasAbsTimestamp = 0;
	rtmp->_packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	rtmp->_packet.m_nInfoField2 = rtmp->_stream_id;

	/*调用发送接口*/
	int nRet = RTMP_SendPacket(&rtmp->_rtmp, &rtmp->_packet, TRUE);
	return nRet;
}

/**
 * 发送H264数据帧
 *
 * @param data 存储数据帧内容
 * @param size 数据帧的大小
 * @param bIsKeyFrame 记录该帧是否为关键帧
 * @param nTimeStamp 当前帧的时间戳
 *
 * @成功则返回 1 , 失败则返回0
 */
int rtmp_send_h264_packet(rtmp_client_t *rtmp, int nultype, char *data, int size, int nTimeStamp)  
{  
	if(data == NULL && size<11){  
		return -1;  
	}  
	char *body = data-9;//(unsigned char*)malloc(size+9);  
	int i = 0;
	switch(nultype) {
		case NALU_TYPE_SPS:
		case NALU_TYPE_PPS:
		{
			zpl_video_extradata_t extradata;
			memset(&extradata, 0, sizeof(zpl_video_extradata_t));
			if(zpl_media_channel_extradata_get(rtmp->media_channel, &extradata) == OK)
				return rtmp_send_h264_spspps(rtmp, extradata.fPPS + extradata.fPPSHdrLen, extradata.fPPSSize - extradata.fPPSHdrLen, 
					extradata.fSPS + extradata.fSPSHdrLen, extradata.fSPSSize - extradata.fSPSHdrLen);
		}
		break;
		case NALU_TYPE_IDR:
		body[i++] = 0x17;// 1:Iframe  7:AVC   
		body[i++] = 0x01;// 0:AVC 1:NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		break;
		default:
		body[i++] = 0x27;// 2:Pframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		break;
	}
	// NALU size   
	body[i++] = size>>24 &0xff;  
	body[i++] = size>>16 &0xff;  
	body[i++] = size>>8 &0xff;  
	body[i++] = size&0xff;
	// NALU data   
	//memcpy(&body[i],data,size);  
	return rtmp_client_send_packet(rtmp, RTMP_PACKET_TYPE_VIDEO, data - 9, i+size, nTimeStamp);
	//int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i+size, nTimeStamp);  
} 



#if 0

/**
 * 将内存中的一段H.264编码的视频数据利用RTMP协议发送到服务器
 *
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据。
 *					2个参数功能：
 *					uint8_t *buf：外部数据送至该地址
 *					int buf_size：外部数据大小
 *					返回值：成功读取的内存大小
 * @成功则返回1 , 失败则返回0
 */ 
int RTMP264_Send(int (*read_buffer)(unsigned char *buf, int buf_size))  
{    
	int ret;
	uint32_t now,last_update;
	  
	memset(&metaData,0,sizeof(RTMPMetadata));
	memset(m_pFileBuf,0,BUFFER_SIZE);
	if((ret=read_buffer(m_pFileBuf,m_nFileBufSize))<0)
	{
		return FALSE;
	}

	NaluUnit naluUnit;  
	// 读取SPS帧   
	ReadFirstNaluFromBuf(naluUnit,read_buffer);  
	metaData.nSpsLen = naluUnit.size;  
	metaData.Sps=NULL;
	metaData.Sps=(unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Sps,naluUnit.data,naluUnit.size);

	// 读取PPS帧   
	ReadOneNaluFromBuf(naluUnit,read_buffer);  
	metaData.nPpsLen = naluUnit.size; 
	metaData.Pps=NULL;
	metaData.Pps=(unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Pps,naluUnit.data,naluUnit.size);
	
	// 解码SPS,获取视频图像宽、高信息   
	int width = 0,height = 0, fps=0;  
	h264_decode_sps(metaData.Sps,metaData.nSpsLen,width,height,fps);  
	//metaData.nWidth = width;  
	//metaData.nHeight = height;  
	if(fps)
		metaData.nFrameRate = fps; 
	else
		metaData.nFrameRate = 25;

	//发送PPS,SPS
	//ret=SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	//if(ret!=1)
	//	return FALSE;

	unsigned int tick = 0;  
	unsigned int tick_gap = 1000/metaData.nFrameRate; 
	ReadOneNaluFromBuf(naluUnit,read_buffer);
	int bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
	while(SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick))  
	{    
got_sps_pps:
		//if(naluUnit.size==8581)
			printf("NALU size:%8d\n",naluUnit.size);
		last_update=RTMP_GetTime();
		if(!ReadOneNaluFromBuf(naluUnit,read_buffer))
				goto end;
		if(naluUnit.type == 0x07 || naluUnit.type == 0x08)
			goto got_sps_pps;
		bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
		tick +=tick_gap;
		now=RTMP_GetTime();
		msleep(tick_gap-now+last_update);  
		//msleep(40);
	}  
	end:
	free(metaData.Sps);
	free(metaData.Pps);
	return TRUE;  
}  



/*************************************************************/
static int rtmp_write_video_header_h264(unsigned char *sps, int sps_len,
										unsigned char *pps, int pps_len)
{//264: index[2] + cts[3] + nalu.len[4] + nalu.index[1] + nalu.data[n]
    RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	int ret = RTMPPacket_Alloc(&packet, 2 + 3 + 5 + 1 + 2 + sps_len + 1 + 2 + pps_len);
 
    packet.m_packetType 	= RTMP_PACKET_TYPE_VIDEO;
    packet.m_nBodySize  	= 2 + 3 + 5 + 1 + 2 + sps_len + 1 + 2 + pps_len;
    packet.m_nChannel   	= 0x04;
    packet.m_nTimeStamp 	= 0;
    packet.m_hasAbsTimestamp= 0;
    packet.m_headerType 	= RTMP_PACKET_SIZE_LARGE;
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
    packet.m_nInfoField2 	= rtmpctx->m_stream_id;
 
	unsigned char *body = (unsigned char *)packet.m_body;	
    int i = 0;	
    body[i++] = 0x17;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    //AVCDecoderConfigurationRecord
    body[i ++] = 0x01;
    body[i ++] = sps[1];
    body[i ++] = sps[2];
    body[i ++] = sps[3];
    body[i ++] = 0xff;
    //sps
    body[i ++] = 0xe1;
    body[i ++] = (sps_len >> 8) & 0xff;
    body[i ++] = (sps_len) & 0xff;
    memcpy(body + i, sps, sps_len); i += sps_len;
    
    //pps
    body[i ++] = 0x01;
    body[i ++] = (pps_len >> 8) & 0xff;
    body[i ++] = (pps_len) & 0xff;
    memcpy(body + i, pps, pps_len); i += pps_len;
	
	printf("rtmp_config_send: sps=%d, pps=%d\n", sps_len, pps_len);
    int nRet = RTMP_SendPacket(rtmpctx, &packet, 0);
	RTMPPacket_Free(&packet);
	return nRet;
}
static int rtmp_write_video_header_h265(unsigned char *vps, int vps_len,
										unsigned char *sps, int sps_len,
										unsigned char *pps, int pps_len)
{
    RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	int ret = RTMPPacket_Alloc(&packet, 43 + vps_len + sps_len + pps_len);
 
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
 
	unsigned char *body = (unsigned char *)packet.m_body;	
    int i = 0;	
    body[i++] = 0x1c;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    body[i ++] = 0x00;
    body[i ++] = 0x01;
 
	body[i++] = sps[6];
	body[i++] = sps[7];
	body[i++] = sps[8];
	body[i++] = sps[9];
 
	body[i++] = sps[12];
	body[i++] = sps[13];
	body[i++] = sps[14];
 
	//48 bit nothing deal in rtmp
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
 
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	body[i ++] = 0x00;
	
	//bit(16) avgFrameRate
	/* bit(2) constantFrameRate; */
	/* bit(3) numTemporalLayers; */
	/* bit(1) temporalIdNested; */
	body[i ++] = 0x83;
 
	/*unsigned int(8) numOfArrays; 03*/
	body[i ++] = 0x03;
 
	//vps 32
	body[i ++] = 0x20;
	body[i ++] = 0x00;
	body[i ++] = 0x01;
	body[i ++] = (vps_len >> 8) & 0xff;
	body[i ++] = (vps_len) & 0xff;
	memcpy(&body[i], vps, vps_len);
	i += vps_len;
	
    //sps
    body[i ++] = 0x21;
	body[i ++] = 0x00;
	body[i ++] = 0x01;
    body[i ++] = (sps_len >> 8) & 0xff;
    body[i ++] = (sps_len) & 0xff;
    memcpy(&body[i], sps, sps_len); 
	i += sps_len;
    
    //pps
    body[i ++] = 0x22;
	body[i ++] = 0x00;
	body[i ++] = 0x01;
    body[i ++] = (pps_len >> 8) & 0xff;
    body[i ++] = (pps_len) & 0xff;
    memcpy(&body[i], pps, pps_len);
	i += pps_len;
 
	packet.m_packetType 	= RTMP_PACKET_TYPE_VIDEO;
    packet.m_nBodySize  	= i;
    packet.m_nChannel   	= 0x04;
    packet.m_nTimeStamp 	= 0;
    packet.m_hasAbsTimestamp= 0;
    packet.m_headerType 	= RTMP_PACKET_SIZE_LARGE;
	packet.m_nInfoField2 	= rtmpctx->m_stream_id;
	
	printf("rtmp_config_send: vps_lend=%d, sps_len=%d, pps_len=%d\n", vps_len, sps_len, pps_len);
    int nRet = RTMP_SendPacket(rtmpctx, &packet, 0);
	RTMPPacket_Free(&packet);
	return nRet;
}

static int rtmp_write_audio_header(void)
{
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
	RTMPPacket packet;  
	RTMPPacket_Reset(&packet);  
	RTMPPacket_Alloc(&packet, 4);
 
#if 1//8k
	packet.m_body[0] = 0xAA;
	packet.m_body[1] = 0x00;
	packet.m_body[2] = 0x15;
	packet.m_body[3] = 0x88;
#else //16k
	packet.m_body[0] = 0xAA;
	packet.m_body[1] = 0x00;
	packet.m_body[2] = 0x14;
	packet.m_body[3] = 0x08;
#endif
 
	packet.m_headerType  = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;  
	packet.m_hasAbsTimestamp = 0;
	packet.m_nChannel   = 0x04;  
	packet.m_nTimeStamp = 0;  
	packet.m_nInfoField2 = rtmpctx->m_stream_id;
	packet.m_nBodySize  = 4;  
 
	//printf("audio header send....\n");
 
	//调用发送接口  
	int nRet = RTMP_SendPacket(rtmpctx, &packet, TRUE);  
	RTMPPacket_Free(&packet);//释放内存  
	return nRet;  
}

static int rtmp_packet_send_h264(unsigned char *frame, int length, unsigned long long pts)
{
	if( length < 0 ) 
		return 0;
	uint8_t *data = frame/*skip 0001*/; 
	int      size = length;
 
	unsigned long long rpts = pts;
 
	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, 9 + size);
 
	packet.m_packetType 	= RTMP_PACKET_TYPE_VIDEO;
	packet.m_nBodySize  	= 9 + size;
	packet.m_nChannel   	= 0x04;
	packet.m_nTimeStamp 	= rpts/1000;
	packet.m_hasAbsTimestamp= 0;
	packet.m_headerType 	= RTMP_PACKET_SIZE_MEDIUM;
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
	packet.m_nInfoField2 	= rtmpctx->m_stream_id;
 
	memcpy(packet.m_body    , (data[0]&0x1f)!=1? "\x17\x01":"\x27\x01", 2);
	memcpy(packet.m_body + 2, "\x00\x00\x00", 3);
	AMF_EncodeInt32(packet.m_body + 5, packet.m_body + 9, size);
	memcpy(packet.m_body + 9, data, size);
 
	int nRet = RTMP_SendPacket(rtmpctx, &packet, 1);
	RTMPPacket_Free(&packet);
	return nRet;
}

static int rtmp_packet_send_h265(unsigned char *frame, int length, unsigned long long pts)
{
	if( length < 0 ) 
		return 0;
	uint8_t *data = frame/*skip 0001*/; 
	int      size = length;
	int i = 0;
 
	unsigned long long rpts = pts;
 
	RTMPPacket packet;
	RTMPPacket_Alloc(&packet, 9 + size);
 
	packet.m_packetType 	= RTMP_PACKET_TYPE_VIDEO;
	packet.m_nBodySize  	= 9 + size;
	packet.m_nChannel   	= 0x04;
	packet.m_nTimeStamp 	= rpts/1000;
	packet.m_hasAbsTimestamp= 0;
	packet.m_headerType 	= RTMP_PACKET_SIZE_MEDIUM;
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
	packet.m_nInfoField2 	= rtmpctx->m_stream_id;
 
	if (((data[0] & 0x7e) >> 1) == 19) {
		packet.m_body[i++] = 0x1c;
	} else {
		packet.m_body[i++] = 0x2c;
	}
	packet.m_body[i++] = 0x01;//AVC NALU
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = 0x00;
	packet.m_body[i++] = 0x00;
 
	//NALU size
	packet.m_body[i++] = (size >> 24) & 0xff;
	packet.m_body[i++] = (size >> 16) & 0xff;
	packet.m_body[i++] = (size >> 8) & 0xff;
	packet.m_body[i++] = (size) & 0xff;
	
	memcpy(&packet.m_body[i], data, size);
 
	int nRet = RTMP_SendPacket(rtmpctx, &packet, 1);
	RTMPPacket_Free(&packet);
 
	//printf("send package body...\n");
	
	return nRet;
}

static int rtmp_write_audio_data(char *data, int data_len, unsigned long long pts)
{
	RTMP* rtmpctx = (RTMP*)rtmp_handle->rtmpctx;
	int size = data_len -7 + 2;  
	RTMPPacket packet;  
	RTMPPacket_Reset(&packet);  
	RTMPPacket_Alloc(&packet, size);  
	int i=0;
	unsigned long long rpts = pts;// - pusher->vref;
#if 1//8k
 
	packet.m_body[i++] = 0xAA; //8K
	packet.m_body[i++] = 0x01;
#else //16k
	packet.m_body[i++] = 0xAA; //16k
	packet.m_body[i++] = 0x01;
#endif
	memcpy(&packet.m_body[i], data + 7, data_len - 7);
	packet.m_headerType  = RTMP_PACKET_SIZE_MEDIUM;
	packet.m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet.m_hasAbsTimestamp = 0;  
	packet.m_nChannel   = 0x04;  
	packet.m_nTimeStamp = rpts/1000;
	packet.m_nInfoField2 = rtmpctx->m_stream_id;
	packet.m_nBodySize  = size;  
	//调用发送接口 
	int nRet = RTMP_SendPacket(rtmpctx, &packet, TRUE);
	RTMPPacket_Free(&packet);//释放内存  
	return nRet;  
}

#endif
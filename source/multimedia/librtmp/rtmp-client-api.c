/**
 * @file      : rtmp-client-api.c
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
/*
RTMP_Alloc() ：用于创建一个RTMP会话的句柄。
RTMP_Init()：初始化句柄。
RTMP_SetupURL()：设置会话的参数。
RTMP_Connect()：建立RTMP链接中的网络连接（NetConnection）。
RTMP_ConnectStream()：建立RTMP链接中的网络流（NetStream）。
RTMP_Read()：读取RTMP流的内容。
客户端可以在调用RTMP_Connect()之前调用RTMP_EnableWrite()，然后在会话开始之后调用 RTMP_Write()。
RTMP_Pause()：流播放的时候可以用于暂停和继续
RTMP_Seek()：改变流播放的位置
当RTMP_Read()返回0 字节的时候,代表流已经读取完毕，而后可以调用RTMP_Close()
RTMP_Free()：用于清理会话。
*/

rtmp_client_t *rtmp_client_create(void *mchn, char *url)
{
	rtmp_client_t *info = malloc(sizeof(rtmp_client_t));
	if(info)
	{
		memset(info, 0, sizeof(rtmp_client_t));
		RTMP_Init(&info->_rtmp); 
		if(!RTMP_SetupURL(&info->_rtmp, url))
		{
			free(info);
			return NULL;
		}
		info->media_channel = mchn;
		RTMP_EnableWrite(&info->_rtmp);
		return info;
	}
	return NULL;
}

int rtmp_client_start(rtmp_client_t *rtmp)
{
	if(rtmp)
	{
		if(!RTMP_Connect(&rtmp->_rtmp, NULL)) 
		{
			return -1;
		}
		if(!RTMP_ConnectStream(&rtmp->_rtmp, 0))
		{
			RTMP_Close(&rtmp->_rtmp);
			return -1;
		}
		return 0;
	}
	return -1;
}

int rtmp_client_pause(rtmp_client_t *rtmp, int val)
{
	if(rtmp)
	{
		if(RTMP_IsConnected(&rtmp->_rtmp))
		{
			if(RTMP_Pause(&rtmp->_rtmp, val))
				return 0;
			return -1;	
		}
		return 0;
	}
	return -1;
}

int rtmp_client_destroy(rtmp_client_t *rtmp)
{
	if(rtmp)
	{
		RTMP_DeleteStream(&rtmp->_rtmp); 
		RTMP_Close(&rtmp->_rtmp);
		return 0;
	}
	return -1;
}
/**
 * 发送RTMP数据包
 *
 * @param nPacketType 数据类型
 * @param data 存储数据内容
 * @param size 数据大小
 * @param nTimestamp 当前包的时间戳
 *
 * @成功则返回 1 , 失败则返回一个小于0的数
 */
int rtmp_client_send_packet(rtmp_client_t *rtmp, int type, char *data, int len, int timestamp)  
{  
	int nRet =0;
	if (RTMP_IsConnected(&rtmp->_rtmp))
	{
		rtmp->_packet.m_nBodySize = len;
		//memcpy(rtmp->_packet.m_body, data, size);
		rtmp->_packet.m_body = data;
		rtmp->_packet.m_hasAbsTimestamp = 0;
		rtmp->_packet.m_packetType = type; /*此处为类型有两种一种是音频,一种是视频*/
		rtmp->_packet.m_nInfoField2 = rtmp->_stream_id;
		rtmp->_packet.m_nChannel = 0x04;

		rtmp->_packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
		if (RTMP_PACKET_TYPE_AUDIO ==type && len !=4)
		{
			rtmp->_packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
		}
		rtmp->_packet.m_nTimeStamp = timestamp;

		nRet = RTMP_SendPacket(&rtmp->_rtmp, &rtmp->_packet, false); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
	}
	return nRet;  
}  

int rtmp_client_send(rtmp_client_t *rtmp, int type, char *data, int len, int timestamp)
{
	if(rtmp)
	{
		if(RTMP_IsConnected(&rtmp->_rtmp))
		{
			return rtmp_client_send_packet(rtmp, type, data, len, timestamp) ;
		}
		return -1;
	}
	return -1;
}
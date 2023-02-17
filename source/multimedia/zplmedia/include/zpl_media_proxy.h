/*
 * zpl_media_proxy.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_VIDEO_PROXY_H__
#define __ZPL_VIDEO_PROXY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "zpl_media_internal.h"

#define ZPL_MEDIA_PROXY_PORT    63000
struct zpl_media_msg
{
    char *buf;
    zpl_uint16 length_max;
    zpl_uint16 getp;
    zpl_uint16 setp;
};


typedef struct
{
    NODE            node;
    zpl_socket_t    sock;
    uint16_t        port;
    char            *address;
    void            *t_read;
    struct zpl_media_msg    ibuf;
    struct zpl_media_msg    obuf;
    int             type;       //1 data channel; 2 cmd channel
    int             drop;
    int             call_index;
    os_mutex_t	    *mutex;
    int             channel;
    int             level;
}zpl_media_proxy_client_t;

typedef struct
{
    uint16_t    port;
    zpl_socket_t sock;
    uint16_t    cmd_port;
    zpl_socket_t cmd_sock;

    os_mutex_t	*mutex;
    LIST        list;			//add queue
    zpl_taskid_t    t_taskid;
    void        *t_master;
    void        *t_read;
    void        *t_cmd_read;
    int         initalition;
}zpl_media_proxy_server_t;

#define ZPL_MEDIA_PROXY_LOCK(n)     if((n)) os_mutex_lock((n)->mutex, OS_WAIT_FOREVER)
#define ZPL_MEDIA_PROXY_UNLOCK(n)     if((n)) os_mutex_unlock((n)->mutex)

#define ZPL_MEDIA_MSG_MAX_PACKET_SIZ 4096
#define ZPL_MEDIA_MSG_HEADER_MARKER 245

#define ZPL_MEDIA_CMD_SET(c,s)          (((c)&0xff)<<8)|((s)&0xff)
#define ZPL_MEDIA_CMD_GET(C)            (((C) >> 8)&0xFF)
#define ZPL_MEDIA_SUBCMD_GET(C)         ((C)&0xFF)

enum zpl_media_msgcmd 
{
    ZPL_MEDIA_MSG_NONE,
    ZPL_MEDIA_MSG_REGISTER,
    ZPL_MEDIA_MSG_DATA,     //码流数据
    ZPL_MEDIA_MSG_CAPTURE,  //抓拍数据
    ZPL_MEDIA_MSG_REPORT,   //参数变动和定时汇报
    ZPL_MEDIA_MSG_SET_CMD,
    ZPL_MEDIA_MSG_GET_CMD,
    ZPL_MEDIA_MSG_ACK,
    ZPL_MEDIA_MSG_GET_ACK,
};

enum zpl_media_msgsubcmd 
{
    ZPL_MEDIA_MSG_SUB_START,       //开启
    ZPL_MEDIA_MSG_SUB_STOP,        //停止
    ZPL_MEDIA_MSG_SUB_RECORD,      //录像
    ZPL_MEDIA_MSG_SUB_CAPTURE,     //抓拍

    ZPL_MEDIA_MSG_SUB_CODEC,        //设置通道编解码
    ZPL_MEDIA_MSG_SUB_RESOLVING,    //设置通道分辨率
    ZPL_MEDIA_MSG_SUB_FRAME_RATE,   //设置通道帧率
    ZPL_MEDIA_MSG_SUB_BITRATE,      //设置通道码率
    ZPL_MEDIA_MSG_SUB_IKEYRATE,     //I帧间隔
    ZPL_MEDIA_MSG_SUB_PROFILE,      //编码等级
    ZPL_MEDIA_MSG_SUB_RCMODE,       //ZPL_VENC_RC_E
    ZPL_MEDIA_MSG_SUB_GOPMODE,      /* the gop mode */
};

typedef struct
{
    zpl_uint16 length;
    zpl_uint8 marker;
    zpl_uint16 command;
    zpl_uint8 channel;
    zpl_uint8 level;
}__attribute__ ((packed)) zpl_media_proxy_msg_header_t;

#define ZPL_MEDIA_MSG_HEADER_SIZE sizeof(zpl_media_proxy_msg_header_t)

//应答消息
struct zpl_media_proxy_msg_result
{
    zpl_int32 result;
}__attribute__ ((packed)) ;

struct zpl_media_proxy_msg_register
{
    zpl_int32 type;
}__attribute__ ((packed)) ;

struct zpl_media_proxy_msg_cmd
{
    zpl_int32   resolving;      //设置通道分辨率
	zpl_int32   codectype;		//编码类型
	zpl_int32   framerate;		//帧率
	zpl_int32   bitrate;		//码率
    zpl_int32   bitrate_type;	//码率类型
    zpl_int32   profile;        //编码等级
    zpl_int32   ikey_rate;      //I帧间隔
    zpl_int32   enRcMode;
    zpl_int32   gopmode; 
    zpl_int32   record;
    zpl_int32   capture;
}__attribute__ ((packed)) ;

struct zpl_media_proxy_msg_data
{
    zpl_uint8 	type;        //音频/视频 ZPL_MEDIA_E
    zpl_uint8 	codectype;   //编码类型 ZPL_VIDEO_CODEC_E
    zpl_uint8 	frame_type;  //帧类型  ZPL_VIDEO_FRAME_TYPE_E
    zpl_uint8 	buffertype;    //ZPL_MEDIA_FRAME_DATA_E
    zpl_int32	length;      //帧长度  
}__attribute__ ((packed)) ;


int zpl_media_proxy_msg_create_header(struct zpl_media_msg *ipcmsg, zpl_uint32 command, int channel, int level);
int zpl_media_proxy_msg_get_header(struct zpl_media_msg *ipcmsg, zpl_media_proxy_msg_header_t *header);

int zpl_media_proxy_init(void);
int zpl_media_proxy_exit(void);
int zpl_media_proxy_task_init(void);
int zpl_media_proxy_task_exit(void);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_VIDEO_PROXY_H__ */

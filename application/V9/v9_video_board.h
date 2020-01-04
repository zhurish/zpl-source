/*
 * v9_video_board.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_BOARD_H__
#define __V9_VIDEO_BOARD_H__



#define V9_APP_VIDEO_LOAD(f,c)		((f)*(c))

#define V9_APP_VIDEO_FPS_DEFAULT	1080

enum
{
	V9_VIDEO_PROTO_HTTP,
	V9_VIDEO_PROTO_HTTPS,
	V9_VIDEO_PROTO_RTSP,
};

typedef enum v9_video_stream_type_s
{
	V9_VIDEO_STREAM_TYPE_STATIC,
	V9_VIDEO_STREAM_TYPE_DYNAMIC,
	V9_VIDEO_STREAM_TYPE_DYNAMIC_ID,
	V9_VIDEO_STREAM_TYPE_DYNAMIC_CH,
} v9_video_stream_type_t;


typedef struct v9_video_channel_s	//映射到摄像头
{
	//NODE		node;
	u_int8		id;					//板卡ID
	u_int8		ch;					//通道
	u_int32		address;			//IP 地址
	u_int16		port;				//RTSP 端口号
	char		username[V9_APP_USERNAME_MAX];		//用户名
	char		password[V9_APP_PASSWORD_MAX];		//密码
	u_int8		proto;				//协议 RTSP

	u_int32		fps;				//帧率

	BOOL		connect;				//视频流连接状态

	int			dev_status;					// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
	int			rtsp_status;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
	int			decode_status;									// 解码状态

	BOOL		change;

	char		video_url[V9_APP_VIDEO_URL_MAX];


	v9_video_stream_type_t type;
#ifdef V9_VIDEO_SDK_API
	ST_SDKStatusInfo status;
#endif
}v9_video_channel_t;


typedef struct v9_video_board_s
{
	u_int32		id;

	u_int32		address;			//计算板IP 地址
	u_int16		port;				//计算板SDK端口号

	u_int8		channel_cnt;			//计算板连接视频通道数
	u_int8		channel[V9_APP_CHANNEL_MAX];			//计算板连接视频通道数
	v9_video_channel_t *video_stream[V9_APP_CHANNEL_MAX];//计算板下连接视频数据指针

	u_int32		video_load;				//计算板视频处理负载大小，视频流量均衡分发依据
										// fps0*ch0 + fps1*ch1 ...


	v9_address_t	board;
	v9_video_sdk_t	sdk;

	BOOL		use;					//计算板卡使用标志
	BOOL		active;				//计算板卡SDK正常(SDK连接状态，添加视频流等上层操作依赖于该状态)

	BOOL		disabled;			//计算板卡禁止使用
}v9_video_board_t;


extern v9_video_board_t *v9_video_board;

//板卡参数操作
int v9_video_board_init();
int v9_video_board_exit();
v9_video_board_t * v9_video_board_lookup(u_int32 id);
int v9_video_board_add(u_int32 id);
int v9_video_board_del(u_int32 id);
int v9_video_board_active(u_int32 id, BOOL enable);
BOOL v9_video_board_isactive(u_int32 id);
int v9_video_board_address(u_int32 id, u_int32 address, u_int16 port);

int v9_video_board_get_vch(u_int32 id);
int v9_video_board_disabled(u_int32 id, BOOL enable);
BOOL v9_video_board_isdisabled(u_int32 id);
int v9_video_board_show(struct vty *vty, BOOL detail);

/********************************************************************/
/*
 *
 *    STM32          Video Stream
 * 		|                 ^
 * 		|                 |
 * 		|                 |
 * 		v                 |
 *  board state       video state
 *        \              ^
 *         \            /
 *          \          /
 *           \        /
 *            \      /
 *             \    /
 *              v  /
 *            SDK state
 */
/********************************************************************/

//获取计算板负载最小的板卡ID
int v9_video_board_get_minload();
//根据板卡ID获取空闲通道
int v9_video_board_channel_alloc(u_int32 id);
//把视频流加入该板卡
int v9_video_board_channel_add(u_int32 id, v9_video_channel_t *value);
//把视频流从该板卡删除
int v9_video_board_channel_del(u_int32 id, v9_video_channel_t *value);
//在该板卡查询视频流
int v9_video_board_channel_lookup(u_int32 id, v9_video_channel_t *value);



/********************************************************************/
/********************************************************************/
v9_video_channel_t * v9_video_board_video_channel_lookup_by_id_and_ch(u_int8 id, u_int8 ch);

v9_video_channel_t * v9_video_channel_alloc_api(u_int8 ch, u_int32 address, u_int16 port,
												char *username, char *password, u_int32 fps);
v9_video_channel_t * v9_video_channel_lookup_api(u_int8 ch, u_int32 address, u_int16 port);

int v9_video_board_ID_lookup_api_by_video_channel(u_int8 ch, u_int32 address, u_int16 port);

int v9_video_channel_free_api(v9_video_channel_t *v);

int v9_video_channel_show(struct vty *vty, u_int32 id, BOOL detail);
int v9_video_channel_write_config(struct vty *vty);
void * v9_video_app_tmp();

#endif /* __V9_VIDEO_BOARD_H__ */

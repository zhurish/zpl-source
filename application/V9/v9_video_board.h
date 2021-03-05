/*
 * v9_video_board.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_VIDEO_BOARD_H__
#define __V9_VIDEO_BOARD_H__


#ifdef __cplusplus
extern "C" {
#endif


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


typedef struct v9_video_stream_s	//映射到摄像头
{
	//NODE		node;
	ospl_uint8		id;					//板卡ID
	ospl_uint8		ch;					//通道
	ospl_uint32		address;			//IP 地址
	ospl_uint16		port;				//RTSP 端口号
	char		username[APP_USERNAME_MAX];		//用户名
	char		password[APP_USERNAME_MAX];		//密码
	ospl_uint8		proto;				//协议 RTSP

	ospl_uint32		fps;				//帧率

	ospl_bool		connect;				//视频流连接状态

	int			dev_status;					// EAIS设备状态，默认离线。 ENUM_EAIS_DEVICE_STATUS
	int			rtsp_status;									// 通道RTSP状态 ENUM_EAIS_DEVICE_STATUS
	int			decode_status;									// 解码状态

	ospl_bool		change;

	char		video_url[V9_APP_VIDEO_URL_MAX];
	//char		*video_url;
	char		mainstream[V9_APP_VIDEO_URL_PARAM_MAX];
	char		secondary[V9_APP_VIDEO_URL_PARAM_MAX];

	v9_video_stream_type_t type;

	ospl_bool		hw_sync;				//参数是否同步至底层

}v9_video_stream_t;


typedef struct v9_video_board_s
{
	ospl_uint32		id;

	ospl_uint32		address;			//计算板IP 地址
	ospl_uint16		port;				//计算板SDK端口号

	ospl_uint8		channel_cnt;			//计算板连接视频通道数
	ospl_uint8		channel[V9_APP_CHANNEL_MAX];			//计算板连接视频通道数
	v9_video_stream_t *video_stream[V9_APP_CHANNEL_MAX];//计算板下连接视频数据指针

	ospl_uint32		video_load;				//计算板视频处理负载大小，视频流量均衡分发依据
										// fps0*ch0 + fps1*ch1 ...


	v9_board_t	board;
	v9_video_sdk_t	sdk;

	ospl_bool		use;					//计算板卡使用标志
	ospl_bool		active;				//计算板卡SDK正常(SDK连接状态，添加视频流等上层操作依赖于该状态)

	ospl_bool		disabled;			//计算板卡禁止使用

	//void 		*mutex;
	void		*t_timeout;
}v9_video_board_t;


extern v9_video_board_t *v9_video_board;

void v9_video_board_lock();
void v9_video_board_unlock();

//板卡参数操作
int v9_video_board_init();
int v9_video_board_exit();
v9_video_board_t * v9_video_board_lookup(ospl_uint32 id);
int v9_video_board_add(ospl_uint32 id);
int v9_video_board_del(ospl_uint32 id);
int v9_video_board_active(ospl_uint32 id, ospl_bool enable);
ospl_bool v9_video_board_isactive(ospl_uint32 id);

ospl_bool v9_board_ready(v9_video_board_t *vboard);
int v9_board_set_ready(v9_video_board_t *vboard);

int v9_video_board_address(ospl_uint32 id, ospl_uint32 address, ospl_uint16 port);

int v9_video_board_get_vch(ospl_uint32 id);
int v9_video_board_disabled(ospl_uint32 id, ospl_bool enable);
ospl_bool v9_video_board_isdisabled(ospl_uint32 id);
int v9_video_board_show(struct vty *vty, ospl_bool detail);

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
int v9_video_board_stream_alloc(ospl_uint32 id);
//把视频流加入该板卡
int v9_video_board_stream_add(ospl_uint32 id, v9_video_stream_t *value, ospl_bool load);
//把视频流从该板卡删除
int v9_video_board_stream_del(ospl_uint32 id, v9_video_stream_t *value);
//在该板卡查询视频流
int v9_video_board_stream_lookup(ospl_uint32 id, v9_video_stream_t *value);


/********************************************************************/
/*int v9_video_stream_split(char *url, ospl_uint8 *ch, ospl_uint32 *address, ospl_uint16 *port,
							char *username, char *password, char *param, char *secondary);*/
/********************************************************************/
int v9_video_board_stream_status_change(ospl_uint8 id, ospl_uint8 ch, int rtsp, int decode);

v9_video_stream_t * v9_video_board_stream_lookup_by_id_and_ch(ospl_uint8 id, ospl_uint8 ch);

v9_video_stream_t * v9_video_board_stream_alloc_api(ospl_uint8 ch, ospl_uint32 address, ospl_uint16 port,
												char *username, char *password, ospl_uint32 fps,
												char *param, char *secondary);

v9_video_stream_t * v9_video_board_stream_lookup_api(ospl_uint8 ch, ospl_uint32 address, ospl_uint16 port);
int v9_video_board_stream_update_api(ospl_uint8 id, ospl_uint8 ch, char *param, char *secondary);

int v9_video_board_ID_lookup_api_by_video_stream(ospl_uint8 ch, ospl_uint32 address, ospl_uint16 port);

int v9_video_board_stream_free_api(v9_video_stream_t *v);
int v9_video_board_stream_cleanup();


int v9_video_board_stream_show(struct vty *vty, ospl_uint32 id, ospl_bool detail);
int v9_video_board_stream_write_config(struct vty *vty);


int v9_video_sdk_config_show(struct vty *vty);

void * v9_video_app_tmp();

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_BOARD_H__ */

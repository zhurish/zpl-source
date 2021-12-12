#ifndef __V9_CMD_H__
#define __V9_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define V9_APP_CMD_ID		0
#define V9_APP_CMD_ADD(n)	(n)


#define V9_APP_ACK_OK		0
#define V9_APP_ACK_ERROR	1



enum
{
	V9_APP_CMD_ACK = 0X0000 | 0X01,	   //单片机 -> 主控/计算板, 应答命令
	V9_APP_CMD_KEEPALIVE = 0X0000 | 0X02, //单片机 -> 主控/计算板, KEEPALIVE
	V9_APP_CMD_REBOOT = 0X0000 | 0X04,	   //单片机 -> 主控/计算板, 重启命令
	V9_APP_CMD_RESET = 0X0000 | 0X05,	   //单片机 -> 主控/计算板, 恢复出厂设置
	V9_APP_CMD_SHUTDOWN = 0X0000 | 0X06,  //单片机 -> 主控/计算板, 关机

	V9_APP_CMD_SET_ROUTE = 0X0200 | 0X01, //单片机 -> 主控, 设置参数
	V9_APP_CMD_SEND_LOAD = 0X0200 | 0X03, //单片机 -> 主控, 发送计算板状态数据到主控板
	V9_APP_CMD_AUTOIP = 0X0200 | 0X04,	   //单片机 -> 主控/计算板, 发送设置IP命令到计算板

	V9_APP_CMD_STARTUP = 0X0200 | 0X05, //单片机 -> 主控, 通知主控板计算板完成启动，开始互联配置

	V9_APP_CMD_DEVICE = 0X0B00 | 0X01, //单片机 -> 主控, 设备信息
	V9_APP_CMD_DOWNLOAD_OTA = 0X0B00 | 0X02, //主控 -> 单片机, 主控通知单片机执行OTA升级

	V9_APP_CMD_SYNC_TIME = 0X0C00 | 0X01,	//主控 -> 单片机, 有主控同步过来的时间
	V9_APP_CMD_SYNC_LED = 0X0C00 | 0X02,	//主控 -> 单片机, 网口LED
	V9_APP_CMD_PASS_RESET = 0X0C00 | 0X03, //单片机 -> 主控, 恢复默认密码
};

enum
{
	V9_APP_RUNNING	= 0X0F00|0X01,		//正在运行状态
	V9_APP_WARNING	= 0X0F00|0X02,		//告警
	V9_APP_EMERG	= 0X0F00|0X03,		//异常
};

#pragma pack(1)

typedef struct app_cmd_hdr_s
{
	zpl_uint8		id;
	//zpl_uint8		addr;
	zpl_uint8		seqnum;
	zpl_uint16		len;
}app_cmd_hdr_t;

#define V9_APP_HDR_LEN		sizeof(app_cmd_hdr_t)

typedef struct app_cmd_ack_s
{
	zpl_uint16		cmd;
	zpl_uint8		status;
}app_cmd_ack_t, app_cmd_startup_t;

typedef struct app_cmd_reboot_s
{
	zpl_uint16		cmd;
}app_cmd_reboot_t, app_cmd_status_t, app_cmd_autoip_t;

typedef struct app_cmd_keepalive_s
{
	zpl_uint16		cmd;
	zpl_int8		zone;
	zpl_uint32		timesp;
}app_cmd_keepalive_t;

typedef struct app_cmd_route_s
{
	zpl_uint16		cmd;
	zpl_uint8		port;
	zpl_uint32		address;
	zpl_uint32		netmask;
	zpl_uint32		gateway;
	zpl_uint32		dns;
}app_cmd_route_t;


typedef struct app_cmd_status_ack_s
{
	zpl_uint16		cmd;
	zpl_uint8		temp;			//温度
	zpl_uint8		status;			//状态
	zpl_uint8		vch;			//处理视频路数
	zpl_uint8		synctime;		//时间是否已经同步
	zpl_uint16		cpuload;		//CPU负载（百分比）
	zpl_uint32		memtotal;		//内存(M)
	zpl_uint8		memload;		//内存占用（百分比）
	zpl_uint32		disktatol1;		//硬盘(M)
	zpl_uint8		diskload1;		//硬盘占用（百分比）
	zpl_uint32		disktatol2;		//硬盘(M)
	zpl_uint8		diskload2;		//硬盘占用（百分比）

}app_cmd_status_ack_t;



typedef struct app_cmd_rtc_s
{
	zpl_uint16		cmd;
	zpl_int8		zone;
	zpl_uint32		timesp;
}app_cmd_rtc_t, app_cmd_led_t;

typedef struct app_cmd_device_s
{
	zpl_uint16 cmd;
	zpl_int8 devicename[16];	 // 设备名称
	zpl_int8 deviceid[16];	 // 设备Id
	zpl_int8 serialno[16];	 // SN
	zpl_int8 manufacturer[16]; // 厂商名称
	zpl_int8 kervel_version[16];
	zpl_int8 app_version[16];
	zpl_int8 buildtime[32];
} app_cmd_device_t;

typedef struct
{
	zpl_uint16 cmd;
	zpl_uint16 filelen;
} app_cmd_bios_t;

#define V9_APP_HDR_LEN_MAX		V9_APP_HDR_LEN + sizeof(app_cmd_status_ack_t)*4 + sizeof(app_cmd_device_t)

#pragma pack()

extern app_cmd_device_t bios_device;

int v9_cmd_get(v9_serial_t *mgt);
int v9_cmd_send_ack(v9_serial_t *mgt, zpl_uint8 status);
int v9_cmd_handle_keepalive(v9_serial_t *mgt);
int v9_cmd_handle_reboot(v9_serial_t *mgt);
int v9_cmd_handle_route(v9_serial_t *mgt);
int v9_cmd_handle_autoip(v9_serial_t *mgt);
int v9_cmd_handle_startup(v9_serial_t *mgt);
//int v9_cmd_handle_status(v9_serial_t *mgt);
int v9_cmd_handle_board(v9_serial_t *mgt);
int v9_cmd_handle_device(v9_serial_t *mgt);
int v9_web_device_info(char *buf);

int v9_cmd_handle_pass_reset(v9_serial_t *mgt);
#ifdef V9_SLIPNET_ENABLE
int v9_cmd_sync_time_to_rtc(v9_serial_t *mgt, zpl_uint32 timesp);
int v9_cmd_sync_time_test(void);
int v9_cmd_web_reboot();
int v9_cmd_sync_led(v9_serial_t *mgt, zpl_uint32 led, int status);
int v9_cmd_update_bios(zpl_uint32 state);
int v9_cmd_update_bios_ack(v9_serial_t *mgt);
int v9_cmd_update_bios_finsh();
#else
int v9_cmd_handle_sntp_sync(v9_serial_t *mgt);
#endif/* V9_SLIPNET_ENABLE */


#ifdef __cplusplus
}
#endif

#endif /* __V9_CMD_H__ */

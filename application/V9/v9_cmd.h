#ifndef __V9_CMD_H__
#define __V9_CMD_H__

#define V9_APP_CMD_ID		0
#define V9_APP_CMD_ADD(n)	(n)


#define V9_APP_ACK_OK		0
#define V9_APP_ACK_ERROR	1



enum
{
	V9_APP_CMD_ACK			= 0X0000|0X01,			//应答命令
	V9_APP_CMD_KEEPALIVE	= 0X0000|0X02,			//KEEPALIVE
	V9_APP_CMD_REBOOT		= 0X0000|0X04,			//重启命令
	V9_APP_CMD_RESET		= 0X0000|0X05,			//恢复出厂设置
	V9_APP_CMD_SHUTDOWN		= 0X0000|0X06,			//关机

	V9_APP_CMD_SET_ROUTE	= 0X0200|0X01,	//设置参数
	V9_APP_CMD_SEND_LOAD	= 0X0200|0X03,	//发送计算板状态数据到主控板
	V9_APP_CMD_AUTOIP		= 0X0200|0X04,	//发送设置IP命令到计算板

	V9_APP_CMD_STARTUP		= 0X0200|0X05,	//通知主控板计算板完成启动，开始互联配置


	V9_CMD_SYNC_TIME	= 0X0F00|0X01,	//有主控同步过来的时间

	V9_APP_CMD_PASS_RESET	= 0X0E00|0X01,	//恢复默认密码
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
	u_int8		id;
	//u_int8		addr;
	u_int8		seqnum;
	u_int16		len;
}app_cmd_hdr_t;

#define V9_APP_HDR_LEN		sizeof(app_cmd_hdr_t)

typedef struct app_cmd_ack_s
{
	u_int16		cmd;
	u_int8		status;
}app_cmd_ack_t, app_cmd_startup_t;

typedef struct app_cmd_reboot_s
{
	u_int16		cmd;
}app_cmd_reboot_t, app_cmd_status_t, app_cmd_autoip_t;

typedef struct app_cmd_keepalive_s
{
	u_int16		cmd;
	s_int8		zone;
	u_int32		timesp;
}app_cmd_keepalive_t;

typedef struct app_cmd_route_s
{
	u_int16		cmd;
	u_int8		port;
	u_int32		address;
	u_int32		netmask;
	u_int32		gateway;
	u_int32		dns;
}app_cmd_route_t;


typedef struct app_cmd_status_ack_s
{
	u_int16		cmd;
	u_int8		temp;			//温度
	u_int8		status;			//状态
	u_int8		vch;			//处理视频路数
	u_int8		synctime;		//时间是否已经同步
	u_int16		cpuload;		//CPU负载（百分比）
	u_int32		memtotal;		//内存(M)
	u_int8		memload;		//内存占用（百分比）
	u_int32		disktatol1;		//硬盘(M)
	u_int8		diskload1;		//硬盘占用（百分比）
	u_int32		disktatol2;		//硬盘(M)
	u_int8		diskload2;		//硬盘占用（百分比）

}app_cmd_status_ack_t;



typedef struct app_cmd_rtc_s
{
	u_int16		cmd;
	s_int8		zone;
	struct tm	ptm;
}app_cmd_rtc_t;


#define V9_APP_HDR_LEN_MAX		V9_APP_HDR_LEN + sizeof(app_cmd_status_ack_t)*4

#pragma pack()

int v9_cmd_get(v9_serial_t *mgt);
int v9_cmd_send_ack(v9_serial_t *mgt, u_int8 status);
int v9_cmd_handle_keepalive(v9_serial_t *mgt);
int v9_cmd_handle_reboot(v9_serial_t *mgt);
int v9_cmd_handle_route(v9_serial_t *mgt);
int v9_cmd_handle_autoip(v9_serial_t *mgt);
int v9_cmd_handle_startup(v9_serial_t *mgt);
//int v9_cmd_handle_status(v9_serial_t *mgt);
int v9_cmd_handle_board(v9_serial_t *mgt);
int v9_cmd_handle_pass_reset(v9_serial_t *mgt);
int v9_cmd_sync_time_to_rtc(v9_serial_t *mgt, u_int32 timesp);
int v9_cmd_sync_time_test(void);

#endif /* __V9_CMD_H__ */

/*
 * x5_b_cmd.h
 *
 *  Created on: 2019年3月22日
 *      Author: DELL
 */

#ifndef __X5_B_CMD_H__
#define __X5_B_CMD_H__

#include "x5_b_global.h"
#define X5B_APP_HDR_MAKR				0X7E

enum E_CALL_RESULT
{
	E_CALL_RESULT_NO_SEARCH,		//房间号不存在
	E_CALL_RESULT_UNREGISTER,		//号码未注册
	E_CALL_RESULT_CALLING,			//呼叫中
	E_CALL_RESULT_TALKLING,			//通话中
	E_CALL_RESULT_STOP,				//ͨ挂断
	E_CALL_RESULT_FAIL,				//ͨ拨号失败
	E_CALL_RESULT_INCOME,			//来电中
};

enum E_NETWORK_STATE
{
	E_CMD_NETWORK_STATE_PHY_DOWN,
	E_CMD_NETWORK_STATE_PHY_UP,
	E_CMD_NETWORK_STATE_DOWN,
	E_CMD_NETWORK_STATE_UP,
};

enum E_OPEN_STATE
{
	E_CMD_UNLOCK_STATE,
	E_CMD_LOCK_STATE,
};

/*
 * CMD
 */
#define E_CMD_BASE 0X0000
enum
{
	E_CMD_ACK 			= E_CMD_BASE|0X0001,				//应答指令
	E_CMD_REGISTER 		= E_CMD_BASE|0X0002,				//开机注册 A/C->B
	E_CMD_REGISTER_OK 	= E_CMD_BASE|0X0003,				//开机注册成功 B->A/C

	E_CMD_OPEN 			= E_CMD_BASE|0X0004,				//开门信令 B->A

	E_CMD_KEEPALIVE 	= E_CMD_BASE|0X0005,					//keepalive

	E_CMD_ROOM_AUTH 	= E_CMD_BASE|0X0006,				//房间号鉴权
	E_CMD_ROOM_AUTH_ACK = E_CMD_BASE|0X0007,				//

	E_CMD_SECOM_OPEN_REQ = E_CMD_BASE|0X000D,				//C->A NFC&人脸识别开门请求 secom版本开门请求信息

	E_CMD_REBOOT_REQ = E_CMD_BASE|0X000F,					//reboot
	E_CMD_RESET_REQ = E_CMD_BASE|0X0010,					//reset

	E_CMD_CARD_SEQ = E_CMD_BASE|0X000A,					//卡号
};

#define E_CMD_SET	0x0200

enum
{
	E_CMD_FACTORY_MODE 	= E_CMD_SET|0X0000,				//安装工参数  A/C->B
	E_CMD_GET_RTC_TIME 	= E_CMD_SET|0X0001,				//获取A模块RTC 时间  B->A
	E_CMD_ACK_RTC_TIME 	= E_CMD_SET|0X0002,				//获取A模块RTC 时间 A->B

	E_CMD_IP_ADDRESS 	= E_CMD_SET|0X0003,				//

	E_CMD_MAKE_CARD 	= E_CMD_SET|0X0004,				//
	E_CMD_DELETE_CARD 	= E_CMD_SET|0X0005,				//B->A

	E_CMD_OPEN_OPTION 	= E_CMD_SET|0X0006,				//B->A

	E_CMD_FACE_CONFIG 	= E_CMD_SET|0X0007,				//B->C

	E_CMD_FACE_REQ 	= E_CMD_SET|0X0008,				//B->C

	E_CMD_FACE_ACK 	= E_CMD_SET|0X0009,				//B->C

	E_CMD_WIGGINS 	= E_CMD_SET|0X0010,				//B->A wiggins

	E_CMD_FACE_ACK_RES 	= E_CMD_SET|0X0011,				//B->C

	E_CMD_DEVICE_OPT 	= E_CMD_SET|0X0020,				//B->C
	E_CMD_SIP_OPT 	= E_CMD_SET|0X0021,				//C->B

	E_CMD_SYSTEM_CONFIG 	= E_CMD_SET|0X0030,				//B->C/A
	E_CMD_SYNC_WEBTIME 	= E_CMD_SET|0X0040,				//B->C
};

#define E_CMD_CALL	0x0500
enum
{
	E_CMD_START_CALL 	= E_CMD_CALL|0X0000,	//开始呼叫 A/C->B
	E_CMD_CALL_RESULT 	= E_CMD_CALL|0X0001,	//呼叫结果 B->A/C
	E_CMD_STOP_CALL		= E_CMD_CALL|0X0002,	//停止呼叫A/C->B
	E_CMD_START_CALL_PHONE = E_CMD_CALL|0X003,				//C->B
	E_CMD_START_CALL_LIST = E_CMD_CALL|0X004,				//C->B
};

#define E_CMD_STATUS	0x0600
enum
{
	E_CMD_NETSTATUS 	= E_CMD_STATUS|0X0000,	//网络接口状态（主动上报） B->A/C
	E_CMD_REQ_STATUS	= E_CMD_STATUS|0X0001,	//查询网络接口信息A/C->B
	E_CMD_ACK_STATUS 	= E_CMD_STATUS|0X0002,	//网络接口信息 B->A/C

	E_CMD_REG_STATUS	= E_CMD_STATUS|0X0003,	//号码注册状态 B->A/C
	E_CMD_REQ_REGISTER	= E_CMD_STATUS|0X0004,	//查询注册信息A/C->B
	E_CMD_ACK_REGISTER	= E_CMD_STATUS|0X0005,	//号码注册服务器 B->A/C

	E_CMD_REQ_VERSION	= E_CMD_STATUS|0X0006,	//查询版本信息
	E_CMD_ACK_VERSION	= E_CMD_STATUS|0X0007,	//版本信息应答

	E_CMD_OPEN_LOG		= E_CMD_STATUS|0X0008,	//通行日志信息

	E_CMD_UNIT_TEST		= E_CMD_STATUS|0X00ff,	//Unit Test
};

#define E_CMD_UPDATE	0x0800
enum
{
	E_CMD_UPDATE_MODE 	= E_CMD_UPDATE|0X0001,	// B->A
	E_CMD_UPDATE_DATA	= E_CMD_UPDATE|0X0002,	//B->A
};

#define E_CMD_MASK			0X0FFF
#define E_CMD_GET(n)	(((n) & E_CMD_MASK))

#define E_CMD_TYPE_MASK		0X0F00
#define E_CMD_TYPE_GET(n)	(((n) & E_CMD_TYPE_MASK))

#define E_CMD_MODULE_A		(0XA000)
#define E_CMD_MODULE_B		(0XB000)
#define E_CMD_MODULE_C		(0XC000)

#define E_CMD_MAKE(m, i, c)		( ((m)&0xF000) | ((i)&0x0F00) | ((c)&0x00FF) )


#define E_CMD_FROM_A(n)		(((n) & 0XA000) == 0XA000)
#define E_CMD_FROM_B(n)		(((n) & 0XB000) == 0XB000)
#define E_CMD_FROM_C(n)		(((n) & 0XC000) == 0XC000)


enum //to CMD
{
	E_CMD_TO_AUTO,
	E_CMD_TO_A,
	E_CMD_TO_B,
	E_CMD_TO_C,
};

enum //ID
{
	X5B_APP_MODULE_ID_A = 0X5A000000U,
	X5B_APP_MODULE_ID_B = 0X5B000000U,
	X5B_APP_MODULE_ID_C = 0X5C000000U,
};

/*
#define E_CMD_TO_A(n)		(((n) | 0XA000))
#define E_CMD_TO_B(n)		(((n) | 0XB000))
#define E_CMD_TO_C(n)		(((n) | 0XC000))
*/


enum E_CMD_LEN
{
	E_CMD_ACK_LEN = 1,
	E_CMD_CALL_RESULT_LEN = 2,

	E_CMD_REGISTER_OK_LEN = 6,

	//E_CMD_FACTORY_MODE_LEN = 128,

	E_CMD_IP_ADDRESS_LEN = 4,

};


#pragma pack(1)
typedef struct x5b_app_hdr_s
{
	u_int8 makr;
	u_int8 seqnum;
	u_int8 m_seqnum;
	u_int32 total_len;
} x5b_app_hdr_t;

typedef struct x5b_app_room_auth_ack_s
{
	u_int8	building;
	u_int8	unit;
	u_int8	room_number[4];
	u_int8	result;
	u_int8	data[512];
} x5b_app_room_auth_ack_t;//房间号鉴权应答

typedef struct x5b_app_room_auth_s
{
	u_int8	building;
	u_int8	unit;
	u_int8	room_number[4];
	u_int8	result;
	u_int8	data[512];
} x5b_app_room_auth_t;// 房间号鉴权


typedef struct x5b_app_call_s
{
	u_int8	building;
	u_int8	unit;
	u_int16	room_number;
} x5b_app_call_t;//呼叫房间号

typedef struct x5b_app_call_from_s
{
	u_int8	building;
	u_int8	unit;
	u_int8	room_number[4];
} x5b_app_call_from_t;//呼叫房间号

typedef struct x5b_app_factory_s
{
	u_int32 local_address;			//MUST: local IP address/DHCP(0.0.0.0)
	u_int32 local_netmask;
	u_int32 local_gateway;
	u_int32 local_dns;
} x5b_app_factory_t, x5b_app_netinfo_t;//安装工配置网络参数， 网络参数应答上报


typedef struct x5b_app_register_ack_s
{
	u_int8	reg_state;
	char 	phone[32];
} x5b_app_register_ack_t;//本地号码注册状态


typedef struct x5b_app_phone_register_ack_s
{
	u_int8 iface;
	u_int16 l_port;
	u_int32 sip_address;
	u_int16 sip_port;
	u_int32 proxy_address;
	u_int16 proxy_port;
	u_int8 proto;
	u_int8 dtmf;
	u_int8 codec;
} x5b_app_phone_register_ack_t;//本地号码服务信息

typedef struct
{
	u_int RelayOpenTime;
	u_int RelayOpenWaitOpenDoorTime;
	u_int DoorKeepOpenTime;
	u_int8 DoorSensorOutPutLevle;
	u_int8 LockRole;
	u_int8 TamperAlarm;
	u_int8 DoorKeepOpentTimeOutAlarm;
	u_int8 rev;//doorcontact
} ConfiglockType;//A模块开门信息

enum //ID
{
	X5B_APP_UPDATE_STM32 = 0X01U,
	X5B_APP_UPDATE_ESP32 = 0X02U,
};

typedef struct
{
	u_int8 mode;
	u_int32 len;
} x5b_app_update_mode_t;//A模块升级模式

/*
#define X5B_APP_UPDATE_DATA_MAX	1024
typedef struct
{
	u_int8 data[X5B_APP_UPDATE_DATA_MAX];
} x5b_app_update_data_t;
*/


typedef struct _permiList
{
	u_int8     ID[8];
    u_int32    start_time;
    u_int32    stop_time;
    u_int8     status;//1 2
	u_int8     res[3];
}permiListType;

typedef struct
{
    uint8_t year;
    uint8_t mon;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t week;

} rtcTimeType;

typedef struct cardid_respone
{
	u_int8    op_type;
	u_int8    face_result;
    u_int8    card_result;//1 2
	u_int8    clen;
	u_int8    ID[8];
}open_cardid_respone;



typedef struct global_config_s
{
	u_int8 x5cm;//：（1 byte）
	u_int8 blu;//：（1 byte）
	u_int8 nfc;//：（1 byte）
	u_int8 opentype;//：（1 byte）
	u_int8 custom;//：（1 byte）
	u_int8 scene;//：（1 byte）
	u_int8 housing;//：（1 byte）
	u_int8 devname[X5B_APP_DEVICE_NAME_MAX];//：（64 byte）
	u_int8 location[X5B_APP_DEVICE_NAME_MAX];//：（64 byte）
	u_int8 direction;//：（1 byte）
	u_int8 address1[X5B_APP_DEVICE_IP_MAX];//：（32 byte）
	u_int8 address2[X5B_APP_DEVICE_IP_MAX];//：（32byte）
	u_int8 address3[X5B_APP_DEVICE_IP_MAX];//：（32byte
}global_config_t;

#pragma pack(0)

#endif /* __X5_B_CMD_H__ */

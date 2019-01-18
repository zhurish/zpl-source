/*
 * voip_sip.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __VOIP_SIP_H__
#define __VOIP_SIP_H__

#include "config.h"

#include "voip_def.h"
#include "voip_event.h"
#include "voip_state.h"
#include "voip_app.h"


#define SIP_CTL_MSGQ
//#define SIP_CTL_SOCKET
//#define SIP_CTL_PIPE
//#define SIP_CTL_SYNC


#ifdef SIP_CTL_MSGQ
#define SIP_CTL_MSGQ_KEY	0X6000
#define SIP_CTL_LMSGQ_KEY	0X30000
#endif


//#define SIP_CONFIG_FILE		SYSCONFDIR"/voip_cfg.txt"
#define SIP_CONFIG_FILE		SYSCONF_REAL_DIR"/sip.cfg"


#define SIP_CTL_TIMEOUT		5
#define VOIP_SIP_EVENT_LOOPBACK



#define _SIP_CTL_DEBUG

#define SIP_CTL_DEBUG_RECV		0x0001
#define SIP_CTL_DEBUG_SEND		0x0002
#define SIP_CTL_DEBUG_DETAIL	0x0004
#define SIP_CTL_DEBUG_EVENT		0x0008
#define SIP_CTL_DEBUG_MSGQ		0x0010

#define SIP_CTL_DEBUG(n)		(SIP_CTL_DEBUG_ ## n & voip_sip_ctl.debug)
#define SIP_CTL_DEBUG_ON(n)		(voip_sip_ctl.debug |= SIP_CTL_DEBUG_ ## n)
#define SIP_CTL_DEBUG_OFF(n)	(voip_sip_ctl.debug &= ~SIP_CTL_DEBUG_ ## n)





#define SIP_NUMBER_MAX		16
#define SIP_DATA_MAX		128

#define SIP_ENABLE_DEFAULT			TRUE
#define SIP_PORT_DEFAULT			5060
#define SIP_PORT_SEC_DEFAULT		5060
#define SIP_PROXY_PORT_DEFAULT		5060
#define SIP_PROXY_PORT_SEC_DEFAULT	5060
#define SIP_TIME_DEFAULT			TRUE
#define SIP_RING_DEFAULT			1
#define SIP_REGINTER_DEFAULT		8
#define SIP_HOSTPART_DEFAULT		"tslsmart"
#define SIP_INTERVAL_DEFAULT		8
#define SIP_DIALPLAN_DEFAULT		"tslsmart"
#define SIP_REALM_DEFAULT			"tslsmart"
#define SIP_ENCRYPT_DEFAULT			TRUE
#define SIP_DIS_NAME_DEFAULT		TRUE
#define SIP_100_REL_DEFAULT			TRUE

#define SIP_PHONE_DEFAULT			"0003"
#define SIP_USERNAME_DEFAULT		"0003"
#define SIP_PASSWORD_DEFAULT		"0003"
#define SIP_DTMF_DEFAULT			"rfc2833"


typedef struct voip_sip_s
{
	BOOL				sip_enable;
	u_int32				sip_server;					//SIP��������ַ
	u_int32				sip_server_sec;				//SIP���÷�������ַ
	u_int32				sip_proxy_server;			//�����������ַ
	u_int32				sip_proxy_server_sec;		//��ѡ�����������ַ

	u_int16				sip_port;					//SIP�������˿ں�
	u_int16				sip_port_sec;				//SIP���÷������˿ں�
	u_int16				sip_proxy_port;				//����������˿ں�
	u_int16				sip_proxy_port_sec;			//��ѡ����������˿ں�

	u_int16				sip_local_port;				//���ñ���SIP�������˿ں�

	BOOL				sip_time_sync;				//ʱ���Ƿ�ͬ��

	u_int16				sip_ring;					//��������

	u_int16				sip_register_interval;		//

	u_int8				sip_hostpart[SIP_DATA_MAX];	//热线
	u_int16				sip_interval;				//
	BOOL				sip_100_rel;				//
	BOOL				sip_dis_name;				//display name
	u_int8				sip_local_number[SIP_NUMBER_MAX];	//
	u_int8				sip_user[SIP_DATA_MAX];
	u_int8				sip_password[SIP_DATA_MAX];				//
	u_int8				sip_realm[SIP_DATA_MAX];					//realm
	u_int8				sip_dialplan[SIP_DATA_MAX];				//dialplan
	BOOL				sip_encrypt;				//


	void				*t_event;
} voip_sip_t;



typedef enum sip_register_state_s
{
	//注册状态
	VOIP_SIP_UNREGISTER,			//未注册
	VOIP_SIP_REGISTER_FAILED,		//注册失败
	VOIP_SIP_REGISTER_SUCCESS,

}sip_register_state_t;

typedef enum sip_call_state_s
{
	//呼叫状态
	VOIP_SIP_CALL_IDLE,
	VOIP_SIP_CALL_ERROR,			//呼叫出错
	VOIP_SIP_CALL_FAILED = VOIP_SIP_CALL_ERROR,			//呼叫失败
	VOIP_SIP_CALL_RINGING,			//呼叫振铃
	VOIP_SIP_CALL_PICKUP,			//呼叫摘机
	VOIP_SIP_CALL_SUCCESS = VOIP_SIP_CALL_PICKUP,			//呼叫成功

	VOIP_SIP_TALK,					//通话中
}sip_call_state_t;

typedef enum sip_call_error_s
{
	VOIP_SIP_TRYING_100,			//100试呼叫（Trying）
	VOIP_SIP_RINGING_180,			//180振铃（Ringing）
	VOIP_SIP_FORWARD_181,			//181呼叫正在前转（Call is Being Forwarded）
	VOIP_SIP_ACK_200,				//200成功响应（OK）
	VOIP_SIP_MOVETMP_320,			//302临时迁移（Moved Temporarily）
	VOIP_SIP_BAD_REQUEST_400,		//400错误请求（Bad Request）
	VOIP_SIP_UNAUTHORIZED_401,		//401未授权（Unauthorized）
	VOIP_SIP_FORBIDDEN_403,			//403禁止（Forbidden）
	VOIP_SIP_NOTFOUND_404,			//404用户不存在（Not Found）
	VOIP_SIP_REQUEST_TIMEOUT_408,	//408请求超时（Request Timeout）
	VOIP_SIP_UNAVAILABLE_480,		//480暂时无人接听（Temporarily Unavailable）
	VOIP_SIP_BUSY_HERE_486,			//486线路忙（Busy Here）
	VOIP_SIP_SERVER_TIMEOUT_504,	//504服务器超时（Server Time-out）
	VOIP_SIP_BUSY_EVERYWHERE_600,	//600全忙（Busy Everywhere）

}sip_call_error_t;

typedef enum sip_stop_state_s
{
	//挂机状态
	VOIP_SIP_REMOTE_STOP,			//远端挂机
	VOIP_SIP_LOCAL_STOP,			//本端挂机
}sip_stop_state_t;

typedef struct voip_sip_ctl_s
{
	void		*master;
#ifdef SIP_CTL_SOCKET
	BOOL		tcpMode;
	int			sock;
	int			accept;
	int			wfd;
	void		*t_accept;
	void		*t_read;
	void		*t_write;
#endif

#ifdef SIP_CTL_MSGQ
	int			taskid;
	int			rq;
	int			wq;
#endif
	void		*t_event;
	void		*t_time;
	void		*t_regtime;
	u_int8		buf[1024];
	u_int16		len;
	u_int8		sbuf[1024];
	u_int16		slen;

	sip_register_state_t	reg_state;
	sip_call_state_t		call_state;
	sip_call_error_t		call_error;
	sip_stop_state_t		stop_state;

	u_int32		send_cmd;
	u_int32		ack_cmd;

	u_int32		debug;
}voip_sip_ctl_t;


/*
 *
[sip_config]
server_ip = 192.168.1.23
server_port = 5060
user_name = 333
passwd = 123456
realm = test
local_ip = 0.0.0.0
local_port = 5060
dtmf = rfc2833
*/
#define VOIP_SIP_SRCID	6
#define VOIP_SIP_DSTID	8


#define   MSG_TOLINECTL_SETUP        0x00b00001
#define   MSG_TOLINECTL_RELEASE      0x00b00002
#define   MSG_TOLINECTL_RMACK        0x00b00003
#define   MSG_TOLINECTL_RMRPLYERR    0x00b00004
#define   MSG_TOLINECTL_RMRPLY18X    0x00b00005
#define   MSG_TOLINECTL_RMANSWER200  0x00b00006
#define   MSG_TOLINECTL_REGISTER     0x00b00007

#define   MSG_FRLINECTL_LOINVITE     0x00b00008
#define   MSG_FRLINECTL_LOBYE        0x00b00009

enum
{
	SIP_REGISTER_MSG = 1,
	SIP_REGISTER_ACK_MSG = MSG_TOLINECTL_REGISTER,

	SIP_CALL_MSG = MSG_FRLINECTL_LOINVITE,
	SIP_REMOTE_RING_MSG = MSG_TOLINECTL_RMRPLY18X,
	SIP_REMOTE_PICKING_MSG = MSG_TOLINECTL_RMANSWER200,
	SIP_REMOTE_ACK_MSG = MSG_TOLINECTL_RMACK,
	SIP_CALL_ERROR_MSG,
	SIP_REMOTE_STOP_MSG = MSG_TOLINECTL_RELEASE,
	SIP_LOCAL_STOP_MSG = MSG_FRLINECTL_LOBYE,

	SIP_REMOTE_INFO_MSG = 0x00b0000a,
};


enum
{
	SIP_REGISTER_ENABLE = 1,
	SIP_REGISTER_DISABLE = 0,
};

enum
{
	SIP_REGISTER_OK = 1,
	SIP_REGISTER_ERROR = 0,
};


#define SIP_MSG_OFFSET(n)		((n) + sizeof(MSG_HDR_T))
#define SIP_MSG_LEN(n)			((n) + sizeof(MSG_HDR_T))

#pragma pack(1)

#define VOS_MSG_HDR_MAGIC    (0xaabbccdd)
#define SIP_MSG_HDR_MAGIC	VOS_MSG_HDR_MAGIC

#if 0
typedef struct
{
	u_int32         magic;
	u_int32      	priority;  /* Priority must be the first 4-byte */
	u_int32         srcApplId; /* VOS_APPL_ID */
	u_int32    		type;
    u_int32         srcMsgQKey;
    BOOL            sync;      /* Sync or async message */
    u_int32         len;       /* the length of the message, including the message hdr */
    u_int32         tick;
} SIP_MSG_HDR_T;
#endif

typedef struct
{
#if 0
    unsigned char    srcApplId;  /*源进程号  VOIP--6，linectl---8*/
    unsigned char    dstAppId;   /*目的进程号*/
    unsigned char    type;     	/*消息类型 1--注册；2--注册返回，3--*/
    BOOL             sync;      /* 是否是同步消息 */
    u_int32          len;       /* 消息长度，包含消息头 */
#endif
	u_int32         magic;
	u_int32      	priority;  /* Priority must be the first 4-byte */
	u_int32         srcApplId; /* VOS_APPL_ID */
	u_int32    		type;
    u_int32         srcMsgQKey;
    BOOL            sync;      /* Sync or async message */
    u_int32         len;       /* the length of the message, including the message hdr */
    u_int32         tick;
}MSG_HDR_T;

/*type = 1,注册，linectl---->voip*/
typedef struct
{
    unsigned char act;    /*1---注册；0---去注册*/
}MSG_REG_ACT;

/*type = 2,注册结果返回，voip---->linectl*/
typedef struct
{
    unsigned char uid;
    unsigned char rlt;    /*1---注册成功；0---注册失败*/
}MSG_REG_RLT;

/*type = 3,去呼叫，linectl---->voip*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
    unsigned char digit[48]; /* 被叫号码*/
    unsigned char digit_num; /*被叫号码长度*/
    unsigned char name[48];  /*被叫名称*/
    unsigned char name_num;  /*被叫名称长度*/
    unsigned int  rtp_port;  /*本地rtp端口*/
    unsigned char rtp_addr[64];  /*本地RTP地址*/
    unsigned char codec;     /*优先使用的codec*/
}MSG_CAL_ACT;

/*type = 4,被叫振铃18x，voip---->linectl*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
    unsigned int  rtp_port;  /*远端rtp端口*/
    unsigned char rtp_addr[64];  /*远端RTP地址*/
    unsigned char codec;     /*远端使用的codec*/
}MSG_REMOT_ALERT;

/*type = 5,被叫摘机200，voip---->linectl*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
    unsigned int  rtp_port;  /*远端rtp端口*/
    unsigned char rtp_addr[64];  /*远端RTP地址*/
    unsigned char codec;     /*远端使用的codec*/
}MSG_REMOT_ANSWER;

typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
    unsigned int  rtp_port;  /*Ô¶¶Ërtp¶Ë¿Ú*/
    unsigned char rtp_addr[64];  /*Ô¶¶ËRTPµØÖ·*/
    unsigned char codec;     /*Ô¶¶ËÊ¹ÓÃµÄcodec*/
}MSG_REMOT_ACK;


/*type = 6,呼叫出错4XX，voip---->linectl*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
    int           cause;     /*出错原因*/
}MSG_REMOT_ERROR;

/*type = 7,远端挂机，voip---->linectl*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
}MSG_REMOT_BYE;

/*type = 8,本端挂机，linectl---->voip*/
typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
}MSG_LOCAL_BYE;

typedef struct
{
    unsigned char uid;       /* uid = 1*/
    unsigned char senid;     /* senid = 1*/
	unsigned char content[1];   /*按键号码*/
}MSG_SIP_INFO;

#pragma pack(0)

extern voip_sip_t voip_sip_config;


extern int voip_sip_module_init();
extern int voip_sip_module_exit();

/*
 * SIP config Module
 */
extern int voip_sip_enable(BOOL enable, u_int16 port);
extern int voip_sip_server_set_api(u_int32 ip, u_int16 port, BOOL sec);
extern int voip_sip_proxy_server_set_api(u_int32 ip, u_int16 port, BOOL sec);
extern int voip_sip_time_syne_set_api(BOOL enable);
extern int voip_sip_ring_set_api(u_int16 value);
extern int voip_sip_register_interval_set_api(u_int16 value);
extern int voip_sip_hostpart_set_api(u_int8 * value);
extern int voip_sip_interval_set_api(u_int16 value);
extern int voip_sip_100_rel_set_api(BOOL value);
extern int voip_sip_display_name_set_api(BOOL value);
extern int voip_sip_local_number_set_api(char * value);
extern int voip_sip_password_set_api(char * value);
extern int voip_sip_user_set_api(char * value);
extern int voip_sip_realm_set_api(char * value);
extern int voip_sip_dialplan_set_api(u_int8 * value);
extern int voip_sip_encrypt_set_api(BOOL value);


extern int voip_sip_config_update_api(voip_sip_t *sip);
/*
 * SIP state Module
 */
extern sip_register_state_t voip_sip_register_state_get_api();
extern sip_call_error_t voip_sip_call_error_get_api();

extern sip_call_state_t voip_sip_call_state_get_api();
extern sip_stop_state_t voip_sip_stop_state_get_api();

int voip_sip_read_handle(voip_sip_ctl_t *sipctl, char *buf, int len);

/*
 * SIP event Module (sock)
 */

extern int voip_sip_ctl_module_init();
extern int voip_sip_ctl_module_exit();

/*
extern int voip_sip_register(char *phone, char *user, char *password, BOOL enable);
extern int voip_sip_call(char *phone, char *user, char *password, int timeoutms, BOOL start);
*/

extern int voip_sip_register_start(BOOL reg);
extern int voip_sip_call_start(char *phone);
extern int voip_sip_call_stop();

/*
 * cmd module
 */
extern void cmd_sip_init(void);
extern int voip_sip_write_config(struct vty *vty);
extern int voip_sip_show_config(struct vty *vty, BOOL detail);


#endif /* __VOIP_SIP_H__ */

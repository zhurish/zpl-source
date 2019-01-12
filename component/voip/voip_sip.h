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

//#define SIP_CONFIG_FILE		SYSCONFDIR"/voip_cfg.txt"
#define SIP_CONFIG_FILE		SYSCONF_REAL_DIR"/voip_cfg.txt"


#define SIP_CTL_TIMEOUT		5
#define VOIP_SIP_EVENT_LOOPBACK


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

#define SIP_PHONE_DEFAULT			"tslsmart"
#define SIP_USERNAME_DEFAULT		"tslsmart"
#define SIP_PASSWORD_DEFAULT		"tslsmart123456!"
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

	u_int16				sip_register_interval;		//����ע������

	u_int8				sip_hostpart[SIP_DATA_MAX];				//����hostpart
	u_int16				sip_interval;				//��������
	BOOL				sip_100_rel;				//����100rel�Ƿ�ǿ��ʹ��
	BOOL				sip_dis_name;				//����display name
	u_int8				sip_local_number[SIP_NUMBER_MAX];	//���ñ��غ���
	u_int8				sip_user[SIP_DATA_MAX];
	u_int8				sip_password[SIP_DATA_MAX];				//��������
	u_int8				sip_realm[SIP_DATA_MAX];					//����realm
	u_int8				sip_dialplan[SIP_DATA_MAX];				//����dialplan
	BOOL				sip_encrypt;				//����ע������


	void				*t_event;
} voip_sip_t;


typedef struct voip_sip_state_s
{
	voip_state_t state;
}voip_sip_state_t;



typedef struct voip_sip_ctl_s
{
	void		*master;
	BOOL		tcpMode;
	int			sock;
	int			accept;
	int			wfd;
	void		*t_accept;
	void		*t_read;
	void		*t_write;
	void		*t_event;
	void		*t_time;

	u_int8		buf[1024];
	u_int16		len;
	u_int8		sbuf[1024];
	u_int16		slen;

	voip_sip_state_t	state;

	u_int32		send_cmd;
	u_int32		ack_cmd;
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

enum
{
	SIP_REGISTER_MSG = 1,
	SIP_REGISTER_ACK_MSG = 2,

	SIP_CALL_MSG = 3,
	SIP_REMOTE_RING_MSG = 4,
	SIP_REMOTE_PICKING_MSG,
	SIP_CALL_ERROR_MSG,
	SIP_REMOTE_STOP_MSG,
	SIP_LOCAL_STOP_MSG,
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


#define SIP_MSG_OFFSET(n)		(n) + sizeof(MSG_HDR_T)
#define SIP_MSG_LEN(n)			(n) + sizeof(MSG_HDR_T)

#pragma pack(1)
typedef struct
{
    unsigned char    srcApplId;  /*源进程号  VOIP--6，linectl---8*/
    unsigned char    dstAppId;   /*目的进程号*/
    unsigned char    type;     /*消息类型 1--注册；2--注册返回，3--*/
    BOOL             sync;      /* 是否是同步消息 */
    u_int32          len;       /* 消息长度，包含消息头 */
}MSG_HDR_T;

/*type = 1,注册，linectl---->voip*/
typedef struct
{
    unsigned char act;    /*1---注册；0---去注册*/
}MSG_REG_ACT;

/*type = 2,注册结果返回，voip---->linectl*/
typedef struct
{
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
//extern int voip_sip_call_state_get_api(int *value);
extern voip_state_t voip_sip_state_get_api();


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

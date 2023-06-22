/*
 * pjsip_app_api.h
 *
 *  Created on: Jun 15, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_APP_API_H__
#define __PJSIP_APP_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pjsua_app_common.h"

//#define ZPL_PJSIP_CALL_SHELL		1
//#define ZPL_PJSIP_CLI_SHELL		1

#define PJSIP_NUMBER_MAX		32
#define PJSIP_USERNAME_MAX		32
#define PJSIP_PASSWORD_MAX		32
#define PJSIP_ADDRESS_MAX		32
#define PJSIP_FILE_MAX			64
#define PJSIP_DATA_MAX			128
#define PJSIP_CODEC_MAX			PJSUA_APP_CODEC_MAX


#define PJSIP_ENABLE_DEFAULT			zpl_true
#define PJSIP_PORT_DEFAULT				5060
#define PJSIP_PROXY_PORT_DEFAULT		5060
#define PJSIP_100_REL_DEFAULT			zpl_false
#define PJSIP_PROTO_DEFAULT				PJSIP_PROTO_UDP
#define PJSIP_DTMF_DEFAULT				PJSIP_DTMF_RFC2833
#define PJSIP_EXPIRES_DEFAULT			360
#define PJSIP_DEFAULT_CLOCK_RATE		8000
#define PJSIP_RTP_PORT_DEFAULT			8888

#ifdef PJSUA_DEFAULT_CLOCK_RATE
#undef PJSUA_DEFAULT_CLOCK_RATE
#define PJSUA_DEFAULT_CLOCK_RATE PJSIP_DEFAULT_CLOCK_RATE
#endif

typedef enum pjsip_dtmf_s
{
	PJSIP_DTMF_INFO = 1,
	PJSIP_DTMF_RFC2833,
	PJSIP_DTMF_INBAND,
}pjsip_dtmf_t;

typedef enum pjsip_transport_s
{
	PJSIP_PROTO_UDP,
	PJSIP_PROTO_TCP,
	PJSIP_PROTO_TLS,
	PJSIP_PROTO_DTLS,
}pjsip_transport_t;

typedef enum pjsip_user_mode_s
{
	PJSIP_USER_NONE,
	PJSIP_USER_STANDBY,
	PJSIP_USER_SWITCH,
}pjsip_user_mode_t;

typedef enum pjsip_connect_mode_s
{
	PJSIP_CONNECT_NONE,
	PJSIP_CONNECT_STANDBY,
	PJSIP_CONNECT_SWITCH,
}pjsip_connect_mode_t;

typedef enum pjsip_reg_proxy_s
{
	PJSIP_REGISTER_NONE,
	PJSIP_REGISTER_NO_PROXY,
	PJSIP_REGISTER_OUTBOUND_PROXY,
	PJSIP_REGISTER_ACC_ONLY,
	PJSIP_REGISTER_ALL
}pjsip_reg_proxy_t;

typedef enum pjsip_srtp_s
{
	PJSIP_SRTP_DISABLE,
	PJSIP_SRTP_OPTIONAL,
	PJSIP_SRTP_MANDATORY,
	PJSIP_SRTP_OPTIONAL_DUP,
}pjsip_srtp_t;

typedef enum pjsip_srtp_sec_s
{
	PJSIP_SRTP_SEC_NO,
	PJSIP_SRTP_SEC_TLS,
	PJSIP_SRTP_SEC_SIPS,
}pjsip_srtp_sec_t;

typedef enum pjsip_timer_s
{
	PJSIP_TIMER_INACTIVE,
	PJSIP_TIMER_OPTIONAL,
	PJSIP_TIMER_MANDATORY,
	PJSIP_TIMER_ALWAYS,
}pjsip_timer_t;

typedef enum pjsip_echo_mode_s
{
	PJSIP_ECHO_DISABLE,
	PJSIP_ECHO_DEFAULT,
	PJSIP_ECHO_SPEEX,
	PJSIP_ECHO_SUPPRESSER,
	PJSIP_ECHO_WEBRTXC,
}pjsip_echo_mode_t;

typedef enum pjsip_srtp_keying_s
{
	PJSIP_SRTP_KEYING_SDES,
	PJSIP_SRTP_KEYING_DTLS,
}pjsip_srtp_keying_t;

typedef enum pjsip_accept_redirect_s
{
	PJSIP_ACCEPT_REDIRECT_REJECT,
	PJSIP_ACCEPT_REDIRECT_FOLLOW,
	PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE,
	PJSIP_ACCEPT_REDIRECT_ASK
}pjsip_accept_redirect_t;


typedef enum pjsip_register_state_s
{
	PJSIP_STATE_UNKNOW,
	PJSIP_STATE_UNREGISTER,
	PJSIP_STATE_REGISTER_FAILED,
	PJSIP_STATE_REGISTER_SUCCESS,
}pjsip_register_state_t;


typedef enum pjsip_connect_state_s
{
	PJSIP_STATE_UNCONNECT,
	PJSIP_STATE_CONNECT_FAILED,
	PJSIP_STATE_CONNECT_SUCCESS,
	PJSIP_STATE_CONNECT_LOCAL,
}pjsip_connect_state_t;

typedef enum pjsip_call_state_s
{
	//SIP呼叫状态		SIP层面
	PJSIP_STATE_CALL_IDLE,			//呼叫空闲
	PJSIP_STATE_CALL_TRYING,
	PJSIP_STATE_CALL_RINGING,
	PJSIP_STATE_CALL_PICKING,
	PJSIP_STATE_CALL_FAILED,			//呼叫失败
	PJSIP_STATE_CALL_SUCCESS,			//呼叫建立
	PJSIP_STATE_CALL_CANCELLED,			//
	PJSIP_STATE_CALL_CLOSED,				//
	PJSIP_STATE_CALL_RELEASED,

}pjsip_call_state_t;


typedef enum pjsip_state_s
{
	//信息响应
	PJSIP_STATE_100 	= 100,			//100试呼叫（Trying）
	PJSIP_STATE_180 	= 180,			//180振铃（Ringing）
	PJSIP_STATE_181 	= 181,			//181呼叫正在前转（Call is Being Forwarded）
	PJSIP_STATE_182 	= 182,			//182排队
	//成功响应
	PJSIP_STATE_200	= 200,			//200成功响应（OK）

	//重定向响应
	PJSIP_STATE_300	= 300,			//300多重选择
	PJSIP_STATE_301	= 301,			//永久迁移
	PJSIP_STATE_302	= 302,			//302临时迁移（Moved Temporarily）
	PJSIP_STATE_303	= 303,			//见其它
	PJSIP_STATE_305	= 305,			//使用代理
	PJSIP_STATE_380	= 380,			//代换服务

	//客户出错
	PJSIP_STATE_400 	= 400,		//400错误请求（Bad Request）
	PJSIP_STATE_401 	= 401,		//401未授权（Unauthorized）
	PJSIP_STATE_402 	= 402,		//要求付款
	PJSIP_STATE_403 	= 403,		//403禁止（Forbidden）
	PJSIP_STATE_404	= 404,		//404用户不存在（Not Found）
	PJSIP_STATE_405	= 405,		//不允许的方法
	PJSIP_STATE_406	= 406,		//不接受
	PJSIP_STATE_407	= 407,		//要求代理权
	PJSIP_STATE_408 	= 408,		//408请求超时（Request Timeout）
	PJSIP_STATE_410	= 410,		//消失
	PJSIP_STATE_413	= 413,		//请求实体太大
	PJSIP_STATE_414 	= 414,		//请求URI太大
	PJSIP_STATE_415 	= 414,		//不支持的媒体类型
	PJSIP_STATE_416 	= 416,		//不支持的URI方案
	PJSIP_STATE_420	= 420,		//分机无人接听
	PJSIP_STATE_421	= 421,		//要求转机
	PJSIP_STATE_423 	= 423,		//间隔太短
	PJSIP_STATE_480	= 480,		//480暂时无人接听（Temporarily Unavailable）
	PJSIP_STATE_481 	= 481,		//呼叫腿/事务不存在

	PJSIP_STATE_482 	= 482,		//相环探测
	PJSIP_STATE_483 	= 483,		//跳频太高
	PJSIP_STATE_484 	= 484,		//地址不完整
	PJSIP_STATE_485 	= 485,		//不清楚
	PJSIP_STATE_486	= 486,		//486线路忙（Busy Here）

	PJSIP_STATE_487 	= 487,		//终止请求
	PJSIP_STATE_488 	= 488,		//此处不接受
	PJSIP_STATE_491 	= 491,		//代处理请求
	PJSIP_STATE_493 	= 493,		//难以辨认

	//服务器出错
	PJSIP_STATE_500	= 500,		//内部服务器错误
	PJSIP_STATE_501	= 501,		//没实现的
	PJSIP_STATE_502	= 502,		//无效网关
	PJSIP_STATE_503	= 503,		//不提供此服务
	PJSIP_STATE_504 	= 504,		//504服务器超时（Server Time-out）
	PJSIP_STATE_505	= 505,		//SIP版本不支持
	PJSIP_STATE_513	= 513,		//消息太长

	//全局故障
	PJSIP_STATE_600 = 600,		//600全忙（Busy Everywhere）
	PJSIP_STATE_603 = 603,		//拒绝
	PJSIP_STATE_604 = 604,		//都不存在
	PJSIP_STATE_606 = 606,		//不接受
}pjsip_state_t;


typedef struct pjsip_server_s
{
	zpl_int8		sip_address[PJSIP_ADDRESS_MAX];
	zpl_uint16		sip_port;
	pjsip_connect_state_t state;
}pjsip_server_t;

typedef struct pjsip_username_s
{
	char				sip_phone[PJSIP_NUMBER_MAX];
	char				sip_user[PJSIP_USERNAME_MAX];
	char				sip_password[PJSIP_PASSWORD_MAX];
	pjsip_register_state_t	sip_state;
	pjsip_call_state_t	call_state;
	pjsip_server_t		*register_svr;
	zpl_bool				is_default;
	zpl_uint8				id;
	zpl_bool				is_current;
}pjsip_username_t;

typedef struct pjsip_codec_s
{
	zpl_uint16				payload;
	char				payload_name[PJSIP_NUMBER_MAX];
	zpl_bool				is_active;
}pjsip_codec_t;

typedef struct pl_pjsip_s
{
	zpl_bool				sip_enable;
	//just for app
	zpl_bool				sip_active_standby;
	zpl_bool				sip_multi_user ;

	zpl_bool				sip_proxy_enable;
	zpl_uint16				sip_server_cnt;
	zpl_uint16				sip_proxy_cnt;
	zpl_uint16				sip_user_cnt;

	pjsip_user_mode_t	sip_user_mode;
	pjsip_connect_mode_t sip_connect_mode;

	//"SIP Account options:"
	pjsip_server_t		sip_server;
	pjsip_server_t		sip_server_sec;

	pjsip_server_t		sip_proxy;
	pjsip_server_t		sip_proxy_sec;

	pjsip_username_t	sip_user;
	pjsip_username_t	sip_user_sec;

	pjsip_transport_t	proto;
	char				sip_realm[PJSIP_DATA_MAX];

	//zpl_uint16				sip_reg_timeout;		//Optional registration interval (default %d) useing sip_expires
	zpl_uint16				sip_rereg_delay;		//Optional auto retry registration interval (default %d)
	pjsip_reg_proxy_t	sip_reg_proxy;			//Control the use of proxy settings in REGISTER.0=no proxy, 1=outbound only, 2=acc only, 3=all (default)
	zpl_bool				sip_publish;			//Send presence PUBLISH for this account
	zpl_bool				sip_mwi;				//Subscribe to message summary/waiting indication
	zpl_bool				sip_ims_enable;			//Enable 3GPP/IMS related settings on this account
	pjsip_srtp_t		sip_srtp_mode;			//Use SRTP?  0:disabled, 1:optional, 2:mandatory,3:optional by duplicating media offer (def:0)
	pjsip_srtp_sec_t	sip_srtp_secure;		//SRTP require secure SIP? 0:no, 1:tls, 2:sips (def:1)
	zpl_bool				sip_100_rel;
	pjsip_timer_t		sip_timer;				//Use SIP session timers? (default=1) 0:inactive, 1:optional, 2:mandatory, 3:always"
	zpl_uint16				sip_timer_sec;
	zpl_uint16				sip_outb_rid;			//Set SIP outbound reg-id (default:1)
	zpl_bool				sip_auto_update_nat;	//Where N is 0 or 1 to enable/disable SIP traversal behind symmetric NAT (default 1)
	zpl_bool				sip_stun_disable;		//Disable STUN for this account
	zpl_uint16				sip_expires;


	//Transport Options:
	zpl_bool				sip_ipv6_enable;
	zpl_bool				sip_set_qos;			//Enable QoS tagging for SIP and media.
	pjsip_server_t		sip_local;				//Use the specifed address as SIP and RTP addresses. (Hint: the IP may be the public IP of the NAT/router)
	ifindex_t				sip_source_interface;	//Bind transports to this IP interface
	zpl_bool				sip_noudp;
	zpl_bool				sip_notcp;
	pjsip_server_t		sip_nameserver;			//Add the specified nameserver to enable SRV resolution This option can be specified multiple times.
	pjsip_server_t		sip_outbound;			//Set the URL of global outbound proxy server May be specified multiple times
	pjsip_server_t		sip_stun_server;		//Set STUN server host or domain. This option may be specified more than once. FORMAT is hostdom[:PORT]

	//TLS Options:
	zpl_bool				sip_tls_enable;
	char				sip_tls_ca_file[PJSIP_FILE_MAX];		//Specify TLS CA file (default=none)
	char				sip_tls_cert_file[PJSIP_FILE_MAX];		//Specify TLS certificate file (default=none)
	char				sip_tls_privkey_file[PJSIP_FILE_MAX];	//Specify TLS private key file (default=none)
	char				sip_tls_password[PJSIP_PASSWORD_MAX];	//Specify TLS password to private key file (default=none)
	pjsip_server_t		sip_tls_verify_server;					//Verify server's certificate (default=no)
	pjsip_server_t		sip_tls_verify_client;					//Verify client's certificate (default=no)
	zpl_uint16				sip_neg_timeout;						//Specify TLS negotiation timeout (default=no)
	char				sip_tls_cipher[PJSIP_DATA_MAX];			//Specify prefered TLS cipher (optional).May be specified multiple times


	//Audio Options:
	zpl_uint16				sip_clock_rate;
	zpl_uint16				sip_snd_clock_rate;
	zpl_bool				sip_stereo;
	zpl_bool				sip_audio_null;
	char				sip_play_file[PJSIP_FILE_MAX];
	char				sip_play_tone[PJSIP_DATA_MAX];
	zpl_bool				sip_auto_play;
	zpl_bool				sip_auto_loop;
	zpl_bool				sip_auto_conf;
	char				sip_rec_file[PJSIP_FILE_MAX];
	zpl_uint16				sip_quality;							//Specify media quality (0-10, default=2)
	zpl_uint16				sip_ptime;								//Override codec ptime to MSEC (default=specific)
	zpl_bool				sip_no_vad;								//Disable VAD/silence detector (default=vad enabled)
	zpl_uint16				sip_echo_tail;							//Set echo canceller tail length
	pjsip_echo_mode_t	sip_echo_mode;							//Select echo canceller algorithm (0=default, 1=speex, 2=suppressor, 3=WebRtc)
	zpl_uint16				sip_ilbc_mode;							//Set iLBC codec mode (20 or 30, default is 20)
    char                    capture_dev_name[PJSUA_APP_DEV_NAME_MAX];
    char                    playback_dev_name[PJSUA_APP_DEV_NAME_MAX];

	zpl_int32				sip_capture_dev;
	zpl_int32				sip_playback_dev;
	zpl_uint32				sip_capture_lat;						//Audio capture latency, in ms
	zpl_uint32				sip_playback_lat;						//Audio capture latency, in ms
	zpl_int32				sip_snd_auto_close;						//Auto close audio device when idle for N secs (default=1)
																//		Specify N=-1 to disable this feature. Specify N=0 for instant close when unused.
	zpl_bool				sip_notones;							//Disable audible tones
	zpl_int32				sip_jb_max_size;						//Specify jitter buffer maximum size, in frames (default=-1)");

#if PJSUA_HAS_VIDEO
	//Video Options:
	zpl_bool				sip_video;
	zpl_int32				sip_vcapture_dev;
	zpl_int32				sip_vrender_dev;
	char				sip_play_avi[PJSIP_FILE_MAX];
	zpl_bool				sip_auto_play_avi;
    char                    vcapture_dev_name[PJSUA_APP_DEV_NAME_MAX];
    char                    vrender_dev_name[PJSUA_APP_DEV_NAME_MAX];	
#endif


	//Media Transport Options:
	zpl_bool				sip_ice;				//Enable ICE (default:no)
	zpl_uint32				sip_ice_regular;		//Use ICE regular nomination (default: aggressive)
	zpl_uint16				sip_ice_max_host;		//Set maximum number of ICE host candidates
	zpl_bool				sip_ice_nortcp;			//Disable RTCP component in ICE (default: no)
	zpl_uint16				sip_rtp_port;
	zpl_uint16				sip_rx_drop_pct;		//Drop PCT percent of RX RTP (for pkt lost sim, default: 0)
	zpl_uint16				sip_tx_drop_pct;		//Drop PCT percent of TX RTP (for pkt lost sim, default: 0)
	zpl_bool				sip_turn;				//Enable TURN relay with ICE (default:no)
	pjsip_server_t		sip_turn_srv;			//Domain or host name of TURN server (\"NAME:PORT\" format)
	zpl_bool				sip_turn_tcp;			//Use TCP connection to TURN server (default no)
	char				sip_turn_user[PJSIP_USERNAME_MAX];
	char				sip_turn_password[PJSIP_PASSWORD_MAX];
	zpl_bool				sip_rtcp_mux;			//Enable RTP & RTCP multiplexing (default: no)
	pjsip_srtp_keying_t	sip_srtp_keying;		//SRTP keying method for outgoing SDP offer.0=SDES (default), 1=DTLS

	//Buddy List (can be more than one):
	void				*buddy_list;
	//User Agent options:
	zpl_uint16				sip_auto_answer_code;	//Automatically answer incoming calls with code (e.g. 200)
	zpl_uint16				sip_max_calls;			//Maximum number of concurrent calls (default:4, max:255)
	zpl_uint16				sip_thread_max;			//Number of worker threads (default:1)
	zpl_uint32				sip_duration;			//Set maximum call duration (default:no limit)
	zpl_uint16				sip_norefersub;			//Suppress event subscription when transferring calls

	zpl_uint16				sip_use_compact_form;	//Minimize SIP message size
	zpl_uint16				sip_no_force_lr;		//Allow strict-route to be used (i.e. do not force lr)
	pjsip_accept_redirect_t		sip_accept_redirect;	//Specify how to handle call redirect (3xx) response.
												//	0: reject, 1: follow automatically,
												//	2: follow + replace To header (default), 3: ask

	pjsip_dtmf_t			dtmf;
	/*char				sip_codec[PJSIP_DATA_MAX];
	char				sip_discodec[PJSIP_DATA_MAX];*/
	pjsip_codec_t			sip_codec;	//default sip codec
	pjsip_codec_t			codec[PJSIP_CODEC_MAX];
	pjsip_codec_t			dicodec[PJSIP_CODEC_MAX];
	//zpl_uint16				payload;
	//char				payload_name[PJSIP_NUMBER_MAX];


	zpl_uint32						debug_level;
	zpl_bool					debug_detail;

	int (*app_dtmf_cb)(int , int);

	void				*pjsip;

	void				*mutex;

	void				*userdata;
} pl_pjsip_t;
/************************************************************************/
/************************************************************************/
extern pl_pjsip_t *pl_pjsip;
/************************************************************************/
/************************************************************************/
int pl_pjsip_source_change(struct interface *ifp, zpl_bool change);
/************************************************************************/
char *pl_pjsip_dtmf_name(pjsip_dtmf_t );
char *pl_pjsip_transport_name(pjsip_transport_t );
char *pl_pjsip_reg_proxy_name(pjsip_reg_proxy_t );
char *pl_pjsip_srtp_name(pjsip_srtp_t );
char *pl_pjsip_srtp_sec_name(pjsip_srtp_sec_t );
char *pl_pjsip_timer_name(pjsip_timer_t );
char *pl_pjsip_echo_mode_name(pjsip_echo_mode_t );
char *pl_pjsip_accept_redirect_name(pjsip_accept_redirect_t );
char *pl_pjsip_register_state_name(pjsip_register_state_t );
char *pl_pjsip_connect_state_name(pjsip_connect_state_t );
char *pl_pjsip_call_state_name(pjsip_call_state_t );
/************************************************************************/
int pl_pjsip_global_set_api(zpl_bool enable);
int pl_pjsip_global_get_api(zpl_bool *enable);
zpl_bool pl_pjsip_global_isenable(void);

int pl_pjsip_server_set_api(zpl_int8 *ip, zpl_uint16 port, zpl_bool sec);
int pl_pjsip_server_get_api(zpl_int8 *ip, zpl_uint16 *port, zpl_bool sec);

int pl_pjsip_proxy_set_api(zpl_int8 *ip, zpl_uint16 port, zpl_bool sec);
int pl_pjsip_proxy_get_api(zpl_int8 *ip, zpl_uint16 *port, zpl_bool sec);

int pl_pjsip_local_address_set_api(zpl_int8 *address);
int pl_pjsip_local_address_get_api(zpl_int8 *address);

int pl_pjsip_source_interface_set_api(ifindex_t ifindex);
int pl_pjsip_source_interface_get_api(ifindex_t *ifindex);

int pl_pjsip_local_port_set_api(zpl_uint16 port);
int pl_pjsip_local_port_get_api(zpl_uint16 *port);

int pl_pjsip_transport_proto_set_api(pjsip_transport_t proto);
int pl_pjsip_transport_proto_get_api(pjsip_transport_t *proto);

int pl_pjsip_dtmf_set_api(pjsip_dtmf_t dtmf);
int pl_pjsip_dtmf_get_api(pjsip_dtmf_t *dtmf);

int pl_pjsip_username_set_api(zpl_int8 *user, zpl_int8 *pass, zpl_bool sec);
int pl_pjsip_username_get_api(zpl_int8 *user, zpl_int8 *pass, zpl_bool sec);

int pl_pjsip_phonenumber_set_api(zpl_int8 *sip_phone, zpl_bool sec);
int pl_pjsip_phonenumber_get_api(zpl_int8 *sip_phone, zpl_bool sec);

int pl_pjsip_expires_set_api(zpl_uint16 sip_expires);
int pl_pjsip_expires_get_api(zpl_uint16 *sip_expires);

int pl_pjsip_100rel_set_api(zpl_bool sip_100_rel);
int pl_pjsip_100rel_get_api(zpl_bool *sip_100_rel);

int pl_pjsip_realm_set_api(char *realm);
int pl_pjsip_realm_get_api(char *realm);

/*
int pl_pjsip_registration_interval_set_api(zpl_uint16 sip_reg_timeout);
int pl_pjsip_registration_interval_get_api(zpl_uint16 *sip_reg_timeout);
*/

int pl_pjsip_reregist_delay_set_api(zpl_uint16 sip_rereg_delay);
int pl_pjsip_reregist_delay_get_api(zpl_uint16 *sip_rereg_delay);

int pl_pjsip_reregister_proxy_set_api(pjsip_reg_proxy_t sip_reg_proxy);
int pl_pjsip_reregister_proxy_get_api(pjsip_reg_proxy_t *sip_reg_proxy);

int pl_pjsip_publish_set_api(zpl_bool sip_publish);
int pl_pjsip_publish_get_api(zpl_bool *sip_publish);

int pl_pjsip_mwi_set_api(zpl_bool sip_mwi);
int pl_pjsip_mwi_get_api(zpl_bool *sip_mwi);

int pl_pjsip_ims_set_api(zpl_bool sip_ims_enable);
int pl_pjsip_ims_get_api(zpl_bool *sip_ims_enable);

int pl_pjsip_srtp_mode_set_api(pjsip_srtp_t sip_srtp_mode);
int pl_pjsip_srtp_mode_get_api(pjsip_srtp_t *sip_srtp_mode);

int pl_pjsip_srtp_secure_set_api(pjsip_srtp_sec_t sip_srtp_secure);
int pl_pjsip_srtp_secure_get_api(pjsip_srtp_sec_t *sip_srtp_secure);

int pl_pjsip_timer_set_api(pjsip_timer_t sip_timer);
int pl_pjsip_timer_get_api(pjsip_timer_t *sip_timer);

int pl_pjsip_timer_sec_set_api(zpl_uint16 sip_timer_sec);
int pl_pjsip_timer_sec_get_api(zpl_uint16 *sip_timer_sec);

int pl_pjsip_outb_rid_set_api(zpl_uint16 sip_outb_rid);
int pl_pjsip_outb_rid_get_api(zpl_uint16 *sip_outb_rid);

int pl_pjsip_auto_update_nat_set_api(zpl_bool sip_auto_update_nat);
int pl_pjsip_auto_update_nat_get_api(zpl_bool *sip_auto_update_nat);

int pl_pjsip_stun_set_api(zpl_bool enable);
int pl_pjsip_stun_get_api(zpl_bool *enable);
/***************************************************************************/
//Transport Options:
int pl_pjsip_ipv6_set_api(zpl_bool enable);
int pl_pjsip_ipv6_get_api(zpl_bool *enable);

int pl_pjsip_qos_set_api(zpl_bool enable);
int pl_pjsip_qos_get_api(zpl_bool *enable);

int pl_pjsip_noudp_set_api(zpl_bool enable);
int pl_pjsip_noudp_get_api(zpl_bool *enable);

int pl_pjsip_notcp_set_api(zpl_bool enable);
int pl_pjsip_notcp_get_api(zpl_bool *enable);

int pl_pjsip_nameserver_set_api(char * address, zpl_uint16 port);
int pl_pjsip_nameserver_get_api(char * address, zpl_uint16 *port);

int pl_pjsip_outbound_set_api(char * address, zpl_uint16 port);
int pl_pjsip_outbound_get_api(char * address, zpl_uint16 *port);

int pl_pjsip_stun_server_set_api(char * address, zpl_uint16 port);
int pl_pjsip_stun_server_get_api(char * address, zpl_uint16 *port);
/***************************************************************************/
//TLS Options:
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
int pl_pjsip_tls_set_api(zpl_bool enable);
int pl_pjsip_tls_get_api(zpl_bool *enable);

int pl_pjsip_tls_ca_set_api(char * filename);
int pl_pjsip_tls_ca_get_api(char * filename);

int pl_pjsip_tls_cert_set_api(char * filename);
int pl_pjsip_tls_cert_get_api(char * filename);

int pl_pjsip_tls_privkey_set_api(char * filename);
int pl_pjsip_tls_privkey_get_api(char * filename);

int pl_pjsip_tls_password_set_api(char * password);
int pl_pjsip_tls_password_get_api(char * password);

int pl_pjsip_tls_verify_server_set_api(char * address, zpl_uint16 port);
int pl_pjsip_tls_verify_server_get_api(char * address, zpl_uint16 *port);

int pl_pjsip_tls_verify_client_set_api(char * address, zpl_uint16 port);
int pl_pjsip_tls_verify_client_get_api(char * address, zpl_uint16 *port);

int pl_pjsip_tls_cipher_set_api(char * cipher);
int pl_pjsip_tls_cipher_get_api(char * cipher);
#endif
int pl_pjsip_neg_timeout_set_api(zpl_uint16 sip_neg_timeout);
int pl_pjsip_neg_timeout_get_api(zpl_uint16 *sip_neg_timeout);
/***************************************************************************/
//Audio Options:
int pl_pjsip_codec_add_api(char * sip_codec);
int pl_pjsip_codec_del_api(char * sip_codec);
int pl_pjsip_codec_default_set_api(char * sip_codec);

int pl_pjsip_discodec_add_api(char * sip_discodec);
int pl_pjsip_discodec_del_api(char * sip_discodec);

int pl_pjsip_payload_name_add_api(char * value);
int pl_pjsip_payload_name_del_api(char * value);
int pl_pjsip_dis_payload_name_add_api(char * value);
int pl_pjsip_dis_payload_name_del_api(char * value);


int pl_pjsip_clock_rate_set_api(zpl_uint16 sip_clock_rate);
int pl_pjsip_clock_rate_get_api(zpl_uint16 *sip_clock_rate);

int pl_pjsip_snd_clock_rate_set_api(zpl_uint16 sip_snd_clock_rate);
int pl_pjsip_snd_clock_rate_get_api(zpl_uint16 *sip_snd_clock_rate);

int pl_pjsip_stereo_set_api(zpl_bool enable);
int pl_pjsip_stereo_get_api(zpl_bool *enable);

int pl_pjsip_audio_null_set_api(zpl_bool enable);
int pl_pjsip_audio_null_get_api(zpl_bool *enable);

int pl_pjsip_play_file_set_api(char * filename);
int pl_pjsip_play_file_get_api(char * filename);

int pl_pjsip_play_tone_set_api(char * filename);
int pl_pjsip_play_tone_get_api(char * filename);

int pl_pjsip_auto_play_set_api(zpl_bool enable);
int pl_pjsip_auto_play_get_api(zpl_bool *enable);

int pl_pjsip_auto_loop_set_api(zpl_bool enable);
int pl_pjsip_auto_loop_get_api(zpl_bool *enable);

int pl_pjsip_auto_conf_set_api(zpl_bool enable);
int pl_pjsip_auto_conf_get_api(zpl_bool *enable);

int pl_pjsip_rec_file_set_api(char * filename);
int pl_pjsip_rec_file_get_api(char * filename);

int pl_pjsip_quality_set_api(zpl_uint16 sip_quality);
int pl_pjsip_quality_get_api(zpl_uint16 *sip_quality);

int pl_pjsip_ptime_set_api(zpl_uint16 sip_ptime);
int pl_pjsip_ptime_get_api(zpl_uint16 *sip_ptime);

int pl_pjsip_no_vad_set_api(zpl_bool enable);
int pl_pjsip_no_vad_get_api(zpl_bool *enable);

int pl_pjsip_echo_tail_set_api(zpl_uint16 sip_echo_tail);
int pl_pjsip_echo_tail_get_api(zpl_uint16 *sip_echo_tail);

int pl_pjsip_echo_mode_set_api(pjsip_echo_mode_t sip_echo_mode);
int pl_pjsip_echo_mode_get_api(pjsip_echo_mode_t *sip_echo_mode);

int pl_pjsip_ilbc_mode_set_api(zpl_uint16 sip_ilbc_mode);
int pl_pjsip_ilbc_mode_get_api(zpl_uint16 *sip_ilbc_mode);

int pl_pjsip_capture_lat_set_api(zpl_uint16 sip_capture_lat);
int pl_pjsip_capture_lat_get_api(zpl_uint16 *sip_capture_lat);

int pl_pjsip_playback_lat_set_api(zpl_uint16 sip_playback_lat);
int pl_pjsip_playback_lat_get_api(zpl_uint16 *sip_playback_lat);

int pl_pjsip_auto_close_delay_set_api(zpl_int32 sip_snd_auto_close);
int pl_pjsip_auto_close_delay_get_api(zpl_int32 *sip_snd_auto_close);

int pl_pjsip_no_tones_set_api(zpl_bool enable);
int pl_pjsip_no_tones_get_api(zpl_bool *enable);

int pl_pjsip_jb_max_size_set_api(zpl_int32 sip_jb_max_size);
int pl_pjsip_jb_max_size_get_api(zpl_int32 *sip_jb_max_size);
/************************************************************************/
#if PJSUA_HAS_VIDEO
int pl_pjsip_video_enable_set_api(zpl_bool enable);
int pl_pjsip_video_enable_get_api(zpl_bool *enable);

int pl_pjsip_video_play_file_set_api(char * filename);
int pl_pjsip_video_play_file_get_api(char * filename);

int pl_pjsip_video_auto_play_set_api(zpl_bool enable);
int pl_pjsip_video_auto_play_get_api(zpl_bool *enable);
#endif

/************************************************************************/
int pl_pjsip_ice_enable_set_api(zpl_bool enable);
int pl_pjsip_ice_enable_get_api(zpl_bool *enable);

int pl_pjsip_ice_nortcp_set_api(zpl_bool enable);
int pl_pjsip_ice_nortcp_get_api(zpl_bool *enable);

int pl_pjsip_ice_regular_set_api(zpl_uint32 value);
int pl_pjsip_ice_regular_get_api(zpl_uint32 *value);

int pl_pjsip_ice_max_host_set_api(zpl_uint32 value);
int pl_pjsip_ice_max_host_get_api(zpl_uint32 *value);

int pl_pjsip_rtp_port_set_api(zpl_uint16 value);
int pl_pjsip_rtp_port_get_api(zpl_uint16 *value);

int pl_pjsip_rx_drop_pct_set_api(zpl_uint16 value);
int pl_pjsip_rx_drop_pct_get_api(zpl_uint16 *value);

int pl_pjsip_tx_drop_pct_set_api(zpl_uint16 value);
int pl_pjsip_tx_drop_pct_get_api(zpl_uint16 *value);

int pl_pjsip_turn_enable_set_api(zpl_bool enable);
int pl_pjsip_turn_enable_get_api(zpl_bool *enable);

int pl_pjsip_turn_server_set_api(char * address, zpl_uint16 port);
int pl_pjsip_turn_server_get_api(char * address, zpl_uint16 *port);

int pl_pjsip_turn_tcp_set_api(zpl_bool enable);
int pl_pjsip_turn_tcp_get_api(zpl_bool *enable);

int pl_pjsip_turn_username_set_api(char * username);
int pl_pjsip_turn_username_get_api(char * username);

int pl_pjsip_turn_password_set_api(char * password);
int pl_pjsip_turn_password_get_api(char * password);

int pl_pjsip_rtcp_mux_set_api(zpl_bool value);
int pl_pjsip_rtcp_mux_get_api(zpl_bool *value);

int pl_pjsip_srtp_keying_set_api(pjsip_srtp_keying_t value);
int pl_pjsip_srtp_keying_get_api(pjsip_srtp_keying_t *value);

//User Agent options:
int pl_pjsip_auto_answer_code_set_api(zpl_uint16 value);
int pl_pjsip_auto_answer_code_get_api(zpl_uint16 *value);

int pl_pjsip_max_calls_set_api(zpl_uint16 value);
int pl_pjsip_max_calls_get_api(zpl_uint16 *value);

int pl_pjsip_max_thread_set_api(zpl_uint16 value);
int pl_pjsip_max_thread_get_api(zpl_uint16 *value);

int pl_pjsip_duration_set_api(zpl_uint32 value);
int pl_pjsip_duration_get_api(zpl_uint32 *value);

int pl_pjsip_norefersub_set_api(zpl_uint16 value);
int pl_pjsip_norefersub_get_api(zpl_uint16 *value);

int pl_pjsip_use_compact_form_set_api(zpl_uint16 value);
int pl_pjsip_use_compact_form_get_api(zpl_uint16 *value);

int pl_pjsip_no_force_lr_set_api(zpl_uint16 value);
int pl_pjsip_no_force_lr_get_api(zpl_uint16 *value);

int pl_pjsip_accept_redirect_set_api(pjsip_accept_redirect_t value);
int pl_pjsip_accept_redirect_get_api(pjsip_accept_redirect_t *value);
/************************************************************************/
int pl_pjsip_debug_level_set_api(zpl_uint32 level);
int pl_pjsip_debug_level_get_api(zpl_uint32 *level);
int pl_pjsip_debug_detail_set_api(zpl_bool enable);
int pl_pjsip_debug_detail_get_api(zpl_bool *enable);
/************************************************************************/
int pl_pjsip_account_set_api(pjsua_acc_id id, void *p);
int pl_pjsip_account_get_api(pjsua_acc_id id, pjsip_username_t *p);
zpl_bool pl_pjsip_isregister_api(void);

/************************************************************************/
/************************************************************************/
int pl_pjsip_app_add_acc(char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass, pjsua_acc_id *accid);
int pl_pjsip_app_del_acc(pjsua_acc_id accid);
int pl_pjsip_app_mod_acc(pjsua_acc_id accid, char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass);
int pl_pjsip_app_select_acc(pjsua_acc_id accid, zpl_uint32 type);
int pl_pjsip_app_reg_acc(zpl_bool reg);
int pl_pjsip_app_list_acc(pjsua_acc_id accid);
/************************************************************************/
/************************************************************************/
int pl_pjsip_app_start_call(pjsua_acc_id accid, char *num, pjsua_call_id *callid);
int pl_pjsip_app_stop_call(pjsua_call_id callid, zpl_bool all);
int pl_pjsip_app_answer_call(pjsua_call_id callid, zpl_uint32 st_code);
int pl_pjsip_app_hold_call(pjsua_call_id callid);
int pl_pjsip_app_reinvite_call(pjsua_call_id callid);
int pl_pjsip_app_dtmf_call(pjsua_call_id callid, zpl_uint32 type, zpl_uint32 code);
int pl_pjsip_app_select_call(pjsua_call_id callid, zpl_uint32 type);
/************************************************************************/
/************************************************************************/
int pl_pjsip_multiuser_set_api(zpl_bool enable);
zpl_bool pl_pjsip_multiuser_get_api(void);
int pl_pjsip_active_standby_set_api(zpl_bool enable);
zpl_bool pl_pjsip_active_standby_get_api(void);
/************************************************************************/
/************************************************************************/
int pl_pjsip_show_account_state(void *p);
int pl_pjsip_write_config(void *p);
int pl_pjsip_show_config(void *p, zpl_bool detail);
void cmd_voip_init(void);
void cmd_voip_test_init(int node);
/************************************************************************/
int pl_pjsip_module_init(void);
int pl_pjsip_module_exit(void);
int pl_pjsip_module_task_init(void);
int pl_pjsip_module_task_exit(void);
int pl_pjsip_json_test(void);
/************************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_APP_API_H__ */

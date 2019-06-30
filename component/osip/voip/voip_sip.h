/*
 * voip_sip.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __VOIP_SIP_H__
#define __VOIP_SIP_H__

//#include "config.h"
//#include "voip_state.h"
/*
#include "voip_def.h"
#include "voip_event.h"
#include "voip_state.h"
#include "voip_app.h"

*/
#define _VOIP_SIP_DEBUG
#ifdef _VOIP_SIP_DEBUG
#define VOIP_SIP_DEBUG(fmt,...)		zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)
#else
#define VOIP_SIP_DEBUG(fmt,...)
#endif


#define SIP_NUMBER_MAX			32
#define SIP_USERNAME_MAX		32
#define SIP_PASSWORD_MAX		32
#define SIP_ADDRESS_MAX			32
#define SIP_DATA_MAX		128

#define SIP_ENABLE_DEFAULT			TRUE
#define SIP_PORT_DEFAULT			5060
#define SIP_PORT_SEC_DEFAULT		5060
#define SIP_PROXY_PORT_DEFAULT		5060
#define SIP_PROXY_PORT_SEC_DEFAULT	5060
#define SIP_TIME_DEFAULT			TRUE
#define SIP_RING_DEFAULT			1
#define SIP_REGINTER_DEFAULT		1200
#define SIP_HOSTPART_DEFAULT		"tslsmart"
#define SIP_INTERVAL_DEFAULT		8
#define SIP_DIALPLAN_DEFAULT		"tslsmart"
#define SIP_REALM_DEFAULT			"tslsmart"
#define SIP_ENCRYPT_DEFAULT			TRUE
#define SIP_DIS_NAME_DEFAULT		TRUE
#define SIP_100_REL_DEFAULT			TRUE

#define SIP_PHONE_DEFAULT			"1003"
#define SIP_USERNAME_DEFAULT		"1003"
#define SIP_PASSWORD_DEFAULT		"0003"
#define SIP_DTMF_DEFAULT			VOIP_SIP_INFO



//#define OSIP_LOAD_DEBUG



typedef enum sip_dtmf_s
{
	VOIP_SIP_INFO = 1,
	VOIP_SIP_RFC2833,
	VOIP_SIP_INBAND,
}sip_dtmf_t;



typedef enum sip_transport_s
{
	SIP_PROTO_UDP,
	SIP_PROTO_TCP,
	SIP_PROTO_TLS,
	SIP_PROTO_DTLS,
}sip_transport_t;

typedef struct voip_sip_s
{
	BOOL				sip_enable;

	u_int32				sip_server;					//SIP
	u_int16				sip_port;					//SIPַַ

	BOOL				sip_active_standby;
	BOOL				sip_keepalive;				//
	u_int16				sip_keepalive_interval;

	u_int32				sip_server_sec;				//SIP
	u_int16				sip_port_sec;				//SIP

	BOOL				sip_proxy_enable;
	u_int32				sip_proxy_server;			//ַ
	u_int16				sip_proxy_port;				//

	u_int32				sip_proxy_server_sec;		//
	u_int16				sip_proxy_port_sec;			//

	u_int16				sip_local_port;				//
	u_int32				sip_local_address;
	u_int32				sip_source_interface;

	BOOL				sip_multi_user ;

	char				sip_local_number[SIP_NUMBER_MAX];	//
	char				sip_user[SIP_USERNAME_MAX];
	char				sip_password[SIP_PASSWORD_MAX];				//

	char				sip_local_number_alias[SIP_NUMBER_MAX];	//
	char				sip_user_alias[SIP_USERNAME_MAX];
	char				sip_password_alias[SIP_PASSWORD_MAX];				//

	u_int16				sip_register_interval;		//
	BOOL				sip_100_rel;				//

	char				sip_realm[SIP_DATA_MAX];				//realm
	char				sip_realm_alias[SIP_DATA_MAX];				//realm

	BOOL				sip_time_sync;				//
	u_int16				sip_ring;					//
	char				sip_hostpart[SIP_DATA_MAX];	//热线
	u_int16				sip_interval;				//
	BOOL				sip_dis_name;				//display name
	char				sip_dialplan[SIP_DATA_MAX];				//dialplan
	BOOL				sip_encrypt;				//

	sip_dtmf_t			dtmf;
	sip_transport_t		proto;
	u_int16				payload;
	char				payload_name[SIP_NUMBER_MAX];

	//BOOL				bClouds;			//当前是否注册在云端

	int (*app_dtmf_cb)(int , int);

	BOOL				bRegMain;			//主号码注册状态
	BOOL				bRegStanby;			//备号码注册状态
	/*
	 * 用于切换
	 */
	BOOL				bUserMain;			//当前是否使用主号码
	BOOL				bSipMain;			//当前是否使用主服务器

	void				*t_event;
	void				*osip;

	void				*mutex;

	//int					debug;

} voip_sip_t;


typedef struct voip_payload_s
{
	char 		rtpmap[SIP_NUMBER_MAX*2];
	u_int8 		ptime;
	char 		name[SIP_NUMBER_MAX];
	u_int8 		payload;
}voip_payload_t;


extern voip_sip_t *sip_config;


extern int voip_sip_module_init();
extern int voip_sip_module_exit();

extern int voip_sip_module_task_exit();
extern int voip_sip_module_task_exit();
extern int voip_sip_dtmf_cb_api(int (*cb)(int, int));
/*
 * SIP config Module
 */
extern int voip_sip_enable(BOOL enable);
extern BOOL voip_sip_isenable(void);

extern BOOL voip_sip_main_regstate(void);
extern BOOL voip_sip_stanby_regstate(void);

extern BOOL voip_sip_is_usemain(void);
extern BOOL voip_sip_is_sipmain(void);

extern int voip_sip_server_set_api(u_int32 ip, u_int16 port, BOOL sec);
extern int voip_sip_server_get_api(u_int32 *ip, u_int16 *port, BOOL sec);

extern int voip_sip_proxy_server_set_api(u_int32 ip, u_int16 port, BOOL sec);
extern int voip_sip_proxy_server_get_api(u_int32 *ip, u_int16 *port, BOOL sec);

extern int voip_sip_local_number_set_api(char * value, BOOL secondary);
extern int voip_sip_local_number_get_api(char * value, BOOL secondary);

extern int voip_sip_password_set_api(char * value, BOOL secondary);
extern int voip_sip_password_get_api(char * value, BOOL secondary);

extern int voip_sip_user_set_api(char * value, BOOL secondary);
extern int voip_sip_user_get_api(char * value, BOOL secondary);

extern int voip_sip_multiuser_set_api(BOOL enable);
extern BOOL voip_sip_multiuser_get_api();

extern int voip_sip_active_standby_set_api(BOOL enable);
extern BOOL voip_sip_active_standby_get_api();

extern int voip_sip_proxy_enable_set_api(BOOL enable);
extern BOOL voip_sip_proxy_enable_get_api();

extern int voip_sip_keepalive_set_api(BOOL enable);
extern BOOL voip_sip_keepalive_get_api();

extern int voip_sip_keepalive_interval_set_api(u_int16 value);
extern int voip_sip_keepalive_interval_get_api();

extern int voip_sip_local_address_set_api(u_int32 address);
extern int voip_sip_local_address_get_api(u_int32 *address);

extern int voip_sip_source_interface_set_api(u_int32 ifindex);
extern int voip_sip_source_interface_get_api(u_int32 *ifindex);

extern int voip_sip_local_port_set_api(u_int16 port);
extern int voip_sip_local_port_get_api(u_int16 *port);

extern int voip_sip_dtmf_set_api(sip_dtmf_t dtmf);
extern int voip_sip_dtmf_get_api(sip_dtmf_t *dtmf);

extern int voip_sip_transport_proto_set_api(sip_transport_t proto);
extern int voip_sip_transport_proto_get_api(sip_transport_t *proto);

extern int voip_sip_register_interval_set_api(u_int16 value);
extern u_int16 voip_sip_register_interval_get_api(u_int16 value);

extern int voip_sip_interval_set_api(u_int16 value);
extern u_int16 voip_sip_interval_get_api(void);

extern int voip_sip_100_rel_set_api(BOOL value);
extern BOOL voip_sip_100_rel_get_api(void);

extern int voip_sip_realm_set_api(char * value, BOOL secondary);
extern int voip_sip_realm_get_api(char * value, BOOL secondary);




extern int voip_sip_time_syne_set_api(BOOL enable);
extern BOOL voip_sip_time_syne_get_api(void);

extern int voip_sip_ring_set_api(u_int16 value);

extern int voip_sip_hostpart_set_api(char * value);
extern int voip_sip_hostpart_get_api(char * value);

extern int voip_sip_display_name_set_api(BOOL value);
extern BOOL voip_sip_display_name_get_api(void);


extern int voip_sip_dialplan_set_api(char * value);
extern int voip_sip_dialplan_get_api(char * value);

extern int voip_sip_encrypt_set_api(BOOL value);
extern BOOL voip_sip_encrypt_get_api(void);


extern int voip_sip_payload_name_set_api(char * value);

extern int voip_sip_get_payload_index(void);
extern int voip_sip_payload_index(char *name);
extern char * voip_sip_payload_name(int index);
extern char * voip_sip_payload_rtpmap(int index);
extern int voip_sip_payload_ptime(int index);


#define VOIP_PAYLOAD_STR	\
	"(pcmu|" \
	"pcma|" \
	"l016|" \
	"lpc|" \
	"g729|" \
	"speex-nb|" \
	"speex-wb|" \
	"ilbc|" \
	"l015|" \
	"amr|" \
	"g722|" \
	"opus)"

#define VOIP_PAYLOAD_STR_HELP	\
	"PCMU\n" \
	"PCMA\n" \
	"L016\n" \
	"LPC\n" \
	"G729\n" \
	"SPEEX-NB\n" \
	"SPEEX-WB\n" \
	"iLBC\n" \
	"L015\n" \
	"AMR\n" \
	"G722\n" \
	"OPUS\n"




extern int voip_sip_config_load(voip_sip_t *sip);


/*
extern int voip_sip_register_start(BOOL reg);
extern int voip_sip_call_start(void *app, int id, char *usename, char *phone);
extern int voip_sip_call_stop(void *app, int id);
*/

extern int voip_sip_config_update_api(voip_sip_t *sip);





/*
 * cmd module
 */
extern void cmd_sip_init(void);
extern int voip_sip_write_config(struct vty *vty);
extern int voip_sip_show_config(struct vty *vty, BOOL detail);


#endif /* __VOIP_SIP_H__ */

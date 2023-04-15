/*
 * voip_app.h
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_APP_H__
#define __VOIP_APP_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "pjsip_app_api.h"
#include "voip_volume.h"

#define VOIP_APP_GET_PHONE
//#define VOIP_APP_GET_ROOM

#define V_APP_DEBUG(fmt,...)		zlog_debug(MODULE_PJSIP, fmt, ##__VA_ARGS__)

#define VOIP_APP_DEBUG

//#define VOIP_PHONE_MAX	32

//#define OSIP_MULTI_CALL_MAX	8
#define VOIP_MULTI_CALL_MAX	8
#define VOIP_MULTI_CALL_NUMBER_MAX	8

#define VOIP_APP_DEBUG_EVENT	0X01
#define VOIP_APP_DEBUG_SIP		0X02
#define VOIP_APP_DEBUG_RECV		0X04
#define VOIP_APP_DEBUG_SEND		0X08
#define VOIP_APP_DEBUG_DETAIL	0X1000


#define VOIP_APP_DEBUG(n)		(VOIP_APP_DEBUG_ ## n & voip_app->debug)
#define VOIP_APP_DEBUG_ON(n)	(voip_app->debug |= (VOIP_APP_DEBUG_ ## n ))
#define VOIP_APP_DEBUG_OFF(n)	(voip_app->debug &= ~(VOIP_APP_DEBUG_ ## n ))


#define VOIP_EVENT_NAME_MAX		128
#define VOIP_EVENT_DATA_MAX		1024

typedef enum
{
	//VOIP_EVENT_NONE,
	VOIP_EVENT_READY,
	VOIP_EVENT_TIMER,
	VOIP_EVENT_HIGH,
	VOIP_EVENT_EXECUTE,
	VOIP_EVENT_UNUSE,
}voip_event_type_t;

typedef struct voip_event_s voip_event_t;

typedef struct voip_event_s
{
	NODE		node;
	voip_event_type_t type;
	int			(*ev_cb)(voip_event_t *);
	void		*pVoid;
	zpl_uint8		data[VOIP_EVENT_DATA_MAX];
	zpl_uint8		dlen;
	zpl_uint16		interval;
	zpl_uint32		timer;
	char 		entry_name[VOIP_EVENT_NAME_MAX];
}voip_event_t;

#define VOIP_EVENT_ARGV(n)	(((voip_event_t *)(n))->pVoid)




typedef enum
{
	APP_CALL_ID_NONE,
	APP_CALL_ID_UI,
	APP_CALL_ID_CLI,
	APP_CALL_ID_WEB,
}app_call_source_t;


typedef struct
{
	char 			phone[PJSIP_NUMBER_MAX];
	char 			username[PJSIP_USERNAME_MAX];
	char 			user_id[PJSIP_USERNAME_MAX];
}call_phone_t;



typedef struct voip_app_s voip_app_t;

typedef struct voip_call_s
{
	zpl_bool			active;
	zpl_uint16			room_number;		//呼叫实例内房间号

	zpl_uint16			num;				//呼叫实例内号码数量
	call_phone_t	phonetab[VOIP_MULTI_CALL_NUMBER_MAX];
	zpl_uint16			index;				//当前呼叫号码索引
	zpl_uint32			time_id;			//定时器ID
	int				time_interval;		//定时间隔（多长时间没有人接听）
#define APP_RINGING_TIME_INTERVAL		50000//15 SEC

	zpl_uint32			start_timer;		//呼叫接通时间
	zpl_uint32			open_timer;			//呼叫开门时间
	zpl_uint32			stop_timer;			//呼叫挂断时间

	app_call_source_t 	source;			//呼叫来源

	zpl_bool			talking;			//正在通话中
	zpl_uint8			building;			//楼栋编号
	zpl_uint8			unit;				//单元编号

	int				instance;			//呼叫实例编号

	voip_app_t		*app;
	zpl_bool			local_stop;		//local stop
}voip_call_t;

typedef enum
{
	//APP通话状态	APP层面
	APP_STATE_TALK_IDLE,			//通话空闲
	APP_STATE_TALK_FAILED,			//通话建立失败
	APP_STATE_TALK_CALLING,			//呼叫中
	APP_STATE_TALK_SUCCESS,			//通话建立
	APP_STATE_TALK_RUNNING,			//通话中
}voip_app_state_t;


typedef struct voip_call_incoming_s
{
	zpl_uint16			room_number;		//呼叫实例内房间号
	zpl_uint8			building;			//楼栋编号
	zpl_uint8			unit;				//单元编号

	zpl_uint32			start_timer;		//呼叫接通时间
	zpl_uint32			open_timer;			//呼叫开门时间
	zpl_uint32			stop_timer;			//呼叫挂断时间

	call_phone_t 	phone;

	char 			remote_ip[PJSIP_USERNAME_MAX];
	zpl_uint16 		port;
	char 			remote_proto[PJSIP_USERNAME_MAX];
}voip_call_incoming_t;

typedef struct voip_app_s
{
	void				*pjsip;
	voip_volume_t 		voip_volume;

	voip_app_state_t	state;

	voip_call_t			*call_session[VOIP_MULTI_CALL_MAX];//呼叫实例
	voip_call_t			*session;		//当前呼叫实例
	zpl_uint16				call_index;
	zpl_bool				stop_and_next;

	voip_call_incoming_t			*incoming_session;

	void				*x5b_app;

	zpl_bool				local_stop;		//local stop

	int					debug;
}voip_app_t;





extern voip_app_t  *voip_app;


extern int void_module_init(pl_pjsip_t *pj);
extern int void_module_exit(pl_pjsip_t *pj);
extern int void_module_task_init(void);
extern int void_module_task_exit(void);

#ifdef ZPL_OPENWRT_UCI
extern int pl_pjsip_module_reload(void);
#endif
extern voip_app_state_t voip_app_state_get(voip_app_t *osip);
extern int voip_app_state_set(voip_app_t *osip, voip_app_state_t state);

extern zpl_bool voip_app_already_call(voip_app_t *app);

extern int voip_app_sip_register_start(zpl_bool reg);

extern voip_call_t * voip_app_call_session_lookup_by_number(voip_app_t *app, char *number);
extern voip_call_t * voip_app_call_session_lookup_by_instance(voip_app_t *app, int instance);


//extern void * voip_app_call_ID_instance_lookup(voip_call_t *call, int instance);

//int voip_app_call_next_number(void *p);

extern int voip_app_call_make(voip_call_t *call, app_call_source_t source, zpl_uint8 building,
		zpl_uint8 unit, zpl_uint16 room);

//extern int voip_app_multi_call_next();


extern zpl_bool voip_app_call_event_from_cli_web(void);
extern zpl_bool voip_app_call_event_from_ui(void);

extern int voip_app_start_call_event_ui(voip_event_t *ev);
extern int voip_app_stop_call_event_ui(voip_event_t *ev);
extern int voip_app_start_call_event_ui_phone(voip_event_t *ev);
extern int voip_app_start_call_event_ui_user(voip_event_t *ev);

extern int voip_app_start_call_event_cli_web(app_call_source_t source, zpl_uint8 building,
		zpl_uint8 unit, zpl_uint16 room, char *number);

extern int voip_app_stop_call_event_cli_web(voip_call_t *call);

/*extern int voip_app_call_spilt_from_web(char *input, zpl_uint8 *building,
		zpl_uint8 *unit, zpl_uint16 *room, char *phonelist, int len);*/


extern void *voip_app_call_event_current(void);

/*
int voip_app_dtmf_recv_callback(int id, void *p, int input);
int voip_app_register_state_callback(int id, void *p, int input);
int voip_app_call_state_callback(int id, void *p, int input);
*/

extern int voip_app_debug_set_api(int);
extern int voip_app_debug_get_api(void);


#ifdef __cplusplus
}
#endif

#endif /* __VOIP_APP_H__ */

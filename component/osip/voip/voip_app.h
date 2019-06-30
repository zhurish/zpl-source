/*
 * voip_app.h
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_APP_H__
#define __VOIP_APP_H__


#include "voip_event.h"
#include "voip_sip.h"
#include "voip_socket.h"
#include "voip_volume.h"
#include "voip_stream.h"


#define VOIP_APP_GET_PHONE
//#define VOIP_APP_GET_ROOM

#define V_APP_DEBUG(fmt,...)		zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)

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


//typedef struct voip_stream_remote_s voip_stream_remote_t;


typedef enum
{
	APP_CALL_ID_NONE,
	APP_CALL_ID_UI,
	APP_CALL_ID_CLI,
	APP_CALL_ID_WEB,
}app_call_source_t;


typedef struct
{
	char 			phone[SIP_NUMBER_MAX];
	char 			username[SIP_USERNAME_MAX];
	char 			user_id[SIP_USERNAME_MAX];
}call_phone_t;



typedef struct voip_app_s voip_app_t;

typedef struct voip_call_s
{
	BOOL			active;
	//char 			username[SIP_USERNAME_MAX];
	u_int16			room_number;
//#ifdef VOIP_MULTI_CALL_MAX
	u_int16			num;
	call_phone_t	phonetab[VOIP_MULTI_CALL_NUMBER_MAX];
	u_int16			index;
	int				time_id;			//定时器ID
	int				time_interval;		//定时间隔（多长时间没有人接听）
#define APP_RINGING_TIME_INTERVAL		30000//15 SEC
//#endif

	u_int32			start_timer;
	u_int32			open_timer;
	u_int32			stop_timer;
	//char 			phone[SIP_NUMBER_MAX];

	app_call_source_t 	source;

	BOOL			talking;
	u_int8			building;
	u_int8			unit;

	int				instance;
	void			*sip_session;
	voip_app_t		*app;
	BOOL			local_stop;		//local stop
}voip_call_t;

typedef enum
{
	//APP通话状态	APP层面
	APP_STATE_TALK_IDLE,			//通话空闲
	APP_STATE_TALK_FAILED,			//通话建立失败
	APP_STATE_TALK_CALLING,
	APP_STATE_TALK_SUCCESS,			//通话建立
	APP_STATE_TALK_RUNNING,			//通话中
}voip_app_state_t;



typedef struct voip_app_s
{
	void				*voip_task;

	void				*voip_sip;

	void				*voip_stream;

	voip_volume_t 		voip_volume;

	voip_socket_t 		voip_socket;

	voip_app_state_t	state;

	voip_call_t			*call_session[VOIP_MULTI_CALL_MAX];
	voip_call_t			*session;

	//voip_stream_remote_t remote;
	void				*x5b_app;
	//voip_call_t 	voip_call;


	BOOL				local_stop;		//local stop

	int					debug;
}voip_app_t;


extern voip_app_t  *voip_app;


extern int voip_app_gl_init();
extern int voip_app_gl_exit();

extern voip_app_state_t voip_app_state_get(voip_app_t *osip);
extern int voip_app_state_set(voip_app_t *osip, voip_app_state_t state);

extern BOOL voip_app_already_call(voip_app_t *app);

extern int voip_app_media_callback_init(voip_app_t *app);

extern int voip_app_sip_register_start(BOOL reg);

extern voip_call_t * voip_app_call_session_lookup_by_number(voip_app_t *app, char *number);
extern voip_call_t * voip_app_call_session_lookup_by_instance(voip_app_t *app, int instance);


//extern void * voip_app_call_ID_instance_lookup(voip_call_t *call, int instance);



extern int voip_app_call_make(voip_call_t *call, app_call_source_t source, u_int8 building,
		u_int8 unit, u_int16 room);

extern int voip_app_multi_call_next();


extern BOOL voip_app_call_event_from_cli_web();
extern BOOL voip_app_call_event_from_ui();

extern int voip_app_start_call_event_ui(voip_event_t *ev);
extern int voip_app_stop_call_event_ui(voip_event_t *ev);
extern int voip_app_start_call_event_ui_phone(voip_event_t *ev);

extern int voip_app_start_call_event_cli_web(app_call_source_t source, u_int8 building,
		u_int8 unit, u_int16 room, char *number);

extern int voip_app_stop_call_event_cli_web(voip_call_t *call);

/*extern int voip_app_call_spilt_from_web(char *input, u_int8 *building,
		u_int8 *unit, u_int16 *room, char *phonelist, int len);*/


extern void *voip_app_call_event_current();

extern int voip_app_dtmf_recv_callback(char input, int cmd);


extern int voip_app_debug_set_api(int);
extern int voip_app_debug_get_api();


#endif /* __VOIP_APP_H__ */

/*
 * voip_osip.h
 *
 *  Created on: Jan 31, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_OSIP_H__
#define __VOIP_OSIP_H__

#include "voip_app.h"
#include "voip_sip.h"

//#define _OSIP_DEBUG_ENABLE

#ifdef VOIP_MULTI_CALL_MAX
#define OSIP_MULTI_CALL_MAX	VOIP_MULTI_CALL_MAX
#else
#define OSIP_MULTI_CALL_MAX	8
#endif
//#define OSIP_MULTI_CALL_ENABLE

#define OSIP_SELF_CB_ENABLE
/*
#define VOIP_OSIP_VER  		EXOSIP_VERSION
#define VOIP_OSIP_UA_STRING EXOSIP_PACKAGE_NAME VOIP_OSIP_VER
*/

#define OSIP_DEBUG_EVENT		0X1
#define OSIP_DEBUG_INFO			0X2
#define OSIP_DEBUG_MSG			0X4
#define OSIP_DEBUG_DETAIL		0X10

#define OSIP_DEBUG_ON(n)		_osip_debug |= OSIP_DEBUG_ ## n
#define OSIP_DEBUG_OFF(n)		_osip_debug &= ~OSIP_DEBUG_ ## n
#define OSIP_IS_DEBUG(n)		(_osip_debug & OSIP_DEBUG_ ## n)

#define VOIP_OSIP_VER  		"V0.1"
#define VOIP_OSIP_UA_STRING "X5-B-SIP" " "VOIP_OSIP_VER


//#define OSIP_BUF_MAX	64



typedef struct regparam_s
{
	int regid;
	int expiry;
	int auth;
} regparam_t;

typedef struct callparam_s
{
	int 	tid;
	int 	cid;
	int 	did;

	char 	callnumber[SIP_NUMBER_MAX];
	enum
	{
		OSIP_CALL_NONE,
		OSIP_CALL_RUN,
		OSIP_CALL_TALK,
		OSIP_CALL_HOLD,
	}state;
	int		(*delete_cb)(void *);
	void 	*pVoid;
	int		source;
} callparam_t;

typedef struct osip_statistics_s
{
	int 	reg_fail;			//register fail count
	int 	reg_success;		//register successfully count
	//int 	reg_send;			//register send count
	//int 	reg_ack;			//register send count
	int		reg_no_answer;		//no answer

	int 	call_fail;			//call fail count
	int 	call_success;		//call successfully count
	//int 	call_send;			//call send count
	//int 	call_ack;			//call send count

} osip_statistics_t;

#define OSIP_STATISTICS_REG_INC(n, a)		((n)->statistics.reg_ ## a += 1)
#define OSIP_STATISTICS_REG_DEC(n, a)		((n)->statistics.reg_ ## a -= 1)

#define OSIP_STATISTICS_CALL_INC(n, a)		((n)->statistics.call_ ## a += 1)
#define OSIP_STATISTICS_CALL_DEC(n, a)		((n)->statistics.call_ ## a -= 1)


typedef struct osip_policy_s
{
	//register policy
	int 	reg_count;			//register fail count
	int 	reg_fail;			//register fail count
	int		reg_no_answer;		//no answer
	BOOL 	standby;			//The current registered on main or stanby
	BOOL 	bSwitch;			//当前是否切换(主要功能上用于确定是否切换)
	BOOL 	bSwitching;			//当前是否真正切换（多线程下防止多次调用切换）
	int		switch_reason;

	//call policy
/*
	int 	call_error;			//call fail reason
	int 	call_max;			//call number max
	char   *call_number[OSIP_MULTI_CALL_MAX];//call number table
	int		call_index;
*/

} osip_policy_t;

#define OSIP_POLICY_REG_INC(n, a)		((n)->osip_policy.reg_ ## a += 1)
#define OSIP_POLICY_REG_DEC(n, a)		((n)->osip_policy.reg_ ## a -= 1)
#define OSIP_POLICY_REG_ZERO(n, a)		((n)->osip_policy.reg_ ## a = 0)

typedef enum
{
	OSIP_INACTIVE,
	OSIP_ACTIVE,
}osip_state;


typedef struct osip_remote_param_s
{
	char				remote_address[SIP_ADDRESS_MAX];	//remote address
	unsigned short		remote_port;					//remote port

	char				firewallip[SIP_ADDRESS_MAX];

	char 				username[SIP_USERNAME_MAX];
	char 				userid[SIP_NUMBER_MAX];
	char 				password[SIP_PASSWORD_MAX];

	regparam_t			regparam;
	char 				proxy[SIP_DATA_MAX * 2];
	char 				fromuser[SIP_DATA_MAX * 2];
	char 				contact[SIP_DATA_MAX * 2];

	osip_state			active;

} osip_remote_param_t;

#ifdef OSIP_SELF_CB_ENABLE
#define OSIP_CALLBACK_MAX	16

typedef struct osip_callback_s
{
	BOOL	bReady;
	int		(*cb)(void *);
	void	*pVoid;

} osip_callback_t;
#endif /* OSIP_SELF_CB_ENABLE */

typedef struct osip_media_callback_s
{
	int 	(*media_start_cb)(void *app, void *remote, int type, int id);
	void	*pVoidStart;
	int 	(*media_stop_cb)(void *app, void *remote, int type, int id);
	void	*pVoidStop;

} osip_media_callback_t;

typedef struct voip_osip_s
{
	BOOL				enable;
	BOOL				initialization;
	int					taskid;
	void				*context;
	sip_transport_t		proto;

	char				address[SIP_ADDRESS_MAX];			//local address
	unsigned short		port;							//local port

	osip_policy_t		osip_policy;
	osip_statistics_t	statistics;

	//BOOL				bClouds;			//当前是否注册在云端
	osip_remote_param_t *remote_param;
	osip_remote_param_t remote_param_main;
	osip_remote_param_t remote_param_standby;

	//callparam_t			multicall[OSIP_MULTI_CALL_MAX];
	callparam_t			*multicall;
	callparam_t			*callparam; //point to multicall table
	void				*mutex;

	voip_sip_t			*sip;
	osip_state_t		register_state;
	osip_state_t		call_state;
	//osip_state_t		ack_state;
	osip_call_error_t 	call_error;

	void				*r_event;
	u_int16				register_interval;
	void				*r_reset;
	void				*r_init;
#define REG_NO_ANSWER_INTERVAL  10
#define REG_FAIL_INTERVAL 		10

#ifdef OSIP_SELF_CB_ENABLE
	osip_callback_t		cb_table[OSIP_CALLBACK_MAX];
#endif /* OSIP_SELF_CB_ENABLE */

	osip_media_callback_t media_cb;

	int					debug;
	int					detail;
}voip_osip_t;

extern int _osip_debug;
//extern voip_osip_t *voip_osip;


#define voip_osip_lock(c)	if(((voip_osip_t*)c)->mutex) \
							os_mutex_lock(((voip_osip_t*)c)->mutex, OS_WAIT_FOREVER)

#define voip_osip_unlock(c)	if(((voip_osip_t*)c)->mutex) \
							os_mutex_unlock(((voip_osip_t*)c)->mutex)

extern int voip_osip_module_init(voip_sip_t *);
extern int voip_osip_module_exit();
extern int voip_osip_module_task_init(voip_osip_t *osip);
extern int voip_osip_module_task_exit(voip_osip_t *osip);
extern int voip_osip_media_callback(voip_osip_t *osip, osip_media_callback_t *callback);

extern int voip_osip_initialization(void);
extern int voip_osip_restart(void);

/*
extern int voip_osip_context_init_api(voip_osip_t *osip);
extern int voip_osip_context_username_api(voip_osip_t *osip);
*/

extern int voip_osip_register_initialization_api(voip_osip_t *osip);
extern int voip_osip_register_api(voip_osip_t *osip);
extern int voip_osip_unregister_api(voip_osip_t *osip);


extern int voip_osip_call_start_api(voip_osip_t *osip, char *username, char *phonenumber, int *instance);
extern int voip_osip_call_stop_api(voip_osip_t *osip, int instance);

extern int voip_osip_request_info_api(voip_osip_t *osip, char key);


//extern int voip_osip_call_create_instance(voip_osip_t *osip, char *name, int cid);

//extern callparam_t *voip_osip_call_lookup_instance(voip_osip_t *osip, char *name);
extern callparam_t *voip_osip_call_lookup_instance_bycid(voip_osip_t *osip, int cid);

//extern int voip_osip_call_delete_instance(voip_osip_t *osip, char *name);
//extern int voip_osip_call_delete_instance_bycid(voip_osip_t *osip, int cid);

//extern int voip_osip_call_set_instance(voip_osip_t *osip, char *name, int tid, int cid, int did);
//extern int voip_osip_call_update_instance_bycid(voip_osip_t *osip, int cid, char *name, int tid, int did, int state);


extern int voip_osip_set_log_level(int level, int detail);
extern int voip_osip_get_log_level(int *level, int *detail);


#endif /* __VOIP_OSIP_H__ */

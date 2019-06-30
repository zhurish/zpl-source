/*
 * voip_event.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __VOIP_EVENT_H__
#define __VOIP_EVENT_H__


//#define _VOIP_EVENT_DEBUG

#define VOIP_EVENT_NAME_MAX		128
#define VOIP_EVENT_DATA_MAX		1024

#if 0
enum
{
	VOIP_SIP_EV_NONE,
	/*
	 * control sip
	 */
	VOIP_SIP_EV_START_REGISTER,		//开始注册
	VOIP_SIP_EV_STOP_REGISTER,
	VOIP_SIP_EV_START_CALL,			//开始呼叫
	VOIP_SIP_EV_STOP_CALL,
	VOIP_SIP_EV_START_STREAM,		//开始传输
	VOIP_SIP_EV_STOP_STREAM,

	/*
	 * control by sip
	 */
};
#endif
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
	u_int8		data[VOIP_EVENT_DATA_MAX];
	u_int8		dlen;
	u_int16		interval;
	u_int32		timer;
	char 		entry_name[VOIP_EVENT_NAME_MAX];
}voip_event_t;

#define VOIP_EVENT_ARGV(n)	(((voip_event_t *)(n))->pVoid)


typedef struct voip_event_ctx_s
{
	int 		taskid;
	BOOL		enable;
	LIST		execute_lst;//high
	LIST		high_lst;	//high
	LIST		ready_lst;	//ready
	LIST		timer_lst;	//timer
	LIST		event_unlst;
	os_mutex_t 	*mutex;
	os_sem_t 	*sem;
}voip_event_ctx_t;

//extern voip_event_ctx_t *voip_event;

extern int voip_event_module_init(void);
extern int voip_event_module_exit(void);
extern int voip_event_task_init();
extern int voip_event_task_exit();


extern voip_event_t * voip_event_add_one(voip_event_type_t type, voip_event_t *node);
extern int voip_event_del_one(voip_event_t *node);

#define voip_event_add(c,p,d,l,v)	_voip_event_add_raw_one(VOIP_EVENT_READY, c,p,d,l,v,#c)
#define voip_event_cancel(c)	\
			do { \
				if (c) \
				{ \
					voip_event_del_one(c); \
					c = NULL; \
				} \
			} while (0)

#define voip_event_timer_add(c,p,d,l,v)	_voip_event_add_raw_one(VOIP_EVENT_TIMER, c,p,d,l,v,#c)
#define voip_event_high_add(c,p,d,l,v)	_voip_event_add_raw_one(VOIP_EVENT_HIGH, c,p,d,l,v,#c)
#define voip_event_ready_add(c,p,d,l,v)	_voip_event_add_raw_one(VOIP_EVENT_READY, c,p,d,l,v,#c)

extern voip_event_t * _voip_event_add_raw_one(voip_event_type_t type, int (*cb)(voip_event_t *),
		void *pVoid, char *buf, int len, int value, char *funcname);


extern int _voip_event_list_debug(struct vty *vty);


#ifdef _VOIP_EVENT_DEBUG
#define VOIP_EV_DEBUG(fmt,...)		zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)
#else
#define VOIP_EV_DEBUG(fmt,...)
#endif

#endif /* __VOIP_EVENT_H__ */

/*
 * voip_event.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __VOIP_EVENT_H__
#define __VOIP_EVENT_H__





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

typedef struct event_node_s event_node_t;

typedef struct event_node_s
{
	NODE		node;
	int			(*ev_cb)(event_node_t *);
	void		*pVoid;
	u_int8		data[1024];

	char 		entry_name[128];
}event_node_t;

typedef struct voip_event_s
{
	void		*master;
	int 		taskid;
	BOOL		enable;
	LIST		event_lst;
	LIST		event_unlst;
	os_mutex_t 	*mutex;
	os_sem_t 	*sem;
}voip_event_t;

extern voip_event_t voip_event;

extern int voip_event_module_init(void);
extern int voip_event_module_exit(void);
extern int voip_event_task_init();
extern int voip_event_task_exit();


#define voip_event_node_register(c,p,d,l)	_voip_event_node_register(c,p,d,l,#c)

//extern int voip_event_node_register(int (*cb)(event_node_t *), void *pVoid, char *buf, int len);
/*
extern int voip_event_node_del(event_node_t *node);
extern int voip_event_node_unregister(int (*cb)(event_node_t *), void *pVoid);
*/

extern int voip_event_node_add(event_node_t *node);
extern int _voip_event_node_register(int (*cb)(event_node_t *), void *pVoid, char *buf, int len, char *funcname);
extern int voip_event_node_del(event_node_t *node);
extern int voip_event_node_unregister(int (*cb)(event_node_t *), void *pVoid);



#endif /* __VOIP_EVENT_H__ */

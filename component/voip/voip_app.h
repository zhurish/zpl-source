/*
 * voip_app.h
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_APP_H__
#define __VOIP_APP_H__

#include "voip_state.h"
#include "voip_event.h"

#define V_APP_DEBUG(fmt,...)		zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)

#define VOIP_APP_DEBUG

#define VOIP_PHONE_MAX	32


#define VOIP_APP_DEBUG_EVENT	0X01
#define VOIP_APP_DEBUG_SIP		0X02
#define VOIP_APP_DEBUG_RECV		0X04
#define VOIP_APP_DEBUG_SEND		0X08
#define VOIP_APP_DEBUG_DETAIL	0X1000


#define VOIP_APP_DEBUG(n)		(VOIP_APP_DEBUG_ ## n & voip_call.debug)
#define VOIP_APP_DEBUG_ON(n)	(voip_call.debug |= (VOIP_APP_DEBUG_ ## n ))
#define VOIP_APP_DEBUG_OFF(n)	(voip_call.debug &= ~(VOIP_APP_DEBUG_ ## n ))



typedef struct voip_position_room_s
{
	u_int8		building;
	u_int16		room_number;
	char 		phone[VOIP_PHONE_MAX];
}voip_position_room_t;

typedef struct voip_call_s
{
	voip_position_room_t *room;
	//voip_state_t		state;

	int		debug;
}voip_call_t;



extern voip_call_t voip_call;

extern int voip_app_ev_stop_call(event_node_t *ev);
extern int voip_app_ev_start_call(event_node_t *ev);
extern int voip_app_ev_register(event_node_t *ev);


#ifdef VOIP_APP_DEBUG
extern int voip_app_call_test(BOOL enable, char *address, int port, int lport);
#endif

#endif /* __VOIP_APP_H__ */

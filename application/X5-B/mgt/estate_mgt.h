/*
 * voip_estate_mgt.h
 *
 *  Created on: Jan 1, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_ESTATE_MGT_H__
#define __VOIP_ESTATE_MGT_H__

#include "x5_b_a.h"
#include "voip_app.h"


//#define VOIP_ESTATE_MGT_LOOPBACK

#define VOIP_ESTATE_MGT_TIMEOUT		5


typedef enum voip_estate_state_s
{
	VOIP_ESTATE_STATE_NONE,
	VOIP_ESTATE_LINKUP_NONE,
	VOIP_ESTATE_LINKDOWN_NONE,
}voip_estate_state_t;


typedef struct voip_estate_mgt_s
{
	void		*master;
	BOOL		enable;
	int 		fd;

	int			sock;
	voip_estate_state_t state;
	void		*t_read;
	void		*t_write;
	void		*t_event;
	void		*t_time;
	void		*t_reset;
	u_int8		buf[1024];
	u_int8		sbuf[1024];
	u_int16		slen;

	u_int8		interval;
	char		*local_address;
	u_int16		local_port;

	char		*remote_address;
	u_int16		remote_port;
	int			debug;
}voip_estate_mgt_t;

#define VOIP_EMGT_DEBUG_EVENT	0X01
#define VOIP_EMGT_DEBUG_SIP		0X02
#define VOIP_EMGT_DEBUG_RECV		0X04
#define VOIP_EMGT_DEBUG_SEND		0X08
#define VOIP_EMGT_DEBUG_DETAIL	0X1000


#define VOIP_EMGT_DEBUG(n)		(VOIP_EMGT_DEBUG_ ## n & gestate_mgt.debug)
#define VOIP_EMGT_DEBUG_ON(n)	(gestate_mgt.debug |= (VOIP_EMGT_DEBUG_ ## n ))
#define VOIP_EMGT_DEBUG_OFF(n)	(gestate_mgt.debug &= ~(VOIP_EMGT_DEBUG_ ## n ))



extern int voip_estate_mgt_init(void);
extern int voip_estate_mgt_exit(void);

extern int voip_estate_mgt_start(void);
extern int voip_estate_mgt_stop(void);


extern int voip_estate_mgt_local_address_set_api(char *address);
extern int voip_estate_mgt_local_port_set_api(u_int16 port);
extern int voip_estate_mgt_address_set_api(char *remote);
extern int voip_estate_mgt_port_set_api(u_int16 port);


//extern int voip_estate_mgt_get_phone_number(voip_estate_mgt_t *estate_mgt, int timeoutms);
extern int voip_estate_mgt_get_phone_number(x5_b_room_position_t *room, voip_position_room_t *out);


extern int voip_estate_mgt_get_room_position(voip_estate_mgt_t *estate_mgt, char *position, int len);



#endif /* __VOIP_ESTATE_MGT_H__ */

/*
 * nsm_security.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_SECURITY_H_
#define __NSM_SECURITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_vlan.h"

typedef enum
{
	NSM_STORMCONTROL_RATE = 0x00,
	NSM_STORMCONTROL_PERCENT = 0x01,
	NSM_STORMCONTROL_PACKET = 0x02,
}nsm_strorm_mode_e;

typedef struct nsm_stormcontrol_s
{
	zpl_uint32			stormcontrol_unicast;
	zpl_uint8			unicast_flags;
	zpl_uint32			stormcontrol_multicast;
	zpl_uint8			multicast_flags;
	zpl_uint32			stormcontrol_broadcast;
	zpl_uint8			broadcast_flags;
}nsm_stormcontrol_t;



typedef struct nsm_security_s
{
	zpl_bool			stormcontrol_enable;
	nsm_stormcontrol_t	stormcontrol;			//storm control
}nsm_security_t;


typedef struct Global_Security_s
{
	zpl_bool			security_enable;
	
}Global_Security_t;



extern int nsm_stormcontrol_enable_set_api(struct interface *ifp, zpl_bool enable);
extern zpl_bool nsm_stormcontrol_enable_get_api(struct interface *ifp);

extern int nsm_stormcontrol_unicast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_unicast,
		zpl_uint8 unicastflag);
extern int nsm_stormcontrol_unicast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_unicast,
		zpl_uint8 *unicastflag);

extern int nsm_stormcontrol_multicast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_multicast,
		zpl_uint8 multicastflag);
extern int nsm_stormcontrol_multicast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_multicast,
		zpl_uint8 *multicastflag);

extern int nsm_stormcontrol_broadcast_set_api(struct interface *ifp, zpl_uint32 stormcontrol_broadcast,
		zpl_uint8 broadcastflag);
extern int nsm_stormcontrol_broadcast_get_api(struct interface *ifp, zpl_uint32 *stormcontrol_broadcast,
		zpl_uint8 *broadcastflag);


extern int nsm_security_init(void);
extern int nsm_security_exit(void);
extern int nsm_security_interface_create_api(struct interface *ifp);
extern int nsm_security_interface_del_api(struct interface *ifp);

#ifdef ZPL_SHELL_MODULE
extern int cmd_security_init(void);
extern int nsm_security_interface_write_config(struct vty *vty, struct interface *ifp);
#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_SECURITY_H_ */

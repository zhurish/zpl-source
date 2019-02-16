/*
 * voip_osip.h
 *
 *  Created on: Jan 31, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_OSIP_H__
#define __VOIP_OSIP_H__


typedef enum osip_transport_s
{
	OSIP_PROTO_UDP,
	OSIP_PROTO_TCP,
	OSIP_PROTO_TLS,
	OSIP_PROTO_DTLS,
}osip_transport_t;

typedef struct regparam_s
{
  int regid;
  int expiry;
  int auth;
} regparam_t;


typedef struct voip_osip_s
{
	int					enable;
	int					taskid;
	void				*context;
	regparam_t			regparam;
	char				*address;
	char				*firewallip;
	unsigned short		port;
	osip_transport_t	proto;

	char 				*username;
	char 				*password;


	char 				*proxy;
	char 				*fromuser;
	char 				*contact;
	unsigned int		expiry;

}voip_osip_t;

extern int voip_osip_module_init();
extern int voip_osip_module_exit();
extern int voip_osip_module_task_init(voip_osip_t *osip);
extern int voip_osip_module_task_exit(voip_osip_t *osip);

extern int voip_osip_context_init_api(voip_osip_t *osip);
extern int voip_osip_context_username_api(voip_osip_t *osip);
extern int voip_osip_register_api(voip_osip_t *osip);
extern int voip_osip_call_start_api(voip_osip_t *osip);
extern int voip_osip_call_stop_api(voip_osip_t *osip);
extern int voip_osip_request_info_api(voip_osip_t *osip);


#endif /* __VOIP_OSIP_H__ */

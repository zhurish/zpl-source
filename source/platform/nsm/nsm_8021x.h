/*
 * nsm_8021x.h
 *
 *  Created on: May 10, 2018
 *      Author: zhurish
 */

#ifndef __NSM_8021X_H__
#define __NSM_8021X_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef enum dot1x_state_en
{
	//auto | force-authorized | force-unauthorized
	DOT1X_NONE = 0,
	DOT1X_AUTO,
	DOT1X_FORCE_AUTHORIZED,
	DOT1X_FORCE_UNAUTHORIZED,

}dot1x_state_en;


typedef struct dot1x_s
{
	NODE			node;
	ifindex_t		ifindex;
	zpl_bool			enable;		//enable dot1x

	dot1x_state_en		state;
	zpl_uint32			hwstate;
	zpl_bool			reauthentication;
	zpl_uint32			eap_version;
	zpl_bool			bypass;
	vlan_t				guest_vlan;

	zpl_bool			port_mode;
	zpl_uint32			max_user;	//base MAC type, max user
	zpl_bool			mac_auth_bypass;

	zpl_uint32			reauth_timeout;
	zpl_uint32			server_timeout;
	zpl_uint32			supp_timeout;
	zpl_uint32			tx_period_timeout;
	zpl_uint32			quiet_period_timeout;


	zpl_uint32			max_req;
}dot1x_t;


typedef struct Gdot1x_s
{
	LIST		*dot1xList;
	void		*mutex;
	zpl_bool		enable;
}Gdot1x_t;


typedef int (* dot1x_cb)(dot1x_t *, void *);



int nsm_dot1x_init(void);
int nsm_dot1x_exit(void);

int nsm_dot1x_global_enable(zpl_bool enable);
zpl_bool nsm_dot1x_global_is_enable(void);

int dot1x_callback_api(dot1x_cb cb, void *pVoid);

int nsm_dot1x_enable_set_api(ifindex_t ifindex, zpl_bool enable);
int nsm_dot1x_enable_get_api(ifindex_t ifindex, zpl_bool *enable);
zpl_bool nsm_dot1x_is_enable_api(ifindex_t ifindex);


int nsm_dot1x_auth_state_set_api(ifindex_t ifindex, dot1x_state_en state);
int nsm_dot1x_auth_state_get_api(ifindex_t ifindex, dot1x_state_en *state);

int nsm_dot1x_auth_version_set_api(ifindex_t ifindex, zpl_uint32 version);
int nsm_dot1x_auth_version_get_api(ifindex_t ifindex, zpl_uint32 *version);

int nsm_dot1x_reauthentication_set_api(ifindex_t ifindex, zpl_bool enable);
int nsm_dot1x_reauthentication_get_api(ifindex_t ifindex, zpl_bool *enable);

int nsm_dot1x_port_mode_set_api(ifindex_t ifindex, zpl_bool port);
int nsm_dot1x_port_mode_get_api(ifindex_t ifindex, zpl_bool *port);

int nsm_dot1x_mac_auth_bypass_set_api(ifindex_t ifindex, zpl_bool enable);
int nsm_dot1x_mac_auth_bypass_get_api(ifindex_t ifindex, zpl_bool *enable);

int nsm_dot1x_guest_vlan_set_api(ifindex_t ifindex, vlan_t vlan);
int nsm_dot1x_guest_vlan_get_api(ifindex_t ifindex, vlan_t *vlan);

int nsm_dot1x_max_user_set_api(ifindex_t ifindex, zpl_uint32 maxUser);
int nsm_dot1x_max_user_get_api(ifindex_t ifindex, zpl_uint32 *maxUser);

int nsm_dot1x_reauth_timeout_set_api(ifindex_t ifindex, zpl_uint32 timeout);
int nsm_dot1x_reauth_timeout_get_api(ifindex_t ifindex, zpl_uint32 *timeout);

int nsm_dot1x_server_timeout_set_api(ifindex_t ifindex, zpl_uint32 timeout);
int nsm_dot1x_server_timeout_get_api(ifindex_t ifindex, zpl_uint32 *timeout);

int nsm_dot1x_supp_timeout_set_api(ifindex_t ifindex, zpl_uint32 timeout);
int nsm_dot1x_supp_timeout_get_api(ifindex_t ifindex, zpl_uint32 *timeout);

int nsm_dot1x_period_timeout_set_api(ifindex_t ifindex, zpl_uint32 timeout);
int nsm_dot1x_period_timeout_get_api(ifindex_t ifindex, zpl_uint32 *timeout);

int nsm_dot1x_quiet_period_timeout_set_api(ifindex_t ifindex, zpl_uint32 timeout);
int nsm_dot1x_quiet_period_timeout_get_api(ifindex_t ifindex, zpl_uint32 *timeout);


int nsm_dot1x_max_req_set_api(ifindex_t ifindex, zpl_uint32 count);
int nsm_dot1x_max_req_get_api(ifindex_t ifindex, zpl_uint32 *count);

int nsm_dot1x_reset_api(ifindex_t ifindex);

#ifdef ZPL_SHELL_MODULE
void cmd_dot1x_init(void);
int build_dot1x_config(struct vty *vty, void *p);
int build_dot1x_interface(struct vty *vty, struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_8021X_H__ */
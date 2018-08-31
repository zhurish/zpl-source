/*
 * nsm_8021x.h
 *
 *  Created on: May 10, 2018
 *      Author: zhurish
 */

#ifndef __NSM_8021X_H__
#define __NSM_8021X_H__



typedef enum dot1x_type_e
{
	//auto | force-authorized | force-unauthorized
	DOT1X_NONE = 0,
	DOT1X_AUTO,
	DOT1X_FORCE_AUTHORIZED,
	DOT1X_FORCE_UNAUTHORIZED,

}dot1x_type_en;

typedef enum dot1x_state_e
{
	DOT1X_BLK_NONE = 0,
	DOT1X_BLK_UNACCECT,
	DOT1X_BLK_ACCECT,
}dot1x_state_en;


typedef struct dot1x_s
{
	NODE			node;
	ifindex_t		ifindex;
	BOOL			enable;		//enable dot1x
	dot1x_type_en	type;

	dot1x_state_en	state;
	BOOL			reauthentication;
	u_int			eap_version;
	BOOL			bypass;
	vlan_t			guest_vlan;

	BOOL			port_mode;
	u_int			max_user;	//base MAC type, max user
	BOOL			mac_auth_bypass;

	u_int			reauth_timeout;
	u_int			server_timeout;
	u_int			supp_timeout;
	u_int			tx_period_timeout;
	u_int			quiet_period_timeout;


	u_int			max_req;
}dot1x_t;


typedef struct Gdot1x_s
{
	LIST		*dot1xList;
	void		*mutex;
	BOOL		enable;
}Gdot1x_t;


typedef int (* dot1x_cb)(dot1x_t *, void *);



int nsm_dot1x_init(void);
int nsm_dot1x_exit(void);

int nsm_dot1x_global_enable(BOOL enable);
BOOL nsm_dot1x_global_is_enable();

int dot1x_callback_api(dot1x_cb cb, void *pVoid);

int nsm_dot1x_enable_set_api(ifindex_t ifindex, BOOL enable, dot1x_type_en type);
int nsm_dot1x_enable_get_api(ifindex_t ifindex, BOOL *enable, dot1x_type_en *type);
BOOL nsm_dot1x_is_enable_api(ifindex_t ifindex);

int nsm_dot1x_auth_type_set_api(ifindex_t ifindex, dot1x_type_en type);
int nsm_dot1x_auth_type_get_api(ifindex_t ifindex, dot1x_type_en *type);

int nsm_dot1x_auth_state_set_api(ifindex_t ifindex, dot1x_state_en state);
int nsm_dot1x_auth_state_get_api(ifindex_t ifindex, dot1x_state_en *state);

int nsm_dot1x_auth_version_set_api(ifindex_t ifindex, u_int version);
int nsm_dot1x_auth_version_get_api(ifindex_t ifindex, u_int *version);

int nsm_dot1x_reauthentication_set_api(ifindex_t ifindex, BOOL enable);
int nsm_dot1x_reauthentication_get_api(ifindex_t ifindex, BOOL *enable);

int nsm_dot1x_port_mode_set_api(ifindex_t ifindex, BOOL port);
int nsm_dot1x_port_mode_get_api(ifindex_t ifindex, BOOL *port);

int nsm_dot1x_mac_auth_bypass_set_api(ifindex_t ifindex, BOOL enable);
int nsm_dot1x_mac_auth_bypass_get_api(ifindex_t ifindex, BOOL *enable);

int nsm_dot1x_guest_vlan_set_api(ifindex_t ifindex, vlan_t vlan);
int nsm_dot1x_guest_vlan_get_api(ifindex_t ifindex, vlan_t *vlan);

int nsm_dot1x_max_user_set_api(ifindex_t ifindex, u_int maxUser);
int nsm_dot1x_max_user_get_api(ifindex_t ifindex, u_int *maxUser);

int nsm_dot1x_reauth_timeout_set_api(ifindex_t ifindex, u_int timeout);
int nsm_dot1x_reauth_timeout_get_api(ifindex_t ifindex, u_int *timeout);

int nsm_dot1x_server_timeout_set_api(ifindex_t ifindex, u_int timeout);
int nsm_dot1x_server_timeout_get_api(ifindex_t ifindex, u_int *timeout);

int nsm_dot1x_supp_timeout_set_api(ifindex_t ifindex, u_int timeout);
int nsm_dot1x_supp_timeout_get_api(ifindex_t ifindex, u_int *timeout);

int nsm_dot1x_period_timeout_set_api(ifindex_t ifindex, u_int timeout);
int nsm_dot1x_period_timeout_get_api(ifindex_t ifindex, u_int *timeout);

int nsm_dot1x_quiet_period_timeout_set_api(ifindex_t ifindex, u_int timeout);
int nsm_dot1x_quiet_period_timeout_get_api(ifindex_t ifindex, u_int *timeout);


int nsm_dot1x_max_req_set_api(ifindex_t ifindex, u_int count);
int nsm_dot1x_max_req_get_api(ifindex_t ifindex, u_int *count);

int nsm_dot1x_reset_api(ifindex_t ifindex);


#endif /* __NSM_8021X_H__ */

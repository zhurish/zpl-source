/*
 * nsm_bridge.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_BRIDGE_H__
#define __NSM_BRIDGE_H__


#define BRIDGE_MEMBER_MAX	16

typedef enum bridge_type_s
{
	BRIDGE_IF = 1,
	BRIDGE_MEMBER,
}bridge_type_t;

//const char *stp_str[] = {"Disable","Listening","Learning","Forwarding","Blocking"};
typedef enum stp_state_s
{
	STP_DISABLE = 0,
	STP_LISTENING,
	STP_LEARNING,
	STP_FORWARDING,
	STP_BLOCKING,
}stp_state_t;

typedef struct nsm_bridge_s
{
	struct interface 	*ifp;
	bridge_type_t		br_mode;//接口状态，网桥，桥接接口
	BOOL				br_stp;//网桥生成树
	stp_state_t			br_stp_state;//生成树状态
	int 				max_age;
	int 				hello_time;
	int 				forward_delay;
	ifindex_t			member[BRIDGE_MEMBER_MAX];


	int (*add_member_cb)(struct nsm_bridge_s *br, int ifindex);
	int (*del_member_cb)(struct nsm_bridge_s *br, int ifindex);

} nsm_bridge_t;


extern nsm_bridge_t * nsm_bridge_get(struct interface *ifp);

extern int nsm_bridge_add_interface_api(struct interface *bridge, struct interface *ifp);
extern int nsm_bridge_del_interface_api(struct interface *bridge, struct interface *ifp);


extern int nsm_bridge_interface_stp_set_api(struct interface *bridge, BOOL stp);
extern int nsm_bridge_interface_max_age_set_api(struct interface *bridge, int max_age);
extern int nsm_bridge_interface_hello_time_set_api(struct interface *bridge, int hello_time);
extern int nsm_bridge_interface_forward_delay_set_api(struct interface *bridge, int forward_delay);

extern int nsm_bridge_client_init();
extern int nsm_bridge_client_exit();

extern void cmd_bridge_init(void);

#endif /* __NSM_BRIDGE_H__ */

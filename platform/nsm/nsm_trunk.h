/*
 * nsm_trunk.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __NSM_TRUNK_H__
#define __NSM_TRUNK_H__


#define NSM_TRUNK_ID_MAX	2
#define NSM_TRUNK_MEMBER_MAX	4



#define LACP_PORT_PRIORITY_DEFAULT		32768
#define LACP_SYSTEM_PRIORITY_DEFAULT	32768

#define LOAD_BALANCE_DEFAULT	TRUNK_LOAD_BALANCE_DSTSRCMAC

#define LACP_TIMEOUT_SHORT	1
#define LACP_TIMEOUT_LONG	2
#define LACP_TIMEOUT_DEFAULT	LACP_TIMEOUT_SHORT

typedef enum trunk_type_s
{
	TRUNK_STATIC = 1,
	TRUNK_DYNAMIC,
}trunk_type_t;

typedef enum trunk_mode_s
{
	TRUNK_ACTIVE = 1,
	TRUNK_PASSIVE,
}trunk_mode_t;

typedef enum load_balance_s
{
	TRUNK_LOAD_BALANCE_NONE = 0,
	TRUNK_LOAD_BALANCE_DSTMAC,
	TRUNK_LOAD_BALANCE_SRCMAC,
	TRUNK_LOAD_BALANCE_DSTSRCMAC,
	TRUNK_LOAD_BALANCE_DSTIP,
	TRUNK_LOAD_BALANCE_SRCIP,
	TRUNK_LOAD_BALANCE_DSTSRCIP,
}load_balance_t;

struct l2trunk_group_s;
struct Gl2trunk_s;

typedef struct l2trunk_s
{
	NODE			node;
	u_int			trunkId;

	trunk_type_t	type;
	trunk_mode_t	mode;
	u_int			lacp_port_priority;
	u_int			lacp_timeout;

	ifindex_t 		ifindex;

	struct l2trunk_group_s *group;
}l2trunk_t;// port-channel member interface

typedef struct l2trunk_group_s
{
	LIST				*trunkList;
	u_int				trunkId;
	trunk_type_t		type;
	u_int				lacp_system_priority;
	load_balance_t		load_balance;
	struct Gl2trunk_s 	*global;

	struct interface	*ifp;
}l2trunk_group_t;// port-channel interface

typedef struct Gl2trunk_s
{
	//l2trunk_group_t	*group[NSM_TRUNK_ID_MAX];
	l2trunk_group_t	*group;
	//u_int			lacp_system_priority;
	//load_balance_t	load_balance;
	void		*mutex;
	BOOL		enable;
}Gl2trunk_t;


typedef int (*l2trunk_group_cb)(l2trunk_group_t *, void *);
typedef int (*l2trunk_cb)(l2trunk_t *, void *);

extern int nsm_trunk_init();
extern int nsm_trunk_exit();

extern int nsm_trunk_enable(void);
extern BOOL nsm_trunk_is_enable(void);

extern l2trunk_group_t * nsm_port_channel_get(struct interface *ifp);

extern BOOL l2trunk_lookup_api(u_int trunkid);
extern int l2trunk_lookup_interface_count_api(u_int trunkid);
extern int nsm_trunk_create_api(u_int trunkid, trunk_type_t type);
extern int nsm_trunk_destroy_api(u_int trunkid);


extern BOOL l2trunk_lookup_interface_api(ifindex_t ifindex);
extern int nsm_trunk_get_ID_interface_api(ifindex_t ifindex, u_int *trunkId);

extern int nsm_trunk_add_interface_api(u_int trunkid, trunk_type_t type, trunk_mode_t mode, struct interface *ifp);
extern int nsm_trunk_del_interface_api(u_int trunkid, struct interface *ifp);
extern int nsm_trunk_lacp_port_priority_api(ifindex_t ifindex, u_int pri);
extern int nsm_trunk_lacp_timeout_api(ifindex_t ifindex, u_int timeout);

extern int nsm_trunk_load_balance_api(u_int trunkid, load_balance_t mode);
extern int nsm_trunk_lacp_system_priority_api(u_int trunkid, u_int pri);

extern int nsm_trunk_group_callback_api(l2trunk_group_cb cb, void *pVoid);
extern int nsm_trunk_callback_api(l2trunk_cb cb, void *pVoid);


extern void cmd_trunk_init(void);


#endif /* __NSM_TRUNK_H__ */

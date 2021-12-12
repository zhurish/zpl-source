/*
 * nsm_vlan.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VLAN_H__
#define __NSM_VLAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_include.h"
#include "zpl_include.h"


#ifdef PRODUCT_PORT_MAX
#define PHY_PORT_MAX 	PRODUCT_PORT_MAX
#endif

/* vlan 管理 */

typedef struct trunk_vlan_s
{
	vlan_t	vlan;
	vlan_t 	minvlan;
	vlan_t	maxvlan;
}trunk_vlan_t;

typedef struct nsm_vlan_s
{
	vlan_t	native;
	vlan_t	access;
	zpl_bool all;
	trunk_vlan_t trunk_allowed[VLAN_TABLE_MAX];
	vlan_t	allowed_max;
}nsm_vlan_t;

/*
typedef enum vlan_cmd_s
{
	VLAN_ADD = 1,
	VLAN_DEL,
}vlan_cmd_t;
*/

typedef enum vlan_mode_s
{
	VLAN_UNTAG = 1,
//	VLAN_UNTAG,
	VLAN_TAG,
//	VLAN_TAG,
}vlan_mode_t;


typedef struct l2vlan_s
{
	NODE	node;
	vlan_t	vlan;
	vlan_t 	minvlan;
	vlan_t	maxvlan;
	zpl_uint32		stp;
	zpl_uint32		dscp;
	ifindex_t tagport[PHY_PORT_MAX];
	ifindex_t untagport[PHY_PORT_MAX];
	zpl_char *vlan_name;
	zpl_uint32  name_hash;
}l2vlan_t;

typedef struct Gl2vlan_s
{
	LIST	*vlanList;
	void	*mutex;
	zpl_bool	enable;
}Gl2vlan_t;

typedef int (*l2vlan_cb)(l2vlan_t *, void *);

extern int nsm_vlan_init();
extern int nsm_vlan_exit();
extern int nsm_vlan_cleanall(void);

extern int nsm_vlan_enable(void);
extern zpl_bool nsm_vlan_is_enable(void);

extern int nsm_vlan_interface_create_api(struct interface *ifp);
extern int nsm_vlan_interface_del_api(struct interface *ifp);


extern int nsm_vlan_list_create_api(const char *str);
extern int nsm_vlan_list_destroy_api(const char *str);
extern int nsm_vlan_create_api(vlan_t vlan);
extern int nsm_vlan_destroy_api(vlan_t vlan);
extern int nsm_vlan_batch_create_api(vlan_t minvlan, vlan_t maxvlan);
extern int nsm_vlan_batch_destroy_api(vlan_t minvlan, vlan_t maxvlan);

extern int nsm_vlan_name_api(vlan_t vlan, const char *name);
extern void * nsm_vlan_lookup_api(vlan_t vlan);
extern void * nsm_vlan_lookup_by_name_api(const char *name);

extern int nsm_vlan_callback_api(l2vlan_cb cb, void *);

extern int nsm_interface_add_untag_vlan_api(vlan_t vlan, struct interface *ifp);
extern int nsm_interface_del_untag_vlan_api(vlan_t vlan, struct interface *ifp);
extern int nsm_interface_lookup_untag_vlan_api(vlan_t vlan, struct interface *ifp);

extern int nsm_interface_add_tag_vlan_api(vlan_t vlan, struct interface *ifp);
extern int nsm_interface_del_tag_vlan_api(vlan_t vlan, struct interface *ifp);
extern int nsm_interface_lookup_tag_vlan_api(vlan_t vlan, struct interface *ifp);


extern int nsm_interface_access_vlan_set_api(struct interface *ifp, vlan_t);
extern int nsm_interface_access_vlan_unset_api(struct interface *ifp, vlan_t);
extern int nsm_interface_access_vlan_get_api(struct interface *ifp, vlan_t *);

extern int nsm_interface_native_vlan_set_api(struct interface *ifp, vlan_t);
extern int nsm_interface_native_vlan_get_api(struct interface *ifp, vlan_t *);

extern int nsm_interface_trunk_add_allowed_vlan_api(struct interface *ifp, vlan_t );
extern int nsm_interface_trunk_del_allowed_vlan_api(struct interface *ifp, vlan_t );
extern int nsm_interface_trunk_add_allowed_batch_vlan_api(struct interface *ifp, vlan_t ,vlan_t);
extern int nsm_interface_trunk_del_allowed_batch_vlan_api(struct interface *ifp, vlan_t ,vlan_t);

extern int nsm_interface_trunk_allowed_vlan_list_api(int add, struct interface *ifp, const char *str);
//extern int nsm_interface_trunk_get_allowed_vlan_api(struct interface *ifp, vlan_t );

//extern int vlan_string_explain(const char *str, vlan_t *value, int num, vlan_t *base, vlan_t *end);


#ifdef ZPL_SHELL_MODULE
extern void cmd_vlan_init (void);
extern int nsm_vlan_interface_write_config(struct vty *vty, struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_VLAN_H__ */

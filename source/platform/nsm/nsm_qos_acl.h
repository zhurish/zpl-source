/*
 * qos_acl.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_NSM_QOS_ACL_H_
#define __NSM_NSM_QOS_ACL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"
#include "filter.h"
#include "nsm_qos.h"
#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif
#define ZPL_FILTER_NOSEQ

#define NSM_QOS_ACL_NAME_MAX	32
/*
typedef enum
{
	NSM_QOS_REMARK_NONE = 0,
	NSM_QOS_REMARK_8021P = 1,
	NSM_QOS_REMARK_DSCP,
	NSM_QOS_REMARK_IP_PRI,
}qos_class_remark_e;

typedef enum
{
	NSM_QOS_ACTION_NONE = 0,
	NSM_QOS_PERIMIT = 1,
	NSM_QOS_DENY,
}qos_class_action_e;
*/
typedef enum
{
	NSM_QOS_MATCH_NONE = 0,
	NSM_QOS_MATCH_ALL = 1,
	NSM_QOS_MATCH_ANY,
}qos_class_match_e;


typedef struct qos_access_filter_s
{
	NODE			node;
	zpl_uint32		seqnum;
	enum filter_type type;
	enum access_node_type nodetype;
  	union
    {
      struct filter_cisco cfilter;
      struct filter_zebra zfilter;
      #ifdef ZPL_FILTER_NORMAL_EXT
      struct filter_zebos_ext zextfilter;
      #endif
      #ifdef ZPL_FILTER_MAC
      struct filter_l2    mac_filter;
      #endif
    } u;
	
}qos_access_filter_t;

typedef struct qos_access_filter_list_s
{
	NODE			node;
	char			name[NSM_QOS_ACL_NAME_MAX];
	#define SEQNUM_STEP	5
	zpl_uint32		seqnum_step;
	zpl_uint32		seqnum;
	LIST			list;
	void			*mutex;
	u_int32_t 		ref_cnt;
}qos_access_filter_list_t;


typedef struct qos_class_map_s
{
	NODE			node;
	char			name[NSM_QOS_ACL_NAME_MAX];
	char			acl_name[NSM_QOS_ACL_NAME_MAX];
	qos_class_match_e match_type;
	u_int32_t 		ref_cnt;

	nsm_qos_limit_t		stream_limit;	//流限速

}qos_class_map_t;

typedef struct qos_service_policy_s
{
	NODE			 node;
	char			 name[NSM_QOS_ACL_NAME_MAX];

	char			 class_name[NSM_QOS_ACL_NAME_MAX];
	qos_class_map_t	 *bind;
	u_int32_t ref_cnt;
}qos_service_policy_t;


typedef struct qos_access_list_s
{
	LIST		list;
	void		*mutex;
}qos_access_list_t;


typedef struct global_qos_access_list_s
{
	qos_access_list_t _qos_alc_t;
	LIST			class_map_list;
	void			*class_map_mutex;
	LIST			service_policy_list;
	void			*service_policy_mutex;
	zpl_uint32		init;
}global_qos_access_list_t;


/* ACL内部的规则 */
extern qos_access_filter_t *qos_access_filter_alloc(void);
extern int qos_access_filter_free(qos_access_filter_t *node);
extern int qos_access_filter_list_add(qos_access_filter_list_t *acllist, qos_access_filter_t *node);
extern int qos_access_filter_list_del(qos_access_filter_list_t *acllist, qos_access_filter_t *node);
extern qos_access_filter_t *qos_access_filter_list_lookup(qos_access_filter_list_t *acllist, zpl_int32 seqnum, qos_access_filter_t *node);
extern int qos_access_filter_list_foreach(qos_access_filter_list_t *acllist, int (*cb)(void *, qos_access_filter_t *), void *pVoid);
extern enum filter_type qos_access_filter_list_apply(qos_access_filter_list_t *acllist, void *object);

/* ACL */
extern qos_access_filter_list_t * qos_access_list_create(char *name);
extern int qos_access_list_add(qos_access_filter_list_t *node);
extern qos_access_filter_list_t * qos_access_list_lookup(char *name);
extern int qos_access_list_destroy(char *name);
extern int qos_access_list_reference(char *name, zpl_bool enable);
extern int qos_access_list_foreach(int (*cb)(void *, qos_access_filter_list_t *), void *pVoid);

/* class-map Refresh */
extern qos_class_map_t *qos_class_map_create(char *name);
extern qos_class_map_t *qos_class_map_lookup(char *name);
extern int qos_class_map_add(qos_class_map_t *node);
extern int qos_class_map_del(qos_class_map_t *node);
extern int qos_class_map_reference(char *name, zpl_bool enable);
extern int qos_class_map_bind_access_list_set(qos_class_map_t *node, char *name, zpl_bool enable);
extern char * qos_class_map_bind_access_list_get(qos_class_map_t *node);
//extern int qos_class_map_del(qos_class_map_t *node);
extern int qos_class_map_foreach(int (*cb)(void *, qos_class_map_t *), void *pVoid);
extern int qos_class_map_clean(void);
extern int qos_class_map_limit_set(qos_class_map_t *node, nsm_qos_limit_t *rate);
extern int qos_class_map_limit_get(qos_class_map_t *node, nsm_qos_limit_t *rate);


/* service-policy */
extern qos_service_policy_t *qos_service_policy_create(char *name);
extern qos_service_policy_t *qos_service_policy_lookup(char *name);
extern int qos_service_policy_add(qos_service_policy_t *node);
extern int qos_service_policy_del(qos_service_policy_t *node);
extern int qos_service_policy_reference(char *name, zpl_bool enable);
extern int qos_service_policy_bind_class_map_set(qos_service_policy_t *node, char *name, zpl_bool enable);
extern char * qos_service_policy_bind_class_map_get(qos_service_policy_t *node);
extern int qos_service_policy_foreach(int (*cb)(void *, qos_service_policy_t *), void *pVoid);
extern int qos_service_policy_clean(void);

//extern int qos_service_policy_bind_ifpset(char *name, zpl_bool enable);

extern int qos_access_list_init(void);
extern int qos_access_list_exit(void);
extern int qos_access_list_clean(void);

#ifdef ZPL_SHELL_MODULE
extern void cmd_qos_acl_init(void);
extern int qos_access_list_write_config(struct vty *vty, char *name);
extern int qos_class_map_write_config(struct vty *vty, char *name);
extern int qos_service_policy_write_config(struct vty *vty, char *name);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_NSM_QOS_ACL_H_ */

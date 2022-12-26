/*
 * nsm_vlan_database.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_VLAN_DATABASE_H__
#define __NSM_VLAN_DATABASE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"

#ifdef PRODUCT_PORT_MAX
#ifndef PHY_PORT_MAX
#define PHY_PORT_MAX 	PRODUCT_PORT_MAX
#endif
#endif

#define VLAN_ID_RANGE(n)    (((n)>=1)&&((n) <= VLAN_TABLE_MAX))


typedef struct
{
  zpl_uchar bitmap[VLAN_TABLE_MAX/8+1];
}zpl_vlan_bitmap_t;

void zpl_vlan_bitmap_init(zpl_vlan_bitmap_t bitmap);
void zpl_vlan_bitmap_set(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
void zpl_vlan_bitmap_clr(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
int zpl_vlan_bitmap_tst(zpl_vlan_bitmap_t bitmap, zpl_uint32  bit);
void zpl_vlan_bitmap_or(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);
void zpl_vlan_bitmap_xor(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);			
void zpl_vlan_bitmap_and(zpl_vlan_bitmap_t dst, const zpl_vlan_bitmap_t src1,
			const zpl_vlan_bitmap_t src2);

int zpl_vlan_bitmap_cmp(const zpl_vlan_bitmap_t src1, const zpl_vlan_bitmap_t src2);
void zpl_vlan_bitmap_copy(const zpl_vlan_bitmap_t src, zpl_vlan_bitmap_t dst);

/* vlan database 管理 */

typedef enum nsm_vlan_mode_s
{
	NSM_VLAN_UNTAG = 1,
	NSM_VLAN_TAG,
}nsm_vlan_mode_t;


typedef struct nsm_vlan_database_s
{
	NODE	node;
	vlan_t	vlan;
	vlan_t 	minvlan;
	vlan_t	maxvlan;
    zpl_uint32  mstp;
	ifindex_t tagport[PHY_PORT_MAX];
	ifindex_t untagport[PHY_PORT_MAX];
	zpl_char *vlan_name;
	zpl_uint32  name_hash;
}nsm_vlan_database_t;

typedef struct Gl2vlan_Database_s
{
	LIST	    *vlanList;
	void	    *mutex;
	zpl_bool    enable;
	zpl_bool    port_base_enable;

	zpl_bool	qinq_enable;
	vlan_t		qinq_tpid;
}Gl2vlan_Database_t;

typedef int (*nsm_vlan_database_cb)(nsm_vlan_database_t *, void *);

extern int nsm_vlan_database_init(void);
extern int nsm_vlan_database_exit(void);
extern int nsm_vlan_database_cleanall(void);
extern int nsm_vlan_database_default(void);

extern int nsm_vlan_database_enable(zpl_bool enable);
extern zpl_bool nsm_vlan_database_is_enable(void);

extern int nsm_vlan_portbase_enable(zpl_bool enable);
extern zpl_bool nsm_vlan_portbase_is_enable(void);

extern int nsm_vlan_qinq_enable(zpl_bool enable);
extern zpl_bool nsm_vlan_qinq_is_enable(void);
extern int nsm_vlan_dot1q_tpid_set_api(vlan_t num);
extern vlan_t nsm_vlan_dot1q_tpid_get_api(void);

extern int nsm_vlan_database_list_split_api(const char *str, vlan_t *vlanlist);
extern int nsm_vlan_database_list_lookup_api(vlan_t *vlanlist, zpl_uint32 num);

extern int nsm_vlan_database_list_create_api(const char *str);
extern int nsm_vlan_database_list_destroy_api(const char *str);
extern int nsm_vlan_database_create_api(vlan_t vlan, const char *name);
extern int nsm_vlan_database_destroy_api(vlan_t vlan);
extern int nsm_vlan_database_batch_create_api(vlan_t minvlan, vlan_t maxvlan);
extern int nsm_vlan_database_batch_destroy_api(vlan_t minvlan, vlan_t maxvlan);

extern int nsm_vlan_database_name_api(vlan_t vlan, const char *name);
extern void * nsm_vlan_database_lookup_api(vlan_t vlan);
extern void * nsm_vlan_database_lookup_by_name_api(const char *name);

extern int nsm_vlan_database_callback_api(nsm_vlan_database_cb cb, void *);


extern int nsm_vlan_database_add_port(vlan_t vlan, ifindex_t ifindex, nsm_vlan_mode_t mode);
extern int nsm_vlan_database_del_port(vlan_t vlan, ifindex_t ifindex, nsm_vlan_mode_t mode);
extern int nsm_vlan_database_clear_port(ifindex_t ifindex, nsm_vlan_mode_t mode);




#ifdef ZPL_SHELL_MODULE
extern void cmd_vlan_database_init (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NSM_VLAN_DATABASE_H__ */

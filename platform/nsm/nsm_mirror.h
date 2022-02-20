/*
 * nsm_mirror.h
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */

#ifndef __NSM_MIRROR_H__
#define __NSM_MIRROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_mac.h"

typedef enum
{
	MIRROR_NONE = 0,
	MIRROR_BOTH,
	MIRROR_INGRESS,
	MIRROR_EGRESS,
}mirror_dir_en;


typedef enum mirror_filter_e {
    MIRROR_FILTER_ALL 	= 0,
    MIRROR_FILTER_DA 	= 1,
	MIRROR_FILTER_SA 	= 2,
	MIRROR_FILTER_BOTH	= 3,
} mirror_filter_t;

typedef enum mirror_mode_e {
    MIRROR_SOURCE_PORT 	= 1,
	MIRROR_SOURCE_MAC 	= 2,
	MIRROR_SOURCE_DIV	= 3,
} mirror_mode_t;


struct Gmirror_s;

typedef struct nsm_mirror_s
{
	NODE			node;
	zpl_bool			enable;
	ifindex_t		ifindex;
	zpl_bool			mirror_dst;
	mirror_dir_en	dir;

	struct Gmirror_s	*global;

}nsm_mirror_t;

typedef struct Gmirror_s
{
	zpl_bool			enable;
	LIST		*mirrorList;
	void		*mutex;

	zpl_bool		in_enable;
	zpl_bool		ingress_dst;
	zpl_uchar		ingress_mac[NSM_MAC_MAX];
	zpl_bool		out_enable;
	zpl_bool		egress_dst;
	zpl_uchar		egress_mac[NSM_MAC_MAX];
}Gmirror_t;

typedef int (*mirror_cb)(nsm_mirror_t *, void *);

extern int nsm_mirror_init(void);
extern int nsm_mirror_exit(void );
extern int nsm_mirror_global_enable(zpl_bool enable);
extern zpl_bool nsm_mirror_global_is_enable(void);

extern zpl_bool nsm_mirror_is_enable_api(ifindex_t ifindex);

//mirror destination
extern int nsm_mirror_destination_set_api(ifindex_t ifindex, zpl_bool enable);
extern int nsm_mirror_destination_get_api(ifindex_t ifindex, zpl_bool *enable);
extern zpl_bool nsm_mirror_is_destination_api(ifindex_t ifindex);

// mirror source
/*int nsm_mirror_mode_set_api(zpl_bool mac);
int nsm_mirror_mode_get_api(zpl_bool *mac);

int nsm_mirror_source_mac_set_api(zpl_bool enable, zpl_uchar *mac, mirror_dir_en dir);
int nsm_mirror_source_mac_get_api(zpl_bool *enable, zpl_uchar *mac, mirror_dir_en *dir);
*/
extern int nsm_mirror_source_set_api(ifindex_t ifindex, zpl_bool enable, mirror_dir_en dir);
extern int nsm_mirror_source_get_api(ifindex_t ifindex, zpl_bool *enable, mirror_dir_en *dir);
extern zpl_bool nsm_mirror_is_source_api(void);

extern int nsm_mirror_source_mac_filter_set_api(zpl_bool enable, zpl_uchar *mac, zpl_bool dst,  mirror_dir_en dir);
//extern int nsm_mirror_source_mac_filter_set_api(zpl_bool enable, zpl_uchar *mac, mirror_dir_en dir, zpl_bool dst);
extern int nsm_mirror_source_mac_filter_get_api(mirror_dir_en dir, zpl_bool *enable, zpl_uchar *mac, zpl_bool *dst);

extern int mirror_callback_api(mirror_cb cb, void *pVoid);
#ifdef ZPL_SHELL_MODULE
extern void cmd_mirror_init(void);
extern int bulid_mirror_config(struct vty *vty);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __NSM_MIRROR_H__ */

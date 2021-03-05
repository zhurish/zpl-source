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


struct Gmirror_s;

typedef struct nsm_mirror_s
{
	NODE			node;
	ospl_bool			enable;
	ifindex_t		ifindex;
	ospl_bool			mirror_dst;
	mirror_dir_en	dir;

	struct Gmirror_s	*global;

}nsm_mirror_t;

typedef struct Gmirror_s
{
	ospl_bool			enable;
	LIST		*mirrorList;
	void		*mutex;

	ospl_bool		in_enable;
	ospl_bool		ingress_dst;
	ospl_uchar		ingress_mac[NSM_MAC_MAX];
	ospl_bool		out_enable;
	ospl_bool		egress_dst;
	ospl_uchar		egress_mac[NSM_MAC_MAX];
}Gmirror_t;

typedef int (*mirror_cb)(nsm_mirror_t *, void *);

extern int nsm_mirror_init(void);
extern int nsm_mirror_exit(void );
extern int nsm_mirror_global_enable(ospl_bool enable);
extern ospl_bool nsm_mirror_global_is_enable();

extern ospl_bool nsm_mirror_is_enable_api(ifindex_t ifindex);

//mirror destination
extern int nsm_mirror_destination_set_api(ifindex_t ifindex, ospl_bool enable);
extern int nsm_mirror_destination_get_api(ifindex_t ifindex, ospl_bool *enable);
extern ospl_bool nsm_mirror_is_destination_api(ifindex_t ifindex);

// mirror source
/*int nsm_mirror_mode_set_api(ospl_bool mac);
int nsm_mirror_mode_get_api(ospl_bool *mac);

int nsm_mirror_source_mac_set_api(ospl_bool enable, ospl_uchar *mac, mirror_dir_en dir);
int nsm_mirror_source_mac_get_api(ospl_bool *enable, ospl_uchar *mac, mirror_dir_en *dir);
*/
extern int nsm_mirror_source_set_api(ifindex_t ifindex, ospl_bool enable, mirror_dir_en dir);
extern int nsm_mirror_source_get_api(ifindex_t ifindex, ospl_bool *enable, mirror_dir_en *dir);
extern ospl_bool nsm_mirror_is_source_api();

extern int nsm_mirror_source_mac_filter_set_api(ospl_bool enable, ospl_uchar *mac, ospl_bool dst,  mirror_dir_en dir);
//extern int nsm_mirror_source_mac_filter_set_api(ospl_bool enable, ospl_uchar *mac, mirror_dir_en dir, ospl_bool dst);
extern int nsm_mirror_source_mac_filter_get_api(mirror_dir_en dir, ospl_bool *enable, ospl_uchar *mac, ospl_bool *dst);

extern int mirror_callback_api(mirror_cb cb, void *pVoid);

extern void cmd_mirror_init(void);
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_MIRROR_H__ */

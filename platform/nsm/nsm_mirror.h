/*
 * nsm_mirror.h
 *
 *  Created on: May 11, 2018
 *      Author: zhurish
 */

#ifndef __NSM_MIRROR_H__
#define __NSM_MIRROR_H__

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
	BOOL			enable;
	ifindex_t		ifindex;
	BOOL			mirror_dst;
	mirror_dir_en	dir;

	struct Gmirror_s	*global;

}nsm_mirror_t;

typedef struct Gmirror_s
{
	BOOL			enable;
	LIST		*mirrorList;
	void		*mutex;

	BOOL		in_enable;
	BOOL		ingress_dst;
	u_char		ingress_mac[NSM_MAC_MAX];
	BOOL		out_enable;
	BOOL		egress_dst;
	u_char		egress_mac[NSM_MAC_MAX];
}Gmirror_t;

typedef int (*mirror_cb)(nsm_mirror_t *, void *);

extern int nsm_mirror_init(void);
extern int nsm_mirror_exit(void );
extern int nsm_mirror_global_enable(BOOL enable);
extern BOOL nsm_mirror_global_is_enable();

extern BOOL nsm_mirror_is_enable_api(ifindex_t ifindex);

//mirror destination
extern int nsm_mirror_destination_set_api(ifindex_t ifindex, BOOL enable);
extern int nsm_mirror_destination_get_api(ifindex_t ifindex, BOOL *enable);
extern BOOL nsm_mirror_is_destination_api(ifindex_t ifindex);

// mirror source
/*int nsm_mirror_mode_set_api(BOOL mac);
int nsm_mirror_mode_get_api(BOOL *mac);

int nsm_mirror_source_mac_set_api(BOOL enable, u_char *mac, mirror_dir_en dir);
int nsm_mirror_source_mac_get_api(BOOL *enable, u_char *mac, mirror_dir_en *dir);
*/
extern int nsm_mirror_source_set_api(ifindex_t ifindex, BOOL enable, mirror_dir_en dir);
extern int nsm_mirror_source_get_api(ifindex_t ifindex, BOOL *enable, mirror_dir_en *dir);
extern BOOL nsm_mirror_is_source_api();

extern int nsm_mirror_source_mac_filter_set_api(BOOL enable, u_char *mac, BOOL dst,  mirror_dir_en dir);
//extern int nsm_mirror_source_mac_filter_set_api(BOOL enable, u_char *mac, mirror_dir_en dir, BOOL dst);
extern int nsm_mirror_source_mac_filter_get_api(mirror_dir_en dir, BOOL *enable, u_char *mac, BOOL *dst);

extern int mirror_callback_api(mirror_cb cb, void *pVoid);

extern void cmd_mirror_init(void);

#endif /* __NSM_MIRROR_H__ */

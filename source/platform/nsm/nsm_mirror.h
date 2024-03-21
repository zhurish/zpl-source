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

#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif

#define MIRROR_SESSION_MAX	2
#define MIRROR_SOURCE_MAX	4

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
} mirror_mode_t;

typedef struct nsm_mirror_source_s
{
	zpl_bool		enable;
	ifindex_t		ifindex;
	mirror_dir_en	dir;
	mirror_filter_t filter;
	zpl_uchar		mac[NSM_MAC_MAX];
}nsm_mirror_source_t;

typedef struct nsm_mirror_session_s
{
	zpl_bool			enable;
	ifindex_t			ifindex;
	mirror_mode_t		mode;
	nsm_mirror_source_t	srcport[MIRROR_SOURCE_MAX];
	nsm_mirror_source_t	ingress_filter[MIRROR_SOURCE_MAX];
	nsm_mirror_source_t	egress_filter[MIRROR_SOURCE_MAX];
}nsm_mirror_session_t;

typedef struct Gmirror_s
{
	void		*mutex;
	nsm_mirror_session_t	mirror_session[MIRROR_SESSION_MAX];
}Gmirror_t;


extern int nsm_mirror_init(void);
extern int nsm_mirror_exit(void );

//mirror destination
extern int nsm_mirror_destination_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable);
extern int nsm_mirror_destination_get_api(zpl_uint32 id, ifindex_t *ifindex, zpl_bool *enable);
extern zpl_bool nsm_mirror_is_destination_api(ifindex_t ifindex, zpl_uint32 *id);

// mirror source

extern int nsm_mirror_source_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable, mirror_dir_en dir);
extern int nsm_mirror_source_get_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool *enable, mirror_mode_t *mode, mirror_dir_en *dir);
extern zpl_bool nsm_mirror_is_source_api(ifindex_t ifindex, zpl_uint32 *id, mirror_mode_t *mode, 
	mirror_dir_en *dir,mirror_filter_t *filter, zpl_uint32 *index);

extern int nsm_mirror_source_mac_filter_set_api(zpl_uint32 id, ifindex_t ifindex, zpl_bool enable,
	mirror_dir_en dir, mirror_filter_t filter, zpl_uchar *mac);


#ifdef ZPL_SHELL_MODULE
extern void cmd_mirror_init(void);
extern int bulid_mirror_config(struct vty *vty, void *);
extern int bulid_mirror_show(struct vty *vty);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __NSM_MIRROR_H__ */

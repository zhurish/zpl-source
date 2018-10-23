/*
 * nsm_iw.h
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */

#ifndef __NSM_IW_H__
#define __NSM_IW_H__

#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"

typedef struct iw_s
{
	struct interface	*ifp;
	iw_mode_t			mode;
	union
	{
		iw_ap_t		ap;
		iw_client_t	client;
	}private;

}iw_t;


extern int nsm_iw_client_exit();
extern int nsm_iw_client_exit();

extern iw_t * nsm_iw_get(struct interface *ifp);
extern int nsm_iw_mode_set_api(struct interface *ifp, iw_mode_t mode);
extern int nsm_iw_mode_get_api(struct interface *ifp, iw_mode_t *mode);

extern iw_mode_t nsm_iw_mode(struct interface *ifp);


extern int nsm_iw_channel_freq_show(struct interface *ifp, struct vty *vty);

extern int nsm_iw_capabilities_show(struct interface *ifp, struct vty *vty);
extern int nsm_iw_ap_info_show(struct interface *ifp, struct vty *vty);


#endif /* __NSM_IW_H__ */

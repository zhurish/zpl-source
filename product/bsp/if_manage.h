/*
 * if_usp.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef IF_USP_H_
#define IF_USP_H_

//#include "zebra.h"
//#include "linklist.h"

#ifdef PL_MODEM_MODULE
#define MODEM_PHY_MAX	1
#else
#define MODEM_PHY_MAX	0
#endif

#ifdef PL_WIFI_MODULE
#define WIFI_PHY_MAX	1
#else
#define WIFI_PHY_MAX	0
#endif



#define VTY_IUSP_DEBUG

//extern int unit_slot_module_init();
extern int bsp_usp_module_init();
extern const int if_ifindex2phy(ifindex_t ifindex);
#ifdef USE_IPSTACK_KERNEL
extern ifindex_t ifindex_lookup_by_kname(const char *kname);
extern const char * if_kernel_name_lookup(ifindex_t ifindex);
extern int if_slot_set_port_phy(ifindex_t ifindex, char *name);
extern int if_slot_show_port_phy(struct vty *vty);
#endif
#endif /* IF_USP_H_ */

/*
 * hal_igmp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_IGMP_H__
#define __HAL_IGMP_H__

#ifdef __cplusplus
extern "C" {
#endif



/* Initialize IGMP Snooping. */
extern int hal_igmp_snooping_init(void);

/* Enable IGMP Snooping. */
extern int hal_igmp_snooping_set(zpl_uint32 type, zpl_bool enable);

extern int hal_igmpunknow_snooping_set(zpl_uint32 type, zpl_bool enable);

extern int hal_igmpqry_snooping_set(zpl_uint32 type, zpl_bool enable);

extern int hal_mldqry_snooping_set(zpl_uint32 type, zpl_bool enable);

extern int hal_mld_snooping_set(zpl_uint32 type, zpl_bool enable);

extern int hal_igmp_ipcheck_enable(zpl_bool enable);
extern int hal_arp_snooping_set(zpl_bool enable);
extern int hal_rarp_snooping_set(zpl_bool enable);
extern int hal_dhcp_snooping_set(zpl_bool enable);

    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_IGMP_H__ */

/*
 * hal_misc.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MISC_H__
#define __HAL_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif


enum hal_misc_cmd 
{
    HAL_MISC_NONE,
	HAL_MISC_JUMBO,
	HAL_MISC_JUMBO_SIZE,
	HAL_MISC_DHCP_SNOOP,
	HAL_MISC_IGMP_SNOOP,
    HAL_MISC_EEE,
};

//jumbo
int hal_jumbo_size(zpl_uint32 size);
int hal_jumbo_interface_enable(ifindex_t ifindex, zpl_bool enable);

//snooping
int hal_snooping_enable (zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6);

//EEE
int hal_eee_enable (ifindex_t, zpl_bool);
int hal_eee_set (ifindex_t, void *);
int hal_eee_unset (ifindex_t, void *);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MISC_H__ */

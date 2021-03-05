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

typedef struct sdk_misc_s
{
	//jumbo
	int (*sdk_jumbo_enable_cb) (void *, ifindex_t, ospl_bool);
	int (*sdk_jumbo_size_cb) (void *, ospl_uint32);

	int (*sdk_snooping_cb) (void *, ospl_bool enable, ospl_uint32 mode, ospl_bool ipv6);


	//EEE
	int (*sdk_eee_enable_cb) (void *, ifindex_t, ospl_bool);
	int (*sdk_eee_set_cb) (void *, ifindex_t, void *);
	int (*sdk_eee_unset_cb) (void *, ifindex_t, void *);

	void *sdk_driver;
}sdk_misc_t;

//jumbo
int hal_jumbo_size(ospl_uint32 size);
int hal_jumbo_interface_enable(ifindex_t ifindex, ospl_bool enable);

//snooping
int hal_snooping_enable (ospl_bool enable, ospl_uint32 mode, ospl_bool ipv6);

//EEE
int hal_eee_enable (ifindex_t, ospl_bool);
int hal_eee_set (ifindex_t, void *);
int hal_eee_unset (ifindex_t, void *);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MISC_H__ */

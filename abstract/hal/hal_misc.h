/*
 * hal_misc.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MISC_H__
#define __HAL_MISC_H__



typedef struct sdk_misc_s
{
	//jumbo
	int (*sdk_jumbo_enable_cb) (ifindex_t, BOOL);
	int (*sdk_jumbo_size_cb) (int);

	int (*sdk_snooping_cb) (BOOL enable, int mode, BOOL ipv6);


	//EEE
	int (*sdk_eee_enable_cb) (ifindex_t, BOOL);
	int (*sdk_eee_set_cb) (ifindex_t, void *);
	int (*sdk_eee_unset_cb) (ifindex_t, void *);

}sdk_misc_t;

//jumbo
int hal_jumbo_size(int size);
int hal_jumbo_interface_enable(ifindex_t ifindex, BOOL enable);

//snooping
int hal_snooping_enable (BOOL enable, int mode, BOOL ipv6);

//EEE
int hal_eee_enable (ifindex_t, BOOL);
int hal_eee_set (ifindex_t, void *);
int hal_eee_unset (ifindex_t, void *);

#endif /* __HAL_MISC_H__ */

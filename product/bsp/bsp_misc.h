/*
 * bsp_misc.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MISC_H__
#define __BSP_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct sdk_misc_s
{
	//jumbo
	int (*sdk_jumbo_enable_cb) (void *, zpl_phyport_t, zpl_bool);
	int (*sdk_jumbo_size_cb) (void *, zpl_uint32);

	int (*sdk_snooping_cb) (void *, zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6);


	//EEE
	int (*sdk_eee_enable_cb) (void *, zpl_phyport_t, zpl_bool);
	int (*sdk_eee_set_cb) (void *, zpl_phyport_t, void *);
	int (*sdk_eee_unset_cb) (void *, zpl_phyport_t, void *);

	void *sdk_driver;
}sdk_misc_t;

//jumbo
int bsp_jumbo_size(zpl_uint32 size);
int bsp_jumbo_interface_enable(zpl_phyport_t ifindex, zpl_bool enable);

//snooping
int bsp_snooping_enable (zpl_bool enable, zpl_uint32 mode, zpl_bool ipv6);

//EEE
int bsp_eee_enable (zpl_phyport_t, zpl_bool);
int bsp_eee_set (zpl_phyport_t, void *);
int bsp_eee_unset (zpl_phyport_t, void *);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __BSP_MISC_H__ */

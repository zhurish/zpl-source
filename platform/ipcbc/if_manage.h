/*
 * if_namege.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef __IF_MANAGE_H__
#define __IF_MANAGE_H__

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef ZPL_MODEM_MODULE
#define MODEM_PHY_MAX	1
#else
#define MODEM_PHY_MAX	0
#endif

#ifdef ZPL_WIFI_MODULE
#define WIFI_PHY_MAX	1
#else
#define WIFI_PHY_MAX	0
#endif



//#define VTY_IUSP_DEBUG

//extern int unit_slot_module_init();
extern int bsp_usp_module_init();

#ifdef ZPL_KERNEL_STACK_MODULE
extern int if_ktest_init(void);
#endif


 
#ifdef __cplusplus
}
#endif


#endif /* __IF_MANAGE_H__ */

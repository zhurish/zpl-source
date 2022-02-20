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


/* 动态上线下线板卡 */
extern int unit_board_dynamic_install(zpl_uint8 unit, zpl_uint8 slot, zpl_bool enable);

/* 初始化 */
extern int bsp_usp_module_init(void);


#ifdef ZPL_KERNEL_STACK_MODULE
extern int if_ktest_init(void);
#endif


 
#ifdef __cplusplus
}
#endif


#endif /* __IF_MANAGE_H__ */

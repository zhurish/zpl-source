/*
 * product.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_PRODUCT_H_
#define INCLUDE_PRODUCT_H_


#define PRODUCT_X5_B_BOARD
//#define PRODUCT_BOJING_BOARD

#define OS_SLOT_MAX 	1
#define OS_SLOT_HY_MAX 	5


#define PRODUCT_PORT_MAX	16

#ifdef USE_IPSTACK_KERNEL
/*
 * kernel 接口映射配置文件
 */
//#define SLOT_PORT_CONF	"/etc/plat.conf"
#define SLOT_PORT_CONF	SYSCONF_REAL_DIR"/plat.conf"


#endif

#endif /* INCLUDE_PRODUCT_H_ */

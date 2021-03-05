/*
 * product.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_PRODUCT_H_
#define INCLUDE_PRODUCT_H_
#ifdef __cplusplus
extern "C" {
#endif

//#define PRODUCT_X5_B_BOARD
//#define PRODUCT_BOJING_BOARD
//#define PRODUCT_V9_BOARD

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

#ifndef RT_TABLE_MAIN
//#define RT_TABLE_MAIN 0
#endif
//#define USE_IPSTACK_KERNEL
#ifndef USE_IPSTACK_KERNEL
#define IPNET
#endif

#define IF_IUSPV_SUPPORT //support sub interface, eg:0/1/1.22


#define ROUTE_TAG_MAX UINT32_MAX

#ifndef NSM_MAC_MAX
#define NSM_MAC_MAX	6
#endif

#include "product.h"

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PRODUCT_H_ */

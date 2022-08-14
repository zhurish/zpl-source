/*
 * bsp_types.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_TYPES_H__
#define __BSP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ZPL_SDK_USER) || defined(ZPL_SDK_NONE)
#include "zplos_include.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "hal_client.h"
#include "bsp_driver.h"
#include "bsp_include.h"

#define msleep  os_msleep
#define usleep_range(a,b)   os_usleep((a+b)/2)
#else

#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TYPES_H__ */

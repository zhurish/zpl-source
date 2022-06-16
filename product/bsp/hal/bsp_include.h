/*
 * bsp_include.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_INCLUDE_H__
#define __BSP_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "hal_client.h"

#if defined(ZPL_SDK_MODULE) && defined(ZPL_SDK_USER)
#include "bsp_driver.h"
#endif

#include "bsp_global.h"
#include "bsp_l3if.h"
#include "bsp_route.h"
#ifdef ZPL_NSM_8021X
#include "bsp_8021x.h"
#endif
#ifdef ZPL_NSM_DOS
#include "bsp_dos.h"
#endif
#ifdef ZPL_NSM_MAC
#include "bsp_mac.h"
#endif
#ifdef ZPL_NSM_MIRROR
#include "bsp_mirror.h"
#endif
#include "bsp_misc.h"
#ifdef ZPL_NSM_MSTP
#include "bsp_mstp.h"
#endif
#include "bsp_port.h"
#ifdef ZPL_NSM_VLAN
#include "bsp_vlan.h"
#include "bsp_qinq.h"
#endif
#ifdef ZPL_NSM_QOS
#include "bsp_qos.h"
#endif
#ifdef ZPL_NSM_TRUNK
#include "bsp_trunk.h"
#endif

#include "bsp_cpu.h"


#ifdef __cplusplus
}
#endif

#endif /* __BSP_INCLUDE_H__ */

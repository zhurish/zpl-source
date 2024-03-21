/*
 * hal_driver.h
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#ifndef __HAL_INCLUDE_H__
#define __HAL_INCLUDE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <auto_include.h>
#include <zpl_type.h>

#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_ipcsrv.h"

#include "hal_global.h"
#include "hal_route.h"
#include "hal_l3if.h"
#ifdef ZPL_NSM_8021X
#include "hal_8021x.h"
#endif
#ifdef ZPL_NSM_DOS
#include "hal_dos.h"
#endif
#ifdef ZPL_NSM_MAC
#include "hal_mac.h"
#endif
#ifdef ZPL_NSM_MIRROR
#include "hal_mirror.h"
#endif
#include "hal_misc.h"
#ifdef ZPL_NSM_MSTP
#include "hal_mstp.h"
#endif
#include "hal_port.h"
#ifdef ZPL_NSM_VLAN
#include "hal_vlan.h"
#include "hal_qinq.h"
#endif
#ifdef ZPL_NSM_QOS
#include "hal_qos.h"
#endif
#ifdef ZPL_NSM_TRUNK
#include "hal_trunk.h"
#endif
#ifdef ZPL_NSM_IGMP
#include "hal_igmp.h"
#endif
/*
#include "hal_switch.h"
#include "hal_oam.h"
#include "hal_acl.h"
#include "hal_filter.h"
#include "hal_ipfix.h"
#include "hal_l2mc.h"
#include "hal_l3mc.h"
#include "hal_l2.h"
#include "hal_l3.h"
#include "hal_mpls.h"
#include "hal_vpls.h"
#include "hal_rate.h"
#include "hal_stat.h"
#include "hal_tunnel.h"
#include "hal_wlan.h"
#include "hal_tx.h"
#include "hal_rx.h"
*/
#include "hal_driver.h"


#ifdef __cplusplus
}
#endif

#endif /* __HAL_INCLUDE_H__ */

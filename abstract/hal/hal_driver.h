/*
 * hal_driver.h
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#ifndef __HAL_DRIVER_H__
#define __HAL_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <zpl_include.h>

#include "zpl_include.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
#include <log.h>


#include "nsm_trunk.h"
#include "nsm_vlan.h"
#include "nsm_dos.h"
#include "nsm_mirror.h"
//#include "nsm_switch.h"

#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_8021x.h"
#include "hal_dos.h"
#include "hal_mac.h"
#include "hal_mirror.h"
#include "hal_misc.h"
#include "hal_mstp.h"
#include "hal_port.h"
#include "hal_qinq.h"
#include "hal_trunk.h"
#include "hal_vlan.h"
//#include "hal_switch.h"

#ifdef ZPL_SDK_MODULE
#define NO_SDK ERROR
#else
#define NO_SDK OK
#endif

typedef struct 
{
	zpl_void	*master;
	zpl_uint32 taskid;
}hal_driver_t;

enum hal_switch_cmd 
{
    HAL_SWITCH_NONE,
	HAL_SWITCH_MANEGE,
	HAL_SWITCH_FORWARD,
	HAL_SWITCH_MULTICAST_FLOOD,
	HAL_SWITCH_UNICAST_FLOOD,
	HAL_SWITCH_MULTICAST_LEARNING,
	HAL_SWITCH_BPDU,
	HAL_SWITCH_AGINT,
	HAL_SWITCH_CPU_MODE,
	HAL_SWITCH_CPU,
	HAL_SWITCH_CPU_SPEED,
	HAL_SWITCH_CPU_DUPLEX,
	HAL_SWITCH_CPU_FLOW,
};

int hal_module_init();
int hal_module_exit();
int hal_module_task_init();
int hal_module_task_exit();

int hal_test_init();
/*
 * CPU Port
 */
int hal_cpu_port_mode(zpl_bool enable);
int hal_cpu_port_enable(zpl_bool enable);
int hal_cpu_port_speed(zpl_uint32 value);
int hal_cpu_port_duplex(zpl_uint32 value);
int hal_cpu_port_flow(zpl_bool rx, zpl_bool tx);

/*
 * Global
 */
int hal_switch_mode(zpl_bool manage);
int hal_switch_forward(zpl_bool enable);
int hal_multicast_flood(zpl_bool enable);
int hal_unicast_flood(zpl_bool enable);
int hal_multicast_learning(zpl_bool enable);
//全局使能接收BPDU报文
int hal_global_bpdu_enable(zpl_bool enable);
int hal_global_aging_time(zpl_uint32 value);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_DRIVER_H__ */

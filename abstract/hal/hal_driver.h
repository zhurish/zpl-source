/*
 * hal_driver.h
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#ifndef __HAL_DRIVER_H__
#define __HAL_DRIVER_H__

#include <zebra.h>

#include "zebra.h"
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
#include "nsm_qos.h"

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
#include "hal_qos.h"


typedef struct sdk_global_s
{
	int (*sdk_switch_manege_cb) (void *, BOOL);
	int (*sdk_switch_forward_cb) (void *, BOOL);
	int (*sdk_multicast_flood_cb) (void *, BOOL);
	int (*sdk_unicast_flood_cb) (void *, BOOL);
	int (*sdk_multicast_learning_cb) (void *, BOOL);
	int (*sdk_bpdu_enable_cb) (void *, BOOL);//全局使能接收BPDU报文
	int (*sdk_aging_time_cb) (void *, u_int);
}sdk_global_t;

typedef struct sdk_cpu_s
{
	int (*sdk_cpu_mode_cb) (void *, BOOL);
	int (*sdk_cpu_enable_cb) (void *, BOOL);
	int (*sdk_cpu_speed_cb) (void *, u_int);
	int (*sdk_cpu_duplex_cb) (void *, u_int);
	int (*sdk_cpu_flow_cb) (void *, BOOL, BOOL);
}sdk_cpu_t;


typedef struct hal_driver {
	int		product;
	int 	id;
	char 	*name;

	sdk_cpu_t		*cpu_tbl;
	sdk_global_t	*global_tbl;
	sdk_dos_t 		*dos_tbl;
	sdk_mstp_t 		*mstp_tbl;
	sdk_mac_t 		*mac_tbl;
	sdk_trunk_t 	*trunk_tbl;
	sdk_mirror_t 	*mirror_tbl;
	sdk_misc_t 		*misc_tbl;
	sdk_vlan_t 		*vlan_tbl;
	sdk_8021x_t		*q8021x_tbl;
	sdk_port_t		*port_tbl;
	sdk_qinq_t		*qinq_tbl;
	sdk_qos_t		*qos_tbl;

	//void			*sdk_driver;
	void			*driver;

}hal_driver_t;

extern hal_driver_t *hal_driver;

int hal_module_init();

int hal_test_init();
/*
 * CPU Port
 */
int hal_cpu_port_mode(BOOL enable);
int hal_cpu_port_enable(BOOL enable);
int hal_cpu_port_speed(u_int value);
int hal_cpu_port_duplex(u_int value);
int hal_cpu_port_flow(BOOL rx, BOOL tx);

/*
 * Global
 */
int hal_switch_mode(BOOL manage);
int hal_switch_forward(BOOL enable);
int hal_multicast_flood(BOOL enable);
int hal_unicast_flood(BOOL enable);
int hal_multicast_learning(BOOL enable);
//全局使能接收BPDU报文
int hal_global_bpdu_enable(BOOL enable);
int hal_global_aging_time(u_int value);

#endif /* __HAL_DRIVER_H__ */

/*
 * hal_global.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_GLOBAL_H__
#define __HAL_GLOBAL_H__

#ifdef __cplusplus
extern "C" {
#endif


/*
 * CPU Port
 */
int hal_cpu_port_mode(zpl_bool enable);
int hal_cpu_port_enable(zpl_bool enable);
int hal_cpu_port_speed(zpl_uint32 value);
int hal_cpu_port_duplex(zpl_uint32 value);
int hal_cpu_port_flow(zpl_bool rx, zpl_bool tx);



extern int hal_jumbo_size_set(zpl_uint32 size);
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

extern int hal_port_wan_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_driver_debug_set(zpl_bool enable, zpl_uint32 module, zpl_uint32 val);
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_GLOBAL_H__ */

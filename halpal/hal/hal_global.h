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

enum hal_global_cmd 
{
    HAL_GLOBAL_CMD_NONE,
    HAL_GLOBAL_CMD_JUMBO_SIZE,
	HAL_GLOBAL_MANEGE,
	HAL_GLOBAL_FORWARD,
	HAL_GLOBAL_MULTICAST_FLOOD,
	HAL_GLOBAL_UNICAST_FLOOD,
	HAL_GLOBAL_MULTICAST_LEARNING,
	HAL_GLOBAL_BPDU,
	HAL_GLOBAL_AGINT,  
	HAL_GLOBAL_WAN_PORT, 
};


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

	
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_GLOBAL_H__ */

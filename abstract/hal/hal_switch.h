/*
 * hal_switch.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_SWITCH_H__
#define __HAL_SWITCH_H__
#ifdef __cplusplus
extern "C" {
#endif

/* Configure port-specific and device-wide operating modes. */
extern int hal_switch_control_set(int unit, 
    zpl_uint32 type, 
    int arg);

/* Configure port-specific and device-wide operating modes. */
extern int hal_switch_control_port_set(ifindex_t ifindex, 
    zpl_uint32 type, 
    int arg);

    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_SWITCH_H__ */

/*
 * hal_igmp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_IGMP_H__
#define __HAL_IGMP_H__

#ifdef __cplusplus
extern "C" {
#endif

enum hal_igmp_cmd 
{
    HAL_IGMP_NONE,
    HAL_IGMP_INIT,
	HAL_IGMP_ENABLE,
	HAL_IGMP_DISABLE,
};

/* Initialize IGMP Snooping. */
extern int hal_igmp_snooping_init(void);

/* Enable IGMP Snooping. */
extern int hal_igmp_snooping_enable_set(zpl_bool enable);

	
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_IGMP_H__ */

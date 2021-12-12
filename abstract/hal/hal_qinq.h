/*
 * hal_qinq.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_QINQ_H__
#define __HAL_QINQ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_vlan.h"


enum hal_qinq_cmd 
{
    HAL_QINQ_CMD_NONE,
	HAL_QINQ_CMD_TPID,
	HAL_QINQ_CMD_ENABLE,
    HAL_QINQ_CMD_MAX,
};

typedef struct hal_qinq_param_s
{
	zpl_uint32 value;
}hal_qinq_param_t;

int hal_qinq_enable(zpl_bool enable);
int hal_qinq_vlan_tpid(vlan_t tpid);
int hal_qinq_interface_enable(ifindex_t ifindex, zpl_bool enable);



#ifdef __cplusplus
}
#endif


#endif /* __HAL_QINQ_H__ */

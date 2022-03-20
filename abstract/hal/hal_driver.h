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

typedef enum hal_core_cmd
{
	HAL_CORE_NONE,
  	HAL_CORE_COPY_TO_CPU,
	HAL_CORE_REDIRECT_TO_CPU,
  	HAL_CORE_FORWARED,
	HAL_CORE_DROP,
}hal_core_cmd_t;

enum hal_switch_cmd 
{
    HAL_SWITCH_NONE,
	HAL_SWITCH_CPU_MODE,
	HAL_SWITCH_CPU,
	HAL_SWITCH_CPU_SPEED,
	HAL_SWITCH_CPU_DUPLEX,
	HAL_SWITCH_CPU_FLOW,
};




int hal_module_init(void);
int hal_module_exit(void);
int hal_module_task_init(void);
int hal_module_task_exit(void);

int hal_test_init(void);


/*
 * CPU Port
 */
int hal_cpu_port_mode(zpl_bool enable);
int hal_cpu_port_enable(zpl_bool enable);
int hal_cpu_port_speed(zpl_uint32 value);
int hal_cpu_port_duplex(zpl_uint32 value);
int hal_cpu_port_flow(zpl_bool rx, zpl_bool tx);




#ifdef __cplusplus
}
#endif

#endif /* __HAL_DRIVER_H__ */

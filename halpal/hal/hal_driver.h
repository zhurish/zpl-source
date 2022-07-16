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
#ifdef ZPL_SDK_NONE
#define NO_SDK OK
#else
#define NO_SDK ERROR
#endif
#else
#define NO_SDK OK
#endif

typedef struct 
{
	zpl_void	*master;
	zpl_taskid_t taskid;
}hal_driver_t;


int hal_module_init(void);
int hal_module_exit(void);
int hal_module_task_init(void);
int hal_module_task_exit(void);

int hal_test_init(void);





#ifdef __cplusplus
}
#endif

#endif /* __HAL_DRIVER_H__ */

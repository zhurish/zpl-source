/*
 * bsp_cpu.h
 *
 *  Created on: 2019年9月8日
 *      Author: zhurish
 */

#ifndef __BSP_CPU_H__
#define __BSP_CPU_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER

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
#endif

typedef struct sdk_cpu_s
{
	int (*sdk_cpu_mode_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_cpu_enable_cb) (void *, hal_global_header_t*, zpl_bool);
	int (*sdk_cpu_speed_cb) (void *, hal_global_header_t*, zpl_uint32);
	int (*sdk_cpu_duplex_cb) (void *, hal_global_header_t*, zpl_uint32);
	int (*sdk_cpu_flow_cb) (void *, hal_global_header_t*, zpl_bool);
}sdk_cpu_t;



extern sdk_cpu_t sdk_cpu_cb;


extern int bsp_cpu_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_CPU_H__ */

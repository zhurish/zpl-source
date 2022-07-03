/*
 * bsp_trunk.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_TRUNK_H__
#define __BSP_TRUNK_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifdef ZPL_SDK_KERNEL
enum hal_trunk_cmd 
{
    HAL_TRUNK_CMD_NONE,
	HAL_TRUNK_CMD_ENABLE,
	HAL_TRUNK_CMD_CREATE,
	HAL_TRUNK_CMD_ADDIF,
	HAL_TRUNK_CMD_DELIF,
	HAL_TRUNK_CMD_MODE,
    HAL_TRUNK_CMD_MAX,
};

typedef struct hal_trunk_param_s
{
	zpl_bool enable;
	zpl_uint32 trunkid;
	zpl_uint32 mode;
}hal_trunk_param_t;
#endif
typedef enum {
    BSP_TRUNK_MODE_MACDASA = 0x0,
    BSP_TRUNK_MODE_MACDA = 0x1,
    BSP_TRUNK_MODE_MACSA = 0x2,
} bsp_trunk_mode_t;


typedef struct sdk_trunk_s
{
	int (*sdk_trunk_enable_cb) (void *, zpl_bool);
	int (*sdk_trunk_create_cb) (void *, zpl_index_t, zpl_bool);
	int (*sdk_trunk_mode_cb) (void *, zpl_index_t, zpl_uint32);
	int (*sdk_trunk_addif_cb) (void *, zpl_index_t, zpl_phyport_t);
	int (*sdk_trunk_delif_cb) (void *, zpl_index_t, zpl_phyport_t);
}sdk_trunk_t;

extern sdk_trunk_t sdk_trunk;
extern int bsp_trunk_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_TRUNK_H__ */

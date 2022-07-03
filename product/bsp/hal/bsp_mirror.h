/*
 * bsp_mirror.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MIRROR_H__
#define __BSP_MIRROR_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifdef ZPL_SDK_KERNEL
enum hal_mirror_cmd 
{
    HAL_MIRROR_CMD_NONE,
	HAL_MIRROR_CMD_DST_PORT,
	HAL_MIRROR_CMD_SRC_PORT,
	HAL_MIRROR_CMD_SRC_MAC,
    HAL_MIRROR_CMD_MAX,
};

typedef struct hal_mirror_param_s
{
	zpl_uint32 value;
	zpl_uint8 dir;
	zpl_uint8 filter;
	mac_t mac[NSM_MAC_MAX];
}hal_mirror_param_t;

typedef enum
{
	MIRROR_NONE = 0,
	MIRROR_BOTH,
	MIRROR_INGRESS,
	MIRROR_EGRESS,
}mirror_dir_en;


typedef enum mirror_filter_e {
    MIRROR_FILTER_ALL 	= 0,
    MIRROR_FILTER_DA 	= 1,
	MIRROR_FILTER_SA 	= 2,
	MIRROR_FILTER_BOTH	= 3,
} mirror_filter_t;
#endif
typedef struct sdk_mirror_s
{
	int (*sdk_mirror_enable_cb) (void *, zpl_phyport_t , zpl_bool );
	int (*sdk_mirror_source_enable_cb) (void *, zpl_phyport_t, zpl_bool , mirror_dir_en );
	int (*sdk_mirror_source_filter_enable_cb) (void *, zpl_phyport_t, zpl_bool , mirror_filter_t , mirror_dir_en , mac_t *);

}sdk_mirror_t;

extern sdk_mirror_t sdk_mirror;
extern int bsp_mirror_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_MIRROR_H__ */

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

typedef struct sdk_mirror_s
{
	int (*sdk_mirror_enable_cb) (void *, zpl_phyport_t , zpl_bool );
	int (*sdk_mirror_source_enable_cb) (void *, zpl_phyport_t, zpl_bool , mirror_mode_t , mirror_dir_en );
	int (*sdk_mirror_source_filter_enable_cb) (void *, zpl_bool enable, mirror_filter_t filter, mirror_dir_en type, mac_t *mac, mac_t *mac1);

}sdk_mirror_t;

extern sdk_mirror_t sdk_mirror;
extern int bsp_mirror_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_MIRROR_H__ */

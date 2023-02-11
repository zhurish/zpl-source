/*
 * zpl_media_hal.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_HAL_H__
#define __ZPL_MEDIA_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zpl_type.h>
#include <zpl_media.h>
#include <zpl_media_channel.h>


/*
*
*       hal_input -----------> hal_vpss -----------> hal_venc -----------> hal_hdmi
*                       |
*                       -----> hal_vpss -----------> hal_venc -----------> hal_hdmi
*                       |
*                       -----> hal_vpss -----------> hal_venc 
*                                             |                    
*                                             -----> hal_venc 
*
*
*
*/

int zpl_media_hal_create(zpl_media_channel_t *chn, void *buffer_queue);
int zpl_media_hal_destroy(zpl_media_channel_t *chn);
int zpl_media_hal_active(zpl_media_channel_t *chn);
int zpl_media_hal_start(zpl_media_channel_t *chn);
int zpl_media_hal_stop(zpl_media_channel_t *chn);
int zpl_media_hal_inactive(zpl_media_channel_t *chn);

int zpl_media_hal_encode_hwbind(zpl_media_channel_t *chn, zpl_bool bind);
int zpl_media_hal_vpss_hwbind(zpl_media_channel_t *chn, zpl_bool bind);

int zpl_media_hal_read_stop(ZPL_MEDIA_NODE_E, zpl_media_channel_t *chn);
int zpl_media_hal_read_start(ZPL_MEDIA_NODE_E, zpl_void *master, zpl_media_channel_t *chn);

int zpl_media_hal_request_IDR(zpl_media_channel_t *chn);

int zpl_media_hal_sys_init(void);


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_HAL_H__ */

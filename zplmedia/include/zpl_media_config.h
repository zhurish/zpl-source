/*
 * zpl_media_config.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_CONFIG_H__
#define __ZPL_MEDIA_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


int zpl_media_config_load(char *filename);
int zpl_media_config_write(char *filename);



#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_CONFIG_H__ */

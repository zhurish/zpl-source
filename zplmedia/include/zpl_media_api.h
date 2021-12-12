/*
 * zpl_media_api.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_API_H__
#define __ZPL_MEDIA_API_H__

#ifdef __cplusplus
extern "C" {
#endif


int zpl_media_module_init();
int zpl_media_module_exit();
int zpl_media_task_init();
int zpl_media_task_exit();
int zpl_media_cmd_init();

#ifdef ZPL_SHELL_MODULE
void cmd_video_init(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_API_H__ */

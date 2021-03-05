/*
 * pjsip_main.h
 *
 *  Created on: Feb 17, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_MAIN_H__
#define __PJSIP_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zebra.h"

//int pjmain(void *p);

int pjsip_module_init();
int pjsip_module_exit();

int pjsip_module_task_init();
int pjsip_module_task_exit();
int pjsip_media_wait_quit(void);

#ifdef __cplusplus
}
#endif

#endif /* __PJSIP_MAIN_H__ */

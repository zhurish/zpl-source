/*
 * nsm_main.h
 *
 *  Created on: Jan 9, 2018
 *      Author: zhurish
 */

#ifndef __NSM_MAIN_H__
#define __NSM_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int nsm_module_init (void);
extern int nsm_module_exit (void);
extern int nsm_task_init (void);
extern int nsm_task_exit (void);

extern int nsm_module_start(void);

extern int nsm_module_cmd_init (void);
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_MAIN_H__ */

/*
 * nsm_main.h
 *
 *  Created on: Jan 9, 2018
 *      Author: zhurish
 */

#ifndef __NSM_MAIN_H__
#define __NSM_MAIN_H__

extern int nsm_module_init ();
extern int nsm_task_init ();
extern int nsm_task_exit ();
extern int nsm_module_exit ();
extern int nsm_module_cmd_init ();

#endif /* __NSM_MAIN_H__ */

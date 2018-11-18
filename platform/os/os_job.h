/*
 * os_job.h
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */

#ifndef __OS_JOB_H__
#define __OS_JOB_H__

//#include "vty.h"

extern int os_job_init();
extern int os_job_exit();
extern int os_job_load();

extern int os_job_show(void *);

extern int os_job_add_entry(int (*job_entry)(void *), void *pVoid, char *func_name);

#define os_job_add(f,p)		os_job_add_entry(f,p,#f)


extern int os_job_del(int (*job_entry)(void *), void *pVoid);


#endif /* __OS_JOB_H__ */

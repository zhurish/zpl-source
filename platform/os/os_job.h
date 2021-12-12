/*
 * os_job.h
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */

#ifndef __OS_JOB_H__
#define __OS_JOB_H__


#ifdef __cplusplus
extern "C" {
#endif

extern int os_job_init();
extern int os_job_exit();
extern int os_job_load();

extern int os_job_show(void *);

extern int os_job_add_entry(int (*job_entry)(void *), void *pVoid, const zpl_char *func_name);

#define os_job_add(f,p)		os_job_add_entry(f,p,#f)


extern int os_job_del(int (*job_entry)(void *), void *pVoid);


#ifdef __cplusplus
}
#endif

#endif /* __OS_JOB_H__ */

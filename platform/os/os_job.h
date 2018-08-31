/*
 * os_job.h
 *
 *  Created on: Jun 2, 2017
 *      Author: zhurish
 */

#ifndef PLATFORM_OS_OS_JOB_H_
#define PLATFORM_OS_OS_JOB_H_


extern int os_job_init();
extern int os_job_exit();
extern int os_job_finsh();
extern int os_job_add(int (*job_entry)(void *), void *pVoid);


#endif /* PLATFORM_OS_OS_JOB_H_ */

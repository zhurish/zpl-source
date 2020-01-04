/*
 * v9_util.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_UTIL_H__
#define __V9_UTIL_H__



int v9_memory_load(u_int32 *total, u_int8 *use);
int v9_cpu_load(u_int16 *use);
int v9_disk_load(char *path, u_int32 *total, u_int32 *use, u_int8 *puse);


#endif /* __V9_UTIL_H__ */

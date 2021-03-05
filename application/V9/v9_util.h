/*
 * v9_util.h
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */

#ifndef __V9_UTIL_H__
#define __V9_UTIL_H__


#ifdef __cplusplus
extern "C" {
#endif


int v9_memory_load(ospl_uint32 *total, ospl_uint8 *use);
int v9_cpu_load(ospl_uint16 *use);
int v9_disk_load(char *path, ospl_uint32 *total, ospl_uint32 *use, ospl_uint8 *puse);


#ifdef __cplusplus
}
#endif

#endif /* __V9_UTIL_H__ */

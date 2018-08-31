/*
 * os_memory.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef PLATFORM_OS_OS_MEMORY_H_
#define PLATFORM_OS_OS_MEMORY_H_

#define os_malloc	malloc
#define os_calloc	calloc
#define os_free		free

#define os_memset	memset
#define os_memmove	memmove
#define os_memcmp	memcmp
#define os_memcpy	memcpy

#define os_strstr	strstr
#define os_strcpy	strcpy
#define os_strcmp	strcmp
#define os_strncpy	strncpy
#define os_strncmp	strncmp
#define os_strdup	strdup
#define os_strcat	strcat
#define os_strlen	strlen
#define os_strspn	strspn
#define os_strcspn	strcspn

#define os_strerror strerror

#define os_sprintf 		sprintf
#define os_fprintf 		fprintf
#define os_snprintf 	snprintf

#define os_bzero 		bzero
#define os_sscanf 		sscanf

#define os_atoi 		atoi
#define os_atof 		atof
#define os_atol 		atol
//#define os_printf 		snprintf

#endif /* PLATFORM_OS_OS_MEMORY_H_ */

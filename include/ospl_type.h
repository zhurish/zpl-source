/*
 * ospl_type.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __OSPL_TYPE_H__
#define __OSPL_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <endian.h>
#ifdef OS_LINUX
#include "pthread.h"
#include "semaphore.h"
#endif



typedef unsigned char ospl_uint8;
typedef unsigned short ospl_uint16;
typedef unsigned int ospl_uint32;

typedef signed char ospl_int8;
typedef signed short ospl_int16;
typedef signed int ospl_int32;


typedef unsigned char ospl_uchar;
typedef unsigned short ospl_ushort;
typedef unsigned int ospl_uint32;

typedef  char ospl_char;
typedef  short ospl_short;
typedef  int ospl_int;


typedef unsigned long ospl_ulong;
typedef signed long ospl_long;

typedef long long ospl_llong;
typedef unsigned long long ospl_ullong;

typedef float ospl_float;
typedef double ospl_double;

typedef size_t ospl_size_t;
typedef ssize_t ospl_ssize_t;


typedef ospl_int  ifindex_t;
typedef ospl_ushort ospl_proto_t;
typedef ospl_uchar ospl_family_t;
typedef ospl_int ospl_error_t;

typedef ospl_ushort vlan_t;
typedef ospl_ushort ospl_vlan_t;


typedef enum
{   
    ospl_delete = 0, 
    ospl_add  = 1, 
    ospl_update = 2,  
    ospl_clearall = 3,
    ospl_lookup = 4,
    ospl_install = 5,
    ospl_uninstall = 6,
}ospl_action;

#ifdef WIN32
#ifdef __int64
typedef __int64 ospl_int64;
#else /* __int64 */
typedef signed long long ospl_int64;
typedef unsigned long long ospl_uint64;
#endif /* __int64 */
#else /* WIN32 */

#if __WORDSIZE == 64
typedef long int                ospl_int64;
typedef unsigned long int       ospl_uint64;
#else
__extension__
typedef long long int           ospl_int64;
typedef unsigned long long int  ospl_uint64;
#endif

//typedef int64_t ospl_int64;
//typedef uint64_t ospl_uint64;
#endif /* WIN32 */



#ifndef bool
typedef enum
{
    ospl_false  = 0,    
    ospl_true = 1       
}ospl_bool;
#else
typedef bool ospl_bool;
#define ospl_false false
#define ospl_true true
#endif

/*
typedef enum {
  ospl_true  = 1,
  ospl_false = 0,
} ospl_bool;
*/
typedef time_t ospl_time_t;
typedef void ospl_void;

#ifdef WIN32
typedef HANDLE ospl_pthread_t;
typedef DWORD ospl_pid_t;
typedef sem_t ospl_sem_t;
typedef pthread_mutex_t ospl_pthread_mutex_t;
typedef pthread_cond_t ospl_pthread_cond_t;
typedef pthread_attr_t ospl_pthread_attr_t;
typedef pthread_spinlock_t ospl_pthread_spinlock_t;
typedef struct sched_param ospl_sched_param_t;
#else /* WIN32 */
typedef pthread_t ospl_pthread_t;
typedef pid_t ospl_pid_t;
typedef sem_t ospl_sem_t;
typedef pthread_mutex_t ospl_pthread_mutex_t;
typedef pthread_cond_t ospl_pthread_cond_t;
typedef pthread_attr_t ospl_pthread_attr_t;
typedef pthread_spinlock_t ospl_pthread_spinlock_t;
typedef struct sched_param ospl_sched_param_t;
#endif /* WIN32 */


typedef ospl_uint8 safi_t;

/* Zebra types. Used in Zserv message header. */
typedef ospl_uint16 zebra_size_t;
typedef ospl_uint16 zebra_command_t;

/* VRF ID type. */
typedef ospl_uint16 vrf_id_t;

typedef ospl_uint32  route_tag_t;





#ifdef __cplusplus
}
#endif

#endif /* __OSPL_TYPE_H__ */

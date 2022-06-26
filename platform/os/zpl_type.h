/*
 * zpl_type.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_TYPE_H__
#define __ZPL_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <endian.h>
#include <math.h>
#include <signal.h>
//#ifdef ZPL_BUILD_LINUX
#include <pthread.h>
#include <semaphore.h>
//#endif

#ifndef NSM_MAC_MAX
#define NSM_MAC_MAX	6
#endif


#ifndef VLAN_TABLE_MAX
#define VLAN_TABLE_MAX 4096
#endif


typedef unsigned char zpl_uint8;
typedef unsigned short zpl_uint16;
typedef unsigned int zpl_uint32;

typedef signed char zpl_int8;
typedef signed short zpl_int16;
typedef signed int zpl_int32;


typedef unsigned char zpl_uchar;
typedef unsigned short zpl_ushort;
typedef unsigned int zpl_uint;

typedef char zpl_char;
typedef short zpl_short;
typedef int zpl_int;


typedef unsigned long zpl_ulong;
typedef signed long zpl_long;

typedef long long zpl_llong;
typedef unsigned long long zpl_ullong;

typedef float zpl_float;
typedef double zpl_double;

typedef size_t zpl_size_t;
typedef ssize_t zpl_ssize_t;


typedef zpl_uint  ifindex_t;
typedef zpl_ushort zpl_proto_t;
typedef zpl_uchar zpl_family_t;
typedef zpl_int zpl_error_t;
typedef zpl_uchar mac_t;
typedef zpl_uchar zpl_mac_t[NSM_MAC_MAX];
typedef zpl_ushort vlan_t;
typedef zpl_ushort vlan_tpid_t;
typedef zpl_ushort zpl_vlan_t;
typedef zpl_int32 zpl_phyport_t;

typedef zpl_uint32 zpl_group_t;
typedef zpl_uint32 zpl_instace_t;
typedef zpl_uint32 zpl_index_t;

typedef zpl_uint16 vrf_id_t;
typedef zpl_uint32 zpl_vpn_t;

typedef zpl_uint32  route_tag_t;
typedef zpl_uint8   safi_t;

typedef zpl_uint32   zpl_ipaddr_t;
typedef zpl_uchar   zpl_ipv6addr_t[16];

typedef zpl_int32 zpl_taskid_t;

typedef enum
{   
    zpl_delete = 0, 
    zpl_add  = 1, 
    zpl_update = 2,  
    zpl_clearall = 3,
    zpl_lookup = 4,
    zpl_install = 5,
    zpl_uninstall = 6,
}zpl_action;

#ifdef WIN32
#ifdef __int64
typedef __int64 zpl_int64;
#else /* __int64 */
typedef signed long long zpl_int64;
typedef unsigned long long zpl_uint64;
#endif /* __int64 */
#else /* WIN32 */


#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ zpl_int64;
#else
typedef int64_t zpl_int64;
#endif

#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ zpl_uint64;
#else
typedef uint64_t zpl_uint64;
#endif


/*
#if __WORDSIZE == 64
#ifdef ZPL_HISIMPP_MODULE 
typedef long long           zpl_int64;
typedef unsigned long long  zpl_uint64;
#else
typedef long                 zpl_int64;
typedef unsigned long long   zpl_uint64;
#endif
#else
#ifdef ZPL_HISIMPP_MODULE 
typedef long long           zpl_int64;
typedef unsigned long long  zpl_uint64;
#else
typedef long long int           zpl_int64;
typedef unsigned long long int  zpl_uint64;
#endif
#endif
*/
#endif /* WIN32 */

#define double_eq(a,b) (fabs((a)-(b)) < 1e-8)
#define double_ne(a,b) (fabs((a)-(b)) > 1e-8)
#define double_gt(a,b) (((a)-(b)) > 1e-8)
#define double_lt(a,b) (((a)-(b)) < 1e-8)
/*
运算符	说明	举例
-eq	检测两个数是否相等，相等返回 true。	[ $a -eq $b ] 返回 false。
-ne	检测两个数是否不相等，不相等返回 true。	[ $a -ne $b ] 返回 true。
-gt	检测左边的数是否大于右边的，如果是，则返回 true。	[ $a -gt $b ] 返回 false。
-lt	检测左边的数是否小于右边的，如果是，则返回 true。	[ $a -lt $b ] 返回 true。
-ge	检测左边的数是否大于等于右边的，如果是，则返回 true。	[ $a -ge $b ] 返回 false。
-le	检测左边的数是否小于等于右边的，如果是，则返回 true。	[ $a -le $b ] 返回 true。
*/

#ifndef bool
typedef enum
{
    zpl_false  = 0,    
    zpl_true = 1       
}zpl_bool;
#else
typedef bool zpl_bool;
#define zpl_false false
#define zpl_true true
#endif




typedef enum {
  UNKNOEW_STACK = 0,
  OS_STACK	= 1,
  IPCOM_STACK	= 2,
} zpl_ipstack;



typedef struct
{
	int		_fd;
	zpl_ipstack stack;    
}zpl_socket_t;

typedef zpl_socket_t zpl_fd_t;

typedef time_t zpl_time_t;
typedef void zpl_void;

#define zpl_timeval timeval 


#ifdef WIN32
typedef HANDLE zpl_pthread_t;
typedef DWORD zpl_pid_t;
typedef sem_t zpl_sem_t;
typedef pthread_mutex_t zpl_pthread_mutex_t;
typedef pthread_cond_t zpl_pthread_cond_t;
typedef pthread_attr_t zpl_pthread_attr_t;
typedef pthread_spinlock_t zpl_pthread_spinlock_t;
typedef struct sched_param zpl_sched_param_t;
#else /* WIN32 */
typedef pthread_t zpl_pthread_t;
typedef pid_t zpl_pid_t;
typedef sem_t zpl_sem_t;
typedef pthread_mutex_t zpl_pthread_mutex_t;
typedef pthread_cond_t zpl_pthread_cond_t;
typedef pthread_attr_t zpl_pthread_attr_t;
typedef pthread_spinlock_t zpl_pthread_spinlock_t;
typedef struct sched_param zpl_sched_param_t;
#endif /* WIN32 */




/* Zebra types. Used in Zserv message header. */
typedef zpl_uint16 zebra_size_t;
typedef zpl_uint16 zebra_command_t;



//typedef int socklen_t;
#define ZPL_ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))

#define ZPL_BIT(n)		      (1)<<(n)
#define ZPL_TST_BIT(v, n)		(v) & ((1)<<(n))
#define ZPL_SET_BIT(v, n)		(v) |= (1)<<(n)
#define ZPL_CLR_BIT(v, n)		(v) &= ~((1)<<(n))


#define ZPL_SUB_MODULE_MAX  8
#define ZPL_SUB_MODULE_ID(m,n)  ((m)*(ZPL_SUB_MODULE_MAX) + (n))

#include "zpl_errno.h"
#include "zpl_def.h"


#ifdef __cplusplus
}
#endif

#endif /* __ZPL_TYPE_H__ */

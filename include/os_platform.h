/*
 * os_platform.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef OS_PLATFORM_H_
#define OS_PLATFORM_H_

#define OS_LINUX
//#define OS_VXWORKS
//#define OS_OPENBSD
//#define OS_OTHER

//#define OS_PROCESS
#define OS_THREAD


#include "pthread.h"
#include "os_memory.h"

#ifndef RT_TABLE_MAIN
//#define RT_TABLE_MAIN 0
#endif
//#define USE_IPSTACK_KERNEL
#ifndef USE_IPSTACK_KERNEL
#define IPNET
#endif

#define OS_STACK	1
#define IPCOM_STACK	2
//#define IF_IUSPV_SUPPORT //support sub interface, eg:0/1/1.22

//#define _OS_DEBUG_
//#define _OS_SHELL_DEBUG_
//#define _OS_DEBUG_
#define FD_IS_STDOUT(f)	((f) < 3)

#ifndef min
#define min(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef max
#define max(a,b)	( (a) > (b)? (a):(b) )
#endif
#ifndef MIN
#define MIN(a,b)	( (a) < (b)? (a):(b) )
#endif
#ifndef MAX
#define MAX(a,b)	( (a) > (b)? (a):(b) )
#endif

#ifndef INT_MAX_MIN_SPACE
#define INT_MAX_MIN_SPACE(v, m, M)	( ((v) > (m)) && ((v) < (M)) )
#endif

#ifndef NOT_INT_MAX_MIN_SPACE
#define NOT_INT_MAX_MIN_SPACE(v, m, M)	( ((v) < (m)) && ((v) > (M)) )
#endif

#if 0
typedef int 		(*FUNCPTR) (...);     /* ptr to function returning int */
typedef void 		(*VOIDFUNCPTR) (...); /* ptr to function returning void */
typedef double 		(*DBLFUNCPTR) (...);  /* ptr to function returning double*/
typedef float 		(*FLTFUNCPTR) (...);  /* ptr to function returning float */
#endif


#ifndef u_int8
typedef unsigned char u_int8;
#endif

#ifndef u_int16
typedef unsigned short u_int16;
#endif

#ifndef u_int32
typedef unsigned int u_int32;
#endif

#ifndef u_long
typedef unsigned long u_long;
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef u_short
typedef unsigned short u_short;
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef llong
typedef long long llong;
#endif

#ifndef u_llong
typedef unsigned long long u_llong;
#endif

enum {
  TRUE  = 1,
  FALSE = 0,
} ;

typedef int BOOL;

enum
{
  OK  = 0,
  TIMEOUT  = 1,
  CTRL_X  = 2,
  ERROR = -1,
};

#define OS_WAIT_NO	0
#define OS_WAIT_FOREVER	-1



#if defined( _OS_SHELL_DEBUG_)||defined( _OS_DEBUG_)
#define OS_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SHELL_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#define OS_SERVICE_DEBUG(format, ...) printf (format, ##__VA_ARGS__)
#else
#define OS_DEBUG(format, ...)
#define OS_SHELL_DEBUG(format, ...)
#define OS_SERVICE_DEBUG(format, ...)
#endif




#endif /* OS_PLATFORM_H_ */

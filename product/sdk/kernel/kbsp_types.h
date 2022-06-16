/*
 * kbsp_types.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __KBSP_TYPES_H__
#define __KBSP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/types.h>

#define NO_SDK ERROR

#define ZPL_ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))


#ifndef NSM_MAC_MAX
#define NSM_MAC_MAX	6
#endif


#ifndef VLAN_TABLE_MAX
#define VLAN_TABLE_MAX 4096
#endif


#define BSP_ENTER_FUNC() zlog_debug(MODULE_BSP, "Into %s line %d", __func__, __LINE__)
#define BSP_LEAVE_FUNC() zlog_debug(MODULE_BSP, "Leave %s line %d", __func__, __LINE__)



enum zpl_errno
{
	OK = 0,
	ERROR = -1,
	OS_TIMEOUT = -500001,	   //超时
	OS_CLOSE = -500002,		   //关闭
	OS_TRY_AGAIN = -500003,	   //再一次
	OS_CTRL_X = -500004,		   // CTRL 信号
	OS_EXIST = -500005,	   //已存在
	OS_NOTEXIST = -500006,	   //不存在
	OS_UNKNOWN_CMD = -500007, //
	OS_NO_SDKSPUUORT = -500008,
	OS_NO_CALLBACK = -500009,

	ZPL_OK = 0,
	ZPL_ERROR = -1,
	ZPL_OS_TIMEOUT = -500001,
	ZPL_OS_CLOSE = -500002,
	ZPL_OS_TRY_AGAIN = -500003,
	ZPL_OS_CTRL_X = -500004,
	ZPL_OS_EXIST = -500005,
	ZPL_OS_NOTEXIST = -500006,
	ZPL_UNKNOWN_CMD = -500007,
	ZPL_NO_SDKSPUUORT = -500008,
	ZPL_NO_CALLBACK = -500009,
	ZPL_ERRNO_UNKNOW
};

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

typedef int zpl_size_t;
typedef int zpl_ssize_t;


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




typedef enum
{
    zpl_false  = 0,    
    zpl_true = 1       
}zpl_bool;




typedef void zpl_void;

#define ZPL_BIT(n)		      (1)<<(n)
#define ZPL_TST_BIT(v, n)		(v) & ((1)<<(n))
#define ZPL_SET_BIT(v, n)		(v) |= (1)<<(n)
#define ZPL_CLR_BIT(v, n)		(v) &= ~((1)<<(n))


#include <linux/in.h>
#include <linux/in6.h>



union g_addr {
  struct in_addr ipv4;
#ifdef ZPL_BUILD_IPV6
//#error "ssssssssssssssss"
  struct in6_addr ipv6;
#endif /* ZPL_BUILD_IPV6 */
};


#ifdef __cplusplus
}
#endif

#endif /* __KBSP_TYPES_H__ */

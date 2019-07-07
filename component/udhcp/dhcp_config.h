/*
 * dhcp_config.h
 *
 *  Created on: Apr 22, 2019
 *      Author: zhurish
 */

#ifndef __UDHCP_CONFIG_H__
#define __UDHCP_CONFIG_H__

//#include "libbb.h"
#include "zebra.h"
#include "prefix.h"
#include "sockopt.h"
#include "memory.h"
#include "log.h"


#include "sys/poll.h"
#include <netinet/udp.h>
#include <netinet/ip.h>


#include <limits.h>
#if defined(__digital__) && defined(__unix__)
# include <sex.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
   || defined(__APPLE__)
# include <sys/resource.h>  /* rlimit */
# include <machine/endian.h>
# define bswap_64 __bswap64
# define bswap_32 __bswap32
# define bswap_16 __bswap16
#else
# include <byteswap.h>
# include <endian.h>
#endif


#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN
# define BB_BIG_ENDIAN 1
# define BB_LITTLE_ENDIAN 0
#elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#elif defined(__386__)
# define BB_BIG_ENDIAN 0
# define BB_LITTLE_ENDIAN 1
#else
# error "Can't determine endianness"
#endif

#if ULONG_MAX > 0xffffffff
# define bb_bswap_64(x) bswap_64(x)
#endif

#if BB_BIG_ENDIAN
# define SWAP_BE16(x) (x)
# define SWAP_BE32(x) (x)
# define SWAP_BE64(x) (x)
# define SWAP_LE16(x) bswap_16(x)
# define SWAP_LE32(x) bswap_32(x)
# define SWAP_LE64(x) bb_bswap_64(x)
# define IF_BIG_ENDIAN(...) __VA_ARGS__
# define IF_LITTLE_ENDIAN(...)
#else
# define SWAP_BE16(x) bswap_16(x)
# define SWAP_BE32(x) bswap_32(x)
# define SWAP_BE64(x) bb_bswap_64(x)
# define SWAP_LE16(x) (x)
# define SWAP_LE32(x) (x)
# define SWAP_LE64(x) (x)
# define IF_BIG_ENDIAN(...)
# define IF_LITTLE_ENDIAN(...) __VA_ARGS__
#endif

#define RETURNS_MALLOC __attribute__ ((malloc))
#define PACKED __attribute__ ((__packed__))
#define ALIGNED(m) __attribute__ ((__aligned__(m)))

#define UNUSED_PARAM __attribute__ ((__unused__))
#define NORETURN __attribute__ ((__noreturn__))
#define INIT_FUNC __attribute__ ((constructor))
#define FAST_FUNC
#define FIX_ALIASING


#if defined(i386) || defined(__x86_64__) || defined(__powerpc__)
# define BB_UNALIGNED_MEMACCESS_OK 1
# define move_from_unaligned_int(v, intp)  ((v) = *(int*)(intp))
# define move_from_unaligned_long(v, longp) ((v) = *(long*)(longp))
# define move_from_unaligned16(v, u16p) ((v) = *(uint16_t*)(u16p))
# define move_from_unaligned32(v, u32p) ((v) = *(uint32_t*)(u32p))
# define move_to_unaligned16(u16p, v)   (*(uint16_t*)(u16p) = (v))
# define move_to_unaligned32(u32p, v)   (*(uint32_t*)(u32p) = (v))
/* #elif ... - add your favorite arch today! */
#else
# define BB_UNALIGNED_MEMACCESS_OK 0
/* performs reasonably well (gcc usually inlines memcpy here) */
# define move_from_unaligned_int(v, intp) (memcpy(&(v), (intp), sizeof(int)))
# define move_from_unaligned_long(v, longp) (memcpy(&(v), (longp), sizeof(long)))
# define move_from_unaligned16(v, u16p) (memcpy(&(v), (u16p), 2))
# define move_from_unaligned32(v, u32p) (memcpy(&(v), (u32p), 4))
# define move_to_unaligned16(u16p, v) do { \
	uint16_t __t = (v); \
	memcpy((u16p), &__t, 2); \
} while (0)
# define move_to_unaligned32(u32p, v) do { \
	uint32_t __t = (v); \
	memcpy((u32p), &__t, 4); \
} while (0)
#endif

# define move_get_unaligned32(v, u32p) do { \
	uint32_t *p = (uint32_t *)(v); \
	u32p = *p; \
} while (0)
/* Useful for defeating gcc's alignment of "char message[]"-like data */
#if !defined(__s390__)
    /* on s390[x], non-word-aligned data accesses require larger code */
# define ALIGN1 __attribute__((aligned(1)))
# define ALIGN2 __attribute__((aligned(2)))
# define ALIGN4 __attribute__((aligned(4)))
#else
/* Arches which MUST have 2 or 4 byte alignment for everything are here */
# define ALIGN1
# define ALIGN2
# define ALIGN4
#endif

#ifdef HAVE_PRINTF_PERCENTM
# define STRERROR_FMT    "%m"
# define STRERROR_ERRNO  /*nothing*/
#else
# define STRERROR_FMT    "%s"
# define STRERROR_ERRNO  ,strerror(errno)
#endif

#ifndef INT_MAX
# define INT_MAX	2147483647
#endif

# define BB_VER "0.0.0.1"


#define ENABLE_UDHCPC	1
#define ENABLE_UDHCPD	1

#define ENABLE_FEATURE_UDHCP_RFC3397 1
#define ENABLE_FEATURE_UDHCP_8021Q 1
#define ENABLE_FEATURE_UDHCPC_ARPING 1

#define ENABLE_FEATURE_UDHCPC6_RFC3646 1
#define ENABLE_FEATURE_UDHCPC6_RFC4704 1
#define ENABLE_FEATURE_UDHCPC6_RFC4833 1

#define ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY 0
#define ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC 0

#define CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS 0
//#define CONFIG_UDHCPC_DEFAULT_SCRIPT	"dhcp.script"
#define CONFIG_UDHCPC_DEFAULT_SCRIPT 	"/usr/share/udhcpc/dhcp.script"
//#define CONFIG_UDHCPC_DEFAULT_SCRIPT 	"/usr/share/udhcpc/default.script"



#define UDHCPD_POOL_MAX	64
#define UDHCPD_HOSTNAME_MAX	64


#define	spawn_and_wait(n)



#endif /* __UDHCP_CONFIG_H__ */

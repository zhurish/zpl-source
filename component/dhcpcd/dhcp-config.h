/* linux */
#ifndef __DHCPCD_CONFIG_H__
#define __DHCPCD_CONFIG_H__

#include "zebra.h"
#include "md5.h"

#define DHCPC_THREAD

#define	NDEBUG
#define	THERE_IS_NO_FORK
//#define	_GNU_SOURCE
/*
#define	_FILE_OFFSET_BITS 64
#define	_LARGEFILE_SOURCE
#define	_LARGEFILE64_SOURCE
*/
#define	INET
#define	ARP
#define	ARPING
#define	IPV4LL
#define	INET6
#define	DHCP6
#define	AUTH
#define	NO_SIGNALS
#define	__linux__ 1

#ifdef BASE_DIR
#define	SBINDIR			SYSSBINDIR
#define	LIBDIR			SYSLIBDIR
#define	LIBEXECDIR		BASE_DIR"/libexec"
#define	DBDIR			SYSRUNDIR"/db/dhcpcd"
#define	RUNDIR			SYSRUNDIR
#else
#define	SYSCONFDIR		"/etc"
#define	SBINDIR			"/sbin"
#define	LIBDIR			"/lib"
#define	LIBEXECDIR		"/libexec"
#define	DBDIR			"/var/db/dhcpcd"
#define	RUNDIR			"/var/run"
#endif

#include		<sys/socket.h> /* fix broken headers */
#include		<linux/rtnetlink.h>
#define	HAVE_NL80211_H
#define	HAVE_IN6_ADDR_GEN_MODE_NONE
/*
#include			"compat/arc4random.h"
#include			"compat/arc4random_uniform.h"
#include			"compat/strlcpy.h"
#include			"compat/pidfile.h"
#include			"compat/strtoi.h"
#include		<sys/queue.h>
#include			"compat/queue.h"
#include			"compat/reallocarray.h"
*/
//#define	HAVE_REALLOCARRAY
#define	HAVE_EPOLL
#include		<sys/queue.h>
#include		"queue.h"
#include		"endian.h"

#include		"sha256.h"
#include		"hmac.h"

/*
#include			"compat/endian.h"
#include			"compat/crypt/md5.h"
#include			"compat/crypt/sha256.h"
#include			"compat/crypt/hmac.h"
*/
#endif

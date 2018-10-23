/* linux */
#ifndef __DHCPC_CONFIG_H__
#define __DHCPC_CONFIG_H__

#define DHCPC_ONLY_THREAD


#define THERE_IS_NO_FORK

//#define DHCPC_SCRIPT_EXECU

#ifdef DHCPC_ONLY_THREAD
/*
#define SYSCONFDIR	"/var/run/dhcpcd"
#define SBINDIR		"/var/run/dhcpcd"
#define LIBEXECDIR	"/var/run/dhcpcd"
#define DBDIR		"/var/run/dhcpcd"
#define RUNDIR		"/var/run/dhcpcd"
*/
#else
#define SYSCONFDIR	"/system/etc/dhcpcd"
#define SBINDIR		"/system/etc/dhcpcd"
#define LIBEXECDIR	"/system/etc/dhcpcd"
#define DBDIR		"/data/misc/dhcp"
#define RUNDIR		"/data/misc/dhcp"
#endif


#include "arc4random.h"
//#include "dhcpc_util.h"
//#include "strlcpy.h"
//#include "getline.h"

#define OS_ELOOP_THREAD

#ifndef MAX
#define MAX(a,b)	((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) <= (b) ? (a) : (b))
#endif

#include "os_ansync.h"
#include "nsm_client.h"

#include "dhcpc_api.h"

#endif

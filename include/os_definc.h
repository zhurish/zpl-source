/*
 * os_definc.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __OS_DEFINC_H__
#define __OS_DEFINC_H__
#ifdef __cplusplus
extern "C" {
#endif



#ifdef HAVE_PLCONFIG_H
#include "plconfig.h"
#endif /* HAVE_PLCONFIG_H */




#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif /* HAVE_SOCKLEN_T */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <syslog.h>

#include <pthread.h>
#include <semaphore.h>

#include <features.h>


#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif /* HAVE_STROPTS_H */

#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
# include <sys/time.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <sys/resource.h>


#ifdef HAVE_LCAPS
#include <sys/capability.h>
#include <sys/prctl.h>
#endif /* HAVE_LCAPS */

#include <sys/socket.h>
//#include <sys/sockio.h>

#include <netdb.h>

//#include <net/netopt.h>
#include <net/if.h>
//#include <net/if_dl.h>
//#include <net/if_var.h>
#include <net/route.h>

#include <linux/types.h>
#include <linux/version.h>
#ifdef HAVE_NETLINK
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include <linux/if_packet.h>
#else
#define RT_TABLE_MAIN		0
#endif /* HAVE_NETLINK */

#include <arpa/inet.h>

#include <asm/types.h>

#ifdef HAVE_SOLARIS_CAPABILITIES
#include <priv.h>
#endif /* HAVE_SOLARIS_CAPABILITIES */

//#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#ifdef PL_BUILD_IPV6
#include <netinet/icmp6.h>
#endif
//#include <netinet/in6_var.h>
//#include <netinet6/in6_var.h>
//#include <netinet6/in.h>
//#include <netinet6/ip6.h>
//#include <netinet6/nd6.h>
//#endif /* HAVE_NETINET6_ND6_H */

#ifdef HAVE_GLIBC_BACKTRACE
#include <execinfo.h>
#endif /* HAVE_GLIBC_BACKTRACE */



/* misc include group */
#if !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
/* Not C99; do we need to define va_copy? */
#ifndef va_copy
#ifdef __va_copy
#define va_copy(DST,SRC) __va_copy(DST,SRC)
#else
/* Now we are desperate; this should work on many typical platforms. 
   But this is slightly dangerous, because the standard does not require
   va_copy to be a macro. */
#define va_copy(DST,SRC) memcpy(&(DST), &(SRC), sizeof(va_list))
#warning "Not C99 and no va_copy macro available, falling back to memcpy"
#endif /* __va_copy */
#endif /* !va_copy */
#endif /* !C99 */



#ifdef USE_IPSTACK_KERNEL
#include "ip_os_sock.h"
#endif

#ifdef PL_IPCOM_STACK_MODULE
#include "ipnet_config.h"
#include "ipcom_sock.h"
#include "ipnet_eth.h"
#include "ip_os_sock.h"
#include "ipnet_routesock.h"
#include "ipcom_hook.h"
#include  "ipcom_netif.h"
#define RT_TABLE_MAIN 0
#endif /* USE_IPSTACK_KERNEL */


#ifdef __cplusplus
}
#endif

#endif /* __OS_DEFINC_H__ */

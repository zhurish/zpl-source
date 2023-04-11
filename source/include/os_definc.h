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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#include <inttypes.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <time.h>
#include <stdarg.h>
#include <endian.h>
#include <netdb.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/igmp.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#ifdef ZPL_BUILD_IPV6
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#endif

#include <net/if.h>
#include <net/route.h>
#include <net/if_arp.h>
//#include <net/if_packet.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>



#include <linux/sysinfo.h>
//#include <linux/if.h>
#include <linux/igmp.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/if_packet.h>


#include <arpa/inet.h>



//#ifdef HAVE_NETINET_IN_VAR_H

//#include <netinet/in6_var.h>
//#include <netinet6/in6_var.h>
//#include <netinet6/in.h>
//#include <netinet6/ip6.h>
//#include <netinet6/nd6.h>
//#endif /* HAVE_NETINET6_ND6_H */

#ifdef HAVE_GLIBC_BACKTRACE
#include <execinfo.h>
#endif /* HAVE_GLIBC_BACKTRACE */







#ifdef ZPL_IPCOM_MODULE
#include "ipnet_config.h"
#include "ipcom_sock.h"
#include "ipnet_eth.h"
#include "ip_os_sock.h"
#include "ipnet_routesock.h"
#include "ipcom_hook.h"
#include  "ipcom_netif.h"
#define RT_TABLE_MAIN 0
#include "ipcom_stack.h"
#else
#include "os_stack.h"
#endif /* ZPL_KERNEL_MODULE */


#ifdef __cplusplus
}
#endif

#endif /* __OS_DEFINC_H__ */

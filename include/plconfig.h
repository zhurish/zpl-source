/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

//#define USE_IPSTACK_KERNEL
	//USE_IPSTACK_KERNEL
#undef BASE_DIR
#ifndef BASE_DIR
#define BASE_DIR	"/tmp/app"
#endif

#ifdef BUILD_X86
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/zhurish/workspace/SWPlatform/debug"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#else
#ifdef BUILD_OPENWRT
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/app"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#else
#ifndef SYS_REAL_DIR
#define SYS_REAL_DIR		"/home/app"
#endif
#define SYSCONF_REAL_DIR 	SYS_REAL_DIR "/etc"
#endif
#endif

//#ifdef BUILD_OPENWRT
//#define SYSCONFDIR 		"/etc/app"
//#else
//#define SYSCONFDIR 		BASE_DIR "/etc"
#define SYSCONFDIR 		SYSCONF_REAL_DIR	//flash的etc 目录
#define RSYSLOGDIR 		SYS_REAL_DIR "/log"	//flash的log 目录
#define RSYSWWWDIR 		SYS_REAL_DIR "/www"	//flash的www 目录
#define SYSWEBDIR 		SYSCONF_REAL_DIR "/web"	//flash的/etc/web 目录
//#endif

#define PLSYSCONFDIR 	BASE_DIR "/etc"	//缓存目录
#define SYSLIBDIR 		BASE_DIR "/lib"
#define SYSSBINDIR 		BASE_DIR "/sbin"
#define SYSBINDIR 		BASE_DIR "/bin"
#define SYSRUNDIR 		BASE_DIR "/run"
#define SYSLOGDIR 		BASE_DIR "/log"
#define SYSVARDIR 		BASE_DIR "/var"
#define SYSTMPDIR 		BASE_DIR "/tmp"
#define SYSWWWDIR 		BASE_DIR "/www"
#define SYSWWWCACHEDIR 		BASE_DIR "/www/cache"
#define SYSTFTPBOOTDIR 		BASE_DIR "/tftpboot"
#define SYSUPLOADDIR 		SYSTFTPBOOTDIR


/* default daemon app root dir */
#define DAEMON_CONFIG_DIR SYSCONFDIR

/* default daemon logmsg directory */
#define DAEMON_LOG_FILE_DIR SYSLOGDIR

/* daemon vty directory */
#define DAEMON_VTY_DIR SYSVARDIR

/* daemon vty directory */
#define DAEMON_ENV_DIR SYSRUNDIR


#define CONF_BACKUP_EXT ".sav"

#ifdef BUILD_OPENWRT
#define STARTUP_CONFIG_FILE	SYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	SYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	SYSCONFDIR "/factory-config.cfg"
#else
#ifdef BUILD_X86
#define STARTUP_CONFIG_FILE	PLSYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	PLSYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	PLSYSCONFDIR "/factory-config.cfg"
#else
#define STARTUP_CONFIG_FILE	SYSCONFDIR "/startup-config.cfg"
#define DEFAULT_CONFIG_FILE	SYSCONFDIR "/default-config.cfg"
#define FACTORY_CONFIG_FILE	SYSCONFDIR "/factory-config.cfg"
#endif
#endif

#include "config_env.h"


/* Mask for config files */
#define CONFIGFILE_MASK 0664

/* Consumed Time Check */
#define CONSUMED_TIME_CHECK 5000000

/* Build for development */
/* #undef DEV_BUILD */

/* Disable BGP installation to zebra */
#define DISABLE_BGP_ANNOUNCE 0

#define HAVE_GET_NPROCS /* get_nprocs */
#ifndef __UCLIBC__
#define HAVE_PTHREAD_SETNAME_NP /* pthread_setname_np */
#endif
/* include git version info */
/* #undef GIT_VERSION */

/* GNU Linux */
#define GNU_LINUX /**/

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <asm/types.h> header file. */
#define HAVE_ASM_TYPES_H 1

/* Broken CMSG_FIRSTHDR */
/* #undef HAVE_BROKEN_CMSG_FIRSTHDR */

/* BSD ifi_link_state available */
/* #undef HAVE_BSD_IFI_LINK_STATE */

/* BSD link-detect */
/* #undef HAVE_BSD_LINK_DETECT */

/* Can pass ifindex in struct ip_mreq */
/* #undef HAVE_BSD_STRUCT_IP_MREQ_HACK */

/* capabilities */
#ifdef BUILD_X86
#define HAVE_CAPABILITIES 1
#endif
/* Define to 1 if your system has a working `chown' function. */
#define HAVE_CHOWN 1

/* Have monotonic clock */
#define HAVE_CLOCK_MONOTONIC /**/

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define to 1 if you have the declaration of `TCP_MD5SIG', and to 0 if you
   don't. */
#define HAVE_DECL_TCP_MD5SIG 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the `dup2' function. */
#define HAVE_DUP2 1

/* Expand routing protocol Platform :enable other route protocol */
#define HAVE_EXPAND_ROUTE_PLATFORM 1

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <features.h> header file. */
#define HAVE_FEATURES_H 1

/* Define to 1 if your system has a working POSIX `fnmatch' function. */
#define HAVE_FNMATCH 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Forwarding Plane Manager support */
/* #undef HAVE_FPM */

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the `getaddrinfo' function. */
//#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `getgrouplist' function. */
//#define HAVE_GETGROUPLIST 1

/* Define to 1 if you have the `gethostbyname' function. */
#define HAVE_GETHOSTBYNAME 1

/* Define to 1 if you have the `getifaddrs' function. */
//#define HAVE_GETIFADDRS 1

/* Define to 1 if you have the `getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Glibc backtrace */
#ifdef __UCLIBC__
#define HAVE_ULIBC_BACKTRACE /**/
#else
#define HAVE_GLIBC_BACKTRACE /**/
#endif
/* GNU regexp library */
#define HAVE_GNU_REGEX /**/

/* Define to 1 if you have the `if_indextoname' function. */
//#define HAVE_IF_INDEXTONAME 1

/* Define to 1 if you have the `if_nametoindex' function. */
//#define HAVE_IF_NAMETOINDEX 1

/* Define to 1 if you have the `inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define to 1 if you have the <inet/nd.h> header file. */
/* #undef HAVE_INET_ND_H */

/* Define to 1 if you have the `inet_ntoa' function. */
#define HAVE_INET_NTOA 1

/* inet_ntop */
#define HAVE_INET_NTOP /**/

/* inet_pton */
#define HAVE_INET_PTON /**/

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* IPv6 */
#ifdef BUILD_IPV6
#define HAVE_IPV6 1
#endif
/* Have IP_PKTINFO */
#define HAVE_IP_PKTINFO 1

/* Have IP_RECVDSTADDR */
/* #undef HAVE_IP_RECVDSTADDR */

/* Have IP_RECVIF */
/* #undef HAVE_IP_RECVIF */

/* IRDP */
/* #undef HAVE_IRDP */

/* Define to 1 if you have the <kvm.h> header file. */
/* #undef HAVE_KVM_H */

/* Capabilities */
#ifdef BUILD_X86
#define HAVE_LCAPS 1
#endif
/* Define to 1 if you have the `crypt' library (-lcrypt). */
#define HAVE_LIBCRYPT 1

/* Have libm */
#define HAVE_LIBM /**/

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `pcreposix' library (-lpcreposix). */
/* #undef HAVE_LIBPCREPOSIX */

/* Define to 1 if you have the `resolv' library (-lresolv). */
/* #undef HAVE_LIBRESOLV */
#define HAVE_LIBRESOLV 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `umem' library (-lumem). */
/* #undef HAVE_LIBUMEM */

/* Define to 1 if you have the `xnet' library (-lxnet). */
/* #undef HAVE_LIBXNET */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <linux/mroute.h> header file. */
#define HAVE_LINUX_MROUTE_H 1

/* enable linux system shell */
#define HAVE_LINUX_SHELL 1

/* Define to 1 if you have the <linux/version.h> header file. */
#define HAVE_LINUX_VERSION_H 1

/* mallinfo */
//#define HAVE_MALLINFO /**/

/* Define to 1 if you have the `memchr' function. */
#define HAVE_MEMCHR 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1
#ifdef BUILD_IPV6
/* Define to 1 if you have the <netinet6/in6.h> header file. */
/* #undef HAVE_NETINET6_IN6_H */

/* Define to 1 if you have the <netinet6/in6_var.h> header file. */
/* #undef HAVE_NETINET6_IN6_VAR_H */

/* Define to 1 if you have the <netinet6/nd6.h> header file. */
/* #undef HAVE_NETINET6_ND6_H */

/* Define to 1 if you have the <netinet/icmp6.h> header file. */
#define HAVE_NETINET_ICMP6_H 1

/* Define to 1 if you have the <netinet/in6_var.h> header file. */
/* #undef HAVE_NETINET_IN6_VAR_H */
#endif
/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/in_systm.h> header file. */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define to 1 if you have the <netinet/in_var.h> header file. */
/* #undef HAVE_NETINET_IN_VAR_H */

/* Define to 1 if you have the <netinet/ip_icmp.h> header file. */
#define HAVE_NETINET_IP_ICMP_H 1

/* netlink */
#define HAVE_NETLINK /**/

#ifndef __UCLIBC__
/* Have netns */
#define HAVE_NETNS /**/
#endif
/* Define to 1 if you have the <net/if_dl.h> header file. */
/* #undef HAVE_NET_IF_DL_H */

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the <net/if_var.h> header file. */
/* #undef HAVE_NET_IF_VAR_H */

/* Define to 1 if you have the <net/netopt.h> header file. */
/* #undef HAVE_NET_NETOPT_H */

/* Define to 1 if you have the <net/route.h> header file. */
#define HAVE_NET_ROUTE_H 1

/* NET_RT_IFLIST */
/* #undef HAVE_NET_RT_IFLIST */

/* OSPF Opaque LSA */
/* #undef HAVE_OPAQUE_LSA */

/* Have openpam.h */
/* #undef HAVE_OPENPAM_H */

/* enable ospf explain lsa */
#define HAVE_OSPFD_DCN 1

/* OSPF TE */
/* #undef HAVE_OSPF_TE */

/* Have pam_misc.h */
/* #undef HAVE_PAM_MISC_H */

/* Define to 1 if you have the `pow' function. */
#define HAVE_POW 1

/* Solaris printstack */
/* #undef HAVE_PRINTSTACK */

/* Define to 1 if you have the <priv.h> header file. */
/* #undef HAVE_PRIV_H */

/* protobuf */
/* #undef HAVE_PROTOBUF */

/* prctl */
#define HAVE_PR_SET_KEEPCAPS /**/

/* Have RFC3678 protocol-independed API */
#define HAVE_RFC3678 1

/* Optimize routing protocol Platform */
#define HAVE_ROUTE_OPTIMIZE 1

/* Enable IPv6 Routing Advertisement support */
/* #undef HAVE_RTADV */

/* rusage */
#define HAVE_RUSAGE /**/

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

#ifndef __UCLIBC__
/* Define to 1 if you have the `setns' function. */
#define HAVE_SETNS /**/
#endif
/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* SNMP */
/* #undef HAVE_SNMP */

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if the system has the type `socklen_t'. */
#define HAVE_SOCKLEN_T 1

/* getpflags */
/* #undef HAVE_SOLARIS_CAPABILITIES */
#ifdef __UCLIBC__
/* Stack symbol decoding */
//#define HAVE_STACK_TRACE /**/
#else
/* Stack symbol decoding */
#define HAVE_STACK_TRACE /**/
#endif
/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strcspn' function. */
#define HAVE_STRCSPN 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strspn' function. */
#define HAVE_STRSPN 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if the system has the type `struct icmphdr'. */
#define HAVE_STRUCT_ICMPHDR 1

/* Define to 1 if the system has the type `struct if6_aliasreq'. */
/* #undef HAVE_STRUCT_IF6_ALIASREQ */

/* Define to 1 if `ifra_lifetime' is a member of `struct if6_aliasreq'. */
/* #undef HAVE_STRUCT_IF6_ALIASREQ_IFRA_LIFETIME */

/* Define to 1 if the system has the type `struct ifaliasreq'. */
/* #undef HAVE_STRUCT_IFALIASREQ */

/* Define to 1 if `ifm_status' is a member of `struct ifmediareq'. */
/* #undef HAVE_STRUCT_IFMEDIAREQ_IFM_STATUS */

/* Define to 1 if `ifi_link_state' is a member of `struct if_data'. */
/* #undef HAVE_STRUCT_IF_DATA_IFI_LINK_STATE */

/* Define to 1 if the system has the type `struct igmpmsg'. */
/* #undef HAVE_STRUCT_IGMPMSG */

/* Define to 1 if the system has the type `struct in6_aliasreq'. */
/* #undef HAVE_STRUCT_IN6_ALIASREQ */

/* Define to 1 if the system has the type `struct in_pktinfo'. */
#define HAVE_STRUCT_IN_PKTINFO 1

/* Define to 1 if `imr_ifindex' is a member of `struct ip_mreqn'. */
#define HAVE_STRUCT_IP_MREQN_IMR_IFINDEX 1

/* Define to 1 if the system has the type `struct mfcctl'. */
/* #undef HAVE_STRUCT_MFCCTL */

/* Define to 1 if the system has the type `struct nd_opt_adv_interval'. */
#define HAVE_STRUCT_ND_OPT_ADV_INTERVAL 1

/* Define to 1 if `nd_opt_ai_type' is a member of `struct
   nd_opt_adv_interval'. */
/* #undef HAVE_STRUCT_ND_OPT_ADV_INTERVAL_ND_OPT_AI_TYPE */

/* Define to 1 if the system has the type `struct nd_opt_homeagent_info'. */
/* #undef HAVE_STRUCT_ND_OPT_HOMEAGENT_INFO */

/* Define to 1 if the system has the type `struct rt_addrinfo'. */
/* #undef HAVE_STRUCT_RT_ADDRINFO */

/* Define to 1 if the system has the type `struct sioc_sg_req'. */
/* #undef HAVE_STRUCT_SIOC_SG_REQ */

/* Define to 1 if the system has the type `struct sioc_vif_req'. */
/* #undef HAVE_STRUCT_SIOC_VIF_REQ */

/* Define to 1 if the system has the type `struct sockaddr'. */
#define HAVE_STRUCT_SOCKADDR 1

/* Define to 1 if the system has the type `struct sockaddr_dl'. */
/* #undef HAVE_STRUCT_SOCKADDR_DL */

/* Define to 1 if `sdl_len' is a member of `struct sockaddr_dl'. */
/* #undef HAVE_STRUCT_SOCKADDR_DL_SDL_LEN */

/* Define to 1 if the system has the type `struct sockaddr_in'. */
#define HAVE_STRUCT_SOCKADDR_IN 1

/* Define to 1 if the system has the type `struct sockaddr_in6'. */
#define HAVE_STRUCT_SOCKADDR_IN6 1

/* Define to 1 if `sin6_scope_id' is a member of `struct sockaddr_in6'. */
#define HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID 1

/* Define to 1 if `sin_len' is a member of `struct sockaddr_in'. */
/* #undef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */

/* Define to 1 if `sa_len' is a member of `struct sockaddr'. */
/* #undef HAVE_STRUCT_SOCKADDR_SA_LEN */

/* Define to 1 if the system has the type `struct sockaddr_un'. */
#define HAVE_STRUCT_SOCKADDR_UN 1

/* Define to 1 if `sun_len' is a member of `struct sockaddr_un'. */
/* #undef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

/* Define to 1 if the system has the type `struct vifctl'. */
/* #undef HAVE_STRUCT_VIFCTL */

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Enable System Utils Support */
#define HAVE_SYSTEM_UTILS 1

/* Define to 1 if you have the <sys/capability.h> header file. */
#define HAVE_SYS_CAPABILITY_H 1

/* Define to 1 if you have the <sys/cdefs.h> header file. */
#define HAVE_SYS_CDEFS_H 1

/* Define to 1 if you have the <sys/conf.h> header file. */
/* #undef HAVE_SYS_CONF_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ksym.h> header file. */
/* #undef HAVE_SYS_KSYM_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#define HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define this if your system can create weak aliases */
#define HAVE_SYS_WEAK_ALIAS 1

/* Define this if weak aliases may be created with __attribute__ */
/* #undef HAVE_SYS_WEAK_ALIAS_ATTRIBUTE */

/* Define this if weak aliases may be created with #pragma _CRI duplicate */
/* #undef HAVE_SYS_WEAK_ALIAS_CRIDUPLICATE */

/* Define this if weak aliases in other files are honored */
#define HAVE_SYS_WEAK_ALIAS_CROSSFILE 1

/* Define this if weak aliases may be created with #pragma _HP_SECONDARY_DEF
   */
/* #undef HAVE_SYS_WEAK_ALIAS_HPSECONDARY */

/* Define this if weak aliases may be created with #pragma weak */
#define HAVE_SYS_WEAK_ALIAS_PRAGMA 1

/* Old Linux 2.4 TCP MD5 Signature Patch */
/* #undef HAVE_TCP_MD5_LINUX24 */

/* Use TCP for zebra communication */
//#define HAVE_TCP_ZEBRA /**/

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <ucontext.h> header file. */
#define HAVE_UCONTEXT_H 1

/* Define to 1 if `uc_mcontext.gregs' is a member of `ucontext_t'. */
#define HAVE_UCONTEXT_T_UC_MCONTEXT_GREGS 1

/* Define to 1 if `uc_mcontext.regs' is a member of `ucontext_t'. */
/* #undef HAVE_UCONTEXT_T_UC_MCONTEXT_REGS */

/* Define to 1 if `uc_mcontext.regs.nip' is a member of `ucontext_t'. */
/* #undef HAVE_UCONTEXT_T_UC_MCONTEXT_REGS_NIP */

/* Define to 1 if `uc_mcontext.uc_regs' is a member of `ucontext_t'. */
/* #undef HAVE_UCONTEXT_T_UC_MCONTEXT_UC_REGS */

/* Define to 1 if you have the `uname' function. */
#define HAVE_UNAME 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `vfork' function. */
#define HAVE_VFORK 1

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 if the system has the type `vifi_t'. */
/* #undef HAVE_VIFI_T */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have the <wctype.h> header file. */
#define HAVE_WCTYPE_H 1

/* Define to 1 if `fork' works. */
#define HAVE_WORKING_FORK 1

/* Define to 1 if `vfork' works. */
#define HAVE_WORKING_VFORK 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Enable IMI module shell to Routting Protocol */
#define IMISH_IMI_MODULE 65530

/* selected method for isis, == one of the constants */
#define ISIS_METHOD ISIS_METHOD_PFPACKET

/* constant value for isis method bpf */
#define ISIS_METHOD_BPF 3

/* constant value for isis method dlpi */
#define ISIS_METHOD_DLPI 2

/* constant value for isis method pfpacket */
#define ISIS_METHOD_PFPACKET 1

/* KAME IPv6 */
/* #undef KAME */

/* Linux IPv6 stack */
#define LINUX_IPV6 1

/* Mask for log files */
#define LOGFILE_MASK 0664

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Maximum number of paths for a route */
#define MULTIPATH_NUM 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* OpenBSD */
/* #undef OPEN_BSD */

/* Have openpam_ttyconv */
/* #undef PAM_CONV_FUNC */

/* Hide deprecated interfaces */
#define QUAGGA_NO_DEPRECATED_INTERFACES 1

/* Define to the type of arg 1 for `select'. */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for `select'. */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg 5 for `select'. */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* Use SNMP AgentX to interface with snmpd */
/* #undef SNMP_AGENTX */

/* Use SNMP SMUX to interface with snmpd */
/* #undef SNMP_SMUX */

/* Solaris IPv6 */
/* #undef SOLARIS_IPV6 */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* SunOS 5 */
/* #undef SUNOS_5 */

/* SunOS 5.6 to 5.7 */
/* #undef SUNOS_56 */

/* SunOS 5.8 up */
/* #undef SUNOS_59 */

/* OSPFAPI */
#define SUPPORT_OSPF_API /**/

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Enable IS-IS topology generator code */
/* #undef TOPOLOGY_GENERATE */

/* Use PAM for authentication */
/* #undef USE_PAM */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* VTY shell */
#define VTYSH /**/

/* VTY Sockets Group */
/* #undef VTY_GROUP */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif


/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#define restrict __restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Old readline */
/* #undef rl_completion_matches */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */


#include "oem_config.h"

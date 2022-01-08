/*
 * rtpl_def.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __RTPL_DEF_H__
#define __RTPL_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif




/* 
 * OSPF Fragmentation / fragmented writes
 *
 * ospfd can support writing fragmented packets, for cases where
 * kernel will not fragment IP_HDRINCL and/or multicast destined
 * packets (ie TTBOMK all kernels, BSD, SunOS, Linux). However,
 * SunOS, probably BSD too, clobber the user supplied IP ID and IP
 * flags fields, hence user-space fragmentation will not work.
 * Only Linux is known to leave IP header unmolested.
 * Further, fragmentation really should be done the kernel, which already
 * supports it, and which avoids nasty IP ID state problems.
 *
 * Fragmentation of OSPF packets can be required on networks with router
 * with many many interfaces active in one area, or on networks with links
 * with low MTUs.
 */
#ifdef GNU_LINUX
#define WANT_OSPF_WRITE_FRAGMENT
#endif




#ifdef HAVE_BROKEN_CMSG_FIRSTHDR
/* This bug is present in Solaris 8 and pre-patch Solaris 9 <sys/socket.h>;
   please refer to http://bugzilla.quagga.net/show_bug.cgi?id=142 */

/* Check that msg_controllen is large enough. */
#define ZCMSG_FIRSTHDR(mhdr) \
  (((size_t)((mhdr)->msg_controllen) >= sizeof(struct ipstack_cmsghdr)) ? \
   IPSTACK_CMSG_FIRSTHDR(mhdr) : (struct ipstack_cmsghdr *)NULL)

#warning "IPSTACK_CMSG_FIRSTHDR is broken on this platform, using a workaround"

#else /* HAVE_BROKEN_CMSG_FIRSTHDR */
#define ZCMSG_FIRSTHDR(M) IPSTACK_CMSG_FIRSTHDR(M)
#endif /* HAVE_BROKEN_CMSG_FIRSTHDR */



/* 
 * RFC 3542 defines several macros for using struct ipstack_cmsghdr.
 * Here, we define those that are not present
 */

/*
 * Internal defines, for use only in this file.
 * These are likely wrong on other than ILP32 machines, so warn.
 */
#ifndef _CMSG_DATA_ALIGN
#define _CMSG_DATA_ALIGN(n)           (((n) + 3) & ~3)
#endif /* _CMSG_DATA_ALIGN */

#ifndef _CMSG_HDR_ALIGN
#define _CMSG_HDR_ALIGN(n)            (((n) + 3) & ~3)
#endif /* _CMSG_HDR_ALIGN */

/*
 * IPSTACK_CMSG_SPACE and IPSTACK_CMSG_LEN are required in RFC3542, but were new in that
 * version.
 */
#ifndef IPSTACK_CMSG_SPACE
#define IPSTACK_CMSG_SPACE(l)       (_CMSG_DATA_ALIGN(sizeof(struct ipstack_cmsghdr)) + \
                              _CMSG_HDR_ALIGN(l))
#warning "assuming 4-byte alignment for IPSTACK_CMSG_SPACE"
#endif  /* IPSTACK_CMSG_SPACE */


#ifndef IPSTACK_CMSG_LEN
#define IPSTACK_CMSG_LEN(l)         (_CMSG_DATA_ALIGN(sizeof(struct ipstack_cmsghdr)) + (l))
#warning "assuming 4-byte alignment for IPSTACK_CMSG_LEN"
#endif /* IPSTACK_CMSG_LEN */




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


/* 
 * IP_HDRINCL / struct ip byte order
 *
 * Linux: network byte order
 * *BSD: network, except for length and offset. (cf Stevens)
 * SunOS: nominally as per BSD. but bug: network order on LE.
 * OpenBSD: network byte order, apart from older versions which are as per 
 *          *BSD
 */
#if defined(__NetBSD__) \
   || (defined(__FreeBSD__) && (__FreeBSD_version < 1100030)) \
   || (defined(__OpenBSD__) && (OpenBSD < 200311)) \
   || (defined(__APPLE__)) \
   || (defined(SUNOS_5) && defined(WORDS_BIGENDIAN))
#define HAVE_IP_HDRINCL_BSD_ORDER
#endif


/*  The definition of struct ipstack_in_pktinfo is missing in old version of
    GLIBC 2.1 (Redhat 6.1).  */
#if defined (GNU_LINUX) && (!defined (__USE_MISC) && !defined(__UAPI_DEF_IN_PKTINFO))
struct ipstack_in_pktinfo
{
  int ipi_ifindex;
  struct ipstack_in_addr ipi_spec_dst;
  struct ipstack_in_addr ipi_addr;
};
#endif
#if defined (GNU_LINUX) && (!defined (__USE_MISC) && !defined(__UAPI_DEF_IN6_PKTINFO))
struct ipstack_in6_pktinfo
  {
    struct ipstack_in6_addr ipi6_addr;
    zpl_uint32 ipi6_ifindex;
  };
#endif







#ifdef __cplusplus
}
#endif

#endif /* __RTPL_DEF_H__ */

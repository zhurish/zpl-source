/*
 *
 * Copyright (C) 2000  Robert Olsson.
 * Swedish University of Agricultural Sciences
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

/* 
 * This work includes work with the following copywrite:
 *
 * Copyright (C) 1997, 2000 Kunihiro Ishiguro
 *
 */

/* 
 * Thanks to Jens L��s at Swedish University of Agricultural Sciences
 * for reviewing and tests.
 */



#include "auto_include.h"
#include "zpl_type.h"
#include "os_ipstack.h"
#include "module.h"
#include "route_types.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockopt.h"
#include "checksum.h"
#include "log.h"
#include "template.h"
#ifdef ZPL_SHELL_MODULE
#include "vty_include.h"
#endif
#include "nsm_interface.h"
#ifdef ZPL_NSM_IRDP
#include "nsm_rtadv.h"
#include "nsm_irdp.h"
#include "nsm_rib.h"




static void
nsm_parse_irdp_packet(char *p, 
		  int len, 
		  struct interface *ifp)
{
  struct ip *ip = (struct ip *)p ;
  struct ipstack_icmphdr *icmp;
  struct in_addr src;
  int ip_hlen, iplen, datalen;
  struct nsm_interface *zi;
  struct nsm_irdp *irdp;

  zi = ifp->info;
  if (!zi) 
    return;

  irdp = &zi->irdp;
  if (!irdp) 
    return;

  ip_hlen = ip->ip_hl << 2;
  
  sockopt_iphdrincl_swab_systoh (ip);
  
  iplen = ip->ip_len;
  datalen = len - ip_hlen;
  src = ip->ip_src;

  if (len != iplen)
    {
      zlog_err (MODULE_NSM, "IRDP: RX length doesn't match IP length");
      return;
    }

  if (iplen < ICMP_MINLEN) 
    {
      zlog_err (MODULE_NSM, "IRDP: RX ICMP packet too short from %s\n",
  	      inet_ntoa (src));
      return;
    }
    
  /* XXX: RAW doesn't receive link-layer, surely? ??? */
  /* Check so we don't checksum packets longer than oure RX_BUF - (ethlen +
   len of IP-header) 14+20 */
  if (iplen > IRDP_RX_BUF-34) 
    {
      zlog_err (MODULE_NSM, "IRDP: RX ICMP packet too long from %s\n",
	        inet_ntoa (src));
      return;
    }

  icmp = (struct ipstack_icmphdr *) (p+ip_hlen);

  /* check icmp checksum */    
  if (in_cksum (icmp, datalen) != icmp->checksum) 
    {
      zlog_warn (MODULE_NSM, "IRDP: RX ICMP packet from %s. Bad checksum, silently ignored",
                 inet_ntoa (src));
      return;
    }
  
  /* Handle just only IRDP */
  if (!(icmp->type == ICMP_ROUTERADVERT
        || icmp->type == ICMP_ROUTERSOLICIT))
    return;
  
  if (icmp->code != 0) 
    {
      zlog_warn (MODULE_NSM, "IRDP: RX packet type %d from %s. Bad ICMP type code,"
                 " silently ignored",
                 icmp->type, inet_ntoa (src));
      return;
    }

  if (! ((ntohl (ip->ip_dst.s_addr) == INADDR_BROADCAST)
         && (irdp->flags & IF_BROADCAST))
        ||
        (ntohl (ip->ip_dst.s_addr) == INADDR_ALLRTRS_GROUP
         && !(irdp->flags & IF_BROADCAST)))
    {
      zlog_warn (MODULE_NSM, "IRDP: RX illegal from %s to %s while %s operates in %s\n",
                 inet_ntoa (src),
                 ntohl (ip->ip_dst.s_addr) == INADDR_ALLRTRS_GROUP ?
                 "multicast" : inet_ntoa (ip->ip_dst),
                 ifp->name,
                 irdp->flags & IF_BROADCAST ? "broadcast" : "multicast");

      zlog_warn (MODULE_NSM, "IRDP: Please correct settings\n");
      return;
    }

  switch (icmp->type) 
    {
    case ICMP_ROUTERADVERT:
      break;

    case ICMP_ROUTERSOLICIT:

      if(irdp->flags & IF_DEBUG_MESSAGES) 
	zlog_debug (MODULE_NSM, "IRDP: RX Solicit on %s from %s\n",
		    ifp->name,
		    inet_ntoa (src));

      nsm_process_solicit(ifp);
      break;

    default:
      zlog_warn (MODULE_NSM, "IRDP: RX type %d from %s. Bad ICMP type, silently ignored",
		 icmp->type,
		 inet_ntoa (src));
    }
}

static int
nsm_irdp_recvmsg (zpl_socket_t sock, u_char *buf, int size, int *ifindex)
{
  struct ipstack_msghdr msg;
  struct ipstack_iovec iov;
  char adata[CMSG_SPACE( SOPT_SIZE_CMSG_PKTINFO_IPV4() )];
  int ret;

  msg.msg_name = (void *)0;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (void *) adata;
  msg.msg_controllen = sizeof adata;

  iov.iov_base = buf;
  iov.iov_len = size;

  ret = ipstack_recvmsg (sock, &msg, 0);
  if (ret < 0) {
    zlog_warn(MODULE_NSM, "IRDP: recvmsg: read error %s", zpl_strerror(errno));
    return ret;
  }

  if (msg.msg_flags & MSG_TRUNC) {
    zlog_warn(MODULE_NSM, "IRDP: recvmsg: truncated message");
    return ret;
  }
  if (msg.msg_flags & MSG_CTRUNC) {
    zlog_warn(MODULE_NSM, "IRDP: recvmsg: truncated control message");
    return ret;
  }

  *ifindex = getsockopt_ifindex (AF_INET, &msg);

  return ret;
}

int nsm_irdp_read_raw(struct eloop *r)
{
  struct interface *ifp;
  struct nsm_interface *zi;
  struct nsm_irdp *irdp;
  char buf[IRDP_RX_BUF];
  int ret, ifindex = 0;
  
  zpl_socket_t sock = ELOOP_FD (r);
  _nsm_irdp.t_irdp_raw = eloop_add_read (_nsm_irdp.master, nsm_irdp_read_raw, NULL, sock);
  
  ret = nsm_irdp_recvmsg (sock, (u_char *) buf, IRDP_RX_BUF,  &ifindex);
 
  if (ret < 0) zlog_warn (MODULE_NSM, "IRDP: RX Error length = %d", ret);

  ifp = if_lookup_by_index(ifindex);
  if(! ifp ) return ret;

  zi= ifp->info;
  if(! zi ) return ret;

  irdp = &zi->irdp;
  if(! irdp ) return ret;

  if(! (irdp->flags & IF_ACTIVE)) {

    if(irdp->flags & IF_DEBUG_MISC) 
      zlog_debug(MODULE_NSM, "IRDP: RX ICMP for disabled interface %s\n", ifp->name);
    return 0;
  }

  if(irdp->flags & IF_DEBUG_PACKET) {
    int i;
    zlog_debug(MODULE_NSM, "IRDP: RX (idx %d) ", ifindex);
    for(i=0; i < ret; i++) zlog_debug(MODULE_NSM,  "IRDP: RX %x ", buf[i]&0xFF);
  }

  nsm_parse_irdp_packet(buf, ret, ifp);

  return ret;
}

void 
nsm_send_packet(struct interface *ifp, 
	    struct stream *s,
	    u_int32_t dst,
	    struct prefix *p,
	    u_int32_t ttl)
{
  static struct ipstack_sockaddr_in sockdst = {AF_INET};
  struct ipstack_ip *ip;
  struct ipstack_icmphdr *icmp;
  struct ipstack_msghdr *msg;
  struct ipstack_cmsghdr *cmsg;
  struct ipstack_iovec iovector;
  char msgbuf[256];
  char buf[256];
  struct ipstack_in_pktinfo *pktinfo;
  u_long src;
  int on;
 
  if (!(ifp->flags & IFF_UP))
    return;

  if (p)
    src = ntohl(p->u.prefix4.s_addr);
  else 
    src = 0; /* Is filled in */
  
  ip = (struct ipstack_ip *) buf;
  ip->ip_hl = sizeof(struct ipstack_ip) >> 2;
  ip->ip_v = IPVERSION;
  ip->ip_tos = 0xC0;
  ip->ip_off = 0L;
  ip->ip_p = 1;       /* IP_ICMP */
  ip->ip_ttl = ttl;
  ip->ip_src.s_addr = src;
  ip->ip_dst.s_addr = dst;
  icmp = (struct ipstack_icmphdr *) (buf + sizeof (struct ipstack_ip));

  /* Merge IP header with icmp packet */
  assert (stream_get_endp(s) < (sizeof (buf) - sizeof (struct ipstack_ip)));
  stream_get(icmp, s, stream_get_endp(s));

  /* icmp->checksum is already calculated */
  ip->ip_len  = sizeof(struct ipstack_ip) + stream_get_endp(s);

  on = 1;
  if (ipstack_setsockopt(_nsm_irdp.irdp_sock, IPPROTO_IP, IP_HDRINCL,
		 (char *) &on, sizeof(on)) < 0)
    zlog_warn(MODULE_NSM, "sendto %s", zpl_strerror (errno));


  if(dst == INADDR_BROADCAST ) {
    on = 1;
    if (ipstack_setsockopt(_nsm_irdp.irdp_sock, SOL_SOCKET, SO_BROADCAST,
		   (char *) &on, sizeof(on)) < 0)
      zlog_warn(MODULE_NSM, "sendto %s", zpl_strerror (errno));
  }

  if(dst !=  INADDR_BROADCAST) {
      on = 0; 
      if( ipstack_setsockopt(_nsm_irdp.irdp_sock,IPPROTO_IP, IP_MULTICAST_LOOP, 
		     (char *)&on,sizeof(on)) < 0)
	zlog_warn(MODULE_NSM, "sendto %s", zpl_strerror (errno));
  }

  memset(&sockdst,0,sizeof(sockdst));
  sockdst.sin_family=AF_INET;
  sockdst.sin_addr.s_addr = dst;

  cmsg = (struct ipstack_cmsghdr *) (msgbuf + sizeof(struct ipstack_msghdr));
  cmsg->cmsg_len = sizeof(struct ipstack_cmsghdr) + sizeof(struct ipstack_in_pktinfo);
  cmsg->cmsg_level = SOL_IP;
  cmsg->cmsg_type = IP_PKTINFO;
  pktinfo = (struct ipstack_in_pktinfo *) CMSG_DATA(cmsg);
  pktinfo->ipi_ifindex = ifp->ifindex;
  pktinfo->ipi_spec_dst.s_addr = src;
  pktinfo->ipi_addr.s_addr = src;
 
  iovector.iov_base = (void *) buf;
  iovector.iov_len = ip->ip_len; 
  msg = (struct ipstack_msghdr *) msgbuf;
  msg->msg_name = &sockdst;
  msg->msg_namelen = sizeof(sockdst);
  msg->msg_iov = &iovector;
  msg->msg_iovlen = 1;
  msg->msg_control = cmsg;
  msg->msg_controllen = cmsg->cmsg_len;
 
  sockopt_iphdrincl_swab_htosys (ip);
  
  if (ipstack_sendmsg(_nsm_irdp.irdp_sock, msg, 0) < 0) {
    zlog_warn(MODULE_NSM, "sendto %s", zpl_strerror (errno));
  }
  /*   printf("TX on %s idx %d\n", ifp->name, ifp->ifindex); */
}


#endif /* ZPL_NSM_IRDP */




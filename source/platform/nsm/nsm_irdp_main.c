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
#include "zplos_include.h"
#include "module.h"


#ifdef ZPL_NSM_IRDP

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "zmemory.h"
#include "stream.h"
#include "connected.h"
#include "log.h"
#include "zclient.h"
#include "thread.h"
#include "eloop.h"
#include "checksum.h"
#include "log.h"
#include "sockopt.h"

#include "nsm_include.h"
#include "nsm_rtadv.h"
#include "nsm_irdp.h"

/* GLOBAL VARS */





/* Timer interval of irdp. */
int nsm_irdp_timer_interval = IRDP_DEFAULT_INTERVAL;

zpl_socket_t
nsm_irdp_sock_init (void)
{
  int ret, i;
  int save_errno;
  zpl_socket_t sock;

  sock = ipstack_socket (IPSTACK_IPCOM, IPSTACK_AF_INET, IPSTACK_SOCK_RAW, IPSTACK_IPPROTO_ICMP);
  save_errno = errno;


  if (ipstack_invalid(sock)) {
    zlog_warn (MODULE_NSM, "IRDP: can't create irdp socket %s", zpl_strerror(save_errno));
    return sock;
  };
  
  i = 1;
  ret = ipstack_setsockopt (sock, IPSTACK_IPPROTO_IP, IPSTACK_IP_TTL, 
                        (void *) &i, sizeof (i));
  if (ret < 0) {
    zlog_warn (MODULE_NSM, "IRDP: can't do irdp sockopt %s", zpl_strerror(errno));
    ipstack_close(sock);
    return sock;
  };
  
  ret = setsockopt_ifindex (IPSTACK_AF_INET, sock, 1);
  if (ret < 0) {
    zlog_warn (MODULE_NSM, "IRDP: can't do irdp sockopt %s", zpl_strerror(errno));
    ipstack_close(sock);
    return sock;
  };

  nsm_rtadv.t_irdp_raw = eloop_add_read (nsm_rtadv.master, nsm_irdp_read_raw, NULL, sock); 

  return sock;
}


static int
nsm_get_pref(struct nsm_irdp *irdp, struct prefix *p)
{
  struct listnode *node;
  struct nsm_adv *adv;

  /* Use default preference or use the override pref */
  
  if( irdp->AdvPrefList == NULL )
    return irdp->Preference;
  
  for (ALL_LIST_ELEMENTS_RO (irdp->AdvPrefList, node, adv))
    if( p->u.prefix4.s_addr == adv->ip.s_addr )
      return adv->pref;

  return irdp->Preference;
}

/* Make ICMP Router Advertisement Message. */
static int
nsm_make_advertisement_packet (struct interface *ifp, 
			   struct prefix *p,
			   struct stream *s)
{
  struct nsm_interface *zi=ifp->info[MODULE_NSM];
  struct nsm_irdp *irdp=&zi->irdp;
  int size;
  int pref;
  u_int16_t checksum;

  pref =  nsm_get_pref(irdp, p);

  stream_putc (s, ICMP_ROUTERADVERT); /* Type. */
  stream_putc (s, 0);		/* Code. */
  stream_putw (s, 0);		/* Checksum. */
  stream_putc (s, 1);		/* Num address. */
  stream_putc (s, 2);		/* Address Entry Size. */

  if(irdp->flags & IF_SHUTDOWN)  
    stream_putw (s, 0);
  else 
    stream_putw (s, irdp->Lifetime);

  stream_putl (s, htonl(p->u.prefix4.s_addr)); /* Router address. */
  stream_putl (s, pref);

  /* in_cksum return network byte order value */
  size = 16;
  checksum = in_cksum (s->data, size);
  stream_putw_at (s, 2, htons(checksum));

  return size;
}

static void
nsm_irdp_send(struct interface *ifp, struct prefix *p, struct stream *s)
{
  struct nsm_interface *zi=ifp->info[MODULE_NSM];
  struct nsm_irdp *irdp=&zi->irdp;
  char buf[PREFIX_STRLEN];
  u_int32_t dst;
  u_int32_t ttl=1;

  if (! (ifp->flags & IFF_UP)) return; 

  if (irdp->flags & IF_BROADCAST) 
    dst =INADDR_BROADCAST ;
  else 
    dst = htonl(INADDR_ALLHOSTS_GROUP);

  if(irdp->flags & IF_DEBUG_MESSAGES) 
    zlog_debug(MODULE_NSM, "IRDP: TX Advert on %s %s Holdtime=%d Preference=%d",
	      ifp->name,
	      prefix2str(p, buf, sizeof buf),
	      irdp->flags & IF_SHUTDOWN? 0 : irdp->Lifetime,
	      nsm_get_pref(irdp, p));

  nsm_send_packet (ifp, s, dst, p, ttl);
}

static void nsm_irdp_advertisement (struct interface *ifp, struct prefix *p)
{
  struct stream *s;
  s = stream_new (128);
  nsm_make_advertisement_packet (ifp, p, s);
  nsm_irdp_send(ifp, p, s);
  stream_free (s);
}

int nsm_irdp_send_thread(struct eloop *t_advert)
{
  u_int32_t timer, tmp;
  struct interface *ifp = ELOOP_ARG (t_advert);
  struct nsm_interface *zi=ifp->info[MODULE_NSM];
  struct nsm_irdp *irdp=&zi->irdp;
  struct prefix *p;
  struct listnode *node, *nnode;
  struct connected *ifc;

  irdp->flags &= ~IF_SOLICIT;

  if(ifp->connected) 
    for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, ifc))
      {
        p = ifc->address;
        
        if (p->family != AF_INET)
          continue;
        
        nsm_irdp_advertisement(ifp, p);
        irdp->nsm_irdp_sent++;
      }

  tmp = irdp->MaxAdvertInterval-irdp->MinAdvertInterval;
  timer =  (random () % tmp ) + 1;
  timer = irdp->MinAdvertInterval + timer;

  if(irdp->nsm_irdp_sent <  MAX_INITIAL_ADVERTISEMENTS &&
     timer > MAX_INITIAL_ADVERT_INTERVAL ) 
	  timer= MAX_INITIAL_ADVERT_INTERVAL;

  if(irdp->flags & IF_DEBUG_MISC)
    zlog_debug(MODULE_NSM, "IRDP: New timer for %s set to %u\n", ifp->name, timer);

  irdp->t_advertise = eloop_add_timer(nsm_rtadv.master, nsm_irdp_send_thread, ifp, timer);
  return 0;
}

void nsm_irdp_advert_off(struct interface *ifp)
{
  struct nsm_interface *zi=ifp->info[MODULE_NSM];
  struct nsm_irdp *irdp=&zi->irdp;
  struct listnode *node, *nnode;
  int i;
  struct connected *ifc;
  struct prefix *p;

  if(irdp->t_advertise)  eloop_cancel(irdp->t_advertise);
  irdp->t_advertise = NULL;
  
  if(ifp->connected) 
    for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, ifc))
      {
        p = ifc->address;

        /* Output some packets with Lifetime 0 
           we should add a wait...
        */

        for(i=0; i< IRDP_LAST_ADVERT_MESSAGES; i++) 
          {
            irdp->nsm_irdp_sent++;
            nsm_irdp_advertisement(ifp, p);
          }
      }
}


void nsm_process_solicit (struct interface *ifp)
{
  struct nsm_interface *zi=ifp->info[MODULE_NSM];
  struct nsm_irdp *irdp=&zi->irdp;
  u_int32_t timer;

  /* When SOLICIT is active we reject further incoming solicits 
     this keeps down the answering rate so we don't have think
     about DoS attacks here. */

  if( irdp->flags & IF_SOLICIT) return;

  irdp->flags |= IF_SOLICIT;
  if(irdp->t_advertise)  eloop_cancel(irdp->t_advertise);
  irdp->t_advertise = NULL;

  timer =  (random () % MAX_RESPONSE_DELAY) + 1;

  irdp->t_advertise = eloop_add_timer(nsm_rtadv.master, 
				       nsm_irdp_send_thread, 
				       ifp, 
				       timer);
}

void nsm_irdp_finish(void)
{

  struct listnode *node, *nnode;
  struct interface *ifp;
  struct nsm_interface *zi;
  struct nsm_irdp *irdp;
  struct list *iflist = NULL;
  iflist = if_list_get();
  zlog_info(MODULE_NSM, "IRDP: Received shutdown notification.");
  
  for (ALL_LIST_ELEMENTS (iflist, node, nnode, ifp))
    {
      zi = ifp->info[MODULE_NSM];
      
      if (!zi) 
        continue;
      irdp = &zi->irdp;
      if (!irdp) 
        continue;

      if (irdp->flags & IF_ACTIVE ) 
        {
	  irdp->flags |= IF_SHUTDOWN;
	  nsm_irdp_advert_off(ifp);
        }
    }
}

#endif /* ZPL_NSM_IRDP */

/*
 * Zebra debug related function
 * Copyright (C) 1999 Kunihiro Ishiguro
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#ifndef _NSM_DEBUG_H
#define _NSM_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Debug flags. */
#define NSM_DEBUG_EVENT   0x01

#define NSM_DEBUG_PACKET  0x01
#define NSM_DEBUG_SEND    0x20
#define NSM_DEBUG_RECV    0x40
#define NSM_DEBUG_DETAIL  0x80

#define NSM_DEBUG_KERNEL  0x01

#define NSM_DEBUG_RIB     0x01
#define NSM_DEBUG_RIB_Q   0x02


#define NSM_DEBUG_NHT     0x01

/* Debug related macro. */
#define IS_NSM_DEBUG_EVENT  (nsm_debug_event & NSM_DEBUG_EVENT)

#define IS_NSM_DEBUG_PACKET (nsm_debug_packet & NSM_DEBUG_PACKET)
#define IS_NSM_DEBUG_SEND   (nsm_debug_packet & NSM_DEBUG_SEND)
#define IS_NSM_DEBUG_RECV   (nsm_debug_packet & NSM_DEBUG_RECV)
#define IS_NSM_DEBUG_DETAIL (nsm_debug_packet & NSM_DEBUG_DETAIL)

#define IS_NSM_DEBUG_KERNEL (nsm_debug_kernel & NSM_DEBUG_KERNEL)

#define IS_NSM_DEBUG_RIB  (nsm_debug_rib & NSM_DEBUG_RIB)
#define IS_NSM_DEBUG_RIB_Q  (nsm_debug_rib & NSM_DEBUG_RIB_Q)

#define IS_NSM_DEBUG_NHT  (nsm_debug_nht & NSM_DEBUG_NHT)

extern zpl_ulong nsm_debug_event;
extern zpl_ulong nsm_debug_packet;
extern zpl_ulong nsm_debug_kernel;
extern zpl_ulong nsm_debug_rib;

extern zpl_ulong nsm_debug_nht;

extern int cmd_debug_init(void);


extern int nsm_debug_init (void);
 
#ifdef __cplusplus
}
#endif

#endif /* _NSM_DEBUG_H */

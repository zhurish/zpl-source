/*
 * $Id: gport.h,v 1.39 Broadcom SDK $
 * $Copyright: Copyright 2011 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 * This file defines gport (generic port) parameters. The gport is useful
 * for specifying the following types of identifiers:
 *   LOCAL      :  port on the local unit
 *   MODPORT    :  {module ID, port} pair
 *   TRUNK      :  trunk ID
 *   BLACK_HOLE :  identifier indicating drop destination
 *   LOCAL_CPU  :  identifier indicating local CPU destination
 *   MPLS_PORT  :  L2 MPLS virtual-port (VPLS/VPWS)
 *   SUBPORT_GROUP : Subport group
 *   SUBPORT_PORT  : Subport virtual-port
 *   UCAST_QUEUE_GROUP : Group of unicast COSQs
 *   MCAST_QUEUE_GROUP : Group of multicast COSQs
 *   UCAST_SUBSCRIBER_QUEUE_GROUP : Group of 64K subscriber unicast COSQs
 *   MCAST_SUBSCRIBER_QUEUE_GROUP : Group of 64K subscriber multicast COSQs
 *   MCAST :  distribution set identifier
 *   SCHEDULER  : COSQ scheduler
 *   DEVPORT    : {device ID, port} pair (ports on devices without a module ID)
 *   SPECIAL    :  application special value (invalid in BCM APIs)
 *   MIRROR     :  Mirror (modport & encapsulation) for mirrored traffic.
 *   MIM_PORT   :  MIM virtual-port
 *   VLAN_PORT  :  VLAN virtual-port
 *   WLAN_PORT  :  WLAN virtual-port
 *   TRILL_PORT :  TRILL virtual-port
 *   NIV_PORT   :  NIV virtual-port
 *   TUNNEL     :  Tunnel ID
 *
 * This header requires that the uint32 type be defined before inclusion.
 * Using <sal/types.h> is the simplest (and recommended) way of doing this.
 *
 * Its contents are not used directly by applications; it is used only
 * by header files of parent APIs which need to define gport parameters.
 *
 * The following macros are made available.  All have names starting
 * with _SHR_, which have been elided from this list:
 *
 * Constants or Expressions suitable for assignment:
 * GPORT_NONE                   gport initializer
 * GPORT_INVALID                invalid gport identifier
 * GPORT_BLACK_HOLE             black-hole gport idenitifier
 * GPORT_LOCAL_CPU              local CPU gport identifier
 *
 * Predicates: (return 0 or 1, suitable for using in if statements)
 * GPORT_IS_SET                 is the gport set?
 * GPORT_IS_LOCAL               is the gport a local port type?
 * GPORT_IS_MODPORT             is the gport a modid/port type?
 * GPORT_IS_TRUNK               is the gport a trunk type?
 * GPORT_IS_BLACK_HOLE          is the gport a black-hole type?
 * GPORT_IS_LOCAL_CPU           is the gport a local CPU type?
 * GPORT_IS_MPLS_PORT           is the gport a MPLS port type?
 * GPORT_IS_SUBPORT_GROUP       is the gport a subport group type?
 * GPORT_IS_SUBPORT_PORT        is the gport a subport port type?
 * GPORT_IS_UCAST_QUEUE_GROUP   is the gport a unicast group type?
 * GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP   is the gport a subscriber unicast group type?
 * GPORT_IS_MCAST               is the gport a multicast set type?
 * GPORT_IS_MCAST_QUEUE_GROUP   is the gport a multicast queue group type?
 * GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP   is the gport a multicast subscriber queue group type?
 * GPORT_IS_SCHEDULER           is the gport a scheduler type?
 * GPORT_IS_DEVPORT             is the gport a devid/port type?
 * GPORT_IS_SPECIAL             is the gport a special type?
 * GPORT_IS_MIRROR              is the gport a mirror destination type?
 * GPORT_IS_MIM_PORT            is the gport a MIM port type?
 * GPORT_IS_VLAN_PORT           is the gport a VLAN port type?
 * GPORT_IS_WLAN_PORT           is the gport a WLAN port type?
 * GPORT_IS_TRILL_PORT          is the gport a TRILL port type?
 * GPORT_IS_NIV_PORT            is the gport a NIV port type?
 * GPORT_IS_TUNNEL              is the gport a tunnel type?
 * GPORT_IS_CHILD               is the gport a child port type?
 * GPORT_IS_EGRESS_GROUP        is the gport a egress group type?
 * GPORT_IS_EGRESS_CHILD        is the gport a egress child type?
 * GPORT_IS_EGRESS_MODPORT      is the gport a egress modport type?
 *
 * Statements: (cannot be used as a predicate)
 * GPORT_LOCAL_SET              set a gport local port type and value
 * GPORT_LOCAL_GET              get a port value from a local gport
 * GPORT_LOCAL_CPU_SET          set a gport local CPU port type and value
 * GPORT_LOCAL_CPU_GET          get a CPU port value from a local gport
 * GPORT_MODPORT_SET            set a gport modid/port type and values
 * GPORT_MODPORT_MODID_GET      get a modid value from a modport gport
 * GPORT_MODPORT_PORT_GET       get a port value from a modport gport
 * GPORT_TRUNK_SET              set a gport trunk type and value
 * GPORT_TRUNK_GET              get a trunk_id value from a trunk gport
 * GPORT_MPLS_PORT_ID_SET       set a MPLS ID type and value
 * GPORT_MPLS_PORT_ID_GET       get a MPLS ID from a MPLS gport
 * GPORT_SUBPORT_GROUP_SET      set a subport group type and value
 * GPORT_SUBPORT_GROUP_GET      get a subport group ID from a gport
 * GPORT_SUBPORT_PORT_SET       set a subport port type and value
 * GPORT_SUBPORT_PORT_GET       get a subport port ID from a gport
 * GPORT_SCHEDULER_SET          set a scheduler type and value
 * GPORT_SCHEDULER_GET          get a scheduler ID from a gport
 * GPORT_DEVPORT_SET            set a gport devid/port type and values
 * GPORT_DEVPORT_DEVID_GET      get a devid value from a devport gport
 * GPORT_DEVPORT_PORT_GET       get a port value from a devport gport
 * GPORT_UCAST_QUEUE_GROUP_SET  set a queue group type and value
 * GPORT_UCAST_QUEUE_GROUP_GET  get a queue group ID from a gport
 * GPORT_MCAST_SET              set a mcast dist set and value
 * GPORT_MCAST_GET              get an mcast dist set from a gport
 * GPORT_MCAST_DS_ID_SET        set an mcast DS ID and value
 * GPORT_MCAST_DS_ID_GET        get an mcast DS ID from a gport
 * GPORT_MCAST_QUEUE_GROUP_SET  set an mcast queue group type and value
 * GPORT_MCAST_QUEUE_GROUP_GET  get an mcast queue group ID from a gport
 * GPORT_MCAST_QUEUE_GROUP_SYSQID_SET set an mcast queue group system queue ID
 * GPORT_MCAST_QUEUE_GROUP_SYSPORTID_GET get an mcast queue group system port ID
 * GPORT_MCAST_QUEUE_GROUP_QID_GET get an mcast queue group queue ID
 * GPORT_UCAST_QUEUE_GROUP_SYSQID_SET get a unicast queue group system queue ID
 * GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET get a unicast queue group system port ID
 * GPORT_UCAST_QUEUE_GROUP_QID_GET get a unicast queue group queue ID
 * GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET set a ucast subscriber queue group queue id
 * GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET get a ucast subscriber queue group queue id
 * GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET set an mcast subscriber queue group queue id
 * GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET get an mcast subscriber queue group queue id
 * GPORT_SPECIAL_SET            set a gport special type and value
 * GPORT_SPECIAL_GET            get a value from a special gport
 * GPORT_MIRROR_SET             set a gport mirror type and value
 * GPORT_MIRROR_GET             get a value from a mirror gport
 * GPORT_MIM_PORT_ID_SET        set a MIM ID type and value
 * GPORT_MIM_PORT_ID_GET        get a MIM ID from a Mim gport
 * GPORT_VLAN_PORT_ID_SET       set a VLAN port ID type and value
 * GPORT_VLAN_PORT_ID_GET       get a VLAN port ID from a VLAN gport
 * GPORT_WLAN_PORT_ID_SET       set a WLAN ID type and value
 * GPORT_WLAN_PORT_ID_GET       get a WLAN ID from a WLAN gport
 * GPORT_TRILL_PORT_ID_SET      set a TRILL ID type and value
 * GPORT_TRILL_PORT_ID_GET      get a TRILL ID from a Trill gport
 * GPORT_NIV_PORT_ID_SET        set a NIV ID type and value 
 * GPORT_NIV_PORT_ID_GET        get a NIV ID from a NIV gport
 * GPORT_TUNNEL_ID_SET          set a gport tunnel type and value
 * GPORT_TUNNEL_ID_GET          get a tunnel_id value from a tunnel gport
 * GPORT_CHILD_SET              set a child gport
 * GPORT_CHILD_MODID_GET        get a child gport modid
 * GPORT_CHILD_PORT_GET         get a child gport port
 * GPORT_EGRESS_GROUP_SET       set an egress group
 * GPORT_EGRESS_GROUP_GET       get an egress group
 * GPORT_EGRESS_GROUP_MODID_GET get an egress group modid
 * GPORT_EGRESS_CHILD_SET       set an egress child gport
 * GPORT_EGRESS_CHILD_MODID_GET get an egress child gport modid
 * GPORT_EGRESS_CHILD_PORT_GET  get an egress child gport port
 * GPORT_EGRESS_MODPORT_SET     set an egress modport
 * GPORT_EGRESS_MODPORT_MODID_GET get an egress modport modid
 * GPORT_EGRESS_MODPORT_PORT_GET  get an egress modport port
 */

#ifndef _SHR_GPORT_H
#define _SHR_GPORT_H
/*
 * Typedef:
 *    _shr_gport_t
 * Purpose:
 *    GPORT (generic port) type for shared definitions
 */
typedef uint32 _shr_gport_t;

/*
 * Defines:
 *     _SHR_GPORT_*
 * Purpose:
 *     GPORT (Generic port) definitions. GPORT can be used to specify
 *     a {module, port} pair, trunk, and other port types.
 */

#define _SHR_GPORT_NONE             (0)
#define _SHR_GPORT_INVALID         (-1)

#define _SHR_GPORT_TYPE_LOCAL               1  /* Port on local unit     */
#define _SHR_GPORT_TYPE_MODPORT             2  /* Module ID and port     */
#define _SHR_GPORT_TYPE_TRUNK               3  /* Trunk ID               */
#define _SHR_GPORT_TYPE_BLACK_HOLE          4  /* Black hole destination */
#define _SHR_GPORT_TYPE_LOCAL_CPU           5  /* CPU destination        */
#define _SHR_GPORT_TYPE_MPLS_PORT           6  /* MPLS port ID           */
#define _SHR_GPORT_TYPE_SUBPORT_GROUP       7  /* Subport group ID       */
#define _SHR_GPORT_TYPE_SUBPORT_PORT        8  /* Subport port ID        */
#define _SHR_GPORT_TYPE_UCAST_QUEUE_GROUP   9  /* Queue Group            */
#define _SHR_GPORT_TYPE_DEVPORT            10  /* Device ID and port     */
#define _SHR_GPORT_TYPE_MCAST              11  /* Multicast Set          */
#define _SHR_GPORT_TYPE_MCAST_QUEUE_GROUP  12  /* Multicast Queue Group  */
#define _SHR_GPORT_TYPE_SCHEDULER          13  /* Scheduler              */
#define _SHR_GPORT_TYPE_SPECIAL            14  /* Special value          */
#define _SHR_GPORT_TYPE_MIRROR             15  /* Mirror destination.    */
#define _SHR_GPORT_TYPE_MIM_PORT           16  /* MIM port ID            */
#define _SHR_GPORT_TYPE_VLAN_PORT          17  /* VLAN port              */
#define _SHR_GPORT_TYPE_WLAN_PORT          18  /* WLAN port              */
#define _SHR_GPORT_TYPE_TUNNEL             19  /* Tunnel ID              */
#define _SHR_GPORT_TYPE_CHILD              20  /* Child port             */
#define _SHR_GPORT_TYPE_EGRESS_GROUP       21  /* Egress group port      */
#define _SHR_GPORT_TYPE_EGRESS_CHILD       22  /* Egress child port      */
#define _SHR_GPORT_TYPE_EGRESS_MODPORT     23  /* Egress Mod and port    */
#define _SHR_GPORT_TYPE_UCAST_SUBSCRIBER_QUEUE_GROUP  24  /* Local Queue Group */
#define _SHR_GPORT_TYPE_MCAST_SUBSCRIBER_QUEUE_GROUP  25  /* Local Multicast Queue Group  */
#define _SHR_GPORT_TYPE_TRILL_PORT         26   /* TRILL port */  
#define _SHR_GPORT_TYPE_NIV_PORT           27   /* NIV port */  
#define _SHR_GPORT_TYPE_MAX _SHR_GPORT_TYPE_NIV_PORT /* Used for sanity */
                                                            /* checks only.    */

#define _SHR_GPORT_TYPE_SHIFT                           26
#define _SHR_GPORT_TYPE_MASK                            0x3f
#define _SHR_GPORT_MODID_SHIFT                          10
#define _SHR_GPORT_MODID_MASK                           0xffff
#define _SHR_GPORT_PORT_SHIFT                           0
#define _SHR_GPORT_PORT_MASK                            0x3ff
#define _SHR_GPORT_TRUNK_SHIFT                          0
#define _SHR_GPORT_TRUNK_MASK                           0x3ffffff
#define _SHR_GPORT_MPLS_PORT_SHIFT                      0
#define _SHR_GPORT_MPLS_PORT_MASK                       0xffffff
#define _SHR_GPORT_SUBPORT_GROUP_SHIFT                  0
#define _SHR_GPORT_SUBPORT_GROUP_MASK                   0xffffff
#define _SHR_GPORT_SUBPORT_PORT_SHIFT                   0
#define _SHR_GPORT_SUBPORT_PORT_MASK                    0xffffff
#define _SHR_GPORT_UCAST_QUEUE_GROUP_SHIFT              0
#define _SHR_GPORT_UCAST_QUEUE_GROUP_MASK               0x3ffffff
#define _SHR_GPORT_UCAST_QUEUE_GROUP_QID_SHIFT          0
#define _SHR_GPORT_UCAST_QUEUE_GROUP_QID_MASK           0x3fff
#define _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_SHIFT    14
#define _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_MASK     0xfff
#define _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT 0
#define _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK  0xffff
#define _SHR_GPORT_MCAST_SHIFT                          0
#define _SHR_GPORT_MCAST_MASK                           0x3ffffff
#define _SHR_GPORT_MCAST_DS_ID_SHIFT                    0
#define _SHR_GPORT_MCAST_DS_ID_MASK                     0x3ffffff
#define _SHR_GPORT_MCAST_QUEUE_GROUP_SHIFT              0
#define _SHR_GPORT_MCAST_QUEUE_GROUP_MASK               0x3ffffff
#define _SHR_GPORT_MCAST_QUEUE_GROUP_QID_SHIFT          0
#define _SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK           0x3fff
#define _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_SHIFT    14
#define _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_MASK     0xfff
#define _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT 0
#define _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK  0x3fff
#define _SHR_GPORT_SCHEDULER_SHIFT                      0
#define _SHR_GPORT_SCHEDULER_MASK                       0x3ffffff
#define _SHR_GPORT_SCHEDULER_LEVEL_SHIFT                20
#define _SHR_GPORT_SCHEDULER_LEVEL_MASK                 0x1f
#define _SHR_GPORT_SCHEDULER_NODE_SHIFT                 0
#define _SHR_GPORT_SCHEDULER_NODE_MASK                  0xfffff
#define _SHR_GPORT_DEVID_SHIFT                          _SHR_GPORT_MODID_SHIFT
#define _SHR_GPORT_DEVID_MASK                           _SHR_GPORT_MODID_MASK
#define _SHR_GPORT_SPECIAL_SHIFT                        0
#define _SHR_GPORT_SPECIAL_MASK                         0xffff
#define _SHR_GPORT_MIRROR_SHIFT                         0
#define _SHR_GPORT_MIRROR_MASK                          0xffff
#define _SHR_GPORT_MIM_PORT_SHIFT                       0
#define _SHR_GPORT_MIM_PORT_MASK                        0xffffff
#define _SHR_GPORT_VLAN_PORT_SHIFT                      0
#define _SHR_GPORT_VLAN_PORT_MASK                       0xffffff
#define _SHR_GPORT_WLAN_PORT_SHIFT                      0
#define _SHR_GPORT_WLAN_PORT_MASK                       0xffffff
#define _SHR_GPORT_TRILL_PORT_SHIFT                     0
#define _SHR_GPORT_TRILL_PORT_MASK                      0xffffff
#define _SHR_GPORT_NIV_PORT_SHIFT                       0
#define _SHR_GPORT_NIV_PORT_MASK                        0xffffff
#define _SHR_GPORT_TUNNEL_SHIFT                         0
#define _SHR_GPORT_TUNNEL_MASK                          0x3ffffff
#define _SHR_GPORT_CHILD_SHIFT                          0
#define _SHR_GPORT_CHILD_MASK                           0x3ffffff
#define _SHR_GPORT_CHILD_MODID_SHIFT                    10
#define _SHR_GPORT_CHILD_MODID_MASK                     0xffff
#define _SHR_GPORT_CHILD_PORT_SHIFT                     0
#define _SHR_GPORT_CHILD_PORT_MASK                      0x3ff
#define _SHR_GPORT_EGRESS_GROUP_MODID_SHIFT             10
#define _SHR_GPORT_EGRESS_GROUP_MODID_MASK              0xffff
#define _SHR_GPORT_EGRESS_GROUP_SHIFT                   0
#define _SHR_GPORT_EGRESS_GROUP_MASK                    0x3ff
#define _SHR_GPORT_EGRESS_CHILD_SHIFT                   0
#define _SHR_GPORT_EGRESS_CHILD_MASK                    0x3ffffff
#define _SHR_GPORT_EGRESS_CHILD_PORT_SHIFT              0
#define _SHR_GPORT_EGRESS_CHILD_PORT_MASK               0x3ff
#define _SHR_GPORT_EGRESS_CHILD_MODID_SHIFT             10
#define _SHR_GPORT_EGRESS_CHILD_MODID_MASK              0xffff
#define _SHR_GPORT_EGRESS_MODPORT_SHIFT                 0
#define _SHR_GPORT_EGRESS_MODPORT_MASK                  0x3ffffff
#define _SHR_GPORT_EGRESS_MODPORT_MODID_SHIFT           10
#define _SHR_GPORT_EGRESS_MODPORT_MODID_MASK            0xffff
#define _SHR_GPORT_EGRESS_MODPORT_PORT_SHIFT            0
#define _SHR_GPORT_EGRESS_MODPORT_PORT_MASK             0x3ff

#define _SHR_GPORT_IS_SET(_gport)    \
        ((((_gport) >> _SHR_GPORT_TYPE_SHIFT) > 0) && \
         (((_gport) >> _SHR_GPORT_TYPE_SHIFT) <= _SHR_GPORT_TYPE_MAX))

#define _SHR_GPORT_IS_LOCAL(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_LOCAL)

#define _SHR_GPORT_LOCAL_SET(_gport, _port)                                  \
        ((_gport) = (_SHR_GPORT_TYPE_LOCAL     << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_port) & _SHR_GPORT_PORT_MASK)     << _SHR_GPORT_PORT_SHIFT))

#define _SHR_GPORT_LOCAL_GET(_gport)    \
        (((_gport) >> _SHR_GPORT_PORT_SHIFT) & _SHR_GPORT_PORT_MASK)

#define _SHR_GPORT_IS_MODPORT(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MODPORT)

#define _SHR_GPORT_MODPORT_SET(_gport, _module, _port)                       \
        ((_gport) = (_SHR_GPORT_TYPE_MODPORT   << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_module) & _SHR_GPORT_MODID_MASK)  << _SHR_GPORT_MODID_SHIFT)  | \
         (((_port) & _SHR_GPORT_PORT_MASK)     << _SHR_GPORT_PORT_SHIFT))

#define _SHR_GPORT_MODPORT_MODID_GET(_gport)    \
        (((_gport) >> _SHR_GPORT_MODID_SHIFT) & _SHR_GPORT_MODID_MASK)

#define _SHR_GPORT_MODPORT_PORT_GET(_gport)     \
        (((_gport) >> _SHR_GPORT_PORT_SHIFT) & _SHR_GPORT_PORT_MASK)

#define _SHR_GPORT_IS_TRUNK(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_TRUNK)

#define _SHR_GPORT_TRUNK_SET(_gport, _trunk_id)                              \
        ((_gport) = (_SHR_GPORT_TYPE_TRUNK      << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_trunk_id) & _SHR_GPORT_TRUNK_MASK) << _SHR_GPORT_TRUNK_SHIFT))

#define _SHR_GPORT_TRUNK_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_TRUNK_SHIFT) & _SHR_GPORT_TRUNK_MASK)

#define _SHR_GPORT_IS_MPLS_PORT(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MPLS_PORT)

#define _SHR_GPORT_MPLS_PORT_ID_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_MPLS_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_MPLS_PORT_MASK) << _SHR_GPORT_MPLS_PORT_SHIFT))

#define _SHR_GPORT_MPLS_PORT_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_MPLS_PORT_SHIFT) & _SHR_GPORT_MPLS_PORT_MASK)

#define _SHR_GPORT_IS_SUBPORT_GROUP(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_SUBPORT_GROUP)

#define _SHR_GPORT_SUBPORT_GROUP_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_SUBPORT_GROUP << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_SUBPORT_GROUP_MASK) << _SHR_GPORT_SUBPORT_GROUP_SHIFT))

#define _SHR_GPORT_SUBPORT_GROUP_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_SUBPORT_GROUP_SHIFT) & _SHR_GPORT_SUBPORT_GROUP_MASK)

#define _SHR_GPORT_IS_SUBPORT_PORT(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_SUBPORT_PORT)

#define _SHR_GPORT_SUBPORT_PORT_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_SUBPORT_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_SUBPORT_PORT_MASK) << _SHR_GPORT_SUBPORT_PORT_SHIFT))

#define _SHR_GPORT_SUBPORT_PORT_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_SUBPORT_PORT_SHIFT) & _SHR_GPORT_SUBPORT_PORT_MASK)

#define _SHR_GPORT_IS_SCHEDULER(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_SCHEDULER)

#define _SHR_GPORT_SCHEDULER_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_SCHEDULER << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_SCHEDULER_MASK) << _SHR_GPORT_SCHEDULER_SHIFT))

#define _SHR_GPORT_SCHEDULER_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_SCHEDULER_SHIFT) & _SHR_GPORT_SCHEDULER_MASK)

#define _SHR_GPORT_SCHEDULER_NODE_SET(_gport, _level, _node)		\
        ((_gport) = (_SHR_GPORT_TYPE_SCHEDULER << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_level) & _SHR_GPORT_SCHEDULER_LEVEL_MASK) << _SHR_GPORT_SCHEDULER_LEVEL_SHIFT) | \
         (((_node) & _SHR_GPORT_SCHEDULER_NODE_MASK) << _SHR_GPORT_SCHEDULER_NODE_SHIFT))

#define _SHR_GPORT_SCHEDULER_LEVEL_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_SCHEDULER_LEVEL_SHIFT) & _SHR_GPORT_SCHEDULER_LEVEL_MASK)

#define _SHR_GPORT_SCHEDULER_NODE_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_SCHEDULER_NODE_SHIFT) & _SHR_GPORT_SCHEDULER_NODE_MASK)

#define _SHR_GPORT_IS_DEVPORT(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_DEVPORT)

#define _SHR_GPORT_DEVPORT(_device, _port)                       \
        ((_SHR_GPORT_TYPE_DEVPORT   << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_device) & _SHR_GPORT_DEVID_MASK)  << _SHR_GPORT_DEVID_SHIFT)  | \
         (((_port) & _SHR_GPORT_PORT_MASK)     << _SHR_GPORT_PORT_SHIFT))

#define _SHR_GPORT_DEVPORT_SET(_gport, _device, _port)                       \
        ((_gport) = _SHR_GPORT_DEVPORT(_device, _port))

#define _SHR_GPORT_DEVPORT_DEVID_GET(_gport)    \
        (((_gport) >> _SHR_GPORT_DEVID_SHIFT) & _SHR_GPORT_DEVID_MASK)

#define _SHR_GPORT_DEVPORT_PORT_GET(_gport)     \
        (((_gport) >> _SHR_GPORT_PORT_SHIFT) & _SHR_GPORT_PORT_MASK)

#define _SHR_GPORT_BLACK_HOLE    \
        (_SHR_GPORT_TYPE_BLACK_HOLE << _SHR_GPORT_TYPE_SHIFT)

#define _SHR_GPORT_IS_BLACK_HOLE(_gport)  ((_gport) == _SHR_GPORT_BLACK_HOLE)

#define _SHR_GPORT_LOCAL_CPU    \
        (_SHR_GPORT_TYPE_LOCAL_CPU << _SHR_GPORT_TYPE_SHIFT)

#define _SHR_GPORT_IS_LOCAL_CPU(_gport)   ((_gport) == _SHR_GPORT_LOCAL_CPU)

#define _SHR_GPORT_IS_UCAST_QUEUE_GROUP(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_UCAST_QUEUE_GROUP)

#define _SHR_GPORT_UCAST_QUEUE_GROUP_SET(_gport, _qid)                           \
            _SHR_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(_gport,                      \
            _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_MASK, \
             _qid)

#define _SHR_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(_gport, _sysport_id, _qid)                       \
        ((_gport) = (_SHR_GPORT_TYPE_UCAST_QUEUE_GROUP << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_sysport_id) & _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_MASK)  << _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_SHIFT)  | \
	 (((_qid) & _SHR_GPORT_UCAST_QUEUE_GROUP_QID_MASK)     << _SHR_GPORT_UCAST_QUEUE_GROUP_QID_SHIFT))

#define _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(_gport)                       \
        (((_gport) >> _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_SHIFT) & _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_MASK)


#define _SHR_GPORT_UCAST_QUEUE_GROUP_QID_GET(_gport)                       \
        (((_gport) >> _SHR_GPORT_UCAST_QUEUE_GROUP_QID_SHIFT) & _SHR_GPORT_UCAST_QUEUE_GROUP_QID_MASK)

#define _SHR_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_UCAST_SUBSCRIBER_QUEUE_GROUP)

#define _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(_gport, _qid)           	        \
        ((_gport) = (_SHR_GPORT_TYPE_UCAST_SUBSCRIBER_QUEUE_GROUP << _SHR_GPORT_TYPE_SHIFT)   | \
	 (((_qid) & _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) << _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT))

#define _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(_gport)                       \
        (((_gport) >> _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT) & \
	              _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK)

#define _SHR_GPORT_IS_MCAST(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MCAST)

#define _SHR_GPORT_MCAST_SET(_gport, _mcast_id)                              \
        ((_gport) = (_SHR_GPORT_TYPE_MCAST      << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_mcast_id) & _SHR_GPORT_MCAST_MASK) << _SHR_GPORT_MCAST_SHIFT))

#define _SHR_GPORT_MCAST_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_MCAST_SHIFT) & _SHR_GPORT_MCAST_MASK)

#define _SHR_GPORT_MCAST_DS_ID_SET(_gport, _ds_id) \
        ((_gport) = (_SHR_GPORT_TYPE_MCAST      << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_ds_id) & _SHR_GPORT_MCAST_DS_ID_MASK) << _SHR_GPORT_MCAST_DS_ID_SHIFT))

#define _SHR_GPORT_MCAST_DS_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_MCAST_DS_ID_SHIFT) & _SHR_GPORT_MCAST_DS_ID_MASK)

#define _SHR_GPORT_IS_MCAST_QUEUE_GROUP(_gport) \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MCAST_QUEUE_GROUP)

#define _SHR_GPORT_MCAST_QUEUE_GROUP_SET(_gport, _qid)            \
        _SHR_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(_gport,           \
        _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_MASK,              \
         _qid)

#define _SHR_GPORT_MCAST_QUEUE_GROUP_GET(_gport)   \
        _SHR_GPORT_MCAST_QUEUE_GROUP_QID_GET(_gport) 

#define _SHR_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(_gport, _sysport_id, _qid)                       \
        ((_gport) = (_SHR_GPORT_TYPE_MCAST_QUEUE_GROUP << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_sysport_id) & _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_MASK)  << _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_SHIFT)  | \
         (((_qid) & _SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK)     << _SHR_GPORT_MCAST_QUEUE_GROUP_QID_SHIFT))

#define _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_GET(_gport)                       \
        (((_gport) >> _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_SHIFT) & _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_MASK)

#define _SHR_GPORT_MCAST_QUEUE_GROUP_QID_GET(_gport)                       \
         (((_gport) >> _SHR_GPORT_MCAST_QUEUE_GROUP_QID_SHIFT) & _SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK)

#define _SHR_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MCAST_SUBSCRIBER_QUEUE_GROUP)

#define _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(_gport, _qid)           	        \
        ((_gport) = (_SHR_GPORT_TYPE_MCAST_SUBSCRIBER_QUEUE_GROUP << _SHR_GPORT_TYPE_SHIFT)   | \
	 (((_qid) & _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) << _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT))

#define _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(_gport)                       \
        (((_gport) >> _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SHIFT) & \
	              _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK)

#define _SHR_GPORT_IS_SPECIAL(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_SPECIAL)

#define _SHR_GPORT_SPECIAL(_value)                              \
        ((_SHR_GPORT_TYPE_SPECIAL << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_value) & _SHR_GPORT_SPECIAL_MASK) << _SHR_GPORT_SPECIAL_SHIFT))

#define _SHR_GPORT_SPECIAL_SET(_gport, _value)  \
        ((_gport) = _SHR_GPORT_SPECIAL(_value))

#define _SHR_GPORT_SPECIAL_GET(_gport)    \
        (((_gport) >> _SHR_GPORT_SPECIAL_SHIFT) & _SHR_GPORT_SPECIAL_MASK)

#define _SHR_GPORT_IS_MIRROR(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MIRROR)

#define _SHR_GPORT_MIRROR_SET(_gport, _value)                               \
        ((_gport) = (_SHR_GPORT_TYPE_MIRROR << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_value) & _SHR_GPORT_MIRROR_MASK) << _SHR_GPORT_MIRROR_SHIFT))

#define _SHR_GPORT_MIRROR_GET(_gport)    \
        (((_gport) >> _SHR_GPORT_MIRROR_SHIFT) & _SHR_GPORT_MIRROR_MASK)

#define _SHR_GPORT_IS_MIM_PORT(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_MIM_PORT)

#define _SHR_GPORT_MIM_PORT_ID_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_MIM_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_MIM_PORT_MASK) << _SHR_GPORT_MIM_PORT_SHIFT))

#define _SHR_GPORT_MIM_PORT_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_MIM_PORT_SHIFT) & _SHR_GPORT_MIM_PORT_MASK)

#define _SHR_GPORT_IS_VLAN_PORT(_gport) \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_VLAN_PORT)

#define _SHR_GPORT_VLAN_PORT_ID_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_VLAN_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_VLAN_PORT_MASK) << _SHR_GPORT_VLAN_PORT_SHIFT))

#define _SHR_GPORT_VLAN_PORT_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_VLAN_PORT_SHIFT) & _SHR_GPORT_VLAN_PORT_MASK)

#define _SHR_GPORT_IS_WLAN_PORT(_gport) \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_WLAN_PORT)

#define _SHR_GPORT_WLAN_PORT_ID_SET(_gport, _id)                            \
        ((_gport) = (_SHR_GPORT_TYPE_WLAN_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_id) & _SHR_GPORT_WLAN_PORT_MASK) << _SHR_GPORT_WLAN_PORT_SHIFT))

#define _SHR_GPORT_WLAN_PORT_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_WLAN_PORT_SHIFT) & _SHR_GPORT_WLAN_PORT_MASK)

#define _SHR_GPORT_IS_TRILL_PORT(_gport) \
			(((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_TRILL_PORT)
	
#define _SHR_GPORT_TRILL_PORT_ID_SET(_gport, _id)                            \
			((_gport) = (_SHR_GPORT_TYPE_TRILL_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
			 (((_id) & _SHR_GPORT_TRILL_PORT_MASK) << _SHR_GPORT_TRILL_PORT_SHIFT))
	
#define _SHR_GPORT_TRILL_PORT_ID_GET(_gport)   \
			(((_gport) >> _SHR_GPORT_TRILL_PORT_SHIFT) & _SHR_GPORT_TRILL_PORT_MASK)

#define _SHR_GPORT_IS_NIV_PORT(_gport) \
			(((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_NIV_PORT)
	
#define _SHR_GPORT_NIV_PORT_ID_SET(_gport, _id)                            \
			((_gport) = (_SHR_GPORT_TYPE_NIV_PORT << _SHR_GPORT_TYPE_SHIFT)  | \
			 (((_id) & _SHR_GPORT_NIV_PORT_MASK) << _SHR_GPORT_NIV_PORT_SHIFT))
	
#define _SHR_GPORT_NIV_PORT_ID_GET(_gport)   \
			(((_gport) >> _SHR_GPORT_NIV_PORT_SHIFT) & _SHR_GPORT_NIV_PORT_MASK)

#define _SHR_GPORT_IS_TUNNEL(_gport)    \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_TUNNEL)

#define _SHR_GPORT_TUNNEL_ID_SET(_gport, _tunnel_id)                       \
        ((_gport) = (_SHR_GPORT_TYPE_TUNNEL   << _SHR_GPORT_TYPE_SHIFT)  | \
         (((_tunnel_id) & _SHR_GPORT_TUNNEL_MASK) << _SHR_GPORT_TUNNEL_SHIFT))

#define _SHR_GPORT_TUNNEL_ID_GET(_gport)   \
        (((_gport) >> _SHR_GPORT_TUNNEL_SHIFT) & _SHR_GPORT_TUNNEL_MASK)

#define _SHR_GPORT_IS_CHILD(_gport)   \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_CHILD)

#define _SHR_GPORT_CHILD_SET(_child_gport,_child_modid, _child_port) \
        ((_child_gport) = (_SHR_GPORT_TYPE_CHILD << _SHR_GPORT_TYPE_SHIFT)  |	\
	 (((_child_modid) & _SHR_GPORT_CHILD_MODID_MASK) << _SHR_GPORT_CHILD_MODID_SHIFT) | \
	 (((_child_port) & _SHR_GPORT_CHILD_PORT_MASK) << _SHR_GPORT_CHILD_PORT_SHIFT))        

#define _SHR_GPORT_CHILD_MODID_GET(_gport)  \
        (((_gport) >> _SHR_GPORT_CHILD_MODID_SHIFT) & _SHR_GPORT_CHILD_MODID_MASK)

#define _SHR_GPORT_CHILD_PORT_GET(_gport)  \
        (((_gport) >> _SHR_GPORT_CHILD_PORT_SHIFT) & _SHR_GPORT_CHILD_PORT_MASK)

#define _SHR_GPORT_IS_EGRESS_GROUP(_gport)  \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_EGRESS_GROUP)

#define _SHR_GPORT_EGRESS_GROUP_SET(_egress_group_gport, _child_modid, _egress_child) \
        ((_egress_group_gport) = (_SHR_GPORT_TYPE_EGRESS_GROUP << _SHR_GPORT_TYPE_SHIFT)  |	\
	 (((_child_modid) & _SHR_GPORT_EGRESS_CHILD_MODID_MASK) << _SHR_GPORT_EGRESS_CHILD_MODID_SHIFT) | \
	 (((_egress_child) & _SHR_GPORT_EGRESS_CHILD_PORT_MASK) << (_SHR_GPORT_EGRESS_GROUP_SHIFT)))

#define _SHR_GPORT_EGRESS_GROUP_MODID_GET(_egress_group_gport)  \
        (((_egress_group_gport) >> _SHR_GPORT_EGRESS_GROUP_MODID_SHIFT) & _SHR_GPORT_EGRESS_GROUP_MODID_MASK)

#define _SHR_GPORT_EGRESS_GROUP_GET(_egress_group_gport)  \
        (((_egress_group_gport) >> _SHR_GPORT_EGRESS_GROUP_SHIFT) & _SHR_GPORT_EGRESS_GROUP_MASK)

#define _SHR_GPORT_IS_EGRESS_CHILD(_gport)  \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_EGRESS_CHILD)

#define _SHR_GPORT_EGRESS_CHILD_SET(_egress_child_gport,_child_modid, _child_port) \
        ((_egress_child_gport) = (_SHR_GPORT_TYPE_EGRESS_CHILD << _SHR_GPORT_TYPE_SHIFT)  |	\
	 (((_child_modid) & _SHR_GPORT_EGRESS_CHILD_MODID_MASK) << _SHR_GPORT_EGRESS_CHILD_MODID_SHIFT) | \
	 (((_child_port) & _SHR_GPORT_EGRESS_CHILD_PORT_MASK) << _SHR_GPORT_EGRESS_CHILD_PORT_SHIFT))        

#define _SHR_GPORT_EGRESS_CHILD_MODID_GET(_egress_child_gport)  \
        (((_egress_child_gport) >> _SHR_GPORT_EGRESS_CHILD_MODID_SHIFT) & _SHR_GPORT_EGRESS_CHILD_MODID_MASK)

#define _SHR_GPORT_EGRESS_CHILD_PORT_GET(_egress_child_gport)  \
        (((_egress_child_gport) >> _SHR_GPORT_EGRESS_CHILD_PORT_SHIFT) & _SHR_GPORT_EGRESS_CHILD_PORT_MASK)

#define _SHR_GPORT_IS_EGRESS_MODPORT(_gport)  \
        (((_gport) >> _SHR_GPORT_TYPE_SHIFT) == _SHR_GPORT_TYPE_EGRESS_MODPORT)

#define _SHR_GPORT_EGRESS_MODPORT_SET(_gport, _module, _port)  \
        ((_gport) = (_SHR_GPORT_TYPE_EGRESS_MODPORT   << _SHR_GPORT_TYPE_SHIFT)   | \
         (((_module) & _SHR_GPORT_EGRESS_MODPORT_MODID_MASK)  << _SHR_GPORT_EGRESS_MODPORT_MODID_SHIFT)  | \
         (((_port) & _SHR_GPORT_EGRESS_MODPORT_PORT_MASK)     << _SHR_GPORT_EGRESS_MODPORT_PORT_SHIFT))

#define _SHR_GPORT_EGRESS_MODPORT_MODID_GET(_gport)  \
        (((_gport) >> _SHR_GPORT_EGRESS_MODPORT_MODID_SHIFT) & _SHR_GPORT_EGRESS_MODPORT_MODID_MASK)

#define _SHR_GPORT_EGRESS_MODPORT_PORT_GET(_gport)  \
        (((_gport) >> _SHR_GPORT_EGRESS_MODPORT_PORT_SHIFT) & _SHR_GPORT_EGRESS_MODPORT_PORT_MASK)

#endif  /* !_SHR_GPORT_H */

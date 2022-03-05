/*
 * $Id: port.h,v 1.41 Broadcom SDK $
 * 
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
 * DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __BCMX_PORT_H__
#define __BCMX_PORT_H__

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>
#include <bcm/port.h>

/* BCMX LPORT Flags */
#define BCMX_PORT_LP_ALL        (1 << 0)   /* 0x00000001 */
#define BCMX_PORT_LP_FE         (1 << 1)   /* 0x00000002 */
#define BCMX_PORT_LP_GE         (1 << 2)   /* 0x00000004 */
#define BCMX_PORT_LP_XE         (1 << 3)   /* 0x00000008 */
#define BCMX_PORT_LP_HG         (1 << 4)   /* 0x00000010 */
#define BCMX_PORT_LP_E          (1 << 5)   /* 0x00000020 */
#define BCMX_PORT_LP_PORT       (1 << 6)   /* 0x00000040 */
#define BCMX_PORT_LP_CPU        (1 << 7)   /* 0x00000080 */
#define BCMX_PORT_LP_STACK_INT  (1 << 8)   /* 0x00000100 */
#define BCMX_PORT_LP_STACK_EXT  (1 << 9)   /* 0x00000200 */

/* Port Configuration structure. */
typedef struct bcmx_port_config_s {
    bcmx_lplist_t fe;           /* List of FE ports. */
    bcmx_lplist_t ge;           /* List of GE ports. */
    bcmx_lplist_t xe;           /* List of 10-Gig ports. */
    bcmx_lplist_t e;            /* List of Ethernet ports. */
    bcmx_lplist_t hg;           /* List of Higig ports. */
    bcmx_lplist_t port;         /* List of all front panel ports. */
    bcmx_lplist_t cpu;          /* List of CPU ports. */
    bcmx_lplist_t all;          /* List of all ports. */
    bcmx_lplist_t stack_int;    /* Deprecated - unused. */
    bcmx_lplist_t stack_ext;    /* List of Stack ports. */
} bcmx_port_config_t;

/* Port Ability structure */
typedef bcm_port_ability_t bcmx_port_ability_t;

/* Port Encapsulation Configuration structure */
typedef bcm_port_encap_config_t bcmx_port_encap_config_t;

/* Port Congestion Configuration structure */
typedef bcm_port_congestion_config_t bcmx_port_congestion_config_t;

/* Port Match Attribute structure */
typedef bcm_port_match_info_t bcmx_port_match_info_t;

/* Port Timesync Configuration structure */
typedef bcm_port_timesync_config_t bcmx_port_timesync_config_t;

/* Initialize a Port Configuration structure. */
extern void bcmx_port_config_t_init(
    bcmx_port_config_t *pconfig);

extern void bcmx_port_ability_t_init(
    bcmx_port_ability_t *ability);

/* Initialize a Port Encapsulation Configuration structure. */
extern void bcmx_port_encap_config_t_init(
    bcmx_port_encap_config_t *encap_config);

/* Initialize a Port Congestion Configuration structure. */
extern void bcmx_port_congestion_config_t_init(
    bcmx_port_congestion_config_t *config);

/* Initialize the port match structure. */
extern void bcmx_port_match_info_t_init(
    bcmx_port_match_info_t *port_match_info);

extern void bcmx_port_timesync_config_t_init(
    bcmx_port_timesync_config_t *conf);

/* 
 * Retrieved the port configuration for the specified device or for all
 * ports known to bcmx.
 */
extern int bcmx_port_config_get(
    bcmx_port_config_t *config);

/* Enable or disable a port. */
extern int bcmx_port_enable_set(
    bcmx_lport_t port, 
    int enable);

/* Enable or disable a port. */
extern int bcmx_port_enable_get(
    bcmx_lport_t port, 
    int *enable);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_advert_set(
    bcmx_lport_t port, 
    bcm_port_abil_t ability_mask);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_ability_advert_set(
    bcmx_lport_t port, 
    bcmx_port_ability_t *ability_mask);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_advert_get(
    bcmx_lport_t port, 
    bcm_port_abil_t *ability_mask);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_ability_advert_get(
    bcmx_lport_t port, 
    bcmx_port_ability_t *ability_mask);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_advert_remote_get(
    bcmx_lport_t port, 
    bcm_port_abil_t *ability_mask);

/* Set or retrieve autonegotiation settings for a port. */
extern int bcmx_port_ability_remote_get(
    bcmx_lport_t port, 
    bcmx_port_ability_t *ability_mask);

/* 
 * Retrieve the valid operating modes of a port including speed and
 * duplex.
 */
extern int bcmx_port_ability_get(
    bcmx_lport_t port, 
    bcm_port_abil_t *local_ability_mask);

/* 
 * Retrieve the valid operating modes of a port including speed and
 * duplex.
 */
extern int bcmx_port_ability_local_get(
    bcmx_lport_t port, 
    bcmx_port_ability_t *local_ability_mask);

/* Get or set the default vlan for packets that ingress untagged. */
extern int bcmx_port_untagged_vlan_set(
    bcmx_lport_t port, 
    bcm_vlan_t vid);

/* Get or set the default vlan for packets that ingress untagged. */
extern int bcmx_port_untagged_vlan_get(
    bcmx_lport_t port, 
    bcm_vlan_t *vid_ptr);

/* Get or set the default priority for packets that ingress untagged. */
extern int bcmx_port_untagged_priority_set(
    bcmx_lport_t port, 
    int priority);

/* Get or set the default priority for packets that ingress untagged. */
extern int bcmx_port_untagged_priority_get(
    bcmx_lport_t port, 
    int *priority);

/* Control mapping of Differentiated Services Code Points (DSCP). */
extern int bcmx_port_dscp_map_mode_set(
    bcmx_lport_t port, 
    int mode);

/* Control mapping of Differentiated Services Code Points (DSCP). */
extern int bcmx_port_dscp_map_mode_get(
    bcmx_lport_t port, 
    int *mode);

/* Control mapping of Differentiated Services Code Points (DSCP). */
extern int bcmx_port_dscp_map_set(
    bcmx_lport_t port, 
    int srccp, 
    int mapcp, 
    int prio);

/* Control mapping of Differentiated Services Code Points (DSCP). */
extern int bcmx_port_dscp_map_get(
    bcmx_lport_t port, 
    int srccp, 
    int *mapcp, 
    int *prio);

/* Get or set the current linkscan mode for the specified port. */
extern int bcmx_port_linkscan_set(
    bcmx_lport_t port, 
    int linkscan);

/* Get or set the current linkscan mode for the specified port. */
extern int bcmx_port_linkscan_get(
    bcmx_lport_t port, 
    int *linkscan);

/* 
 * Configure or retrieve the current autonegotiation settings for a port,
 * or restart autonegotiation if already enabled.
 */
extern int bcmx_port_autoneg_set(
    bcmx_lport_t port, 
    int autoneg);

/* 
 * Configure or retrieve the current autonegotiation settings for a port,
 * or restart autonegotiation if already enabled.
 */
extern int bcmx_port_autoneg_get(
    bcmx_lport_t port, 
    int *autoneg);

/* Get or set the current operating speed of a port. */
extern int bcmx_port_speed_max(
    bcmx_lport_t port, 
    int *speed);

/* Get or set the current operating speed of a port. */
extern int bcmx_port_speed_set(
    bcmx_lport_t port, 
    int speed);

/* Get or set the current operating speed of a port. */
extern int bcmx_port_speed_get(
    bcmx_lport_t port, 
    int *speed);

/* Set or get the Master mode on the PHY attached to the specified port. */
extern int bcmx_port_master_set(
    bcmx_lport_t port, 
    int ms);

/* Set or get the Master mode on the PHY attached to the specified port. */
extern int bcmx_port_master_get(
    bcmx_lport_t port, 
    int *ms);

/* 
 * Configure the physical interface between the MAC and the PHY for the
 * specified port.
 */
extern int bcmx_port_interface_set(
    bcmx_lport_t port, 
    bcm_port_if_t intf);

/* 
 * Configure the physical interface between the MAC and the PHY for the
 * specified port.
 */
extern int bcmx_port_interface_get(
    bcmx_lport_t port, 
    bcm_port_if_t *intf);

/* Get or set the current duplex mode of a port. */
extern int bcmx_port_duplex_set(
    bcmx_lport_t port, 
    int duplex);

/* Get or set the current duplex mode of a port. */
extern int bcmx_port_duplex_get(
    bcmx_lport_t port, 
    int *duplex);

/* 
 * Enable or disable transmission of pause frames and honoring received
 * pause frames on a port.
 */
extern int bcmx_port_pause_set(
    bcmx_lport_t port, 
    int pause_tx, 
    int pause_rx);

/* 
 * Enable or disable transmission of pause frames and honoring received
 * pause frames on a port.
 */
extern int bcmx_port_pause_get(
    bcmx_lport_t port, 
    int *pause_tx, 
    int *pause_rx);

/* 
 * Get or set the source MAC address transmitted in MAC control pause
 * frames.
 */
extern int bcmx_port_pause_addr_set(
    bcmx_lport_t port, 
    bcm_mac_t mac);

/* 
 * Get or set the source MAC address transmitted in MAC control pause
 * frames.
 */
extern int bcmx_port_pause_addr_get(
    bcmx_lport_t port, 
    bcm_mac_t mac);

/* Configure or retrieve asymmetric pause setting for a port. */
extern int bcmx_port_pause_sym_set(
    bcmx_lport_t port, 
    int pause);

/* Configure or retrieve asymmetric pause setting for a port. */
extern int bcmx_port_pause_sym_get(
    bcmx_lport_t port, 
    int *pause);

/* Process a link change event and perform required device configuration. */
extern int bcmx_port_update(
    bcmx_lport_t port, 
    int link);

/* Set or retrieve the current maximum packet size permitted on a port. */
extern int bcmx_port_frame_max_set(
    bcmx_lport_t port, 
    int size);

/* Set or retrieve the current maximum packet size permitted on a port. */
extern int bcmx_port_frame_max_get(
    bcmx_lport_t port, 
    int *size);

/* 
 * Set or retrieve the current maximum L3 packet size permitted on a
 * port.
 */
extern int bcmx_port_l3_mtu_set(
    bcmx_lport_t port, 
    int size);

/* 
 * Set or retrieve the current maximum L3 packet size permitted on a
 * port.
 */
extern int bcmx_port_l3_mtu_get(
    bcmx_lport_t port, 
    int *size);

/* Get or set half duplex flow control (jam) for a port. */
extern int bcmx_port_jam_set(
    bcmx_lport_t port, 
    int enable);

/* Get or set half duplex flow control (jam) for a port. */
extern int bcmx_port_jam_get(
    bcmx_lport_t port, 
    int *enable);

/* Get or set the interframe gap settings for the specified port. */
extern int bcmx_port_ifg_set(
    bcmx_lport_t port, 
    int speed, 
    bcm_port_duplex_t duplex, 
    int bit_times);

/* Get or set the interframe gap settings for the specified port. */
extern int bcmx_port_ifg_get(
    bcmx_lport_t port, 
    int speed, 
    bcm_port_duplex_t duplex, 
    int *bit_times);

/* Set/Get force vlan attribute of a port. */
extern int bcmx_port_force_vlan_set(
    bcmx_lport_t port, 
    bcm_vlan_t vlan, 
    int pkt_prio, 
    uint32 flags);

/* Set/Get force vlan attribute of a port. */
extern int bcmx_port_force_vlan_get(
    bcmx_lport_t port, 
    bcm_vlan_t *vlan, 
    int *pkt_prio, 
    uint32 *flags);

/* Write phy registers associated with a port. */
extern int bcmx_port_phy_set(
    bcmx_lport_t port, 
    uint32 flags, 
    uint32 phy_reg_addr, 
    uint32 phy_data);

/* Read phy registers associated with a port. */
extern int bcmx_port_phy_get(
    bcmx_lport_t port, 
    uint32 flags, 
    uint32 phy_reg_addr, 
    uint32 *phy_data);

/* Modify phy registers associated with a port. */
extern int bcmx_port_phy_modify(
    bcmx_lport_t port, 
    uint32 flags, 
    uint32 phy_reg_addr, 
    uint32 phy_data, 
    uint32 phy_mask);

/* Get or set the Auto MDIX mode for a port. */
extern int bcmx_port_mdix_set(
    bcmx_lport_t port, 
    bcm_port_mdix_t mode);

/* Get or set the Auto MDIX mode for a port. */
extern int bcmx_port_mdix_get(
    bcmx_lport_t port, 
    bcm_port_mdix_t *mode);

/* Get or set the Auto MDIX mode for a port. */
extern int bcmx_port_mdix_status_get(
    bcmx_lport_t port, 
    bcm_port_mdix_status_t *status);

/* Get or set medium preferences for combination ports. */
extern int bcmx_port_medium_config_set(
    bcmx_lport_t port, 
    bcm_port_medium_t medium, 
    bcm_phy_config_t *config);

/* Get or set medium preferences for combination ports. */
extern int bcmx_port_medium_config_get(
    bcmx_lport_t port, 
    bcm_port_medium_t medium, 
    bcm_phy_config_t *config);

/* Get or set medium preferences for combination ports. */
extern int bcmx_port_medium_get(
    bcmx_lport_t port, 
    bcm_port_medium_t *medium);

/* Set the current loopback mode of a port. */
extern int bcmx_port_loopback_set(
    bcmx_lport_t port, 
    int loopback);

/* Retrieve the current loopback mode of a port. */
extern int bcmx_port_loopback_get(
    bcmx_lport_t port, 
    int *loopback);

/* 
 * Set the spanning tree state for a port (single instance spanning tree
 * only).
 */
extern int bcmx_port_stp_set(
    bcmx_lport_t port, 
    int state);

/* 
 * Get the spanning tree state for a port (single instance spanning tree
 * only).
 */
extern int bcmx_port_stp_get(
    bcmx_lport_t port, 
    int *state);

/* 
 * Configure port discard mode for packets ingressing tagged and
 * untagged.
 */
extern int bcmx_port_discard_set(
    bcmx_lport_t port, 
    int mode);

/* Get the port discard mode for packets ingressing tagged and untagged. */
extern int bcmx_port_discard_get(
    bcmx_lport_t port, 
    int *mode);

/* Control the hardware and software learning support on a port. */
extern int bcmx_port_learn_set(
    bcmx_lport_t port, 
    uint32 flags);

/* Get the hardware and software learning support on a port. */
extern int bcmx_port_learn_get(
    bcmx_lport_t port, 
    uint32 *flags);

/* Control the hardware and software learning support on a port. */
extern int bcmx_port_learn_modify(
    bcmx_lport_t port, 
    uint32 add, 
    uint32 remove);

/* Retrieve the current link status of a port. */
extern int bcmx_port_link_status_get(
    bcmx_lport_t port, 
    int *status);

/* 
 * Clear failed link status from a port which has undergone LAG failover.
 *  The port is moved to down status.  The application is responsible for
 * removing the port from all trunk memberships before calling this
 * function.
 */
extern int bcmx_port_link_failed_clear(
    bcm_gport_t port);

/* 
 * Set current behavior of tagged packets arriving on a port not a member
 * of the specified VLAN.
 */
extern int bcmx_port_ifilter_set(
    bcmx_lport_t port, 
    int mode);

/* 
 * Retrieve current behavior of tagged packets arriving on a port not a
 * member of the specified VLAN.
 */
extern int bcmx_port_ifilter_get(
    bcmx_lport_t port, 
    int *mode);

/* 
 * Set current behavior of tagged packets arriving/leaving on a port not
 * a member of the specified VLAN.
 */
extern int bcmx_port_vlan_member_set(
    bcmx_lport_t port, 
    uint32 flags);

/* 
 * Retrieve current behavior of tagged packets arriving/leaving on a port
 * not a member of the specified VLAN.
 */
extern int bcmx_port_vlan_member_get(
    bcmx_lport_t port, 
    uint32 *flags);

/* Enable or disable BPDU processing on a port. */
extern int bcmx_port_bpdu_enable_set(
    bcmx_lport_t port, 
    int enable);

/* Enable or disable BPDU processing on a port. */
extern int bcmx_port_bpdu_enable_get(
    bcmx_lport_t port, 
    int *enable);

/* Enable or disable l3 switching on an ingress port. */
extern int bcmx_port_l3_enable_set(
    bcmx_lport_t port, 
    int enable);

/* Enable or disable l3 switching on an ingress port. */
extern int bcmx_port_l3_enable_get(
    bcmx_lport_t port, 
    int *enable);

extern int bcmx_port_tgid_set(
    bcmx_lport_t port, 
    int tgid, 
    int psc);

extern int bcmx_port_tgid_get(
    bcmx_lport_t port, 
    int *tgid, 
    int *psc);

/* Set or retrieve the current unknown multicast behavior. */
extern int bcmx_port_pfm_set(
    bcmx_lport_t port, 
    int mode);

/* Set or retrieve the current unknown multicast behavior. */
extern int bcmx_port_pfm_get(
    bcmx_lport_t port, 
    int *mode);

/* Set the encapsulation of the given port. */
extern int bcmx_port_encap_config_set(
    bcm_gport_t gport, 
    bcmx_port_encap_config_t *encap_config);

/* Get the encapsulation of the given port. */
extern int bcmx_port_encap_config_get(
    bcm_gport_t gport, 
    bcmx_port_encap_config_t *encap_config);

/* Set or get the link level encapsulation mode. */
extern int bcmx_port_encap_set(
    bcmx_lport_t port, 
    int mode);

/* Set or get the link level encapsulation mode. */
extern int bcmx_port_encap_get(
    bcmx_lport_t port, 
    int *mode);

/* 
 * Get the current count of cells or packets queued on a port for
 * transmission.
 */
extern int bcmx_port_queued_count_get(
    bcmx_lport_t port, 
    uint32 *count);

/* Add or delete a protocol based vlan. */
extern int bcmx_port_protocol_vlan_add(
    bcmx_lport_t port, 
    bcm_port_frametype_t frame, 
    bcm_port_ethertype_t ether, 
    bcm_vlan_t vid);

/* Add or delete a protocol based vlan. */
extern int bcmx_port_protocol_vlan_delete(
    bcmx_lport_t port, 
    bcm_port_frametype_t frame, 
    bcm_port_ethertype_t ether);

/* Add or delete a protocol based vlan. */
extern int bcmx_port_protocol_vlan_delete_all(
    bcmx_lport_t port);

/* 
 * Configure ports to block or allow packets from a given ingress port
 * for egressing.
 */
extern int bcmx_port_egress_set(
    bcmx_lport_t port, 
    int modid, 
    bcmx_lplist_t lplist);

/* 
 * Get list of ports to block or allow packets from a given ingress port
 * for egressing.
 */
extern int bcmx_port_egress_get(
    bcmx_lport_t port, 
    int modid, 
    bcmx_lplist_t *lplist);

/* 
 * Configure which egress ports an ingress port is permitted to forward
 * packets to.
 */
extern int bcmx_port_modid_enable_set(
    bcmx_lport_t port, 
    int modid, 
    int enable);

/* 
 * Configure which egress ports an ingress port is permitted to forward
 * packets to.
 */
extern int bcmx_port_modid_enable_get(
    bcmx_lport_t port, 
    int modid, 
    int *enable);

/* Selectively block flooding traffic. */
extern int bcmx_port_flood_block_set(
    bcmx_lport_t ingress_port, 
    bcmx_lport_t egress_port, 
    uint32 flags);

/* Selectively block flooding traffic. */
extern int bcmx_port_flood_block_get(
    bcmx_lport_t ingress_port, 
    bcmx_lport_t egress_port, 
    uint32 *flags);

/* Configure a port for egress rate shaping. */
extern int bcmx_port_rate_egress_set(
    bcmx_lport_t port, 
    uint32 kbits_sec, 
    uint32 kbits_burst);

/* Get a port's egress rate shaping parameters. */
extern int bcmx_port_rate_egress_get(
    bcmx_lport_t port, 
    uint32 *kbits_sec, 
    uint32 *kbits_burst);

/* Configure a port for ingress rate policing. */
extern int bcmx_port_rate_ingress_set(
    bcmx_lport_t port, 
    uint32 kbits_sec, 
    uint32 kbits_burst);

/* Configure a port for ingress rate policing. */
extern int bcmx_port_rate_ingress_get(
    bcmx_lport_t port, 
    uint32 *kbits_sec, 
    uint32 *kbits_burst);

/* 
 * Configure a port's ingress rate limiting pause frame control
 * parameters.
 */
extern int bcmx_port_rate_pause_set(
    bcmx_lport_t port, 
    uint32 kbits_pause, 
    uint32 kbits_resume);

/* 
 * Configure a port's ingress rate limiting pause frame control
 * parameters.
 */
extern int bcmx_port_rate_pause_get(
    bcmx_lport_t port, 
    uint32 *kbits_pause, 
    uint32 *kbits_resume);

/* Control the sampling of packets ingressing or egressing a port. */
extern int bcmx_port_sample_rate_set(
    bcm_port_t port, 
    int ingress_rate, 
    int egress_rate);

/* Control the sampling of packets ingressing or egressing a port. */
extern int bcmx_port_sample_rate_get(
    bcm_port_t port, 
    int *ingress_rate, 
    int *egress_rate);

/* Set or retrieve the current double tagging mode for a port. */
extern int bcmx_port_dtag_mode_set(
    bcmx_lport_t port, 
    int mode);

/* Set or retrieve the current double tagging mode for a port. */
extern int bcmx_port_dtag_mode_get(
    bcmx_lport_t port, 
    int *mode);

/* Set the default tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_tpid_set(
    bcmx_lport_t port, 
    uint16 tpid);

/* Set the default tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_tpid_get(
    bcmx_lport_t port, 
    uint16 *tpid);

/* Add an allowed outer tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_tpid_add(
    bcmx_lport_t port, 
    uint16 tpid, 
    int color_select);

/* Delete allowed outer tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_tpid_delete(
    bcmx_lport_t port, 
    uint16 tpid);

/* Delete allowed outer tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_tpid_delete_all(
    bcmx_lport_t port);

/* Set the default tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_inner_tpid_set(
    bcmx_lport_t port, 
    uint16 tpid);

/* Set the default tag protocol ID (TPID) for the specified port. */
extern int bcmx_port_inner_tpid_get(
    bcmx_lport_t port, 
    uint16 *tpid);

/* Perform cable diagnostics on the specified port. */
extern int bcmx_port_cable_diag(
    bcmx_lport_t port, 
    bcm_port_cable_diag_t *status);

/* Get or set the L3 unicast packet modification operations of a port. */
extern int bcmx_port_l3_modify_set(
    bcmx_lport_t port, 
    uint32 flags);

/* Get or set the L3 unicast packet modification operations of a port. */
extern int bcmx_port_l3_modify_get(
    bcmx_lport_t port, 
    uint32 *flags);

/* Get or set the L3 multicast packet modification operations of a port. */
extern int bcmx_port_ipmc_modify_set(
    bcmx_lport_t port, 
    uint32 flags);

/* Get or set the L3 multicast packet modification operations of a port. */
extern int bcmx_port_ipmc_modify_get(
    bcmx_lport_t port, 
    uint32 *flags);

/* Get or set multiple port characteristics. */
extern int bcmx_port_info_get(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get or set multiple port characteristics. */
extern int bcmx_port_info_set(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get or set multiple port characteristics. */
extern int bcmx_port_selective_get(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get or set multiple port characteristics. */
extern int bcmx_port_selective_set(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get or set multiple port characteristics. */
extern int bcmx_port_info_restore(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get or set multiple port characteristics. */
extern int bcmx_port_info_save(
    bcmx_lport_t port, 
    bcm_port_info_t *info);

/* Get link fault type. */
extern int bcmx_port_fault_get(
    bcmx_lport_t port, 
    uint32 *flags);

/* Populate the port list based on flags. */
extern int bcmx_port_lplist_populate(
    bcmx_lplist_t *list, 
    uint32 flags);

/* Set or retrieve color assignment for a given port and priority. */
extern int bcmx_port_priority_color_set(
    bcmx_lport_t port, 
    int prio, 
    bcm_color_t color);

/* Set or retrieve color assignment for a given port and priority. */
extern int bcmx_port_priority_color_get(
    bcmx_lport_t port, 
    int prio, 
    bcm_color_t *color);

/* Set or retrieve color assignment for a given port and CFI. */
extern int bcmx_port_cfi_color_set(
    bcmx_lport_t port, 
    int cfi, 
    bcm_color_t color);

/* Set or retrieve color assignment for a given port and CFI. */
extern int bcmx_port_cfi_color_get(
    bcmx_lport_t port, 
    int cfi, 
    bcm_color_t *color);

/* 
 * Map the incoming packet priority and CFI to internal priority and
 * color.
 */
extern int bcmx_port_vlan_priority_map_set(
    bcmx_lport_t port, 
    int pkt_pri, 
    int cfi, 
    int internal_pri, 
    bcm_color_t color);

/* 
 * Map the incoming packet priority and CFI to internal priority and
 * color.
 */
extern int bcmx_port_vlan_priority_map_get(
    bcmx_lport_t port, 
    int pkt_pri, 
    int cfi, 
    int *internal_pri, 
    bcm_color_t *color);

/* 
 * Map the internal priority and color to outgoing packet priority and
 * CFI.
 */
extern int bcmx_port_vlan_priority_unmap_set(
    bcmx_lport_t port, 
    int internal_pri, 
    bcm_color_t color, 
    int pkt_pri, 
    int cfi);

/* 
 * Map the internal priority and color to outgoing packet priority and
 * CFI.
 */
extern int bcmx_port_vlan_priority_unmap_get(
    bcmx_lport_t port, 
    int internal_pri, 
    bcm_color_t color, 
    int *pkt_pri, 
    int *cfi);

/* Set the inner tag value to be added to the outgoing packet. */
extern int bcmx_port_vlan_inner_tag_set(
    bcmx_lport_t port, 
    uint16 inner_tag);

/* Set the inner tag value to be added to the outgoing packet. */
extern int bcmx_port_vlan_inner_tag_get(
    bcmx_lport_t port, 
    uint16 *inner_tag);

/* 
 * Set or get port classification ID to aggregate a group of ports for
 * further processing such as Vlan translation and field processing.
 */
extern int bcmx_port_class_set(
    bcmx_lport_t port, 
    bcm_port_class_t pclass, 
    uint32 class_id);

/* 
 * Set or get port classification ID to aggregate a group of ports for
 * further processing such as Vlan translation and field processing.
 */
extern int bcmx_port_class_get(
    bcmx_lport_t port, 
    bcm_port_class_t pclass, 
    uint32 *class_id);

/* Get or set various features at the port level. */
extern int bcmx_port_control_set(
    bcmx_lport_t port, 
    bcm_port_control_t type, 
    int value);

/* Get or set various features at the port level. */
extern int bcmx_port_control_get(
    bcmx_lport_t port, 
    bcm_port_control_t type, 
    int *value);

/* 
 * Given a controlling port, this API returns the set of ancillary
 * ports belonging to the group (port block) that can be configured to
 * operate
 * either as a single high-speed port or multiple GE ports. If the input
 * port is
 * not a controlling port BCM_E_PORT error will be returned.
 */
extern int bcmx_port_subsidiary_ports_get(
    bcm_gport_t port, 
    bcmx_lplist_t *lplist);

/* Set congestion mode. */
extern int bcmx_port_congestion_config_set(
    bcm_gport_t port, 
    bcmx_port_congestion_config_t *config);

/* Get congestion mode. */
extern int bcmx_port_congestion_config_get(
    bcm_gport_t port, 
    bcmx_port_congestion_config_t *config);

/* Set/Get PHY specific configurations. */
extern int bcmx_port_phy_control_set(
    bcmx_lport_t port, 
    bcm_port_phy_control_t type, 
    uint32 value);

/* Set/Get PHY specific configurations. */
extern int bcmx_port_phy_control_get(
    bcmx_lport_t port, 
    bcm_port_phy_control_t type, 
    uint32 *value);

/* Get the GPORT ID for the specified physical port. */
extern int bcmx_port_gport_get(
    bcmx_lport_t port, 
    bcm_gport_t *gport);

/* Enable/disable packet and byte counters for the selected gport. */
extern int bcmx_port_stat_enable_set(
    bcm_gport_t port, 
    int enable);

/* Get 64-bit counter value for specified port statistic type. */
extern int bcmx_port_stat_get(
    bcm_gport_t port, 
    bcm_port_stat_t stat, 
    uint64 *val);

/* Get lower 32-bit counter value for specified port statistic type. */
extern int bcmx_port_stat_get32(
    bcm_gport_t port, 
    bcm_port_stat_t stat, 
    uint32 *val);

/* Set 64-bit counter value for specified port statistic type. */
extern int bcmx_port_stat_set(
    bcm_gport_t port, 
    bcm_port_stat_t stat, 
    uint64 val);

/* Set lower 32-bit counter value for specified port statistic type. */
extern int bcmx_port_stat_set32(
    bcm_gport_t port, 
    bcm_port_stat_t stat, 
    uint32 val);

/* Get 64-bit counter value for multiple port statistic types. */
extern int bcmx_port_stat_multi_get(
    bcm_gport_t port, 
    int nstat, 
    bcm_port_stat_t *stat_arr, 
    uint64 *value_arr);

/* Get lower 32-bit counter value for multiple port statistic types. */
extern int bcmx_port_stat_multi_get32(
    bcm_gport_t port, 
    int nstat, 
    bcm_port_stat_t *stat_arr, 
    uint32 *value_arr);

/* Set 64-bit counter value for multiple port statistic types. */
extern int bcmx_port_stat_multi_set(
    bcm_gport_t port, 
    int nstat, 
    bcm_port_stat_t *stat_arr, 
    uint64 *value_arr);

/* Set lower 32-bit counter value for multiple port statistic types. */
extern int bcmx_port_stat_multi_set32(
    bcm_gport_t port, 
    int nstat, 
    bcm_port_stat_t *stat_arr, 
    uint32 *value_arr);

/* Add a match to an existing port */
extern int bcmx_port_match_add(
    bcm_gport_t port, 
    bcmx_port_match_info_t *match);

/* Remove a match from an existing port */
extern int bcmx_port_match_delete(
    bcm_gport_t port, 
    bcmx_port_match_info_t *match);

/* Replace an old match with a new one for an existing port */
extern int bcmx_port_match_replace(
    bcm_gport_t port, 
    bcmx_port_match_info_t *old_match, 
    bcmx_port_match_info_t *new_match);

/* Get all the matches for an existing port */
extern int bcmx_port_match_multi_get(
    bcm_gport_t port, 
    int size, 
    bcmx_port_match_info_t *match_array, 
    int *count);

/* Assign a set of matches to an existing port */
extern int bcmx_port_match_set(
    bcm_gport_t port, 
    int size, 
    bcmx_port_match_info_t *match_array);

/* Delete all matches for an existing port */
extern int bcmx_port_match_delete_all(
    bcm_gport_t port);

extern int bcmx_port_timesync_config_set(
    bcm_gport_t port, 
    bcmx_port_timesync_config_t *conf);

extern int bcmx_port_timesync_config_get(
    bcm_gport_t port, 
    bcmx_port_timesync_config_t *conf);

extern int bcmx_port_control_timesync_set(
    bcm_gport_t port, 
    bcm_port_control_timesync_t type, 
    uint64 value);

extern int bcmx_port_control_timesync_get(
    bcm_gport_t port, 
    bcm_port_control_timesync_t type, 
    uint64 *value);

#endif /* __BCMX_PORT_H__ */

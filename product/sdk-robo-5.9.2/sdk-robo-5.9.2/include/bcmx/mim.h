/*
 * $Id: mim.h,v 1.2 Broadcom SDK $
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

#ifndef __BCMX_MIM_H__
#define __BCMX_MIM_H__

#if defined(INCLUDE_L3)

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcm/mim.h>

typedef bcm_mim_vpn_config_t bcmx_mim_vpn_config_t;

typedef bcm_mim_port_t bcmx_mim_port_t;

/* Initialize the MIM VPN config structure. */
extern void bcmx_mim_vpn_config_t_init(
    bcmx_mim_vpn_config_t *mim_vpn_config);

/* Initialize the MIM port structure. */
extern void bcmx_mim_port_t_init(
    bcmx_mim_port_t *mim_port);

/* Initialize the MIM subsystem. */
extern int bcmx_mim_init(void);

/* Detach the MIM software module. */
extern int bcmx_mim_detach(void);

/* Create a VPN instance. */
extern int bcmx_mim_vpn_create(
    bcmx_mim_vpn_config_t *info);

/* Delete a VPN instance. */
extern int bcmx_mim_vpn_destroy(
    bcm_mim_vpn_t vpn);

/* Delete all VPN instances. */
extern int bcmx_mim_vpn_destroy_all(void);

/* Get a VPN instance by ID. */
extern int bcmx_mim_vpn_get(
    bcm_mim_vpn_t vpn, 
    bcmx_mim_vpn_config_t *info);

/* bcm_mim_port_add */
extern int bcmx_mim_port_add(
    bcm_mim_vpn_t vpn, 
    bcmx_mim_port_t *mim_port);

/* bcm_mim_port_delete */
extern int bcmx_mim_port_delete(
    bcm_mim_vpn_t vpn, 
    bcm_gport_t mim_port_id);

/* bcm_mim_port_delete_all */
extern int bcmx_mim_port_delete_all(
    bcm_mim_vpn_t vpn);

/* bcm_mim_port_get */
extern int bcmx_mim_port_get(
    bcm_mim_vpn_t vpn, 
    bcmx_mim_port_t *mim_port);

/* bcm_mim_port_get_all */
extern int bcmx_mim_port_get_all(
    bcm_mim_vpn_t vpn, 
    int port_max, 
    bcmx_mim_port_t *port_array, 
    int *port_count);

#endif /* defined(INCLUDE_L3) */

#endif /* __BCMX_MIM_H__ */

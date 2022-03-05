/*
 * $Id: ipfix.h,v 1.5 Broadcom SDK $
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

#ifndef __BCMX_IPFIX_H__
#define __BCMX_IPFIX_H__

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcm/ipfix.h>

typedef bcm_ipfix_config_t bcmx_ipfix_config_t;

typedef bcm_ipfix_rate_t bcmx_ipfix_rate_t;

typedef bcm_ipfix_mirror_config_t bcmx_ipfix_mirror_config_t;

/* Structure initializer */
extern void bcmx_ipfix_config_t_init(
    bcmx_ipfix_config_t *config);

/* Structure initializer */
extern void bcmx_ipfix_rate_t_init(
    bcmx_ipfix_rate_t *rate_info);

/* Structure initializer */
extern void bcmx_ipfix_mirror_config_t_init(
    bcmx_ipfix_mirror_config_t *config);

/* Set the IPFIX configuration of the specified port */
extern int bcmx_ipfix_config_set(
    bcm_ipfix_stage_t stage, 
    bcmx_lport_t port, 
    bcmx_ipfix_config_t *config);

/* Get the IPFIX configuration of the specified port */
extern int bcmx_ipfix_config_get(
    bcm_ipfix_stage_t stage, 
    bcmx_lport_t port, 
    bcmx_ipfix_config_t *config);

/* Add an IPFIX flow rate meter entry */
extern int bcmx_ipfix_rate_create(
    bcmx_ipfix_rate_t *rate_info);

/* Delete an IPFIX flow rate meter entry */
extern int bcmx_ipfix_rate_destroy(
    bcm_ipfix_rate_id_t rate_id);

/* Get IPFIX flow rate meter entry for the specified id */
extern int bcmx_ipfix_rate_get(
    bcmx_ipfix_rate_t *rate_info);

/* Delete all IPFIX flow rate meter entries */
extern int bcmx_ipfix_rate_destroy_all(void);

/* Add a mirror destination to the IPFIX flow rate meter entry */
extern int bcmx_ipfix_rate_mirror_add(
    bcm_ipfix_rate_id_t rate_id, 
    bcm_gport_t mirror_dest_id);

/* Delete a mirror destination from the IPFIX flow rate meter entry */
extern int bcmx_ipfix_rate_mirror_delete(
    bcm_ipfix_rate_id_t rate_id, 
    bcm_gport_t mirror_dest_id);

/* 
 * Delete all mirror destination associated to the IPFIX flow rate meter
 * entry
 */
extern int bcmx_ipfix_rate_mirror_delete_all(
    bcm_ipfix_rate_id_t rate_id);

/* Get all mirror destination from the IPFIX flow rate meter entry */
extern int bcmx_ipfix_rate_mirror_get(
    bcm_ipfix_rate_id_t rate_id, 
    int mirror_dest_size, 
    bcm_gport_t *mirror_dest_id, 
    int *mirror_dest_count);

/* Set IPFIX mirror control configuration of the specified port */
extern int bcmx_ipfix_mirror_config_set(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port, 
    bcmx_ipfix_mirror_config_t *config);

/* Get all IPFIX mirror control configuration of the specified port */
extern int bcmx_ipfix_mirror_config_get(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port, 
    bcmx_ipfix_mirror_config_t *config);

/* Add an IPFIX mirror destination to the specified port */
extern int bcmx_ipfix_mirror_port_dest_add(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port, 
    bcm_gport_t mirror_dest_id);

/* Delete an IPFIX mirror destination from the specified port */
extern int bcmx_ipfix_mirror_port_dest_delete(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port, 
    bcm_gport_t mirror_dest_id);

/* Delete all IPFIX mirror destination from the specified port */
extern int bcmx_ipfix_mirror_port_dest_delete_all(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port);

/* Get IPFIX mirror destination of the specified port */
extern int bcmx_ipfix_mirror_port_dest_get(
    bcm_ipfix_stage_t stage, 
    bcm_gport_t port, 
    int mirror_dest_size, 
    bcm_gport_t *mirror_dest_id, 
    int *mirror_dest_count);

#endif /* __BCMX_IPFIX_H__ */

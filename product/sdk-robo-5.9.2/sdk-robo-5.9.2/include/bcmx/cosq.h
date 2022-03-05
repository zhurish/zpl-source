/*
 * $Id: cosq.h,v 1.14 Broadcom SDK $
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

#ifndef __BCMX_COSQ_H__
#define __BCMX_COSQ_H__

#include <bcm/types.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>
#include <bcm/cosq.h>

typedef bcm_cosq_gport_discard_t bcmx_cosq_gport_discard_t;

/* Structure initializer */
extern void bcmx_cosq_gport_discard_t_init(
    bcmx_cosq_gport_discard_t *discard);

/* Initialize the COSQ subsystem. */
extern int bcmx_cosq_init(void);

/* De-initialize the COSQ subsystem. */
extern int bcmx_cosq_detach(void);

/* Configure the number of Class of Service Queues (COSQs). */
extern int bcmx_cosq_config_set(
    int numq);

/* Get the number of Class of Service Queues (COSQs). */
extern int bcmx_cosq_config_get(
    int *numq);

/* Set the mapping from internal priority to COS queue. */
extern int bcmx_cosq_mapping_set(
    bcm_cos_t priority, 
    bcm_cos_queue_t cosq);

/* Get the mapping from internal priority to COS queue. */
extern int bcmx_cosq_mapping_get(
    bcm_cos_t priority, 
    bcm_cos_queue_t *cosq);

/* Set the mapping from internal priority to COS queue. */
extern int bcmx_cosq_port_mapping_set(
    bcmx_lport_t port, 
    bcm_cos_t priority, 
    bcm_cos_queue_t cosq);

/* Get the mapping from internal priority to COS queue. */
extern int bcmx_cosq_port_mapping_get(
    bcmx_lport_t port, 
    bcm_cos_t priority, 
    bcm_cos_queue_t *cosq);

/* Set Class of Service policy, weights and delay. */
extern int bcmx_cosq_sched_set(
    int mode, 
    const int weights[BCM_COS_COUNT], 
    int delay);

/* Set Class of Service policy, weights and delay. */
extern int bcmx_cosq_port_sched_set(
    bcmx_lplist_t lplist, 
    int mode, 
    const int weights[BCM_COS_COUNT], 
    int delay);

/* Get Class of Service policy, weights and delay. */
extern int bcmx_cosq_sched_get(
    int *mode, 
    int weights[BCM_COS_COUNT], 
    int *delay);

/* Get Class of Service policy, weights and delay. */
extern int bcmx_cosq_port_sched_get(
    bcmx_lplist_t lplist, 
    int *mode, 
    int weights[BCM_COS_COUNT], 
    int *delay);

/* Retrieve maximum weights for given COS policy. */
extern int bcmx_cosq_sched_weight_max_get(
    int mode, 
    int *weight_max);

/* Configure a port bandwidth distribution among COS queues. */
extern int bcmx_cosq_port_bandwidth_set(
    bcmx_lport_t port, 
    bcm_cos_queue_t cosq, 
    uint32 kbits_sec_min, 
    uint32 kbits_sec_max, 
    uint32 flags);

/* Configure a port bandwidth distribution among COS queues. */
extern int bcmx_cosq_port_bandwidth_get(
    bcmx_lport_t port, 
    bcm_cos_queue_t cosq, 
    uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, 
    uint32 *flags);

/* Configure Weighted Random Early Discard (WRED). */
extern int bcmx_cosq_discard_set(
    uint32 flags);

/* Configure Weighted Random Early Discard (WRED). */
extern int bcmx_cosq_discard_get(
    uint32 *flags);

/* Configure a port's Weighted Random Early Discard (WRED) parameters. */
extern int bcmx_cosq_discard_port_set(
    bcmx_lport_t port, 
    bcm_cos_queue_t cosq, 
    uint32 color, 
    int drop_start, 
    int drop_slope, 
    int average_time);

/* Get a port's Weighted Random Early Discard (WRED) parameters. */
extern int bcmx_cosq_discard_port_get(
    bcmx_lport_t port, 
    bcm_cos_queue_t cosq, 
    uint32 color, 
    int *drop_start, 
    int *drop_slope, 
    int *average_time);

/* Set various features at the gport/cosq level. */
extern int bcmx_cosq_control_set(
    bcm_gport_t port, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_control_t type, 
    int arg);

/* Get various features at the gport/cosq level. */
extern int bcmx_cosq_control_get(
    bcm_gport_t port, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_control_t type, 
    int *arg);

extern int bcmx_cosq_gport_add(
    bcm_gport_t port, 
    int numq, 
    uint32 flags, 
    bcm_gport_t *gport);

extern int bcmx_cosq_gport_delete(
    bcm_gport_t gport);

extern int bcmx_cosq_gport_bandwidth_set(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 kbits_sec_min, 
    uint32 kbits_sec_max, 
    uint32 flags);

extern int bcmx_cosq_gport_bandwidth_get(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, 
    uint32 *flags);

extern int bcmx_cosq_gport_sched_set(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int mode, 
    int weight);

extern int bcmx_cosq_gport_sched_get(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    int *mode, 
    int *weight);

extern int bcmx_cosq_gport_discard_set(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcmx_cosq_gport_discard_t *discard);

extern int bcmx_cosq_gport_discard_get(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcmx_cosq_gport_discard_t *discard);

extern int bcmx_cosq_gport_attach(
    bcm_gport_t sched_port, 
    bcm_gport_t input_port, 
    bcm_cos_queue_t cosq);

extern int bcmx_cosq_gport_detach(
    bcm_gport_t sched_port, 
    bcm_gport_t input_port, 
    bcm_cos_queue_t cosq);

extern int bcmx_cosq_gport_attach_get(
    bcm_gport_t sched_port, 
    bcm_gport_t *input_port, 
    bcm_cos_queue_t *cosq);

extern int bcmx_cosq_gport_destmod_attach(
    bcm_gport_t gport, 
    bcm_gport_t ingress_port, 
    bcm_module_t dest_modid, 
    int fabric_egress_port);

extern int bcmx_cosq_gport_destmod_detach(
    bcm_gport_t gport, 
    bcm_gport_t ingress_port, 
    bcm_module_t dest_modid, 
    int fabric_egress_port);

extern int bcmx_cosq_stat_get(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint64 *value);

extern int bcmx_cosq_stat_get32(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint32 *value);

extern int bcmx_cosq_stat_set(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint64 value);

extern int bcmx_cosq_stat_set32(
    bcm_gport_t gport, 
    bcm_cos_queue_t cosq, 
    bcm_cosq_stat_t stat, 
    uint32 value);

#endif /* __BCMX_COSQ_H__ */

/*
 * $Id: bcm53280_service.h,v 1.25 Broadcom SDK $
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
 */
 
#ifndef _BCM53280_SERVICE_H
#define _BCM53280_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>

int drv_bcm53280_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n);
int drv_bcm53280_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n);
int drv_bcm53280_queue_port_dfsv_set
    (int unit, uint8 port, uint8 dscp, uint8 prio, uint8 queue_n);
int drv_bcm53280_queue_port_dfsv_get
    (int unit, uint8 port, uint8 dscp, uint8 *prio, uint8 *queue_n);
int drv_bcm53280_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode);
int drv_bcm53280_queue_mode_get(int unit, uint32 port, uint32 *mode);
int drv_bcm53280_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_bcm53280_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_bcm53280_queue_WRR_weight_set(int unit, 
    uint32 port_type, uint8 queue, uint32 weight);
int drv_bcm53280_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint8 queue, uint32 *weight);
int drv_bcm53280_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n);
int drv_bcm53280_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n);
int drv_bcm53280_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio, uint8 prio);
int drv_bcm53280_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, uint8 *prio);
int drv_bcm53280_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio);
int drv_bcm53280_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio);
int drv_bcm53280_queue_dfsv_unmap_set(int unit, uint8 prio, uint8 dscp);
int drv_bcm53280_queue_dfsv_unmap_get(int unit, uint8 prio, uint8 * dscp);
int drv_bcm53280_queue_tos_set(int unit, uint8 precedence, uint8 queue_n);
int drv_bcm53280_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n);
int drv_bcm53280_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n);
int drv_bcm53280_queue_dfsv_get(int unit, uint8 code_point, uint8 * queue_n);
int drv_bcm53280_queue_mapping_type_set
    (int unit, soc_pbmp_t bmp, uint32 mapping_type, uint8 state);
int drv_bcm53280_queue_mapping_type_get
    (int unit, uint32 port, uint32 mapping_type, uint8 *state);
int drv_bcm53280_queue_rx_reason_set
    (int unit, uint8 reason, uint32 queue);
int drv_bcm53280_queue_rx_reason_get
    (int unit, uint8 reason, uint32 *queue);
int drv_bcm53280_queue_port_txq_pause_set
    (int unit, uint32 port, uint8 queue_n, uint8 enable);
int drv_bcm53280_queue_port_txq_pause_get
    (int unit, uint32 port, uint8 queue_n, uint8 * enable);
int drv_bcm53280_queue_qos_control_set(int unit, uint32 type, uint32 state);
int drv_bcm53280_queue_qos_control_get(int unit, uint32 type, uint32 * state);
int drv_bcm53280_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op);
int drv_bcm53280_trunk_get(int unit, int tid, soc_pbmp_t * bmp, 
    uint32 flag, uint32 * hash_op);
int drv_bcm53280_trunk_hash_field_add(int unit, uint32 field_type);
int drv_bcm53280_trunk_hash_field_remove(int unit, uint32 field_type);
int drv_bcm53280_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm53280_mem_search(int unit, uint32 mem, uint32 *key, uint32 *entry, uint32 *entry_1, uint32 flags);
int drv_bcm53280_mem_clear(int unit, uint32 mem);
int drv_bcm53280_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_bcm53280_mem_field_get(int unit, uint32 mem, 
                uint32 field_index, uint32 *entry, uint32 *fld_data);
int drv_bcm53280_mem_field_set(int unit, uint32 mem, 
                uint32 field_index, uint32 *entry, uint32 *fld_data);
int drv_bcm53280_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_bcm53280_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_bcm53280_mem_read(int unit, uint32 mem, uint32 entry_id, 
                uint32 count, uint32 *entry);
int drv_bcm53280_mem_write(int unit, uint32 mem, uint32 entry_id, 
                uint32 count, uint32 *entry);
int drv_bcm53280_mem_fill(int unit, uint32 mem, 
    int entry_id, int count, uint32 *entry);
int drv_bcm53280_mem_move(int unit, uint32 mem,int src_index, 
    int dest_index, int count, int flag);
int drv_bcm53280_mirror_get(int unit, uint32 * enable, soc_pbmp_t * mport_bmp, 
    soc_pbmp_t * ingress_bmp, soc_pbmp_t * egress_bmp);
int drv_bcm53280_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_bcm53280_port_get(int unit, int port, uint32 prop_type, uint32 * prop_val);
int drv_bcm53280_port_set(int unit, soc_pbmp_t bmp, uint32 prop_type, uint32 prop_val);
int drv_bcm53280_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 * pri_new, uint32 * cfi_new);
int drv_bcm53280_port_pri_mapop_set(int unit, int port, int op_type,
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_bcm53280_security_get(int unit, uint32 port, uint32 * state, uint32 * mask);
int drv_bcm53280_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_bcm53280_security_egress_get(int unit, int port, int * enable);
int drv_bcm53280_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_bcm53280_security_eap_control_get(int unit, uint32 type, uint32 * value);
int drv_bcm53280_security_eap_control_set(int unit, uint32 type, uint32 value);
int drv_bcm53280_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, 
    uint32 port_state);
int drv_bcm53280_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, 
    uint32 * port_state);
int drv_bcm53280_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask);
int drv_bcm53280_trap_get(int unit, soc_port_t port, uint32 * trap_mask);
int drv_bcm53280_snoop_set(int unit, uint32 snoop_mask);
int drv_bcm53280_snoop_get(int unit, uint32 * snoop_mask);
int drv_bcm53280_rate_config_set(int unit, soc_pbmp_t pbmp, 
    uint32 config_type, uint32 value);
int drv_bcm53280_rate_config_get(int unit, uint32 port, 
    uint32 config_type, uint32 * value);
int drv_bcm53280_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, 
    int direction, uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size);
int drv_bcm53280_rate_get(int unit, uint32 port, uint8 queue_n, 
    int direction, uint32 * kbits_sec_min, uint32 * kbits_sec_max, uint32 * burst_size);
int drv_bcm53280_storm_control_enable_set(int unit, uint32 port, 
    uint8 enable);
int drv_bcm53280_storm_control_enable_get(int unit, uint32 port, 
    uint8 * enable);
int drv_bcm53280_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_bcm53280_storm_control_get(int unit, uint32 port, uint32 * type, 
    uint32 * limit, uint32 * burst_size);
int drv_bcm53280_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_bcm53280_vlan_mode_get(int unit, uint32 *mode);
int drv_bcm53280_vlan_mode_set(int unit, uint32 mode);
int drv_bcm53280_vlan_prop_get(int unit, uint32 prop_type, uint32 * prop_val);
int drv_bcm53280_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_bcm53280_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 * val);
int drv_bcm53280_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val);
int drv_bcm53280_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp);
int drv_bcm53280_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp);
int drv_bcm53280_port_vlan_pvid_set(int unit, uint32 port, uint32 outer_tag, uint32 inner_tag);
int drv_bcm53280_port_vlan_pvid_get(int unit, uint32 port, uint32 *outer_tag, uint32 *inner_tag);
int drv_bcm53280_vm_init(int unit);
int drv_bcm53280_vm_deinit(int unit);
int drv_bcm53280_vm_qset_get(int unit, uint32 qual, drv_vm_entry_t *entry, uint32 *val);
int drv_bcm53280_vm_qset_set(int unit, uint32 qual, drv_vm_entry_t *entry, uint32 val);
int drv_bcm53280_vm_format_id_select(int unit, drv_vm_entry_t *entry, uint32 *id, uint32 flags);
int drv_bcm53280_vm_format_to_qset(int unit, uint32 mem_type, uint32 id, drv_vm_entry_t *entry);
int drv_bcm53280_vm_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val);
int drv_bcm53280_vm_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_vm_entry_t* entry, uint32* fld_val);
int drv_bcm53280_vm_action_get(int unit, uint32 action, drv_vm_entry_t* entry, uint32* act_param);
int drv_bcm53280_vm_action_set(int unit, uint32 action, drv_vm_entry_t* entry, uint32 act_param);
int drv_bcm53280_vm_entry_read(int unit, uint32 index, uint32 ram_type, drv_vm_entry_t *entry);
int drv_bcm53280_vm_entry_write(int unit, uint32 index, uint32 ram_type, drv_vm_entry_t *entry);
int drv_bcm53280_vm_range_set(int unit, uint32 id, uint32 min, uint32 max);
int drv_bcm53280_vm_flow_alloc(int unit, uint32 type, int *flow_id);
int drv_bcm53280_vm_flow_free(int unit, int flow_id);
int drv_bcm53280_vm_ranger_inc(int unit, int ranger_id);
int drv_bcm53280_vm_ranger_dec(int unit, int ranger_id);
int drv_bcm53280_vm_ranger_count_get(int unit, int ranger_id, uint32 *count);
int drv_bcm53280_mcrep_vpgrp_vport_config_set(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param);
int drv_bcm53280_mcrep_vpgrp_vport_config_get(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param);
int drv_bcm53280_mcrep_vport_config_set(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 vport, uint32 vid);
int drv_bcm53280_mcrep_vport_config_get(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 *vport, uint32 *vid);
int drv_bcm53280_mcrep_vport_vid_search(int unit, uint32 port, 
        uint32 *vport, int *param);
int drv_bcm53280_dev_control_set(int unit,uint32 *ctrl_cnt,
        uint32 *type_list,int *value_list);
int drv_bcm53280_dev_control_get(int unit,uint32 *ctrl_cnt,
        uint32 *type_list,int *value_list);

int drv_bcm53280_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);
int drv_bcm53280_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);
int drv_bcm53280_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode);
int drv_bcm53280_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode);

int drv53280_cfp_init(int unit);
int drv53280_cfp_action_get(int unit,uint32 *action,drv_cfp_entry_t *entry,uint32 *act_param);
int drv53280_cfp_action_set(int unit,uint32 action,drv_cfp_entry_t *entry,uint32 act_param1,uint32 act_param2);
int drv53280_cfp_control_get(int unit,uint32 control_type,uint32 param1,uint32 *param2);
int drv53280_cfp_control_set(int unit,uint32 control_type,uint32 param1,uint32 param2);
int drv53280_cfp_entry_read(int unit,uint32 index,uint32 ram_type,drv_cfp_entry_t *entry);
int drv53280_cfp_entry_search(int unit,uint32 flags,uint32 *index,drv_cfp_entry_t *entry);
int drv53280_cfp_entry_write(int unit,uint32 index,uint32 ram_type,drv_cfp_entry_t *entry);
int drv53280_cfp_field_get(int unit,uint32 mem_type,uint32 field_type,drv_cfp_entry_t *entry,uint32 *fld_val);
int drv53280_cfp_field_set(int unit,uint32 mem_type,uint32 field_type,drv_cfp_entry_t *entry,uint32 *fld_val);
int drv53280_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);
int drv53280_cfp_meter_get(int unit,drv_cfp_entry_t *entry,uint32 *kbits_sec,uint32 *kbits_burst);
int drv53280_cfp_meter_set(int unit,drv_cfp_entry_t *entry,uint32 kbits_sec,uint32 kbits_burst);
int drv53280_cfp_qset_get(int unit,uint32 qual,drv_cfp_entry_t *entry,uint32 *val);
int drv53280_cfp_qset_set(int unit,uint32 qual,drv_cfp_entry_t *entry,uint32 val);
int drv53280_cfp_slice_id_select(int unit,drv_cfp_entry_t *entry,uint32 *slice_id,uint32 flags);
int drv53280_cfp_slice_to_qset(int unit,uint32 slice_id,drv_cfp_entry_t *entry);
int drv53280_cfp_stat_get(int unit,uint32 stat_type,uint32 index,uint32 *counter);
int drv53280_cfp_stat_set(int unit,uint32 stat_type,uint32 index,uint32 counter);
int drv53280_cfp_udf_get(int unit,uint32 port,uint32 udf_index,uint32 *offset,uint32 *base);
int drv53280_cfp_udf_set(int unit,uint32 port,uint32 udf_index,uint32 offset,uint32 base);
int drv53280_cfp_ranger(int unit,uint32 flags,uint32 min,uint32 max);
int drv53280_cfp_range_set(int unit,uint32 type,uint32 id,uint32 param1,uint32 param2);
int drv53280_cfp_range_get(int unit,uint32 type,uint32 id,uint32 *param1,uint32 *param2);
int drv_bcm53280_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type,uint8* mac, uint32 bpdu_idx);
int drv_bcm53280_mac_get(int unit, uint32 val, uint32 mac_type,soc_pbmp_t *bmp, uint8* mac);
int drv_bcm53280_arl_learn_count_set(int unit,uint32 port,uint32 type,int value);
int drv_bcm53280_arl_learn_count_get(int unit,uint32 port,uint32 type,int *value);
int drv_bcm53280_fp_init(int unit,int stage_id);
int drv_bcm53280_fp_deinit(int unit,int stage_id);
int drv_bcm53280_fp_qual_value_set(int unit,int stage_id,drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_bcm53280_fp_qual_value_get(int unit,int stage_id,drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_bcm53280_fp_udf_value_set(int unit,int stage_id,uint32 udf_idex,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_bcm53280_fp_udf_value_get(int unit,int stage_id,uint32 udf_idex,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_bcm53280_fp_entry_mem_control(int unit,int stage_id,int op,void *src_entry,void *dst_entry,void **alloc_entry);
int drv_bcm53280_fp_entry_tcam_control(int unit,int stage_id,void *drv_entry,int op,int param1,void *param2);
int drv_bcm53280_fp_action_conflict(int unit,int stage_id,drv_field_action_t act1,drv_field_action_t act2);
int drv_bcm53280_fp_action_support_check(int unit,int stage_id,drv_field_action_t action);
int drv_bcm53280_fp_action_add(int unit,int stage_id,void *drv_entry,drv_field_action_t action,uint32 param0,uint32 param1);
int drv_bcm53280_fp_action_remove(int unit,int stage_id,void *drv_entry,drv_field_action_t action,uint32 param0,uint32 param1);
int drv_bcm53280_fp_selcode_mode_get(int unit,int stage_id,void * qset,int mode, int8 *slice_id,uint32 *slice_map,void **entry);
int drv_bcm53280_fp_selcode_to_qset(int unit,  int stage_id, int slice_id, void *qset);
int drv_bcm53280_fp_qualify_support(int unit,int stage_id,void *qset);
int drv_bcm53280_fp_id_control(int unit, int type, int op, int flags, int *id, uint32 *count);
int drv_bcm53280_fp_tcam_parity_check(int unit,drv_fp_tcam_checksum_t *drv_fp_tcam_chksum);
int drv_bcm53280_fp_policer_control(int unit,  int stage_id, int op, void *entry, drv_policer_config_t *policer_cfg);
int drv_bcm53280_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode);
int drv_bcm53280_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode, drv_field_stat_t stat, int *type1, int *type2, int *type3);
int drv_bcm53280_port_block_get(int unit,int port,uint32 block_type, soc_pbmp_t *egress_pbmp);
int drv_bcm53280_port_block_set(int unit,int port,uint32 block_type, soc_pbmp_t egress_pbmp);
int drv_bcm53280_dos_enable_get(int unit, uint32 type, uint32 *param);
int drv_bcm53280_dos_enable_set(int unit, uint32 type, uint32 param);
int drv_bcm53280_dos_event_bitmap_get(int unit, uint32 op, uint32 *event_bitmap);
int drv_bcm53280_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_bcm53280_igmp_mld_snoop_mode_set(int unit, int type, int mode);

static drv_if_t drv_bcm53280_services = {
/* reg_read                 */  drv_reg_read,
/* reg_write                */  drv_reg_write,
/* reg_length_get       */  drv_reg_length_get,
/* reg_addr                 */ drv_reg_addr,
/* reg_field_set            */    drv_reg_field_set,
/* reg_field_get            */    drv_reg_field_get,
/* miim_read                */  drv_miim_read,
/* miim_write               */  drv_miim_write,
/* phy_addr_set             */  NULL,
/* mac_driver_set           */  NULL,
/* mem_length_get           */  drv_bcm53280_mem_length_get,
/* mem_width_get            */  drv_bcm53280_mem_width_get,
/* mem_read                 */  drv_bcm53280_mem_read,
/* mem_write                */  drv_bcm53280_mem_write,
/* mem_field_get            */  drv_bcm53280_mem_field_get,
/* mem_field_set            */  drv_bcm53280_mem_field_set,
/* mem_clear                */  drv_bcm53280_mem_clear,
/* mem_search               */  drv_bcm53280_mem_search,
/* mem_insert               */  drv_bcm53280_mem_insert,
/* mem_delete               */  drv_bcm53280_mem_delete,
/* mem_cache_get        */ drv_mem_cache_get,
/* mem_cache_set        */ drv_mem_cache_set,
/* mem_move             */ drv_bcm53280_mem_move,
/* mem_fill             */ drv_bcm53280_mem_fill,
/* counter_thread_set    */ drv_counter_thread_set,
/* counter_set              */ drv_counter_set,
/* counter_get              */ drv_counter_get,
/* counter_reset            */ drv_counter_reset,
/* vlan_mode_set            */  drv_bcm53280_vlan_mode_set,
/* vlan_mode_get            */  drv_bcm53280_vlan_mode_get,
/* port_vlan_pvid_set       */  drv_bcm53280_port_vlan_pvid_set,
/* port_vlan_pvid_get       */  drv_bcm53280_port_vlan_pvid_get,
/* port_vlan_set            */  drv_bcm53280_port_vlan_set,
/* port_vlan_get            */  drv_bcm53280_port_vlan_get,
/* vlan_prop_set            */ drv_bcm53280_vlan_prop_set,
/* vlan_prop_get            */ drv_bcm53280_vlan_prop_get,
/* vlan_prop_port_enable_set */ drv_bcm53280_vlan_prop_port_enable_set,
/* vlan_prop_port_enable_get */ drv_bcm53280_vlan_prop_port_enable_get,
/* vlan_vt_set              */ drv_vlan_vt_set,
/* vlan_vt_get              */ drv_vlan_vt_get,
/* vlan_vt_add              */ drv_vlan_vt_add,
/* vlan_vt_delete           */ drv_vlan_vt_delete,
/* vlan_vt_delete_all       */ drv_vlan_vt_delete_all,
/* trunk_set                */  drv_bcm53280_trunk_set,
/* trunk_get                */  drv_bcm53280_trunk_get,
/* trunk_hash_field_add     */  drv_bcm53280_trunk_hash_field_add,
/* trunk_hash_field_remove  */  drv_bcm53280_trunk_hash_field_remove,
/* queue_mode_set           */  drv_bcm53280_queue_mode_set,
/* queue_mode_get           */  drv_bcm53280_queue_mode_get,
/* queue_count_set  **/ drv_bcm53280_queue_count_set,
/* queue_count_get          */  drv_bcm53280_queue_count_get,
/* queue_WRR_weight_set     */  drv_bcm53280_queue_WRR_weight_set,
/* queue_WRR_weight_get     */  drv_bcm53280_queue_WRR_weight_get,
/* queue_prio_set           */  drv_bcm53280_queue_prio_set,
/* queue_prio_get           */  drv_bcm53280_queue_prio_get,
/* queue_tos_set           */   drv_bcm53280_queue_tos_set,
/* queue_tos_get           */   drv_bcm53280_queue_tos_get,
/* queue_dfsv_set           */  drv_bcm53280_queue_dfsv_set,
/* queue_dfsv_get           */  drv_bcm53280_queue_dfsv_get,
/* queue_mapping_type_set  */  drv_bcm53280_queue_mapping_type_set,
/* queue_mapping_type_get  */  drv_bcm53280_queue_mapping_type_get,
/* age_timer_set            */  drv_age_timer_set,
/* age_timer_get            */  drv_age_timer_get,
/* mac_set                  */  drv_bcm53280_mac_set,
/* mac_get                  */  drv_bcm53280_mac_get,
/* mirror_set               */  drv_bcm53280_mirror_set,
/* mirror_get               */  drv_bcm53280_mirror_get,
/* port_oper_mode_set       */  drv_port_oper_mode_set,
/* port_oper_mode_get       */  drv_port_oper_mode_get,
/* port_set                 */  drv_bcm53280_port_set,
/* port_get                 */  drv_bcm53280_port_get,
/* port_advertise_set       */  drv_port_advertise_set,
/* port_advertise_get       */   drv_port_advertise_get,
/* port_status_get          */  drv_port_status_get,
/* port_sw_mac_update       */  drv_port_sw_mac_update,
/* port_pri_mapop_set       */  drv_bcm53280_port_pri_mapop_set,
/* port_pri_mapop_get       */  drv_bcm53280_port_pri_mapop_get,
/* port_bitmap_get          */  drv_port_bitmap_get,
/* port_cross_connect_set */ NULL,
/* port_cross_connect_get */ NULL,
/* security_set             */  drv_bcm53280_security_set,
/* security_get             */  drv_bcm53280_security_get,
/* security_egress_set      */  drv_bcm53280_security_egress_set,
/* security_egress_get      */  drv_bcm53280_security_egress_get,
/* security_eap_control_set      */  drv_bcm53280_security_eap_control_set,
/* security_eap_control_get      */  drv_bcm53280_security_eap_control_get,
/* mstp_config_set          */  drv_mstp_config_set,
/* mstp_config_get          */  drv_mstp_config_get,
/* mstp_port_set            */  drv_bcm53280_mstp_port_set,
/* mstp_port_get            */  drv_bcm53280_mstp_port_get,
/* trap_set                 */  drv_bcm53280_trap_set,
/* trap_get                 */  drv_bcm53280_trap_get,
/* snoop_set                */  drv_bcm53280_snoop_set,
/* snoop_get                */  drv_bcm53280_snoop_get,
/* rate_config_set                 */   drv_bcm53280_rate_config_set,
/* rate_config_get                 */   drv_bcm53280_rate_config_get,
/* rate_set                 */  drv_bcm53280_rate_set,
/* rate_get                 */  drv_bcm53280_rate_get,
/* storm_control_enable_set        */   drv_bcm53280_storm_control_enable_set,
/* storm_control_enable_get        */   drv_bcm53280_storm_control_enable_get,
/* storm_control_set        */  drv_bcm53280_storm_control_set,
/* storm_control_get        */  drv_bcm53280_storm_control_get,
/* dev_prop_get             */  drv_bcm53280_dev_prop_get,
/* dev_prop_set             */  drv_dev_prop_set,
/* mcast_init                */ drv_mcast_init,
/* mcast_to_marl             */ drv_mcast_to_marl,
/* mcast_from_marl           */ drv_mcast_from_marl,
/* mcast_bmp_get        */  drv_bcm53280_mcast_bmp_get,
/* mcast_bmp_set        */  drv_bcm53280_mcast_bmp_set,
/* arl_table_process */     drv_arl_table_process,
/* arl_sync */              drv_arl_sync,
/* arl_learn_enable_set */     drv_bcm53280_arl_learn_enable_set,
/* arl_learn_enable_get */     drv_bcm53280_arl_learn_enable_get,
/* queue_port_prio_to_queue_set */ drv_bcm53280_queue_port_prio_to_queue_set,
/* queue_port_prio_to_queue_get */ drv_bcm53280_queue_port_prio_to_queue_get,
/* queue_port_dfsv_set */ drv_bcm53280_queue_port_dfsv_set,
/* queue_port_dfsv_get */drv_bcm53280_queue_port_dfsv_get,
/* queue_prio_remap_set */ drv_bcm53280_queue_prio_remap_set,
/* queue_prio_remap_get */ drv_bcm53280_queue_prio_remap_get,
/* queue_dfsv_remap_set */ drv_bcm53280_queue_dfsv_remap_set,
/* queue_dfsv_remap_get */ drv_bcm53280_queue_dfsv_remap_get,
/* queue_dfsv_unmap_set */ drv_bcm53280_queue_dfsv_unmap_set,
/* queue_dfsv_unmap_get */ drv_bcm53280_queue_dfsv_unmap_get,
/* queue_rx_reason_set */ drv_bcm53280_queue_rx_reason_set,
/* queue_rx_reason_get */ drv_bcm53280_queue_rx_reason_get,
/* queue_port_txq_pause_set */ drv_bcm53280_queue_port_txq_pause_set,
/* queue_port_txq_pause_get */ drv_bcm53280_queue_port_txq_pause_get,
/* queue_qos_control_set */ drv_bcm53280_queue_qos_control_set,
/* queue_qos_control_get */ drv_bcm53280_queue_qos_control_get,
/* drv_cfp_init */ drv53280_cfp_init,
/* drv_cfp_action_get */    drv53280_cfp_action_get,
/* drv_cfp_action_set */    drv53280_cfp_action_set,
/* drv_cfp_control_get */   drv53280_cfp_control_get,
/* drv_cfp_control_set */   drv53280_cfp_control_set,
/* drv_cfp_entry_read */    drv53280_cfp_entry_read,
/* drv_cfp_entry_search */  drv53280_cfp_entry_search,
/* drv_cfp_entry_write */   drv53280_cfp_entry_write,
/* drv_cfp_field_get */ drv53280_cfp_field_get,
/* drv_cfp_field_set */ drv53280_cfp_field_set,
/* drv_cfp_meter_rate_transform */drv53280_cfp_meter_rate_transform,
/* drv_cfp_meter_get */ drv53280_cfp_meter_get,
/* drv_cfp_meter_set */ drv53280_cfp_meter_set,
/* drv_cfp_qset_get */  drv53280_cfp_qset_get,
/* drv_cfp_qset_set */  drv53280_cfp_qset_set,
/* drv_cfp_slice_id_select */   drv53280_cfp_slice_id_select,
/* drv_cfp_slice_to_qset */   drv53280_cfp_slice_to_qset,
/* drv_cfp_stat_get */  drv53280_cfp_stat_get,
/* drv_cfp_stat_set */  drv53280_cfp_stat_set,
/* drv_cfp_udf_get */   drv53280_cfp_udf_get,
/* drv_cfp_udf_set */   drv53280_cfp_udf_set,
/* drv_cfp_ranger */   drv53280_cfp_ranger,
/* drv_cfp_range_set */   drv53280_cfp_range_set,
/* drv_cfp_range_get */   drv53280_cfp_range_get,
/* drv_cfp_sub_qual_by_udf */ drv_cfp_sub_qual_by_udf,
/* drv_eav_control_set */ drv_eav_control_set,
/* drv_eav_control_get */ drv_eav_control_get,
/* drv_eav_enable_set */ drv_eav_enable_set,
/* drv_eav_enable_get */ drv_eav_enable_get,
/* drv_eav_link_status_set */ drv_eav_link_status_set,
/* drv_eav_link_status_get */ drv_eav_link_status_get,
/* drv_eav_egress_timestamp_get */ drv_eav_egress_timestamp_get,
/* drv_eav_time_sync_set */ drv_eav_time_sync_set,
/* drv_eav_time_sync_get */ drv_eav_time_sync_get,
/* drv_eav_queue_control_set */ drv_eav_queue_control_set,
/* drv_eav_queue_control_get */ drv_eav_queue_control_get,
/* drv_eav_time_sync_mac_set */ drv_eav_time_sync_mac_set,
/* drv_eav_time_sync_mac_get */ drv_eav_time_sync_mac_get,
/* drv_dos_enable_set */ drv_bcm53280_dos_enable_set,
/* drv_dos_enable_get */ drv_bcm53280_dos_enable_get,
/* drv_dos_event_bitmap_get */  drv_bcm53280_dos_event_bitmap_get,
/* drv_vm_init */ drv_bcm53280_vm_init,
/* drv_vm_deinit */ drv_bcm53280_vm_deinit,
/* drv_vm_field_get */ drv_bcm53280_vm_field_get,
/* drv_vm_field_set */ drv_bcm53280_vm_field_set,
/* drv_vm_action_get */ drv_bcm53280_vm_action_get,
/* drv_vm_action_set */ drv_bcm53280_vm_action_set,
/* drv_vm_entry_read */ drv_bcm53280_vm_entry_read,
/* drv_vm_entry_write */ drv_bcm53280_vm_entry_write,
/* drv_vm_qset_get */ drv_bcm53280_vm_qset_get,
/* drv_vm_qset_set */ drv_bcm53280_vm_qset_set,
/* drv_vm_format_id_select */ drv_bcm53280_vm_format_id_select,
/* drv_vm_slice_to_qset */ drv_bcm53280_vm_format_to_qset,
/* drv_vm_range_set */ drv_bcm53280_vm_range_set,
/* drv_vm_flow_alloc */ drv_bcm53280_vm_flow_alloc,
/* drv_vm_flow_free */ drv_bcm53280_vm_flow_free,
/* drv_vm_ranger_inc */ drv_bcm53280_vm_ranger_inc,
/* drv_vm_ranger_dec */ drv_bcm53280_vm_ranger_dec,
/* drv_vm_ranger_count_get */ drv_bcm53280_vm_ranger_count_get,
/* drv_mcrep_vpgrp_vport_config_set */ drv_bcm53280_mcrep_vpgrp_vport_config_set, 
/* drv_mcrep_vpgrp_vport_config_get */ drv_bcm53280_mcrep_vpgrp_vport_config_get, 
/* drv_mcrep_vport_config_set */ drv_bcm53280_mcrep_vport_config_set, 
/* drv_mcrep_vport_config_get */ drv_bcm53280_mcrep_vport_config_get, 
/* drv_mcrep_vport_vid_search */ drv_bcm53280_mcrep_vport_vid_search,
/* drv_dev_control_set */ drv_bcm53280_dev_control_set,
/* drv_dev_control_get */ drv_bcm53280_dev_control_get,
/* arl_learn_count_set */ drv_bcm53280_arl_learn_count_set,
/* arl_learn_count_get */ drv_bcm53280_arl_learn_count_get,
/* fp_init */ drv_bcm53280_fp_init,
/* fp_deinit */ drv_bcm53280_fp_deinit,
/* fp_qual_value_set        */ drv_bcm53280_fp_qual_value_set,
/* fp_qual_value_get        */ drv_bcm53280_fp_qual_value_get,
/* fp_udf_value_set         */ drv_bcm53280_fp_udf_value_set,
/* fp_udf_value_get         */ drv_bcm53280_fp_udf_value_get,       
/* fp_entry_mem_control     */ drv_bcm53280_fp_entry_mem_control,
/* fp_entry_tcam_control    */ drv_bcm53280_fp_entry_tcam_control,
/* fp_action_conflict       */ drv_bcm53280_fp_action_conflict,   
/* fp_action_support_check  */ drv_bcm53280_fp_action_support_check,
/* fp_action_add            */ drv_bcm53280_fp_action_add,
/* fp_action_remove         */ drv_bcm53280_fp_action_remove,
/* fp_selcode_mode_get      */ drv_bcm53280_fp_selcode_mode_get,
/* fp_selcode_to_qset      */drv_bcm53280_fp_selcode_to_qset,
/* fp_qualify_support       */ drv_bcm53280_fp_qualify_support,
/* fp_id_control    */ drv_bcm53280_fp_id_control,
/* fp_tcam_parity_check */ drv_bcm53280_fp_tcam_parity_check,
/* fp_policer_control */ drv_bcm53280_fp_policer_control,
/* fp_stat_support_check */ drv_bcm53280_fp_stat_support_check,
/* fp_stat_type_get */ drv_bcm53280_fp_stat_type_get,
/* port_block_get           */ drv_bcm53280_port_block_get,
/* port_block_set           */ drv_bcm53280_port_block_set,
/* led_func_get             */ drv_led_func_get,
/* led_func_set             */ drv_led_func_set,
/* led_funcgrp_select_get   */ drv_led_funcgrp_select_get,
/* led_funcgrp_select_set   */ drv_led_funcgrp_select_set,
/* led_mode_get             */ drv_led_mode_get,
/* led_mode_set             */ drv_led_mode_set,
/* igmp_mld_snoop_mode_get  */ drv_bcm53280_igmp_mld_snoop_mode_get,
/* igmp_mld_snoop_mode_set  */ drv_bcm53280_igmp_mld_snoop_mode_set,
};

#endif

/*
 * $Id: robo_5324_service.h,v 1.5 Broadcom SDK $
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
 
#ifndef _ROBO_5324_SERVICE_H
#define _ROBO_5324_SERVICE_H

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>


int drv_reg_read(int unit, uint32 addr, void *data, int len);
int drv_reg_write(int unit, uint32 addr, void *data, int len);
int drv_reg_length_get(int unit, uint32 reg);
uint32 drv_reg_addr(int unit, uint32 reg, int port, int index);
int drv_reg_field_set(int unit, uint32 reg, uint32 * regbuf, 
                                                    uint32 field, uint32 *fldbuf);
int drv_reg_field_get(int unit, uint32 reg, uint32 * regbuf, 
                                                    uint32 field, uint32 *fldbuf);
int drv_miim_read(int unit, uint32 phyid, uint32 phyaddr, uint16 *data);
int drv_miim_write(int unit, uint32 phyid, uint32 phyaddr, uint16 data);
int drv_phy_addr_set(int uint, int **map);
int drv_mac_driver_set(int uint, uint32 **p);
int drv_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_mem_read(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_mem_write(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_mem_clear(int unit, uint32 mem);
int drv_mem_search(int unit, uint32 mem, uint32 *key, 
                                                    uint32 *entry, uint32 flags);
int drv_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_mem_cache_get(int unit, uint32 mem, uint32 *enable);
int drv_mem_cache_set(int unit, uint32 mem, uint32 enable);
int drv_counter_thread_set(int unit, uint32 thread_op, 
                                                    uint32 flags, int interval, uint32 bmp);
int drv_counter_set(int unit, soc_pbmp_t bmp, 
                                                    uint32 counter_type, uint64 val);
int drv_counter_get(int unit, uint32 port, 
                                                    uint32 counter_type, uint64 *val);
int drv_counter_reset(int unit);
int drv_vlan_mode_set(int unit, uint32 mode);
int drv_vlan_mode_get(int unit, uint32 *mode);
int drv_port_vlan_pvid_set(int unit, uint32 bmp, 
                                                    uint32 vid, uint32 prio);
int drv_port_vlan_pvid_get(int unit, uint32 bmp, 
                                                    uint32 *vid, uint32 *prio);
int drv_port_vlan_set(int unit, uint32 port, uint32 bmp);
int drv_port_vlan_get(int unit, uint32 port, uint32 *bmp);
int drv_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_trunk_set(int unit, int tid, uint32 bmp, uint32 flag);
int drv_trunk_get(int unit, int tid, uint32 *bmp, uint32 flag);
int drv_trunk_hash_field_add(int unit, uint32 field_type);
int drv_trunk_hash_field_remove(int unit, uint32 field_type);
int drv_trunk_timer_set(int unit, uint32 timer_type, uint32 time);
int drv_trunk_timer_get(int unit, uint32 timer_type, uint32 *time);
int drv_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 mode);
int drv_queue_mode_get(int unit, uint32 port, uint32 *mode);
int drv_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_queue_WRR_weight_set(int unit, uint32 port_type, 
                                                    uint8 queue, uint32 weight);
int drv_queue_WRR_weight_get(int unit, uint32 port_type, 
                                                    uint8 queue, uint32 *weight);
int drv_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n);
int drv_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n);
int drv_queue_tos_set(int unit, uint8 precedence, uint8 queue_n);
int drv_queue_tos_get(int unit, uint8 precedence, uint8 *queue_n);
int drv_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n);
int drv_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n);
int drv_queue_mapping_type_set(int unit, uint32 bmp, 
                                                    uint32 mapping_type, uint8 state);
int drv_queue_mapping_type_get(int unit, uint32 port, 
                                                    uint32 mapping_type, uint8 *state);
int drv_age_timer_set(int unit, uint32 enable, uint32 age_time);
int drv_age_timer_get(int unit, uint32 *enable, uint32 *age_time);
int drv_mac_set(int unit, uint32 bmp, uint32 mac_type, uint8* mac);
int drv_mac_get(int unit, uint32 port, uint32 mac_type, uint8* mac);
int drv_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_mirror_get(int unit, uint32 * enable, soc_pbmp_t * mport_bmp, 
    soc_pbmp_t * ingress_bmp, soc_pbmp_t * egress_bmp);
int drv_port_oper_mode_set(int unit, uint32 bmp, uint32 oper_mode);
int drv_port_oper_mode_get(int unit, int port_n, uint32 *oper_mode);
int drv_port_set(int unit, uint32 bmp, uint32 prop_type, uint32 prop_val);
int drv_port_get(int unit, uint32 port_n, uint32 prop_type, 
                                                                        uint32 *prop_val);
int drv_port_advertise_set(int unit, uint32 bmp, uint32 prop_mask);
int drv_port_advertise_get(int unit, int port_n, uint32 *prop_val);
int drv_port_status_get(int unit, uint32 port, 
                                                    uint32 status_type, uint32 *val);
int drv_phy_set(int unit, uint32 phy_id, uint32 reg_id, uint32 value);
int drv_phy_get(int unit, uint32 phy_id, uint32 reg_id, uint32 *value);
int drv_port_bitmap_get(int unit, uint32 port_type, uint32 *bitmap);
int drv_security_set(int unit, uint32 bmp, uint32 state, uint32 mask);
int drv_security_get(int unit, uint32 bmp, uint32 *state, uint32 *mask);
int drv_mstp_config_set(int unit, uint32 vlan_id, uint32 mstp_gid);
int drv_mstp_config_get(int unit, uint32 vlan_id, uint32 *mstp_gid);
int drv_mstp_port_set(int unit, uint32 mstp_gid, 
                                        uint32 port, uint32 port_state);
int drv_mstp_port_get(int unit, uint32 mstp_gid, 
                                        uint32 port, uint32 *port_state);
int drv_trap_set(int unit,soc_pbmp_t pbmp,uint32 trap_mask);
int drv_trap_get(int unit,soc_port_t port,uint32 *trap_mask);
int drv_snoop_set(int unit, uint32 snoop_mask);
int drv_snoop_get(int unit, uint32 *snoop_mask);
int drv_rate_config_set(int unit, uint32 pbmp, 
                                        uint32 config_type, uint32 value);
int drv_rate_config_get(int unit, uint32 port, 
                                        uint32 config_type, uint32 *value);
int drv_rate_set(int unit, uint32 bmp, int direction, 
                                        uint32 limit, uint32 burst_size);
int drv_rate_get(int unit, uint32 port, int direction, 
                                        uint32 *limit, uint32 *burst_size);
int drv_storm_control_enable_set(int unit, uint32 port, uint8 enable);
int drv_storm_control_enable_get(int unit, uint32 port, uint8 *enable);
int drv_storm_control_set(int unit, uint32 bmp, 
                                        uint32 type, uint32 limit);
int drv_storm_control_get(int unit, uint32 port, 
                                        uint32 *type, uint32 *limit);
int drv_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int  drv_mcast_bmp_get(int unit, uint32 *entry, uint32 *bmp);
int  drv_mcast_bmp_set(int unit, uint32 *entry, uint32 bmp);



drv_if_t drv_5324_services = {
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
/* mem_length_get           */  drv_mem_length_get,
/* mem_width_get            */  drv_mem_width_get,
/* mem_read                 */  drv_mem_read,
/* mem_write                */  drv_mem_write,
/* mem_field_get            */  drv_mem_field_get,
/* mem_field_set            */  drv_mem_field_set,
/* mem_clear                */  drv_mem_clear,
/* mem_search               */  drv_mem_search,
/* mem_insert               */  drv_mem_insert,
/* mem_delete               */  drv_mem_delete,
/* mem_cache_get        */ drv_mem_cache_get,
/* mem_cache_set        */ drv_mem_cache_set,
/* counter_thread_set    */ drv_counter_thread_set,
/* counter_set              */ drv_counter_set,
/* counter_get              */ drv_counter_get,
/* counter_reset            */ drv_counter_reset,
/* vlan_mode_set            */  drv_vlan_mode_set,
/* vlan_mode_get            */  drv_vlan_mode_get,
/* port_vlan_pvid_set       */  drv_port_vlan_pvid_set,
/* port_vlan_pvid_get       */  drv_port_vlan_pvid_get,
/* port_vlan_set            */  drv_port_vlan_set,
/* port_vlan_get            */  drv_port_vlan_get,
/* vlan_prop_set            */ drv_vlan_prop_set,
/* vlan_prop_get            */ drv_vlan_prop_get,
/* trunk_set                */  drv_trunk_set,
/* trunk_get                */  drv_trunk_get,
/* trunk_hash_field_add     */  drv_trunk_hash_field_add,
/* trunk_hash_field_remove  */  drv_trunk_hash_field_remove,
/* queue_mode_set           */  drv_queue_mode_set,
/* queue_mode_get           */  drv_queue_mode_get,
/* queue_count_set	**/ drv_queue_count_set,
/* queue_count_get          */  drv_queue_count_get,
/* queue_WRR_weight_set     */  drv_queue_WRR_weight_set,
/* queue_WRR_weight_get     */  drv_queue_WRR_weight_get,
/* queue_prio_set           */  drv_queue_prio_set,
/* queue_prio_get           */  drv_queue_prio_get,
/* queue_tos_set           */   drv_queue_tos_set,
/* queue_tos_get           */   drv_queue_tos_get,
/* queue_dfsv_set           */  drv_queue_dfsv_set,
/* queue_dfsv_get           */  drv_queue_dfsv_get,
/* queue_mapping_type_set  */  drv_queue_mapping_type_set,
/* queue_mapping_type_get  */  drv_queue_mapping_type_get,
/* age_timer_set            */  drv_age_timer_set,
/* age_timer_get            */  drv_age_timer_get,
/* mac_set                  */  drv_mac_set,
/* mac_get                  */  drv_mac_get,
/* mirror_set               */  drv_mirror_set,
/* mirror_get               */  drv_mirror_get,
/* port_oper_mode_set       */  drv_port_oper_mode_set,
/* port_oper_mode_get       */  drv_port_oper_mode_get,
/* port_set                 */  drv_port_set,
/* port_get                 */  drv_port_get,
/* port_advertise_set       */  drv_port_advertise_set,
/* port_advertise_get       */   drv_port_advertise_get,
/* port_status_get          */  drv_port_status_get,
/* port_bitmap_get          */  drv_port_bitmap_get,
/* security_set             */  drv_security_set,
/* security_get             */  drv_security_get,
/* mstp_config_set          */  drv_mstp_config_set,
/* mstp_config_get          */  drv_mstp_config_get,
/* mstp_port_set            */  drv_mstp_port_set,
/* mstp_port_get            */  drv_mstp_port_get,
/* trap_set                 */  drv_trap_set,
/* trap_get                 */  drv_trap_get,
/* snoop_set                */  drv_snoop_set,
/* snoop_get                */  drv_snoop_get,
/* rate_config_set                 */   drv_rate_config_set,
/* rate_config_get                 */   drv_rate_config_get,
/* rate_set                 */  drv_rate_set,
/* rate_get                 */  drv_rate_get,
/* storm_control_enable_set        */   drv_storm_control_enable_set,
/* storm_control_enable_get        */   drv_storm_control_enable_get,
/* storm_control_set        */  drv_storm_control_set,
/* storm_control_get        */  drv_storm_control_get,
/* dev_prop_get             */  drv_dev_prop_get,
/* dev_prop_set             */  drv_dev_prop_set,
/* mcast_bmp_get		*/  drv_mcast_bmp_get,
/* mcast_bmp_set		*/  drv_mcast_bmp_set,
};


#endif

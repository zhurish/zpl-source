/*
 * $Id: cfp.h,v 1.43.6.2 Broadcom SDK $
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
#ifndef _SOC_CFP_H
#define _SOC_CFP_H

#include <shared/bitop.h>
#include <soc/debug.h>
#include <bcm/types.h>
#include <soc/robo_fp.h>

/* Action conflict check macro. */
#define _FIELD_ACTIONS_CONFLICT(_val_)    \
    if (act2 == _val_) {             \
        return (SOC_E_CONFIG);         \
    }
#define DRV_FIELD_STAGE_INGRESS    (0)
#define DRV_FIELD_STAGE_LOOKUP     (1)
#define DRV_FIELD_STAGE_EGRESS     (2)

#define DRV_FIELD_QSET_ADD(qset, q)  SHR_BITSET(((qset).w), (q)) 

#define DRV_FIELD_QSET_REMOVE(qset, q)  SHR_BITCLR(((qset).w), (q)) 

#define DRV_FIELD_QSET_TEST(qset, q)  SHR_BITGET(((qset).w), (q)) 

/* op for drv_fp_entry_mem_control */
/* get the allocated drv_entry pointer from alloc_entry*/
#define DRV_FIELD_ENTRY_MEM_ALLOC       0x1

/* entry copy from dst to src */
#define DRV_FIELD_ENTRY_MEM_COPY        0x2

/* clear the drv_entry data & mask */
#define DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK       0x3
/* end of the op for drv_fp_entry_mem_control*/

/* op for drv_fp_entry_tcam_control*/
/* set param1 as slice id ,param2 as slice_bmp in drv_entry */
#define DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET   0x1

/* give an index to clear this drv_entry from TCAM. param1:tcam_index param2: chain_index */
#define DRV_FIELD_ENTRY_TCAM_CLEAR      0x2

/* give an index to unset entry's valid bit, param1:tcam_index param2: chain_index  */
#define DRV_FIELD_ENTRY_TCAM_REMOVE     0x3

/* move drv_entry only or both entry & param2(counter) by param1(amount) */
#define DRV_FIELD_ENTRY_TCAM_MOVE       0x4

/* block of entries move. drv_entry: src_idx, param1: dst_idx, param2: amount of blocks */
#define DRV_FIELD_ENTRY_TCAM_BLOCK_MOVE 0x5

/*write the drv_entry and act to TCAM, param1:tcam_index.*/
#define DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL 0x6

/* assign the param1 to drv_entry->id */
#define DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET   0x7

/* give an index to clear this drv_entry data and mask only from TCAM. param1:tcam_index */
#define DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR      0x8

/* assign the param1 to drv_entry->flags */
#define DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET       0x9

/* mode:chain_mode get from the drv_entry by the giving slice_Id */
#define DRV_FIELD_ENTRY_TCAM_CHAIN_MODE_GET     0xa

/* destroy the memory crated for the chain entry support */
#define DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY  0xb

/* reinstall the entry when the parity check error happened. param1: error index, param2: chain mode or not */
#define DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL 0xc

/* write the meter to TCAM, param1:tcam_index. param2: chain_index */
#define DRV_FIELD_ENTRY_TCAM_METER_INSTALL 0xd
/* end of op for drv_fp_entry_tcam_control*/


#define DRV_FIELD_POLICER_MODE_SUPPORT  (1<<0)
#define DRV_FIELD_POLICER_CONFIG   (1<<1)
#define DRV_FIELD_POLICER_FREE          (1<<2)

#define _DRV_FP_ENTRY_CHAIN 0x1

#define _DRV_FP_STAT_OP_COUNTER_MODE    (1 << 0)
#define _DRV_FP_STAT_OP_STAT_MODE   (1 << 1)

typedef struct drv_fp_tcam_checksum_s{
    uint32  tcam_error;
    uint32  stage_ingress_addr;
    uint32  stage_lookup_addr;
    uint32  stage_egress_addr;
} drv_fp_tcam_checksum_t;

/*
 * Debugging output macros.
 */

#define DRV_FP_DEBUG(flags, stuff) SOC_DEBUG((flags) | SOC_DBG_TCAM, stuff)
#define DRV_FP_OUT(stuff)          SOC_DEBUG(SOC_DBG_TCAM, stuff)
#define DRV_FP_WARN(stuff)         DRV_FP_DEBUG(SOC_DBG_WARN, stuff)
#define DRV_FP_ERR(stuff)          DRV_FP_DEBUG(SOC_DBG_ERR, stuff)
#define DRV_FP_VERB(stuff)         DRV_FP_DEBUG(SOC_DBG_VERBOSE, stuff)
#define DRV_FP_VVERB(stuff)        DRV_FP_DEBUG(SOC_DBG_VVERBOSE, stuff)

#define DRV_FP_SHOW(stuff)         ((*soc_cm_print) stuff)



/* Only for BCM53242 */
#define FP_BCM53242_L2_FRM_FMT_OTHERS   0x00
#define FP_BCM53242_L2_FRM_FMT_ETHER_II 0x01
#define FP_BCM53242_L2_FRM_FMT_SNAP     0x02
#define FP_BCM53242_L2_FRM_FMT_MASK     0x03

#define FP_BCM53242_L3_FRM_FMT_OTHERS   0x00
#define FP_BCM53242_L3_FRM_FMT_IP4      0x01
#define FP_BCM53242_L3_FRM_FMT_IP6      0x02
#define FP_BCM53242_L3_FRM_FMT_MASK     0x03

#define FP_BCM53242_L4_FRM_FMT_OTHERS   0x00
#define FP_BCM53242_L4_FRM_FMT_TCP      0x01
#define FP_BCM53242_L4_FRM_FMT_UDP      0x02
#define FP_BCM53242_L4_FRM_FMT_ICMPIGMP 0x03
#define FP_BCM53242_L4_FRM_FMT_MASK     0x03

#define FP_ACT_CHANGE_FWD_ALL_PORTS      0x3f
#define FP_ACT_CHANGE_FWD_MIRROT_TO_PORT 0x40

/* BCM53115 Vulcan */
#define FP_BCM53115_L3_FRM_FMT_IP4      0x00
#define FP_BCM53115_L3_FRM_FMT_IP6      0x01
#define FP_BCM53115_L3_FRM_FMT_NON_IP   0x03
#define FP_BCM53115_L3_FRM_FMT_MASK     0x03

#define FP_BCM53115_CHAIN_IDX_MAX 255


/* TunderBolt */
#define FP_TB_L3_FRM_FMT_IP4      0x00
#define FP_TB_L3_FRM_FMT_IP6      0x01
#define FP_TB_L3_FRM_FMT_NON_IP   0x03
#define FP_TB_L3_FRM_FMT_MASK     0x03
#define FP_TB_L3_FRM_FMT_CHAIN  0x2

/* ram type */
typedef enum drv_cfp_ram_e {
    DRV_CFP_RAM_TCAM = 1,
    DRV_CFP_RAM_TCAM_MASK,
    DRV_CFP_RAM_ACT,
    DRV_CFP_RAM_METER,
    DRV_CFP_RAM_STAT_IB,
    DRV_CFP_RAM_STAT_OB,
    DRV_CFP_RAM_TCAM_INVALID, /* write an invalid tcam entry */
    DRV_CFP_RAM_ALL
}drv_cfp_ram_t;

/* control type */
typedef enum drv_cfp_control_e {
    DRV_CFP_ENABLE,
    DRV_CFP_FLOOD_VLAN,
    DRV_CFP_FLOOD_TRUNK,
    DRV_CFP_ISPVLAN_DELIMITER,
    DRV_CFP_LLC_ENCAP,
    DRV_CFP_SLICE_SELECT,
    DRV_CFP_TCAM_RESET,
    DRV_CFP_OTHER_RAM_CLEAR, /* Action, Meter, Statistic */
    DRV_CFP_UDP_DEFAULT_ACTION_ENABLE, /* Enable the UDP wildcard rule */
    DRV_CFP_UDP_DEFAULT_ACTION,  /* Action for UDP wildcard rule */
    DRV_CFP_BYPASS_VLAN_CHECK, 
    DRV_CFP_CTRL_COUNT
}drv_cfp_control_t;

/* action type */
typedef enum drv_cfp_action_e {
    DRV_CFP_ACT_IB_NONE = 1,
    DRV_CFP_ACT_IB_APPEND,
    DRV_CFP_ACT_IB_MOD_INT_PRI,
    DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL,
    DRV_CFP_ACT_IB_DROP,
    DRV_CFP_ACT_IB_FLOOD,
    DRV_CFP_ACT_IB_REDIRECT,
    DRV_CFP_ACT_IB_MOD_PRI,
    DRV_CFP_ACT_IB_MOD_PRI_CANCEL,
    DRV_CFP_ACT_IB_MOD_TOS,
    DRV_CFP_ACT_IB_MOD_TOS_CANCEL,
    DRV_CFP_ACT_IB_DSCP_NEW,
    DRV_CFP_ACT_IB_DSCP_CANCEL,
    DRV_CFP_ACT_OB_NONE,
    DRV_CFP_ACT_OB_APPEND,
    DRV_CFP_ACT_OB_MOD_INT_PRI,
    DRV_CFP_ACT_OB_MOD_INT_PRI_CANCEL,
    DRV_CFP_ACT_OB_DROP,
    DRV_CFP_ACT_OB_FLOOD,
    DRV_CFP_ACT_OB_REDIRECT,
    DRV_CFP_ACT_OB_MOD_PRI,
    DRV_CFP_ACT_OB_MOD_PRI_CANCEL,
    DRV_CFP_ACT_OB_MOD_TOS,
    DRV_CFP_ACT_OB_MOD_TOS_CANCEL,
    DRV_CFP_ACT_OB_DSCP_NEW,
    DRV_CFP_ACT_OB_DSCP_CANCEL,
    DRV_CFP_ACT_PCP_NEW,
    DRV_CFP_ACT_PCP_CANCEL,
    DRV_CFP_ACT_CHANGE_CVID,
    DRV_CFP_ACT_CHANGE_CVID_CANCEL,
    DRV_CFP_ACT_CHANGE_SPVID,
    DRV_CFP_ACT_CHANGE_SPVID_CANCEL,
    DRV_CFP_ACT_CHANGE_CVID_SPVID,
    DRV_CFP_ACT_CHANGE_CVID_SPVID_CANCEL,
    DRV_CFP_ACT_IB_COSQ_NEW,
    DRV_CFP_ACT_IB_COSQ_CANCEL,
    DRV_CFP_ACT_OB_COSQ_NEW,
    DRV_CFP_ACT_OB_COSQ_CANCEL,
    DRV_CFP_ACT_COSQ_CPU_NEW,
    DRV_CFP_ACT_COSQ_CPU_CANCEL,
    /* add for BCM53115 */
    DRV_CFP_ACT_CHAIN_ID,
    DRV_CFP_ACT_CLASSFICATION_ID,
    DRV_CFP_ACT_CHANGE_TC,
    DRV_CFP_ACT_CHANGE_TC_CANCEL,
    DRV_CFP_ACT_LOOPBACK,
    DRV_CFP_ACT_REASON_CODE,
    DRV_CFP_ACT_STP_BYPASS,
    DRV_CFP_ACT_EAP_BYPASS,
    DRV_CFP_ACT_VLAN_BYPASS,
    /* TB */    
    DRV_CFP_ACT_CHAIN_ID_CANCEL,
    DRV_CFP_ACT_NEW_VLAN,
    DRV_CFP_ACT_CHANGE_FWD,
    DRV_CFP_ACT_CHANGE_FWD_CANCEL,
    DRV_CFP_ACT_REDIRECT_MGID,    
    DRV_CFP_ACT_REDIRECT_VPORT_PORT,
    DRV_CFP_ACT_DROP,
    DRV_CFP_ACT_IB_CHANGE_DP,
    DRV_CFP_ACT_OB_CHANGE_DP,
    DRV_CFP_ACT_IB_CHANGE_DP_CANCEL,
    DRV_CFP_ACT_OB_CHANGE_DP_CANCEL,
    DRV_CFP_ACT_IB_NEW_DP,
    DRV_CFP_ACT_OB_NEW_DP,
    DRV_CFP_ACT_NEW_TC,
    DRV_CFP_ACT_CPU_COPY,
    DRV_CFP_ACT_CPU_COPY_CANCEL,
    DRV_CFP_ACT_MIRROR_COPY,
    DRV_CFP_ACT_MIRROR_COPY_CANCEL,
    DRV_CFP_ACT_RATE_VIOLATE_DROP,
    DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL,
    DRV_CFP_ACT_DA_KNOWN,
    DRV_CFP_ACT_DA_KNOWN_CANCEL,
    DRV_CFP_ACT_DIS_LRN,
    DRV_CFP_ACT_DIS_LRN_CANCEL,
    DRV_CFP_ACT_FILTER_LAG,
    DRV_CFP_ACT_FILTER_TAGGED,
    DRV_CFP_ACT_FILTER_PORT_MASK,
    DRV_CFP_ACT_FILTER_STP,
    DRV_CFP_ACT_FILTER_EAP,
    DRV_CFP_ACT_FILTER_INGRESS_VLAN,
    DRV_CFP_ACT_FILTER_EGRESS_VLAN,
    DRV_CFP_ACT_FILTER_SA,
    DRV_CFP_ACT_FILTER_ALL,
    /* BCM53128 */
    DRV_CFP_ACT_APPEND
               
}drv_cfp_action_t;

#define DRV_FIELD_ACT_CHANGE_FWD_CPU                  0x1
#define DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE       0x2

/* search flags */
typedef enum drv_cfp_search_flag_e {
    DRV_CFP_SEARCH_START = 0x1,
    DRV_CFP_SEARCH_GET = 0x2,
    DRV_CFP_SEARCH_DONE = 0x4,
    DRV_CFP_SEARCH_BY_PORT = 0x8
}drv_cfp_search_flasg_t;

/* meter type */
typedef enum drv_cfp_meter_type_e {
    DRV_CFP_METER_QUOTA,
    DRV_CFP_METER_START,
    DRV_CFP_METER_BUCKET_SIZE,
    DRV_CFP_METER_RATE
}drv_cfp_meter_type_t;

/* counter type */
typedef enum drv_cfp_stat_type_e {
    DRV_CFP_STAT_GREEN,
    DRV_CFP_STAT_INBAND = DRV_CFP_STAT_GREEN,
    DRV_CFP_STAT_YELLOW,
    DRV_CFP_STAT_RED,
    DRV_CFP_STAT_OUTBAND = DRV_CFP_STAT_RED,
    DRV_CFP_STAT_ALL,
    DRV_CFP_STAT_COUNT
}drv_cfp_stat_type_t;

/* qualify set */
typedef enum drv_cfp_qualify_e {
    DRV_CFP_QUAL_SRC_PORT = 0,
    DRV_CFP_QUAL_1QTAG,
    DRV_CFP_QUAL_SPTAG,
    DRV_CFP_QUAL_8023_OR_EII,
    DRV_CFP_QUAL_8022_LLC,
    DRV_CFP_QUAL_8022_SNAP,
    DRV_CFP_QUAL_MAC_DA,
    DRV_CFP_QUAL_MAC_SA,
    DRV_CFP_QUAL_USR_VID,
    DRV_CFP_QUAL_USR_PRI,
    DRV_CFP_QUAL_USR_CFI,
    DRV_CFP_QUAL_SP_VID,
    DRV_CFP_QUAL_SP_PRI,
    DRV_CFP_QUAL_SP_CFI,
    DRV_CFP_QUAL_ETYPE,
    DRV_CFP_QUAL_IP_DA,
    DRV_CFP_QUAL_IP_SA,
    DRV_CFP_QUAL_IPV4,
    DRV_CFP_QUAL_IP_SAME,
    DRV_CFP_QUAL_TCPUDP,
    DRV_CFP_QUAL_UDP,
    DRV_CFP_QUAL_L4_DST,
    DRV_CFP_QUAL_L4_SRC,
    DRV_CFP_QUAL_L4_SAME,
    DRV_CFP_QUAL_L4SRC_LESS1024,
    DRV_CFP_QUAL_L4_PORTS,
    DRV_CFP_QUAL_TCP,
    DRV_CFP_QUAL_TCP_SEQ_ZERO,
    DRV_CFP_QUAL_TCP_HDR_LEN,
    DRV_CFP_QUAL_TCP_FLAG,
    DRV_CFP_QUAL_IP_PROTO,
    DRV_CFP_QUAL_IP_VER,
    DRV_CFP_QUAL_IP_TOS,
    DRV_CFP_QUAL_IP_TTL,
    DRV_CFP_QUAL_UDF0,
    DRV_CFP_QUAL_UDF2,
    DRV_CFP_QUAL_UDF3A,
    DRV_CFP_QUAL_UDF3B,
    DRV_CFP_QUAL_UDF3C,
    DRV_CFP_QUAL_UDF4A,
    DRV_CFP_QUAL_UDF4B,
    DRV_CFP_QUAL_UDF4C,
    DRV_CFP_QUAL_UDF4D,
    DRV_CFP_QUAL_UDF4E,
    /* New TCAM qualify of BCM53242 */
    DRV_CFP_QUAL_SRC_PBMP,
    DRV_CFP_QUAL_L2_FRM_FORMAT,
    DRV_CFP_QUAL_L3_FRM_FORMAT,
    DRV_CFP_QUAL_L4_FRM_FORMAT,
    DRV_CFP_QUAL_VLAN_RANGE,
    DRV_CFP_QUAL_L4_PORT_RANGE,
    DRV_CFP_QUAL_IP6_FLOW_ID,
    DRV_CFP_QUAL_IP6_SA,
    DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
    DRV_CFP_QUAL_IP6_NEXT_HEADER,
    DRV_CFP_QUAL_IP6_HOP_LIMIT,
    DRV_CFP_QUAL_IPV6,
    DRV_CFP_QUAL_ICMPIGMP_TYPECODE,
    DRV_CFP_QUAL_BIG_ICMP_CHECK,
    DRV_CFP_QUAL_UDFA0,
    DRV_CFP_QUAL_UDFA1,
    DRV_CFP_QUAL_UDFA2,
    DRV_CFP_QUAL_UDFA3,
    DRV_CFP_QUAL_UDFA4,
    DRV_CFP_QUAL_UDFA5,
    DRV_CFP_QUAL_UDFA6,
    DRV_CFP_QUAL_UDFA7,
    DRV_CFP_QUAL_UDFA8,
    DRV_CFP_QUAL_UDFA9,
    DRV_CFP_QUAL_UDFA10,
    DRV_CFP_QUAL_UDFA11,
    DRV_CFP_QUAL_UDFA12,
    DRV_CFP_QUAL_UDFA13,
    DRV_CFP_QUAL_UDFA14,
    DRV_CFP_QUAL_UDFA15,
    DRV_CFP_QUAL_UDFA16,
    DRV_CFP_QUAL_UDFA17,
    DRV_CFP_QUAL_UDFA18,
    DRV_CFP_QUAL_UDFA19,
    DRV_CFP_QUAL_UDFA20,
    DRV_CFP_QUAL_UDFA21,
    DRV_CFP_QUAL_UDFA22,
    DRV_CFP_QUAL_UDFA23,
    DRV_CFP_QUAL_UDFA24,
    DRV_CFP_QUAL_UDFA25,
    DRV_CFP_QUAL_UDFA26,
    DRV_CFP_QUAL_UDFB0,
    DRV_CFP_QUAL_UDFB1,
    DRV_CFP_QUAL_UDFB2,
    DRV_CFP_QUAL_UDFB3,
    DRV_CFP_QUAL_UDFB4,
    DRV_CFP_QUAL_UDFB5,
    DRV_CFP_QUAL_UDFB6,
    DRV_CFP_QUAL_UDFB7,
    DRV_CFP_QUAL_UDFB8,
    DRV_CFP_QUAL_UDFB9,
    DRV_CFP_QUAL_UDFB10,
    DRV_CFP_QUAL_UDFB11,
    DRV_CFP_QUAL_UDFB12,
    DRV_CFP_QUAL_UDFB13,
    DRV_CFP_QUAL_UDFB14,
    DRV_CFP_QUAL_UDFB15,
    DRV_CFP_QUAL_UDFB16,
    DRV_CFP_QUAL_UDFB17,
    DRV_CFP_QUAL_UDFB18,
    DRV_CFP_QUAL_UDFB19,
    DRV_CFP_QUAL_UDFB20,
    DRV_CFP_QUAL_UDFB21,
    DRV_CFP_QUAL_UDFB22,
    DRV_CFP_QUAL_UDFB23,
    DRV_CFP_QUAL_UDFB24,
    DRV_CFP_QUAL_UDFB25,
    DRV_CFP_QUAL_UDFB26,
    DRV_CFP_QUAL_UDFC0,
    DRV_CFP_QUAL_UDFC1,
    DRV_CFP_QUAL_UDFC2,
    DRV_CFP_QUAL_UDFC3,
    DRV_CFP_QUAL_UDFC4,
    DRV_CFP_QUAL_UDFC5,
    DRV_CFP_QUAL_UDFC6,
    DRV_CFP_QUAL_UDFC7,
    DRV_CFP_QUAL_UDFC8,
    DRV_CFP_QUAL_UDFC9,
    DRV_CFP_QUAL_UDFC10,
    DRV_CFP_QUAL_UDFC11,
    DRV_CFP_QUAL_UDFC12,
    DRV_CFP_QUAL_UDFC13,
    DRV_CFP_QUAL_UDFC14,
    DRV_CFP_QUAL_UDFC15,
    DRV_CFP_QUAL_UDFC16,
    DRV_CFP_QUAL_UDFC17,
    DRV_CFP_QUAL_UDFC18,
    DRV_CFP_QUAL_UDFC19,
    DRV_CFP_QUAL_UDFC20,
    DRV_CFP_QUAL_UDFC21,
    DRV_CFP_QUAL_UDFC22,
    DRV_CFP_QUAL_UDFC23,
    DRV_CFP_QUAL_UDFC24,
    DRV_CFP_QUAL_UDFC25,
    DRV_CFP_QUAL_UDFC26,
    DRV_CFP_QUAL_UDFD0,
    DRV_CFP_QUAL_UDFD1,
    DRV_CFP_QUAL_UDFD2,
    DRV_CFP_QUAL_UDFD3,
    DRV_CFP_QUAL_UDFD4,
    DRV_CFP_QUAL_UDFD5,
    DRV_CFP_QUAL_UDFD6,
    DRV_CFP_QUAL_UDFD7,
    DRV_CFP_QUAL_UDFD8,
    DRV_CFP_QUAL_UDFD9,
    DRV_CFP_QUAL_UDFD10,
    DRV_CFP_QUAL_UDFD11,
    /* Add for BCM5395 */
    DRV_CFP_QUAL_UDF5A,
    DRV_CFP_QUAL_UDF5B,
    DRV_CFP_QUAL_UDF5C,
    DRV_CFP_QUAL_UDF5D,
    DRV_CFP_QUAL_UDF5E,
    /* add for BCM53115 */
    DRV_CFP_QUAL_IP_FRGA,
    DRV_CFP_QUAL_IP_NON_FIRST_FRGA,
    DRV_CFP_QUAL_IP_AUTH,
    DRV_CFP_QUAL_CHAIN_ID,
    DRV_CFP_QUAL_IP6_DA,
    DRV_CFP_QUAL_SNAP_HEADER,
    DRV_CFP_QUAL_LLC_HEADER,
    DRV_CFP_QUAL_CLASS_ID, /* To indicate slice can set the class ID action */

    DRV_CFP_QUAL_SRC_PROFILE,
    DRV_CFP_QUAL_INVALID,
    DRV_CFP_QUAL_COUNT
}drv_cfp_qual_t;

/* field set */
typedef enum drv_cfp_field_e {
    /* TCAM */
    DRV_CFP_FIELD_VALID,
    DRV_CFP_FIELD_SLICE_ID,
    DRV_CFP_FIELD_SRC_PORT,
    DRV_CFP_FIELD_1QTAGGED,
    DRV_CFP_FIELD_SPTAGGED,
    DRV_CFP_FIELD_EII_OR_8023,
    DRV_CFP_FIELD_BRCM_TAGGED,
    DRV_CFP_FIELD_IEEE_LLC,
    DRV_CFP_FIELD_IEEE_SNAP,
    DRV_CFP_FIELD_MAC_DA,
    DRV_CFP_FIELD_MAC_SA,
    DRV_CFP_FIELD_SP_PRI,
    DRV_CFP_FIELD_SP_CFI,
    DRV_CFP_FIELD_SP_VID,
    DRV_CFP_FIELD_USR_PRI,
    DRV_CFP_FIELD_USR_CFI,
    DRV_CFP_FIELD_USR_VID,
    DRV_CFP_FIELD_ETYPE,
    DRV_CFP_FIELD_UDF0_VALID,
    DRV_CFP_FIELD_UDF0,
    DRV_CFP_FIELD_IPV4_VALID,
    DRV_CFP_FIELD_IP_DA,
    DRV_CFP_FIELD_IP_SA,
    DRV_CFP_FIELD_SAME_IP,
    DRV_CFP_FIELD_TCPUDP_VALID,
    DRV_CFP_FIELD_UDP_VALID,
    DRV_CFP_FIELD_TCP_VALID,
    DRV_CFP_FIELD_L4DST,
    DRV_CFP_FIELD_L4SRC,
    DRV_CFP_FIELD_SAME_L4PORT,
    DRV_CFP_FIELD_L4SRC_LESS1024,
    DRV_CFP_FIELD_TCP_FRAME,
    DRV_CFP_FIELD_TCP_SEQ_ZERO,
    DRV_CFP_FIELD_TCP_HDR_LEN,
    DRV_CFP_FIELD_TCP_FLAG,
    DRV_CFP_FIELD_UDF2_VALID,
    DRV_CFP_FIELD_UDF2,
    DRV_CFP_FIELD_IP_PROTO,
    DRV_CFP_FIELD_IP_VER,
    DRV_CFP_FIELD_IP_TOS,
    DRV_CFP_FIELD_IP_TTL,
    DRV_CFP_FIELD_UDF3A_VALID,
    DRV_CFP_FIELD_UDF3A,
    DRV_CFP_FIELD_UDF3B_VALID,
    DRV_CFP_FIELD_UDF3B,
    DRV_CFP_FIELD_UDF3C_VALID,
    DRV_CFP_FIELD_UDF3C,
    DRV_CFP_FIELD_UDF4A_VALID,
    DRV_CFP_FIELD_UDF4A,
    DRV_CFP_FIELD_UDF4B_VALID,
    DRV_CFP_FIELD_UDF4B,
    DRV_CFP_FIELD_UDF4C_VALID,
    DRV_CFP_FIELD_UDF4C,
    DRV_CFP_FIELD_UDF4D_VALID,
    DRV_CFP_FIELD_UDF4D,
    DRV_CFP_FIELD_UDF4E_VALID,
    DRV_CFP_FIELD_UDF4E,
    /* New TCAM fields of BCM53242 */
    DRV_CFP_FIELD_IN_PBMP,
    DRV_CFP_FIELD_L2_FRM_FORMAT,
    DRV_CFP_FIELD_L3_FRM_FORMAT,
    DRV_CFP_FIELD_L4_FRM_FORMAT,
    DRV_CFP_FIELD_VLAN_RANGE,
    DRV_CFP_FIELD_L4_PORT_RANGE,
    DRV_CFP_FIELD_IP6_FLOW_ID,
    DRV_CFP_FIELD_IP6_SA,
    DRV_CFP_FIELD_IP6_TRAFFIC_CLASS,
    DRV_CFP_FIELD_IP6_NEXT_HEADER,
    DRV_CFP_FIELD_IP6_HOP_LIMIT,
    DRV_CFP_FIELD_ICMPIGMP_TYPECODE,
    DRV_CFP_FIELD_BIG_ICMP_CHECK,
    DRV_CFP_FIELD_UDFA0,
    DRV_CFP_FIELD_UDFA1,
    DRV_CFP_FIELD_UDFA2,
    DRV_CFP_FIELD_UDFA3,
    DRV_CFP_FIELD_UDFA4,
    DRV_CFP_FIELD_UDFA5,
    DRV_CFP_FIELD_UDFA6,
    DRV_CFP_FIELD_UDFA7,
    DRV_CFP_FIELD_UDFA8,
    DRV_CFP_FIELD_UDFA0_VALID,
    DRV_CFP_FIELD_UDFA1_VALID,
    DRV_CFP_FIELD_UDFA2_VALID,
    DRV_CFP_FIELD_UDFA3_VALID,
    DRV_CFP_FIELD_UDFA4_VALID,
    DRV_CFP_FIELD_UDFA5_VALID,
    DRV_CFP_FIELD_UDFA6_VALID,
    DRV_CFP_FIELD_UDFA7_VALID,
    DRV_CFP_FIELD_UDFA8_VALID,
    DRV_CFP_FIELD_UDFB0,
    DRV_CFP_FIELD_UDFB1,
    DRV_CFP_FIELD_UDFB2,
    DRV_CFP_FIELD_UDFB3,
    DRV_CFP_FIELD_UDFB4,
    DRV_CFP_FIELD_UDFB5,
    DRV_CFP_FIELD_UDFB6,
    DRV_CFP_FIELD_UDFB7,
    DRV_CFP_FIELD_UDFB8,
    DRV_CFP_FIELD_UDFB9,
    DRV_CFP_FIELD_UDFB10,
    DRV_CFP_FIELD_UDFB0_VALID,
    DRV_CFP_FIELD_UDFB1_VALID,
    DRV_CFP_FIELD_UDFB2_VALID,
    DRV_CFP_FIELD_UDFB3_VALID,
    DRV_CFP_FIELD_UDFB4_VALID,
    DRV_CFP_FIELD_UDFB5_VALID,
    DRV_CFP_FIELD_UDFB6_VALID,
    DRV_CFP_FIELD_UDFB7_VALID,
    DRV_CFP_FIELD_UDFB8_VALID,
    DRV_CFP_FIELD_UDFB9_VALID,
    DRV_CFP_FIELD_UDFB10_VALID,
    DRV_CFP_FIELD_UDFC0,
    DRV_CFP_FIELD_UDFC1,
    DRV_CFP_FIELD_UDFC2,
    DRV_CFP_FIELD_UDFC3,
    DRV_CFP_FIELD_UDFC4,
    DRV_CFP_FIELD_UDFC5,
    DRV_CFP_FIELD_UDFC6,
    DRV_CFP_FIELD_UDFC7,
    DRV_CFP_FIELD_UDFC8,
    DRV_CFP_FIELD_UDFC0_VALID,
    DRV_CFP_FIELD_UDFC1_VALID,
    DRV_CFP_FIELD_UDFC2_VALID,
    DRV_CFP_FIELD_UDFC3_VALID,
    DRV_CFP_FIELD_UDFC4_VALID,
    DRV_CFP_FIELD_UDFC5_VALID,
    DRV_CFP_FIELD_UDFC6_VALID,
    DRV_CFP_FIELD_UDFC7_VALID,
    DRV_CFP_FIELD_UDFC8_VALID,
    DRV_CFP_FIELD_UDFD0,
    DRV_CFP_FIELD_UDFD1,
    DRV_CFP_FIELD_UDFD2,
    DRV_CFP_FIELD_UDFD3,
    DRV_CFP_FIELD_UDFD4,
    DRV_CFP_FIELD_UDFD5,
    DRV_CFP_FIELD_UDFD6,
    DRV_CFP_FIELD_UDFD7,
    DRV_CFP_FIELD_UDFD8,
    DRV_CFP_FIELD_UDFD9,
    DRV_CFP_FIELD_UDFD10,
    DRV_CFP_FIELD_UDFD11,
    DRV_CFP_FIELD_UDFD0_VALID,
    DRV_CFP_FIELD_UDFD1_VALID,
    DRV_CFP_FIELD_UDFD2_VALID,
    DRV_CFP_FIELD_UDFD3_VALID,
    DRV_CFP_FIELD_UDFD4_VALID,
    DRV_CFP_FIELD_UDFD5_VALID,
    DRV_CFP_FIELD_UDFD6_VALID,
    DRV_CFP_FIELD_UDFD7_VALID,
    DRV_CFP_FIELD_UDFD8_VALID,
    DRV_CFP_FIELD_UDFD9_VALID,
    DRV_CFP_FIELD_UDFD10_VALID,
    DRV_CFP_FIELD_UDFD11_VALID,
    /* Add for BCM5395 */
    DRV_CFP_FIELD_UDF5A_VALID,
    DRV_CFP_FIELD_UDF5A,
    DRV_CFP_FIELD_UDF5B_VALID,
    DRV_CFP_FIELD_UDF5B,
    DRV_CFP_FIELD_UDF5C_VALID,
    DRV_CFP_FIELD_UDF5C,
    DRV_CFP_FIELD_UDF5D_VALID,
    DRV_CFP_FIELD_UDF5D,
    DRV_CFP_FIELD_UDF5E_VALID,
    DRV_CFP_FIELD_UDF5E,
    /* Add for BCM53115 */
    DRV_CFP_FIELD_IP_FRAG,
    DRV_CFP_FIELD_IP_NON_FIRST_FRAG,
    DRV_CFP_FIELD_IP_AUTH,
    DRV_CFP_FIELD_TTL_RANGE,
    DRV_CFP_FIELD_CHAIN_ID,
    DRV_CFP_FIELD_IP6_DA,
    DRV_CFP_FIELD_SNAP_HEADER,
    DRV_CFP_FIELD_LLC_HEADER,
    /* BCM53280*/
    DRV_CFP_FIELD_UDF_ALL_VALID,
    DRV_CFP_FIELD_UDF11_VALID,
    DRV_CFP_FIELD_UDF10_VALID,
    DRV_CFP_FIELD_UDF9_VALID,    
    DRV_CFP_FIELD_UDF8_VALID,    
    DRV_CFP_FIELD_UDF7_VALID,    
    DRV_CFP_FIELD_UDF6_VALID,    
    DRV_CFP_FIELD_UDF5_VALID,        
    DRV_CFP_FIELD_UDF4_VALID,    
    DRV_CFP_FIELD_UDF3_VALID,    
    DRV_CFP_FIELD_UDF1_VALID,        
    DRV_CFP_FIELD_UDFA4_0,
    DRV_CFP_FIELD_UDFA3_0,
    DRV_CFP_FIELD_UDFA2_0,
    DRV_CFP_FIELD_UDFA1_0,    
    DRV_CFP_FIELD_UDFA0_0,
    DRV_CFP_FIELD_UDFA1_1,
    DRV_CFP_FIELD_UDFA0_1,
    DRV_CFP_FIELD_UDFA8_2,
    DRV_CFP_FIELD_UDFA7_2,
    DRV_CFP_FIELD_UDFA6_2,
    DRV_CFP_FIELD_UDFA5_2,
    DRV_CFP_FIELD_UDFA4_2,
    DRV_CFP_FIELD_UDFA3_2,
    DRV_CFP_FIELD_UDFA2_2,
    DRV_CFP_FIELD_UDFA1_2,    
    DRV_CFP_FIELD_UDFA0_2,    
    DRV_CFP_FIELD_UDFB8_0,    
    DRV_CFP_FIELD_UDFB8_2,
    DRV_CFP_FIELD_UDFB7_2,
    DRV_CFP_FIELD_UDFB6_2,
    DRV_CFP_FIELD_UDFB5_2,
    DRV_CFP_FIELD_UDFB4_2,
    DRV_CFP_FIELD_UDFB3_2,
    DRV_CFP_FIELD_UDFB2_2,
    DRV_CFP_FIELD_UDFB1_2,    
    DRV_CFP_FIELD_UDFB0_2,
    DRV_CFP_FIELD_SRC_PROFILE,
    /* ACTION RAM */
    DRV_CFP_FIELD_MOD_PRI_EN_IB,
    DRV_CFP_FIELD_MOD_PRI_MAP_IB,
    DRV_CFP_FIELD_MOD_TOS_EN_IB,
    DRV_CFP_FIELD_MOD_TOS_IB,
    DRV_CFP_FIELD_REDIRECT_EN_IB,
    DRV_CFP_FIELD_ADD_CHANGE_DEST_IB,
    DRV_CFP_FIELD_NEW_DEST_IB,
    DRV_CFP_FIELD_MOD_PRI_EN_OB,
    DRV_CFP_FIELD_MOD_PRI_MAP_OB,
    DRV_CFP_FIELD_MOD_TOS_EN_OB,
    DRV_CFP_FIELD_MOD_TOS_OB,
    DRV_CFP_FIELD_REDIRECT_EN_OB,
    DRV_CFP_FIELD_ADD_CHANGE_DEST_OB,
    DRV_CFP_FIELD_NEW_DEST_OB,
    /* ACTION RAM of BCM53242*/
    DRV_CFP_FIELD_CHANGE_DSCP_OB_EN,
    DRV_CFP_FIELD_NEW_DSCP_OB,
    DRV_CFP_FIELD_CHANGE_FWD_OB_EN,
    DRV_CFP_FIELD_NEW_FWD_OB,
    DRV_CFP_FIELD_CHANGE_FLOW_EN,
    DRV_CFP_FIELD_NEW_FLOW_INDEX,
    DRV_CFP_FIELD_CHANGE_DSCP_IB_EN,
    DRV_CFP_FIELD_NEW_DSCP_IB,
    DRV_CFP_FIELD_CHANGE_PCP_EN,
    DRV_CFP_FIELD_NEW_PCP,
    DRV_CFP_FIELD_CHANGE_COS_EN,
    DRV_CFP_FIELD_NEW_COS,
    DRV_CFP_FIELD_CHANGE_COS_IMP_EN,
    DRV_CFP_FIELD_NEW_COS_IMP,
    DRV_CFP_FIELD_CHANGE_FWD_IB_EN,
    DRV_CFP_FIELD_NEW_FWD_IB,
    /* Add for BCM53115 */
    DRV_CFP_FIELD_CHANGE_TC,
    DRV_CFP_FIELD_NEW_TC,
    DRV_CFP_FIELD_ACTION_CHAIN,
    DRV_CFP_FIELD_ACTION_LOOPBACK_EN,
    DRV_CFP_FIELD_ACTION_REASON,
    DRV_CFP_FIELD_ACTION_STP_BYPASS,
    DRV_CFP_FIELD_ACTION_EAP_BYPASS,
    DRV_CFP_FIELD_ACTION_VLAN_BYPASS,
    /* Add for BCM53128 */
    DRV_CFP_FIELD_L4PORTS,

    /* METER RAM */
    DRV_CFP_FIELD_CURR_QUOTA,
    DRV_CFP_FIELD_RATE_REFRESH_EN,
    DRV_CFP_FIELD_REF_CAP,
    DRV_CFP_FIELD_RATE,
    /* METER RAM of BCM53242 */
    DRV_CFP_FIELD_BUCKET_SIZE,
    DRV_CFP_FIELD_REF_UNIT,
    DRV_CFP_FIELD_REF_CNT,

    /* IN BAND COUNTER */
    DRV_CFP_FIELD_IB_CNT,
    /* OUT BAND COUNTER */
    DRV_CFP_FIELD_OB_CNT,
    DRV_CFP_FIELD_COUNT
}drv_cfp_field_t;

typedef enum drv_cfp_range_e {
    DRV_CFP_RANGE_SRCPORT,
    DRV_CFP_RANGE_DSTPORT,
    DRV_CFP_RANGE_VLAN,
    DRV_CFP_RANGE_TCP_HEADER_LEN,
    DRV_CFP_RANGE_BIG_ICMP,
    
    DRV_CFP_RANGE_COUNT
} drv_cfp_range_t;

typedef enum drv_cfp_udf_offset_e {
    DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG,
    DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR, /* also "End of Ether Type". */
    DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR, /* also "End of IP Header". */
    DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME,

    DRV_CFP_UDF_OFFSET_BASE_COUNT
} drv_cfp_udf_offset_t;

#define DRV_FIELD_RANGE_SRCPORT                 0x00000001
#define DRV_FIELD_RANGE_DSTPORT                 0x00000002
#define DRV_FIELD_RANGE_TCP                     0x00000004
#define DRV_FIELD_RANGE_UDP                     0x00000008
#define DRV_FIELD_RANGE_INVERT                  0x00000010
#define DRV_FIELD_RANGE_EXTERNAL                0x00000020
#define DRV_FIELD_RANGE_VLAN                    0x00000040

typedef struct drv_cfp_tcam_s {
    int     chain_id;           /* chain_id in chain slice searh key */
    uint32 tcam_data[12];
    uint32 tcam_mask[12];
    uint32  act_data[3];
}drv_cfp_tcam_t;


/* driver cfp entry format */
typedef struct drv_cfp_entry_s {
    uint32  id;
    uint32  prio;
    uint32  tcam_data[12];
    uint32  tcam_mask[12];
    uint32  act_data[3];
    uint32  meter_data[3];
    uint32  slice_bmp; /* all matched and valid slices */
    SHR_BITDCL  w[_SHR_BITDCLSIZE(DRV_CFP_QUAL_COUNT)];

    int flow2vlan_sw_idx;
    int slice_id;                   /* current selected slice_id for this entry */
    uint32 flags;
    drv_cfp_tcam_t *cfp_chain;      /* used for chain slice */
    drv_policer_config_t *pl_cfg;
    drv_field_qset_t drv_qset;
}drv_cfp_entry_t;

/* flags in drv_cfp_entry_t  mostly for ThunderBolt internal use */

 /* for TB default action would bypass all filters*/
#define _DRV_CFP_ACTION_INIT  0x1                  
/* for TB fixed udf need to set the valid bit if the qualify is selected */
#define _DRV_CFP_FIX_UDF_ALL_VALID  0x2
/* once the udf is selected, framing need to be configured */
#define _DRV_CFP_UDF_VALID 0x4

/* used for auto slice select when frame type changed */
#define _DRV_CFP_FRAME_ANY   0x8 /* don't care*/
#define _DRV_CFP_FRAME_IP4   0x10
#define _DRV_CFP_FRAME_IP6   0x20
#define _DRV_CFP_FRAME_IP    0x30   /* ipv4 or ipv6*/
#define _DRV_CFP_FRAME_IPANY   0x40  /* exclude nonip */
#define _DRV_CFP_FRAME_NONIP 0x80
#define _DRV_CFP_FRAME_ALL  (_DRV_CFP_FRAME_ANY \
    |_DRV_CFP_FRAME_IP | _DRV_CFP_FRAME_IPANY \
    |_DRV_CFP_FRAME_NONIP)

/* used for chain slice support */
#define _DRV_CFP_SLICE_CHAIN        0x100           /* {slice0,slice3}*/
#define _DRV_CFP_SLICE_CHAIN_DOUBLE     _DRV_CFP_SLICE_CHAIN
#define _DRV_CFP_SLICE_CHAIN_SINGLE         0x200   /* slice3 only*/
#define _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN 0x400    /* config slice3 */
#define _DRV_CFP_SLICE_CONFIG_SLICE_MAIN 0x800   /* config slice0 */


/* end define flags in drv_cfp_entry_t */


/* flags for DRV_FP_SELCODE_MODE_GET */
#define DRV_FIELD_GROUP_MODE_SINGLE 0x0
#define DRV_FIELD_GROUP_MODE_DOUBLE 0x1
#define DRV_FIELD_GROUP_MODE_AUTO   0x2

/* flags for DRV_CFP_SLICE_ID_SELECT*/
#define DRV_FIELD_QUAL_REPLACE_BY_UDF 0x1
#define DRV_FIELD_QUAL_CHAIN 0x2
#define DRV_FIELD_QUAL_SINGLE 0x3

typedef struct drv_cfp_qual_udf_info_s {
    int valid;
    int qual;
    int udf_num;
    int udf_index[8];
    int udf_base;
    int udf_off[8];
} drv_cfp_qual_udf_info_t;

#define DRV_CFP_UDF_OFFSET_GET    0
#define DRV_CFP_UDF_QUAL_GET    0x1
#define DRV_CFP_UDF_FIELD_GET    0x2
#define DRV_CFP_UDF_SLICE_ID_GET    0x3

typedef struct {
    int chain_id_size;
    SHR_BITDCL *chain_id_used;
    int flow_id_size;
    SHR_BITDCL *flow_id_used;
    int flow_id_cpu_default;
    int ranger_id_size;
    int *ranger_id_used;    
} _fp_id_info_t;

extern _fp_id_info_t _robo_fp_id_info[BCM_MAX_NUM_UNITS];

#define FP_ID_INFO(unit)        (&_robo_fp_id_info[unit])
#define CFP_CHAIN_SIZE(unit)        FP_ID_INFO(unit)->chain_id_size
#define CFP_CHAIN_USED(unit)        FP_ID_INFO(unit)->chain_id_used
#define CFP_CHAIN_USED_SET(unit, n) SHR_BITSET(CFP_CHAIN_USED(unit), n)
#define CFP_CHAIN_USED_CLR(unit, n) SHR_BITCLR(CFP_CHAIN_USED(unit), n)
#define CFP_CHAIN_USED_ISSET(unit, n) SHR_BITGET(CFP_CHAIN_USED(unit), n)

#define VM_FLOW_SIZE(unit)      FP_ID_INFO(unit)->flow_id_size
#define VM_FLOW_USED(unit)      FP_ID_INFO(unit)->flow_id_used
#define VM_FLOW_USED_SET(unit, n) SHR_BITSET(VM_FLOW_USED(unit), n)
#define VM_FLOW_USED_CLR(unit, n) SHR_BITCLR(VM_FLOW_USED(unit), n)
#define VM_FLOW_USED_ISSET(unit, n) SHR_BITGET(VM_FLOW_USED(unit), n)
#define VM_FLOW_CPU_DEFAULT(unit)   FP_ID_INFO(unit)->flow_id_cpu_default

#define VM_RANGER_SIZE(unit)        FP_ID_INFO(unit)->ranger_id_size
#define VM_RANGER_USED(unit)        FP_ID_INFO(unit)->ranger_id_used
#define VM_RANGER_USED_INC(unit, n) VM_RANGER_USED(unit)[n] += 1
#define VM_RANGER_USED_DEC(unit, n) VM_RANGER_USED(unit)[n] -= 1
#define VM_RANGER_USED_COUNT(unit, n) VM_RANGER_USED(unit)[n]

#define _DRV_FP_ID_CFP_CHAIN_ID     0x1
#define _DRV_FP_ID_VM_FLOW_ID       0x2
#define _DRV_FP_ID_VM_RANGE_ID      0x3

#define _DRV_FP_ID_CTRL_ALLOC       0x1
#define _DRV_FP_ID_CTRL_FREE        0x2
#define _DRV_FP_ID_CTRL_INC         0x3
#define _DRV_FP_ID_CTRL_DEC         0x4
#define _DRV_FP_ID_CTRL_COUNT       0x5
#define _DRV_FP_ID_CTRL_ALLOC_CPU_DEFAULT 0x6
#define _DRV_FP_ID_CTRL_GET_CPU_DEFAULT   0x7

#define _DRV_FP_ID_GET_FROM_HEAD    0x1
#define _DRV_FP_ID_GET_FROM_TAIL    0x2


#define _DRV_FP_ACTION_PRIVATE  0xf000

int drv_cfp_init(int unit);

int drv_cfp_action_get(int unit, uint32* action, drv_cfp_entry_t* entry, 
            uint32* act_param);

int drv_cfp_action_set(int unit, uint32 action, drv_cfp_entry_t* entry, 
            uint32 act_param1, uint32 act_param2);

int drv_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2);

int drv_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2);

int drv_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);

int drv_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry);

int drv_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);

int drv_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);

int drv_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);

int drv_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)    ;

int drv_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst);

int drv_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry,
            uint32 *val);

int drv_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val);

int drv_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags);

int drv_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry);

int drv_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter);

int drv_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter);

int drv_cfp_udf_get(int unit, uint32 port, uint32 udf_index, uint32 *offset, uint32 *base);

int drv_cfp_udf_set(int unit, uint32 port, uint32 udf_index, uint32 offset, uint32 base);

int drv_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max);

int drv_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2);

int drv_cfp_sub_qual_by_udf(int unit, int enable, int slice_id, uint32 sub_qual, 
    drv_cfp_qual_udf_info_t *qual_udf_info);


#endif /* _SOC_CFP_H */

/*
 * $Id: robo_53280.h,v 1.22.6.2 Broadcom SDK $
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
 
#ifndef _ROBO_53280_H
#define _ROBO_53280_H


#define DRV_MCAST_GROUP_NUM             4096
#define DRV_AGE_TIMER_MAX               1048575
#define DRV_TRUNK_GROUP_NUM             14
#define DRV_TRUNK_MAX_PORT_NUM          8
#define DRV_MSTP_GROUP_NUM              256
#define DRV_SEC_MAC_NUM_PER_PORT        0
#define DRV_COS_QUEUE_MAX_WEIGHT_VALUE  255
#define DRV_AUTH_SUPPORT_PBMP           0x01fffffff
#define DRV_RATE_CONTROL_SUPPORT_PBMP   0x01fffffff
#define DRV_VLAN_ENTRY_NUM              4095
#define DRV_BPDU_NUM                    1
#define DRV_CFP_TCAM_SIZE               1536
#define DRV_CFP_UDFS_NUM                47
#define DRV_CFP_UDFS_OFFSET_MAX         (2 * (32 - 1))
#define DRV_CFP_RNG_NUM                 0
#define DRV_CFP_VID_RNG_NUM             4
#define DRV_MAX_MCREP_MCAST_GROUP_NUM   256
#define DRV_MAX_MCREP_VPORT_NUM         16
#define DRV_MAX_L2_USER_ADDRESS         2
#define DRV_PORT_MAX_PROFILE_NUM        4
#define DRV_FLOW_ID_NUM        4096
#define DRV_VLAN_TRANSLATE_MODE_MAX 3
#define DRV_POWER_DOWN_MODE_SUPPORT_PBMP 0xffffff

#define DRV_IVM_TCAM_SIZE(unit) \
    (SOC_IS_TB_B0(unit) ? 2048 : 1024)
#define DRV_EVM_TCAM_SIZE(unit) \
    (SOC_IS_TB_B0(unit) ? 2048 : 1024)

#define DRV_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED |\
                                        DRV_SECURITY_VIOLATION_SA_NUM |\
                                        DRV_SECURITY_VIOLATION_SA_MATCH |\
                                        DRV_SECURITY_VIOLATION_SA_MOVEMENT)

/* ------ bcm53280 Ingress/Egress Rate Control related definition ------- */
#define TB_RATE_MAX_REF_CNTS(unit)  \
    (SOC_IS_TB_B0(unit) ? (1<<17)-1 : (1<<16)-1)

/* kbps*/
#define TB_RATE_REF_COUNT_GRANULARITY(unit) \
    (SOC_IS_TB_B0(unit) ? 32 : 64)

/* packets/sec */
#define TB_RATE_REF_COUNT_PKT_GRANULARITY(unit) \
    (SOC_IS_TB_B0(unit) ? 50 : 100)

/* bytes */
#define TB_RATE_BUCKET_UNIT_SIZE(unit) \
    (SOC_IS_TB_B0(unit) ? 16 : 256)

/* bucket unit */
#define TB_RATE_MAX_BUCKET_UNIT(unit) \
    (SOC_IS_TB_B0(unit) ? (1<<16)-1 : (1<<9)-1 )


/* max burst size (bytes)  */
#define TB_RATE_MAX_BUCKET_SIZE(unit)   \
    (TB_RATE_MAX_BUCKET_UNIT(unit) * TB_RATE_BUCKET_UNIT_SIZE(unit))

#define TB_RATE_IRC_PKT_MASK(unit)  0x3f

/*kbps*/
#define TB_RATE_METER_MAX(unit)    \
    (TB_RATE_MAX_REF_CNTS(unit) * TB_RATE_REF_COUNT_GRANULARITY(unit))


/*pps*/
#define TB_RATE_METER_PKT_MAX(unit)    \
    (TB_RATE_MAX_REF_CNTS(unit) * TB_RATE_REF_COUNT_PKT_GRANULARITY(unit))

/* kbits*/
#define TB_RATE_BURST_MAX(unit) \
    ((TB_RATE_MAX_BUCKET_SIZE(unit) * 8)/1000)

#define TB_RATE_METER_MIN(unit)  0

#define TB_RATE_BURST_MIN(unit)  0

/* IRC minimum available bucket size */
#define TB_RATE_IRC_MIN_AVAIL_BUCKET_SIZE(unit) \
    (SOC_IS_TB_B0(unit) ? 257 : 9)

/* ERC minimum available bucket size */
#define TB_RATE_ERC_MIN_AVAIL_BUCKET_SIZE(unit) \
    (SOC_IS_TB_B0(unit) ? 145 : 0)

#define TB_RATE_TYPE_KBPS   0
#define TB_RATE_TYPE_PPS    1

/* kbits*/
#define TB_RATE_MAX_LIMIT_KBPS_GE_S    2500000
#define TB_RATE_MAX_LIMIT_KBPS_GE    1000000
#define TB_RATE_MAX_LIMIT_KBPS_FE    100000

/*pps (minimum packet length = 64 bytes)*/
#define TB_RATE_MAX_LIMIT_PPS_GE_S    3720000
#define TB_RATE_MAX_LIMIT_PPS_GE    1488000
#define TB_RATE_MAX_LIMIT_PPS_FE    148800

/* ------ bcm53280 Statistics related definition ------- */
#define TXRX_CTR_TYPE_RX  0
#define TXRX_CTR_TYPE_TX  1
#define TXRX_CTR_TYPE_NUM  2

#define TXRX_MAX_COUNTER_NUM(unit)  \
        (SOC_CTR_MAP_SIZE(unit, SOC_CTR_TYPE_RX) + \
        SOC_CTR_MAP_SIZE(unit, SOC_CTR_TYPE_TX))

#define RX_MAX_COUNTER_NUM(unit)  \
        (SOC_CTR_MAP_SIZE(unit, SOC_CTR_TYPE_RX))

#define MIN_INTERNAL_MII_PORT_STATE_PAGE 0xa0
#define MAX_INTERNAL_MII_PORT_STATE_PAGE 0xb7
#define MIN_EXTERNAL_MII_PORT_STATE_PAGE 0xc0
#define MAX_EXTERNAL_MII_PORT_STATE_PAGE 0xdc

#define IS_MII_PORT_STATE_PAGE(page) \
    ((page >= MIN_INTERNAL_MII_PORT_STATE_PAGE) && (page <= MAX_INTERNAL_MII_PORT_STATE_PAGE) || \
     (page >= MIN_EXTERNAL_MII_PORT_STATE_PAGE) && (page <= MAX_EXTERNAL_MII_PORT_STATE_PAGE)) ? \
    TRUE : FALSE
    
#define IS_PHY_STATUS_REGS(unit, addr) \
        ( (addr == SOC_REG_INFO(unit, LNKSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, LNKSTSCHGr).offset) || \
          (addr == SOC_REG_INFO(unit, SPDSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, DUPSTSr).offset) || \
          (addr == SOC_REG_INFO(unit, PAUSESTSr).offset) ) ? \
          TRUE : FALSE

/* ------ bcm53280 memory access related definition ------- */
/* for Mcast vPort mapping table read/write through MEM_ADDR_0r*/
#define DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm_MGRP 0xFF    /* Mcast Group id */

#define DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm_MEM_PORT  0x1F  /* Port id */
#define DRV_MEMADDR0_OFFSET_MCAST_VPORT_MAPm_MEM_PORT  8
#define DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm                   \
            (DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm_MGRP |  \
            (DRV_MEMADDR0_MCAST_VPORT_MAPm_MEM_PORT <<  \
            DRV_MEMADDR0_OFFSET_MCAST_VPORT_MAPm_MEM_PORT))
#define DRV_MCREP_GROUPID_GET(_mc_id, _port_id)  \
            (((_mc_id) & DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm_MGRP) |         \
            (((_port_id) & DRV_MEMADDR0_MASK_MCAST_VPORT_MAPm_MEM_PORT) <<  \
            DRV_MEMADDR0_OFFSET_MCAST_VPORT_MAPm_MEM_PORT))
/* the bitmap of the vports in Mcast replication table entry */
#define DRV_MCREP_VPORTS_BMPMASK        0xFFFF

/* for port basis VPORT vid mapping tabke */
#define DRV_MEMADDR0_VPORT_VID_MAPm                 0x1C

#define DRV_VPORT_MAX_ID            (DRV_MAX_MCREP_VPORT_NUM - 1)
#define DRV_VPORT_NONE              DRV_MAX_MCREP_VPORT_NUM

#define DRV_MAX_VPORT_VID       0xFFF   /* 0~4095 can be valid vPort VID */

#define _TB_ARL_INDEX_FULL_ID_MASK      0x3FFF      /* 16K l2 entry */
#define _TB_ARL_INDEX_TABLE_ID_MASK     0xFFF       /* 4K l2_table entry */
#define _TB_ARL_BIN_NUMBER              4      /* 4 bins on l2 table entry */
#define _TB_ARL_INDEX_BIN_ID_MASK       0x3
#define _TB_ARL_INDEX_TABLE_ID_SHIFT    2
#define _TB_ARL_INDEX_TABLE_ID_GET(id)  \
        (((id) >> _TB_ARL_INDEX_TABLE_ID_SHIFT) & _TB_ARL_INDEX_TABLE_ID_MASK)

/* -- definition for ARL -- */
#define _TB_ARL_AGE_HIT_VAL       0x4     /* b100 for new learned entry */
#define _TB_ARL_AGE_DEF_VAL       0x3     /* b011 for new insert entry */
/* status valud in ARL entry */
#define _TB_ARL_STATUS_VALID        0x03
#define _TB_ARL_STATUS_PENDING      0x01
#define _TB_ARL_STATUS_INVALID      0x00

/* -- definition for Multicast table index -- */
/* dummy id is used to indicate that no id is assigned */
#define _TB_MCAST_PBMP_DUMMY_ID     0

/* special case for TB to indicat user's reuqest on auto mcast id select.
 *  - This value will be placed in MARL entry on USER field.
 *      >> Such value assignment must be clear before real mem data write.
 */
#define _TB_MCAST_USER_FLD_AUTOID    0x1

/* -- definition for PortMask table -- */
/* port mask table config type definition :
 * 
 *  Note : 
 *  1. This type definition is zero based, can't be used directly for bit 
 *      shift.
 */
typedef enum _drv_tb_pmask_config_type_e {
    _TB_PMASK_CONFIG_TYPE_UNTAG_DROP = 0,
    _TB_PMASK_CONFIG_TYPE_PRITAG_DROP,
    _TB_PMASK_CONFIG_TYPE_1QTAG_DROP,
    _TB_PMASK_CONFIG_TYPE_MIS_TAG_DROP,
    _TB_PMASK_CONFIG_TYPE_NON_MEMBER_DROP,
    _TB_PMASK_CONFIG_TYPE_DUMMY,            /* dummy definition */
    _TB_PMASK_CONFIG_TYPE_SA_LRN_DISABLE,
    _TB_PMASK_CONFIG_TYPE_SA_LRN_MODE,      /* SW/HW learn */
    _TB_PMASK_CONFIG_TYPE_SA_MOVE_DROP,
    _TB_PMASK_CONFIG_TYPE_SA_MOVE_CPU,
    _TB_PMASK_CONFIG_TYPE_SA_MOVE_NO_LEARN,
    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_DROP,
    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_CPU,
    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_DROP,
    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_CPU,
    _TB_PMASK_CONFIG_TYPE_RAW_VALUE, /* special type for raw_data set/get */ 
    _TB_PMASK_CONFIG_TYPE_COUNT
} _drv_tb_pmask_config_type_t;

#define _TB_PMASK_CONFIG_TYPE_SHIFT_BIT_GET(_type)  \
        (((_type) < _TB_PMASK_CONFIG_TYPE_RAW_VALUE) ? (_type) : 0)

typedef enum _drv_tb_pmask_mask_type_e {
    _TB_PMASK_MASK_TYPE_ANY = 0,
    _TB_PMASK_MASK_TYPE_DLF_UCAST,
    _TB_PMASK_MASK_TYPE_DLF_L2MCAST,
    _TB_PMASK_MASK_TYPE_DLF_L3MCAST,
    _TB_PMASK_MASK_TYPE_BCAST,
    _TB_PMASK_MASK_TYPE_COUNT
} _drv_tb_pmask_mask_type_t;

/* Portmask MASK field's bit mask definition */
#define _TB_PMASK_MASK_ANY_SHIFT            0
#define _TB_PMASK_MASK_DLF_UCAST_SHIFT      29
#define _TB_PMASK_MASK_DLF_L2MCAST_SHIFT    58
#define _TB_PMASK_MASK_DLF_L3MCAST_SHIFT    87
#define _TB_PMASK_MASK_BCAST_SHIFT          116
#define _TB_PMASK_MASK_BITMASK              0x1FFFFFFF

/* Portmask Config field's bit mask definition */
#define _TB_PMASK_PCONFIG_SHIFT             145
#define _TB_PMASK_PCONFIG_MASK              0x7FFF
#define _TB_PMASK_PCONFIG_EN_UNTAG_DROP         0x0001
#define _TB_PMASK_PCONFIG_EN_PRITAG_DROP        0x0002
#define _TB_PMASK_PCONFIG_EN_1QTAG_DROP         0x0004
#define _TB_PMASK_PCONFIG_EN_MIS_TAG_DROP       0x0008
#define _TB_PMASK_PCONFIG_EN_NON_MEM_DROP       0x0010
/* 0x0020 no defined */
#define _TB_PMASK_PCONFIG_SA_LRN_DISABLE        0x0040
#define _TB_PMASK_PCONFIG_SA_LRN_CTRL           0x0080  /* HW/SW learn mode */
#define _TB_PMASK_PCONFIG_EN_SA_MOVE_DROP       0x0100
#define _TB_PMASK_PCONFIG_EN_SA_MOVE_CPU        0x0200
#define _TB_PMASK_PCONFIG_DIS_LRN_SA_MOVE       0x0400
#define _TB_PMASK_PCONFIG_EN_SA_UNKNOWN_DROP    0x0800
#define _TB_PMASK_PCONFIG_EN_SA_UNKNOWN_CPU     0x1000
#define _TB_PMASK_PCONFIG_EN_SA_OVERLIMIT_DROP  0x2000
#define _TB_PMASK_PCONFIG_EN_SA_OVERLIMIT_CPU   0x4000

/* -- definition for SA_LRN_CNT table -- */
#define _TB_SA_LRNCNT_MAX_TABLE_ID      0x1C

/* internal memory routine for SA_LRN_CNT table process :
 *  - this routine is not a driver serivce interface for the operation process
 *      is just bcm53280 specific only and the earily ROBO device has simular
 *      function designed but used different configuration flow.
 */
extern int _drv_bcm53280_mem_sa_lrncnt_control_set(
        int uint, uint32 entry_id, uint32 control_type, int  value);

/* ARL scan valid only avilable for B0 */
extern int _tb_arl_scan_valid(int unit, int index, SHR_BITDCL *valid_list);

/* Report the ARL hash index through the register interface on bcm53280.
 *  this routine reports the requested dual hash index (CRC16 or CRC32)
 */
extern int _drv_tb_arl_hash_index_get(int unit, 
        int is_crc16, uint32 *sw_arl, int *index);

#endif  /* _ROBO_53280_H */

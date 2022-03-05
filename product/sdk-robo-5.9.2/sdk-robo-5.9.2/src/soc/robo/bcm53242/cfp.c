/*
 * $Id: cfp.c,v 1.48.6.1 Broadcom SDK $
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

#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>
#include <soc/cfp.h>


/* Ram select code for bcm53242 */
#define CFP_53242_MEM_SEL_OB_STAT 0x10  
#define CFP_53242_MEM_SEL_IB_STAT 0x8
#define CFP_53242_MEM_SEL_METER   0x4
#define CFP_53242_MEM_SEL_ACT     0x2
#define CFP_53242_MEM_SEL_TCAM     0x1

/* OP code */
#define CFP_53242_OP_DONE   0x0
#define CFP_53242_OP_READ   0x1
#define CFP_53242_OP_WRITE  0x2
#define CFP_53242_OP_FILL   0x3
#define CFP_53242_OP_SEARCH 0x4

/* Action OP codes */
#define CFP_53242_ACT_FWD_OP_NONE     0x0
#define CFP_53242_ACT_FWD_OP_REDIRECT 0x1
#define CFP_53242_ACT_FWD_OP_APPEND   0x2
#define CFP_53242_ACT_FWD_OP_DROP_COPY   0x3

#define CFP_53242_ACT_FWD_VAL_FLOOD 0x3f

#define CFP_53242_ACT_FWD_VAL_AS_ARL                     0x38 /* 1'b111000 */
#define CFP_53242_ACT_FWD_VAL_COPY_MIRROR_TO             0x39 /* 1'b111001 */
#define CFP_53242_ACT_FWD_VAL_COPY_CPU                   0x3a /* 1'b111010 */
#define CFP_53242_ACT_FWD_VAL_COPY_CPU_AND_MIRROR_TO     0x3b /* 1'b111011 */
#define CFP_53242_ACT_FWD_VAL_DROP                       0x3c /* 1'b111100 */
#define CFP_53242_ACT_FWD_VAL_REDIRECT_MIRROR_TO         0x3d /* 1'b111101 */
#define CFP_53242_ACT_FWD_VAL_REDIRECT_CPU               0x3e /* 1'b111110 */
#define CFP_53242_ACT_FWD_VAL_REDIRECT_CPU_AND_MIRROR_TO 0x3f /* 1'b111111 */

/* User defined fields for BCM53242 */ 
#define CFP_53242_UDF_NUM_MAX 25
#define CFP_53242_UDF_OFFSET_MAX 80 

#define CFP_53242_METER_RATE_MAX 16383000 /* (0x3fff * 1000) */
#define CFP_53242_METER_RATE_MIN 0
#define CFP_53242_METER_BURST_MAX 0x1fc0 /* (1016 * 8) */
#define CFP_53242_METER_BURST_MIN 0
#define CFP_53242_METER_REF_UNIT_64K 0
#define CFP_53242_METER_REF_UNIT_1M  1

/* offset of UDFs to calculate register address */
#define UDF_A_BASE_INDEX 0
#define UDF_B_BASE_INDEX 3
#define UDF_C_BASE_INDEX 14
#define UDF_D_BASE_INDEX 17


/* Slice 0~2 qualify set */
static int s0_qset[] = { DRV_CFP_QUAL_SRC_PBMP,/* ingress port map */
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_L4_FRM_FORMAT,
                                  DRV_CFP_QUAL_VLAN_RANGE,
                                  DRV_CFP_QUAL_L4_PORT_RANGE,
                                  DRV_CFP_QUAL_MAC_DA,
                                  DRV_CFP_QUAL_MAC_SA,
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
                                  DRV_CFP_QUAL_ETYPE,
                                  DRV_CFP_QUAL_IP_DA, /* combined field */
                                  DRV_CFP_QUAL_IP_SA, /* combined field */
                                  DRV_CFP_QUAL_IP_TOS, /* combined field */
                                  DRV_CFP_QUAL_IP_PROTO, /* combined field */
                                  DRV_CFP_QUAL_IP_TTL, /* combined field */
                                  DRV_CFP_QUAL_IP6_FLOW_ID, 
                                  DRV_CFP_QUAL_IP6_SA, 
                                  DRV_CFP_QUAL_IP6_TRAFFIC_CLASS,
                                  DRV_CFP_QUAL_IP6_NEXT_HEADER,
                                  DRV_CFP_QUAL_IP6_HOP_LIMIT,
                                  DRV_CFP_QUAL_IP_SAME,
                                  DRV_CFP_QUAL_L4_DST,
                                  DRV_CFP_QUAL_L4_SRC, /* combined field */
                                  DRV_CFP_QUAL_ICMPIGMP_TYPECODE,
                                  DRV_CFP_QUAL_TCP_FLAG,
                                  DRV_CFP_QUAL_L4_SAME,
                                  DRV_CFP_QUAL_L4SRC_LESS1024,
                                  DRV_CFP_QUAL_TCP_SEQ_ZERO,
                                  DRV_CFP_QUAL_TCP_HDR_LEN, /* combined field */
                                  DRV_CFP_QUAL_BIG_ICMP_CHECK,
                                  DRV_CFP_QUAL_UDFA0,
                                  DRV_CFP_QUAL_UDFA1,
                                  DRV_CFP_QUAL_UDFA2,
                                  DRV_CFP_QUAL_INVALID};

static int s1_qset[] = { DRV_CFP_QUAL_SRC_PBMP,/* ingress port map */
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_L4_FRM_FORMAT,
                                  DRV_CFP_QUAL_VLAN_RANGE,
                                  DRV_CFP_QUAL_L4_PORT_RANGE,
                                  DRV_CFP_QUAL_MAC_DA,
                                  DRV_CFP_QUAL_MAC_SA,
                                  DRV_CFP_QUAL_SP_VID,
                                  DRV_CFP_QUAL_SP_CFI,
                                  DRV_CFP_QUAL_SP_PRI,
                                  DRV_CFP_QUAL_USR_VID,
                                  DRV_CFP_QUAL_USR_CFI,
                                  DRV_CFP_QUAL_USR_PRI,
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
                                  DRV_CFP_QUAL_INVALID};

static int s2_qset[] = { DRV_CFP_QUAL_SRC_PBMP,/* ingress port map */
                                  DRV_CFP_QUAL_SPTAG,
                                  DRV_CFP_QUAL_1QTAG,
                                  DRV_CFP_QUAL_L2_FRM_FORMAT,
                                  DRV_CFP_QUAL_L3_FRM_FORMAT,
                                  DRV_CFP_QUAL_L4_FRM_FORMAT,
                                  DRV_CFP_QUAL_VLAN_RANGE,
                                  DRV_CFP_QUAL_L4_PORT_RANGE,
                                  DRV_CFP_QUAL_UDFC0,
                                  DRV_CFP_QUAL_UDFC1,
                                  DRV_CFP_QUAL_UDFC2,
                                  DRV_CFP_QUAL_UDFD0,
                                  DRV_CFP_QUAL_UDFD1,
                                  DRV_CFP_QUAL_UDFD2,
                                  DRV_CFP_QUAL_UDFD3,
                                  DRV_CFP_QUAL_UDFD4,
                                  DRV_CFP_QUAL_UDFD5,
                                  DRV_CFP_QUAL_UDFD6,
                                  DRV_CFP_QUAL_UDFD7,
                                  DRV_CFP_QUAL_INVALID};

static uint16 drv_53242_tcam_search_ptr = 0; /* should be 0~1023. */

static uint32 drv_53242_tcam_valid[32];

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? BYTES2WORDS((m)->bytes)-1-(v) : (v))
                                         
/*
 * Declare a software table(16 elements) to map the FLOW2VLAN table of each port(also 16 entries of each port).
 */
#define FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM 16

typedef struct flow_based_vlan_entry_s {
    int16 vid; /* New vid */
} flow_based_vlan_entry_t;

typedef struct flow_based_vlan_tbl_s {
    flow_based_vlan_entry_t flow_entry[FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM];
} flow_based_vlan_tbl_t;

flow_based_vlan_tbl_t flow_table[SOC_MAX_NUM_PORTS]; /* max ports num of 53242/53262 */
uint8 flow_entry_used[FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM];

typedef flow2vlan_entry_t f2v_entry_t;

/*
 * Function: _drv53242_cfp_field_mapping
 *
 * Purpose:
 *     Translate the driver field type to chip field index.
 *
 * Parameters:
 *     unit - BCM device number
 *     field_type - driver field value
 *     field_id (OUT) - chip field index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_BADID - unknown driver field value
 */
int
_drv53242_cfp_field_mapping(int unit, uint32 field_type, uint32 *field_id)
{
    int rv = SOC_E_NONE;
   
    switch (field_type) {
        /* TCAM fields */
        case DRV_CFP_FIELD_VALID:
            *field_id = VALID_Rf;
            break;
        case DRV_CFP_FIELD_SLICE_ID:
            *field_id = SLICEIDf;
            break;
        case DRV_CFP_FIELD_IN_PBMP:
            *field_id = IN_PBMPf;
            break;
        case DRV_CFP_FIELD_SPTAGGED:
            *field_id = SPTAGGEDf;
            break;
        case DRV_CFP_FIELD_1QTAGGED:
            *field_id = VLANTAGGEDf;
            break;
        case DRV_CFP_FIELD_L2_FRM_FORMAT:
            *field_id = L2_FORMATf;
            break;
        case DRV_CFP_FIELD_L3_FRM_FORMAT:
            *field_id = L3_FORMATf;
            break;
        case DRV_CFP_FIELD_L4_FRM_FORMAT:
            *field_id = L4_FORMATf;
            break;
        case DRV_CFP_FIELD_VLAN_RANGE:
            *field_id = VLAN_RANGEf;
            break;
        case DRV_CFP_FIELD_L4_PORT_RANGE:
            *field_id = L4_PORT_RANGEf;
            break;
        case DRV_CFP_FIELD_MAC_DA:
            *field_id = MAC_DAf;
            break;
        case DRV_CFP_FIELD_MAC_SA:
            *field_id = MAC_SAf;
            break;
        case DRV_CFP_FIELD_SP_PRI:
            *field_id = SP_PRIf;
            break;
        case DRV_CFP_FIELD_SP_CFI:
            *field_id = SP_CFIf;
            break;
        case DRV_CFP_FIELD_SP_VID:
            *field_id = SP_VIDf;
            break;
        case DRV_CFP_FIELD_USR_PRI:
            *field_id = USR_PRIf;
            break;
        case DRV_CFP_FIELD_USR_CFI:
            *field_id = USR_CFIf;
            break;
        case DRV_CFP_FIELD_USR_VID:
            *field_id = USR_VIDf;
            break;
        case DRV_CFP_FIELD_ETYPE:
            *field_id = ETYPEf; 
            break;
        /*
         * CHECKME: Combination field.
         */
        case DRV_CFP_FIELD_IP_DA:
        case DRV_CFP_FIELD_IP_SA:
            /* 
             * These two fields should already be processed at
             * drv53242_cfp_field_set/get().
             */
            soc_cm_debug(DK_ERR, 
                "DRV_CFP_FIELD_IP4_DA/SA should not be processed at _drv53242_cfp_field_mapping\n");
            return SOC_E_BADID;
            break;
        case DRV_CFP_FIELD_IP6_FLOW_ID:
            *field_id = IP6_FLOW_IDf;
            break;
        case DRV_CFP_FIELD_IP6_SA:
            *field_id = IP6_SAf;
            break;
        case DRV_CFP_FIELD_IP_TOS:
        case DRV_CFP_FIELD_IP6_TRAFFIC_CLASS:
            *field_id = IP4TOS_IP6TRAFFICCLASSf;
            break;
        case DRV_CFP_FIELD_IP_PROTO:
        case DRV_CFP_FIELD_IP6_NEXT_HEADER:
            *field_id = IP4PROTOCOL_IP6NEXTHEADERf;
            break;
        case DRV_CFP_FIELD_IP_TTL:
        case DRV_CFP_FIELD_IP6_HOP_LIMIT:
            *field_id = IP4TTL_IP6HOPLIMITf;
            break;
        case DRV_CFP_FIELD_SAME_IP:
            *field_id = SAMEIPADDRf;
            break;
        case DRV_CFP_FIELD_L4DST:
            *field_id = L4DSTf;
            break;
        /*
         * CHECKME: Combination field.
         */
        case DRV_CFP_FIELD_L4SRC:
            /* 
             * This field should already be processed at
             * drv53242_cfp_field_set/get().
             */
            soc_cm_debug(DK_ERR, 
                "DRV_CFP_FIELD_L4SRC should not be processed at _drv53242_cfp_field_mapping\n");
            return SOC_E_BADID;
            break;
        case DRV_CFP_FIELD_ICMPIGMP_TYPECODE:
            *field_id = ICMPIGMP_TYPECODEf;
            break;
        case DRV_CFP_FIELD_TCP_FLAG:
            *field_id = TCP_FLAGf;
            break;
        case DRV_CFP_FIELD_SAME_L4PORT:
            *field_id = SAMEL4PORTf;
            break;
        case DRV_CFP_FIELD_L4SRC_LESS1024:
            *field_id = L4SRC_LESS_1024f;
            break;
        case DRV_CFP_FIELD_TCP_SEQ_ZERO:
            *field_id = TCP_SEQUENCE_ZEROf;
            break;
        case DRV_CFP_FIELD_TCP_HDR_LEN:
        case DRV_CFP_FIELD_BIG_ICMP_CHECK:
            *field_id = TCPHEADER_BIGICMP_CHKf;
            break;
        case DRV_CFP_FIELD_UDFA0_VALID:
            *field_id = UDFA0_VLDf;
            break;
        case DRV_CFP_FIELD_UDFA0:
            *field_id = UDFA0f;
            break;
        case DRV_CFP_FIELD_UDFA1_VALID:
            *field_id = UDFA1_VLDf;
            break;
        case DRV_CFP_FIELD_UDFA1:
            *field_id = UDFA1f;
            break;
        case DRV_CFP_FIELD_UDFA2_VALID:
            *field_id = UDFA2_VLDf;
            break;
        case DRV_CFP_FIELD_UDFA2:
            *field_id = UDFA2f;
            break;
        case DRV_CFP_FIELD_UDFB0_VALID:
            *field_id = UDFB0_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB0:
            *field_id = UDFB0f;
            break;
        case DRV_CFP_FIELD_UDFB1_VALID:
            *field_id = UDFB1_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB1:
            *field_id = UDFB1f;
            break;
        case DRV_CFP_FIELD_UDFB2_VALID:
            *field_id = UDFB2_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB2:
            *field_id = UDFB2f;
            break;
        case DRV_CFP_FIELD_UDFB3_VALID:
            *field_id = UDFB3_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB3:
            *field_id = UDFB3f;
            break;
        case DRV_CFP_FIELD_UDFB4_VALID:
            *field_id = UDFB4_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB4:
            *field_id = UDFB4f;
            break;
        case DRV_CFP_FIELD_UDFB5_VALID:
            *field_id = UDFB5_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB5:
            *field_id = UDFB5f;
            break;
        case DRV_CFP_FIELD_UDFB6_VALID:
            *field_id = UDFB6_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB6:
            *field_id = UDFB6f;
            break;
        case DRV_CFP_FIELD_UDFB7_VALID:
            *field_id = UDFB7_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB7:
            *field_id = UDFB7f;
            break;
        case DRV_CFP_FIELD_UDFB8_VALID:
            *field_id = UDFB8_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB8:
            *field_id = UDFB8f;
            break;
        case DRV_CFP_FIELD_UDFB9_VALID:
            *field_id = UDFB9_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB9:
            *field_id = UDFB9f;
            break;
        case DRV_CFP_FIELD_UDFB10_VALID:
            *field_id = UDFB10_VLDf;
            break;
        case DRV_CFP_FIELD_UDFB10:
            *field_id = UDFB10f;
            break;
        case DRV_CFP_FIELD_UDFC0_VALID:
            *field_id = UDFC0_VLDf;
            break;
        case DRV_CFP_FIELD_UDFC0:
            *field_id = UDFC0f;
            break;
        case DRV_CFP_FIELD_UDFC1_VALID:
            *field_id = UDFC1_VLDf;
            break;
        case DRV_CFP_FIELD_UDFC1:
            *field_id = UDFC1f;
            break;
        case DRV_CFP_FIELD_UDFC2_VALID:
            *field_id = UDFC2_VLDf;
            break;
        case DRV_CFP_FIELD_UDFC2:
            *field_id = UDFC2f;
            break;
        case DRV_CFP_FIELD_UDFD0_VALID:
            *field_id = UDFD0_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD0:
            *field_id = UDFD0f;
            break;
        case DRV_CFP_FIELD_UDFD1_VALID:
            *field_id = UDFD1_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD1:
            *field_id = UDFD1f;
            break;
        case DRV_CFP_FIELD_UDFD2_VALID:
            *field_id = UDFD2_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD2:
            *field_id = UDFD2f;
            break;
        case DRV_CFP_FIELD_UDFD3_VALID:
            *field_id = UDFD3_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD3:
            *field_id = UDFD3f;
            break;
        case DRV_CFP_FIELD_UDFD4_VALID:
            *field_id = UDFD4_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD4:
            *field_id = UDFD4f;
            break;
        case DRV_CFP_FIELD_UDFD5_VALID:
            *field_id = UDFD5_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD5:
            *field_id = UDFD5f;
            break;
        case DRV_CFP_FIELD_UDFD6_VALID:
            *field_id = UDFD6_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD6:
            *field_id = UDFD6f;
            break;
        case DRV_CFP_FIELD_UDFD7_VALID:
            *field_id = UDFD7_VLDf;
            break;
        case DRV_CFP_FIELD_UDFD7:
            *field_id = UDFD7f;
            break;

        /* Action fields */
        case DRV_CFP_FIELD_CHANGE_DSCP_OB_EN:
            *field_id = CHANGE_DSCP_OBf;
            break;
        case DRV_CFP_FIELD_NEW_DSCP_OB:
            *field_id = NEW_DSCP_OBf;
            break;
        case DRV_CFP_FIELD_CHANGE_FWD_OB_EN:
            *field_id = CHANGE_FWD_OBf;
            break;
        case DRV_CFP_FIELD_NEW_FWD_OB:
            *field_id = NEW_FWD_OBf;
            break;
        case DRV_CFP_FIELD_CHANGE_FLOW_EN:
            *field_id = CHANGE_FLOWf;
            break;
        case DRV_CFP_FIELD_NEW_FLOW_INDEX:
            *field_id = NEW_FLOW_INDEXf;
            break;
        case DRV_CFP_FIELD_CHANGE_DSCP_IB_EN:
            *field_id = CHANGE_DSCP_IBf;
            break;
        case DRV_CFP_FIELD_NEW_DSCP_IB:
            *field_id = NEW_DSCP_IBf;
            break;
        case DRV_CFP_FIELD_CHANGE_PCP_EN:
            *field_id = CHANGE_PCPf;
            break;
        case DRV_CFP_FIELD_NEW_PCP:
            *field_id = NEW_PCPf;
            break;
        case DRV_CFP_FIELD_CHANGE_COS_EN:
            *field_id = CHANGE_COSf;
            break;
        case DRV_CFP_FIELD_NEW_COS:
            *field_id = NEW_COSf_ROBO;
            break;
        case DRV_CFP_FIELD_CHANGE_COS_IMP_EN:
            *field_id = CHANGE_COS_IMPf;
            break;
        case DRV_CFP_FIELD_NEW_COS_IMP:
            *field_id = NEW_COS_IMPf;
            break;
        case DRV_CFP_FIELD_CHANGE_FWD_IB_EN:
            *field_id = CHANGE_FWD_IBf;
            break;
        case DRV_CFP_FIELD_NEW_FWD_IB:
            *field_id = NEW_FWD_IBf;
            break;

        /* Rate meter */
        case DRV_CFP_FIELD_BUCKET_SIZE:
            *field_id = BKTSIZEf;
            break;
        case DRV_CFP_FIELD_REF_UNIT:
            *field_id = RFSHUNITf;
            break;
        case DRV_CFP_FIELD_REF_CNT:
            *field_id = RFSHCNTf;
            break;

        /* IB/OB counter*/
        case DRV_CFP_FIELD_IB_CNT:
            *field_id = IN_BAND_CNTf;
            break;
        case DRV_CFP_FIELD_OB_CNT:
            *field_id = OUT_BAND_CNTf;
            break;
        default:
            rv = SOC_E_BADID;
    }

    return rv;
}

/*
 * Function: _drv53242_cfp_tcam_fill
 *
 * Purpose:
 *     Fill TCAM with data.
 *
 * Parameters:
 *     unit - BCM device number
 *     data - data to fill TCAM entries
 *
 * Returns:
 *     SOC_E_NONE
 */
int
_drv53242_cfp_tcam_fill(int unit, uint32 data)
{
    int rv = SOC_E_NONE;
    int mem_len, retry;
    uint32  reg_addr, reg_len, fld_32, reg_val32;
    uint64  reg_val64;
    int i;

    MEM_LOCK(unit, CFP_TCAM_S0m);

    /* mem_len is 12 words */
    mem_len = soc_mem_entry_words(unit, CFP_TCAM_S0m);
    /* Fill TCAM DATAs with desired value */
    COMPILER_64_SET(reg_val64, data, data);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_DATAr);
    for (i = 0; i < mem_len; ) {
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (reg_addr + i * 4), (uint32 *)&reg_val64, reg_len)) < 0) {
            goto cfp_tcam_fill_exit;
        }
        i += 2;
    }

    /* Process fill operation */
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto cfp_tcam_fill_exit;
    }

    fld_32 = CFP_53242_MEM_SEL_TCAM;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val32, MEMSELf, &fld_32);

    fld_32 = CFP_53242_OP_FILL;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val32, OPCODE_Rf, &fld_32);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_val32, reg_len)) < 0) {
        goto cfp_tcam_fill_exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val32, reg_len)) < 0) {
            goto cfp_tcam_fill_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val32, OPCODE_Rf, &fld_32);
        if (!fld_32) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_tcam_fill_exit;
    }

cfp_tcam_fill_exit:    
    MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_meter_rate2chip
 *
 * Purpose:
 *     Translate the driver rate value to register value.
 *
 * Parameters:
 *     unit - BCM device number
 *     kbits_sec - driver rate value
 *     chip_val(OUT) - register value
 *
 * Returns:
 *     Nothing
 */
void
_drv53242_cfp_meter_rate2chip(int unit, uint32 kbits_sec, uint32 *ref_u, uint32 *ref_cnt)
{
    uint32 quotient_64k, quotient_1m, remainder;
    /* Can be divided by 1M(1000*1000) with no remainder? */
    quotient_1m = kbits_sec / 1000;
    remainder = kbits_sec - (quotient_1m * 1000);
    if (remainder == 0) {
        *ref_u = CFP_53242_METER_REF_UNIT_1M;
        *ref_cnt = quotient_1m;
        return;
    }

    /* Can be divided by 62.5K with no remainder? */
    quotient_64k = (kbits_sec * 10) / 625;
    remainder = (kbits_sec * 10) - (quotient_64k * 625);
    if (remainder == 0) {
        *ref_u = CFP_53242_METER_REF_UNIT_64K;
        *ref_cnt = quotient_64k;
        return;
    }

    /* Others */    
    if (kbits_sec <= (1024000)) { /* (2^14 * 62.5) */
        /* Use 62.5K as unit */
        *ref_u = CFP_53242_METER_REF_UNIT_64K;
        *ref_cnt = quotient_64k;
    } else {
        /* 62.5K unit can't represent, so use 1M as unit */
        *ref_u = CFP_53242_METER_REF_UNIT_1M;
        *ref_cnt = quotient_1m;
    }
    return;
}

/*
 * Function: _drv53242_cfp_read
 *
 * Purpose:
 *     Read the CFP raw data by ram type from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_read(int unit, uint32 ram_type, uint32 index, drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr, mask_reg_addr;
    uint64 reg_val64;

    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            mem_id = CFP_ACT_POLm;
            ram_val = CFP_53242_MEM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = CFP_METERm;
            ram_val = CFP_53242_MEM_SEL_METER;
            break;
        case DRV_CFP_RAM_TCAM:
            assert(entry->tcam_mask);
            mem_id = CFP_TCAM_S0m;
            ram_val = CFP_53242_MEM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    soc_cm_debug(DK_VERBOSE, 
        "_drv53242_cfp_read : mem_id = %d, ram_val = %d, index_max = %d\n",
        mem_id, ram_val, index_max);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform TCAM read operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
    }

    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, MEMSELf, &fld_val);

    fld_val = index;    
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

    fld_val = CFP_53242_OP_READ;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_read_exit;
    }

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
        mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_MASKr, 0, 0);
        for (i = 0; i < mem_len; ) {
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (data_reg_addr + i * 4), 
                (uint32 *)&reg_val64, 8)) < 0) {
                goto cfp_read_exit;
            }
            *(data_p + i) = COMPILER_64_LO(reg_val64);
            *(data_p + i + 1) = COMPILER_64_HI(reg_val64);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, (mask_reg_addr + i * 4), 
                (uint32 *)&reg_val64, 8)) < 0) {
                goto cfp_read_exit;
            }
            *(mask_p + i) = COMPILER_64_LO(reg_val64);
            *(mask_p + i + 1) = COMPILER_64_HI(reg_val64);

            i += 2;
        }
        break;
    case DRV_CFP_RAM_ACT:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ACT_POL_DATAr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ACT_POL_DATAr);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, 
            (uint32 *)&reg_val64, reg_len)) < 0) {
            goto cfp_read_exit;
        }
        entry->act_data[0] = COMPILER_64_LO(reg_val64);
        entry->act_data[1] = COMPILER_64_HI(reg_val64);
        break;
    case DRV_CFP_RAM_METER:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, CFP_RCr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_read_exit;
        }
        entry->meter_data[0] = reg_val;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        goto cfp_read_exit;
    }

    cfp_read_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_tcam_search
 *
 * Purpose:
 *     Search the Valid TCAM raw data
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_tcam_search(int unit, uint32 flags, uint32* index, 
            drv_cfp_entry_t *entry)
{
    int i, j, rv = SOC_E_NONE;
    int srch_array_idx, srch_bit_offset;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int retry;
    uint64 reg_val64;
    
    assert(entry);
    assert(index);

    MEM_LOCK(unit, CFP_TCAM_S0m);

    /* Get SEARCH DONE bit */
    if (flags & DRV_CFP_SEARCH_DONE) {
        for (i = 0; i < 32; i++) {
            if (drv_53242_tcam_valid[i]) {
                *index = 0; /* search not finished. */
                goto cfp_search_exit;
            }
        }
        *index = 1; /* search finished. */
        drv_53242_tcam_search_ptr = 0;
        goto cfp_search_exit;
    }

    /* 
     * Get valid TCAM entry
     * Return TCAM entry index only. The main search driver service
     * will get the entry content by the index.
     */
    if(flags & DRV_CFP_SEARCH_GET) {
        srch_array_idx = drv_53242_tcam_search_ptr / 32;
        srch_bit_offset = drv_53242_tcam_search_ptr - 
                          (srch_array_idx*32);

        for (i = srch_array_idx; i < 32; i++) {
            for (j = srch_bit_offset; j < 32; j++) {
                if ((drv_53242_tcam_valid[i] >> j) 
                    & 0x1) {
                    drv_53242_tcam_valid[i] &= ~(0x1 << j);
                    drv_53242_tcam_search_ptr = i * 32 + j;
                    *index = drv_53242_tcam_search_ptr;
                    goto cfp_search_exit;
                }
            }
        }
    }

    /* Get valid array */
    if (flags & DRV_CFP_SEARCH_START) {
        for (j = 0; j < 2; j++) {
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_val, reg_len)) < 0) {
                    return rv;
            }
            /* Select TCAM */
            fld_val = CFP_TCAM_S0m;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, MEMSELf, &fld_val);
    
            /* Set initial search address, every 512 entries.
             * first loop: start from entry 0. 
             * second loop: start from entry 512.
             */
            fld_val = j * 512;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);
    
            /* Set search op code and start search. */
            fld_val = CFP_53242_OP_SEARCH;
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);
    
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_val, reg_len)) < 0) {
                goto cfp_search_exit;
            }
    
            /* wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                    (unit, reg_addr, &reg_val, reg_len)) < 0) {
                    goto cfp_search_exit;
                }
                (DRV_SERVICES(unit)->reg_field_get)
                    (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);
                if (!fld_val) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto cfp_search_exit;
            }
    
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_DATAr);
            for (i = 0; i < 8; i++) {
                if ((rv = (DRV_SERVICES(unit)->reg_read)
                        (unit, (reg_addr + i*4), 
                        (uint32 *)&reg_val64, reg_len)) < 0) {
                        return rv;
                }
                drv_53242_tcam_valid[j*16 + i*2] = 
                    COMPILER_64_LO(reg_val64);
                drv_53242_tcam_valid[j*16 + i*2 + 1] = 
                    COMPILER_64_HI(reg_val64);
            }
        }
        drv_53242_tcam_search_ptr = 0;
    }
    
    cfp_search_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_stat_read
 *
 * Purpose:
 *     Read the counter raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry(OUT) -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_stat_read(int unit, uint32 counter_type, uint32 index, uint32 *counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max;
    uint32  data_reg_addr;

    assert(counter);
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_53242_MEM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_53242_MEM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform read operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
    }
 
    fld_val = ram_val;     
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, MEMSELf, &fld_val);

    fld_val = index;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);

    /* start read operation */
    fld_val = CFP_53242_OP_READ;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_read_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_read_exit;
    }
    
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
           (unit, RATE_INBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    case DRV_CFP_RAM_STAT_OB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_OUTBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_stat_read_exit;
        }
        *counter = reg_val;
        break;
    default:
        rv = SOC_E_UNAVAIL;
        goto cfp_stat_read_exit;
    }

    cfp_stat_read_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_write
 *
 * Purpose:
 *     Write the CFP raw data by ram type to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_write(int unit, uint32 ram_type, uint32 index, drv_cfp_entry_t *entry)
{
    int i, rv = SOC_E_NONE;
    uint32  mem_id = 0;
    int ram_val, retry;
    uint32 reg_acc, reg_val, reg_addr, reg_len, fld_val;
    int index_max, mem_len;
    uint32  *data_p, *mask_p, data_reg_addr, mask_reg_addr;
    uint32 reg_val32_lo, reg_val32_hi;
    uint64 reg_val64;

    
    assert(entry);
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            mem_id = CFP_ACT_POLm;
            ram_val = CFP_53242_MEM_SEL_ACT;
            break;
        case DRV_CFP_RAM_METER:
            mem_id = CFP_METERm;
            ram_val = CFP_53242_MEM_SEL_METER;
            break;
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_53242_MEM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_53242_MEM_SEL_OB_STAT;
            break;
        case DRV_CFP_RAM_TCAM:
        case DRV_CFP_RAM_TCAM_INVALID:
            mem_id = CFP_TCAM_S0m;
            ram_val = CFP_53242_MEM_SEL_TCAM;
            break;
        default:
            rv = SOC_E_UNAVAIL;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform TCAM write operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);

    switch (ram_type) {
    case DRV_CFP_RAM_TCAM:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
        mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_MASKr, 0, 0);
        for (i = 0; i < mem_len; ) {
            reg_val32_lo = *(data_p + i);
            reg_val32_hi = *(data_p + i + 1);
            COMPILER_64_SET(reg_val64, reg_val32_hi, reg_val32_lo);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (data_reg_addr + i * 4), 
                (uint32 *)&reg_val64, 8)) < 0) {
                goto cfp_write_exit;
            }
            reg_val32_lo = *(mask_p + i);
            reg_val32_hi = *(mask_p + i + 1);
            COMPILER_64_SET(reg_val64, reg_val32_hi, reg_val32_lo);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, (mask_reg_addr + i * 4), 
                (uint32 *)&reg_val64, 8)) < 0) {
                goto cfp_write_exit;
            }

            i += 2;
        }
        break;
    case DRV_CFP_RAM_TCAM_INVALID:
        mem_len = soc_mem_entry_words(unit, mem_id);
        data_p = entry->tcam_data;
        mask_p = entry->tcam_mask;
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_DATAr, 0, 0);
        mask_reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_MASKr, 0, 0);
        /* write the last word which contain the valid field */
        i = mem_len - 2;
        
        reg_val32_lo = *(data_p + i);
        reg_val32_hi = *(data_p + i + 1);
        COMPILER_64_SET(reg_val64, reg_val32_hi, reg_val32_lo);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (data_reg_addr + i * 4), 
            (uint32 *)&reg_val64, 8)) < 0) {
            goto cfp_write_exit;
        }
        reg_val32_lo = *(mask_p + i);
        reg_val32_hi = *(mask_p + i + 1);
        COMPILER_64_SET(reg_val64, reg_val32_hi, reg_val32_lo);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, (mask_reg_addr + i * 4), 
            (uint32 *)&reg_val64, 8)) < 0) {
            goto cfp_write_exit;
        }
        break;
    case DRV_CFP_RAM_ACT:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, ACT_POL_DATAr, 0, 0);
        reg_len = (DRV_SERVICES(unit)->reg_length_get)
            (unit, ACT_POL_DATAr);
        reg_val32_lo = entry->act_data[0];
        reg_val32_hi = entry->act_data[1];
        COMPILER_64_SET(reg_val64, reg_val32_hi, reg_val32_lo);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, (uint32 *)&reg_val64, reg_len)) < 0) {
            goto cfp_write_exit;
        }
        break;
    case DRV_CFP_RAM_METER:
        reg_val = entry->meter_data[0];
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, CFP_RCr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &reg_val, 4)) < 0) {
            goto cfp_write_exit;
        }
        break;
    default:
        rv = SOC_E_PARAM;
        goto cfp_write_exit;
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
    }

    fld_val = index;       
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, XCESS_ADDRf, &fld_val);


    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, MEMSELf, &fld_val);

    /* Start write */
    fld_val = CFP_53242_OP_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_acc, OPCODE_Rf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_acc, reg_len)) < 0) {
            goto cfp_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_acc, OPCODE_Rf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_write_exit;
    }

    cfp_write_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_stat_write
 *
 * Purpose:
 *     Set the counter raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     counter type - in-band/ out-band
 *     index -entry index
 *     entry -counter raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_stat_write(int unit, uint32 counter_type, uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  mem_id  = 0;
    int ram_val, retry;
    uint32 reg_val, reg_addr, reg_len, fld_val;
    int index_max;
    uint32  data_reg_addr, data_reg_val;
    
    switch (counter_type) {
        case DRV_CFP_RAM_STAT_IB:
            mem_id = CFP_STAT_IBm;
            ram_val = CFP_53242_MEM_SEL_IB_STAT;
            break;
        case DRV_CFP_RAM_STAT_OB:
            mem_id = CFP_STAT_OBm;
            ram_val = CFP_53242_MEM_SEL_OB_STAT;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    index_max = soc_robo_mem_index_max(unit, mem_id);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /*
     * Perform write operation 
     */

    MEM_LOCK(unit, CFP_TCAM_S0m);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, CFP_ACCr, 0, 0);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, CFP_ACCr);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
    }
    
    fld_val = ram_val;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, MEMSELf, &fld_val);

    fld_val = index; 
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, XCESS_ADDRf, &fld_val);


    /* Set counter value */
    data_reg_val = counter;
    switch (counter_type) {
    case DRV_CFP_RAM_STAT_IB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_INBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &data_reg_val, 4)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    case DRV_CFP_RAM_STAT_OB:
        data_reg_addr = (DRV_SERVICES(unit)->reg_addr)
            (unit, RATE_OUTBANDr, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, data_reg_addr, &data_reg_val, 4)) < 0) {
            goto cfp_stat_write_exit;
        }
        break;
    }

    /* Start write */
    fld_val = CFP_53242_OP_WRITE;
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_val, reg_len)) < 0) {
            goto cfp_stat_write_exit;
        }
        (DRV_SERVICES(unit)->reg_field_get)
            (unit, CFP_ACCr, &reg_val, OPCODE_Rf, &fld_val);
        if (!fld_val) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto cfp_stat_write_exit;
    }

    cfp_stat_write_exit:
        MEM_UNLOCK(unit, CFP_TCAM_S0m);
    return rv;
}

/*
 * Function: _drv53242_cfp_flow_based_vlan_idx_get
 *
 * Purpose:
 *     Get/create the FLOW2VLAN index by new vid.
 *
 * Parameters:
 *     unit - BCM device number
 *     vid - new vid
 *     *idx - returned FLOW2VLAN index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_flow_based_vlan_idx_get(int unit, int vid, soc_pbmp_t bmp, int *idx)
{
    int i;
    int rv = SOC_E_NONE;
    int result_idx = -1, found = 0;
    f2v_entry_t f2v_t;
    int port;

    /* Find an available entry id. */
    for (i = 0; i < FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM; i++) {
        if (!flow_entry_used[i]) {
            found = 1;
            result_idx = i;

            flow_entry_used[i] = TRUE;
            break;
        }
    }

    if (!found) {
        /* can't find available or proper entry */
        return SOC_E_RESOURCE;
    } else {
        /* save new vid of desired ports to software copy. */
        PBMP_ITER(bmp, port) {
            flow_table[port].flow_entry[result_idx].vid = vid;
        }
    }

    /* write FLOW2VLAN table */
    sal_memset(&f2v_t, 0, sizeof (f2v_t));
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_FLOWVLAN, DRV_MEM_FIELD_VLANID,
                    (uint32 *)&f2v_t, (uint32 *)&vid));
    PBMP_ITER(bmp, port) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_write)
            (unit, DRV_MEM_FLOWVLAN, result_idx+port*FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM, 1,
            (uint32 *)&f2v_t));
    }

    /* return result */
    *idx = result_idx;

    return rv;
}

/*
 * Function: _drv53242_cfp_flow_based_vlan_idx_remove
 *
 * Purpose:
 *     Remove the SW and HW FLOW2VLAN entry by index.
 *
 * Parameters:
 *     unit - BCM device number
 *     idx - FLOW2VLAN index
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv53242_cfp_flow_based_vlan_idx_remove(int unit, int idx)
{
    uint32 tmp = 0;
    f2v_entry_t f2v_t;
    int port;
    pbmp_t pbm;

    /* clear FLOW2VLAN table */
    sal_memset(&f2v_t, 0, sizeof (f2v_t));
    tmp = 0xfff;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_FLOWVLAN, DRV_MEM_FIELD_VLANID,
                    (uint32 *)&f2v_t, &tmp));

    pbm = PBMP_E_ALL(unit);
    PBMP_ITER(pbm, port) {
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_write)
            (unit, DRV_MEM_FLOWVLAN, idx+port*FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM, 1,
            (uint32 *)&f2v_t));
    }

    /* clear software copy */
    PBMP_ITER(pbm, port) {
        flow_table[port].flow_entry[idx].vid = 0;
    }
    flow_entry_used[idx] = FALSE;

    return SOC_E_NONE;
}

/*
 * Function: drv53242_cfp_init
 *
 * Purpose:
 *     Initialize the CFP module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv53242_cfp_init(int unit)
{
    int port;
    pbmp_t pbm;
    int i;
    uint32  reg_value, temp;
    
    /* Enable CFP */
    pbm = PBMP_E_ALL(unit);

    PBMP_ITER(pbm, port) {
         (DRV_SERVICES(unit)->cfp_control_set)
            (unit, DRV_CFP_ENABLE, port, 1);
    }
    /* Clear HW TABLE */
    (DRV_SERVICES(unit)->cfp_control_set)
        (unit, DRV_CFP_TCAM_RESET, 0, 0);

    /* Initialize sw flow_2_vlan table */
    for (i = 0; i < FLOW_BASED_VLAN_PER_PORT_ENTRY_NUM; i ++) {
        flow_entry_used[i] = FALSE;
        PBMP_ITER(pbm, port) {
            flow_table[port].flow_entry[i].vid = 0;
        }
    }

    /* Initialize rate meter global enable bit */
    SOC_IF_ERROR_RETURN(REG_READ_CFP_ACCr(unit, &reg_value));
    temp = 1;
    soc_CFP_ACCr_field_set(unit, &reg_value, RATE_METER_ENf, &temp);
    temp = 0;
    soc_CFP_ACCr_field_set(unit, &reg_value, MEMSELf, &temp);
    soc_CFP_ACCr_field_set(unit, &reg_value, OPCODE_Rf, &temp);

    SOC_IF_ERROR_RETURN(REG_WRITE_CFP_ACCr(unit, &reg_value));

    return SOC_E_NONE;
}

/*
 * Function: drv53242_cfp_action_get
 *
 * Purpose:
 *     Get the CFP action type and parameters value from 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action(IN/OUT) - driver action type
 *     entry -cfp entry
 *     act_param(OUT) - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 *     The action types were divided into 2 groups.
 *     One is to changed priority and the other is switch behavior 
 *     (drop/flood/append/redirect). So the user configure the input paramter,
 *     'action', to decide which action type you need to get. 
 *     action                              act_param
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv53242_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    switch (*action) {
        case DRV_CFP_ACT_IB_NONE:
        case DRV_CFP_ACT_IB_FLOOD:
        case DRV_CFP_ACT_IB_DROP:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_IB_NONE;
            }
            break;
        case DRV_CFP_ACT_IB_DSCP_NEW:
        case DRV_CFP_ACT_IB_DSCP_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_DSCP_IB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_IB_DSCP_CANCEL;
            }
            break;
        case DRV_CFP_ACT_OB_NONE:
        case DRV_CFP_ACT_OB_FLOOD:
        case DRV_CFP_ACT_OB_DROP:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_OB_NONE;
            }
            break;
        case DRV_CFP_ACT_IB_APPEND:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_IB_NONE;
            } else if (fld_val == CFP_53242_ACT_FWD_OP_DROP_COPY) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_CPU) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_CPU_AND_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU | \
                                                DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else {
                    *act_param = fld_val;
                }
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * register value need to sub 24 for real port id.
                     */            
                    fld_val -= 24;
                }
                *act_param = fld_val;
            }
            break;
        case DRV_CFP_ACT_OB_APPEND:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_OB_NONE;
            } else if (fld_val == CFP_53242_ACT_FWD_OP_DROP_COPY) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_CPU) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_COPY_CPU_AND_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU | \
                                                DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else {
                    *act_param = fld_val;
                }
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * register value need to sub 24 for real port id.
                     */            
                    fld_val -= 24;
                }
                *act_param = fld_val;
            }
            break;
        case DRV_CFP_ACT_IB_REDIRECT:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_IB_NONE;
            } else if (fld_val == CFP_53242_ACT_FWD_OP_DROP_COPY) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_CPU) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_CPU_AND_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU | \
                                                DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
            } else {
                    *act_param = fld_val;
                }
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * register value need to sub 24 for real port id.
                     */            
                    fld_val -= 24;
                }
                *act_param = fld_val;
            }
            break;
        case DRV_CFP_ACT_OB_REDIRECT:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            if (!fld_val) {
                *action = DRV_CFP_ACT_OB_NONE;
            } else if (fld_val == CFP_53242_ACT_FWD_OP_DROP_COPY) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_CPU) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else if (fld_val == CFP_53242_ACT_FWD_VAL_REDIRECT_CPU_AND_MIRROR_TO) {
                    *act_param = DRV_FIELD_ACT_CHANGE_FWD_CPU | \
                                                DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                } else {
                    *act_param = fld_val;
                }
            } else {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * register value need to sub 24 for real port id.
                     */            
                    fld_val -= 24;
                }
                *act_param = fld_val;
            }
            break;
        case DRV_CFP_ACT_OB_DSCP_NEW:
        case DRV_CFP_ACT_OB_DSCP_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_DSCP_OB, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_OB_DSCP_CANCEL;
            }
            break;
        case DRV_CFP_ACT_CHANGE_CVID:
        case DRV_CFP_ACT_CHANGE_SPVID:
        case DRV_CFP_ACT_CHANGE_CVID_SPVID:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FLOW_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FLOW_INDEX, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *act_param = 0;
            }
            break;
        case DRV_CFP_ACT_PCP_NEW:
        case DRV_CFP_ACT_PCP_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_PCP_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_PCP, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_PCP_CANCEL;
            }
            break;
        case DRV_CFP_ACT_IB_COSQ_NEW:
        case DRV_CFP_ACT_IB_COSQ_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_COS, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_IB_COSQ_CANCEL;
            }
            break;
        case DRV_CFP_ACT_OB_COSQ_NEW:
        case DRV_CFP_ACT_OB_COSQ_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_COS, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_OB_COSQ_CANCEL;
            }
            break;
        case DRV_CFP_ACT_COSQ_CPU_NEW:
        case DRV_CFP_ACT_COSQ_CPU_CANCEL:
            (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_IMP_EN, entry, &fld_val);
            if (fld_val) {
                (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_COS_IMP, entry, &fld_val);
                *act_param = fld_val;
            } else {
                *action = DRV_CFP_ACT_COSQ_CPU_CANCEL;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_action_set
 *
 * Purpose:
 *     Set the CFP action type and parameters value to 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry(OUT) -cfp entry
 *     act_param - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 *     action                              act_param
 *     DRV_CFP_ACT_IB_MOD_INT_PRI     Priority
 *     DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL     XXX
 *     DRV_CFP_ACT_IB_NONE           XXX
 *     DRV_CFP_ACT_IB_REDIRECT     port number
 *     DRV_CFP_ACT_IB_APPEND     port number
 *     DRV_CFP_ACT_IB_FLOOD     XXX
 *     DRV_CFP_ACT_IB_DROP     XXX
 */
int
drv53242_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;
    soc_pbmp_t pbmp;

    assert(entry);
    
    SOC_PBMP_CLEAR(pbmp);
    switch (action) {
        /* Inband actions */
        case DRV_CFP_ACT_IB_NONE:
            /* Cancel FWD actions */
            fld_val = CFP_53242_ACT_FWD_OP_NONE;
            /* action */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_REDIRECT:
            if (act_param2) {
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);

                if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_CPU) {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU;
                } else if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE) {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_MIRROR_TO;
                } else {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU_AND_MIRROR_TO;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            } else if (act_param1 == CMIC_PORT(unit)) { /* Redirect to CPU */
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
                
                fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            } else {
                fld_val = CFP_53242_ACT_FWD_OP_REDIRECT;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
                fld_val = act_param1;
                if ((!SOC_PBMP_MEMBER(PBMP_ALL(unit), fld_val)) &&
                    (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD)) {
                    rv = SOC_E_PARAM;
                    break;
                }
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * each port need to add 24 for setting register 
                     */            
                    fld_val += 24;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_IB_APPEND:
            if (act_param2) {
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);

                if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_CPU) {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU;
                } else if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE) {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_MIRROR_TO;
                } else {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU_AND_MIRROR_TO;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            } else if (act_param1 == CMIC_PORT(unit)) { /* Copy to CPU */
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
                fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            } else { /* other ports appending */
                fld_val = CFP_53242_ACT_FWD_OP_APPEND;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
                fld_val = act_param1;
                if ((!SOC_PBMP_MEMBER(PBMP_ALL(unit), fld_val)) &&
                    (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD)) {
                    rv = SOC_E_PARAM;
                    break;
                }
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * each port need to add 24 for setting register 
                     */            
                    fld_val += 24;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_IB_FLOOD:
            fld_val = CFP_53242_ACT_FWD_OP_REDIRECT;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            fld_val = CFP_53242_ACT_FWD_VAL_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DROP:
            fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_IB_EN, entry, &fld_val);
            fld_val = CFP_53242_ACT_FWD_VAL_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DSCP_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DSCP_IB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_DSCP_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_IB_EN, entry, &fld_val);
            break;
        /* Outband actions */
        case DRV_CFP_ACT_OB_NONE:
            /* Cancel FWD actions */
            fld_val = CFP_53242_ACT_FWD_OP_NONE;
            /* action */
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_REDIRECT:
            if (act_param2) {
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);

                if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_CPU) {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU;
                } else if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE) {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_MIRROR_TO;
                } else {
                    fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU_AND_MIRROR_TO;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            } else if (act_param1 == CMIC_PORT(unit)) { /* Redirect to CPU */
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
                
                fld_val = CFP_53242_ACT_FWD_VAL_REDIRECT_CPU;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            } else {
            fld_val = CFP_53242_ACT_FWD_OP_REDIRECT;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            fld_val = act_param1;
            if ((!SOC_PBMP_MEMBER(PBMP_ALL(unit), fld_val)) &&
                (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD)) {
                rv = SOC_E_PARAM;
                break;
            }
            if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                /* 
                 * if not flood to all ports,
                 * each port need to add 24 for setting register 
                 */            
                fld_val += 24;
            }
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_OB_APPEND:
            if (act_param2) {
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);

                if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_CPU) {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU;
                } else if (act_param2 == DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE) {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_MIRROR_TO;
                } else {
                    fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU_AND_MIRROR_TO;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            } else if (act_param1 == CMIC_PORT(unit)) { /* Copy to CPU */
                fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
                fld_val = CFP_53242_ACT_FWD_VAL_COPY_CPU;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            } else { /* other ports appending */
                fld_val = CFP_53242_ACT_FWD_OP_APPEND;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
                fld_val = act_param1;
                if ((!SOC_PBMP_MEMBER(PBMP_ALL(unit), fld_val)) &&
                    (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD)) {
                    rv = SOC_E_PARAM;
                    break;
                }
                if (fld_val != CFP_53242_ACT_FWD_VAL_FLOOD) {
                    /* 
                     * if not flood to all ports,
                     * each port need to add 24 for setting register 
                     */            
                    fld_val += 24;
                }
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_OB_FLOOD:
            fld_val = CFP_53242_ACT_FWD_OP_REDIRECT;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            fld_val = CFP_53242_ACT_FWD_VAL_FLOOD;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_DROP:
            fld_val = CFP_53242_ACT_FWD_OP_DROP_COPY;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val);
            fld_val = CFP_53242_ACT_FWD_VAL_DROP;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_DSCP_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_DSCP_OB, entry, &fld_val);
            break;
        case DRV_CFP_ACT_OB_DSCP_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_DSCP_OB_EN, entry, &fld_val);
            break;
        /* Other actions */
        case DRV_CFP_ACT_CHANGE_CVID:
            fld_val = 0x1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FLOW_EN, entry, &fld_val);
            SOC_PBMP_WORD_SET(pbmp, 0, act_param2);
            rv = _drv53242_cfp_flow_based_vlan_idx_get(unit, act_param1, pbmp,
                                                       (int *) &fld_val);
            if (!rv) {
                /*
                 * Record the idx in entry.
                 * Idx for DRV_CFP_FIELD_NEW_FLOW_INDEX.
                 */
                entry->flow2vlan_sw_idx = fld_val;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FLOW_INDEX, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_CHANGE_SPVID:
            fld_val = 0x2;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FLOW_EN, entry, &fld_val);
            SOC_PBMP_WORD_SET(pbmp, 0, act_param2);
            rv = _drv53242_cfp_flow_based_vlan_idx_get(unit, act_param1, pbmp,
                                                       (int *) &fld_val);
            if (!rv) {
                entry->flow2vlan_sw_idx = fld_val;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FLOW_INDEX, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_CHANGE_CVID_SPVID:
            fld_val = 0x3;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FLOW_EN, entry, &fld_val);
            SOC_PBMP_WORD_SET(pbmp, 0, act_param2);
            rv = _drv53242_cfp_flow_based_vlan_idx_get(unit, act_param1, pbmp,
                                                       (int *) &fld_val);
            if (!rv) {
                entry->flow2vlan_sw_idx = fld_val;
                (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_FLOW_INDEX, entry, &fld_val);
            }
            break;
        case DRV_CFP_ACT_CHANGE_CVID_CANCEL:
        case DRV_CFP_ACT_CHANGE_SPVID_CANCEL:
        case DRV_CFP_ACT_CHANGE_CVID_SPVID_CANCEL:
            /* find the idx of FLOW2VLAN table in f_ent->drv_entry. */
            rv = _drv53242_cfp_flow_based_vlan_idx_remove(unit, entry->flow2vlan_sw_idx);
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FLOW_EN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_PCP_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_PCP_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_PCP, entry, &fld_val);
            break;
        case DRV_CFP_ACT_PCP_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_PCP_EN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_COSQ_NEW:
        case DRV_CFP_ACT_OB_COSQ_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_COS, entry, &fld_val);
            break;
        case DRV_CFP_ACT_IB_COSQ_CANCEL:
        case DRV_CFP_ACT_OB_COSQ_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_EN, entry, &fld_val);
            break;
        case DRV_CFP_ACT_COSQ_CPU_NEW:
            fld_val = 1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_IMP_EN, entry, &fld_val);
            fld_val = act_param1;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_COS_IMP, entry, &fld_val);
            break;
        case DRV_CFP_ACT_COSQ_CPU_CANCEL:
            fld_val = 0;
            (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_COS_IMP_EN, entry, &fld_val);
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 * Function: drv53242_cfp_control_get
 *
 * Purpose:
 *     Get the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2(OUT) -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP delimter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv53242_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, reg_32, fld_32;
    uint64  reg_64;

    switch (control_type) {
        case DRV_CFP_ENABLE:

            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_EN_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_EN_CTRLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, CFP_EN_CTRLr, (uint32 *)&reg_64, 
                EN_CFP_MAPf, (uint32 *)&fld_32);
            if ((fld_32 & (0x1 << param1))  != 0) {
                *param2 = TRUE;
            } else {
                *param2 = FALSE;
            }
            break;
        case DRV_CFP_FLOOD_TRUNK:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_GLOBAL_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_GLOBAL_CTLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, CFP_GLOBAL_CTLr, &reg_32, 
                TRUNK_HANDLEf, &fld_32);
            *param2 = (fld_32) ? 1 : 0;
            break;
        case DRV_CFP_FLOOD_VLAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_GLOBAL_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_GLOBAL_CTLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, CFP_GLOBAL_CTLr, &reg_32, 
                CFP_VLAN_HANDLEf, &fld_32);
            *param2 = (fld_32) ? 1 : 0;
            break;
        case DRV_CFP_ISPVLAN_DELIMITER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ISP_VIDr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ISP_VIDr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ISP_VIDr, &reg_32, 
                ISP_VLAN_DELIMITERf, &fld_32);
            *param2 = fld_32;
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function: drv53242_cfp_control_set
 *
 * Purpose:
 *     Set the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2 -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     control_type     param1     param2
 *     DRV_CFP_ENABLE, port number, enable
 *     DRV_CFP_FLOOD_TRUNK, XXX, enable
 *     DRV_CFP_FLOOD_VLAN, XXX, enable
 *     DRV_CFP_ISPVLAN_DELIMITER, XXX, ISP demimeter
 *     DRV_CFP_LLC_ENCAP, XXX, DSAP+SSAP+Control field
 *     DRV_CFP_SLICE_SELECT, port number, slice id map
 *     DRV_CFP_TCAM_RESET, XXX, XXX
 *     
 */
int
drv53242_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    int rv = SOC_E_NONE;
    uint32  reg_addr, reg_len, reg_32, fld_32;
    uint64  reg_64, temp;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, CFP_EN_CTRLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_EN_CTRLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }

            (DRV_SERVICES(unit)->reg_field_get)
                (unit, CFP_EN_CTRLr, (uint32 *)&reg_64, 
                EN_CFP_MAPf, (uint32 *)&fld_32);;
            temp = 0x1 << param1;
            if (param2) {
                fld_32 |= temp;
            } else {
                fld_32 &= ~temp;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_EN_CTRLr, (uint32 *)&reg_64, 
                EN_CFP_MAPf, (uint32 *)&fld_32);

            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, (uint32 *)&reg_64, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_FLOOD_TRUNK:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, 
                CFP_GLOBAL_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_GLOBAL_CTLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            fld_32 = (param2) ? 1 : 0; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_GLOBAL_CTLr, &reg_32, 
                TRUNK_HANDLEf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_FLOOD_VLAN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, 
                CFP_GLOBAL_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, CFP_GLOBAL_CTLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            fld_32 = (param2) ? 1 : 0; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, CFP_GLOBAL_CTLr, &reg_32, 
                VLAN_HANDLEf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_ISPVLAN_DELIMITER:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, ISP_VIDr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, ISP_VIDr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, ISP_VIDr, &reg_32, 
                ISP_VLAN_DELIMITERf, &fld_32);
            fld_32 = param2; 
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, ISP_VIDr, &reg_32, 
                ISP_VLAN_DELIMITERf, &fld_32);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_TCAM_RESET:
            rv = _drv53242_cfp_tcam_fill(unit, 0);
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}


/*
 * Function: drv53242_cfp_entry_read
 *
 * Purpose:
 *     Read the TCAM/ACTION/POLICY/METER raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 */
int
drv53242_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n", 
                    index, rv);
                return rv;
            }
            if ( (rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
 "drv_cfp_entry_read : failed to read meter ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_ACT:
            if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_METER:
            if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_read : failed to read meter ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_entry_search
 *
 * Purpose:
 *     Search the valid CFP entry from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - search flags
 *     index(OUT) - CFP entry index
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *
 */
int
drv53242_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    if ((rv = _drv53242_cfp_tcam_search(unit, flags, 
            index, entry)) != SOC_E_NONE) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_entry_search : failed to serach TCAM rv = %d\n",
            rv);
        return rv;
    }
    /* If found valid TCAM entry, 
     * get the correspoding TCAM, ACT and Meter data */
    if ((flags & DRV_CFP_SEARCH_GET) && (rv == SOC_E_NONE)) {
        if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_TCAM, *index, entry)) 
            != SOC_E_NONE) {
            soc_cm_debug(DK_ERR,
"drv_cfp_entry_search:failed to read tcam with index = 0x%x, rv = %d. \n",
                *index, rv);
            return rv;
        }
        if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_ACT, *index, entry)) 
            != SOC_E_NONE) {
            soc_cm_debug(DK_ERR,
"drv_cfp_entry_search:failed to read action ram with index = 0x%x, rv = %d. \n",
                *index, rv);
            return rv;
        }
        if ((rv = _drv53242_cfp_read(unit, DRV_CFP_RAM_METER, *index, entry)) 
            != SOC_E_NONE) {
            soc_cm_debug(DK_ERR,
"drv_cfp_entry_search :failed to read meter ram with index = 0x%x, rv = %d. \n",
                *index, rv);
            return rv;
        }
        entry->id = *index;
    }
    return rv;
}


/*
 * Function: drv53242_cfp_entry_write
 *
 * Purpose:
 *     Write the TCAM/ACTION/POLICY/METER raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 * Note:
 *     
 */
int
drv53242_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
    "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write:failed to write action ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write :failed to write meter ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
             if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
    "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_ACT:
             if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
 "drv_cfp_entry_write : failed to write ACT ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_METER:
             if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_METER, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
"drv_cfp_entry_write :failed to write METER ram with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_TCAM_INVALID:
             if ((rv = _drv53242_cfp_write(unit, DRV_CFP_RAM_TCAM_INVALID, index, entry)) 
                != SOC_E_NONE) {
                soc_cm_debug(DK_ERR,
    "drv_cfp_entry_write : failed to write invalid TCAM with index = 0x%x, rv = %d. \n",
                    index, rv);
                return rv;
            }
            entry->id = index; 
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_field_get
 *
 * Purpose:
 *     Get the field value from the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry -cfp entry
 *     fld_val(OUT) - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;
    uint64 fld_val64, mask64;
    uint32 temp, fld_val32;
    uint32 temp_hi = 0, temp_lo = 0;
    
    assert(entry);
    assert(fld_val);

    /*
     * Process combined field.
     */
    if ((mem_type == DRV_CFP_RAM_TCAM) || (mem_type == DRV_CFP_RAM_TCAM_MASK)) {
        if (field_type == DRV_CFP_FIELD_IP_SA) {
            /* Get value of DRV_CFP_FIELD_IP6_SA */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&fld_val64));
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(fld_val64);
            temp_lo = COMPILER_64_LO(fld_val64);
            COMPILER_64_SET(fld_val64, temp_lo, temp_hi);
#endif
            COMPILER_64_SHR(fld_val64, 12);
            COMPILER_64_TO_32_LO(*fld_val, fld_val64);
            return SOC_E_NONE;
        } else if (field_type == DRV_CFP_FIELD_IP_DA) {
            /* Get value of DRV_CFP_FIELD_IP6_SA */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&fld_val64));
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(fld_val64);
            temp_lo = COMPILER_64_LO(fld_val64);
            COMPILER_64_SET(fld_val64, temp_lo, temp_hi);
#endif
            temp_hi = 0x00000000;
            temp_lo = 0x00000fff;
            COMPILER_64_SET(mask64, temp_hi, temp_lo);
            COMPILER_64_AND(fld_val64, mask64);
            COMPILER_64_TO_32_LO(temp, fld_val64);

            /* Get value of DRV_CFP_FIELD_IP6_FLOW_ID */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_IP6_FLOW_ID, entry, &fld_val32));
            fld_val32 &= 0xfffff;
            *fld_val = (temp <<= 20) | fld_val32;

            return SOC_E_NONE;
        } else if (field_type == DRV_CFP_FIELD_L4SRC) {
            /* combine ICMPIGMP_TYPE/CODE */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_ICMPIGMP_TYPECODE, entry, &fld_val32));
            *fld_val = fld_val32;
            return SOC_E_NONE;
        }
    }

    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        if ((field_type == DRV_CFP_FIELD_SLICE_ID) || 
            (field_type == DRV_CFP_FIELD_IN_PBMP)) {
            /* For slice ID field, don;t care the TCAM type */
            mem_id = CFP_TCAM_S0m; 
        } else {
            if (( rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID,
                    entry, &fld_id)) < 0 ) {
                soc_cm_debug(DK_ERR,
                    "drv_cfp_field_get : UNKNOW SLICE ID. \n");
                return rv;
            }
            switch (fld_id) {
                case 0:
                    mem_id = CFP_TCAM_S0m;
                    break;
                case 1:
                    mem_id = CFP_TCAM_S1m;
                    break;
                case 2:
                    mem_id = CFP_TCAM_S2m;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
        }
        if (mem_type == DRV_CFP_RAM_TCAM) {
            data_p = entry->tcam_data;
        } else { /* mask data */
            data_p = entry->tcam_mask;
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = CFP_ACT_POLm;
        data_p = entry->act_data;
        break;
    case DRV_CFP_RAM_METER:
        mem_id = CFP_METERm;
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    if (( rv = _drv53242_cfp_field_mapping(unit, field_type, &fld_id)) < 0) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_field_get : UNKNOW FIELD ID. \n");
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;

    if (fieldinfo->len < 32) {
        mask = (1 << fieldinfo->len) - 1;
    } else {
        mask = -1;
    }
    
    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_val[0] = ((data_p[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {
        if (fieldinfo->flags & SOCF_LE) {
            for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
                if (len > 32) {
                    mask = 0xffffffff;
                } else {
                    mask = (1 << len) - 1;
                }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
                mask_lo = mask;
                mask_hi = 0;
                /* if field is not aligned with 32-bit word boundary */
                /* adjust hi and lo masks accordingly. */
                if (bp) {
                    mask_lo = mask << bp;
                    mask_hi = mask >> (32 - bp);
                }
                /* get field value --- 32 bits each time */
                fld_val[i] = (data_p[FIX_MEM_ORDER_E(wp++, meminfo)] 
                    & mask_lo) >> bp;
                if (mask_hi) {
                    fld_val[i] |= (data_p[FIX_MEM_ORDER_E(wp, meminfo)] 
                        & mask_hi) << (32 - bp);
                }
                i++;
            }
        } else {
            i = (len - 1) / 32;

            while (len > 0) {
                assert(i >= 0);
                fld_val[i] = 0;
                do {
                    fld_val[i] = (fld_val[i] << 1) |
                    ((data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                    (bp & (32 - 1))) & 1);
                    len--;
                    bp++;
                } while (len & (32 - 1));
                i--;
            }
        }
    }
    return rv;
}

/*
 * Function: drv53242_cfp_field_set
 *
 * Purpose:
 *     Set the field value to the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry(OUT) -cfp entry
 *     fld_val - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    int mem_id;
    uint32  fld_id;
    uint32  mask, mask_hi, mask_lo;
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    int i, wp, bp, len;
    uint32  *data_p;
    uint64 temp64, mask64;
    uint32 temp32;
    uint32 temp_hi = 0, temp_lo = 0;

    assert(entry);
    assert(fld_val);

    /*
     * Process combined field.
     */
    if ((mem_type == DRV_CFP_RAM_TCAM) || (mem_type == DRV_CFP_RAM_TCAM_MASK)) {
        if (field_type == DRV_CFP_FIELD_IP_SA) {
            /* Get value of DRV_CFP_FIELD_IP6_SA */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&temp64));
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(temp64);
            temp_lo = COMPILER_64_LO(temp64);
            COMPILER_64_SET(temp64, temp_lo, temp_hi);
#endif
            /* Place input value at bit 43~12 of field DRV_CFP_FIELD_IP6_SA */
            temp_hi = 0x00000000;
            temp_lo = 0x00000fff;
            COMPILER_64_SET(mask64, temp_hi, temp_lo);
            COMPILER_64_AND(temp64, mask64);

            COMPILER_64_SET(mask64, 0, *fld_val);
            COMPILER_64_SHL(mask64, 12);

            COMPILER_64_OR(temp64, mask64);
            
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(temp64);
            temp_lo = COMPILER_64_LO(temp64);
            COMPILER_64_SET(temp64, temp_lo, temp_hi);
#endif
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_set)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&temp64));
            return SOC_E_NONE;
        } else if (field_type == DRV_CFP_FIELD_IP_DA) {
            /* Get value of DRV_CFP_FIELD_IP6_SA */
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_get)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&temp64));
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(temp64);
            temp_lo = COMPILER_64_LO(temp64);
            COMPILER_64_SET(temp64, temp_lo, temp_hi);
#endif
            /* 
             * Place input value's upper 12 bits 
             * at DRV_CFP_FIELD_IP6_SA's lower 12 bits 
             */
            temp_hi = 0xffffffff;
            temp_lo = 0xfffff000;
            COMPILER_64_SET(mask64, temp_hi, temp_lo);
            COMPILER_64_AND(temp64, mask64);

            COMPILER_64_SET(mask64, 0, (*fld_val >> 20));
            COMPILER_64_OR(temp64, mask64);
#ifdef BE_HOST
            temp_hi = COMPILER_64_HI(temp64);
            temp_lo = COMPILER_64_LO(temp64);
            COMPILER_64_SET(temp64, temp_lo, temp_hi);
#endif
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_set)
                (unit, mem_type, DRV_CFP_FIELD_IP6_SA, entry, (uint32 *)&temp64));

            /* 
             * Place input value's lower 20 bits 
             * at DRV_CFP_FIELD_IP6_FLOW_ID.
             */
            temp32 = *fld_val & 0xfffff;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_set)
                (unit, mem_type, DRV_CFP_FIELD_IP6_FLOW_ID, entry, &temp32));

            return SOC_E_NONE;
        } else if (field_type == DRV_CFP_FIELD_L4SRC) {
            temp32 = *fld_val;
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->cfp_field_set)
                (unit, mem_type, DRV_CFP_FIELD_ICMPIGMP_TYPECODE, entry, &temp32));
            return SOC_E_NONE;
        }
    }

    switch (mem_type) {
    case DRV_CFP_RAM_TCAM:
    case DRV_CFP_RAM_TCAM_MASK:
        if (field_type == DRV_CFP_FIELD_SLICE_ID) {
            /* For slice ID field, don;t care the TCAM type */
            mem_id = CFP_TCAM_S0m; 
        } else {
            if (( rv = (DRV_SERVICES(unit)->cfp_field_get)
                (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID,
                    entry, &fld_id)) < 0 ) {
                soc_cm_debug(DK_ERR,
                    "drv_cfp_field_set : UNKNOW SLICE ID. \n");
                return rv;
            }
            switch (fld_id) {
                case 0:
                    mem_id = CFP_TCAM_S0m;
                    break;
                case 1:
                    mem_id = CFP_TCAM_S1m;
                    break;
                case 2:
                    mem_id = CFP_TCAM_S2m;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    return rv;
            }
        }
        if (mem_type == DRV_CFP_RAM_TCAM) {
            data_p = entry->tcam_data;
        } else { /* mask data */
            data_p = entry->tcam_mask;
        }
        break;
    case DRV_CFP_RAM_ACT:
        mem_id = CFP_ACT_POLm;
        data_p = entry->act_data;
        break;
    case DRV_CFP_RAM_METER:
        mem_id = CFP_METERm;
        data_p = entry->meter_data;
        break;
    default:
        rv = SOC_E_PARAM;
        return rv;
    }
    
    if (( rv = _drv53242_cfp_field_mapping(unit, field_type, &fld_id)) < 0) {
        soc_cm_debug(DK_ERR,
            "drv_cfp_field_set : UNKNOW FIELD ID. \n");
        return rv;
    }

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);
    SOC_FIND_FIELD(fld_id, meminfo->fields,
                             meminfo->nFields, fieldinfo);
    assert(fieldinfo);

    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips.
     */
    if ((fld_id == VALID_Rf) &&  *fld_val != 0){
        len = fieldinfo->len;
        *fld_val = 0;
        for (i = 0; i < len; i++) {
            *fld_val |= (0x1 << i);
        }
    }
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            data_p[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_val[i] << bp) & mask_lo);
            if (mask_hi) {
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                data_p[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_val[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            data_p[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_val[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
    return rv;
}

/*
 * Function: drv53242_cfp_meter_get
 *
 * Purpose:
 *     Get the meter value from CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry -cfp entry
 *     kbits_sec(OUT) -meter rate (kbits per second)
 *     kbits_burst(OUT) -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)    
{
    int rv = SOC_E_NONE;
    uint32  bkt_size;
    uint32 ref_u, ref_cnt;

    assert(entry);
    assert(kbits_sec);
    assert(kbits_burst);
    
    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_REF_UNIT, entry, &ref_u);
    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_REF_CNT, entry, &ref_cnt);
    if (ref_u == CFP_53242_METER_REF_UNIT_64K) {
        ref_cnt *= 625;
        *kbits_sec = ref_cnt / 10;
    } else {
        *kbits_sec = ref_cnt*1000;
    }

    (DRV_SERVICES(unit)->cfp_field_get)(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_BUCKET_SIZE, entry, &bkt_size);
    *kbits_burst = bkt_size*8*8;

    return rv;
}

/*
 * Function: drv53242_cfp_meter_set
 *
 * Purpose:
 *     Set the meter value to CFP entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) -cfp entry
 *     kbits_sec -meter rate (kbits per second)
 *     kbits_burst -meter bucket size (kbits)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst)
{
    int rv = SOC_E_NONE;
    uint32  bkt_size;
    uint32 ref_u, ref_cnt;

    assert(entry);
    if (kbits_sec) {
    /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_53242_METER_RATE_MAX) ||
            (kbits_sec < CFP_53242_METER_RATE_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : rate unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv53242_cfp_meter_rate2chip(unit, kbits_sec, &ref_u, &ref_cnt);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_UNIT, entry, &ref_u);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CNT, entry, &ref_cnt);
    } else {
        ref_u = 0;
        ref_cnt = 0;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_UNIT, entry, &ref_u);
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_REF_CNT, entry, &ref_cnt);
    }

    if (kbits_burst) {
        if ((kbits_burst > CFP_53242_METER_BURST_MAX) ||
            (kbits_burst < CFP_53242_METER_BURST_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : burst size unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        bkt_size = kbits_burst / 8; /* tranfer to KByte */
        bkt_size = bkt_size / 8; /* tranfer value to unit, to set register */
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_BUCKET_SIZE, entry, &bkt_size);
    } else {
        bkt_size = CFP_53242_METER_BURST_MIN / 8;
        bkt_size = bkt_size / 8;
        (DRV_SERVICES(unit)->cfp_field_set)(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_BUCKET_SIZE, entry, &bkt_size);
    }
    return rv;
}

int
drv53242_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst, 
                uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit)
{
    int rv = SOC_E_NONE;

    if (kbits_sec) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_sec > CFP_53242_METER_RATE_MAX) ||
            (kbits_sec < CFP_53242_METER_RATE_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : rate unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        _drv53242_cfp_meter_rate2chip(unit, kbits_sec, ref_unit, ref_cnt);
    } else {
        *ref_cnt = 0;
    }

    if (kbits_burst) {
        /*    coverity[unsigned_compare]    */
        if ((kbits_burst > CFP_53242_METER_BURST_MAX) ||
            (kbits_burst < CFP_53242_METER_BURST_MIN)) {
            soc_cm_debug(DK_ERR, 
                "drv_cfp_meter_set : burst size unsupported. \n");
            rv = SOC_E_UNAVAIL;
            return (rv);
        }
        *bucket_size = kbits_burst / 8; /* tranfer to KByte */
        *bucket_size = *bucket_size / 8; /* tranfer value to unit, to set register */
    } else {
        *bucket_size = 0;
    }

    return rv;
}


/*
 * Function: drv53242_cfp_qset_get
 *
 * Purpose:
 *     Get the qualify bit value from CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val(OUT) -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (entry->w[wp] & (1 << bp)) {
        *val = TRUE;
    } else {
        *val = FALSE;
    }

    return rv;
}

/*
 * Function: drv53242_cfp_qset_set
 *
 * Purpose:
 *     Set/Reset the qualify bit value to CFP entry. 
 *
 * Parameters:
 *     unit - BCM device number
 *     qual - qualify ID
 *     entry -CFP entry
 *     val -TRUE/FALSE
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv53242_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val)
{
    int rv = SOC_E_NONE;
    uint32  wp, bp, temp = 0;

    assert(entry);
    
    wp = qual / 32;
    bp = qual & (32 - 1);
    if (val) {
        temp = 1;
    }

    if (temp) {
        entry->w[wp] |= (1 << bp);
    } else {
        entry->w[wp] &= ~(1 << bp);
    }
    
    return rv;
}

/*
 * Function: drv53242_cfp_slice_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver cfp entry
 *     slice_id(OUT) - slice id for this entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable slice id for this entry.
 */
int
drv53242_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    uint32 i;
    int match;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];

    for (i=0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s0_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s0_qset[i]/32)] |= (0x1 << (s0_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 0; /* SLICE 0 */
        return SOC_E_NONE;
    }

    for (i=0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    i = 0;
    while (s1_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s1_qset[i]/32)] |= (0x1 << (s1_qset[i] % 32));
        i++;
    }
    match = TRUE;
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 1; /* SLICE 1 */
        return SOC_E_NONE;
    }
 
    for (i=0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    match = TRUE;
    i = 0;
    while (s2_qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(s2_qset[i]/32)] |= (0x1 << (s2_qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        if (entry->w[i] & ~slice[i]) {
            match = FALSE;
            break;
        }
    }
    if (match) {
        *slice_id = 2; /* SLICE 2 */
        return SOC_E_NONE;
    }

    return SOC_E_RESOURCE;
}


/*
 * Function: drv53242_cfp_slice_to_qset
 *
 * Purpose:
 *     According to slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) - driver cfp entry
 *     slice_id - slice id 
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - un-support slice id.
 */
int
drv53242_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    uint32 i;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];
    int     *qset;

    switch(slice_id) {
        case 0:
            qset = s0_qset;
            break;
        case 1:
            qset = s1_qset;
            break;
        case 2:
            qset = s2_qset;
            break;
        default:
            return SOC_E_PARAM;
    }

    for (i=0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        slice[i] = 0;
    }
    i = 0;
    while (qset[i] != DRV_CFP_QUAL_INVALID) {
        slice[(qset[i]/32)] |= (0x1 << (qset[i] % 32));
        i++;
    }
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        entry->w[i] = slice[i];
    }
    return SOC_E_NONE;
}


/*
 * Function: drv53242_cfp_stat_get
 *
 * Purpose:
 *     Get the counter value from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter(OUT) -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv53242_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max, temp;
    
    assert(counter);

   index_max = soc_robo_mem_index_max(unit, CFP_STAT_IBm);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv53242_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv53242_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv53242_cfp_stat_read(unit, DRV_CFP_RAM_STAT_IB, index, 
                &temp);
            *counter = temp;
            _drv53242_cfp_stat_read(unit, DRV_CFP_RAM_STAT_OB, index, 
                &temp);
            *counter += temp;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_stat_set
 *
 * Purpose:
 *     Set the CFP counter value to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     stat_type - In-band/Out-band counter/Both
 *     index -entry index
 *     counter -counter value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
drv53242_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter)
{
    int rv = SOC_E_NONE;
    uint32  index_max;

    index_max = soc_robo_mem_index_max(unit, CFP_STAT_IBm);
    if (index > index_max) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (stat_type) {
        case DRV_CFP_STAT_INBAND:
            _drv53242_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            break;
        case DRV_CFP_STAT_OUTBAND:
            _drv53242_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        case DRV_CFP_STAT_ALL:
            _drv53242_cfp_stat_write(unit, DRV_CFP_RAM_STAT_IB, index, 
                counter);
            _drv53242_cfp_stat_write(unit, DRV_CFP_RAM_STAT_OB, index, 
                counter);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_udf_get
 *
 * Purpose:
 *     Get the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset(OUT) -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     For BCM53242, there are 10 UDFs.
 *     
 */
int 
drv53242_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base)
{
    int rv = SOC_E_NONE;
    uint32  reg_index, fld_index1, fld_index2, reg_addr, reg_len, temp;
    uint32  reg_32, idx_shift;
    uint32  offset_unit = 1;
    uint16 dev_id= 0;
    uint8 rev_id = 0;

    if (udf_index >=  CFP_53242_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM53242 
       0 ~ 2: UDFA0, A1, A2
       3 ~ 13: UDFB0~B10
       14 ~ 16: UDFC0, C1, C2
       17 ~ 24: UDFD0~D7
     */  

    if (udf_index <= 2) {
        reg_index = CFP_UDF_Ar;
        fld_index1 = UDF_A_OFFSETf;
        fld_index2 = UDF_A_REFf;
        idx_shift = udf_index - UDF_A_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else if ((udf_index >= 3) && (udf_index <= 13)) {
        reg_index = CFP_UDF_Br;
        fld_index1 = UDF_B_OFFSETf;
        fld_index2 = UDF_B_REFf;
        idx_shift = udf_index - UDF_B_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else if ((udf_index >= 14) && (udf_index <= 16)) {
        reg_index = CFP_UDF_Cr;
        fld_index1 = UDF_C_OFFSETf;
        fld_index2 = UDF_C_REFf;
        idx_shift = udf_index - UDF_C_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else { /* 17~24 */
        reg_index = CFP_UDF_Dr;
        fld_index1 = UDF_D_OFFSETf;
        fld_index2 = UDF_D_REFf;
        idx_shift = udf_index - UDF_D_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_index, 0, idx_shift);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);

    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    (DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, &reg_32, fld_index1, &temp);

    *offset = temp * offset_unit;

    (DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, &reg_32, fld_index2, &temp);
    switch (temp) {
        case 0:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG;
            break;
        case 1:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR;
            break;
        case 2:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR;
            break;
        case 3:
            soc_cm_get_id(unit, &dev_id, &rev_id);
            if (((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_B0_REV_ID)) || 
                ((dev_id == BCM53262_DEVICE_ID) && (rev_id == BCM53262_B0_REV_ID)) || 
                ((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_B1_REV_ID)) || 
                ((dev_id == BCM53262_DEVICE_ID) && (rev_id == BCM53262_B1_REV_ID))) {
                *base = DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME;
            } else {
                rv = SOC_E_INTERNAL;
            }
            break;
    }

    return rv;
}

/*
 * Function: drv53242_cfp_udf_set
 *
 * Purpose:
 *     Set the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - port numbrt
 *     udf_index -the index of user defined fields
 *     offset -offset value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     For BCM53242, there are 10 UDFs.
 *     
 */
int 
drv53242_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base)
{
    int rv = SOC_E_NONE;
    uint32  reg_index, fld_index1, fld_index2, reg_addr, reg_len, temp = 0;
    uint32  reg_32, idx_shift;
    uint32  offset_unit = 1;
    uint16 dev_id= 0;
    uint8 rev_id = 0;

    if (udf_index >=  CFP_53242_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (offset > CFP_53242_UDF_OFFSET_MAX) {
        return SOC_E_CONFIG;
    }

    /* For BCM53242 
       0 ~ 2: UDFA0, A1, A2
       3 ~ 13: UDFB0~B10
       14 ~ 16: UDFC0, C1, C2
       17 ~ 24: UDFD0~D7
     */  

    if (udf_index <= 2) {
        reg_index = CFP_UDF_Ar;
        fld_index1 = UDF_A_OFFSETf;
        fld_index2 = UDF_A_REFf;
        idx_shift = udf_index - UDF_A_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else if ((udf_index >= 3) && (udf_index <= 13)) {
        reg_index = CFP_UDF_Br;
        fld_index1 = UDF_B_OFFSETf;
        fld_index2 = UDF_B_REFf;
        idx_shift = udf_index - UDF_B_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else if ((udf_index >= 14) && (udf_index <= 16)) {
        reg_index = CFP_UDF_Cr;
        fld_index1 = UDF_C_OFFSETf;
        fld_index2 = UDF_C_REFf;
        idx_shift = udf_index - UDF_C_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    } else { /* 17~24 */
        reg_index = CFP_UDF_Dr;
        fld_index1 = UDF_D_OFFSETf;
        fld_index2 = UDF_D_REFf;
        idx_shift = udf_index - UDF_D_BASE_INDEX;
        offset_unit = 2; /* UNIT is 2 bytes */
    }

    reg_addr = (DRV_SERVICES(unit)->reg_addr)
        (unit, reg_index, 0, idx_shift);
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);

    if (offset_unit) { /* Check divide 0 case. It should never happen. */
        temp = offset / offset_unit;
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    (DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_32, fld_index1, &temp);

    switch(base) {
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG:
            temp = 0x0;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR:
            temp = 0x1;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR:
            temp = 0x2;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME:
            soc_cm_get_id(unit, &dev_id, &rev_id);
            if (((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_B0_REV_ID)) || 
                ((dev_id == BCM53262_DEVICE_ID) && (rev_id == BCM53262_B0_REV_ID)) || 
                ((dev_id == BCM53242_DEVICE_ID) && (rev_id == BCM53242_B1_REV_ID)) || 
                ((dev_id == BCM53262_DEVICE_ID) && (rev_id == BCM53262_B1_REV_ID))) {
                temp = 0x3;
            } else {
                return SOC_E_PARAM;
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    (DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, &reg_32, fld_index2, &temp);

    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, &reg_32, reg_len)) < 0) {
        return rv;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_ranger
 *
 * Purpose:
 *     Check ranger type and parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     flags - types of ranger
 *     min - range lower bound
 *     max - ranger upper bound
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv53242_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max)
{
    int rv = SOC_E_NONE;

    if ((flags & DRV_FIELD_RANGE_DSTPORT) || (flags & DRV_FIELD_RANGE_SRCPORT)) {
        if (max > 0xffff) {
            rv = SOC_E_PARAM;
        }
    } else if (flags & DRV_FIELD_RANGE_VLAN) {
        if (max > 4095) {
            rv = SOC_E_PARAM;
        }
    } else {
        rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_range_set
 *
 * Purpose:
 *     Check ranger type and parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     type - types of ranger
 *     id - ranger's id (if applicable)
 *     param1 - for L4 src/dst port and vlan, it's the minimum value
 *              for other rangers, it's the value to set to chip
 *     param2 - for L4 src/dst port and vlan, it's the maximum value
 *              for other rangers, it's reserved
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv53242_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_index = 0;
    uint32  reg_32, reg_addr, reg_len, temp;

    if ((type == DRV_CFP_RANGE_SRCPORT) || 
        (type == DRV_CFP_RANGE_DSTPORT)) {
        switch(id) {
            case 0:
                fld_index = RCC0f;
                break;
            case 1:
                fld_index = RCC1f;
                break;
            case 2:
                fld_index = RCC2f;
                break;
            case 3:
                fld_index = RCC3f;
                break;
            default:
                return SOC_E_PARAM;
        }
    }

    if (type == DRV_CFP_RANGE_TCP_HEADER_LEN) {
        fld_index = TCP_HEAD_LENf;
    }
    if (type == DRV_CFP_RANGE_BIG_ICMP) {
        fld_index = BIG_ICMPf;
    }

    switch(type) {
        case DRV_CFP_RANGE_SRCPORT:
        case DRV_CFP_RANGE_DSTPORT:
            /* Select source port or destination port */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, RANGE_CHECKER_CTLr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, RANGE_CHECKER_CTLr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            if (type == DRV_CFP_RANGE_SRCPORT) {
                temp = 0;
            } else {
                temp = 1;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, RANGE_CHECKER_CTLr, &reg_32, fld_index, &temp);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }

            /* set min and max range */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, L4PORT_RANGE_CHECKERr, 0, id);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, L4PORT_RANGE_CHECKERr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, L4PORT_RANGE_CHECKERr, &reg_32, SMALLf, &param1);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, L4PORT_RANGE_CHECKERr, &reg_32, LARGEf, &param2);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            
            break;
        case DRV_CFP_RANGE_VLAN:
            /* set min and max range */
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, VID_RANGE_CHECKERr, 0, id);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, VID_RANGE_CHECKERr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, VID_RANGE_CHECKERr, &reg_32, SMALLf, &param1);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, VID_RANGE_CHECKERr, &reg_32, LARGEf, &param2);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_RANGE_TCP_HEADER_LEN:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, OTHER_CHECKERr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, OTHER_CHECKERr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }

            (DRV_SERVICES(unit)->reg_field_set)
                (unit, OTHER_CHECKERr, &reg_32, fld_index, &param1);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        case DRV_CFP_RANGE_BIG_ICMP:
            reg_addr = (DRV_SERVICES(unit)->reg_addr)
                (unit, OTHER_CHECKERr, 0, 0);
            reg_len = (DRV_SERVICES(unit)->reg_length_get)
                (unit, OTHER_CHECKERr);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, OTHER_CHECKERr, &reg_32, fld_index, &param1);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_32, reg_len)) < 0) {
                return rv;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function: drv53242_cfp_range_get
 *
 * Purpose:
 *     Get ranger parameters.
 *
 * Parameters:
 *     unit - BCM device number
 *     type - types of ranger
 *     id - ranger's id (if applicable)
 *     param1 - for L4 src/dst port and vlan, it's the minimum value
 *              for other rangers, it's the value to set to chip
 *     param2 - for L4 src/dst port and vlan, it's the maximum value
 *              for other rangers, it's reserved
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 */
int 
drv53242_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2)
{
    int rv = SOC_E_NONE;
    uint32  reg_32, temp = 0;

    switch(type) {
        case DRV_CFP_RANGE_SRCPORT:
            rv = REG_READ_RANGE_CHECKER_CTLr(unit, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            if (id == 0) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC0f, &temp);
            } else if (id == 1) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC1f, &temp);
            } else if (id == 2) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC2f, &temp);
            } else if (id == 3) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC3f, &temp);
            } else {
                return SOC_E_PARAM;
            }
            if (temp) {
                /* The ranger is configured as L4 destination port */
                return SOC_E_PARAM;
            }

            rv = REG_READ_L4PORT_RANGE_CHECKERr(unit, id, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            soc_L4PORT_RANGE_CHECKERr_field_get(unit, &reg_32, SMALLf, param1);
            soc_L4PORT_RANGE_CHECKERr_field_get(unit, &reg_32, LARGEf, param2);
            break;
        case DRV_CFP_RANGE_DSTPORT:
            rv = REG_READ_RANGE_CHECKER_CTLr(unit, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            if (id == 0) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC0f, &temp);
            } else if (id == 1) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC1f, &temp);
            } else if (id == 2) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC2f, &temp);
            } else if (id == 3) {
                soc_RANGE_CHECKER_CTLr_field_get(unit, &reg_32, RCC3f, &temp);
            } else {
                return SOC_E_PARAM;
            }
            if (!temp) {
                /* The ranger is configured as L4 source port */
                return SOC_E_PARAM;
            }

            rv = REG_READ_L4PORT_RANGE_CHECKERr(unit, id, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            soc_L4PORT_RANGE_CHECKERr_field_get(unit, &reg_32, SMALLf, param1);
            soc_L4PORT_RANGE_CHECKERr_field_get(unit, &reg_32, LARGEf, param2);
            break;
        case DRV_CFP_RANGE_VLAN:
            rv = REG_READ_VID_RANGE_CHECKERr(unit, id, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            soc_VID_RANGE_CHECKERr_field_get(unit, &reg_32, SMALLf, param1);
            soc_VID_RANGE_CHECKERr_field_get(unit, &reg_32, LARGEf, param2);
            break;
        case DRV_CFP_RANGE_TCP_HEADER_LEN:
            rv = REG_READ_OTHER_CHECKERr(unit, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            soc_OTHER_CHECKERr_field_get(unit, &reg_32, TCP_HEAD_LENf, param1);
            break;
        case DRV_CFP_RANGE_BIG_ICMP:
            rv = REG_READ_OTHER_CHECKERr(unit, &reg_32);
            SOC_IF_ERROR_RETURN(rv);
            soc_OTHER_CHECKERr_field_get(unit, &reg_32, BIG_ICMPf, param1);
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }
    return rv;
}

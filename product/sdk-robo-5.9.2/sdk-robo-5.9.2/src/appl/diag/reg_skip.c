/* 
 * $Id: reg_skip.c,v 1.56.6.2 Broadcom SDK $
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
 * Diag routines to identify registers that are only implemented
 * on a subset of ports/cos.
 */

#include <appl/diag/system.h>

/*
 * Description:
 *  There are many per-port registers in Triumph which only exist on
 *  a subset of ports as described by PORTLIST and PERPORT_MASKBITS
 *  attributes in the regsfile. There are also several indexed registers
 *  with different number of elements depending on the port as described
 *  by the NUMELS_PERPORT attribute. These routines identify whether
 *  a register should be skipped for a given port/cos index. These routines 
 *  also adjust the mask for these special registers so unimplemented
 *  bits get skipped during register tests. 
 */



#ifdef BCM_TRIUMPH_SUPPORT
STATIC int
reg_mask_subset_tr(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int tr_port_init = 0;
    STATIC pbmp_t tr_8pg_ports;
    STATIC pbmp_t tr_higig_ports;
    STATIC pbmp_t tr_24q_ports;
    STATIC pbmp_t tr_non_cpu_ports;
    STATIC pbmp_t tr_all_ports;
    uint64 temp_mask;
    pbmp_t *pbmp;

    if (!tr_port_init) {
        /* 8PG_PORTS = [2,14,26..31] */
        SOC_PBMP_CLEAR(tr_8pg_ports);
        if (IS_XE_PORT(unit, 2) || IS_HG_PORT(unit, 2)) {
            SOC_PBMP_PORT_ADD(tr_8pg_ports, 2);
        }
        if (IS_XE_PORT(unit, 14) || IS_HG_PORT(unit, 14)) {
            SOC_PBMP_PORT_ADD(tr_8pg_ports, 14);
        }
        if (IS_XE_PORT(unit, 26) || IS_HG_PORT(unit, 26)) {
            SOC_PBMP_PORT_ADD(tr_8pg_ports, 26);
        }
        if (IS_XE_PORT(unit, 27) || IS_HG_PORT(unit, 27)) {
            SOC_PBMP_PORT_ADD(tr_8pg_ports, 27);
        }
        SOC_PBMP_PORT_ADD(tr_8pg_ports, 28);
        SOC_PBMP_PORT_ADD(tr_8pg_ports, 29);
        SOC_PBMP_PORT_ADD(tr_8pg_ports, 30); 
        SOC_PBMP_PORT_ADD(tr_8pg_ports, 31);

        /* Higig ports [0,2,14,26,27,28,29,30,31] */
        SOC_PBMP_CLEAR(tr_higig_ports);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 0);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 2);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 14);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 26);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 27);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 28);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 29);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 30);
        SOC_PBMP_PORT_ADD(tr_higig_ports, 31);

        /*  24Q_PORTS = [2,3,14,15,26..32,43] */
        SOC_PBMP_CLEAR(tr_24q_ports);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 2);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 3);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 14);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 15);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 26);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 27);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 28);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 29);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 30);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 31);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 32);
        SOC_PBMP_PORT_ADD(tr_24q_ports, 43);

        /* all port except CMIC */
        SOC_PBMP_CLEAR(tr_non_cpu_ports);
        SOC_PBMP_ASSIGN(tr_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(tr_non_cpu_ports, 0);
 
        /* All ports */
        SOC_PBMP_CLEAR(tr_all_ports); 
        SOC_PBMP_ASSIGN(tr_all_ports, PBMP_ALL(unit)); 
 
        tr_port_init = 1; 
    }
    if (ainfo->port < 0) {
        return 0;
    }
 
    switch(ainfo->reg) {
        case MAC_CTRLr:
        case MAC_XGXS_CTRLr:
        case MAC_XGXS_STATr:
        case MAC_TXMUXCTRLr:
        case MAC_CNTMAXSZr:
        case MAC_CORESPARE0r:
        case MAC_TXCTRLr:
        case MAC_TXMACSAr:
        case MAC_TXMAXSZr:
        case MAC_TXPSETHRr:
        case MAC_TXSPARE0r:
        case ITPOKr:
        case ITXPFr: 
        case ITFCSr:
        case ITUCr:
        case ITMCAr:
        case ITBCAr:
        case ITOVRr:
        case ITFRGr:
        case ITPKTr:
        case IT64r:
        case IT127r:
        case IT255r:
        case IT511r:
        case IT1023r:
        case IT1518r:
        case IT2047r:
        case IT4095r:
        case IT9216r:
        case IT16383r:
        case ITMAXr:
        case ITUFLr:
        case ITERRr:
        case ITBYTr:
        case MAC_RXCTRLr:
        case MAC_RXMACSAr:
        case MAC_RXMAXSZr:
        case MAC_RXLSSCTRLr:
        case MAC_RXLSSSTATr:
        case MAC_RXSPARE0r:
        case IR64r:
        case IR127r:
        case IR255r:
        case IR511r:
        case IR1023r:
        case IR1518r:
        case IR2047r:
        case IR4095r:
        case IR9216r:
        case IR16383r:
        case IRMAXr:
        case IRPKTr:
        case IRFCSr:
        case IRUCr:
        case IRMCAr:
        case IRBCAr:
        case IRXCFr:
        case IRXPFr:
        case IRXUOr:
        case IRJBRr:
        case IROVRr:
        case IRFLRr:
        case IRPOKr:
        case IRMEGr:
        case IRMEBr:
        case IRBYTr:
        case IRUNDr:
        case IRFRGr:
        case IRERBYTr:
        case IRERPKTr:
        case IRJUNKr:
        case MAC_TXLLFCCTRLr:
        case MAC_TXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGCNTr:
            if (IS_GE_PORT(unit, ainfo->port)) {
                goto skip;
            }
            break;
        default:
            break;
    }

    switch(ainfo->reg) {
        case IE2E_CONTROLr:
        case ING_MODMAP_CTRLr:
        case IHG_LOOKUPr:
        case ICONTROL_OPCODE_BITMAPr:
        case UNKNOWN_HGI_BITMAPr:
        case IUNHGIr:
        case ICTRLr:
        case IBCASTr:
        case ILTOMCr:
        case IIPMCr:
        case IUNKOPCr:
            pbmp = &tr_higig_ports;
            break;
        case PG_THRESH_SELr:
        case PORT_PRI_GRP0r:
        case PORT_PRI_GRP1r:
        case PORT_PRI_XON_ENABLEr:
            pbmp = &tr_8pg_ports;
            break;
        case ECN_CONFIGr:
        case HOL_STAT_PORTr:
        case PORT_WREDPARAM_CELLr:
        case PORT_WREDPARAM_YELLOW_CELLr:
        case PORT_WREDPARAM_RED_CELLr:
        case PORT_WREDPARAM_NONTCP_CELLr:
        case PORT_WREDCONFIG_CELLr:
        case PORT_WREDAVGQSIZE_CELLr:
        case PORT_WREDPARAM_PACKETr:
        case PORT_WREDPARAM_YELLOW_PACKETr:
        case PORT_WREDPARAM_RED_PACKETr:
        case PORT_WREDPARAM_NONTCP_PACKETr:
        case PORT_WREDCONFIG_PACKETr:
        case PORT_WREDAVGQSIZE_PACKETr:
        case ESCONFIGr:
        case COSMASKr:
        case MINSPCONFIGr:
            pbmp = &tr_non_cpu_ports;
            break;
        /* ING_PORTS_24Q */
        case ING_COS_MODEr:
        /* MMU_24Q_PORTS */
        case COS_MODEr:
        case S1V_CONFIGr:
        case S1V_COSWEIGHTSr:
        case S1V_COSMASKr:
        case S1V_MINSPCONFIGr:
        case S1V_WDRRCOUNTr:
            pbmp = &tr_24q_ports;
            break;
        /* MMU_PERPORTPERCOS_REGS */
        case OP_QUEUE_CONFIG_CELLr:
        case OP_QUEUE_CONFIG1_CELLr:
        case OP_QUEUE_CONFIG_PACKETr:
        case OP_QUEUE_CONFIG1_PACKETr:
        case OP_QUEUE_LIMIT_YELLOW_CELLr:
        case OP_QUEUE_LIMIT_YELLOW_PACKETr:
        case OP_QUEUE_LIMIT_RED_CELLr:
        case OP_QUEUE_LIMIT_RED_PACKETr:
        case OP_QUEUE_RESET_OFFSET_CELLr:
        case OP_QUEUE_RESET_OFFSET_PACKETr:
        case OP_QUEUE_MIN_COUNT_CELLr:
        case OP_QUEUE_MIN_COUNT_PACKETr:
        case OP_QUEUE_SHARED_COUNT_CELLr:
        case OP_QUEUE_SHARED_COUNT_PACKETr:
        case OP_QUEUE_TOTAL_COUNT_CELLr:
        case OP_QUEUE_TOTAL_COUNT_PACKETr:
        case OP_QUEUE_RESET_VALUE_CELLr:
        case OP_QUEUE_RESET_VALUE_PACKETr:
        case DROP_PKT_CNTr:
        case DROP_BYTE_CNTr:
            if (ainfo->port == 0) {
                return 0;
            } else if (ainfo->idx < 8) {
                pbmp = &tr_all_ports;
            } else if (ainfo->idx < 24) {
                pbmp = &tr_24q_ports;
            } else {
                goto skip;
            }
            break;
        /* MMU_PERPORTPERCOS_NOCPU_REGS */
        case WREDPARAM_CELLr:
        case WREDPARAM_YELLOW_CELLr:
        case WREDPARAM_RED_CELLr:
        case WREDPARAM_NONTCP_CELLr:
        case WREDCONFIG_CELLr:
        case WREDAVGQSIZE_CELLr:
        case WREDPARAM_PACKETr:
        case WREDPARAM_YELLOW_PACKETr:
        case WREDPARAM_RED_PACKETr:
        case WREDPARAM_NONTCP_PACKETr:
        case WREDCONFIG_PACKETr:
        case WREDAVGQSIZE_PACKETr:
        /* MMU_MTRO_REGS */
        case MINBUCKETCONFIG_64r:
        case MINBUCKETr:
        case MAXBUCKETCONFIG_64r:
        case MAXBUCKETr:
        /* MMU_ES_REGS */
        case COSWEIGHTSr:
        case WDRRCOUNTr:
            if (ainfo->port == 0) {
                goto skip;
            } else if (ainfo->idx < 8) {
                pbmp = &tr_all_ports;
            } else {
                pbmp = &tr_24q_ports;
            }
            break;
        /* MMU_PERPORTPERPRI_REGS */
        case PG_RESET_OFFSET_CELLr:
        case PG_RESET_OFFSET_PACKETr:
        case PG_RESET_FLOOR_CELLr:
        case PG_MIN_CELLr:
        case PG_MIN_PACKETr:
        case PG_HDRM_LIMIT_CELLr:
        case PG_HDRM_LIMIT_PACKETr:
        case PG_COUNT_CELLr:
        case PG_COUNT_PACKETr:
        case PG_MIN_COUNT_CELLr:
        case PG_MIN_COUNT_PACKETr:
        case PG_PORT_MIN_COUNT_CELLr:
        case PG_PORT_MIN_COUNT_PACKETr:
        case PG_SHARED_COUNT_CELLr:
        case PG_SHARED_COUNT_PACKETr:
        case PG_HDRM_COUNT_CELLr:
        case PG_HDRM_COUNT_PACKETr:
        case PG_GBL_HDRM_COUNTr:
        case PG_RESET_VALUE_CELLr:
        case PG_RESET_VALUE_PACKETr:
            if (ainfo->idx == 0) {
                pbmp = &tr_all_ports;
            } else {
                pbmp = &tr_8pg_ports;
            }
            break;
        default:
            pbmp = &tr_all_ports;
            break; 
    }

    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip; 
    }

    if (mask != NULL) {
        switch(ainfo->reg) {
            case HOL_STAT_PORTr:
            case ECN_CONFIGr:
                if (!SOC_PBMP_MEMBER(tr_24q_ports, ainfo->port)) {
                    /* adjust mask for ports without 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
            case TOQ_QUEUESTAT_64r:
            case TOQ_ACTIVATEQ_64r:
            case TOQEMPTY_64r:
            case DEQ_AGINGMASK_64r:
                if (SOC_PBMP_MEMBER(tr_24q_ports, ainfo->port)) {
                    /* adjust mask for ports with 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x00ffffff);
                    COMPILER_64_AND(*mask, temp_mask);
                } else if (ainfo->port != 0) {
                    /* remaining ports only have 8 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
        }
    }
    return 0; 
skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
STATIC int
reg_mask_subset_sh(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int sh_port_init = 0;
    STATIC pbmp_t sh_non_cpu_ports;
    STATIC pbmp_t sh_non_il_non_cpu_ports;
    STATIC pbmp_t sh_port_link_ports;
    STATIC pbmp_t sh_il_ports;
    STATIC pbmp_t sh_all_ports;
    pbmp_t *pbmp;

    if (!sh_port_init) {
        /* all port except CMIC */
        SOC_PBMP_CLEAR(sh_non_cpu_ports);
        SOC_PBMP_ASSIGN(sh_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(sh_non_cpu_ports, 0);

        /* All ports except Interlaken and CPU */
        SOC_PBMP_CLEAR(sh_non_il_non_cpu_ports);
        SOC_PBMP_ASSIGN(sh_non_il_non_cpu_ports, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(sh_non_il_non_cpu_ports, PBMP_IL_ALL(unit));

        /* All port link ports */
        SOC_PBMP_CLEAR(sh_port_link_ports);
        SOC_PBMP_ASSIGN(sh_port_link_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(sh_port_link_ports, 0);
        SOC_PBMP_WORD_SET(sh_port_link_ports, 0, 
                          (SOC_PBMP_WORD_GET(sh_port_link_ports, 0) & 0x1ff));

        /* All Interlaken ports */
        SOC_PBMP_CLEAR(sh_il_ports);
        SOC_PBMP_ASSIGN(sh_il_ports, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(sh_il_ports, sh_port_link_ports);

        /* All ports */
        SOC_PBMP_CLEAR(sh_all_ports);
        SOC_PBMP_ASSIGN(sh_all_ports, PBMP_ALL(unit));

        sh_port_init = 1;
    }
    if (ainfo->port < 0) {
        return 0;
    }

    switch(ainfo->reg) {
        case BUCKET_ECCr:
        case CONFIG_ECCr:
        case HOL_STAT_PORTr:
        case TOQ_QUEUESTATr:
        case TOQ_ACTIVATEQr:
        case TOQEMPTYr:
        case ECN_CONFIGr:
        case ES_PORTGRP_WDRR_WEIGHTSr:
        case MTRO_SHAPE_MINMASKr:
        case MMU_TO_XLP_BKP_STATUSr:
        case XLP_TO_MMU_BKP_STATUSr:
        case PORT_WREDPARAM_CELLr:
        case PORT_WREDPARAM_YELLOW_CELLr:
        case PORT_WREDPARAM_RED_CELLr:
        case PORT_WREDPARAM_NONTCP_CELLr:
        case PORT_WREDCONFIG_CELLr:
        case PORT_WREDAVGQSIZE_CELLr:
        case PORT_LB_WREDAVGQSIZE_CELLr:
        case PORT_WREDCONFIG_ECCPr:
        case PORT_WREDPARAM_END_CELLr:
        case PORT_WREDPARAM_PRI0_END_CELLr:
        case PORT_WREDPARAM_PRI0_START_CELLr:
        case PORT_WREDPARAM_RED_END_CELLr:
        case PORT_WREDPARAM_RED_START_CELLr:
        case PORT_WREDPARAM_START_CELLr:
        case PORT_WREDPARAM_YELLOW_END_CELLr:
        case PORT_WREDPARAM_YELLOW_START_CELLr:
        case PORT_WRED_THD_0_ECCPr:
        case PORT_WRED_THD_1_ECCPr:
        case WREDPARAM_CELLr:
        case WREDPARAM_YELLOW_CELLr:
        case WREDPARAM_RED_CELLr:
        case WREDPARAM_NONTCP_CELLr:
        case WREDCONFIG_CELLr:
        case WREDAVGQSIZE_CELLr:
        case WREDPARAM_END_CELLr:
        case WREDPARAM_PRI0_END_CELLr:
        case WREDPARAM_PRI0_START_CELLr:
        case WREDPARAM_RED_END_CELLr:
        case WREDPARAM_RED_START_CELLr:
        case WREDPARAM_START_CELLr:
        case WREDPARAM_YELLOW_END_CELLr:
        case WREDPARAM_YELLOW_START_CELLr:
        case WREDCONFIG_ECCPr:
        case WRED_DEBUG_ENQ_DROP_PORTr:
        case WRED_THD_0_ECCPr:
        case WRED_THD_1_ECCPr:
        case COSWEIGHTSr:
        case MINSPCONFIGr:
        case WDRRCOUNTr:
        case MINBUCKETCONFIGr:
        case MINBUCKETCONFIG1r:
        case MINBUCKETr:
        case MAXBUCKETCONFIGr:
        case MAXBUCKETCONFIG1r:
        case MAXBUCKETr:
        case MMU_TO_XPORT_BKPr:
        case DEQ_AGINGMASKr:
        case MMU_LLFC_RX_CONFIGr:
        case XPORT_TO_MMU_BKPr:
        case UNIMAC_PFC_CTRLr:
        case MAC_PFC_REFRESH_CTRLr:
        case MAC_PFC_TYPEr:
        case MAC_PFC_OPCODEr:
        case MAC_PFC_DA_0r:
        case MAC_PFC_DA_1r:
        case MACSEC_PROG_TX_CRCr:
        case MACSEC_CNTRLr:
        case TS_STATUS_CNTRLr:
        case TX_TS_DATAr:
        case BMAC_PFC_CTRLr:
        case BMAC_PFC_TYPEr:
        case BMAC_PFC_OPCODEr:
        case BMAC_PFC_DA_LOr:
        case BMAC_PFC_DA_HIr:
        case BMAC_PFC_COS0_XOFF_CNTr:
        case BMAC_PFC_COS1_XOFF_CNTr:
        case BMAC_PFC_COS2_XOFF_CNTr:
        case BMAC_PFC_COS3_XOFF_CNTr:
        case BMAC_PFC_COS4_XOFF_CNTr:
        case BMAC_PFC_COS5_XOFF_CNTr:
        case BMAC_PFC_COS6_XOFF_CNTr:
        case BMAC_PFC_COS7_XOFF_CNTr:
        case BMAC_PFC_COS8_XOFF_CNTr:
        case BMAC_PFC_COS9_XOFF_CNTr:
        case BMAC_PFC_COS10_XOFF_CNTr:
        case BMAC_PFC_COS11_XOFF_CNTr:
        case BMAC_PFC_COS12_XOFF_CNTr:
        case BMAC_PFC_COS13_XOFF_CNTr:
        case BMAC_PFC_COS14_XOFF_CNTr:
        case BMAC_PFC_COS15_XOFF_CNTr:
        case PFC_COS0_XOFF_CNTr:
        case PFC_COS1_XOFF_CNTr:
        case PFC_COS2_XOFF_CNTr:
        case PFC_COS3_XOFF_CNTr:
        case PFC_COS4_XOFF_CNTr:
        case PFC_COS5_XOFF_CNTr:
        case PFC_COS6_XOFF_CNTr:
        case PFC_COS7_XOFF_CNTr:
        case PFC_COS8_XOFF_CNTr:
        case PFC_COS9_XOFF_CNTr:
        case PFC_COS10_XOFF_CNTr:
        case PFC_COS11_XOFF_CNTr:
        case PFC_COS12_XOFF_CNTr:
        case PFC_COS13_XOFF_CNTr:
        case PFC_COS14_XOFF_CNTr:
        case PFC_COS15_XOFF_CNTr:
            pbmp = &sh_non_cpu_ports;
            break;
        default:
            pbmp = &sh_all_ports;
            break;
    }

    switch(ainfo->reg) {
        /* EP_PERPORTPERCOS_REGS */
        case EGR_PERQ_XMT_COUNTERSr:
        /* MMU_PERPORTPERCOS_REGS */
        case OP_QUEUE_CONFIGr:
        case OP_QUEUE_RESET_OFFSETr:
        case OP_QUEUE_MIN_COUNTr:
        case OP_QUEUE_SHARED_COUNTr:
        case OP_QUEUE_TOTAL_COUNTr:
        case OP_QUEUE_RESET_VALUEr:
        case OP_QUEUE_LIMIT_YELLOWr:
        case OP_QUEUE_LIMIT_REDr:
        case HOLDROP_PKT_CNTr:
            if (ainfo->port == 0) {
                return 0;
            } else if (ainfo->idx < 10) { 
                pbmp = &sh_all_ports;
            } else {
                goto skip;
            }
            break;
        default:
            break;
    }

    if (SOC_REG_INFO(unit, ainfo->reg).block == SOC_BLK_XLPORT) {
        pbmp = &sh_non_il_non_cpu_ports;
    } else if ((SOC_REG_INFO(unit, ainfo->reg).block == SOC_BLK_MS_ISEC) || 
               (SOC_REG_INFO(unit, ainfo->reg).block == SOC_BLK_MS_ESEC)) {
        pbmp = &sh_port_link_ports;
    } else if (SOC_REG_INFO(unit, ainfo->reg).block == SOC_BLK_IL) {
        pbmp = &sh_il_ports;
    }

    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip;
    }
    return 0;

skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_SCORPION_SUPPORT
STATIC int
reg_mask_subset_sc(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int sc_port_init = 0;
    STATIC pbmp_t sc_no_ports;
    STATIC pbmp_t sc_xports;
    STATIC pbmp_t sc_non_cpu_ports;
    STATIC pbmp_t sc_all_ports;
    pbmp_t *pbmp;

    if (!sc_port_init) {
        /* all port except CMIC */
        SOC_PBMP_CLEAR(sc_non_cpu_ports);
        SOC_PBMP_ASSIGN(sc_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(sc_non_cpu_ports, 0);

        SOC_PBMP_CLEAR(sc_xports);
        SOC_PBMP_ASSIGN(sc_xports, PBMP_GX_ALL(unit));
        SOC_PBMP_PORT_REMOVE(sc_xports, 0);

        /* All ports */
        SOC_PBMP_CLEAR(sc_all_ports);
        SOC_PBMP_ASSIGN(sc_all_ports, PBMP_ALL(unit));

        /* No ports */
        SOC_PBMP_CLEAR(sc_no_ports);

        sc_port_init = 1;
    }
    if (ainfo->port < 0) {
        return 0;
    }

    switch(ainfo->reg) {
        case HOL_STAT_PORTr:
        case TOQ_QUEUESTATr:
        case TOQ_ACTIVATEQr:
        case TOQEMPTYr:
        case ECN_CONFIGr:
        case PORT_WREDPARAM_CELLr:
        case PORT_WREDPARAM_YELLOW_CELLr:
        case PORT_WREDPARAM_RED_CELLr:
        case PORT_WREDPARAM_NONTCP_CELLr:
        case PORT_WREDCONFIG_CELLr:
        case PORT_WREDAVGQSIZE_CELLr:
        case WREDPARAM_CELLr:
        case WREDPARAM_YELLOW_CELLr:
        case WREDPARAM_RED_CELLr:
        case WREDPARAM_NONTCP_CELLr:
        case WREDCONFIG_CELLr:
        case WREDAVGQSIZE_CELLr:
        case COSWEIGHTSr:
        case MINSPCONFIGr:
        case WDRRCOUNTr:
        case MINBUCKETCONFIGr:
        case MINBUCKETCONFIG1r:
        case MINBUCKETr:
        case MAXBUCKETCONFIGr:
        case MAXBUCKETCONFIG1r:
        case MAXBUCKETr:
        case MMU_TO_XPORT_BKPr:
        case DEQ_AGINGMASKr:
            pbmp = &sc_non_cpu_ports;
            break;
        case MMU_LLFC_RX_CONFIGr:
        case XPORT_TO_MMU_BKPr:
            pbmp = &sc_xports;
            break;
        case UNIMAC_PFC_CTRLr:
        case MAC_PFC_REFRESH_CTRLr:
        case MAC_PFC_TYPEr:
        case MAC_PFC_OPCODEr:
        case MAC_PFC_DA_0r:
        case MAC_PFC_DA_1r:
        case MACSEC_PROG_TX_CRCr:
        case MACSEC_CNTRLr:
        case TS_STATUS_CNTRLr:
        case TX_TS_DATAr:
            if (soc_feature(unit, soc_feature_priority_flow_control)) {
                pbmp = &sc_non_cpu_ports;
            } else {
                pbmp = &sc_no_ports;
            }
            break;
        case BMAC_PFC_CTRLr:
        case BMAC_PFC_TYPEr:
        case BMAC_PFC_OPCODEr:
        case BMAC_PFC_DA_LOr:
        case BMAC_PFC_DA_HIr:
        case BMAC_PFC_COS0_XOFF_CNTr:
        case BMAC_PFC_COS1_XOFF_CNTr:
        case BMAC_PFC_COS2_XOFF_CNTr:
        case BMAC_PFC_COS3_XOFF_CNTr:
        case BMAC_PFC_COS4_XOFF_CNTr:
        case BMAC_PFC_COS5_XOFF_CNTr:
        case BMAC_PFC_COS6_XOFF_CNTr:
        case BMAC_PFC_COS7_XOFF_CNTr:
        case BMAC_PFC_COS8_XOFF_CNTr:
        case BMAC_PFC_COS9_XOFF_CNTr:
        case BMAC_PFC_COS10_XOFF_CNTr:
        case BMAC_PFC_COS11_XOFF_CNTr:
        case BMAC_PFC_COS12_XOFF_CNTr:
        case BMAC_PFC_COS13_XOFF_CNTr:
        case BMAC_PFC_COS14_XOFF_CNTr:
        case BMAC_PFC_COS15_XOFF_CNTr:
        case PFC_COS0_XOFF_CNTr:
        case PFC_COS1_XOFF_CNTr:
        case PFC_COS2_XOFF_CNTr:
        case PFC_COS3_XOFF_CNTr:
        case PFC_COS4_XOFF_CNTr:
        case PFC_COS5_XOFF_CNTr:
        case PFC_COS6_XOFF_CNTr:
        case PFC_COS7_XOFF_CNTr:
        case PFC_COS8_XOFF_CNTr:
        case PFC_COS9_XOFF_CNTr:
        case PFC_COS10_XOFF_CNTr:
        case PFC_COS11_XOFF_CNTr:
        case PFC_COS12_XOFF_CNTr:
        case PFC_COS13_XOFF_CNTr:
        case PFC_COS14_XOFF_CNTr:
        case PFC_COS15_XOFF_CNTr:
            if (soc_feature(unit, soc_feature_priority_flow_control)) {
                pbmp = &sc_xports;
            } else {
                pbmp = &sc_no_ports;
            }
            break;
        default:
            pbmp = &sc_all_ports;
            break;
    }

    switch(ainfo->reg) {
        /* EP_PERPORTPERCOS_REGS */
        case EGR_PERQ_XMT_COUNTERSr:
        /* MMU_PERPORTPERCOS_REGS */
        case OP_QUEUE_CONFIGr:
        case OP_QUEUE_RESET_OFFSETr:
        case OP_QUEUE_MIN_COUNTr:
        case OP_QUEUE_SHARED_COUNTr:
        case OP_QUEUE_TOTAL_COUNTr:
        case OP_QUEUE_RESET_VALUEr:
        case OP_QUEUE_LIMIT_YELLOWr:
        case OP_QUEUE_LIMIT_REDr:
        case HOLDROP_PKT_CNTr:
            if (ainfo->port == 0) {
                return 0;
            } else if (ainfo->idx < 10) { 
                pbmp = &sc_all_ports;
            } else {
                goto skip;
            }
            break;
        default:
            break;
    }
    
    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip;
    }
    return 0;

skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_SCORPION_SUPPORT */

#ifdef BCM_TRIUMPH2_SUPPORT
STATIC int
reg_mask_subset_tr2(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int ch_port_init = 0;
    STATIC pbmp_t ch_2pg_ports;
    STATIC pbmp_t ch_8pg_ports;
    STATIC pbmp_t ch_24q_ports;
    STATIC pbmp_t ch_24q_ports_with_cpu;
    STATIC pbmp_t ch_ext_ports;
    STATIC pbmp_t ch_non_cpu_ports;
    STATIC pbmp_t ch_all_ports;
    STATIC pbmp_t ch_lb_ports;
    STATIC pbmp_t ch_all_ports_with_mmu;
    STATIC pbmp_t ch_gxports;
    STATIC pbmp_t ch_cmic;
    uint64 temp_mask;
    pbmp_t *pbmp;

    if (!ch_port_init) {
        /* 8PG_PORTS = [26..30,34,38,42,46,50,54] */
        SOC_PBMP_CLEAR(ch_8pg_ports);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 26);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 27);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 28);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 29);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 30); 
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 34);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 38);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 42);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 46);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 50);
        SOC_PBMP_PORT_ADD(ch_8pg_ports, 54);

        /*  24Q_PORTS = [26..31,34,38,39,42,43,46,50,51,54] */
        SOC_PBMP_CLEAR(ch_24q_ports);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 26);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 27);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 28);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 29);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 30);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 31);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 34);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 38);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 39);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 42);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 43);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 46);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 50);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 51);
        SOC_PBMP_PORT_ADD(ch_24q_ports, 54);
        SOC_PBMP_ASSIGN(ch_24q_ports_with_cpu, ch_24q_ports);
        SOC_PBMP_PORT_ADD(ch_24q_ports_with_cpu, 0);

        /* 2PG_PORTS = [0,1..25,31..33,35..37,39..41,43..45,47..49,51..53,55,56] */
        SOC_PBMP_CLEAR(ch_2pg_ports);
        SOC_PBMP_ASSIGN(ch_2pg_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 26);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 27);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 28);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 29);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 30); 
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 34);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 38);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 42);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 46);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 50);
        SOC_PBMP_PORT_REMOVE(ch_2pg_ports, 54);
        SOC_PBMP_PORT_ADD(ch_2pg_ports, 55);
        SOC_PBMP_PORT_ADD(ch_2pg_ports, 56);

        /* GXPORTS = [26, 27, 28, 29] */
        SOC_PBMP_CLEAR(ch_gxports);
        SOC_PBMP_PORT_ADD(ch_gxports, 26);
        SOC_PBMP_PORT_ADD(ch_gxports, 27);
        SOC_PBMP_PORT_ADD(ch_gxports, 28);
        SOC_PBMP_PORT_ADD(ch_gxports, 29);
       
        /* Loopback pbmp */
        SOC_PBMP_CLEAR(ch_lb_ports);
        SOC_PBMP_ASSIGN(ch_lb_ports, PBMP_LB(unit));

        /* All ports except CMIC */
        SOC_PBMP_CLEAR(ch_non_cpu_ports);
        SOC_PBMP_ASSIGN(ch_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(ch_non_cpu_ports, 0);

        /* All ports except CMIC and loopback - external ports */
        SOC_PBMP_CLEAR(ch_ext_ports);
        SOC_PBMP_ASSIGN(ch_ext_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(ch_ext_ports, 0);
        SOC_PBMP_PORT_REMOVE(ch_ext_ports, 54);
 
        /* All ports (excluding the internal MMU ports) */
        SOC_PBMP_CLEAR(ch_all_ports); 
        SOC_PBMP_ASSIGN(ch_all_ports, PBMP_ALL(unit)); 

        /* All ports (including the internal MMU ports) */
        SOC_PBMP_CLEAR(ch_all_ports_with_mmu); 
        SOC_PBMP_ASSIGN(ch_all_ports_with_mmu, PBMP_ALL(unit)); 
        SOC_PBMP_PORT_ADD(ch_all_ports_with_mmu, 55);
        SOC_PBMP_PORT_ADD(ch_all_ports_with_mmu, 56);

        /* CPU port */
        SOC_PBMP_CLEAR(ch_cmic); 
        SOC_PBMP_ASSIGN(ch_cmic, PBMP_CMIC(unit));

        ch_port_init = 1; 
    }
    if (ainfo->port < 0) {
        return 0;
    }
 
    switch(ainfo->reg) {
        case MAC_CTRLr:
        case MAC_XGXS_CTRLr:
        case MAC_XGXS_STATr:
        case MAC_TXMUXCTRLr:
        case MAC_CNTMAXSZr:
        case MAC_CORESPARE0r:
        case MAC_TXCTRLr:
        case MAC_TXMACSAr:
        case MAC_TXMAXSZr:
        case MAC_TXPSETHRr:
        case MAC_TXSPARE0r:
        case MAC_TXPPPCTRLr:
        case ITPOKr:
        case ITXPFr: 
        case ITFCSr:
        case ITXPPr:
        case ITUCr:
        case ITMCAr:
        case ITBCAr:
        case ITOVRr:
        case ITFRGr:
        case ITPKTr:
        case IT64r:
        case IT127r:
        case IT255r:
        case IT511r:
        case IT1023r:
        case IT1518r:
        case IT2047r:
        case IT4095r:
        case IT9216r:
        case IT16383r:
        case ITMAXr:
        case ITUFLr:
        case ITERRr:
        case ITBYTr:
        case MAC_RXCTRLr:
        case MAC_RXMACSAr:
        case MAC_RXMAXSZr:
        case MAC_RXLSSCTRLr:
        case MAC_RXLSSSTATr:
        case MAC_RXSPARE0r:
        case IR64r:
        case IR127r:
        case IR255r:
        case IR511r:
        case IR1023r:
        case IR1518r:
        case IR2047r:
        case IR4095r:
        case IR9216r:
        case IR16383r:
        case IRMAXr:
        case IRPKTr:
        case IRFCSr:
        case IRUCr:
        case IRMCAr:
        case IRBCAr:
        case IRXCFr:
        case IRXPFr:
        case IRXPPr:
        case IRXUOr:
        case IRJBRr:
        case IROVRr:
        case IRFLRr:
        case IRPOKr:
        case IRMEGr:
        case IRMEBr:
        case IRBYTr:
        case IRUNDr:
        case IRFRGr:
        case IRERBYTr:
        case IRERPKTr:
        case IRJUNKr:
        case MAC_TXLLFCCTRLr:
        case MAC_TXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGCNTr:
            if (IS_GE_PORT(unit, ainfo->port)) {
                goto skip;
            }
            break;
        case EGR_PORT_REQUESTSr:
            if (IS_LB_PORT(unit, ainfo->port)) {
                goto skip;
            }
            break;
        case MAC_PFC_COS0_XOFF_CNTr:
        case MAC_PFC_COS1_XOFF_CNTr:
        case MAC_PFC_COS2_XOFF_CNTr:
        case MAC_PFC_COS3_XOFF_CNTr:
        case MAC_PFC_COS4_XOFF_CNTr:
        case MAC_PFC_COS5_XOFF_CNTr:
        case MAC_PFC_COS6_XOFF_CNTr:
        case MAC_PFC_COS7_XOFF_CNTr:
        case MAC_PFC_COS8_XOFF_CNTr:
        case MAC_PFC_COS9_XOFF_CNTr:
        case MAC_PFC_COS10_XOFF_CNTr:
        case MAC_PFC_COS11_XOFF_CNTr:
        case MAC_PFC_COS12_XOFF_CNTr:
        case MAC_PFC_COS13_XOFF_CNTr:
        case MAC_PFC_COS14_XOFF_CNTr:
        case MAC_PFC_COS15_XOFF_CNTr:
            /* Not implemented in HW */
            goto skip;
            break;
        default:
            break;
    }

    switch(ainfo->reg) {
        case PG_THRESH_SELr:
        case PORT_PRI_GRP0r:
        case PORT_PRI_GRP1r:
        case PORT_PRI_XON_ENABLEr:
            pbmp = &ch_8pg_ports;
            break;
        case PG_THRESH_SEL2r:
        case PORT_PRI_GRP2r:
            pbmp = &ch_2pg_ports;
            break;
        case ECN_CONFIGr:
        case HOL_STAT_PORTr:
        case PORT_WREDPARAM_CELLr:
        case PORT_WREDPARAM_YELLOW_CELLr:
        case PORT_WREDPARAM_RED_CELLr:
        case PORT_WREDPARAM_NONTCP_CELLr:
        case PORT_WREDCONFIG_CELLr:
        case PORT_WREDAVGQSIZE_CELLr:
        case PORT_WREDPARAM_PACKETr:
        case PORT_WREDPARAM_YELLOW_PACKETr:
        case PORT_WREDPARAM_RED_PACKETr:
        case PORT_WREDPARAM_NONTCP_PACKETr:
        case PORT_WREDCONFIG_PACKETr:
        case PORT_WREDAVGQSIZE_PACKETr:
        case BKPMETERINGCONFIG_64r:
        case BKPMETERINGBUCKETr:
        case MTRI_IFGr:
            pbmp = &ch_ext_ports;
            break;
        case COSMASKr:
        case MINSPCONFIGr:
            pbmp = &ch_non_cpu_ports;
            break;
        case S1V_CONFIGr:
        case S1V_COSWEIGHTSr:
        case S1V_COSMASKr:
        case S1V_MINSPCONFIGr:
        case S1V_WDRRCOUNTr:
            pbmp = &ch_24q_ports;
            break;
        /* MMU_WLP_PERCOS_REGS */
        case OP_QUEUE_FIRST_FRAGMENT_CONFIG_CELLr:
        case OP_QUEUE_FIRST_FRAGMENT_RESET_OFFSET_CELLr:
        case OP_QUEUE_FIRST_FRAGMENT_CONFIG_PACKETr:
        case OP_QUEUE_FIRST_FRAGMENT_RESET_OFFSET_PACKETr:
        case OP_QUEUE_FIRST_FRAGMENT_COUNT_CELLr:
        case OP_QUEUE_FIRST_FRAGMENT_COUNT_PACKETr:
        case OP_QUEUE_REDIRECT_CONFIG_CELLr:
        case OP_QUEUE_REDIRECT_RESET_OFFSET_CELLr:
        case OP_QUEUE_REDIRECT_CONFIG_PACKETr:
        case OP_QUEUE_REDIRECT_RESET_OFFSET_PACKETr:
        case OP_QUEUE_REDIRECT_COUNT_CELLr:
        case OP_QUEUE_REDIRECT_COUNT_PACKETr:
        case OP_QUEUE_REDIRECT_XQ_CONFIG_PACKETr:
        case OP_QUEUE_REDIRECT_XQ_RESET_OFFSET_PACKETr:
        case OP_QUEUE_REDIRECT_XQ_COUNT_PACKETr:
        case OP_PORT_FIRST_FRAGMENT_DISC_RESUME_THD_CELLr:
        case OP_PORT_FIRST_FRAGMENT_DISC_RESUME_THD_PACKETr:
        case OP_PORT_FIRST_FRAGMENT_DISC_SET_THD_CELLr:
        case OP_PORT_FIRST_FRAGMENT_DISC_SET_THD_PACKETr:
        case OP_PORT_FIRST_FRAGMENT_COUNT_CELLr:
        case OP_PORT_FIRST_FRAGMENT_COUNT_PACKETr:
        case FIRST_FRAGMENT_DROP_STATE_CELLr:
        case FIRST_FRAGMENT_DROP_STATE_PACKETr:
        case OP_PORT_REDIRECT_COUNT_CELLr:
        case OP_PORT_REDIRECT_COUNT_PACKETr:
        case REDIRECT_DROP_STATE_CELLr:
        case REDIRECT_DROP_STATE_PACKETr:
        case REDIRECT_XQ_DROP_STATE_PACKETr: 
        case OP_PORT_REDIRECT_DISC_RESUME_THD_CELLr:
        case OP_PORT_REDIRECT_DISC_RESUME_THD_PACKETr:
        case OP_PORT_REDIRECT_DISC_SET_THD_CELLr:
        case OP_PORT_REDIRECT_DISC_SET_THD_PACKETr:
        case OP_PORT_REDIRECT_XQ_DISC_RESUME_THD_PACKETr:
        case OP_PORT_REDIRECT_XQ_DISC_SET_THD_PACKETr:
        case OP_PORT_REDIRECT_XQ_COUNT_PACKETr:
            pbmp = &ch_lb_ports;
            break;
        case PORT_MIN_CELLr:
        case PORT_MIN_PACKETr:
        case PORT_SHARED_LIMIT_CELLr:
        case PORT_SHARED_LIMIT_PACKETr:
        case PORT_COUNT_CELLr:
        case PORT_COUNT_PACKETr:
        case PORT_MIN_COUNT_CELLr:
        case PORT_MIN_COUNT_PACKETr:
        case PORT_SHARED_COUNT_CELLr:
        case PORT_SHARED_COUNT_PACKETr:
            pbmp = &ch_all_ports_with_mmu;
            break;
        default:
            pbmp = &ch_all_ports;
            break;
    }

    switch(ainfo->reg) {
        /* MMU_PERPORTPERCOS_REGS */
        case OP_QUEUE_CONFIG_CELLr:
        case OP_QUEUE_CONFIG1_CELLr:
        case OP_QUEUE_CONFIG_PACKETr:
        case OP_QUEUE_CONFIG1_PACKETr:
        case OP_QUEUE_LIMIT_YELLOW_CELLr:
        case OP_QUEUE_LIMIT_YELLOW_PACKETr:
        case OP_QUEUE_LIMIT_RED_CELLr:
        case OP_QUEUE_LIMIT_RED_PACKETr:
        case OP_QUEUE_RESET_OFFSET_CELLr:
        case OP_QUEUE_RESET_OFFSET_PACKETr:
        case OP_QUEUE_MIN_COUNT_CELLr:
        case OP_QUEUE_MIN_COUNT_PACKETr:
        case OP_QUEUE_SHARED_COUNT_CELLr:
        case OP_QUEUE_SHARED_COUNT_PACKETr:
        case OP_QUEUE_TOTAL_COUNT_CELLr:
        case OP_QUEUE_TOTAL_COUNT_PACKETr:
        case OP_QUEUE_RESET_VALUE_CELLr:
        case OP_QUEUE_RESET_VALUE_PACKETr:
        case OP_QUEUE_LIMIT_RESUME_COLOR_CELLr:
        case OP_QUEUE_LIMIT_RESUME_COLOR_PACKETr:
        /* MMU_PERPORTPERCOS_REGS_CTR */
        case DROP_PKT_CNTr:
        case DROP_BYTE_CNTr:
            if (ainfo->port == 0) {
                return 0;
            } else if (ainfo->idx < 8) {
                pbmp = &ch_all_ports;
            } else if ((ainfo->idx < 24) && (ainfo->port == 54)) {
                goto skip;
            } else if ((ainfo->idx < 24)) {
                pbmp = &ch_24q_ports;
            } else {
                pbmp = &ch_cmic;
            }
            break;

        /* MMU_PERPORTPERCOS_NOCPU_REGS */
        case WREDPARAM_CELLr:
        case WREDPARAM_YELLOW_CELLr:
        case WREDPARAM_RED_CELLr:
        case WREDPARAM_NONTCP_CELLr:
        case WREDCONFIG_CELLr:
        case WREDAVGQSIZE_CELLr:
        case WREDPARAM_PACKETr:
        case WREDPARAM_YELLOW_PACKETr:
        case WREDPARAM_RED_PACKETr:
        case WREDPARAM_NONTCP_PACKETr:
        case WREDCONFIG_PACKETr:
        case WREDAVGQSIZE_PACKETr:
            if (ainfo->port == 0) {
                goto skip;
            } else if (ainfo->idx < 8) {
                pbmp = &ch_non_cpu_ports;
            } else {
                pbmp = &ch_24q_ports;
            }
            break;

        /* MMU_MTRO_REGS */
        case MINBUCKETCONFIG_64r:
        case MINBUCKETr:
        case MAXBUCKETCONFIG_64r:
        case MAXBUCKETr:
            if (ainfo->idx < 8) {
                pbmp = &ch_all_ports;
            } else if (ainfo->idx < 26) {
                pbmp = &ch_24q_ports_with_cpu;
            } else {
                pbmp = &ch_cmic;
            }
            break;

        /* MMU_ES_REGS */
        case COSWEIGHTSr:
        case WDRRCOUNTr:
            if (ainfo->idx < 8) {
                pbmp = &ch_all_ports;
            } else if (ainfo->idx < 10) {
                pbmp = &ch_24q_ports_with_cpu;
            } else {
                pbmp = &ch_cmic;
            }
            break;

        /* MMU_PERPORTPERPRI_REGS */
        case PG_RESET_OFFSET_CELLr:
        case PG_RESET_OFFSET_PACKETr:
        case PG_RESET_FLOOR_CELLr:
        
        case PG_MIN_CELLr:
        case PG_MIN_PACKETr:
        case PG_HDRM_LIMIT_CELLr:
        case PG_HDRM_LIMIT_PACKETr:
        case PG_COUNT_CELLr:
        case PG_COUNT_PACKETr:
        case PG_MIN_COUNT_CELLr:
        case PG_MIN_COUNT_PACKETr:
        case PG_PORT_MIN_COUNT_CELLr:
        case PG_PORT_MIN_COUNT_PACKETr:
        case PG_SHARED_COUNT_CELLr:
        case PG_SHARED_COUNT_PACKETr:
        case PG_HDRM_COUNT_CELLr:
        case PG_HDRM_COUNT_PACKETr:
        case PG_GBL_HDRM_COUNTr:
        case PG_RESET_VALUE_CELLr:
        case PG_RESET_VALUE_PACKETr:
            if (ainfo->idx < 2) {
                pbmp = &ch_all_ports_with_mmu;
            } else {
                pbmp = &ch_8pg_ports;
            }
            break;
 
        default:
            break; 
    }

    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip; 
    }

    if (mask != NULL) {
        switch(ainfo->reg) {
            case HOL_STAT_PORTr:
            case ECN_CONFIGr:
                if (!SOC_PBMP_MEMBER(ch_24q_ports, ainfo->port)) {
                    /* adjust mask for ports without 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
            case TOQ_QUEUESTAT_64r:
            case TOQ_ACTIVATEQ_64r:
            case TOQEMPTY_64r:
            case DEQ_AGINGMASK_64r:
            case SHAPING_MODEr:
            case EAV_MAXBUCKET_64r:
            case EAV_MINBUCKET_64r:
                if (SOC_PBMP_MEMBER(ch_24q_ports, ainfo->port)) {
                    /* adjust mask for ports with 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x00ffffff);
                    COMPILER_64_AND(*mask, temp_mask);
                } else if (ainfo->port != 0) {
                    /* remaining ports only have 8 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
        }
    }
    return 0; 
skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_ENDURO_SUPPORT
STATIC int
reg_mask_subset_en(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int en_port_init = 0;
    STATIC pbmp_t en_8pg_ports;
    STATIC pbmp_t en_higig_ports;
    STATIC pbmp_t en_24q_ports;
    STATIC pbmp_t en_24q_ports_with_cpu;
    STATIC pbmp_t en_non_cpu_ports;
    STATIC pbmp_t en_all_ports;
    STATIC pbmp_t en_cmic;
    uint64 temp_mask;
    pbmp_t *pbmp;

    if (!en_port_init) {
        /* Higig ports [0,26,27,28,29] */
        SOC_PBMP_CLEAR(en_higig_ports);
        SOC_PBMP_PORT_ADD(en_higig_ports, 0);
        SOC_PBMP_PORT_ADD(en_higig_ports, 26);
        SOC_PBMP_PORT_ADD(en_higig_ports, 27);
        SOC_PBMP_PORT_ADD(en_higig_ports, 28);
        SOC_PBMP_PORT_ADD(en_higig_ports, 29);

        /*  24Q_PORTS = [26,27,28,29] */
        SOC_PBMP_CLEAR(en_24q_ports);
        SOC_PBMP_PORT_ADD(en_24q_ports, 26);
        SOC_PBMP_PORT_ADD(en_24q_ports, 27);
        SOC_PBMP_PORT_ADD(en_24q_ports, 28);
        SOC_PBMP_PORT_ADD(en_24q_ports, 29);
        SOC_PBMP_ASSIGN(en_24q_ports_with_cpu, en_24q_ports);
        SOC_PBMP_PORT_ADD(en_24q_ports_with_cpu, 0);

        SOC_PBMP_CLEAR(en_8pg_ports);
        SOC_PBMP_PORT_ADD(en_8pg_ports, 26);
        SOC_PBMP_PORT_ADD(en_8pg_ports, 27);
        SOC_PBMP_PORT_ADD(en_8pg_ports, 28);
        SOC_PBMP_PORT_ADD(en_8pg_ports, 29);

        /* all port except CMIC */
        SOC_PBMP_CLEAR(en_non_cpu_ports);
        SOC_PBMP_ASSIGN(en_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(en_non_cpu_ports, 0);
 
        /* All ports */
        SOC_PBMP_CLEAR(en_all_ports); 
        SOC_PBMP_ASSIGN(en_all_ports, PBMP_ALL(unit)); 
 
        /* CPU port */
        SOC_PBMP_CLEAR(en_cmic); 
        SOC_PBMP_ASSIGN(en_cmic, PBMP_CMIC(unit));
 
        en_port_init = 1; 
    }
    if (ainfo->port < 0) {
        return 0;
    }
 
    switch(ainfo->reg) {
        case MAC_CTRLr:
        case MAC_XGXS_CTRLr:
        case MAC_XGXS_STATr:
        case MAC_TXMUXCTRLr:
        case MAC_CNTMAXSZr:
        case MAC_CORESPARE0r:
        case MAC_TXCTRLr:
        case MAC_TXMACSAr:
        case MAC_TXMAXSZr:
        case MAC_TXPSETHRr:
        case MAC_TXSPARE0r:
        case MAC_TXPPPCTRLr:
        case ITPOKr:
        case ITXPFr: 
        case ITFCSr:
        case ITUCr:
        case ITUCAr:
        case ITMCAr:
        case ITBCAr:
        case ITOVRr:
        case ITFRGr:
        case ITPKTr:
        case IT64r:
        case IT127r:
        case IT255r:
        case IT511r:
        case IT1023r:
        case IT1518r:
        case IT2047r:
        case IT4095r:
        case IT9216r:
        case IT16383r:
        case ITMAXr:
        case ITUFLr:
        case ITERRr:
        case ITBYTr:
        case MAC_RXCTRLr:
        case MAC_RXMACSAr:
        case MAC_RXMAXSZr:
        case MAC_RXLSSCTRLr:
        case MAC_RXLSSSTATr:
        case MAC_RXSPARE0r:
        case IR64r:
        case IR127r:
        case IR255r:
        case IR511r:
        case IR1023r:
        case IR1518r:
        case IR2047r:
        case IR4095r:
        case IR9216r:
        case IR16383r:
        case IRMAXr:
        case IRPKTr:
        case IRFCSr:
        case IRUCAr:
        case IRMCAr:
        case IRBCAr:
        case IRXCFr:
        case IRXPFr:
        case IRXUOr:
        case IRJBRr:
        case IROVRr:
        case IRFLRr:
        case IRPOKr:
        case IRMEGr:
        case IRMEBr:
        case IRBYTr:
        case IRUNDr:
        case IRFRGr:
        case IRERBYTr:
        case IRERPKTr:
        case IRJUNKr:
        case MAC_TXLLFCCTRLr:
        case MAC_TXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGCNTr:
            if (IS_GE_PORT(unit, ainfo->port)) {
                goto skip;
            }
            break;
        default:
            break;
    }

    switch(ainfo->reg) {
        case IE2E_CONTROLr:
        case ING_MODMAP_CTRLr:
        case IHG_LOOKUPr:
        case ICONTROL_OPCODE_BITMAPr:
        case UNKNOWN_HGI_BITMAPr:
        case IUNHGIr:
        case ICTRLr:
        case IBCASTr:
        case ILTOMCr:
        case IIPMCr:
        case IUNKOPCr:
            pbmp = &en_higig_ports;
            break;
        case PG_THRESH_SELr:
        case PORT_PRI_GRP0r:
        case PORT_PRI_GRP1r:
        case PORT_PRI_XON_ENABLEr:
            pbmp = &en_8pg_ports;
            break;
        case ECN_CONFIGr:
        case HOL_STAT_PORTr:
        case PORT_WREDPARAM_CELLr:
        case PORT_WREDPARAM_YELLOW_CELLr:
        case PORT_WREDPARAM_RED_CELLr:
        case PORT_WREDPARAM_NONTCP_CELLr:
        case PORT_WREDCONFIG_CELLr:
        case PORT_WREDAVGQSIZE_CELLr:
        case PORT_WREDPARAM_PACKETr:
        case PORT_WREDPARAM_YELLOW_PACKETr:
        case PORT_WREDPARAM_RED_PACKETr:
        case PORT_WREDPARAM_NONTCP_PACKETr:
        case PORT_WREDCONFIG_PACKETr:
        case PORT_WREDAVGQSIZE_PACKETr:
        case COSMASKr:
        case MINSPCONFIGr:
        case EGRMETERINGCONFIG_64r:
        case EGRMETERINGBUCKETr:
            pbmp = &en_non_cpu_ports;
            break;
        case ING_COS_MODEr:
        case COS_MODEr:
        case S1V_CONFIGr:
        case S1V_COSWEIGHTSr:
        case S1V_COSMASKr:
        case S1V_MINSPCONFIGr:
        case S1V_WDRRCOUNTr:
            pbmp = &en_24q_ports;
            break;
        default:
            pbmp = &en_all_ports;
            break;
    }

    switch(ainfo->reg) {
        /* MMU_PERPORTPERCOS_REGS */
        case OP_QUEUE_CONFIG_CELLr:
        case OP_QUEUE_CONFIG1_CELLr:
        case OP_QUEUE_CONFIG_PACKETr:
        case OP_QUEUE_CONFIG1_PACKETr:
        case OP_QUEUE_LIMIT_YELLOW_CELLr:
        case OP_QUEUE_LIMIT_YELLOW_PACKETr:
        case OP_QUEUE_LIMIT_RED_CELLr:
        case OP_QUEUE_LIMIT_RED_PACKETr:
        case OP_QUEUE_RESET_OFFSET_CELLr:
        case OP_QUEUE_RESET_OFFSET_PACKETr:
        case OP_QUEUE_MIN_COUNT_CELLr:
        case OP_QUEUE_MIN_COUNT_PACKETr:
        case OP_QUEUE_SHARED_COUNT_CELLr:
        case OP_QUEUE_SHARED_COUNT_PACKETr:
        case OP_QUEUE_TOTAL_COUNT_CELLr:
        case OP_QUEUE_TOTAL_COUNT_PACKETr:
        case OP_QUEUE_RESET_VALUE_CELLr:
        case OP_QUEUE_RESET_VALUE_PACKETr:
        case OP_QUEUE_LIMIT_RESUME_COLOR_CELLr:
        case OP_QUEUE_LIMIT_RESUME_COLOR_PACKETr:
        /* MMU_PERPORTPERCOS_REGS_CTR */
        case DROP_PKT_CNTr:
        case DROP_BYTE_CNTr:
            if (ainfo->port == 0) {
                return 0;
            } else if (ainfo->idx < 8) {
                pbmp = &en_all_ports;
            } else if (ainfo->idx < 24) {
                pbmp = &en_24q_ports;
            } else {
                pbmp = &en_cmic;
            }
            break;
        /* MMU_PERPORTPERCOS_NOCPU_REGS */
        case WREDPARAM_CELLr:
        case WREDPARAM_YELLOW_CELLr:
        case WREDPARAM_RED_CELLr:
        case WREDPARAM_NONTCP_CELLr:
        case WREDCONFIG_CELLr:
        case WREDAVGQSIZE_CELLr:
        case WREDPARAM_PACKETr:
        case WREDPARAM_YELLOW_PACKETr:
        case WREDPARAM_RED_PACKETr:
        case WREDPARAM_NONTCP_PACKETr:
        case WREDCONFIG_PACKETr:
        case WREDAVGQSIZE_PACKETr:
            if (ainfo->port == 0) {
                goto skip;
            } else if (ainfo->idx < 8) {
                pbmp = &en_non_cpu_ports;
            } else {
                pbmp = &en_24q_ports;
            }
            break;


        /* MMU_MTRO_REGS */
        case MINBUCKETCONFIG_64r:
        case MINBUCKETr:
        case MAXBUCKETCONFIG_64r:
        case MAXBUCKETr:
            if (ainfo->port == 0) {
                goto skip;
            } else if (ainfo->idx < 8) {
                pbmp = &en_all_ports;
            } else if (ainfo->idx < 26) {
                pbmp = &en_24q_ports_with_cpu;
            } else {
                pbmp = &en_cmic;
            }
            break;

        /* MMU_ES_REGS */
        case COSWEIGHTSr:
        case WDRRCOUNTr:
            if (ainfo->idx < 8) {
                pbmp = &en_all_ports;
            } else if (ainfo->idx < 10) {
                pbmp = &en_24q_ports_with_cpu;
            } else {
                pbmp = &en_cmic;
            }
            break;
        /* MMU_PERPORTPERPRI_REGS */
        case PG_RESET_OFFSET_CELLr:
        case PG_RESET_OFFSET_PACKETr:
        case PG_RESET_FLOOR_CELLr:
        
        case PG_MIN_CELLr:
        case PG_MIN_PACKETr:
        case PG_HDRM_LIMIT_CELLr:
        case PG_HDRM_LIMIT_PACKETr:
        case PG_COUNT_CELLr:
        case PG_COUNT_PACKETr:
        case PG_MIN_COUNT_CELLr:
        case PG_MIN_COUNT_PACKETr:
        case PG_PORT_MIN_COUNT_CELLr:
        case PG_PORT_MIN_COUNT_PACKETr:
        case PG_SHARED_COUNT_CELLr:
        case PG_SHARED_COUNT_PACKETr:
        case PG_HDRM_COUNT_CELLr:
        case PG_HDRM_COUNT_PACKETr:
        case PG_GBL_HDRM_COUNTr:
        case PG_RESET_VALUE_CELLr:
        case PG_RESET_VALUE_PACKETr:
            if (ainfo->idx != 0) {
                pbmp = &en_8pg_ports;
            }
            break;
        default:
            break; 
    }

    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip; 
    }

    if (mask != NULL) {
        switch(ainfo->reg) {
            case HOL_STAT_PORTr:
            case ECN_CONFIGr:
                if (!SOC_PBMP_MEMBER(en_24q_ports, ainfo->port)) {
                    /* adjust mask for ports without 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
            case TOQ_QUEUESTAT_64r:
            case TOQ_ACTIVATEQ_64r:
            case TOQEMPTY_64r:
            case DEQ_AGINGMASK_64r:
                if (SOC_PBMP_MEMBER(en_24q_ports, ainfo->port)) {
                    /* adjust mask for ports with 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x00ffffff);
                    COMPILER_64_AND(*mask, temp_mask);
                } else if (ainfo->port != 0) {
                    /* remaining ports only have 8 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
        }
    }
    return 0; 
skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_ENDURO_SUPPORT */

#ifdef BCM_HURRICANE_SUPPORT
STATIC int
reg_mask_subset_hu(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    STATIC int hu_port_init = 0;
    STATIC pbmp_t hu_8pg_ports;
    STATIC pbmp_t hu_24q_ports;
    STATIC pbmp_t hu_24q_ports_with_cpu;
    STATIC pbmp_t hu_higig_ports;
    STATIC pbmp_t hu_non_cpu_ports;
    STATIC pbmp_t hu_all_ports;
    STATIC pbmp_t hu_cmic;
    uint64 temp_mask;
    pbmp_t *pbmp;

    if (!hu_port_init) {
        /* Higig ports [0,26,27,28,29] */
        SOC_PBMP_CLEAR(hu_higig_ports);
        SOC_PBMP_PORT_ADD(hu_higig_ports, 0);
        SOC_PBMP_PORT_ADD(hu_higig_ports, 26);
        SOC_PBMP_PORT_ADD(hu_higig_ports, 27);
        SOC_PBMP_PORT_ADD(hu_higig_ports, 28);
        SOC_PBMP_PORT_ADD(hu_higig_ports, 29);

        /*  24Q_PORTS = NONE */
        SOC_PBMP_CLEAR(hu_24q_ports);
        SOC_PBMP_ASSIGN(hu_24q_ports_with_cpu, hu_24q_ports);
        SOC_PBMP_PORT_ADD(hu_24q_ports_with_cpu, 0);

        SOC_PBMP_CLEAR(hu_8pg_ports);
        SOC_PBMP_PORT_ADD(hu_8pg_ports, 26);
        SOC_PBMP_PORT_ADD(hu_8pg_ports, 27);
        SOC_PBMP_PORT_ADD(hu_8pg_ports, 28);
        SOC_PBMP_PORT_ADD(hu_8pg_ports, 29);

        /* all port except CMIC */
        SOC_PBMP_CLEAR(hu_non_cpu_ports);
        SOC_PBMP_ASSIGN(hu_non_cpu_ports, PBMP_ALL(unit));
        SOC_PBMP_PORT_REMOVE(hu_non_cpu_ports, 0);
 
        /* All ports */
        SOC_PBMP_CLEAR(hu_all_ports); 
        SOC_PBMP_ASSIGN(hu_all_ports, PBMP_ALL(unit)); 
 
        /* CPU port */
        SOC_PBMP_CLEAR(hu_cmic); 
        SOC_PBMP_ASSIGN(hu_cmic, PBMP_CMIC(unit));
 
        hu_port_init = 1; 
    }
    if (ainfo->port < 0) {
        return 0;
    }

    if (!SOC_REG_IS_VALID(unit, ainfo->reg)) {
        goto skip;
    }
 
    switch(ainfo->reg) {
        case MAC_CTRLr:
        case MAC_XGXS_CTRLr:
        case MAC_XGXS_STATr:
        case MAC_TXMUXCTRLr:
        case MAC_CNTMAXSZr:
        case MAC_CORESPARE0r:
        case MAC_TXCTRLr:
        case MAC_TXMACSAr:
        case MAC_TXMAXSZr:
        case MAC_TXPSETHRr:
        case MAC_TXSPARE0r:
        case MAC_TXPPPCTRLr:
        case ITPOKr:
        case ITXPFr: 
        case ITFCSr:
        case ITUCr:
        case ITUCAr:
        case ITMCAr:
        case ITBCAr:
        case ITOVRr:
        case ITFRGr:
        case ITPKTr:
        case IT64r:
        case IT127r:
        case IT255r:
        case IT511r:
        case IT1023r:
        case IT1518r:
        case IT2047r:
        case IT4095r:
        case IT9216r:
        case IT16383r:
        case ITMAXr:
        case ITUFLr:
        case ITERRr:
        case ITBYTr:
        case MAC_RXCTRLr:
        case MAC_RXMACSAr:
        case MAC_RXMAXSZr:
        case MAC_RXLSSCTRLr:
        case MAC_RXLSSSTATr:
        case MAC_RXSPARE0r:
        case IR64r:
        case IR127r:
        case IR255r:
        case IR511r:
        case IR1023r:
        case IR1518r:
        case IR2047r:
        case IR4095r:
        case IR9216r:
        case IR16383r:
        case IRMAXr:
        case IRPKTr:
        case IRFCSr:
        case IRUCAr:
        case IRMCAr:
        case IRBCAr:
        case IRXCFr:
        case IRXPFr:
        case IRXUOr:
        case IRJBRr:
        case IROVRr:
        case IRFLRr:
        case IRPOKr:
        case IRMEGr:
        case IRMEBr:
        case IRBYTr:
        case IRUNDr:
        case IRFRGr:
        case IRERBYTr:
        case IRERPKTr:
        case IRJUNKr:
        case ITXPPr:
        case IRXPPr:
        case MAC_TXLLFCCTRLr:
        case MAC_TXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGFLDSr:
        case MAC_RXLLFCMSGCNTr:
            if (IS_GE_PORT(unit, ainfo->port)) {
                goto skip;
            }
            break;
        default:
            break;
    }

    switch(ainfo->reg) {
        case IE2E_CONTROLr:
        case ING_MODMAP_CTRLr:
        case IHG_LOOKUPr:
        case ICONTROL_OPCODE_BITMAPr:
        case UNKNOWN_HGI_BITMAPr:
        case IUNHGIr:
        case ICTRLr:
        case IBCASTr:
        case ILTOMCr:
        case IIPMCr:
        case IUNKOPCr:
            pbmp = &hu_higig_ports;
            break;
        case HOL_STAT_PORTr:
            pbmp = &hu_non_cpu_ports;
            break;
        case ING_COS_MODEr:
            pbmp = &hu_24q_ports;
            break;
        default:
            pbmp = &hu_all_ports;
            break;
    }

    if (!SOC_PBMP_MEMBER(*pbmp, ainfo->port)) {
        goto skip; 
    }


    if (mask != NULL) {
        switch(ainfo->reg) {
            case HOL_STAT_PORTr:
                if (!SOC_PBMP_MEMBER(hu_24q_ports, ainfo->port)) {
                    /* adjust mask for ports without 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
            case TOQ_QUEUESTAT_64r:
            case TOQ_ACTIVATEQ_64r:
            case TOQEMPTY_64r:
            case DEQ_AGINGMASK_64r:
                if (SOC_PBMP_MEMBER(hu_24q_ports, ainfo->port)) {
                    /* adjust mask for ports with 24 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x00ffffff);
                    COMPILER_64_AND(*mask, temp_mask);
                } else if (ainfo->port != 0) {
                    /* remaining ports only have 8 queues */
                    COMPILER_64_SET(temp_mask, 0, 0x000000ff);
                    COMPILER_64_AND(*mask, temp_mask);
                }
                break;
			case MAC_MODEr:
                /* skip LINK_STATUS bit */
                COMPILER_64_SET(temp_mask, 0, 0x0000000f);
                COMPILER_64_AND(*mask, temp_mask);
                break;
        }
    }
    return 0; 
skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_HURRICANE_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
reg_mask_subset_trident(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    soc_info_t *si;
    uint64 temp_mask;

    if (!SOC_PORT_VALID(unit, ainfo->port)) {
        return 0;
    }

    si = &SOC_INFO(unit);

    switch (ainfo->reg) {
    case XMAC_CTRLr:
    case XMAC_MODEr:
    case XMAC_SPARE0r:
    case XMAC_SPARE1r:
    case XMAC_TX_CTRLr:
    case XMAC_TX_MAC_SAr:
    case XMAC_RX_CTRLr:
    case XMAC_RX_MAC_SAr:
    case XMAC_RX_MAX_SIZEr:
    case XMAC_RX_VLAN_TAGr:
    case XMAC_RX_LSS_CTRLr:
    case XMAC_RX_LSS_STATUSr:
    case XMAC_CLEAR_RX_LSS_STATUSr:
    case XMAC_PAUSE_CTRLr:
    case XMAC_PFC_CTRLr:
    case XMAC_PFC_TYPEr:
    case XMAC_PFC_OPCODEr:
    case XMAC_PFC_DAr:
    case XMAC_LLFC_CTRLr:
    case XMAC_TX_LLFC_MSG_FIELDSr:
    case XMAC_RX_LLFC_MSG_FIELDSr:
    case XMAC_HCFC_CTRLr:
    case XMAC_TX_TIMESTAMP_FIFO_DATAr:
    case XMAC_TX_TIMESTAMP_FIFO_STATUSr:
    case XMAC_FIFO_STATUSr:
    case XMAC_CLEAR_FIFO_STATUSr:
    case XMAC_TX_FIFO_CREDITSr:
    case XMAC_EEE_CTRLr:
    case XMAC_EEE_TIMERSr:
        /* Skip Xmac registers when the XLPORT is used as GE port */
        if (IS_GE_PORT(unit, ainfo->port)) {
            goto skip;
        }
        break;

    case DROP_PKT_CNT_OVQr:
        /* @OVQ_PORT_DROP_REGS (all ports except cpu port) */
    case DEQ_AGINGMASKr:
    case MCQ_FIFO_BASE_REGr:
    case MCQ_FIFO_EMPTY_REGr:
        /* PORTLIST => [1..65] */
    case COSMASKr:
    case MINSPCONFIGr:
        /* @MMU_NO_CPU_PORTS (all ports except cpu port) */
    case PORT_LLFC_CFGr:
        /* @PFC_PORT_CFG_REGS (all ports except cpu port) */
    case S3_MINBUCKETCONFIG_64r:
    case S3_MINBUCKETr:
    case S3_MAXBUCKETCONFIG_64r:
    case S3_MAXBUCKETr:
        /* @MMU_MTRO_S3_REG (all ports except cpu port) */
    case S2_MINBUCKETCONFIG_64r:
    case S2_MINBUCKETr:
    case S2_MAXBUCKETCONFIG_64r:
    case S2_MAXBUCKETr:
        /* @MMU_MTRO_S2_REG (all ports except cpu port) */
        if (IS_CPU_PORT(unit, ainfo->port)) {
            goto skip;
        }
        break;

    case XPORT_TO_MMU_BKPr:
        /* @PFC_PORT_STS_REGS (all ports except cpu and lb port) */
    case BKPMETERINGCONFIG_64r:
    case BKPMETERINGBUCKETr:
    case MTRI_IFGr:
        /* @MMU_MTRI_PORTS (all ports except cpu and lb port) */
    case OVQ_MCQ_CREDITSr:
    case OVQ_MCQ_STATEr:
        /* @OVQ_PORT_COS_REGS (all ports except cpu and lb port) */
    case OP_UC_PORT_CONFIG_CELLr:
    case OP_UC_PORT_CONFIG1_CELLr:
    case OP_UC_PORT_LIMIT_COLOR_CELLr:
    case OP_UC_PORT_SHARED_COUNT_CELLr:
    case OP_UC_PORT_LIMIT_RESUME_COLOR_CELLr:
    case UCQ_COS_EMPTY_REGr:
    case PORT_SP_WRED_CONFIGr:
    case PORT_SP_WRED_AVG_QSIZEr:
        /* PORTLIST => [1..32,34..65] */
    case OP_UC_QUEUE_MIN_COUNT_CELLr:
    case OP_UC_QUEUE_SHARED_COUNT_CELLr:
    case OP_UC_QUEUE_TOTAL_COUNT_CELLr:
    case OP_UC_QUEUE_RESET_VALUE_CELLr:
        /* @THDO_UC_PORT_COS_REGS (all ports except cpu and lb port) */
    case WRED_CONFIGr:
    case WRED_AVG_QSIZEr:
        /* @MMU_PERPORTPERCOS_NOCPU_REGS (all ports except cpu and lb port) */
        if (IS_CPU_PORT(unit, ainfo->port) || IS_LB_PORT(unit, ainfo->port)) {
            goto skip;
        }
        break;

    case OP_EX_QUEUE_MIN_COUNT_CELLr:
    case OP_EX_QUEUE_SHARED_COUNT_CELLr:
    case OP_EX_QUEUE_TOTAL_COUNT_CELLr:
    case OP_EX_QUEUE_RESET_VALUE_CELLr:
        /* @THDO_EX_PORT_COS_REGS (extended queue ports) */
    case OP_EX_PORT_CONFIG_SPID_0r:
    case OP_EX_PORT_CONFIG_SPID_1r:
    case OP_EX_PORT_CONFIG_SPID_2r:
    case OP_EX_PORT_CONFIG_SPID_3r:
    case OP_EX_PORT_CONFIG_SPID_4r:
    case OP_EX_PORT_CONFIG_COS_MIN_0r:
    case OP_EX_PORT_CONFIG_COS_MIN_1r:
    case OP_EX_PORT_CONFIG_COS_MIN_2r:
    case UCQ_EXTCOS1_EMPTY_REGr:
        /* PORTLIST => [1..4,34..37] */
    case DMVOQ_WRED_CONFIGr:
    case VOQ_WRED_AVG_QSIZEr:
        /* @MMU_PERPORTPERVOQ_NOCPU_REGS (extended queue ports) */
        if (!si->port_num_ext_cosq[ainfo->port]) {
            goto skip;
        }
        break;

    case COSWEIGHTSr:
    case WERRCOUNTr:
        /* @MMU_ES_REGS */
        if (!IS_CPU_PORT(unit, ainfo->port)) {
            if (ainfo->idx >= 7) {
                goto skip;
            }
        }
        break;

    case S3_COSWEIGHTSr:
    case S3_WERRCOUNTr:
        /* @MMU_ES_S3_REGS */
        if (!si->port_num_ext_cosq[ainfo->port]) {
            if (ainfo->idx >= 8) {
                goto skip;
            }
        }
        break;

    case MINBUCKETCONFIG_64r:
    case MINBUCKETr:
    case MAXBUCKETCONFIG_64r:
    case MAXBUCKETr:
        /* @MMU_MTRO_REGS */
        if (IS_CPU_PORT(unit, ainfo->port)) {
            if (ainfo->idx >= 48) {
                goto skip;
            }
        } else if (IS_LB_PORT(unit, ainfo->port)) {
            if (ainfo->idx >= 5) {
                goto skip;
            }
        } else if (!si->port_num_ext_cosq[ainfo->port]) {
            if (ainfo->idx >= 15) {
                goto skip;
            }
        }
        break;

    case OP_QUEUE_CONFIG_CELLr:
    case OP_QUEUE_CONFIG1_CELLr:
    case OP_QUEUE_LIMIT_COLOR_CELLr:
    case OP_QUEUE_RESET_OFFSET_CELLr:
    case OP_QUEUE_MIN_COUNT_CELLr:
    case OP_QUEUE_SHARED_COUNT_CELLr:
    case OP_QUEUE_TOTAL_COUNT_CELLr:
    case OP_QUEUE_RESET_VALUE_CELLr:
    case OP_QUEUE_LIMIT_RESUME_COLOR_CELLr:
        /* @THDO_PORT_COS_REGS */
        if (!IS_CPU_PORT(unit, ainfo->port)) {
            if (ainfo->idx >= 5) {
                goto skip;
            }
        }
        break;
    default:
        break;
    }

    if (mask != NULL) {
        switch (ainfo->reg) {
        case SHAPING_CONTROLr:
        case RESET_ON_EMPTY_MAX_64r:
            if (!si->port_num_ext_cosq[ainfo->port]) {
                COMPILER_64_SET(temp_mask, 0, 0x007fffff);
                COMPILER_64_AND(*mask, temp_mask);
            }
            break;
        default:
            break;
        }
    }

    return 0;
skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    return 1;
}
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
STATIC int
reg_mask_subset_ss(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
    switch (ainfo->reg) {
        /* if there are any field has name debug_rptr or debug_wptr
         * skip the register
         */
        case TS_PRI_SB_DEBUGr:
        case SB_DEBUG_QS_CIr:
        case SB_DEBUG_TX_CIr:
        case SB_DEBUG_RB_CIr:
        case QMA_QS_SB_DEBUGr:
        case QMA_RB_SB_DEBUGr:
        case QMB_ENQD_SB_DEBUGr:
        case QMB_DEQD_SB_DEBUGr:
        case QS_RATE_SB_DEBUGr:
        case QSA_ENQDEQD_SB_DEBUGr:
        case QM_QSB_RAND_SB_DEBUGr:
        case QM_QSB_RATE_SB_DEBUGr:
        case TS_QSB_RATE_SB_DEBUGr:
        case RB_ENQRESP_SB_DEBUGr:
        case CI_RB_RBTAG_SB_DEBUGr:
        case QM_TX_DEQREQ_SB_DEBUGr:
        case QS_TX_GRANT_SB_DEBUGr:
        case CI0_TX_SB_DEBUGr:
        case CI1_TX_SB_DEBUGr:
        case CI2_TX_SB_DEBUGr:
        case CI3_TX_SB_DEBUGr:
        case CI4_TX_SB_DEBUGr:
        case CI5_TX_SB_DEBUGr:
        case CI6_TX_SB_DEBUGr:
        case CI7_TX_SB_DEBUGr:
        case CI8_TX_SB_DEBUGr:
        case CI9_TX_SB_DEBUGr:

        case QMA_DEBUG1r:
        case RB_DEBUG_TEST_CONFIGr:
        case TS_DEBUG_INFOr:
	    /* reset value based on hw configration, not known at reset time, skipfor tr 1 test */ 
        case MAC_CTRLr:
	    /* required to be modified to allow MAC register read, skip for tr 1 test */
        case MAC_RXCTRLr:
        case MAC_TXCTRLr:
	    /* rw value limited by the hardware, some fields not really writable, skip for tr 3 test */
            goto skip;
        default:
  	    return 0;
    }

skip:
    /* set mask to all 0's so test will skip it */
    if (mask != NULL) {
        COMPILER_64_SET(*mask, 0, 0);
    }
    /* registers are implemented in hw, so return 0 here */
    return 0;
}
#endif /* BCM_SIRIUS_SUPPORT */

/*
 * Function: reg_mask_subset
 * 
 * Description: 
 *   Returns 1 if the register is not implemented in HW for the 
 *   specified port/cos. Returns 0 if the register is implemented
 *   in HW. If "mask" argument is supplied, the mask is modified
 *   to only include bits that are implemented in HW for the specified
 *   port/cos index.
 */
int
reg_mask_subset(int unit, soc_regaddrinfo_t *ainfo, uint64 *mask)
{
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
        SOC_IS_VALKYRIE2(unit)) {
        return (reg_mask_subset_tr2(unit, ainfo, mask));
    }
#endif
#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        return (reg_mask_subset_hu(unit, ainfo, mask));
    }
#endif
#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit)) {
        return (reg_mask_subset_en(unit, ainfo, mask));
    }
#endif
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return (reg_mask_subset_trident(unit, ainfo, mask));
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TR_VL(unit)) {
        return (reg_mask_subset_tr(unit, ainfo, mask));
    }
#endif
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        return (reg_mask_subset_sh(unit, ainfo, mask));
    }
#endif
#ifdef BCM_SCORPION_SUPPORT
    if (SOC_IS_SC_CQ(unit)) {
        return (reg_mask_subset_sc(unit, ainfo, mask));
    }
#endif
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        return (reg_mask_subset_ss(unit, ainfo, mask));
    }
#endif
    return 0;
}

/*
 * $Id: wc40_extra.h,v 1.2.2.3 Broadcom SDK $
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
 * File:        wc40_extra.h
 * Purpose:     non-register Macro, data structures and function prototypes for WC driver. 
 *             
 */

#ifndef WC40_EXTRA_H__
#define WC40_EXTRA_H__

/*
 * wc40.h has definitions related to WC registers. It is automatically generated 
 * from RDB files. This file has all non-register Macro, data structures and 
 * function prototypes the WC driver.
 */

/* additional missed register definitions */

#define WC40_XGXSBLK0_XGXSSTATUSr      0x8001
#define WC40_TX0_ANATXACONTROL0r       0x8061
#define WC40_RX0_ANARXCONTROLPCIr      0x80ba
#define WC40_COMBO_IEEE0_MIICNTLr      0xffe0
#define WC40_MICROBLK_DOWNLOAD_STATUSr 0xffc5
#define SERDESDIGITAL_FORCE_SPEED_dr_10G_XFI  0x25
#define SERDESDIGITAL_FORCE_SPEED_dr_10G_SFI  0x29
#define XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_KR2         0x39
#define XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_CR2         0x3a

/* misc. */

#define WC40_PLL_WAIT  250000
#define WC40_AN_WAIT   400000   /* 400ms */
#define TX_DRIVER_DFT_LN_CTRL   0x7fff

#define SERDES_ID0_MODEL_NUMBER_WARPCORE 0x9
#define WC_AER_BCST_OFS_STRAP   0x1ff
#define OVER1G_UP3_20GPLUS_MASK (DIGITAL3_UP3_DATARATE_25P45GX4_MASK | \
                                 DIGITAL3_UP3_DATARATE_21GX4_MASK | \
                                 DIGITAL3_UP3_DATARATE_31P5G_MASK | \
                                 DIGITAL3_UP3_DATARATE_40G_MASK | \
                                 DIGITAL3_UP3_LAST_MASK)
#define PLL_PLLACONTROL4_BIT12_MASK         0x1000
#define WC40_UCODE_MAX_LEN   (32*1024)
#define NUM_LANES             4

/* WC revision IDs */

#define WC40_SERDES_ID0_REVID_A0    0x0000
#define WC40_SERDES_ID0_REVID_A1    0x0800
#define WC40_SERDES_ID0_REVID_A2    0x1000
#define WC40_SERDES_ID0_REVID_B0    0x4000
#define WC40_SERDES_ID0_REVID_C0    0x8000
#define WC40_SERDES_ID0_REVID_D0    0xc000

#define REV_VER_MASK (SERDESID_SERDESID0_REV_LETTER_MASK | \
                         SERDESID_SERDESID0_REV_NUMBER_MASK)
#define WC40_REVID(_pc) \
     ((((WC40_DEV_DESC_t *)((_pc) + 1))->info.serdes_id0) & REV_VER_MASK)
#define WC40_REVID_A0(_pc) \
     (((((WC40_DEV_DESC_t *)((_pc) + 1))->info.serdes_id0) & REV_VER_MASK) == \
        WC40_SERDES_ID0_REVID_A0)
#define WC40_REVID_A1(_pc) \
     (((((WC40_DEV_DESC_t *)((_pc) + 1))->info.serdes_id0) & REV_VER_MASK) == \
        WC40_SERDES_ID0_REVID_A1)
#define WC40_REVID_B0(_pc) \
     (((((WC40_DEV_DESC_t *)((_pc) + 1))->info.serdes_id0) & REV_VER_MASK) == \
        WC40_SERDES_ID0_REVID_B0)
#define WC40_LINK_WAR_REVS(_pc) (WC40_REVID_A0(_pc) || WC40_REVID_A1(_pc))

/* div16 and div33 clocks */

#define CL72_MISC4_CTRL_DIV16_SHIFT       5
#define CL72_MISC4_CTRL_DIV33_SHIFT       3
#define CL72_MISC4_CTRL_DIV16_FIELD(_fd)  ((_fd) << CL72_MISC4_CTRL_DIV16_SHIFT)
#define CL72_MISC4_CTRL_DIV33_FIELD(_fd)  ((_fd) << CL72_MISC4_CTRL_DIV33_SHIFT)
#define CL72_MISC4_CTRL_DIV16_MASK        (0xf << CL72_MISC4_CTRL_DIV16_SHIFT)
#define CL72_MISC4_CTRL_DIV33_MASK        (0x3 << CL72_MISC4_CTRL_DIV33_SHIFT)

/* CL73 */

#define CL73_AN_ADV_TECH_20G_KR2   (1 << 13)
#define CL73_AN_ADV_TECH_100G_CR10 (1 << 10)
#define CL73_AN_ADV_TECH_40G_CR4 (1 << 9)
#define CL73_AN_ADV_TECH_40G_KR4 (1 << 8)
#define CL73_AN_ADV_TECH_10G_KR  (1 << 7)
#define CL73_AN_ADV_TECH_10G_KX4 (1 << 6)
#define CL73_AN_ADV_TECH_1G_KX   (1 << 5)
#define CL73_AN_ADV_TECH_SPEEDS_MASK  (0x3F << 5)
#define CL73_AN_ADV_PLUS_KR2_MASK  (CL73_AN_ADV_TECH_SPEEDS_MASK | \
                CL73_AN_ADV_TECH_20G_KR2)
#define CL73_AN_ADV_PAUSE                                (1 << 10)
#define CL73_AN_ADV_ASYM_PAUSE                           (1 << 11)

/* VCO frequency ctrl */

#define WC40_OS5_VCO_FREQ  0x7200
#define WC40_OS8_VCO_FERQ  0x7A00
#define WC40_TXCK_MASK     0x1E0
#define WC40_RXCK_OVERIDE_MASK  (1 << 11)
#define WC40_TXCK_SHIFTER  5

/* WC40_CL73_KR2 is the HP cl73 KR2 autoneg mode. In this mode
 * CL37,CL37BAM and CL73BAM is disabled. The autoneg is in software
 * controlled mode.
 */
#define WC40_CL73_KR2          3
#define WC40_CL73_WO_CL73BAM   2
#define WC40_CL73_AND_CL73BAM  1

/* lane swap functions in XGXSBLK2 are obsolete, use ones in XGXSBLK8
 * i.e. define WC40_PCS_LANE_SWAP
 */
#define WC40_PCS_LANE_SWAP
#ifdef WC40_PCS_LANE_SWAP
/* For Tx lane swap
 * bits 15-12 11-8 7-4 3-0  represents pma/pmd lane 3,2,1,0
 * value in each field represents corresponding pcs lane number.
 * the default map is pma/pmd lane3 maps to pcs lane3, pma lane2
 * to pcs lane2 and so on.
 *
 * For Rx lane swap
 * The same bit fields represents the pcs lane 3,2,1,0
 * and the value corresponds the pma/pmd lane number.
 */
#define WC40_RX_LANE_MAP_DEFAULT    0x3210
#define WC40_TX_LANE_MAP_DEFAULT    0x3210
#else
#define WC40_RX_LANE_MAP_DEFAULT    0x0123
#define WC40_TX_LANE_MAP_DEFAULT    0x0123
#endif

/* Register lane access. fixed value, must not change */

#define LANE0_ACCESS      1
#define LANE1_ACCESS      2
#define LANE2_ACCESS      3
#define LANE3_ACCESS      4
#define LANE_BCST         0xF
#define LANE_ACCESS_MASK  0xF

/* operation modes */

#define CUSTOM1_MODE(_pc) ((_pc)->phy_mode == PHYCTRL_LANE_MODE_CUSTOM1)
#define CUSTOM_MODE(_pc) (((_pc)->phy_mode == PHYCTRL_LANE_MODE_CUSTOM) || \
                                ((_pc)->phy_mode == PHYCTRL_LANE_MODE_CUSTOM_3p125MHZ))
#define CUSTOMX_MODE(_pc) (CUSTOM_MODE(_pc) || CUSTOM1_MODE(_pc))
#define IND_LANE_MODE(_mode) (((_mode)==xgxs_operationModes_Indlane_OS8) || \
         ((_mode)==xgxs_operationModes_IndLane_OS5))
#define COMBO_LANE_MODE(_mode) ((_mode) == xgxs_operationModes_ComboCoreMode)
#define IS_DUAL_LANE_PORT(_pc) (((_pc)->phy_mode == PHYCTRL_DUAL_LANE_PORT) || \
                                CUSTOM1_MODE(_pc))
#define DUAL_LANE_BCST_ENABLE(_pc)  \
        do { \
            if (IS_DUAL_LANE_PORT(_pc)) { \
                DEV_CFG_PTR(_pc)->dxgxs = (_pc)->lane_num? 2 :1; \
            } \
        } while (0)
#define DUAL_LANE_BCST_DISABLE(_pc)  \
        do { \
            if (IS_DUAL_LANE_PORT(_pc)) { \
                DEV_CFG_PTR(_pc)->dxgxs = 0; \
            } \
        } while (0)
#define AUTONEG_MODE_UNAVAIL(_pc) (CUSTOMX_MODE(_pc))

#define WC40_REG_READ(_unit, _pc, _flags, _reg_addr, _val) \
        phy_wc40_reg_aer_read((_unit), (_pc),(_flags), (_reg_addr), (_val))

#define WC40_REG_WRITE(_unit, _pc, _flags, _reg_addr, _val) \
        phy_wc40_reg_aer_write((_unit), (_pc),(_flags), (_reg_addr), (_val))

#define WC40_REG_MODIFY(_unit, _pc, _flags, _reg_addr, _val, _mask) \
        phy_wc40_reg_aer_modify((_unit), (_pc),(_flags), (_reg_addr), (_val),(_mask))

#define WC40_MEM_ALLOC(_size,_name) sal_alloc(_size,_name)
#define WC40_MEM_SET(_dst,_val,_len) sal_memset(_dst,_val,_len)
#define WC40_UDELAY(_time) sal_udelay(_time)
#define WC40_USLEEP(_time) sal_usleep(_time)

#define FX100_CONTROL1_AUTODET_EN_MASK 4
#define WC40_IF_SR     (1 << SOC_PORT_IF_SR)
#define WC40_IF_KR     (1 << SOC_PORT_IF_KR)
#define WC40_IF_KR4    (1 << SOC_PORT_IF_KR4)
#define WC40_IF_CR     (1 << SOC_PORT_IF_CR)
#define WC40_IF_CR4    (1 << SOC_PORT_IF_CR4)
#define WC40_IF_XFI    (1 << SOC_PORT_IF_XFI)
#define WC40_IF_SFI    (1 << SOC_PORT_IF_SFI)
#define WC40_IF_XLAUI  (1 << SOC_PORT_IF_XLAUI)
#define WC40_IF_XGMII  (1 << SOC_PORT_IF_XGMII)
#define WC40_40G_10G_INTF(_if)  ((_if) == SOC_PORT_IF_KR4 || (_if) == SOC_PORT_IF_XLAUI || \
                             (_if) == SOC_PORT_IF_CR4 || (_if) == SOC_PORT_IF_SR || \
                             (_if) == SOC_PORT_IF_KR || (_if) == SOC_PORT_IF_CR || \
                             (_if) == SOC_PORT_IF_XFI || (_if) == SOC_PORT_IF_SFI)

#define WC40_40G_10G_INTF_ALL   (WC40_IF_KR4 | WC40_IF_XLAUI | WC40_IF_CR4 | WC40_IF_SR | \
                        WC40_IF_KR | WC40_IF_CR | WC40_IF_XFI | WC40_IF_SFI)

#define WC40_UC_CTRL_XLAUI   0x3333
#define WC40_UC_CTRL_SR4     0x1111
#define WC40_UC_CTRL_SFP_DAC 0x2

#define WC40_AN_WAIT_INTERVAL 1000000  /* 1 second */

#define WC40_SP_TIME(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.start_time)
#define WC40_SP_VALID(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.state)
#define WC40_SP_CNT(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.cnt)

#define WC40_SP_RECORD_TIME(_pc)  \
        (DEV_CTRL_PTR(_pc)->lkwa.start_time) = sal_time_usecs()
#define WC40_SP_CNT_INC(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.cnt)++
#define WC40_SP_CNT_CLEAR(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.cnt) = 0
#define WC40_SP_VALID_SET(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.state) = TRUE
#define WC40_SP_VALID_RESET(_pc) \
        (DEV_CTRL_PTR(_pc)->lkwa.state) = FALSE

#define WC40_AN_TIME(_pc) \
        (DEV_CTRL_PTR(_pc)->an.start_time)
#define WC40_AN_VALID(_pc) \
        (DEV_CTRL_PTR(_pc)->an.state)
#define WC40_AN_RECORD_TIME(_pc)  \
        (DEV_CTRL_PTR(_pc)->an.start_time) = sal_time_usecs()
#define WC40_AN_VALID_SET(_pc) \
        (DEV_CTRL_PTR(_pc)->an.state) = TRUE
#define WC40_AN_VALID_RESET(_pc) \
        (DEV_CTRL_PTR(_pc)->an.state) = FALSE
#define WC40_AN_CHECK_TIMEOUT(_pc) \
        ((uint32)SAL_USECS_SUB(sal_time_usecs(), WC40_AN_TIME((_pc))) \
                >= WC40_AN_WAIT_INTERVAL)

/* TX drive configuration  */

#define WC40_NO_CFG_VALUE     (-1)
#define TXDRV_PREEMPH(_tdrv)  (((_tdrv).force << 15) | ((_tdrv).post << 10) | \
			((_tdrv).main << 4) | ((_tdrv).pre))

/* CL49 PRBS control */

#define WC40_PRBS_TYPE_STD   0
#define WC40_PRBS_TYPE_CL49  1
#define WC40_PRBS_CFG_POLY31 3
#define WC40_PRBS_CL49_POLY31  (PCS_IEEE2BLK_PCS_TPCONTROL_PRBS31TX_EN_MASK | \
                                PCS_IEEE2BLK_PCS_TPCONTROL_PRBS31RX_EN_MASK)
#define WC40_PRBS_CL49_POLY9   (PCS_IEEE2BLK_PCS_TPCONTROL_PRBS9TX_EN_MASK | \
                                0x80)

/* typedefs */

/* index to TX drive configuration table for each PMD type.
 */
typedef enum txdrv_inxs {
    TXDRV_XFI_INX,     /* 10G XFI */
    TXDRV_XLAUI_INX,   /* 40G XLAUI mode  */
    TXDRV_SFI_INX,     /* 10G SR fiber mode */
    TXDRV_SFIDAC_INX,  /* 10G SFI DAC mode */
    TXDRV_SR4_INX,     /* 40G SR4 */
    TXDRV_6GOS1_INX,   /* BRCM 40G mode */
    TXDRV_6GOS2_INX,   /*  */
    TXDRV_6GOS2_CX4_INX, /*  */
    TXDRV_AN_INX,      /* a common entry for autoneg mode */
    TXDRV_DFT_INX,     /* temp for any missing speed modes */
    TXDRV_LAST_INX     /* always last */
} TXDRV_INXS_t;

#define TXDRV_ENTRY_NUM     (TXDRV_LAST_INX)

typedef struct 
{
    unsigned int force: 1; /* forced */
    unsigned int post : 5; /* TX post tap */
    unsigned int main : 6; /* TX main tap */
    unsigned int pre  : 4; /* TX pre tap */
    unsigned int rsvd : 16; /* reserved */
} WC40_TX_TAP_t;


typedef struct {
    union {
        WC40_TX_TAP_t tap;
        uint16 preemph;
    } u;
    uint8 post2;   /* TX post2 coeff */
    uint8 idrive;  /* TX idrive  */ 
    uint8 ipredrive;  /* TX pre-idrive */
} WC40_TX_DRIVE_t;

typedef struct {
    int preemph[NUM_LANES];   /* pre-emphasis */
    int idriver[NUM_LANES];
    int pdriver[NUM_LANES];
    int auto_medium;
    int fiber_pref;
    int sgmii_mstr;
    int pdetect10g;   /* 10G paralell detect */
    int pdetect1000x; /* 1000X paralell detect */
    int cx42hg;
    int rxlane_map;
    int txlane_map;
    int cl73an;
    int lane_mode;
    int cx4_10g;
    int lane0_rst;
    int rxaui;
    int dxgxs;
    int custom;
    soc_port_if_t line_intf;
    int scrambler_en;
    int custom1;
    int txpol;
    int rxpol;
    int load_mthd;
    int uc_cksum;
    int medium;
    int hg_mode;  /* choose HG speed mode */
    int refclk; 
    WC40_TX_DRIVE_t tx_drive[TXDRV_ENTRY_NUM];
} WC40_DEV_CFG_t;

#define WC40_LANE_NAME_LEN   30

typedef struct {
    uint16 serdes_id0;
    char   name[WC40_LANE_NAME_LEN];
} WC40_DEV_INFO_t;

typedef struct {
    uint32 start_time;
    uint32 state;
    uint32 cnt;
} WC40_LINK_WAR;

typedef struct {
    uint32 start_time;
    uint32 state;
} WC40_AN_MODE;

typedef struct {
    int type;
    int poly;
} WC40_PRBS_CTRL;

typedef struct {
    WC40_LINK_WAR lkwa;
    WC40_AN_MODE an;
    WC40_PRBS_CTRL prbs;
} WC40_DEV_CTRL_t;

typedef struct {
    WC40_DEV_CFG_t   cfg;
    WC40_DEV_INFO_t  info;
    WC40_DEV_CTRL_t  ctrl;
} WC40_DEV_DESC_t;

#define DEV_CFG_PTR(_pc) (&(((WC40_DEV_DESC_t *)((_pc) + 1))->cfg))
#define DEV_INFO_PTR(_pc) (&(((WC40_DEV_DESC_t *)((_pc) + 1))->info))
#define DEV_CTRL_PTR(_pc) (&(((WC40_DEV_DESC_t *)((_pc) + 1))->ctrl))

typedef struct {
    uint8 *pdata;
    int   *plen;
    uint16 chip_rev;
} WC40_UCODE_DESC;

/* external functions shared within WC driver files  */
extern int phy_wc40_config_init(phy_ctrl_t *pc);


#endif /* WC40_EXTRA_H__ */

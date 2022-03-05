/*
 * $Id: wc40.c,v 1.84.4.33 Broadcom SDK $ 
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
 * File:        wc40.c
 * Purpose:     Broadcom 10M/100M/1G/2.5G/10G/12G/13G/16G/20G/25G/30G/40G SerDes 
 *              (Warpcore 40nm with x1 and x4 lane support)
 */

#include <sal/types.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>
#include <soc/port_ability.h>
#include <soc/phyctrl.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */

#if defined(INCLUDE_XGXS_WC40)
#include "phyconfig.h"     /* Must include before other phy related includes */
#include "phyreg.h"
#include "phyfege.h"
#include "phynull.h"
#include "serdesid.h"
#include "wc40.h"
#include "wc40_extra.h"

#define DIGITAL6_UD_FIELD_UD_ADV_ID_MASK   (3 << 9)
#define DIGITAL6_UD_FIELD_UD_ADV_ID_WC     (1 << 9)


#define WC40_SDK32387_REVS(_pc) (WC40_REVID_A0(_pc) || WC40_REVID_A1(_pc) || WC40_REVID_B0(_pc))
#define WC40_PHY400_REVS(_pc) (WC40_REVID_B0(_pc))

/* static variables */
STATIC char wc_device_name[] = "WarpCore";
int (*_phy_wc40_firmware_set_helper)(int, int, uint8 *,int) = NULL;
 
static int ln_access[4] = {LANE0_ACCESS, LANE1_ACCESS,LANE2_ACCESS,LANE3_ACCESS};

/* uController's firmware */
extern uint8 wc40_ucode_revA0[];
extern int   wc40_ucode_revA0_len;
extern uint8 wc40_ucode_revB0[];
extern int   wc40_ucode_revB0_len;

STATIC WC40_UCODE_DESC wc40_ucodes[] = {
    {&wc40_ucode_revA0[0], &wc40_ucode_revA0_len, WC40_SERDES_ID0_REVID_A0},
    {&wc40_ucode_revA0[0], &wc40_ucode_revA0_len, WC40_SERDES_ID0_REVID_A1},
    {&wc40_ucode_revA0[0], &wc40_ucode_revA0_len, WC40_SERDES_ID0_REVID_A2},
    {&wc40_ucode_revB0[0], &wc40_ucode_revB0_len, WC40_SERDES_ID0_REVID_B0}
};
#define WC40_UCODE_NUM_ENTRIES  (sizeof(wc40_ucodes)/sizeof(wc40_ucodes[0]))

/* function forward declaration */
STATIC int _phy_wc40_notify_duplex(int unit, soc_port_t port, uint32 duplex);
STATIC int _phy_wc40_notify_speed(int unit, soc_port_t port, uint32 speed);
STATIC int _phy_wc40_notify_stop(int unit, soc_port_t port, uint32 flags);
STATIC int _phy_wc40_notify_resume(int unit, soc_port_t port, uint32 flags);
STATIC int _phy_wc40_notify_interface(int unit, soc_port_t port, uint32 intf);
STATIC int _phy_wc40_combo_speed_get(int unit, soc_port_t port, int *speed,
                                     int *intf, int *scr);
STATIC int _phy_wc40_ind_speed_get(int unit, soc_port_t port, int *speed,
                                   int *intf, int *scr);
STATIC int phy_wc40_an_set(int unit, soc_port_t port, int an);
STATIC int phy_wc40_lb_set(int unit, soc_port_t port, int enable);
STATIC int phy_wc40_ability_advert_set(int unit, soc_port_t port,
                                      soc_port_ability_t *ability);
STATIC int phy_wc40_ability_local_get(int unit, soc_port_t port,
                                soc_port_ability_t *ability);
STATIC int phy_wc40_speed_set(int unit, soc_port_t port, int speed);
STATIC int phy_wc40_speed_get(int unit, soc_port_t port, int *speed);
STATIC int phy_wc40_an_get(int unit, soc_port_t port, int *an, int *an_done);
STATIC int _wc40_xgmii_scw_config (int unit, phy_ctrl_t *pc);
STATIC int _wc40_rxaui_config(int unit, phy_ctrl_t  *pc,int rxaui);
STATIC int _wc40_soft_reset(int unit, phy_ctrl_t  *pc);
STATIC int phy_wc40_reg_aer_read(int unit, phy_ctrl_t *pc, uint32 flags,
                  uint32 phy_reg_addr, uint16 *phy_data);
STATIC int phy_wc40_reg_aer_write(int unit, phy_ctrl_t *pc,  uint32 flags,
                   uint32 phy_reg_addr, uint16 phy_data);
STATIC int phy_wc40_reg_aer_modify(int unit, phy_ctrl_t *pc,  uint32 flags,
                    uint32 phy_reg_addr, uint16 phy_data,uint16 phy_data_mask);
STATIC int _wc40_chip_init_done(int unit,int chip_num,int phy_mode);
STATIC int phy_wc40_firmware_load(int unit, int port, int offset, 
                                 uint8 *array,int datalen);
STATIC int _phy_wc40_ind_speed_ctrl_get(int unit, soc_port_t port, int speed,
                                        uint16 *speed_val, int *tx_inx);
STATIC int _phy_wc40_vco_set(int unit, soc_port_t port, int speed,int speed_val);
STATIC int _wc40_soft_an_cl73kr2(int unit, soc_port_t port, int an);
STATIC int _wc40_soft_an_cl73kr2_check(int unit, soc_port_t port);

STATIC int
_phy_wc40_ucode_get(int unit, soc_port_t port, uint8 **ppdata, int *len, 
                    int *mem_alloced)
{
    int ix;
    phy_ctrl_t     *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    *mem_alloced = FALSE;
    *ppdata = NULL;

    /* check which firmware to load */
    for (ix = 0; ix < WC40_UCODE_NUM_ENTRIES; ix++) {
        if (wc40_ucodes[ix].chip_rev == WC40_REVID(pc)) {
            break;
        }
    }
    if (ix >= WC40_UCODE_NUM_ENTRIES) {
        SOC_DEBUG_PRINT((DK_WARN,
            "no firmware matches the chip rev number!!! use default\n"));
        ix = WC40_UCODE_NUM_ENTRIES - 1;
    }  
    for (; ix >= 0; ix--) {
        if ((wc40_ucodes[ix].pdata != NULL) && 
            (*(wc40_ucodes[ix].plen) != 0)) {
            break;
        }
    }
    if (ix < 0) {
        SOC_DEBUG_PRINT((DK_WARN, "no valid firmware found!!!\n"));
        return SOC_E_NOT_FOUND;
    }
    
    *ppdata = wc40_ucodes[ix].pdata;
    *len = *(wc40_ucodes[ix].plen);
    return SOC_E_NONE;
}


STATIC int
_phy_wc40_config_init(int unit, soc_port_t port)
{
    WC40_DEV_CFG_t *pCfg;
    WC40_DEV_INFO_t  *pInfo;
    phy_ctrl_t      *pc;
    WC40_DEV_DESC_t *pDesc;
    uint16 serdes_id0;
    WC40_TX_DRIVE_t *p_tx;
    int i;

    pc = INT_PHY_SW_STATE(unit, port);

    pDesc = (WC40_DEV_DESC_t *)(pc + 1);

    /* clear the memory */
    WC40_MEM_SET(pDesc, 0, sizeof(*pDesc));

    pCfg = &pDesc->cfg; 
    pInfo = &pDesc->info;

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESID_SERDESID0r(unit, pc, 0x00, &serdes_id0));
    pInfo->serdes_id0 = serdes_id0;

    /* init the default configuration values */

    for (i = 0; i < NUM_LANES; i++) {
        pCfg->preemph[i] = WC40_NO_CFG_VALUE;
        pCfg->idriver[i] = WC40_NO_CFG_VALUE;
        pCfg->pdriver[i] = WC40_NO_CFG_VALUE;
    }

   /* default */
    p_tx = &pCfg->tx_drive[TXDRV_DFT_INX];
    p_tx->u.tap.post  = 0x00;
    p_tx->u.tap.main  = 0x00;
    p_tx->u.tap.pre   = 0x00;
    p_tx->u.tap.force = 0x00;
    p_tx->post2     = 0x00;
    p_tx->idrive    = 0x09;
    p_tx->ipredrive = 0x09;

    pCfg->hg_mode    = FALSE;
    pCfg->sgmii_mstr = FALSE;
    pCfg->pdetect10g = TRUE;
    pCfg->cx42hg     = FALSE;
    pCfg->rxlane_map = WC40_RX_LANE_MAP_DEFAULT;
    pCfg->txlane_map = WC40_RX_LANE_MAP_DEFAULT;
    pCfg->medium = SOC_PORT_MEDIUM_COPPER;
    pCfg->cx4_10g    = TRUE;
    pCfg->lane0_rst  = TRUE;  
    pCfg->rxaui      = FALSE;
    pCfg->load_mthd = 2;             
    pCfg->uc_cksum  = 0;
    pCfg->refclk    = 156;

    /* this property is for debug and diagnostic purpose. byte0: 
     * 0: not loading WC firmware
     * 1: load from MDIO. default method.
     * 2: load from parallel bus if applicable. Provide fast downloading time
     * 
     * byte1:
     * 0: inform uC not to perform checksum calculation(default). Save ~70ms for WC init time
     * 1: inform uC to perform checksum calculation. 
     */
    pCfg->load_mthd = soc_property_port_get(unit, port, "load_firmware", pCfg->load_mthd);
    pCfg->load_mthd &= 0xff;
    pCfg->uc_cksum = soc_property_port_get(unit, port, "load_firmware", pCfg->uc_cksum);
    pCfg->uc_cksum = (pCfg->uc_cksum >> 8) & 0xff;

    /* before performing device configuration, must not call any driver function. 
     * Only allowed operation is to read common register shared by all four lanes
     */
    SOC_IF_ERROR_RETURN(phy_wc40_config_init(pc));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        int intf;
        int scr;
        int speed;
        int rv;
        uint16 data16;

        intf = 0;
        /* recovery software states from device */
        /* recover interface value from hardware */
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            rv = _phy_wc40_combo_speed_get(unit, port, &speed,&intf,&scr);
        } else {
            rv = _phy_wc40_ind_speed_get(unit, port, &speed,&intf,&scr);
        }
        pCfg->line_intf =  1 << intf;
        pCfg->scrambler_en = scr;

        SOC_IF_ERROR_RETURN
            (READ_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit,pc,LANE0_ACCESS,&data16));
        if (((data16 >> (pc->lane_num * 4)) & 0xf) == WC40_UC_CTRL_SFP_DAC) {
            pCfg->medium = SOC_PORT_MEDIUM_COPPER;
        } else {
            pCfg->medium = SOC_PORT_MEDIUM_FIBER;
        }
    } 
#endif 
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_regbit_set_wait_check(phy_ctrl_t *pc,
                  int reg,   /* register to check */
                  int bit_num,   /* bit number to check */
                  int bitset, /* check bit set or bit clear */
                  int timeout,  /* max wait time to check */
                  int lane_ctrl) /*  point to the specified lane  */
{
    uint16           data16;
    soc_timeout_t    to;
    int              rv;
    int  cond;

    soc_timeout_init(&to, timeout, 0);
    do {
        rv = WC40_REG_READ(pc->unit, pc, lane_ctrl, reg,&data16);
        cond = (bitset && (data16 & bit_num)) || 
               (!bitset && !(data16 & bit_num));
        if (cond || SOC_FAILURE(rv)) {
            break;
        }
    } while (!soc_timeout_check(&to));
    return (cond? SOC_E_NONE: SOC_E_TIMEOUT);
}

STATIC int
_phy_wc40_pll_lock_wait(int unit, soc_port_t port)
{
    phy_ctrl_t      *pc;
    int              rv;

    pc = INT_PHY_SW_STATE(unit, port);
    rv = _phy_wc40_regbit_set_wait_check(pc,WC40_XGXSBLK0_XGXSSTATUSr,
                     XGXSBLK0_XGXSSTATUS_TXPLL_LOCK_MASK,1,WC40_PLL_WAIT,0);
 
    if (rv == SOC_E_TIMEOUT) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "WC40 : TXPLL did not lock: u=%d p=%d\n",
                        unit, port));
        return (SOC_E_TIMEOUT);
    }
    return (SOC_E_NONE);
}

/*
 * Select the analog Rx div/16 and div/33 clocks for digital lanes 
 * this function must called after RX lane swap is done 
 */
STATIC int
_phy_wc40_rx_div16_clk_select(int unit, phy_ctrl_t *pc)
{
    uint16 rxlane_map;
    uint16 data16;
    uint16 mask16;
    uint16 div16_mode;
    uint16 div33_mode;

    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
    
        /* check the pcs lane remapping register */
        SOC_IF_ERROR_RETURN     
            (READ_WC40_XGXSBLK8_RXLNSWAP1r(unit, pc, LANE0_ACCESS, &rxlane_map));

        SOC_IF_ERROR_RETURN     
            (MODIFY_WC40_XGXSBLK2_TESTMODEMUXr(unit, pc, LANE0_ACCESS,
                   (rxlane_map<<8),0xFF00));

        mask16 = 0x80;
        data16 = 0;
        /* enable override in single lane port */
        if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            if (!IS_DUAL_LANE_PORT(pc)) {
                data16 = mask16;
            }
        }
        SOC_IF_ERROR_RETURN     
            (MODIFY_WC40_XGXSBLK2_TESTMODEMUXr(unit, pc, LANE0_ACCESS, data16,mask16));

    } else { /* B0 and above */
        /* 
         * Table 20 Rev B0 Lane Swap Programming Settings
         * Mode	      0x8108.7	0x82EC[8:5]	0x82EC[4:3]
         * Single     0x0	0x1	        0x1
         * Dual	      0x0	0x2	        0x2
         * Quad	      0x0	0x3	        0x3
         */
        /* not to force the override */
        SOC_IF_ERROR_RETURN     
            (MODIFY_WC40_XGXSBLK2_TESTMODEMUXr(unit, pc, LANE0_ACCESS, 0,0x80));

        /* configure for single port mode */
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            div16_mode = 1;
            div33_mode = 1;

        } else { /* quad-port or dual-port modes */

            /* dual-port mode */
            if (IS_DUAL_LANE_PORT(pc)) {
                div16_mode = 2;
                div33_mode = 2;
            } else {
            /* quad-port mode */
                div16_mode = 3;
                div33_mode = 3;
            }
        }
        SOC_IF_ERROR_RETURN     
            (MODIFY_WC40_CL72_USERB0_CL72_MISC4_CONTROLr(unit, pc, LANE0_ACCESS, 
              CL72_MISC4_CTRL_DIV16_FIELD(div16_mode) |
              CL72_MISC4_CTRL_DIV33_FIELD(div33_mode),
              CL72_MISC4_CTRL_DIV16_MASK |
              CL72_MISC4_CTRL_DIV33_MASK));
    } 
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_rxlane_map_set(int unit, phy_ctrl_t *pc, uint16 req_map)
{
    uint16 i;
    uint16 lane, hw_map, chk_map;

    /* Update RX lane map */

    if (req_map != WC40_RX_LANE_MAP_DEFAULT) {
        hw_map  = 0;
        chk_map = 0;
        for (i = 0; i < 4; i++) {
            lane     = (req_map >> (i * 4)) & 0xf;
            hw_map  |= lane << (i * 2);
            chk_map |= 1 << lane;
        }
        if (chk_map == 0xf) {
#ifdef WC40_PCS_LANE_SWAP
            SOC_IF_ERROR_RETURN     /* Enable RX Lane swap */
               (MODIFY_WC40_XGXSBLK8_RXLNSWAP1r(unit, pc, LANE0_ACCESS,
                   hw_map,
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_EN_MASK |
                   XGXSBLK8_RXLNSWAP1_RX0_LNSWAP_SEL_MASK |
                   XGXSBLK8_RXLNSWAP1_RX1_LNSWAP_SEL_MASK |
                   XGXSBLK8_RXLNSWAP1_RX2_LNSWAP_SEL_MASK |
                   XGXSBLK8_RXLNSWAP1_RX3_LNSWAP_SEL_MASK));
#else
            SOC_IF_ERROR_RETURN     /* Enable RX Lane swap */
               (MODIFY_WC40_XGXSBLK2_RXLNSWAPr(unit, pc,
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_EN_MASK |
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE_EN_MASK | hw_map,
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE_EN_MASK | 
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_EN_MASK |
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE0_MASK |
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE1_MASK |
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE2_MASK |
                   XGXSBLK2_RXLNSWAP_RX_LNSWAP_FORCE3_MASK));
#endif
        } else {
            SOC_DEBUG_PRINT((DK_ERR,
                 "unit %d port %s: Invalid RX lane map 0x%04x.\n",
                 unit, SOC_PORT_NAME(unit, pc->port), req_map));
        }
    }
    return SOC_E_NONE;
}
STATIC int
_phy_wc40_txlane_map_set(int unit, phy_ctrl_t *pc, uint16 req_map)
{
    uint16 i;
    uint16 lane, hw_map, chk_map;

    /* Update TX lane map */

    if (req_map != WC40_TX_LANE_MAP_DEFAULT) {
        hw_map  = 0;
        chk_map = 0;
        for (i = 0; i < 4; i++) {
            lane     = (req_map >> (i * 4)) & 0xf;
            hw_map  |= lane << (i * 2);
            chk_map |= 1 << lane;
        }
        if (chk_map == 0xf) {
#ifdef WC40_PCS_LANE_SWAP
            SOC_IF_ERROR_RETURN     /* Enable TX Lane swap */
               (MODIFY_WC40_XGXSBLK8_TXLNSWAP1r(unit, pc, LANE0_ACCESS,
                    hw_map,
                    XGXSBLK8_TXLNSWAP1_TX3_LNSWAP_SEL_MASK |
                    XGXSBLK8_TXLNSWAP1_TX2_LNSWAP_SEL_MASK |
                    XGXSBLK8_TXLNSWAP1_TX1_LNSWAP_SEL_MASK |
                    XGXSBLK8_TXLNSWAP1_TX0_LNSWAP_SEL_MASK));
#else
            SOC_IF_ERROR_RETURN     /* Enable TX Lane swap */
               (MODIFY_WC40_XGXSBLK2_TXLNSWAPr(unit, pc, 0x00, 
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_EN_MASK | hw_map,
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_EN_MASK |
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_FORCE0_MASK |
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_FORCE1_MASK |
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_FORCE2_MASK |
                    XGXSBLK2_TXLNSWAP_TX_LNSWAP_FORCE3_MASK));
#endif
        } else {
            SOC_DEBUG_PRINT((DK_ERR,
                 "unit %d port %s: Invalid TX lane map 0x%04x.\n",
                 unit, SOC_PORT_NAME(unit, pc->port), req_map));
        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_ind_lane_polarity_set (int unit, phy_ctrl_t *pc, int enable)
{
    uint16 data16;
    uint16 mask16;
    int cur_lane = FALSE;
    int nxt_lane = FALSE;

    if (DEV_CFG_PTR(pc)->txpol) {
        /* Flip TX polarity */
        data16 = enable? TX0_ANATXACONTROL0_TXPOL_FLIP_MASK: 0;
        mask16 = TX0_ANATXACONTROL0_TXPOL_FLIP_MASK;

        if (IS_DUAL_LANE_PORT(pc)) {
            if (DEV_CFG_PTR(pc)->txpol == 1) {
                cur_lane = TRUE;
                nxt_lane = TRUE;
            } else {
                if ((DEV_CFG_PTR(pc)->txpol & 0x000F) == 0x000F) {
                    cur_lane = TRUE;
                } 
                if ((DEV_CFG_PTR(pc)->txpol & 0x00F0) == 0x00F0) {
                    nxt_lane = TRUE;
                }
            }
        } else {
            cur_lane = TRUE;
        }
        
        if (cur_lane) {    
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,
                    WC40_TX0_ANATXACONTROL0r + (0x10 * pc->lane_num), 
                     data16, mask16));
        }
        if (nxt_lane) {    
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,
                    WC40_TX0_ANATXACONTROL0r + (0x10 * (pc->lane_num + 1)), 
                     data16, mask16));
        }
    }

    cur_lane = FALSE;
    nxt_lane = FALSE;
    if (DEV_CFG_PTR(pc)->rxpol) {
        /* Flip Rx polarity */
        mask16 = RX0_ANARXCONTROLPCI_RX_POLARITY_FORCE_SM_MASK |
                 RX0_ANARXCONTROLPCI_RX_POLARITY_R_MASK;
        data16 = enable? mask16: 0;

        if (IS_DUAL_LANE_PORT(pc)) {
            if (DEV_CFG_PTR(pc)->rxpol == 1) {
                cur_lane = TRUE;
                nxt_lane = TRUE;
            } else {
                if ((DEV_CFG_PTR(pc)->rxpol & 0x000F) == 0x000F) {
                    cur_lane = TRUE;
                }
                if ((DEV_CFG_PTR(pc)->rxpol & 0x00F0) == 0x00F0) {
                    nxt_lane = TRUE;
                }
            }
        } else {
            cur_lane = TRUE;
        }

        if (cur_lane) {
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,
                    WC40_RX0_ANARXCONTROLPCIr + (0x10 * pc->lane_num),
                     data16, mask16));
        }
        if (nxt_lane) {
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,
                    WC40_RX0_ANARXCONTROLPCIr + (0x10 * (pc->lane_num + 1)),
                     data16, mask16));
        }
         
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_combo_polarity_set (int unit, phy_ctrl_t *pc, int enable)
{
    uint16 data16;
    uint16 mask16;
    if (DEV_CFG_PTR(pc)->txpol) {
        /* Flip TX polarity */
        data16 = enable? TX0_ANATXACONTROL0_TXPOL_FLIP_MASK:0;
        mask16 = TX0_ANATXACONTROL0_TXPOL_FLIP_MASK;
        if (DEV_CFG_PTR(pc)->txpol == 1) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TXB_ANATXACONTROL0r(unit, pc, 0x00, data16, mask16));
        }
        if ((DEV_CFG_PTR(pc)->txpol & 0x000F) == 0x000F) {
            SOC_IF_ERROR_RETURN
              (MODIFY_WC40_TX0_ANATXACONTROL0r(unit, pc, 0x00, data16, mask16));
        }
        if ((DEV_CFG_PTR(pc)->txpol & 0x00F0) == 0x00F0) {
            SOC_IF_ERROR_RETURN
              (MODIFY_WC40_TX1_ANATXACONTROL0r(unit, pc, 0x00, data16, mask16));
        }
        if ((DEV_CFG_PTR(pc)->txpol & 0x0F00) == 0x0F00) {
            SOC_IF_ERROR_RETURN
              (MODIFY_WC40_TX2_ANATXACONTROL0r(unit, pc, 0x00, data16, mask16));
        }
        if ((DEV_CFG_PTR(pc)->txpol & 0xF000) == 0xF000) {
            SOC_IF_ERROR_RETURN
              (MODIFY_WC40_TX3_ANATXACONTROL0r(unit, pc, 0x00, data16, mask16));
        }
    }

    if (DEV_CFG_PTR(pc)->rxpol) {
        /* Flip Rx polarity */
        mask16 = RX0_ANARXCONTROLPCI_RX_POLARITY_FORCE_SM_MASK |
                 RX0_ANARXCONTROLPCI_RX_POLARITY_R_MASK;
        data16 = enable? mask16:0;
        if (DEV_CFG_PTR(pc)->rxpol == 1) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RXB_ANARXCONTROLPCIr(unit,pc,0x00,data16,mask16));
        } 
        if ((DEV_CFG_PTR(pc)->rxpol & 0x000F) == 0x000F) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RX0_ANARXCONTROLPCIr(unit,pc,0x00,data16,mask16));
        } 
        if ((DEV_CFG_PTR(pc)->rxpol & 0x00F0) == 0x00F0) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RX1_ANARXCONTROLPCIr(unit,pc,0x00,data16,mask16));
        } 
        if ((DEV_CFG_PTR(pc)->rxpol & 0x0F00) == 0x0F00) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RX2_ANARXCONTROLPCIr(unit,pc,0x00,data16,mask16));
        } 
        if ((DEV_CFG_PTR(pc)->rxpol & 0xF000) == 0xF000) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RX3_ANARXCONTROLPCIr(unit,pc,0x00,data16,mask16));
        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_cl72_enable(int unit, soc_port_t port, int en)
{
    phy_ctrl_t     *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        DUAL_LANE_BCST_ENABLE(pc);
        /* disable KR PMD start-up protocol */
        SOC_IF_ERROR_RETURN
          (MODIFY_WC40_PMD_IEEE9BLK_TENGBASE_KR_PMD_CONTROL_REGISTER_150r(unit,pc,0x00,
             en? PMD_IEEE9BLK_TENGBASE_KR_PMD_CONTROL_REGISTER_150_TRAINING_ENABLE_MASK: 0,
             PMD_IEEE9BLK_TENGBASE_KR_PMD_CONTROL_REGISTER_150_TRAINING_ENABLE_MASK));

        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_MISC1_CONTROLr(unit, pc, 0x00,
               en? 0:CL72_USERB0_CL72_MISC1_CONTROL_LINK_CONTROL_FORCE_MASK,
               CL72_USERB0_CL72_MISC1_CONTROL_LINK_CONTROL_FORCE_MASK));
        DUAL_LANE_BCST_DISABLE(pc);

    } else {  /* combo mode */
        /* bcst to PMD */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AERBLK_AERr(unit,pc,0x00,
                     (AERBLK_AER_MMD_DEVICETYPE_PMA_PMD <<
                     AERBLK_AER_MMD_DEVICETYPE_SHIFT) |
                     WC_AER_BCST_OFS_STRAP));

        SOC_IF_ERROR_RETURN
          (WC40_REG_MODIFY(unit, pc, 0x00,  0x0096,
             en? PMD_IEEE9BLK_TENGBASE_KR_PMD_CONTROL_REGISTER_150_TRAINING_ENABLE_MASK: 0,
             PMD_IEEE9BLK_TENGBASE_KR_PMD_CONTROL_REGISTER_150_TRAINING_ENABLE_MASK));

        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_MISC1_CONTROLr(unit, pc, 0x00,
               en? 0:CL72_USERB0_CL72_MISC1_CONTROL_LINK_CONTROL_FORCE_MASK,
               CL72_USERB0_CL72_MISC1_CONTROL_LINK_CONTROL_FORCE_MASK));

        /* restore AER */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AERBLK_AERr(unit,pc,0x00,0));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_tx_control_get(int unit, soc_port_t port, WC40_TX_DRIVE_t *tx_drv, int inx)
{
    phy_ctrl_t     *pc;
    int rv = SOC_E_NONE;
    int size;
    int i;

    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        size = 4;
    } else if (IS_DUAL_LANE_PORT(pc)) {
        size = 2;
    } else {
        size = 1;
    }

    for (i = 0; i < size; i++) {
        if (DEV_CFG_PTR(pc)->idriver[i] == WC40_NO_CFG_VALUE) {
            /* take from the default value table */
            (tx_drv + i)->idrive    = DEV_CFG_PTR(pc)->tx_drive[inx].idrive;
        } else {
            /* taking from user */
            (tx_drv + i)->idrive = DEV_CFG_PTR(pc)->idriver[i];
        }
        if (DEV_CFG_PTR(pc)->pdriver[i] == WC40_NO_CFG_VALUE) {
            /* take from the default value table */
            (tx_drv + i)->ipredrive = DEV_CFG_PTR(pc)->tx_drive[inx].ipredrive;
        } else {
            /* taking from user */
            (tx_drv + i)->ipredrive = DEV_CFG_PTR(pc)->pdriver[i];
        }

        /* in autoneg mode no forced preemphasis */
        if ((DEV_CFG_PTR(pc)->preemph[i] == WC40_NO_CFG_VALUE) ||
            ((inx == TXDRV_AN_INX)) ) {
            /* take from the default value table */
            (tx_drv + i)->u.preemph = TXDRV_PREEMPH(DEV_CFG_PTR(pc)->tx_drive[inx].u.tap);
        } else {
            /* taking from user */
            (tx_drv + i)->u.preemph = DEV_CFG_PTR(pc)->preemph[i];
        }
        (tx_drv + i)->post2 = DEV_CFG_PTR(pc)->tx_drive[inx].post2;
    }

    return rv;
}

STATIC int
_phy_wc40_tx_control_set(int unit, soc_port_t port, WC40_TX_DRIVE_t *tx_drv)
{
    phy_ctrl_t     *pc;
    uint32          preemph;
    uint32          idriver;
    uint32          pdriver;
    uint32          post2;
    uint16          data16;
    int lane_num;
    int i;
    int size;
 
    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        size = 4;
    } else if (IS_DUAL_LANE_PORT(pc)) {
        size = 2;
    } else {
        size = 1;
    }

    for (i = 0; i < size; i++) {
        idriver = (tx_drv + i)->idrive;
        pdriver = (tx_drv + i)->ipredrive;
        post2   = (tx_drv + i)->post2;
        preemph = (tx_drv + i)->u.preemph; 
        lane_num = pc->lane_num + i;
        /* WC40 SerDes
         *  idriver[11:8], pdriver[7:4],post2[14:12] 
         */
        idriver = (idriver << TXB_TX_DRIVER_IDRIVER_SHIFT) & 
              TXB_TX_DRIVER_IDRIVER_MASK;
        pdriver = (pdriver << TXB_TX_DRIVER_IPREDRIVER_SHIFT) &
              TXB_TX_DRIVER_IPREDRIVER_MASK;
        post2   = (post2 << TXB_TX_DRIVER_POST2_COEFF_SHIFT) &
              TXB_TX_DRIVER_POST2_COEFF_MASK;
        data16  = (uint16)(idriver | pdriver | post2);

        if (lane_num == 0) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX0_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
            if (IS_DUAL_LANE_PORT(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX1_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
            }
        } else if (lane_num == 1) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX1_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
        } else if (lane_num == 2) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX2_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
            if (IS_DUAL_LANE_PORT(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX3_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
            }
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX3_TX_DRIVERr(unit, pc, 0x00, data16,
                              TXB_TX_DRIVER_IDRIVER_MASK |
                              TXB_TX_DRIVER_IPREDRIVER_MASK |
                              TXB_TX_DRIVER_POST2_COEFF_MASK));
        }

        /* Warpcore preemphasis fields have been taken over by IEEE CL72.
         * It is automatically handled by H/W by default. But application can override 
         * these values by writing to 0x82e2. The value preemph should has the same
         * format as the register 0x82e2.
         */ 

        SOC_IF_ERROR_RETURN
            (WRITE_WC40_CL72_USERB0_CL72_TX_FIR_TAP_REGISTERr(unit, pc, ln_access[lane_num],
                  preemph));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_tx_fir_default_set(int unit, soc_port_t port,int lane)
{
    uint16              data16, mask16;
    phy_ctrl_t         *pc;
    pc                       = INT_PHY_SW_STATE(unit, port);

    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        SOC_IF_ERROR_RETURN
             (MODIFY_WC40_CL72_USERB0_CL72_MISC1_CONTROLr(unit, pc, ln_access[lane],
                63 << CL72_USERB0_CL72_MISC1_CONTROL_TX_FIR_TAP_MAIN_KX_INIT_VAL_SHIFT,
               CL72_USERB0_CL72_MISC1_CONTROL_TX_FIR_TAP_MAIN_KX_INIT_VAL_MASK));

        mask16 = CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_POST_KR_INIT_VAL_MASK |
                 CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_KR_INIT_VAL_MASK |
                 CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_PRE_KR_INIT_VAL_MASK;
        data16 = (22 << CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_POST_KR_INIT_VAL_SHIFT) |
                 (37 << CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_KR_INIT_VAL_SHIFT) |
                 (4 << CL72_USERB0_CL72_KR_DEFAULT_CONTROL_TX_FIR_TAP_PRE_KR_INIT_VAL_SHIFT);
    
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_KR_DEFAULT_CONTROLr(unit, pc, ln_access[lane], 
                            data16,mask16));
    
        mask16 = CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_POST_BR_INIT_VAL_MASK;
        data16 = 22 << CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_POST_BR_INIT_VAL_SHIFT;
        mask16 += CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_BR_INIT_VAL_MASK;
        data16 += 37 << CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_BR_INIT_VAL_SHIFT;
        mask16 += CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_PRE_BR_INIT_VAL_MASK;
        data16 += 4 << CL72_USERB0_CL72_BR_DEFAULT_CONTROL_TX_FIR_TAP_PRE_BR_INIT_VAL_SHIFT;
    
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_BR_DEFAULT_CONTROLr(unit, pc, ln_access[lane],
                    data16,mask16));
    
        mask16 = CL72_USERB0_CL72_OS_DEFAULT_CONTROL_TX_FIR_TAP_POST_OS_INIT_VAL_MASK;
        data16 = 0;
        mask16 += CL72_USERB0_CL72_OS_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_OS_INIT_VAL_MASK;
        data16 += 63 << CL72_USERB0_CL72_OS_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_OS_INIT_VAL_SHIFT;
        mask16 += CL72_USERB0_CL72_OS_DEFAULT_CONTROL_TX_FIR_TAP_PRE_OS_INIT_VAL_MASK;
        data16 += 0;
    
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_OS_DEFAULT_CONTROLr(unit, pc, ln_access[lane], 
                data16,mask16));
    
        mask16 = CL72_USERB0_CL72_2P5_DEFAULT_CONTROL_TX_FIR_TAP_POST_2P5_INIT_VAL_MASK;
        data16 = 0;
        mask16 += CL72_USERB0_CL72_2P5_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_2P5_INIT_VAL_MASK;
        data16 += 63 << CL72_USERB0_CL72_2P5_DEFAULT_CONTROL_TX_FIR_TAP_MAIN_2P5_INIT_VAL_SHIFT;
        mask16 += CL72_USERB0_CL72_2P5_DEFAULT_CONTROL_TX_FIR_TAP_PRE_2P5_INIT_VAL_MASK;
        data16 += 0;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB0_CL72_2P5_DEFAULT_CONTROLr(unit, pc, ln_access[lane], 
                   data16,mask16));
    
        mask16 = CL72_USERB1_CL72_DEBUG_1_REGISTER_TAP_MAX_VAL_MASK;
        data16 = 63 << CL72_USERB1_CL72_DEBUG_1_REGISTER_TAP_MAX_VAL_SHIFT;
        mask16 += CL72_USERB1_CL72_DEBUG_1_REGISTER_TAP_MIN_VAL_MASK;
        data16 += 27 << CL72_USERB1_CL72_DEBUG_1_REGISTER_TAP_MIN_VAL_SHIFT;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB1_CL72_DEBUG_1_REGISTERr(unit, pc, ln_access[lane],
                      data16,mask16));
    
        mask16 = CL72_USERB1_CL72_DEBUG_4_REGISTER_TAP_V2_VAL_MASK;
        data16 = 9 << CL72_USERB1_CL72_DEBUG_4_REGISTER_TAP_V2_VAL_SHIFT;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB1_CL72_DEBUG_4_REGISTERr(unit, pc, ln_access[lane], 
                   data16,mask16));

        if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* Write below values in one mdio write command.
             * This write should be done before configuring any speed.
             *  a. 0x820E[15:8] = 0x3 restart link
             *  b. 0x820E[7:6] = 0x0  ready_cmd =0 , error =0
             *  c. 0x820E[3:0] = 0x1  command restart
             */
            data16 = 3 << DSC1B0_UC_CTRL_SUPPLEMENT_INFO_SHIFT;
            data16 |= 1;
            mask16 = DSC1B0_UC_CTRL_SUPPLEMENT_INFO_MASK |
                     DSC1B0_UC_CTRL_READY_FOR_CMD_MASK |
                     DSC1B0_UC_CTRL_ERROR_FOUND_MASK |
                     DSC1B0_UC_CTRL_GP_UC_REQ_MASK;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], data16,mask16));
        }
    } else if (WC40_REVID_B0(pc)) {
        mask16 = CL72_USERB1_CL72_DEBUG_4_REGISTER_TAP_V2_VAL_MASK;
        data16 = 9 << CL72_USERB1_CL72_DEBUG_4_REGISTER_TAP_V2_VAL_SHIFT;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_CL72_USERB1_CL72_DEBUG_4_REGISTERr(unit, pc, ln_access[lane], 
                   data16,mask16));

        if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* Write below values in one mdio write command. 
             * This write should be done before configuring any speed. 
             *  a. 0x820E[15:8] = 0x3 restart link 
             *  b. 0x820E[7:6] = 0x0  ready_cmd =0 , error =0 
             *  c. 0x820E[3:0] = 0x1  command restart 
             */
            data16 = 3 << DSC1B0_UC_CTRL_SUPPLEMENT_INFO_SHIFT;
            data16 |= 1;    
            mask16 = DSC1B0_UC_CTRL_SUPPLEMENT_INFO_MASK |
                     DSC1B0_UC_CTRL_READY_FOR_CMD_MASK |
                     DSC1B0_UC_CTRL_ERROR_FOUND_MASK |
                     DSC1B0_UC_CTRL_GP_UC_REQ_MASK;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], data16,mask16));
        }
    }
    return SOC_E_NONE; 
}

STATIC int
_phy_wc40_independent_lane_init(int unit, soc_port_t port)
{
    int                 fiber;
    uint16              data16, mask16;
    int                 rv;
    phy_ctrl_t         *pc;
    soc_port_ability_t  ability;
    WC40_TX_DRIVE_t tx_drv[NUM_LANES];

    /* Only need to track fiber mode state.
     * Copper mode state is tracked by external PHY.
     */
    pc                       = INT_PHY_SW_STATE(unit, port);
    fiber                    = DEV_CFG_PTR(pc)->fiber_pref;
    pc->fiber.enable         = fiber;
    pc->fiber.preferred      = fiber;
    pc->fiber.autoneg_enable = 1;
    pc->fiber.force_speed    = 1000;
    pc->fiber.force_duplex   = TRUE;
    pc->fiber.master         = SOC_PORT_MS_NONE;
    pc->fiber.mdix           = SOC_PORT_MDIX_NORMAL;

   
    /* cannot do the RESET on lane0 for B0 and above. The div16/div33 clock configuration 
     * for all lanes uses the per-lane register on lane0. 
     */ 
    if ((pc->lane_num != 0) || WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        /* perform IEEE control register reset on each lane  */
     
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, MII_CTRL_RESET));

        rv = _phy_wc40_regbit_set_wait_check(pc,WC40_COMBO_IEEE0_MIICNTLr,
                     MII_CTRL_RESET,0,10000,0);

        if (rv == SOC_E_TIMEOUT) {
            SOC_DEBUG_PRINT((DK_WARN,
                             "Combo SerDes reset failed: u=%d p=%d\n",
                             unit, port));
        }
    }

    /* restore tx fir setting after reset  */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_fir_default_set(unit,port,pc->lane_num));
    if (IS_DUAL_LANE_PORT(pc)) {
        SOC_IF_ERROR_RETURN
            (_phy_wc40_tx_fir_default_set(unit,port,pc->lane_num + 1));
    }

    /* configure autoneg default settings */
    SOC_IF_ERROR_RETURN  /* OS CDR default */
        (WRITE_WC40_CL72_USERB0_CL72_OS_DEFAULT_CONTROLr(unit, pc, 0x00,0x03f0));

    SOC_IF_ERROR_RETURN  /* BR CDR default */
        (WRITE_WC40_CL72_USERB0_CL72_2P5_DEFAULT_CONTROLr(unit, pc, 0x00,0x03f0));

    SOC_IF_ERROR_RETURN  /* BR CDR default */
        (MODIFY_WC40_CL72_USERB0_CL72_MISC1_CONTROLr(unit, pc, 0x00,
                  0x3f << CL72_USERB0_CL72_MISC1_CONTROL_TX_FIR_TAP_MAIN_KX_INIT_VAL_SHIFT,
                  CL72_USERB0_CL72_MISC1_CONTROL_TX_FIR_TAP_MAIN_KX_INIT_VAL_MASK));

    /* Warpcore B0 workaround in autoneg mode Jira# PHY-388 */
    if (WC40_REVID_B0(pc) && (!IS_DUAL_LANE_PORT(pc))) {
        if (DEV_CFG_PTR(pc)->cl73an) {
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,LANE0_ACCESS,
                    WC40_RX0_ANARXCONTROLPCIr + (0x10 * pc->lane_num), 
                      RXB_ANARXCONTROLPCI_SYNC_STATUS_FORCE_R_SM_MASK,
                      RXB_ANARXCONTROLPCI_SYNC_STATUS_FORCE_R_SM_MASK |
                      RXB_ANARXCONTROLPCI_SYNC_STATUS_FORCE_R_MASK));
        }
    }

    /* Workaround Jira# SDK-32387, init. default IEEE 1G speed */    
    if (WC40_SDK32387_REVS(pc) && (!IS_DUAL_LANE_PORT(pc))) {
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, MII_CTRL_SS_1000 | MII_CTRL_FD));
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 
                      XGXSBLK2_UNICOREMODE10G_RESERVED0_MASK,
                      XGXSBLK2_UNICOREMODE10G_RESERVED0_MASK));
    }

    /* 0x8357[10:9] = 2'b0
     * Set new advertisement page count identifier(5 user pages) that differentiates it from 
     * HyperCore or other legacy devices(3 user pages)
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL6_UD_FIELDr(unit, pc, 0x00, 
                DIGITAL6_UD_FIELD_UD_ADV_ID_WC, DIGITAL6_UD_FIELD_UD_ADV_ID_MASK));

    /* hold tx/rx in reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK,
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));

    if (IS_DUAL_LANE_PORT(pc)) {
        /* sync clock divider for dxgxs two lanes, B0 and above */
        if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_CL49_USERB0_CONTROLr(unit, pc, 0x00, 0xc0, 0xc0));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_CL49_USERB0_CONTROLr(unit, pc, ln_access[pc->lane_num+1], 
                             0xc0, 0xc0));
        }
   
        /* workaround for KR2 speed status, select the PCS type as 40GBASE-R PCS */
        if (WC40_REVID_B0(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_PCS_IEEE0BLK_PCS_IEEECONTROL2r(unit, pc, 0x00, 
                         0x4, PCS_IEEE0BLK_PCS_IEEECONTROL2_PCS_TYPESEL_MASK));
        }
    }

    /* cannot be applied in dual-lane port */
    if (!IS_DUAL_LANE_PORT(pc)) {
        /* keep CL48 sync acquisition state machine in reset to avoid CL73 problem*/
        if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit, pc, LANE0_ACCESS, 0x80ba+(0x10*pc->lane_num),
                       RX0_ANARXCONTROLPCI_LINK_EN_FORCE_SM_MASK,
                       RX0_ANARXCONTROLPCI_LINK_EN_FORCE_SM_MASK |
                       RX0_ANARXCONTROLPCI_LINK_EN_R_MASK));
        }
    }

    /* XXX not to control the speeds thru pma/pmd register */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                           0,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));

    /* check if TX/RX polarities should be flipped */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_ind_lane_polarity_set(unit,pc, TRUE));

    /* config RX clock compensation used in single-lane and dual-lane ports */
    if (IS_DUAL_LANE_PORT(pc)) { 
        if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00,
                         0x0080));
        } else { /* B0 and other versions */ 
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 0x8091));
        }
    } else { 
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_RX66_CONTROLr(unit, pc, 0x00,
		RX66_CONTROL_CC_EN_MASK |RX66_CONTROL_CC_DATA_SEL_MASK, 
                RX66_CONTROL_CC_EN_MASK | RX66_CONTROL_CC_DATA_SEL_MASK));
    }

    /*  support back to back packets in higig+/higig2 mode without IPG */
    if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
        if (IS_DUAL_LANE_PORT(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK0_XGMIIRCONTROLr(unit, pc, 0x00,
                DEV_CFG_PTR(pc)->hg_mode? XGXSBLK0_XGMIIRCONTROL_CKCMP_NOIPG_EN_MASK: 0,
                XGXSBLK0_XGMIIRCONTROL_CKCMP_NOIPG_EN_MASK));
        } else { /* quad-port mode */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RX66_CONTROLr(unit, pc, 0x00,
                DEV_CFG_PTR(pc)->hg_mode? RX66_CONTROL_CC_NOIPG_EN_MASK: 0,
                RX66_CONTROL_CC_NOIPG_EN_MASK));
        }
    }

    /* Set Local Advertising Configuration */
    SOC_IF_ERROR_RETURN
        (phy_wc40_ability_local_get(unit, port, &ability));
 
    pc->fiber.advert_ability = ability;
    SOC_IF_ERROR_RETURN
        (phy_wc40_ability_advert_set(unit, port, &ability));

    data16 = SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_MASK |
             SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_MASK;
    mask16 = data16 | SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK;

    if (DEV_CFG_PTR(pc)->pdetect1000x) {
        /* Enable 1000X parallel detect */
        data16 |= SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK;
    }
    
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, data16, mask16));

    /* Configure default preemphasis, predriver, idriver values. Use
     * KR entry as default
     */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_get(unit, port,&tx_drv[0],TXDRV_DFT_INX));
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_set(unit, port,&tx_drv[0]));

    /* Configuring operating mode */
    data16 = SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK;
    mask16 = SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK |
             SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK |
             SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK;

    /*
     * Configure signal auto-detection between SGMII and fiber.
     * This detection only works if auto-negotiation is enabled.
     */
    if (DEV_CFG_PTR(pc)->auto_medium) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK;
    }

    /*
     * Put the Serdes in Fiber or SGMII mode
     */
    if (DEV_CFG_PTR(pc)->fiber_pref) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    } else {
        data16 &= ~SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    }

    if (DEV_CFG_PTR(pc)->sgmii_mstr) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_SGMII_MASTER_MODE_MASK;
        mask16 |= SERDESDIGITAL_CONTROL1000X1_SGMII_MASTER_MODE_MASK;
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit, pc, 0x00, data16, mask16));

    /* Set FIFO Elasticity to 13.5k and
     * Disable CRS generation on TX in half duplex mode
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X3r(unit, pc, 0x00,
             SERDESDIGITAL_CONTROL1000X3_DISABLE_TX_CRS_MASK |
             (1 << 2),
             SERDESDIGITAL_CONTROL1000X3_DISABLE_TX_CRS_MASK |
             SERDESDIGITAL_CONTROL1000X3_FIFO_ELASICITY_TX_MASK));

    /* Set the dswin to 7 as deskew fifo depth is reduced */
    if (WC40_LINK_WAR_REVS(pc)) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK2_ASWAP66CONTROL2r(unit, pc, 0x00, 0x7, 0x1F));
    }

    if (IS_DUAL_LANE_PORT(pc)) {
        DUAL_LANE_BCST_ENABLE(pc);
        /* config the 64B/66B for 20g dxgxs, won't affect other speeds and AN  */
        SOC_IF_ERROR_RETURN
            (_wc40_xgmii_scw_config (unit,pc));

        DUAL_LANE_BCST_DISABLE(pc);
    }

    /* autoneg is not supported in dual-port and custom1 mode */
    if (CUSTOM1_MODE(pc) || IS_DUAL_LANE_PORT(pc)) {
        uint16 sp_ctrl0;
        uint16 sp_ctrl1;
        uint16 speed_val = 0;
        int    tx_inx;

        DUAL_LANE_BCST_ENABLE(pc);
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, 0, MII_CTRL_AE));

        
        if (CUSTOM1_MODE(pc)) {
            sp_ctrl0 = 0xf;
            sp_ctrl1 = DIGITAL4_MISC3_FORCE_SPEED_B5_MASK;
        } else { /* general dual-port mode, default 20G if supported, otherwise 10G*/
            SOC_IF_ERROR_RETURN
                (_phy_wc40_ind_speed_ctrl_get(unit,port,pc->speed_max,&speed_val,&tx_inx));
            sp_ctrl1 = (speed_val & 0x20)? DIGITAL4_MISC3_FORCE_SPEED_B5_MASK: 0;
            sp_ctrl0 = speed_val & 0x1f;
        }
        
        /* force to the speed */
        SOC_IF_ERROR_RETURN
             (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, sp_ctrl0, 
                                  SERDESDIGITAL_MISC1_FORCE_SPEED_MASK));
         SOC_IF_ERROR_RETURN
             (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, 
                     sp_ctrl1,
                     DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));
        DUAL_LANE_BCST_DISABLE(pc);

    } else { /* all other modes, enable autoneg by default */
        /* clear forced speed control. Some two-lane ports are strapped
         * in forced dxgxs speed.
         */
        DUAL_LANE_BCST_ENABLE(pc);
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, 0,
                               SERDESDIGITAL_MISC1_FORCE_SPEED_MASK));
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, 0,
                    DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));

        DUAL_LANE_BCST_DISABLE(pc);

        /* enable autoneg */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, 
                          MII_CTRL_AE | MII_CTRL_RAN,
                          MII_CTRL_AE | MII_CTRL_RAN));
        if (DEV_CFG_PTR(pc)->cl73an) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, 
                                      MII_CTRL_AE | MII_CTRL_RAN,
                                      MII_CTRL_AE | MII_CTRL_RAN));
        }
    }

    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        /* put inband mdio rx in reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK3_LOCALCONTROL0r(unit,pc,0x00, (1 << 3), (1 << 3)));
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
              0,
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));

    /* XXX 0x833d bit 15 set, enable the auto select of KR,KRx4,KXx4 KX link
     * reporting selection of PCS device
     */
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, (1 << 15),
                    (1 << 15)));

    SOC_DEBUG_PRINT((DK_PHY,
                     "_phy_wc40_independent_lane_init: u=%d p=%d %s\n",
                     unit, port, (fiber) ? "Fiber" : "Copper"));
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_combo_core_init(int unit, soc_port_t port)
{
    phy_ctrl_t        *pc;
    soc_port_ability_t ability;
    uint16             data16, mask16;
    uint8 *pdata;
    int   ucode_len;
    int   alloc_flag;
    int   lane;
    WC40_TX_DRIVE_t tx_drv[NUM_LANES];

    pc = INT_PHY_SW_STATE(unit, port);

    /* must be done after reset, otherwise will be cleared  */
    data16 = (XGXSBLK0_XGXSCONTROL_MODE_10G_ComboCoreMode <<
                        XGXSBLK0_XGXSCONTROL_MODE_10G_SHIFT);
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00,data16,
                                       (XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK |
                                        XGXSBLK0_XGXSCONTROL_MODE_10G_MASK)));

    /* configure default fir settings for all lanes */
    for (lane = 0; lane < 4; lane++) {
        SOC_IF_ERROR_RETURN
            (_phy_wc40_tx_fir_default_set(unit,port,lane));
    }

    /* configure autoneg default settings */
    SOC_IF_ERROR_RETURN  /* OS CDR default */
        (WRITE_WC40_CL72_USERB0_CL72_OS_DEFAULT_CONTROLr(unit, pc, LANE_BCST,0x03f0));

    if (WC40_REVID_B0(pc)) {
        SOC_IF_ERROR_RETURN  /* BR CDR default */
            (WRITE_WC40_CL72_USERB0_CL72_BR_DEFAULT_CONTROLr(unit, pc, LANE_BCST,0x035a));
    }

    /* 0x8357[10:9] = 2'b0
     * Set new advertisement page count identifier(5 user pages) that differentiates it from 
     * HyperCore or other legacy devices(3 user pages)
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL6_UD_FIELDr(unit, pc, 0x00, 
                DIGITAL6_UD_FIELD_UD_ADV_ID_WC, DIGITAL6_UD_FIELD_UD_ADV_ID_MASK));

    /* XXX not to control the speeds thru pma/pmd register */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                           0,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));

    /* configure txck/rx  */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK6_XGXSX2CONTROL2r(unit,pc,0x00, 
                  WC40_TXCK_MASK | WC40_RXCK_OVERIDE_MASK,
                  WC40_TXCK_MASK | WC40_RXCK_OVERIDE_MASK)); 

    /* disable CL73 BAM */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_CL73_USERB0_CL73_BAMCTRL1r(unit,pc,0x00,0,
               CL73_USERB0_CL73_BAMCTRL1_CL73_BAMEN_MASK));
                                                                                
    /* disable CL73 AN device*/
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit,pc,0x00,0));

    SOC_IF_ERROR_RETURN
        (phy_wc40_ability_local_get(unit, port, &ability));
    
    SOC_IF_ERROR_RETURN
        (phy_wc40_ability_advert_set(unit, port, &ability));

    data16 = 0;
    mask16 = XGXSBLK5_PARDET10GCONTROL_PD_12G_TXDON_DISABLE_MASK |
             XGXSBLK5_PARDET10GCONTROL_PD_12G_DISABLE_MASK |
             XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK;
    if (DEV_CFG_PTR(pc)->pdetect10g) { 
        data16 |=XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK;
        if (pc->speed_max <= 10000) {
            /* Disable parallel detect for 12Gbps. */
            data16 |= XGXSBLK5_PARDET10GCONTROL_PD_12G_TXDON_DISABLE_MASK |
                      XGXSBLK5_PARDET10GCONTROL_PD_12G_DISABLE_MASK;
        }
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK5_PARDET10GCONTROLr(unit, pc, 0x00, data16, mask16));

    /* Configure default preemphasis, predriver, idriver values. Use
     * KR4 entry as default
     */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_get(unit, port,&tx_drv[0],TXDRV_DFT_INX));
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_set(unit, port,&tx_drv[0]));

    /*
     * Transform CX4 pin out on the board to HG pinout
     */
    if (DEV_CFG_PTR(pc)->cx42hg) {
        /* If CX4 to HG conversion is enabled, do not allow individual lane
         * swapping.
         */
        SOC_IF_ERROR_RETURN     /* Enable TX Lane swap */
            (WRITE_WC40_XGXSBLK8_TXLNSWAP1r(unit, pc, 0x00, 0x80e4));

        SOC_IF_ERROR_RETURN     /* Flip TX Lane polarity */
            (WRITE_WC40_TXB_ANATXACONTROL0r(unit, pc, 0x00, 0x1020));
    } else {

        /* check if any tx/rx lanes are remapped */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_rxlane_map_set(unit,pc,
                 (uint16)(DEV_CFG_PTR(pc)->rxlane_map)));
   
        SOC_IF_ERROR_RETURN
            (_phy_wc40_txlane_map_set(unit,pc,
                 (uint16)(DEV_CFG_PTR(pc)->txlane_map)));
        
        /* Tx/Rx Polarity control */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_combo_polarity_set(unit,pc,TRUE));

        /* select div16 clock */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_rx_div16_clk_select(unit,pc));
    }

    if (DEV_CFG_PTR(pc)->refclk == 161) {
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_SERDESDIGITAL_MISC1r(unit,pc,0x00,0xB900));
    }

    /* broadcast to all lanes */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, WC_AER_BCST_OFS_STRAP));

    /* set BRCM 31G control */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit, pc, 0x00, 
                       DIGITAL5_MISC6_USE_BRCM6466_31500_CYA_MASK, 
                       DIGITAL5_MISC6_USE_BRCM6466_31500_CYA_MASK));

    /* config the 64B/66B for 25G, won't affect other speeds and AN  */
    SOC_IF_ERROR_RETURN
        (_wc40_xgmii_scw_config (unit,pc));

    /* Set FIFO Elasticity to 13.5k and
     * Disable CRS generation on TX in half duplex mode
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X3r(unit, pc, 0x00,
             SERDESDIGITAL_CONTROL1000X3_DISABLE_TX_CRS_MASK |
             (1 << 2),
             SERDESDIGITAL_CONTROL1000X3_DISABLE_TX_CRS_MASK |
             SERDESDIGITAL_CONTROL1000X3_FIFO_ELASICITY_TX_MASK));

    /* clock compensation configuration in combo mode */
    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 0x91));
    } else {  /* B0 and ... */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 0x8091));
    }

    /* reset AER */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, 0));

    /* Set the dswin to 7 as deskew fifo depth is reduced */
    if (WC40_LINK_WAR_REVS(pc)) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK2_ASWAP66CONTROL2r(unit, pc, 0x00, 0x7, 0x1F));
    }

    /*
     * Configure signal auto-detection between SGMII and fiber.
     */
    data16 = SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK;
    mask16 = SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK |
             SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK |
             SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;

    if (DEV_CFG_PTR(pc)->auto_medium) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK;
    }
   
    if (DEV_CFG_PTR(pc)->fiber_pref) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit, pc, 0x00, data16, mask16));

    /* set filter_force_link and disable_false_link */

    mask16 = SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_MASK |
             SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_MASK;
    data16 = mask16;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, data16, mask16));

    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        /* put inband mdio rx in reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK3_LOCALCONTROL0r(unit,pc,0x00, (1 << 3), (1 << 3)));
    }

    /* uCode should be done in the last in init sequence. Firmware may do additional
     * configuration.
     */
    if (DEV_CFG_PTR(pc)->load_mthd) {
        SOC_IF_ERROR_RETURN
            (_phy_wc40_ucode_get(unit,port,&pdata,&ucode_len,&alloc_flag));
        SOC_IF_ERROR_RETURN
            (phy_wc40_firmware_load(unit,port,0,pdata, ucode_len));
        if (alloc_flag) {
            sal_free(pdata);
        }
    } else {
        SOC_DEBUG_PRINT((DK_WARN,
                   "WC40 combo mode : uC RAM download skipped: u=%d p=%d\n",
                            unit, port));
    }

    mask16 = XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK;
    data16 = mask16; 
    /* Workaround: tx fifo recenter problem in aggregated mode */ 
    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
        mask16 |= XGXSBLK0_XGXSCONTROL_AFRST_EN_MASK;
    }
    SOC_IF_ERROR_RETURN
         (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, data16, mask16));

    (void) _phy_wc40_pll_lock_wait(unit, port);

    /* disable CL73 AN device in case it is not used*/
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit,pc,0x00,0));
    
    /* put device in autoneg mode */
    /* clear forced bit */
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, 0, 
                         SERDESDIGITAL_MISC1_FORCE_SPEED_MASK));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, 0, 
                         DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00,
                                             MII_CTRL_AE | MII_CTRL_RAN,
                                             MII_CTRL_AE | MII_CTRL_RAN));
    if (DEV_CFG_PTR(pc)->cl73an) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00,
                                          MII_CTRL_AE | MII_CTRL_RAN,
                                          MII_CTRL_AE | MII_CTRL_RAN));
    }

    /* XXX 0x833d bit 15 set, enable the auto select of KR,KRx4,KXx4 KX link
     * reporting selection of PCS device
     */
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, (1 << 15),
                    (1 << 15)));

    /* XXX set default speed mode */
    /* SOC_IF_ERROR_RETURN(phy_wc40_an_set(unit, port, TRUE)); */

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_custom_mode_init(int unit, soc_port_t port)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_ind_init
 * Purpose:     
 *      Initialize hc phys
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
_phy_wc40_ind_init(int unit, soc_port_t port)
{
    phy_ctrl_t         *pc;               
    uint16             xgxs_ctrl;
    uint16             data16;
    uint16             mask16;
    uint16             txck_mode;
    int vco_freq;
    uint8 *pdata;
    int   ucode_len;
    int   alloc_flag;

    pc = INT_PHY_SW_STATE(unit, port);

    /* In this mode, the device's 4 lanes are usaually connected to more than
     * one switch port.
     * It is necessary to configure and initialize the shared resource
     * once before going to each lane. 
     */

    if (!_wc40_chip_init_done(unit,pc->chip_num,pc->phy_mode)) {
        /* configure and initialize the resource shared by all 4 lanes*/

        SOC_IF_ERROR_RETURN(_wc40_soft_reset(unit,pc)); /* reset once */

        /* select multi mmd and multi-port address mode */
        data16 = XGXSBLK0_MMDSELECT_DEVCL22_EN_MASK |
                 XGXSBLK0_MMDSELECT_DEVDEVAD_EN_MASK |
                 XGXSBLK0_MMDSELECT_DEVPMD_EN_MASK  |
                 XGXSBLK0_MMDSELECT_DEVAN_EN_MASK |
                 XGXSBLK0_MMDSELECT_MULTIMMDS_EN_MASK;
        mask16 = data16 |
                 XGXSBLK0_MMDSELECT_MULTIPRTS_EN_MASK;
        if (!(pc->flags & PHYCTRL_MDIO_ADDR_SHARE)) {
            data16 |= XGXSBLK0_MMDSELECT_MULTIPRTS_EN_MASK;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_MMDSELECTr(unit, pc, LANE0_ACCESS, data16,mask16));

        /* set mdio override control to send out 312.5MHz clock on txck_out[0].
         * this allow lane1-lane3 to support 10G speed while lane 0 runs at 1G.
         * Apply to a1/b0. No workaround for a0
         */
        if ((!IS_DUAL_LANE_PORT(pc)) && (WC40_REVID_A1(pc) || WC40_REVID_B0(pc))) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK6_XGXSX2CONTROL2r(unit, pc, LANE0_ACCESS,
                       (1 << 13), (1 << 13)));
        }

        /* warpcore does not support os4(xgxs_ctrl=6). 
         * Should use os8 for independent channel mode
         */
        xgxs_ctrl = DEV_CFG_PTR(pc)->lane_mode;

        xgxs_ctrl <<= XGXSBLK0_XGXSCONTROL_MODE_10G_SHIFT;

        /* set device's mode and disable PLL sequencer */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, LANE0_ACCESS, 
                       xgxs_ctrl,
                       XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK |
                       XGXSBLK0_XGXSCONTROL_MODE_10G_MASK));

        /* configure the txck/rxck for all lanes */
        if (IS_DUAL_LANE_PORT(pc)) {
            txck_mode = 0x9;
        } else {
            txck_mode = 0;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK6_XGXSX2CONTROL2r(unit,pc,LANE0_ACCESS, 
                  (txck_mode << WC40_TXCK_SHIFTER) | WC40_RXCK_OVERIDE_MASK,
                      WC40_TXCK_MASK | WC40_RXCK_OVERIDE_MASK)); 

        /* broadcast to all lanes */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AERBLK_AERr(unit,pc,LANE0_ACCESS, WC_AER_BCST_OFS_STRAP));

        if (DEV_CFG_PTR(pc)->lane_mode == xgxs_operationModes_IndLane_OS5) {
            vco_freq = WC40_OS5_VCO_FREQ;
        } else  { /* OS8 */
            vco_freq = WC40_OS8_VCO_FERQ;
            if (DEV_CFG_PTR(pc)->refclk == 161) {
                vco_freq = 0xB900;
            }
        }

        /* configure VCO frequency */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_SERDESDIGITAL_MISC1r(unit,pc,LANE0_ACCESS,vco_freq));

        /* disable 10G parallel detect */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_XGXSBLK5_PARDET10GCONTROLr(unit, pc, LANE0_ACCESS, 0));

        /* reset AER */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AERBLK_AERr(unit,pc,LANE0_ACCESS, 0));

        /* check if any tx/rx lanes are remapped */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_rxlane_map_set(unit,pc,
                 (uint16)(DEV_CFG_PTR(pc)->rxlane_map)));
   
        SOC_IF_ERROR_RETURN
            (_phy_wc40_txlane_map_set(unit,pc,
                 (uint16)(DEV_CFG_PTR(pc)->txlane_map)));

        /* select div16 clock */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_rx_div16_clk_select(unit,pc));

        if (DEV_CFG_PTR(pc)->load_mthd) { 
            SOC_IF_ERROR_RETURN
                (_phy_wc40_ucode_get(unit,port,&pdata,&ucode_len,&alloc_flag));
            SOC_IF_ERROR_RETURN
                (phy_wc40_firmware_load(unit,port,0,pdata, ucode_len));
            if (alloc_flag) {
                sal_free(pdata);
            }
        } else {
            SOC_DEBUG_PRINT((DK_WARN,
                       "WC40 independent mode : uC RAM download skipped: u=%d p=%d\n",
                            unit, port));
        }

        mask16 = XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK;
        data16 = mask16;
        /* Workaround: tx fifo recenter problem in aggregated mode */
        if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
            if (IS_DUAL_LANE_PORT(pc)) {
                mask16 |= XGXSBLK0_XGXSCONTROL_AFRST_EN_MASK;
            }
        }

        /* enable PLL sequencer */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, LANE0_ACCESS, data16, mask16));

        /* PLL clock lock wait */
        (void) _phy_wc40_pll_lock_wait(unit, port);
    }

    SOC_IF_ERROR_RETURN
        (_phy_wc40_independent_lane_init(unit, port));

    pc->flags |= PHYCTRL_INIT_DONE;

    return SOC_E_NONE;
}
    
/*
 * Function:
 *      phy_wc40_init
 * Purpose:     
 *      Initialize hc phys
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_init(int unit, soc_port_t port)
{
    phy_ctrl_t         *pc;               

    /* init the configuration value */
    _phy_wc40_config_init(unit,port);

    pc = INT_PHY_SW_STATE(unit, port);

    if (PHY_INDEPENDENT_LANE_MODE(unit, port)) {
        if (DEV_CFG_PTR(pc)->custom) {
            SOC_IF_ERROR_RETURN
                (_phy_wc40_custom_mode_init(unit, port));
        } else {
            SOC_IF_ERROR_RETURN
                (_phy_wc40_ind_init(unit,port));
        }
    } else {
        SOC_IF_ERROR_RETURN(_wc40_soft_reset(unit,pc)); /* soft reset */

        /* Force to comboCore mode.
         * Support SGMII 10/100/1000Mbps, 1000X, 2500X, 10G, and 12G.
         */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_combo_core_init(unit, port));
    } 

    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_wc40_init: u=%d p=%d\n", unit, port));
    return SOC_E_NONE;
}

#define PCS_STATUS_LANE4_SYNC   0x80F
STATIC int
_phy_wc40_war_link_check(int unit, soc_port_t port)
{     
    uint16 sync_stat;
    uint16 sync_good;
    uint16 link;
    phy_ctrl_t         *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    /* need to read the current link state */
    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK4_XGXSSTATUS1r(unit, pc, 0x00, &link));

    if (link & XGXSBLK4_XGXSSTATUS1_LINK10G_MASK) {
        link = TRUE;
    } else {
        link = FALSE;
    }

    /* if not a valid state, such as in autoneg mode or not BRCM 64B/66B speeds,
     * or if linkup, no need to go thru workaround check
     */
    if ((!WC40_SP_VALID(pc)) || (link==TRUE)) {
        WC40_SP_CNT_CLEAR(pc);
        return SOC_E_NONE;
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "_phy_wc40_war_link_check: u=%d p=%d: \n", unit, port));

    sync_good = PCS_STATUS_LANE4_SYNC;

    /* now check the sync status on appropriate lanes. dxgxs0/dxgxs1 and combo
     * should have the same sync status, i.e. 0x80F. dxgxs1 is read from lane 2.
     */
    SOC_IF_ERROR_RETURN
        (READ_WC40_PCS_IEEE1BLK_PCS_LANESTATUSr(unit, pc, 0x00, &sync_stat));

    /* check if all lane sync'ed up */
    if (sync_stat == sync_good) {
        if (WC40_SP_CNT(pc)) {
            SOC_DEBUG_PRINT((DK_PHY,
                "_phy_wc40_war_link_check workaround applied: u=%d p=%d: \n", unit, port));

            if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
                /* broadcast to all lanes */
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, WC_AER_BCST_OFS_STRAP));

            } else { /* dual xgxs mode */
                DUAL_LANE_BCST_ENABLE(pc);
            }
               
            /* if it is synced and no link, do the rx reset */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DIGITAL5_MISC6r(unit, pc, 0x00,
                                     DIGITAL5_MISC6_RESET_RX_ASIC_MASK,
                                     DIGITAL5_MISC6_RESET_RX_ASIC_MASK));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DIGITAL5_MISC6r(unit, pc, 0x00, 0,
                                         DIGITAL5_MISC6_RESET_RX_ASIC_MASK));

            if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
                /* restore AER */
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, 0));
            } else { /* dual xgxs mode */
                DUAL_LANE_BCST_DISABLE(pc);
            }
        } else {
            WC40_SP_CNT_INC(pc);
        }
    }

    return SOC_E_NONE;
}

/*
 *      phy_wc40_link_get
 * Purpose:
 *      Get layer2 connection status.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      link - address of memory to store link up/down state.
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_link_get(int unit, soc_port_t port, int *link)
{
    uint16      mii_stat;
/*    uint16      data16; */
    phy_ctrl_t *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (CUSTOM_MODE(pc)) {
        *link = TRUE;
        return SOC_E_NONE;
    }

    if (PHY_DISABLED_MODE(unit, port)) {
        *link = FALSE;
        return SOC_E_NONE;
    }

    *link = FALSE;
    /* Check XAUI link first if XAUI mode */
    SOC_IF_ERROR_RETURN
        (READ_WC40_PCS_IEEE0BLK_PCS_IEEESTATUS1r(unit, pc, 0x00, &mii_stat));
    if (mii_stat & MII_STAT_LA) {
        *link = TRUE;
    }

    /* Finally Check combo link */
    if (*link == FALSE) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_COMBO_IEEE0_MIISTATr(unit, pc, 0x00, &mii_stat));
        if (mii_stat & MII_STAT_LA) {
            *link = TRUE;
        }
    }

    if (WC40_LINK_WAR_REVS(pc)) {
        (void)_phy_wc40_war_link_check(unit,port);
    }

    /* restore mld local fault configuration one second after autoneg starts
     * It needs to be disabled during autoneg
     */
    if (WC40_AN_VALID(pc)) {
        if (WC40_AN_CHECK_TIMEOUT(pc)) {
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit, pc, 0x00, 0x842A,0,(1 << 5)));
            WC40_AN_VALID_RESET(pc);
        }
    }

    if (DEV_CFG_PTR(pc)->cl73an == WC40_CL73_KR2) {
        SOC_IF_ERROR_RETURN
            (_wc40_soft_an_cl73kr2_check(unit,port));
    }

    /* XXX probably need to qualify  with speed status */
    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_wc40_ind_enable_set
 * Purpose:
 *      Enable/Disable phy
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - on/off state to set
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
_phy_wc40_ind_enable_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;
    uint16 mask16;
    uint16 data16;
    uint16 ln_ctrl;
    pc = INT_PHY_SW_STATE(unit, port);

    /* disable the CL73 timer */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_CL73_USERB0_CL73_UCTRL2r(unit,pc,0x00,
                enable? 0: (1 << 3), (1 << 3) ));

    /* disable the TX */
    mask16 = 1 << (XGXSBLK0_MISCCONTROL1_PMD_LANE0_TX_DISABLE_SHIFT + pc->lane_num); 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_MISCCONTROL1r(unit,pc,LANE0_ACCESS,
                enable? 0: mask16, mask16 ));

    /* now power down the lane if not used */
    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK1_LANECTRL3r(unit,pc,0x00,&ln_ctrl));

    mask16 = (1 << pc->lane_num);    /* rx lane */
    if (IS_DUAL_LANE_PORT(pc)) {
        mask16 |= (mask16 << 1);
    }

    mask16 |= (mask16 << 4); /* both tx and rx lane */

    if (!enable) {
        mask16 |= 0x800;
        /* MAC uses rx0_ck/tx0_ck clock from lane0 for all lanes.
         * Only powerdown lane0 when all other three lane's are powered down
         */ 
        if ((pc->speed_max >= 10000) && (pc->lane_num == 0)) {
            if (((ln_ctrl | mask16) & XGXSBLK1_LANECTRL3_PWRDN_RX_MASK) != 
                XGXSBLK1_LANECTRL3_PWRDN_RX_MASK) {
                mask16 &= ~0x11;  /* remove lane0 RX/TX control */
            }
        }
        data16 = mask16;
    } else {
        data16 = 0;
        mask16 |= 0x11;  /* always add lane0 RX when any lane is enabled */
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANECTRL3r(unit,pc,0x00,data16,mask16));

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_enable_set(int unit, soc_port_t port, int enable)
{
    int         rv;
    phy_ctrl_t *pc;
    uint16 mask16;
    uint16 data16;

    pc = INT_PHY_SW_STATE(unit, port);

    mask16 = XGXSBLK1_LANECTRL3_PWRDN_RX_MASK | XGXSBLK1_LANECTRL3_PWRDN_TX_MASK |
             XGXSBLK1_LANECTRL3_PWRDWN_FORCE_MASK;
    data16 = enable? 0: mask16;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANECTRL3r(unit,pc,0x00,data16,mask16));
    rv = SOC_E_NONE;
    if (enable) {
        rv = MODIFY_WC40_XGXSBLK0_MISCCONTROL1r(unit, pc, 0x00, 0,
                         XGXSBLK0_MISCCONTROL1_GLOBAL_PMD_TX_DISABLE_MASK);
    } else {
        rv = MODIFY_WC40_XGXSBLK0_MISCCONTROL1r(unit, pc, 0x00,
                         XGXSBLK0_MISCCONTROL1_GLOBAL_PMD_TX_DISABLE_MASK,
                         XGXSBLK0_MISCCONTROL1_GLOBAL_PMD_TX_DISABLE_MASK);
    }
    return rv;
}

/*
 * Function:
 *      phy_wc40_enable_set
 * Purpose:
 *      Enable/Disable phy 
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - on/off state to set
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_enable_set(int unit, soc_port_t port, int enable)
{
    int rv;
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);
    if (enable) {
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
    }
    if (CUSTOM_MODE(pc)) {
        return SOC_E_NONE;
    }

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        rv = _phy_wc40_enable_set(unit,port,enable);
    } else {
        rv = _phy_wc40_ind_enable_set(unit, port, enable);
    }
    return rv;
}

/* XXX VCO 10.3125: 10G,20G_dxgxs,1G,2.5,10M,100M, div66  autoneg
 *          6.25:  10M,100M, 1G, 2.5G, 10G_dxgxs   div40  autoneg
 *          6.5625: 10.5G_dxgxs and 12G_dxgxs      div42  force mode only in ind.
 *          8.125:  15.77G_dxgxs                   div52  force mode only in ind.
 * VCO is only controlled by lane0. All VCO settings, i.e. reg. 0x8308.[8:15]
 * in all lanes must have the same value as for lane0. Switching VCO will bring down
 * the link on all four lanes. 
 * Generally the port configuration thru local_ability_get function should ensure the 
 * supported speeds in a given port should not require switching the VCO in the independent
 * channel mode. However the driver speed/autoneg function will not enforce this 
 * restriction.  
 */
STATIC int
_phy_wc40_vco_set(int unit, soc_port_t port, int speed,int speed_val)
{
    phy_ctrl_t  *pc;
    uint16 data16;
    uint16 mask16;
    uint16 vco_freq;
    uint16 misc1_ctrl;
    uint16 misc3_ctrl;
   
    pc = INT_PHY_SW_STATE(unit, port);

    /* only need to check in dxgxs mode */

    if (!IS_DUAL_LANE_PORT(pc)) {
        return SOC_E_NONE;
    }

    vco_freq = SERDESDIGITAL_MISC1_REFCLK_SEL_clk_156p25MHz << 
               SERDESDIGITAL_MISC1_REFCLK_SEL_SHIFT;
    vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_BITS <<
                SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_SHIFT;

    if (DEV_CFG_PTR(pc)->custom1) {
        vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div40 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;

    /* use 10G dxgxs for ethernet mode, 10.5G in HG mode to make it compatiable
     * with HC.
     */
    } else if (speed == 10000) {
        if (DEV_CFG_PTR(pc)->hg_mode) { 
            vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div42 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
        } else {
            vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div40 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
        }
    } else if (speed == 12000) {
        vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div42 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
    } else if (speed == 15000) {
        vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div52 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
    } else if (speed == 20000) {
        vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div66 <<
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
    } else if (speed == 0) {
      /* autoneg, vco needs to be div66 */
      vco_freq |= SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div66 <<
                  SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT;
    }

    /* read current vco freq */
    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, &data16));

    mask16 = SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK |
             SERDESDIGITAL_MISC1_REFCLK_SEL_MASK |
             SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK;

    /* return if speed is in same vco frequency */
    if ((data16  & mask16) == vco_freq) {
        return SOC_E_NONE;
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "_phy_wc40_vco_set: u=%d p=%d: vco freq switched: 0x%x\n", unit, port,
         vco_freq));

    /* switch to desired VCO freq */

    /* read current speed value */
    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, &misc1_ctrl));
    misc1_ctrl &= ~(SERDESDIGITAL_MISC1_REFCLK_SEL_MASK |
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                    SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK |
                    SERDESDIGITAL_MISC1_FORCE_SPEED_MASK);                   
    misc1_ctrl |= vco_freq;
    misc1_ctrl |= (speed_val & SERDESDIGITAL_MISC1_FORCE_SPEED_MASK);

    SOC_IF_ERROR_RETURN
        (READ_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, &misc3_ctrl));
    if (speed_val & 0x20) {
        misc3_ctrl |= DIGITAL4_MISC3_FORCE_SPEED_B5_MASK;
    } else {
        misc3_ctrl &= ~DIGITAL4_MISC3_FORCE_SPEED_B5_MASK;
    }

    /* stop PLL sequencer. Bring down link on all ports */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00,
                                       0,
                                       XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));

    /* broadcast to all lanes */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AERBLK_AERr(unit,pc, LANE0_ACCESS, WC_AER_BCST_OFS_STRAP));

    SOC_IF_ERROR_RETURN
        (WRITE_WC40_SERDESDIGITAL_MISC1r(unit, pc, LANE0_ACCESS, misc1_ctrl));
   
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_DIGITAL4_MISC3r(unit, pc, LANE0_ACCESS, misc3_ctrl));
   
    /* restore AER */ 
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AERBLK_AERr(unit,pc, LANE0_ACCESS, 0));

    /* start PLL sequencer */ 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00,
                                       XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK,
                                       XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));
    (void) _phy_wc40_pll_lock_wait(unit, port);

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_enable_get
 * Purpose:
 *      Get Enable/Disable status
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - address of where to store on/off state
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_enable_get(int unit, soc_port_t port, int *enable)
{
    *enable = !PHY_FLAGS_TST(unit, port, PHY_FLAGS_DISABLE);

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_ind_speed_ctrl_get(int unit, soc_port_t port, int speed,
                             uint16 *speed_val,int *tx_inx)
{
    phy_ctrl_t  *pc;
    int          hg10g_port = FALSE;

    pc = INT_PHY_SW_STATE(unit, port);

    if (DEV_CFG_PTR(pc)->hg_mode) { 
        hg10g_port = TRUE;
    }
   
    *speed_val = 0;
    switch (speed) {
    case 2500:
        *speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_2500BRCM_X1;
        *tx_inx = TXDRV_6GOS2_INX; 
        break;

    /* need to revisit here */
    /* XXX VCO 10.3125: 10G,20G_dxgxs,1G,2.5,10M,100M, div66  autoneg
     *          6.25:  10M,100M, 1G, 2.5G, 10G_dxgxs   div40  autoneg
     *          6.5625: 10.5G_dxgxs and 12G_dxgxs      div42  force mode only in ind. 
     *          8.125:  15.77G_dxgxs                   div52  force mode only in ind.
     */
    case 10000:  /* 10G_XFI, 10G_SFI, 10G dxgxs, 10G dxgxs hig */
        /* speed_val = 0x25;  10G XFI */
        /* speed_val = 0x29;  10G SFI */

        if (IS_DUAL_LANE_PORT(pc)) {
            if (DEV_CFG_PTR(pc)->custom1) {
               *speed_val = 0x30;  /* speed_x2_10000 */
            } else if (hg10g_port == TRUE) {
                /* speed_val = 0x21;   10.5HiG dual-XGXS SCR only */
                /* speed_val = 0x2D 10GHig DXGXS SCR */
                *speed_val = 0x21;
            } else {
                /* speed_val = 0x20;  10G ethernet dual-XGXS */
                /* speed_val = 0x2E 10G ethernet DXGXS SCR */
                *speed_val = DEV_CFG_PTR(pc)->scrambler_en? 0x2E: 0x20;
                *tx_inx = TXDRV_6GOS1_INX; 
            }
        } else {
           if ((DEV_CFG_PTR(pc)->line_intf & WC40_IF_SFI) ||
               (DEV_CFG_PTR(pc)->line_intf & WC40_IF_SR) ||
               (DEV_CFG_PTR(pc)->line_intf & WC40_IF_CR)) {
               *speed_val = SERDESDIGITAL_FORCE_SPEED_dr_10G_SFI;
           } else if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_XFI) {
               *speed_val = SERDESDIGITAL_FORCE_SPEED_dr_10G_XFI;
               *tx_inx = TXDRV_XFI_INX; 
           } else {
               /* XXX default to XFI */
               *speed_val = SERDESDIGITAL_FORCE_SPEED_dr_10G_XFI;
               *tx_inx = TXDRV_XFI_INX; 
           }
        }
        break;

    case 12000:  /* dxgxs */
        /* 12.773G       0x23
         * 12.773G CX4   0x24
         * 12G X2        0x2F
         */
        if (DEV_CFG_PTR(pc)->custom1) {
            *speed_val =  0x2F; /* custom1 12.7HiG dual-XGXS*/
        } else if (hg10g_port) {
            *speed_val = 0x23;
        } else {
            *speed_val = 0x24;
        }
        break;
    /* XXX PLL mode sets to SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_div52*/ 
    case 15000: /* speed_15750_hi_dxgxs */
        *speed_val = 0x36; 
        break;
    case 20000: /* 20G dxgxs, 20G dxgxs hig */
        *speed_val = (hg10g_port == TRUE) ?
                     0x27:  /* 20GHiG dual-XGXS */
                     0x28; /* 20G ethernet dual-XGXS */
        if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_KR) {
            *speed_val = 0x3F;  /* 20G KR2 mld2 */

            /* part of workaround for KR2 speed status, 
             * disable PCS type auto selection 
             */
            if (WC40_REVID_B0(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, 
                         0, (1 << 15)));
            }
        }
        break;
    default:
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_ind_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t  *pc;
    uint16       speed_val, mask16, data16;
    uint16       speed_mii;
    uint16       sgmii_status = 0;
    int tx_inx;
    WC40_TX_DRIVE_t tx_drv[NUM_LANES];

    pc = INT_PHY_SW_STATE(unit, port);

    /* set default entry first */
    tx_inx = TXDRV_DFT_INX;

    /* part of workaround for KR2 speed status, 
     * enable PCS type auto selection by default for all speeds
     */
    if (IS_DUAL_LANE_PORT(pc)) {
        if (WC40_REVID_B0(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, 
                         (1 << 15), (1 << 15)));
        }
    }
    speed_val = 0;
    speed_mii = 0;
    switch (speed) {
    case 0:
        /* Do not change speed */
        return SOC_E_NONE;
    case 10:
        tx_inx = TXDRV_6GOS2_INX;
        speed_mii = MII_CTRL_SS_10;
        break;
    case 100:
        tx_inx = TXDRV_6GOS2_INX;
        speed_mii = MII_CTRL_SS_100;
        break;
    case 1000:
        tx_inx = TXDRV_6GOS2_INX;
        speed_mii = MII_CTRL_SS_1000;
        break;
    default:   /* check brcm speeds */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_ind_speed_ctrl_get(unit,port,speed,&speed_val,&tx_inx));
        break;
    }

    /* configure uC for copper medium in SFI mode */
    mask16 = WC40_UC_CTRL_SFP_DAC << (pc->lane_num * 4);
    data16 = 0;
    if (speed_val == SERDESDIGITAL_FORCE_SPEED_dr_10G_SFI) {
        if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_SR) {
            data16 = 0;
            tx_inx = TXDRV_SFI_INX;
        } else if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_CR) {
            data16 = mask16;
            tx_inx = TXDRV_SFIDAC_INX; 
        } else if (DEV_CFG_PTR(pc)->medium == SOC_PORT_MEDIUM_COPPER) {
            data16 =mask16;
            tx_inx = TXDRV_SFIDAC_INX; 
        }
    }
    
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit,pc,LANE0_ACCESS,data16,mask16));

    /* configure the TX driver parameters per speed mode */        
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_get(unit, port,&tx_drv[0],tx_inx));
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_set(unit, port,&tx_drv[0]));

    /* check if need to switch to a different VCO */
    if (speed) {
        SOC_IF_ERROR_RETURN
            (_phy_wc40_vco_set(unit,port,speed,speed_val));
    }

    DUAL_LANE_BCST_ENABLE(pc);

    /* hold tx/rx in reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK, 
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));

    /* disable 100FX and 100FX auto-detect */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_FX100_CONTROL1r(unit,pc,0x00,
                              0,
                              FX100_CONTROL1_AUTODET_EN_MASK |
                              FX100_CONTROL1_ENABLE_MASK));

    /* disable 100FX idle detect */
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_FX100_CONTROL3r(unit,pc,0x00,
                              FX100_CONTROL3_CORRELATOR_DISABLE_MASK,
                              FX100_CONTROL3_CORRELATOR_DISABLE_MASK));

    /* Workaround Jira# SDK-32387 */
    /* control the speeds thru pma/pmd register */
    if (WC40_SDK32387_REVS(pc) && (!IS_DUAL_LANE_PORT(pc))) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));
    }

    data16 = speed_val;
    mask16 = SERDESDIGITAL_MISC1_FORCE_SPEED_MASK;

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, data16, mask16));

    data16 = (speed_val & 0x20)?  DIGITAL4_MISC3_FORCE_SPEED_B5_MASK:0;
    if (speed_val == SERDESDIGITAL_FORCE_SPEED_dr_10G_XFI ||
        speed_val == SERDESDIGITAL_FORCE_SPEED_dr_10G_SFI) {
        data16 |= DIGITAL4_MISC3_IND_40BITIF_MASK;
    } 
    mask16 = DIGITAL4_MISC3_FORCE_SPEED_B5_MASK | 
             DIGITAL4_MISC3_IND_40BITIF_MASK;   
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, data16, mask16));

    /* Workaround Jira# SDK-32387 */
    /* control the speeds thru pma/pmd register */
    if (WC40_SDK32387_REVS(pc) && (!IS_DUAL_LANE_PORT(pc))) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                           0,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));
    }

    if (speed <= 1000) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_SERDESDIGITAL_STATUS1000X1r(unit, pc, 0x00, &sgmii_status));

        sgmii_status &= SERDESDIGITAL_STATUS1000X1_SGMII_MODE_MASK;
        if (!sgmii_status && (speed == 100)) {

            /* fiber mode 100fx, enable */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_FX100_CONTROL1r(unit,pc,0x00,
                                  FX100_CONTROL1_FAR_END_FAULT_EN_MASK |
                                  FX100_CONTROL1_ENABLE_MASK,
                                  FX100_CONTROL1_FAR_END_FAULT_EN_MASK |
                                  FX100_CONTROL1_ENABLE_MASK));

            /* enable 100fx extended packet size */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_FX100_CONTROL2r(unit,pc,0x00,
                                FX100_CONTROL2_EXTEND_PKT_SIZE_MASK,
                                FX100_CONTROL2_EXTEND_PKT_SIZE_MASK));
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, speed_mii,
                                              MII_CTRL_SS_MASK));
        }
    }

    /* release the tx/tx reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
              0,
              DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));

    DUAL_LANE_BCST_DISABLE(pc);
    return SOC_E_NONE;
}


STATIC int
_phy_wc40_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t  *pc;
    uint16       speed_val, mask;
    uint16       speed_mii;
    uint16       sgmii_status = 0;
    uint16       data16;
    int          hg10g_port = FALSE;
    int          tx_inx;
    WC40_TX_DRIVE_t tx_drv[NUM_LANES];

    pc = INT_PHY_SW_STATE(unit, port);

    /* set default entry first */
    tx_inx = TXDRV_DFT_INX;

    pc = INT_PHY_SW_STATE(unit, port);
    if ((DEV_CFG_PTR(pc)->hg_mode) && (!DEV_CFG_PTR(pc)->cx4_10g)) { 
        hg10g_port = TRUE;
    }
  
    /* set to the default */ 
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, 0x0));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 
              XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_XGXS_nLQnCC <<
              XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_SHIFT,
              XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_MASK));

    if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
        /* clear 42G control, PLL autotune */
        if (DEV_CFG_PTR(pc)->refclk == 161) {
            data16 = 0x900 |   /*div64 */
                     SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK;
        } else {
            data16 = 0;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00,
                   data16, 
                   SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                   SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK));
    }

    speed_val = 0;
    speed_mii = 0;
    mask      =  SERDESDIGITAL_MISC1_FORCE_SPEED_MASK;
    switch (speed) {
    case 0:
        /* Do not change speed */
        return SOC_E_NONE;
    case 10:
        speed_mii = MII_CTRL_SS_10;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 100:
        speed_mii = MII_CTRL_SS_100;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 1000:
        speed_mii = MII_CTRL_SS_1000;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 2500:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_2500BRCM_X1; 
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 5000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_5000BRCM_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 6000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_6000BRCM_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 10000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_10GBASE_CX4; /* 10G CX4 */
        tx_inx = TXDRV_6GOS2_CX4_INX;
        if ((hg10g_port == TRUE) && DEV_CFG_PTR(pc)->rxaui) {
            /* speed_val = 0x1f;   10HiG dual-XGXS */
            /* speed_val = 0x2D 10GHig DXGXS SCR */
            speed_val = DEV_CFG_PTR(pc)->scrambler_en? 0x2D: 0x1F;
            tx_inx = TXDRV_6GOS1_INX;
        } else if (DEV_CFG_PTR(pc)->rxaui) {
            /* speed_val = 0x20;  10G ethernet dual-XGXS */
            /* speed_val = 0x2E 10G ethernet DXGXS SCR */
            speed_val = DEV_CFG_PTR(pc)->scrambler_en? 0x2E: 0x20;
            tx_inx = TXDRV_6GOS1_INX;
        } else if (hg10g_port == TRUE) {
            speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_10GHiGig_X4; /* 10G HiG */
            tx_inx = TXDRV_6GOS2_INX;
        }
        break;
    case 12000:
        speed_val = DEV_CFG_PTR(pc)->rxaui? 0x23: /* 12.7HiG dual-XGXS*/
                     SERDESDIGITAL_MISC1_FORCE_SPEED_dr_12GHiGig_X4;   /* 12 HiG */
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 12500:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_12p5GHiGig_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 13000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_13GHiGig_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 15000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_15GHiGig_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 16000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_16GHiGig_X4;
        tx_inx = TXDRV_6GOS2_INX;
        break;
    case 20000:
        /* speed_val = 0x2C 20G_SCR */
        speed_val = DEV_CFG_PTR(pc)->scrambler_en? 
                    0x2C:
                    SERDESDIGITAL_MISC1_FORCE_SPEED_dr_20GHiGig_X4;
        if (!(DEV_CFG_PTR(pc)->hg_mode)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 
                  XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_XGXG_nCC <<
                  XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_SHIFT,
                  XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_MASK));
        }
        tx_inx = TXDRV_6GOS1_INX;
        break;
    case 21000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_21GHiGig_X4;
        break;
    case 25000:
        speed_val = SERDESDIGITAL_MISC1_FORCE_SPEED_dr_25p45GHiGig_X4;
        break;
    case 30000:
        speed_val = 0x2a;
        tx_inx = TXDRV_XLAUI_INX;
        break;
    case 40000:
        if (DEV_CFG_PTR(pc)->hg_mode) {
            speed_val = 0x26; /* brcm 40G */
            tx_inx = TXDRV_XLAUI_INX;
        } else {
            if ((DEV_CFG_PTR(pc)->line_intf & WC40_IF_XLAUI) ||
                (DEV_CFG_PTR(pc)->line_intf & WC40_IF_KR4) ||
                (DEV_CFG_PTR(pc)->line_intf & WC40_IF_SR) || 
                (DEV_CFG_PTR(pc)->line_intf & WC40_IF_KR)) { 
                speed_val = 0x31;  /* 40GKR */
                tx_inx = TXDRV_XLAUI_INX;
            } else if ((DEV_CFG_PTR(pc)->line_intf & WC40_IF_CR4) ||
                       (DEV_CFG_PTR(pc)->line_intf & WC40_IF_CR) ) {
                speed_val = 0x32;  /* 40GCR4 */
                tx_inx = TXDRV_XLAUI_INX;
            } else {
                speed_val = 0x31;  /* default 40GKR */
                tx_inx = TXDRV_XLAUI_INX;
            }

            if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
                SOC_IF_ERROR_RETURN
                    (WC40_REG_MODIFY(unit, pc, 0x00, 0x00008420, 0x1, 0x3));
            }
        }
        if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_XLAUI) {
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, WC40_UC_CTRL_XLAUI));
            tx_inx = TXDRV_XLAUI_INX;
        } else if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_SR) {
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, WC40_UC_CTRL_SR4));
            tx_inx = TXDRV_SR4_INX;
        }
        break;

    /* only work for BRCM 40G */
    case 42000:
        if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
            speed_val = 0x26; /* use same brcm 40G speed setting */

            tx_inx = TXDRV_XLAUI_INX;
            /* force the PLL multiplier x70 */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, 
                       SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                       (0xc << SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT),
                       SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                       SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK));
            if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_XLAUI) {
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, 
                                                          WC40_UC_CTRL_XLAUI));
            } else if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_SR) {
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, 
                                                           WC40_UC_CTRL_SR4));
            }
        }
        break;
    default:
        return SOC_E_PARAM;
    }
    /* configure the TX driver parameters per speed mode */        
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_get(unit, port,&tx_drv[0],tx_inx));
    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_control_set(unit, port,&tx_drv[0]));

    /* Puts PLL in reset state and forces all datapath into reset state */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 0,
                                  XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));

    /* 2wire XAUI configuration */
    SOC_IF_ERROR_RETURN
        (_wc40_rxaui_config(unit,pc,DEV_CFG_PTR(pc)->rxaui));

    /* disable 100FX and 100FX auto-detect */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_FX100_CONTROL1r(unit,pc,0x00,
                              0,
                              FX100_CONTROL1_AUTODET_EN_MASK |
                              FX100_CONTROL1_ENABLE_MASK));

    /* disable 100FX idle detect */
    SOC_IF_ERROR_RETURN
            (MODIFY_WC40_FX100_CONTROL3r(unit,pc,0x00,
                              FX100_CONTROL3_CORRELATOR_DISABLE_MASK,
                              FX100_CONTROL3_CORRELATOR_DISABLE_MASK));

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, speed_val, mask));

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, 
                (speed_val & 0x20)?  DIGITAL4_MISC3_FORCE_SPEED_B5_MASK:0,
                DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));

    if (speed <= 1000) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_SERDESDIGITAL_STATUS1000X1r(unit, pc, 0x00, &sgmii_status));

        sgmii_status &= SERDESDIGITAL_STATUS1000X1_SGMII_MODE_MASK;
        if (!sgmii_status && (speed == 100)) {

            /* fiber mode 100fx, enable */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_FX100_CONTROL1r(unit,pc,0x00,
                                  FX100_CONTROL1_FAR_END_FAULT_EN_MASK |
                                  FX100_CONTROL1_ENABLE_MASK,
                                  FX100_CONTROL1_FAR_END_FAULT_EN_MASK |
                                  FX100_CONTROL1_ENABLE_MASK));

            /* enable 100fx extended packet size */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_FX100_CONTROL2r(unit,pc,0x00,
                                FX100_CONTROL2_EXTEND_PKT_SIZE_MASK,
                                FX100_CONTROL2_EXTEND_PKT_SIZE_MASK));
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, speed_mii,
                                              MII_CTRL_SS_MASK));
        }
    }

    /* Bring PLL out of reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));
    (void) _phy_wc40_pll_lock_wait(unit, port);

    if ((WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) && (speed == 40000) &&
        (!DEV_CFG_PTR(pc)->hg_mode)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, 0x00, (1U << 4),
                        ((1U << 4)|(1U << 1))));
                sal_usleep(100);
                SOC_IF_ERROR_RETURN
                    (WC40_REG_MODIFY(unit, pc, 0x00, 0x00008420, 0x0, 0x3));
                SOC_IF_ERROR_RETURN
                    (WC40_REG_MODIFY(unit, pc, 0x00, 0x00008020, (1U << 2), (1U << 2)));
                SOC_IF_ERROR_RETURN
                    (WC40_REG_MODIFY(unit, pc, 0x00, 0x00008020, 0, (1U << 2)));
                sal_usleep(100);
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, 0x00, 0x0, (1U << 4)));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_custom_speed_set(int unit, soc_port_t port, int speed)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_speed_set
 * Purpose:
 *      Set PHY speed
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      speed - link speed in Mbps
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t  *pc;
    int rv;

    pc = INT_PHY_SW_STATE(unit, port);

    if (WC40_LINK_WAR_REVS(pc)) {
        WC40_SP_VALID_RESET(pc);
    }

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        rv = _phy_wc40_speed_set(unit,port,speed);
        if (WC40_LINK_WAR_REVS(pc)) {
            if (speed >=25000) {  /* BRCM 64B/66B mode */
                WC40_SP_VALID_SET(pc);
            }
        }
    } else {
        if (DEV_CFG_PTR(pc)->custom) {
            rv = _phy_wc40_custom_speed_set(unit, port, speed);
        } else {
            rv = _phy_wc40_ind_speed_set(unit,port,speed);
            if (WC40_LINK_WAR_REVS(pc)) {
                if (speed >=12000) {  /* BRCM 64B/66B mode */
                    WC40_SP_VALID_SET(pc);
                }
            }
        }
    }
    return rv;
}

STATIC int
_phy_wc40_tx_fifo_reset(int unit, soc_port_t port,uint32 speed)
{
    uint16 data16;
    phy_ctrl_t  *pc;
    pc = INT_PHY_SW_STATE(unit, port);

    if (speed == 100) {
        /* check if it is in 100fx mode */
        SOC_IF_ERROR_RETURN
            (READ_WC40_FX100_CONTROL1r(unit,pc,0x00,&data16));

        if (data16 & FX100_CONTROL1_ENABLE_MASK) {
            /* reset TX FIFO  */
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,LANE0_ACCESS,0x8061 + (0x10 * pc->lane_num),
                                 TX0_ANATXACONTROL0_TX1G_FIFO_RST_MASK,
                                 TX0_ANATXACONTROL0_TX1G_FIFO_RST_MASK));

            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,LANE0_ACCESS,0x8061 + (0x10 * pc->lane_num),
                              0,
                              TX0_ANATXACONTROL0_TX1G_FIFO_RST_MASK));
        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_speed_mode_decode(int speed_mode, int *speed, int *intf, int *scr)
{
    *scr  = FALSE;
    *intf = SOC_PORT_IF_XGMII;  /* default to XGMII */
    switch (speed_mode) {
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10M:
            *speed = 10;
            *intf = SOC_PORT_IF_MII;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_100M:
            *speed = 100;
            *intf = SOC_PORT_IF_MII;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_1G:
            *speed = 1000;
            *intf = SOC_PORT_IF_GMII;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_2p5G:
            *speed = 2500;
            *intf = SOC_PORT_IF_GMII;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_5G_X4:
            *speed = 5000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_6G_X4:
            *speed = 6000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_HiG          :
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_CX4          :
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_12G_HiG          :
            *speed = 12000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_12p5G_X4         :
            *speed = 12500;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_13G_X4           :
            *speed = 13000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_15G_X4           :
            *speed = 15000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_16G_X4           :
            *speed = 16000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_1G_KX            :
            *speed = 1000;
            *intf = SOC_PORT_IF_GMII;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_KX4          :
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_KR           :
            *speed = 10000;
            *intf = SOC_PORT_IF_KR;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_5G               :
            *speed = 5000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_6p4G             :
            *speed = 6000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_X4           :
            *speed = 20000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_KR2           :
            *speed = 20000;
            *intf = SOC_PORT_IF_KR;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_CR2           :
            *speed = 20000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_21G_X4           :
            *speed = 21000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_25G_X4           :
            *speed = 25000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_HiG_DXGXS    :
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_DXGXS        :
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10p5G_HiG_DXGXS  :
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10p5G_DXGXS      :
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_12p773G_HiG_DXGXS:
            *speed = 12000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_12p773G_DXGXS    :
            *speed = 12000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_XFI          :
            *speed = 10000;
            *intf = SOC_PORT_IF_XFI;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_40G              :
            *speed = 40000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_HiG_DXGXS    :
            *speed = 20000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_DXGXS        :
            *speed = 20000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_SFI          :
            *speed = 10000;
            *intf = SOC_PORT_IF_SFI;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_31p5G            :
            *speed = 30000;  /* MAC has only 30G speed*/
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_32p7G            :
            *speed = 32000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_20G_SCR          :
            *scr = TRUE;
            *speed = 20000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_HiG_DXGXS_SCR:
            *scr = TRUE;
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_DXGXS_SCR    :
            *scr = TRUE;
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_12G_R2           :
            *speed = 12000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_10G_X2           :
            *speed = 10000;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_40G_KR4          :
            *speed = 40000;
            *intf = SOC_PORT_IF_KR4;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_40G_CR4          :
            *speed = 40000;
            *intf = SOC_PORT_IF_CR4;
            break;
        case XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_dr_100G_CR10        :
            *speed = 100000;
            *intf = SOC_PORT_IF_XLAUI;
            break;
        case 0x2c:
            *speed = 15000;
            break;
        default:
            break;
    }

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_ind_speed_get(int unit, soc_port_t port, int *speed, int *intf, int *scr)
{
    phy_ctrl_t *pc;
    uint16 speed_mode;
    int rv;

    pc = INT_PHY_SW_STATE(unit, port);

    *speed = 0;
    if (pc->lane_num < 2) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_GP2_REG_GP2_2r(unit,pc,0x00,&speed_mode));
        if (pc->lane_num == 0) {
            speed_mode &= GP2_REG_GP2_2_ACTUAL_SPEED_LN0_MASK;
        } else {
            speed_mode &= GP2_REG_GP2_2_ACTUAL_SPEED_LN1_MASK;
            speed_mode >>= GP2_REG_GP2_2_ACTUAL_SPEED_LN1_SHIFT;
        } 
    } else {
        SOC_IF_ERROR_RETURN
            (READ_WC40_GP2_REG_GP2_3r(unit,pc,0x00,&speed_mode));
        if (pc->lane_num == 2) {
            speed_mode &= GP2_REG_GP2_3_ACTUAL_SPEED_LN2_MASK;
        } else {
            speed_mode &= GP2_REG_GP2_3_ACTUAL_SPEED_LN3_MASK;
            speed_mode >>= GP2_REG_GP2_3_ACTUAL_SPEED_LN3_SHIFT;
        } 
    }
    rv = _phy_wc40_speed_mode_decode(speed_mode, speed,intf,scr);
    SOC_DEBUG_PRINT((DK_PHY,
        "phy_wc40_ind_speed_get: u=%d p=%d GP2_2/3_SPEEDr %04x speed= %d lane=%d\n",
         unit, port,speed_mode, *speed,pc->lane_num));
    return rv;
}

STATIC int
_phy_wc40_combo_speed_get(int unit, soc_port_t port, int *speed, int *intf, int *scr)
{
    phy_ctrl_t *pc;
    uint16 speed_mode;
    uint16 data16;
    int rv;

    pc = INT_PHY_SW_STATE(unit, port);
    
    /* XXX should use 0x81d2 same as the ind_speed_get */
    *speed = 0;
    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK5_XGXSSTATUS4r(unit,pc,0x00,&speed_mode));
    speed_mode &= XGXSBLK5_XGXSSTATUS4_ACTUAL_SPEED_LN0_MASK;
    rv = _phy_wc40_speed_mode_decode(speed_mode, speed,intf,scr);
    if (*speed == 40000) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00,&data16));
        if ((data16 & (SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                      SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK)) ==
                   (SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                   (0xc << SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SHIFT)) ) {
            *speed = 42000;
        } 
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_wc40_combo_speed_get: u=%d p=%d XGXSSTATUS4_SPEEDr %04x speed= %d\n",
         unit, port,speed_mode, *speed));
    return rv;
}

STATIC int
_phy_wc40_custom_speed_get(int unit, soc_port_t port, int *speed)
{
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_speed_get
 * Purpose:
 *      Get PHY speed
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      speed - current link speed in Mbps
 * Returns:     
 *      SOC_E_NONE
 */
STATIC int
phy_wc40_speed_get(int unit, soc_port_t port, int *speed)
{
    phy_ctrl_t *pc;
    int rv;
    int intf;
    int scr;

    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) { 
        rv = _phy_wc40_combo_speed_get(unit, port, speed,&intf,&scr);
    } else {
        if (DEV_CFG_PTR(pc)->custom) {
            rv = _phy_wc40_custom_speed_get(unit, port, speed);
        } else {
            rv = _phy_wc40_ind_speed_get(unit, port, speed,&intf,&scr);
        }
    }
    return rv;
}

/*
 * software autoneg
 */
STATIC int
_wc40_soft_an_cl73kr2(int unit, soc_port_t port, int an)
{
    phy_ctrl_t       *pc;
    int lane_start;
    int lane_end;
    int lane;
    uint16 data16;
    uint16 mask16;

    pc = INT_PHY_SW_STATE(unit, port);

    /* disable 1000X parallel detect */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00,
          0,
          SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK));

    /* disable 10G parallel detect */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK5_PARDET10GCONTROLr(unit, pc, 0x00,
              0,
              XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK));

    /* diable CL37 BAM */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_DIGITAL6_MP5_NEXTPAGECTRLr(unit,pc,0x00,0));

    /* disable CL37 */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, 0,
                                         MII_CTRL_AE));

    /* diable the CL73 BAM */
    /* need to enable these two bits even BAM is disabled to interop
     * with other BRCM devices properly.
     */
    data16 = CL73_USERB0_CL73_BAMCTRL1_CL73_BAM_STATION_MNGR_EN_MASK |
                 CL73_USERB0_CL73_BAMCTRL1_CL73_BAMNP_AFTER_BP_EN_MASK;
    mask16 = data16;
    mask16 |= CL73_USERB0_CL73_BAMCTRL1_CL73_BAMEN_MASK;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_CL73_USERB0_CL73_BAMCTRL1r(unit, pc, 0x00,
                   data16, mask16));


    /* Set cl73_suppress_mr_page_rx_dis: 0x8378[15:0]=0x100 */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_CL73_USERB0_CL73_UCTRL2r(unit, pc, 0x00, an? 0x100: 0,0x100));

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        lane_start = 0;
        lane_end   = 3;
    } else {
        lane_start = pc->lane_num;
        lane_end   = lane_start + 1;
    }

    
    /* Configure HP Alignment Markers Encodings: set cl82_20g_AM_PCSL0 = 24'hE2_A1_57
     * and cl82_20g_AM_PCSL1 = 24'hCB_75_37");
     */
    for (lane = lane_start; lane <= lane_end; lane++) { 
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x8435, an? 0xA157: 0));
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x8436, an? 0x7537: 0));
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x8437, an? 0xCBE2: 0));
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x8439, an? 0xA157: 0));
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x843a, an? 0x7537: 0));
        SOC_IF_ERROR_RETURN
            (WC40_REG_WRITE(unit,pc,ln_access[lane], 0x843b, an? 0xCBE2: 0));
    }

    if (an) {
        /* Write LD BP to advertise KR2, KR, KX4, and KX */
        /* configure CL73 adv2 page with KR2 */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT2r(unit,pc,0x0, 
                  CL73_AN_ADV_TECH_20G_KR2,CL73_AN_ADV_TECH_20G_KR2));

        /* CL73 adv1 page configuration, from advert_set  */

        /* configure CL73 adv0 page: add next page, pause from advert_set */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT0r(unit,pc,0x0, 0x8001,0x8001));
    }

    /* BAM73 disabled and NP is set, in this mode, the autoneg will not continue until
     * a null page is sent
     */
    /* restart AN */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit,pc,0x0, an? MII_CTRL_AE | MII_CTRL_RAN: 0));
    return SOC_E_NONE;
}
STATIC int
_wc40_soft_an_cl73kr2_check(int unit, soc_port_t port)
{
    uint16 data16;
    uint16 lp_adv;
    uint16 ld_adv;
    uint16 lp_kr2;
    uint16 ld_kr2;
    phy_ctrl_t       *pc;

    pc = INT_PHY_SW_STATE(unit, port);
 
    /* Waiting for cl73_mr_page_rx, wait for 0x1[6]=1, from linkscan thread */
    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE0BLK_AN_IEEESTATUS1r(unit,pc,0x0, &data16));
    if (data16 & (1 << 6)) {
        SOC_DEBUG_PRINT((DK_VERBOSE,
                         "KR2 autoneg wait page_rx bit set: u=%d p=%d\n", unit, port));

        /* check the matched ability */
        SOC_IF_ERROR_RETURN
            (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY1r(unit, pc, 0x00, &lp_adv));
        SOC_IF_ERROR_RETURN
            (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY2r(unit, pc, 0x00, &lp_kr2));
        lp_kr2 &= CL73_AN_ADV_TECH_20G_KR2;
        lp_adv |= lp_kr2;

        SOC_IF_ERROR_RETURN
            (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT1r(unit, pc, 0x00, &ld_adv));
        SOC_IF_ERROR_RETURN
            (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT2r(unit, pc, 0x00, &ld_kr2));
        ld_kr2 &= CL73_AN_ADV_TECH_20G_KR2;
        ld_adv |= ld_kr2;

        /* set 0x8450.0 */
        if ((ld_adv & lp_adv & CL73_AN_ADV_PLUS_KR2_MASK) == CL73_AN_ADV_TECH_20G_KR2) {

            /* workaround for KR2 speed status, disable PCS type auto selection */
            if (WC40_REVID_B0(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, 
                             0, (1 << 15)));
            }
        
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,0x8450, 1, 1));
            SOC_DEBUG_PRINT((DK_VERBOSE,
                         "KR2 autoneg resolves to KR2: u=%d p=%d\n", unit, port));
        } else if ((ld_adv & lp_adv & CL73_AN_ADV_PLUS_KR2_MASK) == 
               (CL73_AN_ADV_TECH_40G_KR4 | CL73_AN_ADV_TECH_20G_KR2)) {
            SOC_DEBUG_PRINT((DK_VERBOSE,
                         "KR2 autoneg both KR2/KR4 received: u=%d p=%d\n", unit, port));
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,0x8450, 1, 1));
        } else {
            SOC_DEBUG_PRINT((DK_VERBOSE,
                         "KR2 autoneg resolves to non KR2 speed: u=%d p=%d\n", unit, port));
        }

        /* completes the autoneg by send out a null page with NP=0 */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AN_IEEE1BLK_AN_XNP_TRANSMIT0r(unit, pc, 0x00, 0x2001));
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AN_IEEE1BLK_AN_XNP_TRANSMIT1r(unit, pc, 0x00, 0));
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_AN_IEEE1BLK_AN_XNP_TRANSMIT2r(unit, pc, 0x00, 0));
    }
    return SOC_E_NONE;
}

/*
 * Function:    
 *      phy_wc40_an_set
 * Purpose:     
 *      Enable or disabled auto-negotiation on the specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      an   - Boolean, if true, auto-negotiation is enabled 
 *              (and/or restarted). If false, autonegotiation is disabled.
 * Returns:     
 *      SOC_E_XXX
 */
int
phy_wc40_an_set(int unit, soc_port_t port, int an)
{
    phy_ctrl_t  *pc;
    uint16             an_enable;
    uint16             auto_det;
    uint16             mask16;
    uint16             data16;
    WC40_TX_DRIVE_t tx_drv[NUM_LANES];

    pc = INT_PHY_SW_STATE(unit, port);

    if (AUTONEG_MODE_UNAVAIL(pc)) {
        return SOC_E_NONE;
    }

    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_wc40_an_set: u=%d p=%d an=%d\n",
                     unit, port, an));

    an_enable = 0;
    auto_det  = 0;

    /* enable cl72 device in autoneg mode and disable it in forced mode */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_cl72_enable(unit,port,an));

    if (an) {
        /* disable the cl82 local fault. Needed to allow device link up at 40G KR4
         * when XMAC interface is still in XGMII mode. Once WC indicates 40G KR4 linkup,
         * the port update function will set the XMAC in XLGMII mode.
         */
        SOC_IF_ERROR_RETURN
            (WC40_REG_MODIFY(unit, pc, 0x00, 0x842A, (1 << 5),(1 << 5)));
        WC40_AN_VALID_SET(pc);
        WC40_AN_RECORD_TIME(pc);

        /* set to the default */ 
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, 0x0));

        if (WC40_LINK_WAR_REVS(pc)) {
            WC40_SP_VALID_RESET(pc);
        }

        /* part of workaround for KR2 speed status, enable PCS type auto selection */
        if (IS_DUAL_LANE_PORT(pc)) {
            if (WC40_REVID_B0(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DIGITAL4_MISC4r(unit, pc, 0x00, 
                             (1 << 15), (1 << 15)));
            }
        }

        /* configure the TX driver parameters for autoneg mode, use AN entry  */        
        SOC_IF_ERROR_RETURN
            (_phy_wc40_tx_control_get(unit, port,&tx_drv[0],TXDRV_AN_INX));
        SOC_IF_ERROR_RETURN
            (_phy_wc40_tx_control_set(unit, port,&tx_drv[0]));

        an_enable = MII_CTRL_AE | MII_CTRL_RAN;

        /*
         * Should read one during init and cache it in Phy flags
         */
        if (DEV_CFG_PTR(pc)->auto_medium) {
            auto_det = SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK; 
        }

        /* check if need to switch to a different VCO */
        SOC_IF_ERROR_RETURN
            (_phy_wc40_vco_set(unit,port,0,0));

        /* stop rxseq and txfifo, need to properly configure for 1G speeds
         * before doing AN. 
         */
        DUAL_LANE_BCST_ENABLE(pc);

        /* hold tx/rx in reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
                  DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK,
                  DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));

        /* If auto-neg is enabled, make sure not forcing any speed */

        /* Workaround Jira# SDK-32387 */
        /* control the speeds thru pma/pmd register */
        if (WC40_SDK32387_REVS(pc) && (!IS_DUAL_LANE_PORT(pc))) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK,
                           SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));
        }

        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, 0, 
                                   SERDESDIGITAL_MISC1_FORCE_SPEED_MASK));
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00, DIGITAL4_MISC3_IND_40BITIF_MASK, 
                           DIGITAL4_MISC3_IND_40BITIF_MASK | DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));

        /* Workaround Jira# SDK-32387 */
        /* control the speeds thru pma/pmd register */
        if (WC40_SDK32387_REVS(pc) && (!IS_DUAL_LANE_PORT(pc))) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                               0,
                               SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));
        }

        /* release the tx/tx reset */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00,
                  0,
                  DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK));



        DUAL_LANE_BCST_DISABLE(pc);

        /* Enable/Disable auto detect */
        SOC_IF_ERROR_RETURN
             (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit, pc, 0x00, auto_det,
                                 SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK));

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) { 

            /* clear 2wire XAUI configuration */
            SOC_IF_ERROR_RETURN
                (_wc40_rxaui_config(unit,pc,FALSE));

            /* set default clock comp clear the configuration for 20X XE combo mode */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 
                      XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_XGXS_nLQnCC <<
                      XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_SHIFT,
                      XGXSBLK2_UNICOREMODE10G_UNICOREMODE10GHIG_MASK));

            if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
                /* clear 42G control, PLL auto tune */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00,
                           0, 
                           SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_SEL_MASK |
                           SERDESDIGITAL_MISC1_FORCE_PLL_MODE_AFE_MASK));
            }

            /* only in combo mode, reset the sequence. In independent mode,
             * resetting sequence affects all lanes
             */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 0,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));

            /* Enable 1000X parallel detect */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, 
                  SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK, 
                  SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK));

            /* enable 10G parallel detect */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK5_PARDET10GCONTROLr(unit, pc, 0x00,
                      XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK, 
                      XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK));
        }
        if (DEV_CFG_PTR(pc)->cl73an == WC40_CL73_KR2) {
            SOC_IF_ERROR_RETURN
                (_wc40_soft_an_cl73kr2(unit,port,an));
        } else {
            /* set BAM/TETON enable */
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_DIGITAL6_MP5_NEXTPAGECTRLr(unit,pc,0x00,
                   DIGITAL6_MP5_NEXTPAGECTRL_BAM_MODE_MASK |
                   DIGITAL6_MP5_NEXTPAGECTRL_TETON_MODE_MASK |
                   DIGITAL6_MP5_NEXTPAGECTRL_TETON_MODE_UP3_EN_MASK));

            /* enable autoneg */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, MII_CTRL_AE,
                                          MII_CTRL_AE));
            if (DEV_CFG_PTR(pc)->cl73an) {
                /* CL73 BAM setup, revisit BAMNP_AFTER_BP bit, any conflict
                 * with EEE page?
                 */
                mask16 = CL73_USERB0_CL73_BAMCTRL1_CL73_BAMEN_MASK |
                     CL73_USERB0_CL73_BAMCTRL1_CL73_BAM_STATION_MNGR_EN_MASK |
                     CL73_USERB0_CL73_BAMCTRL1_CL73_BAMNP_AFTER_BP_EN_MASK;
                /* not to enable CL73 BAM in independent channel mode:
                 * has problem with 8073 phy
                 */
                if (DEV_CFG_PTR(pc)->cl73an == WC40_CL73_WO_CL73BAM) {
                    /* need to enable these two bits even BAM is disabled to interop
                     * with other BRCM devices properly. 
                     */
                    data16 = CL73_USERB0_CL73_BAMCTRL1_CL73_BAM_STATION_MNGR_EN_MASK |
                             CL73_USERB0_CL73_BAMCTRL1_CL73_BAMNP_AFTER_BP_EN_MASK;
                } else {
                    data16 = mask16;
                }

                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_CL73_USERB0_CL73_BAMCTRL1r(unit, pc, 0x00, 
                           data16, mask16));

                /* XXX removed? */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, 
                                          MII_CTRL_AE,
                                          MII_CTRL_AE));
            }
        }
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) { 
            /* restart the sequence */
            SOC_IF_ERROR_RETURN
                 (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));
            /* wait for PLL lock */
            _phy_wc40_pll_lock_wait(unit, port);
        }
    } else {
        /* Disable auto detect */
        SOC_IF_ERROR_RETURN
             (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit, pc, 0x00, auto_det,
                                 SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK));

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 0,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));

            /* disable 1000X parallel detect */
            SOC_IF_ERROR_RETURN
                  (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00,
                  0,
                  SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK));
                                                                                
            /* disable 10G parallel detect */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK5_PARDET10GCONTROLr(unit, pc, 0x00,
                      0,
                      XGXSBLK5_PARDET10GCONTROL_PARDET10G_EN_MASK));
        }

        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DIGITAL6_MP5_NEXTPAGECTRLr(unit,pc,0x00,0));

        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, 0,
                                         MII_CTRL_AE));
        if (DEV_CFG_PTR(pc)->cl73an) { 
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, 
                                          0,
                                          MII_CTRL_AE));
        }

        if (DEV_CFG_PTR(pc)->cl73an == WC40_CL73_KR2) {
            SOC_IF_ERROR_RETURN
                (_wc40_soft_an_cl73kr2(unit,port,an));
        }

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            SOC_IF_ERROR_RETURN
                 (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK,
                              XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));
            /* wait for PLL lock */
            _phy_wc40_pll_lock_wait(unit, port);
        }
    }

    if (DEV_CFG_PTR(pc)->cl73an != WC40_CL73_KR2) {
        /* restart the autoneg if enabled, or disable the autoneg */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, an_enable,
                                          MII_CTRL_AE | MII_CTRL_RAN));
    }

    if (DEV_CFG_PTR(pc)->cl73an) { 
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, an_enable,
                                      MII_CTRL_AE | MII_CTRL_RAN));
    }

    pc->fiber.autoneg_enable = an;

    return SOC_E_NONE;
}

/*
 * Function:    
 *      phy_wc40_an_get
 * Purpose:     
 *      Get the current auto-negotiation status (enabled/busy)
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      an   - (OUT) if true, auto-negotiation is enabled.
 *      an_done - (OUT) if true, auto-negotiation is complete. This
 *              value is undefined if an == false.
 * Returns:     
 *      SOC_E_XXX
 */

STATIC int
phy_wc40_an_get(int unit, soc_port_t port, int *an, int *an_done)
{
    uint16      mii_ctrl;
    uint16      mii_stat;
    phy_ctrl_t *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (AUTONEG_MODE_UNAVAIL(pc)) {
        *an = FALSE;
        *an_done = FALSE;
        return SOC_E_NONE;
    }

    if (DEV_CFG_PTR(pc)->cl73an == WC40_CL73_KR2) {
        *an = FALSE;
        *an_done = FALSE;
    } else {        
        SOC_IF_ERROR_RETURN
            (READ_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, &mii_ctrl));
        SOC_IF_ERROR_RETURN
            (READ_WC40_COMBO_IEEE0_MIISTATr(unit, pc, 0x00, &mii_stat));

        *an = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
        *an_done = (mii_stat & MII_STAT_AN_DONE) ? TRUE : FALSE;
    }

    if (!((*an == TRUE) && (*an_done == TRUE))) {
        if (DEV_CFG_PTR(pc)->cl73an) { 
            /* check clause 73 */
            SOC_IF_ERROR_RETURN
                (READ_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, &mii_ctrl));
            SOC_IF_ERROR_RETURN
                (READ_WC40_AN_IEEE0BLK_AN_IEEESTATUS1r(unit, pc, 0x00, &mii_stat));

            *an = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
            *an_done = (mii_stat & MII_STAT_AN_DONE) ? TRUE : FALSE;
        }
    }
    return SOC_E_NONE; 
}

STATIC int
_phy_wc40_c73_adv_local_set(int unit, soc_port_t port,
                            soc_port_ability_t *ability)
{
    uint16            an_adv;
    uint16            pause;
#ifdef WC_EEE_SUPPORT
    uint16            mask16;
    uint16            data16;
#endif
    phy_ctrl_t       *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    an_adv = (ability->speed_full_duplex & SOC_PA_SPEED_1000MB) ? 
              CL73_AN_ADV_TECH_1G_KX : 0;

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_10GB) ? 
               CL73_AN_ADV_TECH_10G_KR: 0;
    }

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_10GB) ? 
              CL73_AN_ADV_TECH_10G_KX4: 0;
    }    

    an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_40GB) ?
          CL73_AN_ADV_TECH_40G_KR4: 0;
    if (DEV_CFG_PTR(pc)->line_intf & WC40_IF_CR4) {
        an_adv |= (ability->speed_full_duplex & SOC_PA_SPEED_40GB) ?
              CL73_AN_ADV_TECH_40G_CR4: 0;
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT1r(unit, pc, 0x00, an_adv,
                                     CL73_AN_ADV_TECH_SPEEDS_MASK));

#ifdef WC_EEE_SUPPORT
    /* EEE advertisement 0x3c: 
     * 06 RW EEE_10G_KR 1 = EEE is supported for 10GBASE-KR.
     *                  0 = EEE is not supported for 10GBASE-KR. 
     * 05 RW EEE_10G_KX4 1 = EEE is supported for 10GBASE-KX4.
     *                   0 = EEE is not supported for 10GBASE-KX4. 
     * 04 RW EEE_1G_KX 1 = EEE is supported for 1000BASE-KX.
     *                 0 = EEE is not supported for 1000BASE-KX.
     * XXX assume supporting all speeds. use PA_EEE 
     */
    mask16 = AN_IEEE3BLK_EEE_ADV_EEE_10G_KR_MASK |
             AN_IEEE3BLK_EEE_ADV_EEE_10G_KX4_MASK |
             AN_IEEE3BLK_EEE_ADV_EEE_1G_KX_MASK;
    data16 = (ability->eee & SOC_PA_EEE_10GB_KX) ? 
              AN_IEEE3BLK_EEE_ADV_EEE_1G_KX_MASK : 0;
    data16 |= (ability->eee & SOC_PA_EEE_10GB_KX4) ? 
              AN_IEEE3BLK_EEE_ADV_EEE_10G_KX4_MASK:0;
    data16 |= (ability->eee & SOC_PA_EEE_10GB_KR) ? 
              AN_IEEE3BLK_EEE_ADV_EEE_10G_KR_MASK: 0;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_AN_IEEE3BLK_EEE_ADVr(unit, pc, 0x00, data16, mask16));
#endif

    /* CL73 UD_code_field */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_CL73_USERB0_CL73_BAMCTRL3r(unit,pc,0x00,1,
                CL73_USERB0_CL73_BAMCTRL3_UD_CODE_FIELD_MASK));

    switch (ability->pause & (SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX)) {
    case SOC_PA_PAUSE_TX:
        pause = CL73_AN_ADV_ASYM_PAUSE;
        break;
    case SOC_PA_PAUSE_RX:
        pause = CL73_AN_ADV_PAUSE | CL73_AN_ADV_ASYM_PAUSE;
        break;
    case SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX:
        pause = CL73_AN_ADV_PAUSE;
        break;
    default:
        pause = 0;
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT0r(unit, pc, 0x00, pause,
                                     (CL73_AN_ADV_PAUSE |
                                      CL73_AN_ADV_ASYM_PAUSE)));
    
    SOC_DEBUG_PRINT((DK_PHY,
        "_phy_wc40_c73_adv_local_set: u=%d p=%d pause=%08x speeds=%04x,adv=0x%x\n",
        unit, port, pause, an_adv,ability->speed_full_duplex));
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_c73_adv_local_get(int unit, soc_port_t port,
                            soc_port_ability_t *ability)
{
    uint16            an_adv;
    soc_port_mode_t   speeds,pause;
    phy_ctrl_t       *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT1r(unit, pc, 0x00, &an_adv));

    speeds = (an_adv & CL73_AN_ADV_TECH_1G_KX) ? SOC_PA_SPEED_1000MB : 0;
    speeds |= (an_adv & CL73_AN_ADV_TECH_10G_KX4) ? SOC_PA_SPEED_10GB : 0;
    speeds |= (an_adv & CL73_AN_ADV_TECH_10G_KR) ? SOC_PA_SPEED_10GB : 0;
    speeds |= (an_adv & CL73_AN_ADV_TECH_40G_KR4) ? SOC_PA_SPEED_40GB : 0;
    speeds |= (an_adv & CL73_AN_ADV_TECH_40G_CR4) ? SOC_PA_SPEED_40GB : 0;

    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT2r(unit, pc, 0x00, &an_adv));
    speeds |= (an_adv & CL73_AN_ADV_TECH_20G_KR2) ? SOC_PA_SPEED_20GB : 0;

    ability->speed_full_duplex |= speeds;
#ifdef WC_EEE_SUPPORT
    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE3BLK_EEE_ADVr(unit, pc, 0x00, &an_adv));
    ability->eee = (an_adv & AN_IEEE3BLK_EEE_ADV_EEE_10G_KR_MASK)?
                   SOC_PA_EEE_10GB_KR:0;
    ability->eee |= (an_adv & AN_IEEE3BLK_EEE_ADV_EEE_10G_KX4_MASK)?
                    SOC_PA_EEE_10GB_KX4:0;
    ability->eee |= (an_adv & AN_IEEE3BLK_EEE_ADV_EEE_1G_KX_MASK)?
                    SOC_PA_EEE_10GB_KX:0;
#endif

    /* advert register 0x12, bit 15 FEC requested,bit 14 FEC ability */
    /* SOC_IF_ERROR_RETURN
     *    (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT2r(unit, pc, 0x00, &an_adv));
     */
    /* (an_adv & AN_IEEE1BLK_AN_ADVERTISEMENT2_FEC_REQUESTED_MASK) */

    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT0r(unit, pc, 0x00, &an_adv));

    switch (an_adv & (CL73_AN_ADV_PAUSE | CL73_AN_ADV_ASYM_PAUSE)) {
        case CL73_AN_ADV_PAUSE:
            pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case CL73_AN_ADV_ASYM_PAUSE:
            pause = SOC_PA_PAUSE_TX;
            break;
        case CL73_AN_ADV_PAUSE | CL73_AN_ADV_ASYM_PAUSE:
            pause = SOC_PA_PAUSE_RX;
            break;
        default:
            pause = 0;
    }
    ability->pause = pause;

    SOC_DEBUG_PRINT((DK_PHY,
        "_phy_wc40_c73_adv_local_get: u=%d p=%d pause=%08x speeds=%04x\n",
        unit, port, pause, speeds));
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_ability_advert_set
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_wc40_ability_advert_set(int unit, soc_port_t port,
                       soc_port_ability_t *ability)
{
    uint16           an_adv;
    uint16           an_sp_20g;
    soc_port_mode_t  mode;
    phy_ctrl_t      *pc;
    uint16 mask16;
    uint16 data16;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = INT_PHY_SW_STATE(unit, port);

    /*
     * Set advertised duplex (only FD supported).
     */
    an_adv = ability->speed_full_duplex? MII_ANA_C37_FD : 0;

    /*
     * Set advertised pause bits in link code word.
     */
    switch (ability->pause & (SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX)) {
    case SOC_PA_PAUSE_TX:
        an_adv |= MII_ANA_C37_ASYM_PAUSE;
        break;
    case SOC_PA_PAUSE_RX:
        an_adv |= MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE;
        break;
    case SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX:
        an_adv |= MII_ANA_C37_PAUSE;
        break;
    }
    /* Update less than 1G capability */ 
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_COMBO_IEEE0_AUTONEGADVr(unit, pc, 0x00, an_adv));

    mode = ability->speed_full_duplex;
    an_adv = 0;
    an_sp_20g = 0;
    an_adv |= (mode & SOC_PA_SPEED_2500MB) ? 
               DIGITAL3_UP1_DATARATE_2P5GX1_MASK : 0;

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DIGITAL3_UP1r(unit, pc, 0x00, an_adv));

        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DIGITAL3_UP3r(unit, pc, 0x00, 
                          DIGITAL3_UP3_CL72_MASK | DIGITAL3_UP3_LAST_MASK)); 

        if (DEV_CFG_PTR(pc)->cl73an) { 
            SOC_IF_ERROR_RETURN
                (_phy_wc40_c73_adv_local_set(unit, port, ability));
        }

        SOC_DEBUG_PRINT((DK_PHY,
            "phy_wc40_ability_advert_set: u=%d p=%d pause=%08x OVER1G_UP1 %04x\n",
            unit, port, ability->pause, an_adv));
        return SOC_E_NONE;
    }

    /* high speeds, 40G,30G and 20G, 20G/scrambler */ 
    an_adv |= (mode & SOC_PA_SPEED_5000MB) ? 
                DIGITAL3_UP1_DATARATE_5GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_6000MB) ? 
                DIGITAL3_UP1_DATARATE_6GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_12GB) ? 
                DIGITAL3_UP1_DATARATE_12GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_12P5GB) ? 
                DIGITAL3_UP1_DATARATE_12P5GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_13GB) ? 
                    DIGITAL3_UP1_DATARATE_13GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_15GB) ? 
                DIGITAL3_UP1_DATARATE_15GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_16GB) ? 
                DIGITAL3_UP1_DATARATE_16GX4_MASK : 0;
    an_adv |= (mode & SOC_PA_SPEED_20GB) ? 
              DIGITAL3_UP1_DATARATE_20GX4_MASK : 0;
    /* advertise 20G with scrambler */
    if (mode & SOC_PA_SPEED_20GB) {
        data16 = DIGITAL6_UP4_DATARATE_20G_MASK;
    } else {
        data16 = 0;
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL6_UP4r(unit, pc, 0x00, 
                 data16,
                 DIGITAL6_UP4_DATARATE_20G_MASK));
    
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL6_UP4r(unit, pc, 0x00, 
                 DIGITAL6_UP4_LAST_MASK,
                 DIGITAL6_UP4_LAST_MASK));

    an_sp_20g |= (mode & SOC_PA_SPEED_21GB) ? 
                  DIGITAL3_UP3_DATARATE_21GX4_MASK : 0;
    an_sp_20g |= (mode & SOC_PA_SPEED_25GB) ? 
                  DIGITAL3_UP3_DATARATE_25P45GX4_MASK : 0;
    an_sp_20g |= (mode & SOC_PA_SPEED_30GB) ? 
                  DIGITAL3_UP3_DATARATE_31P5G_MASK : 0;

    if (DEV_CFG_PTR(pc)->hg_mode) {
        an_sp_20g |= (mode & SOC_PA_SPEED_40GB) ? 
                  DIGITAL3_UP3_DATARATE_40G_MASK : 0;
    }

    if (mode & SOC_PA_SPEED_10GB) {
        if (DEV_CFG_PTR(pc)->hg_mode) {
            /* For Draco and Hercules, use pre-CX4 signalling */
            an_adv |= DIGITAL3_UP1_DATARATE_10GX4_MASK; 
            if (DEV_CFG_PTR(pc)->cx4_10g) {
                /* Also include 10G CX4 signalling by default */
                an_adv |= DIGITAL3_UP1_DATARATE_10GCX4_MASK; 
            }
        } else {
            an_adv |= DIGITAL3_UP1_DATARATE_10GCX4_MASK; 
        }
    }
    
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_DIGITAL3_UP1r(unit, pc, 0x00, an_adv));

    /* CL72, FEC and Higig2, check Higig2 cap on this port before adv 
     * UP3 last page bit DIGITAL3_UP3_LAST_MASK needs to be set to work with HC/HL.
     */
    mask16 = OVER1G_UP3_20GPLUS_MASK;
    data16 = an_sp_20g;
    mask16 |= DIGITAL3_UP3_CL72_MASK | DIGITAL3_UP3_FEC_MASK |
             DIGITAL3_UP3_HIGIG2_MASK;

    /* 20G with CL72 advertised in WC B0 doesn't work. But cl72 should not
     * be advertised below 21G anyway
     */ 
    if (!(DEV_CFG_PTR(pc)->hg_mode &&
        (!(mode & (SOC_PA_SPEED_40GB | SOC_PA_SPEED_21GB |
                 SOC_PA_SPEED_25GB | SOC_PA_SPEED_30GB)))) ) {
        data16 |= DIGITAL3_UP3_CL72_MASK;
    }
#if 0 
    data16 |= DIGITAL3_UP3_FEC_MASK;
#endif

    /* XXX revisit: no autoneg in dxgxs mode */
    
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL3_UP3r(unit, pc, 0x00, data16, mask16));

    /* preemphasis attributes */
    /*
     * mask16 = DIGITAL3_UP2_PREEMPHASIS_MASK | DIGITAL3_UP2_IDRIVER_MASK |
     *          DIGITAL3_UP2_IPREDRIVER_MASK | DIGITAL3_UP2_VALID_MASK;
     * data16 = (DEV_CFG_PTR(pc)->preemph << DIGITAL3_UP2_PREEMPHASIS_SHIFT) |
     *          (DEV_CFG_PTR(pc)->idriver << DIGITAL3_UP2_IDRIVER_SHIFT) |
     *          (DEV_CFG_PTR(pc)->pdriver << DIGITAL3_UP2_IPREDRIVER_SHIFT);
     * data16 |= DIGITAL3_UP2_VALID_MASK;
     * SOC_IF_ERROR_RETURN
     *     (MODIFY_WC40_DIGITAL3_UP2r(unit, pc, 0x00, data16,mask16));
     */

    if (DEV_CFG_PTR(pc)->cl73an) { 
        SOC_IF_ERROR_RETURN
            (_phy_wc40_c73_adv_local_set(unit, port, ability));
    }

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_wc40_ability_advert_set: u=%d p=%d pause=%08x OVER1G_UP1 %04x\n",
        unit, port, ability->pause, an_adv));
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_ability_advert_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_wc40_ability_advert_get(int unit, soc_port_t port,
                           soc_port_ability_t *ability)
{
    uint16           an_adv;
    uint16           up4;
    uint16           up3;
    uint16           up2;
    soc_port_mode_t  mode;
    phy_ctrl_t      *pc;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = INT_PHY_SW_STATE(unit, port);

    WC40_MEM_SET(ability, 0, sizeof(*ability));
    SOC_IF_ERROR_RETURN
        (READ_WC40_DIGITAL3_UP1r(unit, pc, 0x00, &an_adv));

    SOC_IF_ERROR_RETURN
        (READ_WC40_DIGITAL3_UP2r(unit, pc, 0x00, &up2));

    SOC_IF_ERROR_RETURN
        (READ_WC40_DIGITAL3_UP3r(unit, pc, 0x00, &up3));

    SOC_IF_ERROR_RETURN
        (READ_WC40_DIGITAL6_UP4r(unit, pc, 0x00, &up4));

    /* preemphasis settings */
    /* preemphasis (up2 & DIGITAL3_UP2_PREEMPHASIS_MASK) >>
     *              DIGITAL3_UP2_PREEMPHASIS_SHIFT
     * idriver     (up2 & DIGITAL3_UP2_IDRIVER_MASK) >>
     *              DIGITAL3_UP2_IDRIVER_SHIFT
     * ipredriver  (up2 & DIGITAL3_UP2_IPREDRIVER_MASK) >>
     *              DIGITAL3_UP2_IPREDRIVER_SHIFT
     */
    /* CL72,FEC and Higig2 
     * CL72 cap - (up3 & DIGITAL3_UP3_CL72_MASK)
     * FEC cap  - (up3 & DIGITAL3_UP3_FEC_MASK)
     * Higig2   - (up3 & DIGITAL3_UP3_HIGIG2_MASK)
     */
    ability->encap = (up3 & DIGITAL3_UP3_HIGIG2_MASK)?
                      SOC_PA_ENCAP_HIGIG2: 0;
    
    mode = 0;
    mode |= (up3 & DIGITAL3_UP3_DATARATE_40G_MASK) ?
              SOC_PA_SPEED_40GB : 0;
    mode |= (up3 & DIGITAL3_UP3_DATARATE_31P5G_MASK) ?
              SOC_PA_SPEED_30GB : 0;
    mode |= (up3 & DIGITAL3_UP3_DATARATE_25P45GX4_MASK) ?
              SOC_PA_SPEED_25GB : 0;
    mode |= (up3 & DIGITAL3_UP3_DATARATE_21GX4_MASK) ?
              SOC_PA_SPEED_21GB : 0;

    mode |= (up4 & DIGITAL6_UP4_DATARATE_20G_MASK) ? SOC_PA_SPEED_20GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_20GX4_MASK) ? SOC_PA_SPEED_20GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_16GX4_MASK) ? SOC_PA_SPEED_16GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_15GX4_MASK) ? SOC_PA_SPEED_15GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_13GX4_MASK) ? SOC_PA_SPEED_13GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_12P5GX4_MASK) ? 
            SOC_PA_SPEED_12P5GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_12GX4_MASK) ? SOC_PA_SPEED_12GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_10GCX4_MASK) ? SOC_PA_SPEED_10GB: 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_10GX4_MASK) ? SOC_PA_SPEED_10GB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_6GX4_MASK)? SOC_PA_SPEED_6000MB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_5GX4_MASK)? SOC_PA_SPEED_5000MB : 0;
    mode |= (an_adv & DIGITAL3_UP1_DATARATE_2P5GX1_MASK) ? 
                    SOC_PA_SPEED_2500MB : 0;

    SOC_IF_ERROR_RETURN
        (READ_WC40_COMBO_IEEE0_AUTONEGADVr(unit, pc, 0x00, &an_adv));
      
    mode |= (an_adv & MII_ANA_C37_FD) ? SOC_PA_SPEED_1000MB : 0;
    ability->speed_full_duplex = mode;

    mode = 0;
    switch (an_adv & (MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE)) {
        case MII_ANA_C37_PAUSE:
            mode = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case MII_ANA_C37_ASYM_PAUSE:
            mode = SOC_PA_PAUSE_TX;
            break;
        case MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE:
            mode = SOC_PA_PAUSE_RX;
            break;
    }
    ability->pause = mode;

    /* check for clause73 */
    if (DEV_CFG_PTR(pc)->cl73an) {
        SOC_IF_ERROR_RETURN
            (_phy_wc40_c73_adv_local_get(unit, port, ability));
    }

    SOC_DEBUG_PRINT((DK_PHY,
     "phy_wc40_ability_advert_get:unit=%d p=%d pause=%08x sp=%08x\n",
     unit, port, ability->pause, ability->speed_full_duplex));

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_c73_adv_remote_get(int unit, soc_port_t port,
                             soc_port_ability_t *ability)
{
    uint16            an_adv;
    soc_port_mode_t   mode;
    phy_ctrl_t       *pc;

    pc = INT_PHY_SW_STATE(unit, port);
     SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY1r(unit, pc, 0x00, &an_adv));

    mode =  (an_adv & CL73_AN_ADV_TECH_1G_KX) ? SOC_PA_SPEED_1000MB : 0;
    mode |= (an_adv & CL73_AN_ADV_TECH_10G_KX4) ? SOC_PA_SPEED_10GB : 0;
    mode |= (an_adv & CL73_AN_ADV_TECH_10G_KR) ? SOC_PA_SPEED_10GB : 0;
    mode |= (an_adv & CL73_AN_ADV_TECH_40G_KR4) ? SOC_PA_SPEED_40GB : 0;
    mode |= (an_adv & CL73_AN_ADV_TECH_40G_CR4) ? SOC_PA_SPEED_40GB : 0;

    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY2r(unit, pc, 0x00, &an_adv));
    mode |= (an_adv & CL73_AN_ADV_TECH_20G_KR2) ? SOC_PA_SPEED_20GB : 0;

    ability->speed_full_duplex |= mode;

    /* advert register 0x12, bit 15 FEC requested,bit 14 FEC ability */
    /* SOC_IF_ERROR_RETURN
     *   (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY2r(unit, pc, 0x00, &an_adv));
     */
    /* (an_adv & AN_IEEE1BLK_AN_ADVERTISEMENT2_FEC_REQUESTED_MASK) */


    SOC_IF_ERROR_RETURN
        (READ_WC40_AN_IEEE1BLK_AN_LP_BASEPAGEABILITY0r(unit, pc, 0x00, &an_adv));

    mode = 0;
    switch (an_adv & (CL73_AN_ADV_PAUSE | CL73_AN_ADV_ASYM_PAUSE)) {
        case CL73_AN_ADV_PAUSE:
            mode = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case CL73_AN_ADV_ASYM_PAUSE:
            mode = SOC_PA_PAUSE_TX;
            break;
        case CL73_AN_ADV_PAUSE | CL73_AN_ADV_ASYM_PAUSE:
            mode = SOC_PA_PAUSE_RX;
            break;
    }
    ability->pause = mode;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_ability_remote_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      mode - (OUT) Port mode mask indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_wc40_ability_remote_get(int unit, soc_port_t port,
                        soc_port_ability_t *ability)
{
    uint16           an_adv;
    uint16           up4;
    uint16           up3;
    uint16           up2;
    uint16           data16;
    int              an_enable;
    int              link_1000x = FALSE;
    int              link_combo = FALSE;
    soc_port_mode_t  mode;
    phy_ctrl_t      *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, &data16));

    an_enable = (data16 & MII_CTRL_AE) ? TRUE : FALSE;

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_XGXSBLK4_XGXSSTATUS1r(unit, pc, 0x00, &data16));
        if (data16 & (XGXSBLK4_XGXSSTATUS1_LINKSTAT_MASK |
                               XGXSBLK4_XGXSSTATUS1_LINK10G_MASK)) {
            link_combo = TRUE;
        }
    } else {
        SOC_IF_ERROR_RETURN
            (READ_WC40_GP2_REG_GP2_1r(unit, pc, 0x00, &data16));
        if (data16 & (1 << pc->lane_num)) {
            link_1000x = TRUE;
        }
    } 

    SOC_DEBUG_PRINT((DK_PHY,
               "u=%d p=%d an_enable=%04x link_1000x=%04x link_combo=%04x\n",
                unit, port, an_enable, link_1000x,link_combo));

    WC40_MEM_SET(ability, 0, sizeof(*ability));
    mode = 0;

    if (an_enable && (link_1000x || link_combo)) {
        /* Decode remote advertisement only when link is up and autoneg is 
         * completed.
         */
 
        SOC_IF_ERROR_RETURN
            (READ_WC40_DIGITAL3_LP_UP1r(unit, pc, 0x00, &an_adv));

        SOC_IF_ERROR_RETURN
            (READ_WC40_DIGITAL3_LP_UP2r(unit, pc, 0x00, &up2));

        SOC_IF_ERROR_RETURN
            (READ_WC40_DIGITAL3_LP_UP3r(unit, pc, 0x00, &up3));

        SOC_IF_ERROR_RETURN
            (READ_WC40_DIGITAL5_LP_UP4r(unit, pc, 0x00, &up4));

        /* preemphasis settings */
        /* preemphasis (up2 & DIGITAL3_UP2_PREEMPHASIS_MASK) >>
         *              DIGITAL3_UP2_PREEMPHASIS_SHIFT
         * idriver     (up2 & DIGITAL3_UP2_IDRIVER_MASK) >>
         *              DIGITAL3_UP2_IDRIVER_SHIFT
         * ipredriver  (up2 & DIGITAL3_UP2_IPREDRIVER_MASK) >>
         *              DIGITAL3_UP2_IPREDRIVER_SHIFT
         */
        /* CL72,FEC and Higig2 
         * CL72 cap - (up3 & DIGITAL3_UP3_CL72_MASK)
         * FEC cap  - (up3 & DIGITAL3_UP3_FEC_MASK)
         * Higig2   - (up3 & DIGITAL3_UP3_HIGIG2_MASK)
         */
        ability->encap = (up3 & DIGITAL3_UP3_HIGIG2_MASK)?
                      SOC_PA_ENCAP_HIGIG2: 0;

        mode |= (up3 & DIGITAL3_LP_UP3_DATARATE_40G_MASK)?
                SOC_PA_SPEED_40GB: 0;
        mode |= (up3 & DIGITAL3_LP_UP3_DATARATE_31P5G_MASK)?
                SOC_PA_SPEED_30GB: 0;
        mode |= (up3 & DIGITAL3_LP_UP3_DATARATE_25P45GX4_MASK)?
                SOC_PA_SPEED_25GB: 0;
        mode |= (up3 & DIGITAL3_LP_UP3_DATARATE_21GX4_MASK)?
                SOC_PA_SPEED_21GB: 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_20GX4_MASK) ? 
                SOC_PA_SPEED_20GB : 0;
        mode |= (up4 & DIGITAL5_LP_UP4_DATARATE_20G_MASK) ? 
                SOC_PA_SPEED_20GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_16GX4_MASK) ? 
                SOC_PA_SPEED_16GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_15GX4_MASK) ? 
                SOC_PA_SPEED_15GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_13GX4_MASK) ? 
                SOC_PA_SPEED_13GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_12P5GX4_MASK) ? 
                SOC_PA_SPEED_12P5GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_12GX4_MASK) ? 
                SOC_PA_SPEED_12GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_10GCX4_MASK) ? 
                SOC_PA_SPEED_10GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_10GX4_MASK) ? 
                SOC_PA_SPEED_10GB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_6GX4_MASK) ?
                 SOC_PA_SPEED_6000MB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_5GX4_MASK) ?
                 SOC_PA_SPEED_5000MB : 0;
        mode |= (an_adv & DIGITAL3_LP_UP1_DATARATE_2P5GX1_MASK) ?
                 SOC_PA_SPEED_2500MB : 0;

        SOC_DEBUG_PRINT((DK_PHY,
                         "u=%d p=%d over1G an_adv=%04x\n",
                         unit, port, an_adv));

        SOC_IF_ERROR_RETURN
            (READ_WC40_COMBO_IEEE0_AUTONEGLPABILr(unit, pc, 0x00, &an_adv));

        SOC_DEBUG_PRINT((DK_PHY,
                         "u=%d p=%d combo an_adv=%04x\n",
                         unit, port, an_adv));

        mode |= (an_adv & MII_ANP_C37_FD) ? SOC_PA_SPEED_1000MB : 0;
        ability->speed_full_duplex = mode;
          
        switch (an_adv & (MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE)) {
            case MII_ANP_C37_PAUSE:
                ability->pause |= SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
                break;
            case MII_ANP_C37_ASYM_PAUSE:
                ability->pause |= SOC_PA_PAUSE_TX;
                break;
            case MII_ANP_C37_PAUSE | MII_ANP_C37_ASYM_PAUSE:
                ability->pause |= SOC_PA_PAUSE_RX;
                break;
        }
        if (DEV_CFG_PTR(pc)->cl73an) {
            SOC_IF_ERROR_RETURN
                (READ_WC40_AN_IEEE0BLK_AN_IEEESTATUS1r(unit, pc, 0x00, &data16));
            if (data16 & MII_STAT_AN_DONE) {
                SOC_IF_ERROR_RETURN
                    (_phy_wc40_c73_adv_remote_get(unit, port, ability));
            }
        }
    } else {
        /* Simply return local abilities */
        phy_wc40_ability_advert_get(unit, port, ability);
    }
 
    SOC_DEBUG_PRINT((DK_PHY,
         "phy_wc40_ability_remote_get:unit=%d p=%d pause=%08x sp=%08x\n",
         unit, port, ability->pause, ability->speed_full_duplex));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_lb_set
 * Purpose:
 *      Put hc/FusionCore in PHY loopback
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - binary value for on/off (1/0)
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_lb_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;

    uint16 lane_mask;
    uint16 lane;
    uint16 data16;

    pc = INT_PHY_SW_STATE(unit, port);

    if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc) || WC40_REVID_B0(pc)) {
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK1_LANETESTr(unit,pc,0x00,
                     enable? XGXSBLK1_LANETEST_PWRDN_SAFE_DIS_MASK: 0,
                     XGXSBLK1_LANETEST_PWRDN_SAFE_DIS_MASK));
        }
    }

    /* Use register 0x8017 bit 3-0 gloop, set 0x8000 bit4 for all modes */
     
    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        if (IS_DUAL_LANE_PORT(pc)) {
            lane_mask = 1 << pc->lane_num;
            lane_mask |= (lane_mask << 1);
        } else {
            lane_mask = 1 << pc->lane_num;
        }       
    } else {  /* combo mode */
            lane_mask = XGXSBLK1_LANECTRL2_GLOOP1G_MASK;
    }
    if (enable) {
        lane = lane_mask;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00,
                 XGXSBLK0_XGXSCONTROL_MDIO_CONT_EN_MASK,
                 XGXSBLK0_XGXSCONTROL_MDIO_CONT_EN_MASK));

        if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* reverse flipped TX/RX polarities if any*/
            SOC_IF_ERROR_RETURN
                (_phy_wc40_ind_lane_polarity_set(unit,pc,FALSE));
        } else {
            /* reverse flipped Tx/Rx Polarity  */
            SOC_IF_ERROR_RETURN
                (_phy_wc40_combo_polarity_set(unit,pc,FALSE));
        }
    } else { 
        if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* reverse flipped TX/RX polarities if any*/
            SOC_IF_ERROR_RETURN
                (_phy_wc40_ind_lane_polarity_set(unit,pc, TRUE));
        } else {
            /* reverse flipped Tx/Rx Polarity  */
            SOC_IF_ERROR_RETURN
                (_phy_wc40_combo_polarity_set(unit,pc,TRUE));
        }
        SOC_IF_ERROR_RETURN
            (READ_WC40_XGXSBLK1_LANECTRL2r(unit,pc,0x00,&data16));
        lane = ~lane_mask;
        lane &= data16;
        lane &= XGXSBLK1_LANECTRL2_GLOOP1G_MASK; 
        if (!lane) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00,
                     0,
                     XGXSBLK0_XGXSCONTROL_MDIO_CONT_EN_MASK));
        }
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANECTRL2r(unit, pc, 0x00, lane, lane_mask));
  
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_lb_get
 * Purpose:
 *      Get hc/FusionCore PHY loopback state
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - address of location to store binary value for on/off (1/0)
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_lb_get(int unit, soc_port_t port, int *enable)
{
    phy_ctrl_t *pc;

    uint16 lane;
    uint16 lane_mask;

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK1_LANECTRL2r(unit,pc,0x00,&lane));

    lane &= XGXSBLK1_LANECTRL2_GLOOP1G_MASK;
 
    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        if (IS_DUAL_LANE_PORT(pc)) {
            lane_mask = 1 << pc->lane_num;
            lane_mask |= (lane_mask << 1);
        } else {
            lane_mask = 1 << pc->lane_num;
        }
    } else {  /* combo mode */
            lane_mask = XGXSBLK1_LANECTRL2_GLOOP1G_MASK;
    }
    lane &= lane_mask;

    *enable = lane;

    return SOC_E_NONE;
}

STATIC int
phy_wc40_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    phy_ctrl_t      *pc;
    soc_port_if_t intf;

    pc = INT_PHY_SW_STATE(unit, port);

    if (pif > 31) {
        return SOC_E_PARAM;
    }

    /* need to check valid interfaces
     * ALL 40G/10G interfaces are mutually exclusive
     */
    intf = DEV_CFG_PTR(pc)->line_intf;

    if (WC40_40G_10G_INTF(pif)) {
        intf &= ~WC40_40G_10G_INTF_ALL;  /* clear all 10G/40G interfaces */
        intf |= 1 << pif;
    } else {
        intf |= 1 << pif;
    }

    DEV_CFG_PTR(pc)->line_intf = intf;
    return SOC_E_NONE;
}

STATIC int
phy_wc40_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    phy_ctrl_t *pc;
    int speed;
    int intf;
    int scr;
    int rv;
    uint16 data16;

    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        rv = _phy_wc40_combo_speed_get(unit, port, &speed,&intf,&scr);
    } else {
        rv = _phy_wc40_ind_speed_get(unit, port, &speed,&intf,&scr);
    }

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        if (speed < 10000) {
            if (DEV_CFG_PTR(pc)->fiber_pref) {
                *pif = SOC_PORT_IF_GMII;
            } else {
                *pif = SOC_PORT_IF_SGMII;
            }
        } else {
            *pif = intf;
        }
    } else { /* combo mode */
        if (intf == SOC_PORT_IF_KR4) {
            SOC_IF_ERROR_RETURN
                (READ_WC40_UC_INFO_B1_FIRMWARE_MODEr(unit, pc, 0x00, &data16));
            if (data16 == WC40_UC_CTRL_XLAUI) {
                intf = SOC_PORT_IF_XLAUI;
            } else if (data16 == WC40_UC_CTRL_SR4) {
                intf = SOC_PORT_IF_SR;
            }
        }
        *pif = intf;
    }

    return rv;
}

STATIC int
phy_wc40_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;

    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    pc = INT_PHY_SW_STATE(unit, port);

    WC40_MEM_SET(ability, 0, sizeof(*ability));

    if (CUSTOM_MODE(pc)) {
        ability->speed_full_duplex  = SOC_PA_SPEED_6000MB; 
        ability->speed_full_duplex  = SOC_PA_SPEED_3000MB; 
        ability->medium    = SOC_PA_MEDIUM_FIBER;
        ability->loopback  = SOC_PA_LB_PHY;
        return (SOC_E_NONE);
    }

    if (CUSTOM1_MODE(pc)) {    
        ability->speed_full_duplex  = SOC_PA_SPEED_12GB;
        ability->medium    = SOC_PA_MEDIUM_FIBER;
        ability->loopback  = SOC_PA_LB_PHY;
        ability->interface |= SOC_PA_INTF_XGMII;
        return (SOC_E_NONE);
    }
    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) { 
        ability->speed_full_duplex  = SOC_PA_SPEED_1000MB;
        if (DEV_CFG_PTR(pc)->fiber_pref)   {
            ability->speed_full_duplex  |= SOC_PA_SPEED_2500MB |
                                           SOC_PA_SPEED_10GB;
                ability->speed_full_duplex  |= SOC_PA_SPEED_100MB;
                ability->speed_half_duplex  = SOC_PA_SPEED_100MB;
        } else {
            ability->speed_half_duplex  = SOC_PA_SPEED_10MB |
                                          SOC_PA_SPEED_100MB;
            ability->speed_full_duplex  |= SOC_PA_SPEED_10MB |
                                           SOC_PA_SPEED_100MB;
        }
        switch(pc->speed_max) {
            case 20000:
                ability->speed_full_duplex |= SOC_PA_SPEED_20GB;
                /* fall through */
            case 15000:
                ability->speed_full_duplex |= SOC_PA_SPEED_15GB;
                /* fall through */
            case 12000:
                ability->speed_full_duplex |= SOC_PA_SPEED_12GB;
                /* fall through */
            case 10000:
                ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
                /* fall through */
            default:
                break;
        }
        ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
        ability->interface = SOC_PA_INTF_GMII | SOC_PA_INTF_SGMII;
        if (pc->speed_max >= 10000) {
            ability->interface |= SOC_PA_INTF_XGMII;
        } 
        ability->medium    = SOC_PA_MEDIUM_FIBER;
        ability->loopback  = SOC_PA_LB_PHY;
        if (IS_DUAL_LANE_PORT(pc)) {
            ability->flags     = 0;
        } else {
            ability->flags     = SOC_PA_AUTONEG;
        }
    } else {
        /* Quad-lane single port mode does not support 1G and 2.5G.
         * It supports 10G and above
         */
        ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;
        ability->speed_full_duplex  = 0; 
        switch(pc->speed_max) {
            case 42000:
            case 40000:
                ability->speed_full_duplex |= SOC_PA_SPEED_40GB;
                if (!(WC40_REVID_A0(pc) || WC40_REVID_A1(pc))) {
                    ability->speed_full_duplex |= SOC_PA_SPEED_42GB;
                }
            case 32000:
            case 30000:
                ability->speed_full_duplex |= SOC_PA_SPEED_30GB;
            case 25000:
                ability->speed_full_duplex |= SOC_PA_SPEED_25GB;
            case 21000:
                ability->speed_full_duplex |= SOC_PA_SPEED_21GB;
            case 20000:
                ability->speed_full_duplex |= SOC_PA_SPEED_20GB;
                /* fall through */
            case 16000:
                ability->speed_full_duplex |= SOC_PA_SPEED_16GB; 
                /* fall through */
            case 15000:
                ability->speed_full_duplex |= SOC_PA_SPEED_15GB; 
                /* fall through */
            case 13000:
                ability->speed_full_duplex |= SOC_PA_SPEED_13GB;
                /* fall through */
            case 12000:
                ability->speed_full_duplex |= SOC_PA_SPEED_12GB;
                /* fall through */
            default:
                ability->speed_full_duplex |= SOC_PA_SPEED_10GB; 
        }
        ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
        ability->interface = SOC_PA_INTF_XGMII;
        ability->medium    = SOC_PA_MEDIUM_FIBER;
        ability->loopback  = SOC_PA_LB_PHY;
        ability->flags     = SOC_PA_AUTONEG;
    }

    SOC_DEBUG_PRINT((DK_PHY,
     "phy_wc40_ability_local_get:unit=%d p=%d sp=%08x\n",
     unit, port, ability->speed_full_duplex));

    return (SOC_E_NONE);
}


#define PHY_WC40_LANEPRBS_LANE_SHIFT   4

STATIC int
_phy_wc40_control_prbs_polynomial_set(int unit, soc_port_t port, uint32 value)
{
    uint16 data16 = 0;
    uint16 mask16 = 0;
    int i;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);
       
    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        DEV_CTRL_PTR(pc)->prbs.poly = WC40_PRBS_CFG_POLY31;  /* only support the PRBS31 for now */
        return SOC_E_NONE;
    }
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        /* configure for all four lanes */
        for (i = 0; i < 4; i++) {
            data16 |= value << (PHY_WC40_LANEPRBS_LANE_SHIFT * i);
            mask16 |= XGXSBLK1_LANEPRBS_PRBS_ORDER0_MASK << 
                      (PHY_WC40_LANEPRBS_LANE_SHIFT * i);
        }
    } else if (IS_DUAL_LANE_PORT(pc)) {
        for (i = pc->lane_num; i <= pc->lane_num+1; i++) {
            data16 |= value << (PHY_WC40_LANEPRBS_LANE_SHIFT * i);
            mask16 |= XGXSBLK1_LANEPRBS_PRBS_ORDER0_MASK << 
                      (PHY_WC40_LANEPRBS_LANE_SHIFT * i);
        }
    } else { /* single lane mode */
        data16 = value << (PHY_WC40_LANEPRBS_LANE_SHIFT * pc->lane_num);
        mask16 |= XGXSBLK1_LANEPRBS_PRBS_ORDER0_MASK << 
                      (PHY_WC40_LANEPRBS_LANE_SHIFT * pc->lane_num);
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, data16, mask16));
    DEV_CTRL_PTR(pc)->prbs.poly = value;

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_prbs_polynomial_get(int unit, soc_port_t port, uint32 *value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);;

    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        *value = WC40_PRBS_CFG_POLY31;  /*  prbs31 */
        return SOC_E_NONE;  
    }

    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, &data));

    /* Extract prbs polynomial setting from register */
    data = ((data >> (XGXSBLK1_LANEPRBS_PRBS_ORDER1_SHIFT * pc->lane_num)) &
            XGXSBLK1_LANEPRBS_PRBS_ORDER0_MASK);
    *value = (uint32) data;

    return SOC_E_NONE;
}


#define INV_SHIFTER(ln)  (PHY_WC40_LANEPRBS_LANE_SHIFT * (ln))

STATIC int
_phy_wc40_control_prbs_tx_invert_data_set(int unit, soc_port_t port, uint32 value)
{
    int i;
    uint16 data16 = 0;
    uint16 mask16 = 0;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        return SOC_E_NONE;  /* not supported */
    }

    value <<= XGXSBLK1_LANEPRBS_PRBS_INV0_SHIFT;
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        /* configure for all four lanes */
        for (i = 0; i < 4; i++) {
            data16 |= value << INV_SHIFTER(i);
            mask16 |= XGXSBLK1_LANEPRBS_PRBS_INV0_MASK <<
                      INV_SHIFTER(i);
        }
    } else if (IS_DUAL_LANE_PORT(pc)) {
        for (i = pc->lane_num; i <= pc->lane_num+1; i++) {
            data16 |= value << INV_SHIFTER(i);
            mask16 |= XGXSBLK1_LANEPRBS_PRBS_INV0_MASK <<
                      INV_SHIFTER(i);
        }
    } else { /* single lane mode */
        data16 = value << INV_SHIFTER(pc->lane_num);
        mask16 |= XGXSBLK1_LANEPRBS_PRBS_INV0_MASK <<
                      INV_SHIFTER(pc->lane_num);
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, data16, mask16));
    
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_prbs_tx_invert_data_get(int unit, soc_port_t port, uint32 *value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);
    int inv_shifter;
		
    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        *value = 0;  /* no inversion */
        return SOC_E_NONE;  
    }
    inv_shifter = (PHY_WC40_LANEPRBS_LANE_SHIFT * pc->lane_num) +  
                   XGXSBLK1_LANEPRBS_PRBS_INV0_SHIFT;

    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, &data));
    
    data &= 1 << inv_shifter;
    
    *value = (data) ?  1 : 0;
    
    return SOC_E_NONE;
}

#define PRBS_LANES_MASK  (XGXSBLK1_LANEPRBS_PRBS_EN3_MASK | \
				XGXSBLK1_LANEPRBS_PRBS_EN2_MASK | \
				XGXSBLK1_LANEPRBS_PRBS_EN1_MASK | \
				XGXSBLK1_LANEPRBS_PRBS_EN0_MASK)

/* PRBS test
 * this routine enables the PRBS generator on applicable lanes depending on
 * the port mode. Before calling this function, a forced speed mode should
 * be set and either the external loopback or internal loopback should be
 * configured. Once this function is called, application should wait to
 * let the test run for a while and then calls the 
 * _phy_wc40_control_prbs_rx_status_get() to retrieve PRBS test status.
 * When calling this function to disable the PRBS test, the device or 
 * specific lane will be re-initialized.  
 */

STATIC int
_phy_wc40_control_prbs_enable_set(int unit, soc_port_t port, 
                                          uint32 enable)
{
    uint16      data16;
    uint16      mask16;
    int an;
    int an_done;
    soc_port_if_t intf;
    int prbs_lanes = 0;
    int lane;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    /* If mode is autoneg KR/KR4, do it thru CL49 PRBS and configure KR forced mode and disable autoneg
     * This way the FIR settings negotiated thru CL72 is preserved
     */

    if (DEV_CTRL_PTR(pc)->prbs.type != WC40_PRBS_TYPE_CL49) {
        SOC_IF_ERROR_RETURN
            (phy_wc40_an_get(unit,port,&an,&an_done));

        if (an && an_done) {
            /* check interface */
            SOC_IF_ERROR_RETURN
                (phy_wc40_interface_get(unit,port,&intf));
            if ((intf == SOC_PORT_IF_KR) || (intf == SOC_PORT_IF_KR4) ) {
                DEV_CTRL_PTR(pc)->prbs.type = WC40_PRBS_TYPE_CL49;
            }
        }
    }

    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        SOC_IF_ERROR_RETURN
            (phy_wc40_interface_get(unit,port,&intf));

        if (!enable) {  /* disable */
            if (intf == SOC_PORT_IF_KR4) {
                for (lane = 0; lane < 4; lane++) {
                    SOC_IF_ERROR_RETURN
                        (WRITE_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, ln_access[lane], 0));
                }
            } else {
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, 0x00, 0));
            }
            DEV_CTRL_PTR(pc)->prbs.type = 0;
        } 

        if (intf == SOC_PORT_IF_KR4) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_MISC1r(unit, pc, 0x00, 
                      enable? 0x11: 0, 
                      SERDESDIGITAL_MISC1_FORCE_SPEED_MASK));

            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DIGITAL4_MISC3r(unit, pc, 0x00,
                        enable?  DIGITAL4_MISC3_FORCE_SPEED_B5_MASK:0,
                        DIGITAL4_MISC3_FORCE_SPEED_B5_MASK));

        } else {  /* KR */
            if (WC40_PHY400_REVS(pc)) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DSC2B0_DSC_MISC_CTRL0r(unit, pc, 0x00,
                             enable? DSC2B0_DSC_MISC_CTRL0_RXSEQSTART_AN_DISABLE_MASK:0,
                             DSC2B0_DSC_MISC_CTRL0_RXSEQSTART_AN_DISABLE_MASK));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                             enable? SERDESDIGITAL_MISC2_AN_TXDISABLE_LN_MASK:0,
                             SERDESDIGITAL_MISC2_AN_TXDISABLE_LN_MASK));
            }

            /* force KR speed */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_MISC2r(unit, pc, 0x00,
                   enable? SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK:0,
                   SERDESDIGITAL_MISC2_PMA_PMD_FORCED_SPEED_ENC_EN_MASK));
            mask16 =  PMD_IEEE0BLK_PMD_IEEECONTROL1_SPEEDSELECTION1_MASK |
                      PMD_IEEE0BLK_PMD_IEEECONTROL1_SPEEDSELECTION0_MASK;
            data16 = mask16;
            mask16 |= PMD_IEEE0BLK_PMD_IEEECONTROL1_SPEEDSELECTION2_MASK;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_PMD_IEEE0BLK_PMD_IEEECONTROL1r(unit, pc, 0x00, 
                             data16, mask16));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_PMD_IEEE0BLK_PMD_IEEECONTROL2r(unit, pc, 0x00, 
                             0xb, 0xf));
        }
        /* disable cl73 */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, 
                                          enable? 0: MII_CTRL_AE | MII_CTRL_RAN,
                                              MII_CTRL_AE | MII_CTRL_RAN));
        /* disable cl37 */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, 
                                  enable? 0: MII_CTRL_AE | MII_CTRL_RAN,
                                      MII_CTRL_AE | MII_CTRL_RAN));

        SOC_IF_ERROR_RETURN
            (READ_WC40_AN_IEEE0BLK_AN_IEEECONTROL1r(unit, pc, 0x00, 
                                          &data16));
        /* not to enable PRBS here. Once PRBS is enabled, the link will go down.
         * Autoneg will be restarted by link partner and Tx settings will be lost.
         * It will be enabled in get function when first time called 
         */ 
        return SOC_E_NONE;
    }

    /* non-CL49 PRBS */ 
    if (enable) {

        /* A forced speed mode should be set before running PRBS */

        /* CDED & EDEN off: 0x8000 */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit, pc, 0x00, 0,
                   XGXSBLK0_XGXSCONTROL_CDET_EN_MASK |
                   XGXSBLK0_XGXSCONTROL_EDEN_MASK));

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* CDED & EDEN off: 0x8017 */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK1_LANECTRL2r(unit, pc, 0x00, 0,
                       XGXSBLK1_LANECTRL2_CDET_EN1G_MASK |
                       XGXSBLK1_LANECTRL2_EDEN1G_MASK));
        
            /* Turn off CL36 Tx PCS and 64/66 Endec: 0x8015 */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK1_LANECTRL0r(unit, pc, 0x00, 0,
                         XGXSBLK1_LANECTRL0_CL36_PCS_EN_TX_MASK |
                         XGXSBLK1_LANECTRL0_ED66EN_MASK));
        } else {
            /* Turn off CL36 Tx PCS and 64/66 Endec for active lane(s): 0x8015 */
            mask16 = 1 << pc->lane_num;
            if (IS_DUAL_LANE_PORT(pc)) {
                mask16 |= 1 << (pc->lane_num + 1);
            }
            mask16 |=  (mask16 << 12);
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK1_LANECTRL0r(unit, pc, 0x00, 0, mask16));

            /* CDED & EDEN off for the active lane(s): 0x8017 */
            mask16 = 1 << pc->lane_num;
            if (IS_DUAL_LANE_PORT(pc)) {
                mask16 |= 1 << (pc->lane_num + 1);
            }
            mask16 =  (mask16 << 8) | (mask16 << 12);
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK1_LANECTRL2r(unit, pc, 0x00, 0, mask16));

            /* clear RX clock compensation which seems messed up PRBS function */
            if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc) || WC40_REVID_B0(pc)) {
                if (IS_DUAL_LANE_PORT(pc)) { 
                    SOC_IF_ERROR_RETURN
                        (WRITE_WC40_XGXSBLK2_UNICOREMODE10Gr(unit, pc, 0x00, 0));
                } else {
                    SOC_IF_ERROR_RETURN
                        (MODIFY_WC40_RX66_CONTROLr(unit, pc, 0x00,
                            0,
                            RX66_CONTROL_CC_EN_MASK | RX66_CONTROL_CC_DATA_SEL_MASK));
                }
            }
        }

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* Enable PRBS read on all lanes: 0x80f1=0x1c47 */ 
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_RXB_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));
            prbs_lanes = PRBS_LANES_MASK;

        } else if (IS_DUAL_LANE_PORT(pc)) {
            /* enable both lanes */
            if (pc->lane_num == 0) {
                prbs_lanes = XGXSBLK1_LANEPRBS_PRBS_EN1_MASK |
                             XGXSBLK1_LANEPRBS_PRBS_EN0_MASK;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RX0_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RX1_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));

            } else { /* lane2 */
                prbs_lanes = XGXSBLK1_LANEPRBS_PRBS_EN3_MASK |
                             XGXSBLK1_LANEPRBS_PRBS_EN2_MASK;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RX2_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RX3_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));
            }

        } else {  /* single lane mode */
            prbs_lanes = 1 << ((XGXSBLK1_LANEPRBS_PRBS_ORDER1_SHIFT * 
                            pc->lane_num) + XGXSBLK1_LANEPRBS_PRBS_EN0_SHIFT);
            SOC_IF_ERROR_RETURN
                (WC40_REG_MODIFY(unit,pc,0x0,0x80B1 + (0x10 * pc->lane_num), 
                       RX0_ANARXCONTROL_STATUS_SEL_prbsStatus,
                       RX0_ANARXCONTROL_STATUS_SEL_MASK));
        }
        
        /* enable PRBS generator on applicable lanes: 0x8019 */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, prbs_lanes,
                          prbs_lanes));

        
    } else {
        /* Restore port */
        return phy_wc40_init(unit, port);
    }
    return SOC_E_NONE;
}


STATIC int
_phy_wc40_control_prbs_enable_get(int unit, soc_port_t port, 
                                        uint32 *value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);;

    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, 0x00, &data));
        if (data == WC40_PRBS_CL49_POLY31) {
            *value = 1; 
        } else {
            *value = 0; 
        }
        return SOC_E_NONE;
    }

    /*
     * Hypercore PRBS - note that in the Hypercore there is only 1 enable 
     * for both TX/RX 
     */
    SOC_IF_ERROR_RETURN
        (READ_WC40_XGXSBLK1_LANEPRBSr(unit, pc, 0x00, &data));

    data = data & ((1 << (XGXSBLK1_LANEPRBS_PRBS_ORDER1_SHIFT * pc->lane_num))
                   << XGXSBLK1_LANEPRBS_PRBS_EN0_SHIFT);
    
    *value = (data) ?  1 : 0;
  
    return SOC_E_NONE;
}

/*
 * Returns value 
 *      ==  0: PRBS receive is in sync
 *      == -1: PRBS receive never got into sync
 *      ==  n: number of errors
 */
STATIC int
_phy_wc40_control_prbs_rx_status_get(int unit, soc_port_t port, 
                                          uint32 *value)
{
    int         lane;
    int         lane_end;
    uint16      data;
    int         prbs_cfg;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);;

    /* Get status for all 4 lanes and check for sync
     * 0x80b0, 0x80c0, 0x80d0, 0x80e0 
     */
    *value = 0;
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        /* combo mode, pc->lane_num = 0 */
        lane_end = 3;  
    } else if (IS_DUAL_LANE_PORT(pc)) {
        lane_end = pc->lane_num + 1;  
    } else {
        lane_end = pc->lane_num;  
    }

    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        /* enable PRBS if not already. PRBS31 or PRBS9 */
        for (lane = pc->lane_num; lane <= lane_end; lane++) {
            prbs_cfg = TRUE;
            SOC_IF_ERROR_RETURN
                (READ_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, ln_access[lane], &data));
            if (DEV_CTRL_PTR(pc)->prbs.poly == WC40_PRBS_CFG_POLY31) {
                if (data == WC40_PRBS_CL49_POLY31) { /* prbs already in PRBS31 */
                    prbs_cfg = FALSE;
                }
            } else {
                DEV_CTRL_PTR(pc)->prbs.poly = WC40_PRBS_CFG_POLY31; /* support PRBS31 only */
            }

            if (prbs_cfg) {
                SOC_IF_ERROR_RETURN
                    (WRITE_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, ln_access[lane], 
                              WC40_PRBS_CL49_POLY31));
            }
        }

        data = 0;
        for (lane = pc->lane_num; lane <= lane_end; lane++) {
            SOC_IF_ERROR_RETURN
                (READ_WC40_CL49_USERB0_STATUSr(unit,pc,ln_access[lane],&data));

            if (data & CL49_USERB0_STATUS_PRBS_LOCK_MASK) { /* PRBS lock */
                /* check error count */
                SOC_IF_ERROR_RETURN
                    (READ_WC40_PCS_IEEE2BLK_PCS_TPERRCOUNTERr(unit,pc,ln_access[lane],&data));
                *value += data;
                SOC_DEBUG_PRINT((DK_PHY, "prbs_rx_status_get: u=%d p=%d (lane %d errors 0x%x)\n", 
                                 unit, port, lane, data));

            } else { /* PRBS not in sync */
                *value = -1;
                SOC_DEBUG_PRINT((DK_PHY, "prbs_rx_status_get: u=%d p=%d (lane %d not in sync)\n",
                             unit, port, lane));
                break;
            } 
        }
        return SOC_E_NONE;
    }
 
    for (lane = pc->lane_num; lane <= lane_end; lane++) {
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, 0x00, 0x80b0 + (0x10 * lane), &data));

        if (data == (RX0_ANARXSTATUS_PRBS_STATUS_PRBS_LOCK_BITS  <<
                     RX0_ANARXSTATUS_PRBS_STATUS_PRBS_LOCK_SHIFT)) {
            /* PRBS is in sync */
            continue;
        } else if (data == 0) {
            /* PRBS not in sync */
            SOC_DEBUG_PRINT((DK_PHY, "prbs_rx_status_get: u=%d p=%d (lane %d not in sync)\n",
                             unit, port, lane));
            *value = -1;
            break;
        } else {
            /* Get errors */
            *value += data & RX0_ANARXSTATUS_PRBS_STATUS_PTBS_ERRORS_MASK;
            SOC_DEBUG_PRINT((DK_PHY, "prbs_rx_status_get: u=%d p=%d (lane %d errors 0x%x)\n", 
                    unit, port, lane, (data & RX0_ANARXSTATUS_PRBS_STATUS_PTBS_ERRORS_MASK)));

        }
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_8b10b_set(int unit, soc_port_t port, uint32 value)
{
    uint16      data16;
    uint16      mask16;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    mask16 = 1 << (XGXSBLK1_LANECTRL2_CDET_EN1G_SHIFT + pc->lane_num);
    mask16 |= 1 << (XGXSBLK1_LANECTRL2_EDEN1G_SHIFT + pc->lane_num);

    data16 = value? mask16: 0;
 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANECTRL2r(unit, pc, 0x00, data16,mask16));

    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_preemphasis_set(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 value)
{
    int     lane_ctrl;
    int     lane_num = 0;
    int     i;

    if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE0) {
        lane_ctrl = LANE0_ACCESS;
        lane_num  = 0;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE1) {
        lane_ctrl = LANE1_ACCESS;
        lane_num  = 1;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE2) {
        lane_ctrl = LANE2_ACCESS;
        lane_num  = 2;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE3) {
        lane_ctrl = LANE3_ACCESS;
        lane_num  = 3;
    } else {
        lane_ctrl = TX_DRIVER_DFT_LN_CTRL;
    }

   
    /* write to specific lane */
    if (lane_ctrl != TX_DRIVER_DFT_LN_CTRL) {
        /* qualify the operation: only writes to its own lane. */
        if (!COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) { 
            if (IS_DUAL_LANE_PORT(pc)) {
                if ((lane_num < pc->lane_num) || (lane_num > (pc->lane_num + 1))) {
                    return SOC_E_PARAM;
                } 
            } else {
                if (lane_num != pc->lane_num) {
                    return SOC_E_PARAM;
                }
            }
        }
    
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_CL72_USERB0_CL72_TX_FIR_TAP_REGISTERr(unit,pc,
                     lane_ctrl,value));
        DEV_CFG_PTR(pc)->preemph[lane_num - pc->lane_num] = value;
    } else { /* default mode */
        int size;

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            size = 4;
            /* broadcast to all lanes */
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, WC_AER_BCST_OFS_STRAP));

        } else { /* dual-lane bcst if in dxgxs mode */
            size = IS_DUAL_LANE_PORT(pc)? 2: 1;
            DUAL_LANE_BCST_ENABLE(pc);
        }

        SOC_IF_ERROR_RETURN
            (WRITE_WC40_CL72_USERB0_CL72_TX_FIR_TAP_REGISTERr(unit,pc,0x00,value));

        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            /* restore AER */
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_AERBLK_AERr(unit,pc,0x00, 0));
        } else {
            DUAL_LANE_BCST_DISABLE(pc);
        }
        for (i = 0; i < size; i++) {
            DEV_CFG_PTR(pc)->preemph[i] = value;
        }
    }
    return SOC_E_NONE;
}


STATIC int
_phy_wc40_tx_driver_field_get(soc_phy_control_t type,int *ln_ctrl,uint16 *mask,int *shfter)
{
    int lane_ctrl;
    
    lane_ctrl = TX_DRIVER_DFT_LN_CTRL;
    *mask = 0;
    *shfter = 0;
    /* _LANEn(n=0-3) control type only applies to combo mode or dxgxs in
     * independent channel mode
     */
    switch(type) {
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
    /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
    /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
    /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
    /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        *shfter = TXB_TX_DRIVER_IDRIVER_SHIFT;
        *mask = TXB_TX_DRIVER_IDRIVER_MASK;
        if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0) {
            lane_ctrl = 0;
        } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1) {
            lane_ctrl = 1;
        } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2) {
            lane_ctrl = 2;
        } else if (type == SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3) {
            lane_ctrl = 3;
        }
        break;

    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
    /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
    /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
    /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
    /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        *shfter = TXB_TX_DRIVER_IPREDRIVER_SHIFT;
        *mask = TXB_TX_DRIVER_IPREDRIVER_MASK;
        if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0) {
            lane_ctrl = 0;
        } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1) {
            lane_ctrl = 1;
        } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2) {
            lane_ctrl = 2;
        } else if (type == SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3) {
            lane_ctrl = 3;
        }
        break;
    default:
         /* should never get here */
        return SOC_E_PARAM;
    }
    *ln_ctrl = lane_ctrl;
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_tx_driver_set(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 value)
{
    uint16  data;             /* Temporary holder of reg value to be written */
    uint16  mask;             /* Bit mask of reg value to be updated */
    int     lane_ctrl;
    int     lane_num;
    int     shifter;
    int     i;
    int     size;

    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_driver_field_get(type,&lane_ctrl,&mask,&shifter));

    data = value << shifter;

    /* qualify the operation: only writes to its own lane. */
    if (lane_ctrl != TX_DRIVER_DFT_LN_CTRL) {
        if (!COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            if (IS_DUAL_LANE_PORT(pc)) {
                if ((lane_ctrl < pc->lane_num) || (lane_ctrl > (pc->lane_num + 1))) {
                    return SOC_E_PARAM;
                }
            } else {
                if (lane_ctrl != pc->lane_num) {
                    return SOC_E_PARAM;
                }
            }
        }
    }
    if ((lane_ctrl != TX_DRIVER_DFT_LN_CTRL) ||
        ((!COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) && (!IS_DUAL_LANE_PORT(pc))) ) {
        if (lane_ctrl != TX_DRIVER_DFT_LN_CTRL) {
            lane_num = lane_ctrl;
        } else {
            lane_num = pc->lane_num;         
        }
        if (shifter == TXB_TX_DRIVER_IDRIVER_SHIFT) {
            DEV_CFG_PTR(pc)->idriver[lane_num - pc->lane_num] = value;
        } else {
            DEV_CFG_PTR(pc)->pdriver[lane_num - pc->lane_num] = value;
        }

        if (lane_num == 0) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX0_TX_DRIVERr(unit, pc, 0x00, data, mask));
        } else if (lane_num == 1) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX1_TX_DRIVERr(unit, pc, 0x00, data, mask));
        } else if (lane_num == 2) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX2_TX_DRIVERr(unit, pc, 0x00, data, mask));
        } else if (lane_num == 3) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX3_TX_DRIVERr(unit, pc, 0x00, data, mask));
        }
    } else { /* default control in combo mode, or dxgxs port */
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            size = 4;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX0_TX_DRIVERr(unit, pc, 0x00, data, mask));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX1_TX_DRIVERr(unit, pc, 0x00, data, mask));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX2_TX_DRIVERr(unit, pc, 0x00, data, mask));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX3_TX_DRIVERr(unit, pc, 0x00, data, mask));
        } else if (IS_DUAL_LANE_PORT(pc)) {
            size = 2;
            if (pc->lane_num == 0) {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX0_TX_DRIVERr(unit, pc,0x00, data, mask));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX1_TX_DRIVERr(unit, pc,0x00, data, mask));
            } else {
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX2_TX_DRIVERr(unit, pc, LANE0_ACCESS, data, mask));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TX3_TX_DRIVERr(unit, pc, LANE0_ACCESS, data, mask));
            }
        } else {
            size = 0;
        }
        for (i = pc->lane_num; i < size; i++) {
            if (shifter == TXB_TX_DRIVER_IDRIVER_SHIFT) {
                DEV_CFG_PTR(pc)->idriver[i] = value;
            } else {
                DEV_CFG_PTR(pc)->pdriver[i] = value;
            }
        }

    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_control_preemphasis_get(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 *value)
{
    uint16  data16;             /* Temporary holder of reg value to be written */
    int     lane_ctrl;

    if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE0) {
        lane_ctrl = LANE0_ACCESS;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE1) {
        lane_ctrl = LANE1_ACCESS;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE2) {
        lane_ctrl = LANE2_ACCESS;
    } else if (type == SOC_PHY_CONTROL_PREEMPHASIS_LANE3) {
        lane_ctrl = LANE3_ACCESS;
    } else {
        lane_ctrl = TX_DRIVER_DFT_LN_CTRL;
    }

    /* write to specific lane */
    if (lane_ctrl != TX_DRIVER_DFT_LN_CTRL) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_CL72_USERB0_CL72_TX_FIR_TAP_REGISTERr(unit,pc,
                     lane_ctrl,&data16));
    } else {
        SOC_IF_ERROR_RETURN
            (READ_WC40_CL72_USERB0_CL72_TX_FIR_TAP_REGISTERr(unit,pc,0x00,&data16));
    }
    *value = data16;

    return SOC_E_NONE;
}
STATIC int
_phy_wc40_control_tx_driver_get(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 *value)
{
    uint16  data16;           /* Temporary holder of 16 bit reg value */
    uint16  mask;
    int     shifter;
    int     lane_ctrl;
    int     lane_num;

    SOC_IF_ERROR_RETURN
        (_phy_wc40_tx_driver_field_get(type,&lane_ctrl,&mask,&shifter));

    if (lane_ctrl != TX_DRIVER_DFT_LN_CTRL) {
        lane_num = lane_ctrl;
    } else {
        lane_num = pc->lane_num;
    }
    if (lane_num == 0) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_TX0_TX_DRIVERr(unit, pc, LANE0_ACCESS, &data16));
    } else if (lane_num == 1) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_TX1_TX_DRIVERr(unit, pc, LANE0_ACCESS, &data16));
    } else if (lane_num == 2) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_TX2_TX_DRIVERr(unit, pc, LANE0_ACCESS, &data16));
    } else if (lane_num == 3) {
        SOC_IF_ERROR_RETURN
            (READ_WC40_TX3_TX_DRIVERr(unit, pc, LANE0_ACCESS, &data16));
    } else {
        return SOC_E_PARAM;
    }
    *value = (data16 & mask) >> shifter;

    return SOC_E_NONE;
}

/*
 * Works for 10GX4 8B/10B endec in forced mode. May not work with scrambler 
 * enabled. Doesn't work for 1G/2.5G. Doesn't work for 10G XFI and 40G 
 * using 64/66 endec yet.
 */ 
STATIC int
_phy_wc40_control_bert_set(int unit, phy_ctrl_t *pc, int ctrl,
                                uint32 value)
{
    uint16 data16 = 0;
    uint16 mask16 = 0;
    int rv = SOC_E_NONE;

    switch (ctrl) {
        case SOC_PHY_CONTROL_BERT_PATTERN:   
            if (value == SOC_PHY_CONTROL_BERT_CRPAT) {
                data16 = TXBERT_TXBERTCTRL_CRPAT_EN_MASK;
            } else if (value == SOC_PHY_CONTROL_BERT_CJPAT) {
                data16 = TXBERT_TXBERTCTRL_CJPAT_EN_MASK;
            } else {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00,
                     data16,
                     TXBERT_TXBERTCTRL_CJPAT_EN_MASK |
                     TXBERT_TXBERTCTRL_CRPAT_EN_MASK));
            break;
    
        case SOC_PHY_CONTROL_BERT_RUN:    
            if (value) {
                /* bert test setup */
                /* enable |E| monitor  */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXB_ANARXCONTROL1Gr(unit,pc,0x00,
                             RXB_ANARXCONTROL1G_EMON_EN_MASK,
                             RXB_ANARXCONTROL1G_EMON_EN_MASK));

                /* with |E| monitor enabled, this selects 8B/10B endec status*/
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXB_ANARXCONTROLr(unit, pc, 0x00,
                            RXB_ANARXCONTROL_STATUS_SEL_prbsStatus, 
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));

                /* enable pattern generator and comparator */
                mask16 = XGXSBLK0_XGXSCONTROL_PGEN_EN_MASK |
                         XGXSBLK0_XGXSCONTROL_PCMP_EN_MASK;
                data16 = mask16;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00,
                          data16, mask16));

                /* enable CJ(R)PAT generator wiht Q_en and clear TX counter */
                mask16 = TXBERT_TXBERTCTRL_Q_EN_MASK |
                         TXBERT_TXBERTCTRL_Q_LINK_EN_MASK |
                         TXBERT_TXBERTCTRL_COUNT_CLR_MASK;
                data16 = mask16;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00, data16, mask16)); 

                /* re-enable the TX counter */
                data16 = 0;
                mask16 = TXBERT_TXBERTCTRL_COUNT_CLR_MASK;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00, data16, mask16)); 
 
                /* clear RX counters and enable RX CRC checker */ 
                mask16 = RXBERT_RXBERTCTRL_COUNT_CLR_MASK |
                         RXBERT_RXBERTCTRL_ERR_CLR_MASK |
                         RXBERT_RXBERTCTRL_CRCCHK_EN_MASK;
                data16 = mask16;               
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXBERT_RXBERTCTRLr(unit, pc, 0x00,
                         data16, mask16));
 
                /* re-enable the RX counters */
                data16 = 0;
                mask16 = RXBERT_RXBERTCTRL_COUNT_CLR_MASK |
                         RXBERT_RXBERTCTRL_ERR_CLR_MASK;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXBERT_RXBERTCTRLr(unit, pc, 0x00,
                         data16, mask16));

                /* enable packet transmission  */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00,
                         TXBERT_TXBERTCTRL_PKT_EN_MASK,
                         TXBERT_TXBERTCTRL_PKT_EN_MASK)); 
            } else {
                /* clear bert test setup */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXB_ANARXCONTROL1Gr(unit,pc,0x00,
                             0,
                             RXB_ANARXCONTROL1G_EMON_EN_MASK));

                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_RXB_ANARXCONTROLr(unit, pc, 0x00,
                            0,
                            RXB_ANARXCONTROL_STATUS_SEL_MASK));

                /* disable packet transmission  */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00,
                         0,
                         TXBERT_TXBERTCTRL_PKT_EN_MASK));

                /* this clears counters */
                /* disable pattern generator and comparator */
                /*SOC_IF_ERROR_RETURN
                 *  (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00,
                 *        0,
                 *        XGXSBLK0_XGXSCONTROL_PGEN_EN_MASK |
                 *        XGXSBLK0_XGXSCONTROL_PCMP_EN_MASK));
                 */
            }
            break;

        case SOC_PHY_CONTROL_BERT_PACKET_SIZE:
            break;
        case SOC_PHY_CONTROL_BERT_IPG:
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

STATIC int
_phy_wc40_control_bert_get(int unit, phy_ctrl_t *pc, int ctrl,
                                uint32 *value)
{
    uint16 data16;
    int rv = SOC_E_NONE;

    switch (ctrl) {
        case SOC_PHY_CONTROL_BERT_TX_PACKETS:
            SOC_IF_ERROR_RETURN
                (READ_WC40_TXBERT_TXBERTPACKETUr(unit,pc,0x00,&data16));
            *value = data16 << 16;
            SOC_IF_ERROR_RETURN
                (READ_WC40_TXBERT_TXBERTPACKETLr(unit,pc,0x00,&data16));
            *value |= data16;
            break;

        case SOC_PHY_CONTROL_BERT_RX_PACKETS:
            SOC_IF_ERROR_RETURN
                (READ_WC40_RXBERT_RXBERTPACKETUr(unit,pc,0x00,&data16));
            *value = data16 << 16;
            SOC_IF_ERROR_RETURN
                (READ_WC40_RXBERT_RXBERTPACKETLr(unit,pc,0x00,&data16));
            *value |= data16;
            break;

        case SOC_PHY_CONTROL_BERT_RX_ERROR_BITS:
            SOC_IF_ERROR_RETURN
                (READ_WC40_RXBERT_RXBERTBITERRr(unit,pc,0x00,&data16));
            *value = data16;
            break;

        case SOC_PHY_CONTROL_BERT_RX_ERROR_BYTES:
            SOC_IF_ERROR_RETURN
                (READ_WC40_RXBERT_RXBERTBYTEERRr(unit,pc,0x00,&data16));
            *value = data16;
            break;

        case SOC_PHY_CONTROL_BERT_RX_ERROR_PACKETS:
            SOC_IF_ERROR_RETURN
                (READ_WC40_RXBERT_RXBERTPKTERRr(unit,pc,0x00,&data16));
            *value = data16;
            break;
        case SOC_PHY_CONTROL_BERT_PATTERN:
            SOC_IF_ERROR_RETURN
                (READ_WC40_TXBERT_TXBERTCTRLr(unit, pc, 0x00, &data16));
            if (data16 & TXBERT_TXBERTCTRL_CRPAT_EN_MASK) {
                *value = SOC_PHY_CONTROL_BERT_CRPAT;
            } else {
                *value = SOC_PHY_CONTROL_BERT_CJPAT;
            }
            break;
        case SOC_PHY_CONTROL_BERT_PACKET_SIZE:
            break;
        case SOC_PHY_CONTROL_BERT_IPG:
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

#ifdef WC_EEE_SUPPORT     
STATIC int
_phy_wc40_control_eee_set(int unit, phy_ctrl_t *pc, 
                          soc_phy_control_t type,uint32 enable)
{
    uint16 data0;
    uint16 mask0;
    uint16 data1;
    uint16 mask1;

    switch (type) { 
        case SOC_PHY_CONTROL_EEE:
            mask0 = EEE_USERB0_EEE_COMBO_CONTROL0_EEE_EN_FORCE_VAL_MASK |
                     EEE_USERB0_EEE_COMBO_CONTROL0_EEE_EN_FORCE_MASK |
                     EEE_USERB0_EEE_COMBO_CONTROL0_LPI_EN_FORCE_MASK;
            if (enable == 1) {
                data0 = mask0;
                /* enable EEE, disable the LPI pass-thru */
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DIGITAL4_MISC5r(unit,pc,0x00, 0,
                                (1 << 14) | (1 << 15)));
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_XGXSBLK7_EEECONTROLr(unit,pc,0x00,0,
                                XGXSBLK7_EEECONTROL_LPI_EN_TX_MASK |
                                XGXSBLK7_EEECONTROL_LPI_EN_RX_MASK));
            } else if (enable == 2) {
                /* enable the LPI pass-thru */
                data0 = mask0; 
                mask1 = (1 << 14) | (1 << 15);
                data1 = mask1;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_DIGITAL4_MISC5r(unit,pc,0x00, data1, mask1));
                mask1 = XGXSBLK7_EEECONTROL_LPI_EN_TX_MASK |
                        XGXSBLK7_EEECONTROL_LPI_EN_RX_MASK;
                data1 = mask1;
                SOC_IF_ERROR_RETURN
                    (MODIFY_WC40_XGXSBLK7_EEECONTROLr(unit,pc,0x00,data1,mask1));
            } else {
                /* disable EEE */
                data0 = EEE_USERB0_EEE_COMBO_CONTROL0_EEE_EN_FORCE_MASK;
            }
            /* forced EEE  */
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_EEE_USERB0_EEE_COMBO_CONTROL0r(unit,pc,0x00,
                      data0, mask0));
            break;

        case SOC_PHY_CONTROL_EEE_AUTO:
            /* EEE through autoneg */
            mask0 = EEE_USERB0_EEE_COMBO_CONTROL0_EEE_EN_FORCE_MASK |
                     EEE_USERB0_EEE_COMBO_CONTROL0_LPI_EN_FORCE_MASK;
            if (enable == 1) {
                data0 = 0;
            } else {
                data0 = mask0;
            }
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_EEE_USERB0_EEE_COMBO_CONTROL0r(unit,pc,0x00,data0,
                      mask0));
            break;

    default:
         /* should never get here */
         return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}
#endif

STATIC int 
_phy_wc40_rloop_set(int unit, phy_ctrl_t *pc, uint32 enable)
{
    uint16 mask16;
    uint16 data16;
    int speed;
    int intf;
    int scr;
    int rv;

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        rv = _phy_wc40_combo_speed_get(unit, pc->port, &speed,&intf,&scr);
    } else {
        rv = _phy_wc40_ind_speed_get(unit, pc->port, &speed,&intf,&scr);
    }

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        if (speed <= 2500) {
            mask16 = SERDESDIGITAL_CONTROL1000X1_REMOTE_LOOPBACK_MASK;
            data16 = enable? mask16: 0;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit,pc,0x00, data16,mask16));
        } else if (speed == 10000) { /* XFI/SFI/KR */
            mask16 = TX66_CONTROL_RLOOP_EN_MASK;
            data16 = enable? mask16: 0;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TX66_CONTROLr(unit,pc,0x00, data16,mask16));

            /* need to enable RX clock compensation which is done already in the init */
        } else if (speed > 10000) { /* dxgxs mode */
            mask16 = XGXSBLK0_XGXSCONTROL_RLOOP_MASK;
            data16 = enable? mask16: 0;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00, data16,mask16));
            /* clock compensation should be already enabled in init */
        }
    } else { /* combo mode */
        mask16 = XGXSBLK0_XGXSCONTROL_RLOOP_MASK;
        data16 = enable? mask16:0;

        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00, data16,mask16));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_rloop_get(int unit, phy_ctrl_t *pc, uint32 *value)
{
    uint16 mask16;
    uint16 data16;

    if (IND_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        mask16 = 1 << (XGXSBLK1_LANECTRL2_RLOOP1G_SHIFT + pc->lane_num);
        SOC_IF_ERROR_RETURN
            (READ_WC40_XGXSBLK1_LANECTRL2r(unit,pc,0x00, &data16));
        *value = (mask16 & data16)? TRUE: FALSE;

    } else {
        mask16 = XGXSBLK0_XGXSCONTROL_RLOOP_MASK;
        SOC_IF_ERROR_RETURN
            (READ_WC40_XGXSBLK0_XGXSCONTROLr(unit,pc,0x00, &data16));
        *value = (mask16 & data16)? TRUE: FALSE;
    }
    return SOC_E_NONE;
}

STATIC int
_phy_wc40_fec_set(int unit, phy_ctrl_t *pc, uint32 fec_ctrl)
{
    int rv = SOC_E_NONE;
    uint16 mask16;
    uint16 data16;

    mask16 = CL74_USERB0_UFECCONTROL3_FEC_ENABLE_OVR_VAL_MASK |
             CL74_USERB0_UFECCONTROL3_FEC_ENABLE_OVR_MASK;
    switch (fec_ctrl) {
        case SOC_PHY_CONTROL_FEC_OFF:
            data16 = CL74_USERB0_UFECCONTROL3_FEC_ENABLE_OVR_MASK;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_CL74_USERB0_UFECCONTROL3r(unit,pc,0x00,data16,mask16));
            break;
        case SOC_PHY_CONTROL_FEC_ON:
            data16 = mask16;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_CL74_USERB0_UFECCONTROL3r(unit,pc,0x00,data16,mask16));
            break;
        case SOC_PHY_CONTROL_FEC_AUTO:
            /* turn off forced override */
            data16 = 0; 
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_CL74_USERB0_UFECCONTROL3r(unit,pc,0x00,data16,mask16));

            /* configure the CL73 FEC advertisement */
            mask16 = AN_IEEE1BLK_AN_ADVERTISEMENT2_FEC_REQUESTED_MASK;
            data16 = mask16;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_AN_IEEE1BLK_AN_ADVERTISEMENT2r(unit, pc, 0x00, data16,
                                     mask16));
            /* configure the CL37 FEC advertisement */
            data16 = mask16 = DIGITAL3_UP3_FEC_MASK;
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_DIGITAL3_UP3r(unit, pc, 0x00, data16, mask16));

            break;
        default:
            rv = SOC_E_PARAM;
            break;
    } 
    return rv;
}

STATIC int
_phy_wc40_fec_get(int unit, phy_ctrl_t *pc, uint32 *value)
{
    uint16 data16;

    SOC_IF_ERROR_RETURN
        (READ_WC40_CL74_USERB0_UFECCONTROL3r(unit,pc,0x00,&data16));
    if (data16 & CL74_USERB0_UFECCONTROL3_FEC_ENABLE_OVR_MASK) {
        *value = data16 & CL74_USERB0_UFECCONTROL3_FEC_ENABLE_OVR_VAL_MASK?
                 SOC_PHY_CONTROL_FEC_ON:
                 SOC_PHY_CONTROL_FEC_OFF; 
    } else {
        *value = SOC_PHY_CONTROL_FEC_AUTO;
    }
    return SOC_E_NONE;         
}

STATIC int
_phy_wc40_control_linkdown_transmit_set(int unit, soc_port_t port, uint32 value)
{
    uint16      ctrl2_data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    if (value) {
        ctrl2_data = (SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_SHIFT) |
                     (SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_BITS  <<
                      SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_SHIFT) |
                     (SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_SHIFT);
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, ctrl2_data));

    } else {
        ctrl2_data = (SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_SHIFT) |
                     (SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_SHIFT) |
                     (SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_SHIFT);
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, ctrl2_data));
    }

    return SOC_E_NONE;
}
STATIC int
_phy_wc40_control_linkdown_transmit_get(int unit, soc_port_t port, uint32 *value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, &data));

    /* Check if FORCE_XMIT_DATA_ON_TXSIDE is set */
    *value = (data & (SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_BITS <<
                      SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_SHIFT))
             ? 1 : 0;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_control_set
 * Purpose:
 *      Configure PHY device specific control fucntion. 
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #. 
 *      type  - Control to update 
 *      value - New setting for the control 
 * Returns:     
 *      SOC_E_NONE
 */
STATIC int
phy_wc40_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    int rv;
    phy_ctrl_t *pc;

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }
   
    pc = INT_PHY_SW_STATE(unit, port);
    rv = SOC_E_UNAVAIL;
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS:
        rv = _phy_wc40_control_preemphasis_set(unit, pc, type, value);
        break;

    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        rv = _phy_wc40_control_tx_driver_set(unit, pc, type, value);
        break;
    case SOC_PHY_CONTROL_PRBS_POLYNOMIAL:
        rv = _phy_wc40_control_prbs_polynomial_set(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA:
        rv = _phy_wc40_control_prbs_tx_invert_data_set(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_ENABLE:
        /* TX_ENABLE does both tx and rx */
        rv = _phy_wc40_control_prbs_enable_set(unit, port, value);
        break; 
    case SOC_PHY_CONTROL_PRBS_RX_ENABLE:
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_8B10B:
        rv = _phy_wc40_control_8b10b_set(unit, port, value);
        break;
#ifdef WC_EEE_SUPPORT
    case SOC_PHY_CONTROL_EEE:
        /* fall through */
    case SOC_PHY_CONTROL_EEE_AUTO:
        rv = _phy_wc40_control_eee_set(unit,pc,type,value);
        break;
#endif
    case SOC_PHY_CONTROL_LOOPBACK_REMOTE:
        rv = _phy_wc40_rloop_set(unit,pc,value);
        break;
    case SOC_PHY_CONTROL_FORWARD_ERROR_CORRECTION:
        rv = _phy_wc40_fec_set(unit,pc,value);
        break;
    /* XXX obsolete */
    case SOC_PHY_CONTROL_CUSTOM1:
        DEV_CFG_PTR(pc)->custom1 = value? TRUE: FALSE;
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_SCRAMBLER:
        DEV_CFG_PTR(pc)->scrambler_en = value? TRUE: FALSE;
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        rv = _phy_wc40_control_linkdown_transmit_set(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PARALLEL_DETECTION:
        /* enable 1000X parallel detect */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00,
              value? SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK:0,
              SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK));
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_BERT_PATTERN:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_RUN:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_PACKET_SIZE:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_IPG:
        rv = _phy_wc40_control_bert_set(unit,pc,type,value);
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break; 
    }
    return rv;
}

/*
 * Function:
 *      phy_wc40_control_get
 * Purpose:
 *      Get current control settign of the PHY. 
 * Parameters:
 *      unit  - StrataSwitch unit #.
 *      port  - StrataSwitch port #. 
 *      type  - Control to update 
 *      value - (OUT)Current setting for the control 
 * Returns:     
 *      SOC_E_NONE
 */
STATIC int
phy_wc40_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    int rv;
    uint16 data16;
    int intf;
    int scr;
    int speed;
    phy_ctrl_t *pc;

    if (NULL == value) {
        return SOC_E_PARAM;
    }

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }
   
    pc = INT_PHY_SW_STATE(unit, port);
    rv = SOC_E_UNAVAIL;
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PREEMPHASIS:
        rv = _phy_wc40_control_preemphasis_get(unit, pc, type, value);
        break;

    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE0:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE1:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE2:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT_LANE3:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        rv = _phy_wc40_control_tx_driver_get(unit, pc, type, value);
        break;
    case SOC_PHY_CONTROL_PRBS_POLYNOMIAL:
        rv = _phy_wc40_control_prbs_polynomial_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA:
        rv = _phy_wc40_control_prbs_tx_invert_data_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_TX_ENABLE:
        /* fall through */
    case SOC_PHY_CONTROL_PRBS_RX_ENABLE:
        rv = _phy_wc40_control_prbs_enable_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PRBS_RX_STATUS:
        rv = _phy_wc40_control_prbs_rx_status_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_LOOPBACK_REMOTE:
        rv = _phy_wc40_rloop_get(unit,pc,value);
        break;
    case SOC_PHY_CONTROL_FORWARD_ERROR_CORRECTION:
        rv = _phy_wc40_fec_get(unit,pc,value);
        break;
    case SOC_PHY_CONTROL_CUSTOM1:
        *value = DEV_CFG_PTR(pc)->custom1;
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_SCRAMBLER:
        if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
            rv = _phy_wc40_combo_speed_get(unit, port, &speed,&intf,&scr);
        } else {
            rv = _phy_wc40_ind_speed_get(unit, port, &speed,&intf,&scr);
        }
        *value = scr;
        break;
    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        rv = _phy_wc40_control_linkdown_transmit_get(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PARALLEL_DETECTION:
        SOC_IF_ERROR_RETURN
            (READ_WC40_SERDESDIGITAL_CONTROL1000X2r(unit, pc, 0x00, &data16));
        *value = data16 & SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK?
                 TRUE: FALSE;
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_BERT_TX_PACKETS:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_RX_PACKETS:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_RX_ERROR_BITS:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_RX_ERROR_BYTES:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_RX_ERROR_PACKETS:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_PATTERN:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_PACKET_SIZE:
        /* fall through */
    case SOC_PHY_CONTROL_BERT_IPG:
        rv = _phy_wc40_control_bert_get(unit,pc,type,value);
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break; 
    }
    return rv;
}     

typedef struct {
    uint16 start;
    uint16 end;
} WC40_LANE0_REG_BLOCK;

STATIC WC40_LANE0_REG_BLOCK wc40_lane0_reg_block[] = {
    {0x8050,0x80FE},  /* register in this block only valid at lane0 */
    {0x8000,0x8001},  /* valid at lane0 */
    {0x8015,0x8019}  /* valid at lane0 */
};

STATIC int
_wc40_lane0_reg_access(int unit, phy_ctrl_t *pc,uint16 reg_addr)
{
    int ix = 0;
    int num_blk;
    WC40_LANE0_REG_BLOCK * pBlk;

    num_blk = sizeof(wc40_lane0_reg_block)/sizeof(wc40_lane0_reg_block[0]);
    for (ix = 0; ix < num_blk; ix++) {
        pBlk = &wc40_lane0_reg_block[ix];
        if ((reg_addr >= pBlk->start) && (reg_addr <= pBlk->end)) {
            return TRUE;
        }
    }
    return FALSE;
}

STATIC int
phy_wc40_reg_aer_read(int unit, phy_ctrl_t *pc, uint32 flags,
                  uint32 phy_reg_addr, uint16 *phy_data)
{
    uint16 data;     
    uint32 lane;         /* lane to access, override the default lane */

    lane = flags & LANE_ACCESS_MASK;

    /* safety check */
    if ((lane > 4) || (lane == LANE_BCST)) {
        lane = LANE0_ACCESS;
    }

    /* check if register only available on lane0 */
    if (_wc40_lane0_reg_access(unit,pc,(uint16)(phy_reg_addr & 0xffff))) {
        lane = LANE0_ACCESS;
    }

    /* default lane override
     * register access is done in AER mode
     */
    if (lane) { 
        /* set the base address in multi mdio address mode */
        if (!(pc->flags & PHYCTRL_MDIO_ADDR_SHARE)) {
            pc->phy_id -= pc->lane_num;
        }
        phy_reg_addr |= ((lane - 1) << 16);
        
    } else { /* default lane access */ 
        if (pc->flags & PHYCTRL_MDIO_ADDR_SHARE) {
            phy_reg_addr |= (pc->lane_num << 16);
        }
    }

    /* don't set the lane number if it is access to AER ifself */
    if ((phy_reg_addr & 0xffff) == PHY_AER_REG) {
        phy_reg_addr &= 0xffff;
    }

    SOC_IF_ERROR_RETURN
        (phy_reg_aer_read(unit, pc,  phy_reg_addr, &data));

    /* restore the mdio address for current lane in multi mdio address mode */
    if (lane) {
        if (!(pc->flags & PHYCTRL_MDIO_ADDR_SHARE)) {
            pc->phy_id += pc->lane_num;
        }
    }
    *phy_data = data;

    return SOC_E_NONE;
}

STATIC int
phy_wc40_reg_aer_write(int unit, phy_ctrl_t *pc, uint32 flags,
                   uint32 phy_reg_addr, uint16 phy_data)
{
    uint16               data;     
    int dxgxs;
    uint32 lane;

    dxgxs = DEV_CFG_PTR(pc)->dxgxs;
    data  = (uint16) (phy_data & 0x0000FFFF);

    lane = flags & LANE_ACCESS_MASK;

    /* safety check */
    if ((lane > 4) && (lane != LANE_BCST)) {
        lane = LANE0_ACCESS;
    }

    /* check if register only available on lane0 or should be done from lane0 */
    if ((_wc40_lane0_reg_access(unit,pc,(uint16)(phy_reg_addr & 0xffff))) || dxgxs) {
        lane = LANE0_ACCESS;
    }

    /* default lane override
     * register access is done in AER mode
     */
    if (lane) {
        /* set AER to broadcast two lanes, dxgxs=1 1st dxgxs, dxgxs=2 2nd  */
        if (dxgxs) {
            phy_reg_addr |= (WC_AER_BCST_OFS_STRAP + dxgxs) << 16;
        } else {
            if (lane == LANE_BCST) {
                phy_reg_addr |= (WC_AER_BCST_OFS_STRAP << 16);
            } else {
                phy_reg_addr |= ((lane - 1) << 16);
            }
        }
        if (!(pc->flags & PHYCTRL_MDIO_ADDR_SHARE)) {
            pc->phy_id -= pc->lane_num;
        }
    } else {
        if (pc->flags & PHYCTRL_MDIO_ADDR_SHARE) {
            phy_reg_addr |= (pc->lane_num << 16);
        }
    }

    /* don't set the lane number if it is access to AER ifself */
    if ((phy_reg_addr & 0xffff) == PHY_AER_REG) {
        phy_reg_addr &= 0xffff;
    }

    SOC_IF_ERROR_RETURN
        (phy_reg_aer_write(unit, pc,  phy_reg_addr, data));

    /* restore the mdio address for current lane in multi mdio address mode */
    if (lane) {
        if (!(pc->flags & PHYCTRL_MDIO_ADDR_SHARE)) {
            pc->phy_id += pc->lane_num;
        }
    }

    return SOC_E_NONE;
}

STATIC int
phy_wc40_reg_aer_modify(int unit, phy_ctrl_t *pc, uint32 flags,
                    uint32 phy_reg_addr, uint16 phy_data,
                    uint16 phy_data_mask)
{
    uint16               data;     /* Temporary holder for phy_data */
    uint16               mask;
    uint16  tmp;

    data      = (uint16) (phy_data & 0x0000FFFF);
    mask      = (uint16) (phy_data_mask & 0x0000FFFF);

    SOC_IF_ERROR_RETURN
        (phy_wc40_reg_aer_read(unit, pc,  flags, phy_reg_addr, &tmp));

    data &= mask;
    tmp &= ~(mask);
    tmp |= data;

    SOC_IF_ERROR_RETURN
        (phy_wc40_reg_aer_write(unit, pc,  flags, phy_reg_addr, tmp));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_reg_read
 * Purpose:
 *      Routine to read PHY register
 * Parameters:
 *      uint         - BCM unit number
 *      pc           - PHY state
 *      flags        - Flags which specify the register type
 *      phy_reg_addr - Encoded register address
 *      phy_data     - (OUT) Value read from PHY register
 * Note:
 *      This register read function is not thread safe. Higher level
 * function that calls this function must obtain a per port lock
 * to avoid overriding register page mapping between threads.
 */
STATIC int
phy_wc40_reg_read(int unit, soc_port_t port, uint32 flags,
                  uint32 phy_reg_addr, uint32 *phy_data)
{
    uint16               data;     /* Temporary holder for phy_data */
    phy_ctrl_t          *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_wc40_reg_aer_read(unit, pc, 0x00, phy_reg_addr, &data));

    *phy_data = data;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_reg_write
 * Purpose:
 *      Routine to write PHY register
 * Parameters:
 *      uint         - BCM unit number
 *      pc           - PHY state
 *      flags        - Flags which specify the register type
 *      phy_reg_addr - Encoded register address
 *      phy_data     - Value write to PHY register
 * Note:
 *      This register read function is not thread safe. Higher level
 * function that calls this function must obtain a per port lock
 * to avoid overriding register page mapping between threads.
 */
STATIC int
phy_wc40_reg_write(int unit, soc_port_t port, uint32 flags,
                   uint32 phy_reg_addr, uint32 phy_data)
{
    uint16               data;     /* Temporary holder for phy_data */
    phy_ctrl_t          *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    data      = (uint16) (phy_data & 0x0000FFFF);

    SOC_IF_ERROR_RETURN
        (phy_wc40_reg_aer_write(unit, pc, 0x00, phy_reg_addr, data));

    return SOC_E_NONE;
}  

/*
 * Function:
 *      phy_wc40_reg_modify
 * Purpose:
 *      Routine to write PHY register
 * Parameters:
 *      uint         - BCM unit number
 *      pc           - PHY state
 *      flags        - Flags which specify the register type
 *      phy_reg_addr - Encoded register address
 *      phy_mo_data  - New value for the bits specified in phy_mo_mask
 *      phy_mo_mask  - Bit mask to modify
 * Note:
 *      This register read function is not thread safe. Higher level
 * function that calls this function must obtain a per port lock
 * to avoid overriding register page mapping between threads.
 */
STATIC int
phy_wc40_reg_modify(int unit, soc_port_t port, uint32 flags,
                    uint32 phy_reg_addr, uint32 phy_data,
                    uint32 phy_data_mask)
{
    uint16               data;     /* Temporary holder for phy_data */
    uint16               mask;
    phy_ctrl_t    *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    data      = (uint16) (phy_data & 0x0000FFFF);
    mask      = (uint16) (phy_data_mask & 0x0000FFFF);

    SOC_IF_ERROR_RETURN
        (phy_wc40_reg_aer_modify(unit, pc, 0x00, phy_reg_addr, data, mask));

    return SOC_E_NONE;
}

STATIC int
phy_wc40_probe(int unit, phy_ctrl_t *pc)
{
    uint16      serdes_id0;

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESID_SERDESID0r(unit, pc, 0x00, &serdes_id0));

    if ((serdes_id0 & SERDESID_SERDESID0_MODEL_NUMBER_MASK) !=
             SERDES_ID0_MODEL_NUMBER_WARPCORE) {
        return SOC_E_NOT_FOUND;
    }

    /* ask to allocate the driver specific descripor  */
    pc->size = sizeof(WC40_DEV_DESC_t);
    pc->dev_name = wc_device_name;
    return SOC_E_NONE;
}

STATIC int
phy_wc40_notify(int unit, soc_port_t port,
                       soc_phy_event_t event, uint32 value)
{
    int             rv;
    phy_ctrl_t    *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    if (event >= phyEventCount) {
        return SOC_E_PARAM;
    }

    if (CUSTOM_MODE(pc)) {
        return SOC_E_NONE;
    }

    rv = SOC_E_NONE;
    switch(event) {
    case phyEventInterface:
        rv = (_phy_wc40_notify_interface(unit, port, value));
        break;
    case phyEventDuplex:
        rv = (_phy_wc40_notify_duplex(unit, port, value));
        break;
    case phyEventSpeed:
        rv = (_phy_wc40_notify_speed(unit, port, value));
        break;
    case phyEventStop:
        rv = (_phy_wc40_notify_stop(unit, port, value));
        break;
    case phyEventResume:
        rv = (_phy_wc40_notify_resume(unit, port, value));
        break;
    case phyEventAutoneg:
        rv = (phy_wc40_an_set(unit, port, value));
        break;
    case phyEventTxFifoReset:
        rv = (_phy_wc40_tx_fifo_reset(unit, port, value));
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }

    return rv;
}

STATIC int
phy_wc40_linkup_evt (int unit, soc_port_t port)
{
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);
    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_wc40_linkup_evt: "
                     "u=%d p=%d\n", unit, port));

    /* TX fifo recenter workaround in aggregated mode */
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode) || IS_DUAL_LANE_PORT(pc)) {
        if (WC40_REVID_A0(pc) || WC40_REVID_A1(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit,pc,0x00,
                       TXBERT_TXBERTCTRL_FIFO_RST_MASK,
                       TXBERT_TXBERTCTRL_FIFO_RST_MASK));
            SOC_IF_ERROR_RETURN
                (MODIFY_WC40_TXBERT_TXBERTCTRLr(unit,pc,0x00,
                       0,
                       TXBERT_TXBERTCTRL_FIFO_RST_MASK));
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_duplex_set
 * Purpose:
 *      Set the current duplex mode (forced).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - Boolean, true indicates full duplex, false indicates half.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_wc40_duplex_set(int unit, soc_port_t port, int duplex)
{
    uint16       data16;
    phy_ctrl_t  *pc;
    int duplex_value;

    pc = INT_PHY_SW_STATE(unit, port);
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode) ||
        CUSTOMX_MODE(pc)) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_STATUS1000X1r(unit, pc, 0x00, &data16));

    if (!(data16 & SERDESDIGITAL_STATUS1000X1_SGMII_MODE_MASK)) {

        /* 1000X fiber mode, 100fx  */
            
        duplex_value = duplex? FX100_CONTROL1_FULL_DUPLEX_MASK:0;
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_FX100_CONTROL1r(unit,pc,0x00,
                   duplex_value,
                   FX100_CONTROL1_FULL_DUPLEX_MASK));
        
        /* 1000X should always be full duplex */
        duplex = TRUE;
    }

    data16 = duplex? MII_CTRL_FD: 0;

    /* program the duplex setting */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, data16,MII_CTRL_FD));

    return SOC_E_NONE;
}

STATIC int
phy_wc40_duplex_get(int unit, soc_port_t port, int *duplex)
{
    uint16       reg0_16;
    uint16       mii_ctrl;
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode) ||
        CUSTOMX_MODE(pc)) {
        *duplex = TRUE;
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_STATUS1000X1r(unit, pc, 0x00, &reg0_16));

    /* default to fiber mode duplex */
    *duplex = (reg0_16 & SERDESDIGITAL_STATUS1000X1_DUPLEX_STATUS_MASK)?
              TRUE:FALSE;

    if (reg0_16 & SERDESDIGITAL_STATUS1000X1_SGMII_MODE_MASK) {

    /* retrieve the duplex setting in SGMII mode */
        SOC_IF_ERROR_RETURN
            (READ_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, &mii_ctrl));

        if (mii_ctrl & MII_CTRL_AE) {
            SOC_IF_ERROR_RETURN
                (READ_WC40_COMBO_IEEE0_AUTONEGLPABILr(unit,pc,0x00,&reg0_16));

            /* make sure link partner is also in SGMII mode
             * otherwise fall through to use the FD bit in MII_CTRL reg
             */
            if (reg0_16 & MII_ANP_SGMII_MODE) {
                if (reg0_16 & MII_ANP_SGMII_FD) {
                    *duplex = TRUE;
                } else {
                    *duplex = FALSE;
                }
                return SOC_E_NONE;
            }
        }
        *duplex = (mii_ctrl & MII_CTRL_FD) ? TRUE : FALSE;
    } 

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_firmware_set
 * Purpose:
 *      write the given firmware to the uController's RAM 
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      offset - offset to the data stream
 *      array  - the given data
 *      datalen- the data length
 * Returns:
 *      SOC_E_NONE
 */

STATIC int
phy_wc40_firmware_set(int unit, int port, int offset, uint8 *array,int datalen)
{
    return SOC_E_FAIL;
}

STATIC int
phy_wc40_firmware_load(int unit, int port, int offset, uint8 *array,int datalen)
{
    phy_ctrl_t  *pc;
    int rv;
    int i;
    int len;
    uint16 data16;
    uint16 mask16;
    uint16 ver;
    uint16 cksum = 0;
    int no_cksum;

    pc = INT_PHY_SW_STATE(unit, port);
    no_cksum = !(DEV_CFG_PTR(pc)->uc_cksum);

    rv = SOC_E_NONE;

    /* initialize the RAM */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_MICROBLK_COMMANDr(unit, pc, 0x00,
                                      MICROBLK_COMMAND_INIT_CMD_MASK));

   /* wait for init done */
    rv = _phy_wc40_regbit_set_wait_check(pc,WC40_MICROBLK_DOWNLOAD_STATUSr,
                     MICROBLK_DOWNLOAD_STATUS_INIT_DONE_MASK,1,2000000,0);

    if (rv == SOC_E_TIMEOUT) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "WC40 : uC init fails: u=%d p=%d\n",
                        unit, port));
        return (SOC_E_TIMEOUT);
    }

    /* enable uC timers */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_MICROBLK_COMMAND2r(unit,pc,0x00,MICROBLK_COMMAND2_TMR_EN_MASK,
                    MICROBLK_COMMAND2_TMR_EN_MASK));

   /* starting RAM location */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_MICROBLK_ADDRESSr(unit, pc, 0x00, offset));

    if ((_phy_wc40_firmware_set_helper != NULL) && (DEV_CFG_PTR(pc)->load_mthd == 2)) {

        /* transfer size, 16bytes quantity*/
        if (datalen%16) {
            len = (((datalen/16)+1) * 16) -1;
        } else {
            len = datalen -1;
        }
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_MICROBLK_RAMWORDr(unit, pc, 0x00, len));

        /* allow external access from Warpcore */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_MICROBLK_COMMAND3r(unit, pc, 0x00,
                            MICROBLK_COMMAND3_EXT_MEM_ENABLE_MASK,
                            MICROBLK_COMMAND3_EXT_MEM_ENABLE_MASK));
        WC40_UDELAY(1000);
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_MICROBLK_COMMAND3r(unit, pc, 0x00,
                            MICROBLK_COMMAND3_EXT_CLK_ENABLE_MASK,
                            MICROBLK_COMMAND3_EXT_CLK_ENABLE_MASK));
        WC40_UDELAY(1000);

        rv = _phy_wc40_firmware_set_helper(unit,port,array,datalen);

        /* restore back no external access from Warpcore */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_MICROBLK_COMMAND3r(unit, pc, 0x00, 0,
                            MICROBLK_COMMAND3_EXT_MEM_ENABLE_MASK));
        WC40_UDELAY(1000);
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_MICROBLK_COMMAND3r(unit, pc, 0x00, 0,
                            MICROBLK_COMMAND3_EXT_CLK_ENABLE_MASK));
        WC40_UDELAY(1000);

    } else {
        /* transfer size */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_MICROBLK_RAMWORDr(unit, pc, 0x00, datalen - 1));

        /* start write operation */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_MICROBLK_COMMANDr(unit, pc, 0x00,
                 MICROBLK_COMMAND_WRITE_MASK | MICROBLK_COMMAND_RUN_MASK));


        /* write 16-bit word to data register */
        for (i = 0; i < datalen/2; i++) {
            data16 = array[2*i] | (array[2*i+1] << 8);
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_MICROBLK_WRDATAr(unit, pc, 0x00, data16));
        }

        /* check if the firmware size is odd number */
        if (datalen%2) {
            data16 = array[datalen-1];
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_MICROBLK_WRDATAr(unit, pc, 0x00, data16));
        }
        /* complete the writing */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_MICROBLK_COMMANDr(unit, pc, 0x00, MICROBLK_COMMAND_STOP_MASK));
    }

    SOC_IF_ERROR_RETURN
            (READ_WC40_MICROBLK_DOWNLOAD_STATUSr(unit, pc, 0x00, &data16));

    mask16 = MICROBLK_DOWNLOAD_STATUS_ERR0_MASK | MICROBLK_DOWNLOAD_STATUS_ERR1_MASK;

    if (data16 & mask16) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "WC40 : uC RAM download fails: u=%d p=%d\n",
                        unit, port));
        return (SOC_E_FAIL);
    }

    /* write a non-zero value to this register to signal uC not perform cksum calculation */
    if (no_cksum) {
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_UC_INFO_B1_CRCr(unit, pc, 0x00, 0x1234));
    }

    /* release uC's reset */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_MICROBLK_COMMANDr(unit, pc, 0x00,
                            MICROBLK_COMMAND_MDIO_UC_RESET_N_MASK));

    /* wait for checksum to be written to this register by uC */

    if (!no_cksum) {
        rv = _phy_wc40_regbit_set_wait_check(pc,0x81fe,0xffff,1,100000,0);
        if (rv == SOC_E_TIMEOUT) {
            SOC_DEBUG_PRINT((DK_WARN,
                    "WC40 : uC download: u=%d p=%d timeout: wait for checksum\n",
                        unit, port));
        } else {
           SOC_IF_ERROR_RETURN
            (READ_WC40_UC_INFO_B1_CRCr(unit, pc, 0x00, &cksum));
        } 
    } 

    SOC_IF_ERROR_RETURN
        (READ_WC40_UC_INFO_B1_VERSIONr(unit, pc, 0x00, &ver));

    SOC_DEBUG_PRINT((DK_VERBOSE,
               "WC40 : uC RAM download success: u=%d p=%d ver=%x", unit, port,ver));

    if (!no_cksum) {
        SOC_DEBUG_PRINT((DK_VERBOSE,
               " cksum=0x%x\n", cksum));
    } else {
        SOC_DEBUG_PRINT((DK_VERBOSE, "\n"));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_medium_config_set
 * Purpose:
 *      set the configured medium the device is operating on.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - not used
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
phy_wc40_medium_config_set(int unit, soc_port_t port,
                           soc_port_medium_t  medium,
                           soc_phy_config_t  *cfg)
{
    phy_ctrl_t       *pc;

    pc            = INT_PHY_SW_STATE(unit, port);

    switch (medium) {
    case SOC_PORT_MEDIUM_COPPER:
        DEV_CFG_PTR(pc)->medium = SOC_PORT_MEDIUM_COPPER;
        break;
    case SOC_PORT_MEDIUM_FIBER:
        DEV_CFG_PTR(pc)->medium = SOC_PORT_MEDIUM_FIBER;
        break;
    default:
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}
/*
 * Function:
 *      phy_wc40_medium_get
 * Purpose:
 *      Indicate the configured medium
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      medium - (OUT) One of:
 *              SOC_PORT_MEDIUM_COPPER
 *              SOC_PORT_MEDIUM_FIBER
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
phy_wc40_medium_get(int unit, soc_port_t port, soc_port_medium_t *medium)
{
    phy_ctrl_t       *pc;

    if (medium == NULL) {
        return SOC_E_PARAM;
    }

    pc            = INT_PHY_SW_STATE(unit, port);
    *medium = DEV_CFG_PTR(pc)->medium;
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_wc40_master_get
 * Purpose:
 *      this function is meant for 1000Base-T PHY. Added here to support
 *      internal PHY regression test
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      master - (OUT) SOC_PORT_MS_*
 * Returns:
 *      SOC_E_NONE
 * Notes:
 *      The master mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_wc40_master_get(int unit, soc_port_t port, int *master)
{
    *master = SOC_PORT_MS_NONE;
    return SOC_E_NONE;
}

/*
 * Variable:
 *      phy_wc40_drv
 * Purpose:
 *      Phy Driver for 10G (XAUI x 4) Serdes PHY. 
 */
phy_driver_t phy_wc40_hg = {
    /* .drv_name                      = */ "Warpcore PHY Driver",
    /* .pd_init                       = */ phy_wc40_init,
    /* .pd_reset                      = */ phy_null_reset,
    /* .pd_link_get                   = */ phy_wc40_link_get,
    /* .pd_enable_set                 = */ phy_wc40_enable_set,
    /* .pd_enable_get                 = */ phy_wc40_enable_get,
    /* .pd_duplex_set                 = */ phy_wc40_duplex_set,
    /* .pd_duplex_get                 = */ phy_wc40_duplex_get,
    /* .pd_speed_set                  = */ phy_wc40_speed_set,
    /* .pd_speed_get                  = */ phy_wc40_speed_get,
    /* .pd_master_set                 = */ phy_null_set,
    /* .pd_master_get                 = */ phy_wc40_master_get,
    /* .pd_an_set                     = */ phy_wc40_an_set,
    /* .pd_an_get                     = */ phy_wc40_an_get,
    /* .pd_adv_local_set              = */ NULL, /* Deprecated */
    /* .pd_adv_local_get              = */ NULL, /* Deprecated */
    /* .pd_adv_remote_get             = */ NULL, /* Deprecated */
    /* .pd_lb_set                     = */ phy_wc40_lb_set,
    /* .pd_lb_get                     = */ phy_wc40_lb_get,
    /* .pd_interface_set              = */ phy_wc40_interface_set,
    /* .pd_interface_get              = */ phy_wc40_interface_get,
    /* .pd_ability                    = */ NULL, /* Deprecated */
    /* .pd_linkup_evt                 = */ phy_wc40_linkup_evt,
    /* .pd_linkdn_evt                 = */ NULL,
    /* .pd_mdix_set                   = */ phy_null_mdix_set,
    /* .pd_mdix_get                   = */ phy_null_mdix_get,
    /* .pd_mdix_status_get            = */ phy_null_mdix_status_get,
    /* .pd_medium_config_set          = */ phy_wc40_medium_config_set,
    /* .pd_medium_config_get          = */ NULL, 
    /* .pd_medium_get                 = */ phy_wc40_medium_get,
    /* .pd_cable_diag                 = */ NULL,
    /* .pd_link_change                = */ NULL,
    /* .pd_control_set                = */ phy_wc40_control_set,    
    /* .pd_control_get                = */ phy_wc40_control_get,
    /* .pd_reg_read                   = */ phy_wc40_reg_read,
    /* .pd_reg_write                  = */ phy_wc40_reg_write, 
    /* .pd_reg_modify                 = */ phy_wc40_reg_modify,
    /* .pd_notify                     = */ phy_wc40_notify,
    /* .pd_probe                      = */ phy_wc40_probe,
    /* .pd_ability_advert_set         = */ phy_wc40_ability_advert_set,
    /* .pd_ability_advert_get         = */ phy_wc40_ability_advert_get,
    /* .pd_ability_remote_get         = */ phy_wc40_ability_remote_get,
    /* .pd_ability_local_get          = */ phy_wc40_ability_local_get,
    /* .pd_firmware_set               = */ phy_wc40_firmware_set
};

/***********************************************************************
 *
 * PASS-THROUGH NOTIFY ROUTINES
 *
 ***********************************************************************/

/*
 * Function:
 *      _phy_wc40_notify_duplex
 * Purpose:
 *      Program duplex if (and only if) serdeswc40 is an intermediate PHY.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - Boolean, TRUE indicates full duplex, FALSE
 *              indicates half.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      If PHY_FLAGS_FIBER is set, it indicates the PHY is being used to
 *      talk directly to an external fiber module.
 *
 *      If PHY_FLAGS_FIBER is clear, the PHY is being used in
 *      pass-through mode to talk to an external SGMII PHY.
 *
 *      When used in pass-through mode, autoneg must be turned off and
 *      the speed/duplex forced to match that of the external PHY.
 */

STATIC int
_phy_wc40_notify_duplex(int unit, soc_port_t port, uint32 duplex)
{
    int                 fiber;
    uint16              mii_ctrl;
    phy_ctrl_t  *pc;

    pc    = INT_PHY_SW_STATE(unit, port);
    fiber = DEV_CFG_PTR(pc)->fiber_pref;
    SOC_DEBUG_PRINT((DK_PHY,
                     "_phy_wc40_notify_duplex: "
                     "u=%d p=%d duplex=%d fiber=%d\n",
                     unit, port, duplex, fiber));

    if (SAL_BOOT_SIMULATION) {
        return SOC_E_NONE;
    }

    if (fiber) {
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00,
                                          MII_CTRL_FD, MII_CTRL_FD));
        return SOC_E_NONE;
    }

    /* Put SERDES PHY in reset */

    SOC_IF_ERROR_RETURN
        (_phy_wc40_notify_stop(unit, port, PHY_STOP_DUPLEX_CHG));

    /* Update duplexity */
    mii_ctrl = (duplex) ? MII_CTRL_FD : 0;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_COMBO_IEEE0_MIICNTLr(unit, pc, 0x00, mii_ctrl, MII_CTRL_FD));

    /* Take SERDES PHY out of reset */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_notify_resume(unit, port, PHY_STOP_DUPLEX_CHG));

    /* Autonegotiation must be turned off to talk to external PHY if
     * SGMII autoneg is not enabled.
     */
    if (!PHY_SGMII_AUTONEG_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (phy_wc40_an_set(unit, port, FALSE));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_wc40_notify_speed
 * Purpose:
 *      Program duplex if (and only if) serdeswc40 is an intermediate PHY.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - Speed to program.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      If PHY_FLAGS_FIBER is set, it indicates the PHY is being used to
 *      talk directly to an external fiber module.
 *
 *      If PHY_FLAGS_FIBER is clear, the PHY is being used in
 *      pass-through mode to talk to an external SGMII PHY.
 *
 *      When used in pass-through mode, autoneg must be turned off and
 *      the speed/duplex forced to match that of the external PHY.
 */

STATIC int
_phy_wc40_notify_speed(int unit, soc_port_t port, uint32 speed)
{
    int          fiber;
    uint16       fiber_status;
    phy_ctrl_t  *pc;

    pc    = INT_PHY_SW_STATE(unit, port);
    fiber = DEV_CFG_PTR(pc)->fiber_pref;

    SOC_DEBUG_PRINT((DK_PHY,
                     "_phy_wc40_notify_speed: "
                     "u=%d p=%d speed=%d fiber=%d\n",
                     unit, port, speed, fiber));

    if (SAL_BOOT_SIMULATION) {
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_STATUS1000X1r(unit, pc, 0x00, &fiber_status));

    /* Put SERDES PHY in reset */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_notify_stop(unit, port, PHY_STOP_SPEED_CHG));

    /* Update speed */
    SOC_IF_ERROR_RETURN
        (phy_wc40_speed_set(unit, port, speed));

    /* Take SERDES PHY out of reset */
    SOC_IF_ERROR_RETURN
        (_phy_wc40_notify_resume(unit, port, PHY_STOP_SPEED_CHG));

    /* Autonegotiation must be turned off to talk to external PHY */
    if (!PHY_SGMII_AUTONEG_MODE(unit, port) && PHY_EXTERNAL_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (phy_wc40_an_set(unit, port, FALSE));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_wc40_stop
 * Purpose:
 *      Put serdeswc40 SERDES in or out of reset depending on conditions
 */

STATIC int
_phy_wc40_stop(int unit, soc_port_t port)
{
    int                 stop, copper;
    uint16 mask16,data16;
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        return SOC_E_NONE;
    }

    copper = (pc->stop &
              PHY_STOP_COPPER) != 0;

    stop = ((pc->stop &
             (PHY_STOP_PHY_DIS |
              PHY_STOP_DRAIN)) != 0 ||
            (copper &&
             (pc->stop &
              (PHY_STOP_MAC_DIS |
               PHY_STOP_DUPLEX_CHG |
               PHY_STOP_SPEED_CHG)) != 0));

    SOC_DEBUG_PRINT((DK_PHY,
              "phy_wc40_stop: u=%d p=%d copper=%d stop=%d flg=0x%x\n",
               unit, port, copper, stop,
               pc->stop));

    /* tx/rx reset */
    mask16 = DIGITAL5_MISC6_RESET_RX_ASIC_MASK | DIGITAL5_MISC6_RESET_TX_ASIC_MASK;
    data16 = stop? mask16: 0;
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DIGITAL5_MISC6r(unit,pc,0x00, data16, mask16));

    SOC_DEBUG_PRINT((DK_PHY,
          "phy_wc40_stop: u=%d p=%d mask=0x%x value=0x%x\n",
           unit, port, mask16, data16));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_wc40_notify_stop
 * Purpose:
 *      Add a reason to put serdeswc40 PHY in reset.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flags - Reason to stop
 * Returns:     
 *      SOC_E_XXX
 */

STATIC int
_phy_wc40_notify_stop(int unit, soc_port_t port, uint32 flags)
{
    INT_PHY_SW_STATE(unit, port)->stop |= flags;

    return _phy_wc40_stop(unit, port);
}

/*  
 * Function:
 *      _phy_wc40_notify_resume
 * Purpose:
 *      Remove a reason to put serdeswc40 PHY in reset.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flags - Reason to stop
 * Returns:     
 *      SOC_E_XXX
 */

STATIC int
_phy_wc40_notify_resume(int unit, soc_port_t port, uint32 flags)
{   
    INT_PHY_SW_STATE(unit, port)->stop &= ~flags;
    
    return _phy_wc40_stop(unit, port);
}

/*
 * Function:
 *      phy_wc40_media_setup
 * Purpose:     
 *      Configure 
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      fiber_mode - Configure for fiber mode
 *      fiber_pref - Fiber preferrred (if fiber mode)
 * Returns:     
 *      SOC_E_XXX
 */
STATIC int
_phy_wc40_notify_interface(int unit, soc_port_t port, uint32 intf)
{
    phy_ctrl_t  *pc;
    uint16       data16;
    
    pc = INT_PHY_SW_STATE(unit, port);

    data16 = 0;
    if (intf != SOC_PORT_IF_SGMII) {
        data16 = SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    }
 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_SERDESDIGITAL_CONTROL1000X1r(unit, pc, 0x00, data16,
                           SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK));
 
    return SOC_E_NONE;
}

STATIC
int _wc40_xgmii_scw_config (int unit, phy_ctrl_t *pc)
{
    /* Dual Broadcast mode write */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW0r(unit,pc,0x00,0xE070));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW1r(unit,pc,0x00,0xC0D0));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW2r(unit,pc,0x00,0xA0B0));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW3r(unit,pc,0x00,0x8090));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW0_MASKr(unit,pc,0x00,0xF0F0));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW1_MASKr(unit,pc,0x00,0xF0F0));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW2_MASKr(unit,pc,0x00,0xF0F0));
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_RX66_SCW3_MASKr(unit,pc,0x00,0xF0F0));
    return(SOC_E_NONE);
}

STATIC 
int _wc40_rxaui_config(int unit, phy_ctrl_t  *pc,int rxaui)
{
    uint16 mask16,data16;
    mask16 = XGXSBLK1_LANECTRL3_PWRDWN_FORCE_MASK |
                0xc |    /* lane 2 and 3 RX */
                (0xc << XGXSBLK1_LANECTRL3_PWRDN_TX_SHIFT);  /* TX */

    /* if in reduced XAUI mode, disable lane 2 and 3 */
    data16 = rxaui? mask16: 0;

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK1_LANECTRL3r(unit,pc,0x00, data16,mask16));
    return(SOC_E_NONE);
}

STATIC
int _wc40_soft_reset(int unit, phy_ctrl_t  *pc)
{
   uint16 data16;
   uint16 mask16;

   data16 = XGXSBLK0_MMDSELECT_DEVCL22_EN_MASK |
                 XGXSBLK0_MMDSELECT_DEVDEVAD_EN_MASK |
                 XGXSBLK0_MMDSELECT_DEVPMD_EN_MASK  |
                 XGXSBLK0_MMDSELECT_DEVAN_EN_MASK |
                 XGXSBLK0_MMDSELECT_MULTIMMDS_EN_MASK;
   mask16 = data16 |
            XGXSBLK0_MMDSELECT_MULTIPRTS_EN_MASK;

    if (DEV_CFG_PTR(pc)->lane0_rst) {
        /* reset entire device */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_XGXSBLK2_LANERESETr(unit,pc,LANE0_ACCESS, 
                          XGXSBLK2_LANERESET_RESET_MDIO_MASK,
                          XGXSBLK2_LANERESET_RESET_MDIO_MASK));
        
        /* wait 10us for reset to complete */
        WC40_UDELAY(10);
    }
    /* select multi mmd */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_XGXSBLK0_MMDSELECTr(unit, pc, LANE0_ACCESS, data16,mask16));

    return(SOC_E_NONE);
}

STATIC
int _wc40_chip_init_done(int unit,int chip_num,int phy_mode)
{
    int port;
    phy_ctrl_t  *pc;

    PBMP_ALL_ITER(unit, port) {
        pc = INT_PHY_SW_STATE(unit, port);
        if (pc == NULL) {
            continue;
        }
        if (!pc->dev_name || pc->dev_name != wc_device_name) {
            continue;
        }
        if (pc->chip_num != chip_num) {
            continue;
        }

        if ((pc->flags & PHYCTRL_INIT_DONE) && (phy_mode == pc->phy_mode)) {
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************
*              Begin Diagnostic Code                                          *
*******************************************************************************/
#ifdef BROADCOM_DEBUG
STATIC int
_phy_wc40_cfg_dump(int unit, soc_port_t port)
{
    phy_ctrl_t  *pc;
    WC40_DEV_CFG_t *pCfg;
    WC40_DEV_INFO_t  *pInfo;
    WC40_DEV_DESC_t *pDesc;
    soc_phy_info_t *pi;
    int i;
    int size;

    pc = INT_PHY_SW_STATE(unit, port);
    pDesc = (WC40_DEV_DESC_t *)(pc + 1);
    pi = &SOC_PHY_INFO(unit, port);

    pCfg = &pDesc->cfg;
    pInfo = &pDesc->info;
    if (COMBO_LANE_MODE(DEV_CFG_PTR(pc)->lane_mode)) {
        size = 4;
    } else if (IS_DUAL_LANE_PORT(pc)) {
        size = 2;
    } else {
        size = 1;
    }
 
    soc_cm_print("pc = 0x%x, pCfg = 0x%x, pInfo = 0x%x\n", (int)(size_t)pc,
                  (int)(size_t)pCfg,(int)(size_t)pInfo);
    for (i = 0; i < size; i++) { 
        soc_cm_print("preemph%d     0x%x\n",i,pCfg->preemph[i]);
        soc_cm_print("idriver%d     0x%04x\n",i, pCfg->idriver[i]);
        soc_cm_print("pdriver%d     0x%04x\n",i, pCfg->pdriver[i]);
    }
    soc_cm_print("auto_medium  0x%04x\n", pCfg->auto_medium);
    soc_cm_print("fiber_pref   0x%04x\n", pCfg->fiber_pref);
    soc_cm_print("sgmii_mstr   0x%04x\n", pCfg->sgmii_mstr);
    soc_cm_print("pdetect10g   0x%04x\n", pCfg->pdetect10g);
    soc_cm_print("pdetect1000x 0x%04x\n", pCfg->pdetect1000x);
    soc_cm_print("cx42hg       0x%04x\n", pCfg->cx42hg);
    soc_cm_print("rxlane_map   0x%04x\n", pCfg->rxlane_map);
    soc_cm_print("txlane_map   0x%04x\n", pCfg->txlane_map);
    soc_cm_print("rxpol        0x%04x\n", pCfg->rxpol);
    soc_cm_print("txpol        0x%04x\n", pCfg->txpol);
    soc_cm_print("cl73an       0x%04x\n", pCfg->cl73an);
    soc_cm_print("phy_mode     0x%04x\n", pc->phy_mode);
    soc_cm_print("cx4_10g      0x%04x\n", pCfg->cx4_10g);
    soc_cm_print("lane0_rst    0x%04x\n", pCfg->lane0_rst);
    soc_cm_print("rxaui        0x%04x\n", pCfg->rxaui);
    soc_cm_print("dxgxs        0x%04x\n", pCfg->dxgxs);
    soc_cm_print("line_intf    0x%04x\n", pCfg->line_intf);
    soc_cm_print("hg_mode      0x%04x\n", pCfg->hg_mode);
    soc_cm_print("chip_num     0x%04x\n", pc->chip_num);
    soc_cm_print("lane_num     0x%04x\n", pc->lane_num);
    soc_cm_print("speedMax     0x%04x\n", pc->speed_max);
    soc_cm_print("pc->flags    0x%04x\n", pc->flags);
    soc_cm_print("pc->stop     0x%04x\n", pc->stop);
    soc_cm_print("pi->phy_flags   0x%04x\n", pi->phy_flags);

    return SOC_E_NONE;
}
#endif

#define WC_UC_DEBUG
#ifdef WC_UC_DEBUG
typedef struct {
    int tx_pre_cursor;
    int tx_main;
    int tx_post_cursor;
    char *vga_bias_reduced;
    int postc_metric;
    int pf_ctrl;
    int vga_sum;
    int dfe1_bin;
    int dfe2_bin;
    int dfe3_bin;
    int dfe4_bin;
    int dfe5_bin;
    int integ_reg;
    int integ_reg_xfer;
    int clk90_offset;
    int slicer_target;
    int offset_pe;
    int offset_ze;
    int offset_me;
    int offset_po;
    int offset_zo;
    int offset_mo;
} UC_DESC;

#define MAX_LANES 4
static UC_DESC uc_desc[MAX_LANES];
static int lanes[MAX_LANES] = {LANE0_ACCESS,LANE1_ACCESS,LANE2_ACCESS,LANE3_ACCESS};

int
wc40_uc_status_dump(int unit, soc_port_t port)
{
    phy_ctrl_t  *pc;
    UC_DESC * pDesc;
    int i;
    int reg;
    uint16 data16;
    uint16 mask16; 
    int regval;
    pc = INT_PHY_SW_STATE(unit, port);
/*
 *Const DSC_1_ADDR = &H8200&
 *Const DSC_2_ADDR = &H8210&
 *Const DSC_3_ADDR = &H8220&
 *Const DSC_4_ADDR = &H8230&
 *Const DSC_5_ADDR = &H8240&
 * 0x822c,2d,2e,
*/
    WC40_MEM_SET((char *)&uc_desc[0], 0, (sizeof(UC_DESC))*MAX_LANES);
    for (i = 0; i < MAX_LANES; i++) {
        pDesc = &uc_desc[i];

        /* tx_pre_cursor */
        reg = 0x8063 + 0x10*i;      
        mask16 = (1 << 14) | (1 << 15);
        data16 = 1 << 14;
        SOC_IF_ERROR_RETURN
            (WC40_REG_MODIFY(unit, pc, LANE0_ACCESS, reg, data16,mask16));

        reg = 0x8060 + 0x10*i;      
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, LANE0_ACCESS, reg, &data16));
        regval = data16 & 0xF;
        pDesc->tx_pre_cursor = regval;

        /* tx_main */
        regval = (data16 >> 4) & 0x3F;
        pDesc->tx_main = regval;

        /* tx_post_cursor */
        regval = (data16 >> 10) & 0x1F;
        pDesc->tx_post_cursor = regval;

        /* restore 0x8063 */
        reg = 0x8063 + 0x10*i;      
        mask16 = (1 << 14) | (1 << 15);
        data16 = 0;
        SOC_IF_ERROR_RETURN
            (WC40_REG_MODIFY(unit, pc, LANE0_ACCESS, reg, data16,mask16));

        /* vga_bias_reduced */ 
        reg = 0x80BD + 0x10*i;      
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, LANE0_ACCESS, reg, &data16));
        regval = data16 & (1 << 4);
        if (regval) {
            pDesc->vga_bias_reduced = "88%";
        } else {
            pDesc->vga_bias_reduced = "100%";
        }
    }

    for (i = 0; i < MAX_LANES; i++) {
        pDesc = &uc_desc[i];
        /* postc_metric(lane) = rd22_postc_metric (phy, lane), DSC_5_ADDR=0x8240 */
        reg = 0x8241;      
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 0x7ff;
        if (regval >=1024) {
            regval -= 2048;
        } 
        pDesc->postc_metric = regval;

        /* pf_ctrl(lane) = rd22_pf_ctrl (phy, lane) DSC_3_ADDR=0x8220 */
        reg = 0x822b;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 0xf;
        pDesc->pf_ctrl = regval;

        /* vga_sum(lane) = rd22_vga_sum (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8225;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 63;
        pDesc->vga_sum = regval;

        /* dfe1_bin(lane) = rd22_dfe_tap_1_bin (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8225;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 64; 
        regval &= 63;
        pDesc->dfe1_bin = regval;

        /* dfe2_bin(lane) = rd22_dfe_tap_2_bin (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8226;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->dfe2_bin = regval;
        
        /* dfe3_bin(lane) = rd22_dfe_tap_3_bin (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8226;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 64;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->dfe3_bin = regval;
        
        /* dfe4_bin(lane) = rd22_dfe_tap_4_bin (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8227;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 31;
        if (regval >= 16) {
            regval -=32;
        }
        pDesc->dfe4_bin = regval;
  
        /* dfe5_bin(lane) = rd22_dfe_tap_5_bin (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8227;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 32;
        regval &= 31;
        if (regval >= 16) {
            regval -=32;
        }
        pDesc->dfe5_bin = regval;

        /* integ_reg(lane) = rd22_integ_reg (phy, lane) DSC_3_ADDR=0x8220  */
        reg = 0x8220;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 65535;
        if (regval >= 32768) {
            regval -=65536;
        }
        regval /= 84;
        pDesc->integ_reg = regval;

        /* integ_reg_xfer(lane) = rd22_integ_reg_xfer (phy, lane)   */
        reg = 0x8221;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 65535;
        if (regval >= 32768) {
            regval -=65536;
        }
        pDesc->integ_reg_xfer = regval;

        /* clk90_offset(lane) = rd22_clk90_phase_offset (phy, lane)   */
        reg = 0x8223;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 128;
        regval &= 127;
        pDesc->clk90_offset = regval;

        /* slicer_target(lane) = ((25*rd22_rx_thresh_sel (phy, lane)) + 125)   */
        reg = 0x821c;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 32;
        regval &= 3;
        pDesc->slicer_target = regval * 25 + 125;

        /* offset_pe(lane) = rd22_slicer_offset_pe (phy, lane)   */
        reg = 0x822c;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_pe = regval;

        /* offset_ze(lane) = rd22_slicer_offset_ze (phy, lane)   */
        reg = 0x822d;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_ze = regval;

        /* offset_me(lane) = rd22_slicer_offset_me (phy, lane)   */
        reg = 0x822e;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_me = regval;

        /* offset_po(lane) = rd22_slicer_offset_po (phy, lane) */
        reg = 0x822c;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 64;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_po = regval;

        /* offset_zo(lane) = rd22_slicer_offset_zo (phy, lane) */
        reg = 0x822d;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 64;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_zo = regval;

        /* offset_mo(lane) = rd22_slicer_offset_mo (phy, lane) */
        reg = 0x822e;
        SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, lanes[i], reg, &data16));
        regval = data16;
        regval /= 64;
        regval &= 63;
        if (regval >= 32) {
            regval -=64;
        }
        pDesc->offset_mo = regval;
    }

    /* 0x8058, bits [13:6].  */
    SOC_IF_ERROR_RETURN
            (WC40_REG_READ(unit, pc, LANE0_ACCESS, 0x8058, &data16));
    data16 = (data16 >> 6) & 0xff;

    /* display */
    soc_cm_print("\n\nDSC parameters for port %d\n\n", port);
    soc_cm_print("PLL Range: %d\n\n",data16);

    soc_cm_print("LN  PPM PPM_XFR clk90_ofs PF SL_TRGT VGA BIAS DFE1 DFE2 "
                 "DFE3 DFE4 DFE5 PREC MAIN POSTC MTRC PE   ZE   ME  PO  ZO  MO\n");
    for (i = 0; i < MAX_LANES; i++) {
        pDesc = &uc_desc[i];
        soc_cm_print("%02d %04d %07d %09d %04d %04d %04d %4s %04d %04d %04d %04d "
                     "%04d %04d %04d %04d  %04d %04d %04d %2d %3d %3d %2d\n",
           i,pDesc->integ_reg,pDesc->integ_reg_xfer,pDesc->clk90_offset,
           pDesc->pf_ctrl,pDesc->slicer_target,pDesc->vga_sum,pDesc->vga_bias_reduced,
           pDesc->dfe1_bin,pDesc->dfe2_bin,pDesc->dfe3_bin,pDesc->dfe4_bin,
           pDesc->dfe5_bin,pDesc->tx_pre_cursor,pDesc->tx_main,pDesc->tx_post_cursor,
           pDesc->postc_metric,pDesc->offset_pe,pDesc->offset_ze,pDesc->offset_me,
           pDesc->offset_po,pDesc->offset_zo,pDesc->offset_mo);

    }
    return SOC_E_NONE;

}
#endif

/********   Eye Margin Diagnostic  **********/

#ifndef __KERNEL__
#include <math.h> 

#define MAX_LOOPS 47
#define MIN_RUNTIME  1  
#define MAX_RUNTIME  256  
#define HI_CONFIDENCE_ERR_CNT 100    /* bit errors to determine high confidence */
#define LO_CONFIDENCE_MIN_ERR_CNT 10 /* bit errors, exit condition for low confidence */
#define HI_CONFIDENCE_MIN_ERR_CNT 20 /* bit errors exit condition for high confidence */
#define VEYE_UNIT 1.75
#define HEYE_UNIT 3.125
#define DSC1B0_UC_CTRLr 0x820e
#define WC_UTIL_MAX_ROUND_DIGITS (8) 

/* Do not change the value, used as index */
#define WC_UTIL_VEYE           0  /* vertical eye */
#define WC_UTIL_HEYE_R         1  /* horizontal right eye */
#define WC_UTIL_HEYE_L         2  /* horizontal left eye */

typedef struct {
    int total_errs[MAX_LOOPS];
    int total_elapsed_time[MAX_LOOPS];
    int max_loops;
    int offset_max;
    int veye_cnt;
    uint32 rate;      /* frequency in KHZ */
} WC40_EYE_DIAG_INFOt;

static char *eye_test_name_str[] = {"Vertical Eye","Right Eye","Left Eye"};

STATIC int 
_wc40_veye_margin_data_get( int unit, int port, int lane, WC40_EYE_DIAG_INFOt *pInfo,int type); 
STATIC int  
_wc40_eye_margin_diagram_cal(WC40_EYE_DIAG_INFOt *pInfo,int type); 
STATIC int 
_wc40_eye_margin_ber_cal(WC40_EYE_DIAG_INFOt *pInfo,int type); 


#if 1 
/* DEBUG function: manually construct a set of test data to test
 * extrapolation algorithem. 
 * type: 0 vertical eye, 1 horizontal right eye, 2 horizontal left eye
 * max_offset: the slicer max offset, for example 23
 * the clock rate is hardcoded to 10312500Khz
 */ 
STATIC WC40_EYE_DIAG_INFOt wc40_eye_test_data;

int wc40_eye_margin_test(int type, int max_offset) 
{
    WC40_EYE_DIAG_INFOt *pInfo;
    int cnt;

/*
 *  WC_UTIL_VEYE           0   vertical eye 
 *  WC_UTIL_HEYE_R         1   horizontal right eye 
 *  WC_UTIL_HEYE_L         2   horizontal left eye 
*/

    pInfo = &wc40_eye_test_data;
    pInfo->max_loops = MAX_LOOPS;
    pInfo->offset_max = max_offset; /*24 */
    cnt = 0;
    pInfo->total_errs[cnt]         = 32767;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 28111;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 14417;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 6575;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 3334;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 1420;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 588;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 196;
    pInfo->total_elapsed_time[cnt] = 1; 
    cnt++;
    pInfo->total_errs[cnt]         = 139;
    pInfo->total_elapsed_time[cnt] = 2; 
    cnt++;
    pInfo->total_errs[cnt]         = 103;
    pInfo->total_elapsed_time[cnt] = 4; 
    cnt++;
    pInfo->total_errs[cnt]         = 124;
    pInfo->total_elapsed_time[cnt] = 16; 
    cnt++;
    pInfo->total_errs[cnt]         = 122;
    pInfo->total_elapsed_time[cnt] = 64; 
    cnt++;
    pInfo->total_errs[cnt]         = 80;
    pInfo->total_elapsed_time[cnt] = 256; 
    cnt++;
    pInfo->total_errs[cnt]         = 18;
    pInfo->total_elapsed_time[cnt] = 256; 
    cnt++;
    pInfo->veye_cnt   = cnt;
    pInfo->rate   = 10312500;
    SOC_IF_ERROR_RETURN
        (_wc40_eye_margin_ber_cal(pInfo,type)); 
    SOC_IF_ERROR_RETURN
        (_wc40_eye_margin_diagram_cal(pInfo,type));
    return SOC_E_NONE;
}
#endif /* end debug function */

int
wc40_eye_margin(int unit, soc_port_t port, int type)
{
    phy_ctrl_t      *pc;
    int lane_start;
    int lane_end;
    int i;
    WC40_EYE_DIAG_INFOt veye_info[4];

    pc = INT_PHY_SW_STATE(unit, port);

    /* clear the memory */
    WC40_MEM_SET(veye_info, 0, sizeof(veye_info));

    if (pc->phy_mode == PHYCTRL_ONE_LANE_PORT) {
        lane_start = pc->lane_num;
        lane_end   = lane_start;
    } else if (pc->phy_mode == PHYCTRL_DUAL_LANE_PORT) {
        lane_start = pc->lane_num;
        lane_end   = lane_start + 1;
    } else {
        lane_start = 0;
        lane_end   = 3;
    }
    soc_cm_print("\nPort %d : Start BER extrapolation for %s\n", port,eye_test_name_str[type]);
    soc_cm_print("Port %d : Test time varies from a few minutes to over 20 minutes. " 
                 "Please wait ...\n", port);

    for (i = lane_start; i <= lane_end; i++) {
        if (lane_start != lane_end) {
            soc_cm_print("\nStart test for lane %d\n", i);
        }
        SOC_IF_ERROR_RETURN
            (_wc40_veye_margin_data_get(unit,port,i,&veye_info[i],type));
        SOC_IF_ERROR_RETURN
            (_wc40_eye_margin_ber_cal(&veye_info[i],type)); 
        SOC_IF_ERROR_RETURN
            (_wc40_eye_margin_diagram_cal(&veye_info[i],type));
    }

    return SOC_E_NONE;
}
int
_wc40_lane_prbs_rx_status_get(int unit, soc_port_t port, int lane, 
                                          uint32 *value)
{
    uint16      data;
    int prbs_cfg;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    *value = 0;
    if (DEV_CTRL_PTR(pc)->prbs.type == WC40_PRBS_TYPE_CL49) {
        data = 0;
        /* enable PRBS if not already. PRBS31 or PRBS9 */
        prbs_cfg = TRUE;
        SOC_IF_ERROR_RETURN
            (READ_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, ln_access[lane], &data));
        if (DEV_CTRL_PTR(pc)->prbs.poly == WC40_PRBS_CFG_POLY31) {
            if (data == WC40_PRBS_CL49_POLY31) { /* prbs already in PRBS31 */
                prbs_cfg = FALSE;
            }
        } else {
            DEV_CTRL_PTR(pc)->prbs.poly = WC40_PRBS_CFG_POLY31; /* only support PRBS31 for now*/
        }

        if (prbs_cfg) {
            SOC_IF_ERROR_RETURN
                (WRITE_WC40_PCS_IEEE2BLK_PCS_TPCONTROLr(unit, pc, ln_access[lane],
                        WC40_PRBS_CL49_POLY31));
        }

        SOC_IF_ERROR_RETURN
            (READ_WC40_CL49_USERB0_STATUSr(unit,pc,ln_access[lane],&data));

        if (data & CL49_USERB0_STATUS_PRBS_LOCK_MASK) { /* PRBS lock */
            /* check error count */
            SOC_IF_ERROR_RETURN
                (READ_WC40_PCS_IEEE2BLK_PCS_TPERRCOUNTERr(unit,pc,ln_access[lane],&data));
            *value = data;

        } else { /* PRBS not in sync */
            *value = -1;
        }
        return SOC_E_NONE;
    }

    /* Get status for all 4 lanes and check for sync
     * 0x80b0, 0x80c0, 0x80d0, 0x80e0
     */
    SOC_IF_ERROR_RETURN
        (WC40_REG_READ(unit, pc, LANE0_ACCESS, 0x80b0 + (0x10 * lane), &data));

    if (data == (RX0_ANARXSTATUS_PRBS_STATUS_PRBS_LOCK_BITS  <<
                     RX0_ANARXSTATUS_PRBS_STATUS_PRBS_LOCK_SHIFT)) {
        /* PRBS is in sync */
    } else if (data == 0) {
        /* PRBS not in sync */
        *value = -1;
    } else {
        /* Get errors */
        *value = data & RX0_ANARXSTATUS_PRBS_STATUS_PTBS_ERRORS_MASK;
    }
    return SOC_E_NONE;
}


STATIC int 
_wc40_veye_margin_data_get( int unit, int port, int lane, WC40_EYE_DIAG_INFOt *pInfo,int type) 
{
    int runtime_loop;
    int max_runtime_loop;
    int curr_runtime;
    int offset[MAX_LOOPS];
    int offset_max;
    int loop_var;
    int veye_cnt;
    int hi_confidence_cnt;
    uint32 prbs_status;
    phy_ctrl_t *pc;
    int rv = SOC_E_NONE;
    uint16 data16;
    uint16 vga_frzval;
    uint16 vga_frcfrz;
    uint16 dfe_frzval;
    uint16 dfe_frcfrz;
    int pll_div[16] = {32,36,40,42,48,50,52,54,60,64,66,68,80,120,200,240};
    int ref_clk_freq[8] = {25000,100000,125000,156250,187500,161250,50000,106250};
    uint16 clk90_p_offset;
    int tmp;

    pc = INT_PHY_SW_STATE(unit, port);

    /* check what rate the test will run for */
    SOC_IF_ERROR_RETURN
        (READ_WC40_PLL_ANAPLLSTATUSr(unit,pc,ln_access[0],&data16));

    /* get the ref_clk divider first */
    pInfo->rate = pll_div[data16 & PLL_ANAPLLSTATUS_PLL_MODE_AFE_MASK];

    /* get the ref_clk frequency frequency */    
    SOC_IF_ERROR_RETURN
        (READ_WC40_SERDESDIGITAL_MISC1r(unit,pc,ln_access[0],&data16));
    data16 = (data16 >> SERDESDIGITAL_MISC1_REFCLK_SEL_SHIFT) & 7;
    pInfo->rate *= ref_clk_freq[data16];

    /* max loop_cnt for BER test with test time doubling every iteration */
    /* max_runtime_loop = log(MAX_RUNTIME/MIN_RUNTIME)/log(2); */  
    max_runtime_loop = 8;  
    pInfo->max_loops = MAX_LOOPS;

    SOC_DEBUG_PRINT((DK_PHY,
                       "WC_VEYE : max_runtime_loop: %d u=%d p=%d\n", 
                    max_runtime_loop,unit, port));

    /* Initialize BER array */
    for( loop_var = 0; loop_var < pInfo->max_loops; loop_var++ ) {
        pInfo->total_errs[loop_var] = 0;
    }

    /* 0x820d[0]=1 enable diagnostic */ 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC1B0_DSC_DIAG_CTRL0r(unit, pc, ln_access[lane], 
               DSC1B0_DSC_DIAG_CTRL0_DIAGNOSTICS_EN_MASK,
               DSC1B0_DSC_DIAG_CTRL0_DIAGNOSTICS_EN_MASK));

    /* Freeze DFE/VGA */
    SOC_IF_ERROR_RETURN
        (READ_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane], &data16));
    vga_frzval = data16 & DSC2B0_ACQ_SM_CTRL1_VGA_FRZVAL_MASK; 
    vga_frcfrz = data16 & DSC2B0_ACQ_SM_CTRL1_VGA_FRCFRZ_MASK;
    dfe_frzval = data16 & DSC2B0_ACQ_SM_CTRL1_DFE_FRZVAL_MASK; 
    dfe_frcfrz = data16 & DSC2B0_ACQ_SM_CTRL1_DFE_FRCFRZ_MASK; 
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane], 
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRZVAL_MASK |
                      (0x1f << DSC2B0_ACQ_SM_CTRL1_DFE_FRZVAL_SHIFT) |
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRCFRZ_MASK |
                      DSC2B0_ACQ_SM_CTRL1_DFE_FRCFRZ_MASK,
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRZVAL_MASK |
                      DSC2B0_ACQ_SM_CTRL1_DFE_FRZVAL_MASK |
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRCFRZ_MASK |
                      DSC2B0_ACQ_SM_CTRL1_DFE_FRCFRZ_MASK));

#if 0   
    /* first time, uC seems always indicating not ready */
    /* wait for uC ready for command bit7 */
    rv = _phy_wc40_regbit_set_wait_check(pc,DSC1B0_UC_CTRLr,
                     DSC1B0_UC_CTRL_READY_FOR_CMD_MASK,1,WC40_PLL_WAIT,ln_access[lane]);

    if (rv == SOC_E_TIMEOUT) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "WC_EYE : uController not ready!!!: u=%d p=%d\n", unit, port));
        return (SOC_E_TIMEOUT);
    }
#endif

    if (type == WC_UTIL_HEYE_L) {
        /* Write max to get left eye offset range */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], 0x7f02));

    } else if (type == WC_UTIL_HEYE_R) {
        /* Write min to get right eye offset range */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane],0x8002));

    } else {  /* vertical eye */
        /* write max to get vertical offset range */
        SOC_IF_ERROR_RETURN
            (WRITE_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], 0x7f03));
    }

    /* wait for uC ready for command:  bit7=1 */
    rv = _phy_wc40_regbit_set_wait_check(pc,DSC1B0_UC_CTRLr,
                     DSC1B0_UC_CTRL_READY_FOR_CMD_MASK,1,WC40_PLL_WAIT,ln_access[lane]);

    if (rv == SOC_E_TIMEOUT) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "WC_EYE : uController not ready pass 1!!!: u=%d p=%d\n", 
                        unit, port));
        return (SOC_E_TIMEOUT);
    }

    /* read out the max value */
    SOC_IF_ERROR_RETURN
        (READ_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], &data16));
    offset_max = (data16 >> 8) & 0xff;

    if (type == WC_UTIL_HEYE_L || type == WC_UTIL_HEYE_R) {
        offset_max -= 4;
        pInfo->offset_max = offset_max;
    } else {
        pInfo->offset_max = offset_max;
    }   
    
    SOC_IF_ERROR_RETURN
        (READ_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane], &data16));

    SOC_DEBUG_PRINT((DK_PHY,
                   "WC_EYE : offset_max %d DSC2B0_ctrl 0x%x u=%d p=%d\n", 
                      offset_max,data16,unit, port));

    sal_usleep(10000);  /* 10ms */

    hi_confidence_cnt = 0;
    veye_cnt = 0;

    for (loop_var = 0; loop_var < offset_max; loop_var++) { 
        offset[loop_var] = offset_max-loop_var;
        veye_cnt += 1;
        /*
         * Set a offset
         */
        if (type == WC_UTIL_HEYE_R) {
            data16 =-offset[loop_var];
        } else {
            data16 = offset[loop_var];
        }
        /* write vertical offset */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], 
                      data16 << DSC1B0_UC_CTRL_SUPPLEMENT_INFO_SHIFT,
                      DSC1B0_UC_CTRL_SUPPLEMENT_INFO_MASK));

        /* 0x8223 register read out */
        SOC_IF_ERROR_RETURN
            (READ_WC40_DSC3B0_PI_STATUS0r(unit, pc, ln_access[lane], &clk90_p_offset));
        clk90_p_offset >>= 7;
        clk90_p_offset &= 0x7f;
        tmp = (short)data16;
        SOC_DEBUG_PRINT((DK_PHY,
                     "Starting BER measurement at offset: %d clk90_p_offset: 0x%x u=%d p=%d\n",
                       tmp,clk90_p_offset,unit, port));

        if (type == WC_UTIL_HEYE_L || type == WC_UTIL_HEYE_R) {
            data16 = 2;
        } else {
            data16 = 3;
        }

        /* set offset cmd */
        SOC_IF_ERROR_RETURN
            (MODIFY_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], 
                  data16,
                  DSC1B0_UC_CTRL_GP_UC_REQ_MASK));

        /* wait for uC ready for command:  bit7=1 */
        rv = _phy_wc40_regbit_set_wait_check(pc,DSC1B0_UC_CTRLr,
                         DSC1B0_UC_CTRL_READY_FOR_CMD_MASK,1,WC40_PLL_WAIT,ln_access[lane]);

        if (rv == SOC_E_TIMEOUT) {
            SOC_DEBUG_PRINT((DK_WARN,
                       "WC_EYE : uC waits for offset fail!!!: u=%d p=%d\n", 
                        unit, port));
            return (SOC_E_TIMEOUT);
        }

        pInfo->total_errs[loop_var] = 0;
        pInfo->total_elapsed_time[loop_var] = 0;

        /* PRBS should be enabled before this function called. clear PRBS error. read a few times */
        SOC_IF_ERROR_RETURN
            ( _wc40_lane_prbs_rx_status_get(unit,port, lane,&prbs_status));
        SOC_IF_ERROR_RETURN
            ( _wc40_lane_prbs_rx_status_get(unit,port, lane,&prbs_status));
        SOC_IF_ERROR_RETURN
            ( _wc40_lane_prbs_rx_status_get(unit,port, lane,&prbs_status));

        for (runtime_loop = 0; runtime_loop <= max_runtime_loop; runtime_loop++) {
            if (runtime_loop == 0) {
                curr_runtime = 1 * MIN_RUNTIME;
            } else {
                curr_runtime = (1 << (runtime_loop - 1)) * MIN_RUNTIME;
            }
            SOC_DEBUG_PRINT((DK_PHY,
                   "Starting prbs run for %d seconds : u=%d p=%d\n", curr_runtime,
                        unit, port));
            /* XXX total_errs[loop_var] +=Round(check_prbs(phy, lane, curr_runtime)/2);*/

            /* wait for specified amount of time to collect the PRBS error */
            sal_usleep(curr_runtime * 1000000);

            SOC_IF_ERROR_RETURN
                ( _wc40_lane_prbs_rx_status_get(unit,port, lane,&prbs_status));
            if (prbs_status == -1) { /* not lock */
                  SOC_DEBUG_PRINT((DK_WARN,
                      "PRBS not locked, loop_num %d status=%d u=%d p=%d\n", 
                        loop_var,prbs_status,unit, port));
                prbs_status = 0x7fff;  /* max error */
            } 
            pInfo->total_errs[loop_var] +=  prbs_status/2;
            pInfo->total_elapsed_time[loop_var] +=  curr_runtime;

            if (pInfo->total_errs[loop_var] >= HI_CONFIDENCE_ERR_CNT) {
                break;
            }
        }

        /* Determine high-confidence iterations */
        if (pInfo->total_errs[loop_var] >= HI_CONFIDENCE_ERR_CNT) {
            hi_confidence_cnt = hi_confidence_cnt + 1;
        } 

        if (((hi_confidence_cnt >= 2) && (pInfo->total_errs[loop_var] < 
             HI_CONFIDENCE_MIN_ERR_CNT)) || 
            ((hi_confidence_cnt <  2) && 
             (pInfo->total_errs[loop_var] < LO_CONFIDENCE_MIN_ERR_CNT)) ) {
             break;  /* exit for loop */
        }
    }   /* for loop_var */

    /* Undo setup */
    if (type == WC_UTIL_HEYE_L || type == WC_UTIL_HEYE_R) {
        data16 = 2;
    } else {
        data16 = 3;
    }
 
    /* set vertical offset back to 0 */
    SOC_IF_ERROR_RETURN
        (WRITE_WC40_DSC1B0_UC_CTRLr(unit, pc, ln_access[lane], data16));

    /* wait for uC ready for command:  bit7=1 */
    rv = _phy_wc40_regbit_set_wait_check(pc,DSC1B0_UC_CTRLr,
                     DSC1B0_UC_CTRL_READY_FOR_CMD_MASK,1,WC40_PLL_WAIT,ln_access[lane]);

    if (rv == SOC_E_TIMEOUT) {
        SOC_DEBUG_PRINT((DK_WARN,
                   "WC_VEYE : uC waits for offset=0 fail!!!: u=%d p=%d\n",
                    unit, port));
        return (SOC_E_TIMEOUT);
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane],
                      vga_frzval,
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRZVAL_MASK));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane],
                      dfe_frzval,
                      DSC2B0_ACQ_SM_CTRL1_DFE_FRZVAL_MASK));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane],
                      vga_frcfrz,
                      DSC2B0_ACQ_SM_CTRL1_VGA_FRCFRZ_MASK));
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC2B0_ACQ_SM_CTRL1r(unit, pc, ln_access[lane],
                      dfe_frcfrz,
                      DSC2B0_ACQ_SM_CTRL1_DFE_FRCFRZ_MASK));

    /* disable diagnostics */
    SOC_IF_ERROR_RETURN
        (MODIFY_WC40_DSC1B0_DSC_DIAG_CTRL0r(unit, pc, ln_access[lane],
               0,
               DSC1B0_DSC_DIAG_CTRL0_DIAGNOSTICS_EN_MASK));

    /* Clear prbs monitor */
    SOC_IF_ERROR_RETURN
        ( _wc40_lane_prbs_rx_status_get(unit,port, lane,&prbs_status));

    pInfo->veye_cnt = veye_cnt;

    return SOC_E_NONE;
}

STATIC float 
_wc40_util_round_real( float original_value, int decimal_places ) 
{
    float shift_digits[WC_UTIL_MAX_ROUND_DIGITS+1] = { 0.0, 10.0, 100.0, 1000.0, 
                          10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0 };
    float shifted;
    float rounded;   
    if (decimal_places > WC_UTIL_MAX_ROUND_DIGITS ) {
        soc_cm_print("ERROR: Maximum digits to the right of decimal for rounding "
            "exceeded. Max %d, requested %d\n", 
               WC_UTIL_MAX_ROUND_DIGITS, decimal_places);
        return 0.0;
    } 
    /* shift to preserve the desired digits to the right of the decimal */   
    shifted = original_value * shift_digits[decimal_places];

    /* convert to integer and back to float to truncate undesired precision */
    shifted = (float)(floor(shifted+0.5));

    /* shift back to place decimal point correctly */   
    rounded = shifted / shift_digits[decimal_places];
    return rounded;
}

/*
 * enable PRBS31 on both ports before executing this function
 * Example:
 * phy prbs xe0,xe1 set mode=hc p=3
 */
STATIC int 
_wc40_eye_margin_ber_cal(WC40_EYE_DIAG_INFOt *pInfo, int type) 
{
    float bers[MAX_LOOPS]; /* computed bit error rate */
    float margins[MAX_LOOPS]; /* Eye margin @ each measurement*/
    int loop_var;
    float eye_unit;

    /* Initialize BER array */
    for( loop_var = 0; loop_var < MAX_LOOPS; loop_var++ ) {
        bers[loop_var] = 0.0;
    }

    if (type == WC_UTIL_HEYE_L || type == WC_UTIL_HEYE_R) {
        eye_unit = HEYE_UNIT;
    } else {
        eye_unit = VEYE_UNIT;
    }
 
    SOC_DEBUG_PRINT((DK_PHY, "\nDisplay the result of BER measurement at each offset\n"));

    for (loop_var = 0; loop_var < pInfo->veye_cnt; loop_var++) { 
        margins[loop_var] = (pInfo->offset_max-loop_var)*eye_unit;
        SOC_DEBUG_PRINT((DK_PHY,
                     "BER measurement at offset: %f\n", margins[loop_var]));
             
        /* Compute BER */
        if (pInfo->total_errs[loop_var] == 0) { 
            bers[loop_var] = 1.0/(float)pInfo->total_elapsed_time[loop_var]/pInfo->rate;
            bers[loop_var] /= 1000;

            SOC_DEBUG_PRINT((DK_WARN, "BER @ %04f %% = 1e%04f (%d errors in %d seconds)\n",
                      (float)((pInfo->offset_max-loop_var)*eye_unit), 
                      1.0*(log10(bers[loop_var])),
                      pInfo->total_errs[loop_var],
                      pInfo->total_elapsed_time[loop_var]));
        } else { 
            bers[loop_var] = (float)(pInfo->total_errs[loop_var])/
                             (float)pInfo->total_elapsed_time[loop_var]/pInfo->rate;

            /* the rate unit is KHZ, add -3(log10(1/1000)) for actual display  */
            bers[loop_var] /= 1000;
            SOC_DEBUG_PRINT((DK_WARN, "BER @ %2.2f%% = 1e%2.2f (%d errors in %d seconds)\n",
                  (pInfo->offset_max-loop_var)*eye_unit,
                  log10(bers[loop_var]),
                  pInfo->total_errs[loop_var],
                  pInfo->total_elapsed_time[loop_var]));
        }
    }   /* for loop_var */

    return SOC_E_NONE;
}


STATIC int  
_wc40_eye_margin_diagram_cal(WC40_EYE_DIAG_INFOt *pInfo, int type) 
{
    float lbers[MAX_LOOPS]; /*Internal linear scale sqrt(-log(ber))*/
    float margins[MAX_LOOPS]; /* Eye margin @ each measurement*/
    float bers[MAX_LOOPS]; /* computed bit error rate */
    int delta_n;
    float Exy = 0.0;
    float Eyy = 0.0;
    float Exx = 0.0;
    float Ey  = 0.0;
    float Ex  = 0.0;
    float alpha;
    float beta;
    float proj_ber;
    float proj_margin_12;
    float proj_margin_15;
    float proj_margin_18;
    float sq_err1;
    float sq_err2;
    float ierr;
    int beta_max=0;
    int ber_conf_scale[20];
    int start_n;
    int stop_n;
    int low_confidence;
    int loop_index;
    float outputs[4];
    float eye_unit;

    /* Initialize BER confidence scale */
    ber_conf_scale[0] = 3.02;
    ber_conf_scale[1] = 4.7863;
    ber_conf_scale[2] = 3.1623;
    ber_conf_scale[3] = 2.6303;
    ber_conf_scale[4] = 2.2909;
    ber_conf_scale[5] = 2.138;
    ber_conf_scale[6] = 1.9953;
    ber_conf_scale[7] = 1.9055;
    ber_conf_scale[8] = 1.8197;
    ber_conf_scale[9] = 1.7783;
    ber_conf_scale[10] = 1.6982;
    ber_conf_scale[11] = 1.6596;
    ber_conf_scale[12] = 1.6218;
    ber_conf_scale[13] = 1.6218;
    ber_conf_scale[14] = 1.5849;
    ber_conf_scale[15] = 1.5488;
    ber_conf_scale[16] = 1.5488;
    ber_conf_scale[17] = 1.5136;
    ber_conf_scale[18] = 1.5136;
    ber_conf_scale[19] = 1.4791;


    /* Find the highest data point to use, currently based on at least 20 errors */
    start_n = 0;
    stop_n = pInfo->veye_cnt;

    if (type == WC_UTIL_HEYE_L || type == WC_UTIL_HEYE_R) {
        eye_unit = HEYE_UNIT;
    } else {
        eye_unit = VEYE_UNIT;
    }

    for (loop_index = 0; loop_index < pInfo->veye_cnt; loop_index++) {
        if (pInfo->total_errs[loop_index] == 0) {
            bers[loop_index] = 1.0/(float)pInfo->total_elapsed_time[loop_index]/pInfo->rate;
        } else {
            bers[loop_index] = (float)pInfo->total_errs[loop_index]/
                               (float)pInfo->total_elapsed_time[loop_index]/pInfo->rate;
        }
        bers[loop_index] /= 1000;
        margins[loop_index] = (pInfo->offset_max-loop_index)*eye_unit;
        if( pInfo->total_errs[loop_index] < 1024 ) {
	    if( pInfo->total_errs[loop_index] < HI_CONFIDENCE_MIN_ERR_CNT ) {
	        stop_n = loop_index;
		break;
	    }
	} else {
	    start_n = loop_index + 1;
	}
    }

    if( start_n >= pInfo->veye_cnt ) {
        outputs[0] = -_wc40_util_round_real(log(bers[pInfo->veye_cnt-1])/log(10), 1);
        outputs[1] = -100.0;
        outputs[2] = -100.0;
	/*  No need to print out the decimal portion of the BER */
	soc_cm_print("BER *worse* than 1e-%d\n", (int)outputs[0]);
	soc_cm_print("Negative margin @ 1e-12 & 1e-18\n");
    } else {
        low_confidence = 0;
	if( (stop_n-start_n) < 2  ) {
	    /* Code triggered when less than 2 statistically valid extrapolation points */
            for( loop_index = stop_n; loop_index < pInfo->veye_cnt; loop_index++ ) {
	        if( pInfo->total_errs[loop_index] < 20 ) {
		    bers[loop_index] = ber_conf_scale[pInfo->total_errs[loop_index]] * bers[loop_index];
		} else {
		    bers[loop_index] = ber_conf_scale[19] * bers[loop_index];
		}
	    }
	    /* Add artificial point at 100% margin to enable interpolation */
            margins[pInfo->veye_cnt] = 100.0;
            bers[pInfo->veye_cnt] = 0.1;
            low_confidence = 1;
            stop_n = pInfo->veye_cnt + 1;
	}

	/* Below this point the code assumes statistically valid point available */
        delta_n = stop_n - start_n;

        SOC_DEBUG_PRINT((DK_PHY,"start_n: %d, stop_n: %d, veye: %d\n",
                          start_n,stop_n,pInfo->veye_cnt));

	/* Find all the correlations */
	for( loop_index = start_n; loop_index < stop_n; loop_index++ ) {
	    lbers[loop_index] = (float)sqrt(-log(bers[loop_index]));
	}

        SOC_DEBUG_PRINT((DK_PHY,"\tstart=%d, stop=%d, low_confidence=%d\n",
                                     start_n, stop_n, low_confidence));
        for (loop_index=start_n; loop_index < stop_n; loop_index++){
            SOC_DEBUG_PRINT((DK_PHY,"\ttotal_errs[%d]=0x%08x\n",
                         loop_index,(int)pInfo->total_errs[loop_index]));
            SOC_DEBUG_PRINT((DK_PHY,"\tbers[%d]=%f\n",
                         loop_index,bers[loop_index]));
            SOC_DEBUG_PRINT((DK_PHY,"\tlbers[%d]=%f\n",
                          loop_index,lbers[loop_index]));
	}

	for( loop_index = start_n; loop_index < stop_n; loop_index++ ) {
	    Exy = Exy + margins[loop_index] * lbers[loop_index]/(float)delta_n;
	    Eyy = Eyy + lbers[loop_index]*lbers[loop_index]/(float)delta_n;
	    Exx = Exx + margins[loop_index]*margins[loop_index]/(float)delta_n;
	    Ey  = Ey + lbers[loop_index]/(float)delta_n;
	    Ex  = Ex + margins[loop_index]/(float)delta_n;
	}

	/* Compute fit slope and offset */
        alpha = (Exy - Ey * Ex)/(Exx - Ex * Ex);
        beta = Ey - Ex * alpha;
	
	SOC_DEBUG_PRINT((DK_PHY,"Exy=%f, Eyy=%f, Exx=%f, Ey=%f,Ex=%f alpha=%f, beta=%f\n",
                         Exy,Eyy,Exx,Ey,Ex,alpha,beta));

	/* JPA> Due to the limit of floats, I need to test for a maximum Beta of 9.32 */
	if(beta > 9.32){
	  beta_max=1;
	  soc_cm_print("\n\tWARNING: intermediate float variable is maxed out, what this means is:\n");
	  soc_cm_print("\t\t- The *extrapolated* minimum BER will be reported as 1E-37.\n");
	  soc_cm_print("\t\t- This may occur if the channel is near ideal (e.g. test loopback)\n");
	  soc_cm_print("\t\t- While not discrete, reporting an extrapolated BER < 1E-37 is numerically corect, and informative\n\n");
	}

       
        proj_ber = exp(-beta * beta);
        proj_margin_12 = (sqrt(-log(1e-12)) - beta)/alpha;
        proj_margin_15 = (sqrt(-log(1e-15)) - beta)/alpha;
        proj_margin_18 = (sqrt(-log(1e-18)) - beta)/alpha;

        sq_err1 = (Eyy + (beta*beta) + (Exx*alpha*alpha) - 
                   (2*Ey*beta) - (2*Exy*alpha) + (2*Ex*beta*alpha));
        sq_err2 = 0;
	for( loop_index = start_n; loop_index < stop_n; loop_index++ ) {
	  ierr = (lbers[loop_index] - (alpha*margins[loop_index] + beta));
	  sq_err2 = sq_err2 + ierr*ierr/(float)delta_n;
        }

        outputs[0] = -_wc40_util_round_real(log(proj_ber)/log(10),1);
	outputs[1] = _wc40_util_round_real(proj_margin_18,1);
        outputs[2] = _wc40_util_round_real(proj_margin_12,1);
        outputs[3] = _wc40_util_round_real(proj_margin_15,1);

        SOC_DEBUG_PRINT((DK_PHY,"\t\tlog1e-12=%f, sq=%f\n",(float)log(1e-12),(float)sqrt(-log(1e-12))));
        SOC_DEBUG_PRINT((DK_PHY,"\t\talpha=%f\n",alpha));
        SOC_DEBUG_PRINT((DK_PHY,"\t\tbeta=%f\n",beta));
        SOC_DEBUG_PRINT((DK_PHY,"\t\tproj_ber=%f\n",proj_ber));
        SOC_DEBUG_PRINT((DK_PHY,"\t\tproj_margin12=%f\n",proj_margin_12));
        SOC_DEBUG_PRINT((DK_PHY,"\t\tproj_margin12=%f\n",proj_margin_15));
        SOC_DEBUG_PRINT((DK_PHY,"\t\tproj_margin18=%f\n",proj_margin_18));
        SOC_DEBUG_PRINT((DK_PHY,"\t\toutputs[0]=%f\n",outputs[0]));
        SOC_DEBUG_PRINT((DK_PHY,"\t\toutputs[1]=%f\n",outputs[1]));
        SOC_DEBUG_PRINT((DK_PHY,"\t\toutputs[2]=%f\n",outputs[2]));

#if 0
	/* Extrapolated results, low confidence */
	if( low_confidence == 1 ) {
	  if(beta_max){
	    soc_cm_print("BER(extrapolated) is *better* than 1e-37\n");
	    soc_cm_print("Margin @ 1e-12    is *better* than %d.%d\n", 
		     (int)outputs[2], ((int)(outputs[2]*1000.0) - ((int)outputs[2]*1000)));
	    soc_cm_print("Margin @ 1e-18    is *better* than %d.%d\n", 
		     (int)outputs[1], ((int)(outputs[1]*1000.0) - ((int)outputs[1]*1000)));
	  }
	  else{
	    soc_cm_print("BER(extrapolated) is *better* than 1e-%d\n", (int)outputs[0]);
	    soc_cm_print("Margin @ 1e-12    is *better* than %d.%d\n", 
		     (int)outputs[2], ((int)(outputs[2]*1000.0) - ((int)outputs[2]*1000)));
	    soc_cm_print("Margin @ 1e-18    is *better* than %d.%d\n", 
		     (int)outputs[1], ((int)(outputs[1]*1000.0) - ((int)outputs[1]*1000)));
	  }

	/* JPA> Extrapolated results, high confidence */
	} else {
	  if(beta_max){
	    soc_cm_print("BER(extrapolated) = 1e-37\n");
	    soc_cm_print("Margin @ 1e-12    is *better* than %d.%d\n", 
		     (int)outputs[2], ((int)(outputs[2]*1000.0) - ((int)outputs[2]*1000)));
	    soc_cm_print("Margin @ 1e-18    is *better* than %d.%d\n", 
		     (int)outputs[1], ((int)(outputs[1]*1000.0) - ((int)outputs[1]*1000)));
	  }
	  else{
	  soc_cm_print("BER(extrapolated) = 1e-%d\n", (int)outputs[0]);
          soc_cm_print("Margin @ 1e-12    = %d.%d\n", 
		     (int)outputs[2], ((int)(outputs[2]*1000.0) - ((int)outputs[2]*1000)));
          soc_cm_print("Margin @ 1e-18    = %d.%d\n", 
		     (int)outputs[1], ((int)(outputs[1]*1000.0) - ((int)outputs[1]*1000)));
	  }
	}
#else
        /* Extrapolated results, low confidence */
        if( low_confidence == 1 ) {
          if(beta_max){
            soc_cm_print("BER(extrapolated) is *better* than 1e-37\n");
            soc_cm_print("Margin @ 1e-12    is *better* than %f\n", outputs[2]);
            soc_cm_print("Margin @ 1e-15    is *better* than %f\n", outputs[3]);
            soc_cm_print("Margin @ 1e-18    is *better* than %f\n", outputs[1]);
          }
          else{
            soc_cm_print("BER(extrapolated) is *better* than 1e-%f\n", outputs[0]);
            soc_cm_print("Margin @ 1e-12    is *better* than %f\n", outputs[2]);
            soc_cm_print("Margin @ 1e-15    is *better* than %f\n", outputs[3]);
            soc_cm_print("Margin @ 1e-18    is *better* than %f\n", outputs[1]);
          }

        /* JPA> Extrapolated results, high confidence */
        } else {           
          if(beta_max){
            soc_cm_print("BER(extrapolated) = 1e-37\n");
            soc_cm_print("Margin @ 1e-12    is *better* than %f\n", outputs[2]);
            soc_cm_print("Margin @ 1e-15    is *better* than %f\n", outputs[3]);
            soc_cm_print("Margin @ 1e-18    is *better* than %f\n", outputs[1]);
          }
          else{
          soc_cm_print("BER(extrapolated) = 1e-%4.2f\n", outputs[0]);
          soc_cm_print("Margin @ 1e-12    = %4.2f%%\n", outputs[2]);
          soc_cm_print("Margin @ 1e-15    = %4.2f%%\n", outputs[3]);
          soc_cm_print("Margin @ 1e-18    = %4.2f%%\n", outputs[1]);
          }
        }

#endif
    }
    return SOC_E_NONE;
} 
#else   /* #ifndef __KERNEL__ */

int
wc40_eye_margin(int unit, soc_port_t port, int type)
{
    soc_cm_print("\nThis function is not supported in Linux kernel mode\n");
    return SOC_E_NONE;
}
#endif /* #ifndef __KERNEL__ */
#else /* INCLUDE_XGXS_WC40 */
int _xgxs_wc40_not_empty;
#endif /* INCLUDE_XGXS_WC40 */

/*
 * $Id: xgxs16g1l.c,v 1.20.78.1 Broadcom SDK $
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
 * File:        xgxs16g.c
 * Purpose:     Broadcom 1000 Mbps SerDes 
 *              (XGXS-16G1L)
 */

#include <sal/types.h>

#include <sal/types.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>
#include <soc/port_ability.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */ 

#if defined(INCLUDE_XGXS_16G)
#include "phyconfig.h"     /* Must include before other phy related includes */
#include "phyident.h"
#include "phyreg.h"
#include "phynull.h"
#include "xgxs.h"
#include "serdesid.h"
#include "xgxs16g.h"

#define XGXS16G_PLL_WAIT  250000
STATIC int phy_xgxs16g1l_an_set(int unit, soc_port_t port, int an);
STATIC int phy_xgxs16g1l_an_get(int unit, soc_port_t port,
                              int *an, int *an_done);
STATIC int phy_xgxs16g1l_speed_get(int unit, soc_port_t port, int *speed);
STATIC int phy_xgxs16g1l_ability_advert_set(int unit, soc_port_t port, 
                                         soc_port_ability_t *ability);
STATIC int phy_xgxs16g1l_ability_advert_get(int unit, soc_port_t port,
                                         soc_port_ability_t *ability);
STATIC int phy_xgxs16g1l_ability_local_get(int unit, soc_port_t port,
                                   soc_port_ability_t *ability); 

/* Notify event forward declaration */
STATIC int 
_phy_xgxs16g1l_notify_duplex(int unit, soc_port_t port, uint32 duplex);
STATIC int 
_phy_xgxs16g1l_notify_speed(int unit, soc_port_t port, uint32 speed);
STATIC int 
_phy_xgxs16g1l_notify_stop(int unit, soc_port_t port, uint32 flags);
STATIC int 
_phy_xgxs16g1l_notify_resume(int unit, soc_port_t port, uint32 flags);
STATIC int 
_phy_xgxs16g1l_notify_interface(int unit, soc_port_t port, uint32 intf);

STATIC int
_phy_xgxs16g1l_pll_lock_wait(int unit, soc_port_t port)
{
    uint16           data16;
    phy_ctrl_t      *pc;
    soc_timeout_t    to;
    int              rv;

    pc = INT_PHY_SW_STATE(unit, port);
    soc_timeout_init(&to, XGXS16G_PLL_WAIT, 0);
    while (!soc_timeout_check(&to)) {
        rv = READ_XGXS16G_XGXSBLK0_XGXSSTATUSr(unit, pc, &data16);
        if ((data16 & XGXSBLK0_XGXSSTATUS_TXPLL_LOCK_MASK) ||
            SOC_FAILURE(rv)) {
            break;
        }
    }
    if ((data16 & XGXSBLK0_XGXSSTATUS_TXPLL_LOCK_MASK) == 0) {
        SOC_DEBUG_PRINT((DK_WARN,
                       "XGXS_16G : TXPLL did not lock: u=%d p=%d\n",
                        unit, port));
        return (SOC_E_TIMEOUT);
    }
    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_init
 * Purpose:     
 *      Initialize xgxs6 phys
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_xgxs16g1l_init(int unit, soc_port_t port)
{
    uint16              data16;
    uint16              mask16;
    uint16             mode_1000x;
    int                 preemph, idriver, pdriver;
    phy_ctrl_t         *pc;
    soc_port_ability_t  ability;

    pc = INT_PHY_SW_STATE(unit, port);

    /* Initialize software state */
    pc->fiber.autoneg_enable = TRUE;

    if ((!PHY_FLAGS_TST(unit, port, PHY_FLAGS_INIT_DONE)) &&
        (pc->lane_num == 0)) {

        /* Reset the core */

        /* Stop PLL Sequencer and configure the core into correct mode */
        data16 = (XGXSBLK0_XGXSCONTROL_MODE_10G_IndLane <<
                    XGXSBLK0_XGXSCONTROL_MODE_10G_SHIFT) |
                 XGXSBLK0_XGXSCONTROL_HSTL_MASK |
                 XGXSBLK0_XGXSCONTROL_CDET_EN_MASK |
                 XGXSBLK0_XGXSCONTROL_EDEN_MASK |
                 XGXSBLK0_XGXSCONTROL_AFRST_EN_MASK |
                 XGXSBLK0_XGXSCONTROL_TXCKO_DIV_MASK;
        SOC_IF_ERROR_RETURN
            (WRITE_XGXS16G_XGXSBLK0_XGXSCONTROLr(unit, pc, data16)); 

        /* Disable IEEE block select auto-detect. 
         * The driver will select desired block as necessary.
         * By default, the driver keeps the XAUI block in
         * IEEE address space.
         */
        SOC_IF_ERROR_RETURN
            (MODIFY_XGXS16G_XGXSBLK0_MISCCONTROL1r(unit, pc, 
                           XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK,
                           XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_AUTODET_MASK |
                           XGXSBLK0_MISCCONTROL1_IEEE_BLKSEL_VAL_MASK));
    }

    /* Set Local Advertising Configuration */
    SOC_IF_ERROR_RETURN
        (phy_xgxs16g1l_ability_local_get(unit, port, &ability));
    /* Do not advertise 2.5G. 1Gbps vs 2.5 Gbps AN is not supported */
    ability.speed_full_duplex &= SOC_PA_SPEED_10MB | SOC_PA_SPEED_100MB |
                                 SOC_PA_SPEED_1000MB;
    pc->fiber.advert_ability = ability;
    SOC_IF_ERROR_RETURN
        (phy_xgxs16g1l_ability_advert_set(unit, port, &ability));
                                                                                
    /* Disable BAM in Independent Lane mode. Over1G AN not supported  */
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_BAM_NEXTPAGE_MP5_NEXTPAGECTRLr(unit, pc, 0));
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_BAM_NEXTPAGE_UD_FIELDr(unit, pc, 0));
                                                                                
    data16 = SERDESDIGITAL_CONTROL1000X1_CRC_CHECKER_DISABLE_MASK |
             SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK;
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_SERDESDIGITAL_CONTROL1000X1r(unit, pc, data16));

    mask16 = SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK |
             SERDESDIGITAL_CONTROL1000X1_DISABLE_PLL_PWRDWN_MASK |
             SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    /*
     * Configure signal auto-detection between SGMII and fiber.
     * This detection only works if auto-negotiation is enabled.
     */
    if (soc_property_port_get(unit, port, 
                              spn_SERDES_AUTOMEDIUM, FALSE)) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK;
    }

    /*
     * Put the Serdes in Fiber or SGMII mode
     */
    mode_1000x = 0;
    if ((PHY_FIBER_MODE(unit, port) && !PHY_EXTERNAL_MODE(unit, port)) ||
        PHY_PASSTHRU_MODE(unit, port)) {
        mode_1000x = SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    }
    /* Allow property to override */
    if (soc_property_port_get(unit, port,
                              spn_SERDES_FIBER_PREF, mode_1000x)) {
        data16 |= SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    } else {
        data16 &= ~SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;
    }

    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X1r(unit, pc, data16, mask16));   
    if ((PHY_FIBER_MODE(unit, port) && !PHY_EXTERNAL_MODE(unit, port)) ||
        PHY_PASSTHRU_MODE(unit, port) ||
        PHY_SGMII_AUTONEG_MODE(unit, port)) {
        /* Enable 1000X parallel detect */
        data16 = SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK;
        mask16 = SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK;
    } else {
        data16 = 0;
        mask16 = SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK |
                 SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_MASK;
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc, data16, mask16));

    /* Initialialize autoneg and fullduplex */
    data16 = MII_CTRL_FD | MII_CTRL_SS_1000;
    if ((PHY_FIBER_MODE(unit, port) && !PHY_EXTERNAL_MODE(unit, port)) ||
        PHY_PASSTHRU_MODE(unit, port) ||
        PHY_SGMII_AUTONEG_MODE(unit, port)) {
        data16 |= MII_CTRL_AE | MII_CTRL_RAN;
    }
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, data16));

    /* Disable 10G parallel detect */
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_AN73_PDET_PARDET10GCONTROLr(unit, pc, 0));

    /* Disable BAM mode and Teton mode */
    SOC_IF_ERROR_RETURN
        (WRITE_XGXS16G_BAM_NEXTPAGE_MP5_NEXTPAGECTRLr(unit, pc, 0));

    /* Initialize XAUI to reasonable default settings */
    preemph = soc_property_port_get(unit, port, spn_XGXS_PREEMPHASIS, 0);
    idriver = soc_property_port_get(unit, port, spn_XGXS_DRIVER_CURRENT, 9);
    pdriver = soc_property_port_get(unit, port, spn_XGXS_PRE_DRIVER_CURRENT, 9);
    data16  = ((preemph & 0xf) << TX_ALL_TX_DRIVER_PREEMPHASIS_SHIFT) |
              ((idriver & 0xf) << TX_ALL_TX_DRIVER_IDRIVER_SHIFT) |
              ((pdriver & 0xf) << TX_ALL_TX_DRIVER_IPREDRIVER_SHIFT);
    mask16  = TX_ALL_TX_DRIVER_PREEMPHASIS_MASK |
              TX_ALL_TX_DRIVER_IDRIVER_MASK |
              TX_ALL_TX_DRIVER_IPREDRIVER_MASK;
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_TX_ALL_TX_DRIVERr(unit, pc, data16, mask16));

    /* Enable lanes */
    mask16 = XGXSBLK1_LANECTRL0_CL36_PCS_EN_RX_MASK |
             XGXSBLK1_LANECTRL0_CL36_PCS_EN_TX_MASK;
    data16 = mask16;
    SOC_IF_ERROR_RETURN
      (MODIFY_XGXS16G_XGXSBLK1_LANECTRL0r(unit, pc, data16, mask16));

    /* set elasticity fifo size to 13.5k to support 12k jumbo pkt size*/
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X3r(unit,pc,
                   (1 << 2),
                   SERDESDIGITAL_CONTROL1000X3_FIFO_ELASICITY_TX_RX_MASK));

    if ((!PHY_FLAGS_TST(unit, port, PHY_FLAGS_INIT_DONE)) &&
        (pc->lane_num == 0)) {
        /* Start PLL Sequencer and wait for PLL to lock */
        SOC_IF_ERROR_RETURN
            (MODIFY_XGXS16G_XGXSBLK0_XGXSCONTROLr(unit, pc,
                                 XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK,
                                 XGXSBLK0_XGXSCONTROL_START_SEQUENCER_MASK));
        (void) _phy_xgxs16g1l_pll_lock_wait(unit, port);
    }

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_link_get
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
phy_xgxs16g1l_link_get(int unit, soc_port_t port, int *link)
{
    phy_ctrl_t *pc;
    uint16  mii_serdes_stat;

    if (PHY_DISABLED_MODE(unit, port)) {
        *link = FALSE;
        return (SOC_E_NONE);
    }

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_COMBO_IEEE0_MIISTATr(unit, pc, &mii_serdes_stat));

    *link = ((mii_serdes_stat & MII_STAT_LA) != 0);

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_enable_set
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
phy_xgxs16g1l_enable_set(int unit, soc_port_t port, int enable)
{
    int rv;
    phy_ctrl_t *pc;
   
    pc = INT_PHY_SW_STATE(unit, port);

    if (enable) {
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_DISABLE);
        rv = _phy_xgxs16g1l_notify_resume(unit, port, PHY_STOP_PHY_DIS);
    } else {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_DISABLE);
        rv = _phy_xgxs16g1l_notify_stop(unit, port, PHY_STOP_PHY_DIS);
    }

    return (rv);
}

/*
 * Function:
 *      phy_xgxs16g1l_speed_set
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
phy_xgxs16g1l_speed_set(int unit, soc_port_t port, int speed)
{
    phy_ctrl_t  *pc;
    uint16       speed_val, mask;
    uint16       speed_mii;

    pc = INT_PHY_SW_STATE(unit, port);
                                                                               
    if (speed > 2500) {
        return (SOC_E_CONFIG);
    }

    speed_val = 0;
    speed_mii = 0;
    mask      = SERDESDIGITAL_MISC1_FORCE_SPEED_SEL_MASK |
                SERDESDIGITAL_MISC1_FORCE_SPEED_MASK;

    switch (speed) {
    case 0:
        /* Do not change speed */
        return SOC_E_NONE;
    case 10:
        speed_mii = MII_CTRL_SS_10;
        break;
    case 100:
        speed_mii = MII_CTRL_SS_100;
        break;
    case 1000:
        speed_mii = MII_CTRL_SS_1000;
        break;
    case 2500:
        speed_val = 0;
        break;
    default:
        return SOC_E_PARAM;
    }

    /* 2.5G only valid in fiber mode */
    if (speed == 2500) {
        speed_val |= SERDESDIGITAL_MISC1_FORCE_SPEED_SEL_MASK;
    }

    /* Hold rxSeqStart */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_RX0_RX_CONTROLr(unit, pc,
                                  DSC_2_0_DSC_CTRL0_RXSEQSTART_MASK,
                                  DSC_2_0_DSC_CTRL0_RXSEQSTART_MASK));
                                                                               
    /* hold TX FIFO in reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X3r(unit, pc,
                           SERDESDIGITAL_CONTROL1000X3_TX_FIFO_RST_MASK,
                           SERDESDIGITAL_CONTROL1000X3_TX_FIFO_RST_MASK));

    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_MISC1r(unit, pc, speed_val, mask));
                                                                               
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, speed_mii,
                                              MII_CTRL_SS_MASK));

    /* release rxSeqStart */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_RX0_RX_CONTROLr(unit, pc,
                              0,
                              DSC_2_0_DSC_CTRL0_RXSEQSTART_MASK));
                                                                               
    /* release TX FIFO reset */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X3r(unit, pc,
                           0,
                           SERDESDIGITAL_CONTROL1000X3_TX_FIFO_RST_MASK));

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_speed_get
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
phy_xgxs16g1l_speed_get(int unit, soc_port_t port, int *speed)
{
    uint16 data16;
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    if (NULL == speed) {
        return (SOC_E_PARAM);
    }
 
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_SERDESDIGITAL_STATUS1000X1r(unit, pc, &data16));

    data16 &= SERDESDIGITAL_STATUS1000X1_SPEED_STATUS_MASK;
    data16 >>= SERDESDIGITAL_STATUS1000X1_SPEED_STATUS_SHIFT;

    if (data16 == 3) {
        *speed= 2500;
    } else if (data16 == 2) {
        *speed= 1000;
    } else if (data16 == 1) {
        *speed= 100;
    } else {   
        *speed= 10;
    }    
    
    return (SOC_E_NONE);
}

/*
 * Function:    
 *      phy_xgxs16g1l_an_set
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
STATIC int
phy_xgxs16g1l_an_set(int unit, soc_port_t port, int an)
{
    phy_ctrl_t  *pc;
    uint16             an_enable;
    uint16             auto_det;
                                                                                
    pc = INT_PHY_SW_STATE(unit, port);
                                                                                
    SOC_DEBUG_PRINT((DK_PHY,
                     "phy_xgxs16g1l_an_set: u=%d p=%d an=%d\n",
                     unit, port, an));
                                                                                
    an_enable = 0;
    auto_det  = 0;
                                                                                
    if (an) {
                                                                                
        an_enable = MII_CTRL_AE | MII_CTRL_RAN;
                                                                                
        /*
         * Should read one during init and cache it in Phy flags
         */
        if (soc_property_port_get(unit, port,
                                  spn_SERDES_AUTOMEDIUM,
                                  FALSE)) {
            auto_det = SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK;
        }
                                                                                
        /* If auto-neg is enabled, make sure not forcing any speed */
        SOC_IF_ERROR_RETURN
            (MODIFY_XGXS16G_SERDESDIGITAL_MISC1r(unit, pc, 0,
                                   SERDESDIGITAL_MISC1_FORCE_SPEED_SEL_MASK));
        /* Enable/Disable auto detect */
        SOC_IF_ERROR_RETURN
             (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X1r(unit, pc, auto_det,
                                 SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK));
                                                                                
    } else {
        /* Disable auto detect */
        SOC_IF_ERROR_RETURN
             (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X1r(unit, pc, auto_det,
                                 SERDESDIGITAL_CONTROL1000X1_AUTODET_EN_MASK));
                                                                                
    }
                                                                                
    /* restart the autoneg if enabled, or disable the autoneg */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, an_enable,
                                      MII_CTRL_AE | MII_CTRL_RAN));
                                                                                
    pc->fiber.autoneg_enable = an;
                                                                                
    return SOC_E_NONE;
}


/*
 * Function:    
 *      phy_xgxs16g1l_an_get
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
phy_xgxs16g1l_an_get(int unit, soc_port_t port, int *an, int *an_done)
{
    uint16      mii_ctrl;
    uint16      mii_stat;
    phy_ctrl_t *pc;
                                                                                
    if ((NULL == an) || (NULL == an_done)) {
        return (SOC_E_PARAM);
    }

    pc = INT_PHY_SW_STATE(unit, port);
                                                                                
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, &mii_ctrl));
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_COMBO_IEEE0_MIISTATr(unit, pc, &mii_stat));
                                                                                
    *an = (mii_ctrl & MII_CTRL_AE) ? TRUE : FALSE;
    *an_done = (mii_stat & MII_STAT_AN_DONE) ? TRUE : FALSE;
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_xgxs16g1l_ability_advert_set
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
phy_xgxs16g1l_ability_advert_set(int unit, soc_port_t port,
                          soc_port_ability_t *ability)
{
    uint16           an_adv;
    phy_ctrl_t      *pc;
                                                                                
    if (NULL == ability) {
        return (SOC_E_PARAM);
    }
                                                                                
    pc = INT_PHY_SW_STATE(unit, port);
                                                                                
    /* Set advertised duplex (only FD supported).  */
    an_adv = (ability->speed_full_duplex & SOC_PA_SPEED_1000MB) ? 
              MII_ANA_C37_FD : 0;
                                                                                
    /* Set advertised pause bits in link code word.  */
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
        (WRITE_XGXS16G_COMBO_IEEE0_AUTONEGADVr(unit, pc, an_adv));
                                                                                
    SOC_DEBUG_PRINT((DK_PHY,
        "phy_xgxs16g1l_ability_advert_set: u=%d p=%d pause=%08x OVER1G_UP1 %04x\n",
        unit, port, ability->pause, an_adv));

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_ability_advert_get
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
phy_xgxs16g1l_ability_advert_get(int unit, soc_port_t port,
                              soc_port_ability_t *ability)
{
    phy_ctrl_t      *pc;
    uint16           an_adv;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = INT_PHY_SW_STATE(unit, port);

    sal_memset(ability, 0, sizeof(*ability));
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_COMBO_IEEE0_AUTONEGADVr(unit, pc, &an_adv));
                                                                                
    ability->speed_full_duplex = 
                  (an_adv & MII_ANA_C37_FD) ? SOC_PA_SPEED_1000MB : 0;

    switch (an_adv & (MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE)) {
        case MII_ANA_C37_PAUSE:
            ability->pause = SOC_PA_PAUSE_TX | SOC_PA_PAUSE_RX;
            break;
        case MII_ANA_C37_ASYM_PAUSE:
            ability->pause = SOC_PA_PAUSE_TX;
            break;
        case MII_ANA_C37_PAUSE | MII_ANA_C37_ASYM_PAUSE:
            ability->pause = SOC_PA_PAUSE_RX;
            break;
    }
                                                                                
    SOC_DEBUG_PRINT((DK_PHY,
     "phy_xgxs16g1l_ability_advert_get:unit=%d p=%d pause=%08x sp=%08x\n",
     unit, port, ability->pause, ability->speed_full_duplex));
                                                                                
    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_ability_remote_get
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
phy_xgxs16g1l_ability_remote_get(int unit, soc_port_t port,
                               soc_port_ability_t *ability)
{
    if (NULL == ability) {
        return (SOC_E_PARAM);
    }
 
    sal_memset(ability, 0, sizeof(*ability));
    return (SOC_E_NONE);
}


/*
 * Function:
 *      phy_xgxs16g1l_lb_set
 * Purpose:
 *      Put XGXS6/FusionCore in PHY loopback
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - binary value for on/off (1/0)
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_xgxs16g1l_lb_set(int unit, soc_port_t port, int enable)
{
    uint16      misc_ctrl;
    uint16      lb_bit;
    uint16      lb_mask;
    phy_ctrl_t *pc;

    pc = INT_PHY_SW_STATE(unit, port);
    
    /* Configure Loopback in XAUI */
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_XGXSBLK0_MISCCONTROL1r(unit, pc, &misc_ctrl));
    if (misc_ctrl & XGXSBLK0_MISCCONTROL1_PCS_DEV_EN_OVERRIDE_MASK) {
          /* PCS */
          lb_bit  = (enable) ? IEEE0BLK_IEEECONTROL0_GLOOPBACK_MASK : 0;
          lb_mask = IEEE0BLK_IEEECONTROL0_GLOOPBACK_MASK;
    } else if (misc_ctrl & XGXSBLK0_MISCCONTROL1_PMD_DEV_EN_OVERRIDE_MASK) {
          /* PMA/PMD */
          lb_bit  = (enable) ? 1 : 0;
          lb_mask = 1;
    } else {
          /* PHY XS, DTE XS */
          lb_bit  = (enable) ? IEEE0BLK_IEEECONTROL0_GLOOPBACK_MASK : 0;
          lb_mask = IEEE0BLK_IEEECONTROL0_GLOOPBACK_MASK;
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_IEEE0BLK_IEEECONTROL0r(unit, pc, lb_bit, lb_mask));

    /* Configure Loopback in SerDes */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, 
                                             (enable) ? MII_CTRL_LE : 0,
                                             MII_CTRL_LE));

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_lb_get
 * Purpose:
 *      Get XGXS6/FusionCore PHY loopback state
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #. 
 *      enable - address of location to store binary value for on/off (1/0)
 * Returns:     
 *      SOC_E_NONE
 */

STATIC int
phy_xgxs16g1l_lb_get(int unit, soc_port_t port, int *enable)
{
    uint16      mii_ctrl;
    phy_ctrl_t *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, &mii_ctrl));
    *enable = ((mii_ctrl & MII_CTRL_LE) == MII_CTRL_LE);

    return (SOC_E_NONE);
}


STATIC int
phy_xgxs16g1l_ability_local_get(int unit, soc_port_t port,
                              soc_port_ability_t *ability)
{
    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;
    ability->speed_full_duplex  = SOC_PA_SPEED_1000MB;
    if (PHY_FIBER_MODE(unit, port))   {
        ability->speed_full_duplex  |= SOC_PA_SPEED_2500MB;
    } else {
        ability->speed_half_duplex  = SOC_PA_SPEED_10MB |
                                      SOC_PA_SPEED_100MB;
        ability->speed_full_duplex  |= SOC_PA_SPEED_10MB |
                                       SOC_PA_SPEED_100MB;
    }

    ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM; 
    ability->interface = SOC_PA_INTF_GMII | SOC_PA_INTF_SGMII;
    ability->medium    = SOC_PA_MEDIUM_FIBER;
    ability->loopback  = SOC_PA_LB_PHY;
    ability->flags     = SOC_PA_AUTONEG;

    return (SOC_E_NONE);
}

STATIC int
_phy_xgxs16g1l_control_tx_driver_set(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 value)
{
    uint16  data;             /* Temporary holder of reg value to be written */
    uint16  mask;             /* Bit mask of reg value to be updated */

    if (value > 15) {
        return SOC_E_PARAM;
    }

    data = mask = 0;
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS:
         data = value << TX_ALL_TX_DRIVER_PREEMPHASIS_SHIFT;
         mask = TX_ALL_TX_DRIVER_PREEMPHASIS_MASK;
         break;
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
         data = value << TX_ALL_TX_DRIVER_IDRIVER_SHIFT;
         mask = TX_ALL_TX_DRIVER_IDRIVER_MASK;
         break;
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
         data = value << TX_ALL_TX_DRIVER_IPREDRIVER_SHIFT;
         mask = TX_ALL_TX_DRIVER_IPREDRIVER_MASK;
         break;
    default:
         /* should never get here */
         return SOC_E_PARAM;
    }

    /* Update preemphasis/driver/pre-driver current */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_TX_ALL_TX_DRIVERr(unit, pc, data, mask));
    return SOC_E_NONE;
}

STATIC int
_phy_xgxs16g1l_control_linkdown_transmit_set(int unit, soc_port_t port, 
                                           uint32 value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    if (value) {
        data = (SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_BITS <<
                SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_SHIFT) |
               (SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_BITS <<
                SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_SHIFT) |
               (SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_BITS <<
                SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_SHIFT);
        SOC_IF_ERROR_RETURN
            (WRITE_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc, data));
    } else {
        data = (SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_BITS <<
                SERDESDIGITAL_CONTROL1000X2_DISABLE_FALSE_LINK_SHIFT) |
               (SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_BITS <<
                SERDESDIGITAL_CONTROL1000X2_FILTER_FORCE_LINK_SHIFT) |
               (SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_BITS <<
                SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_SHIFT);
        SOC_IF_ERROR_RETURN
            (WRITE_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc, data));
    }
    return SOC_E_NONE;
}

STATIC int
_phy_xgxs16g1l_control_linkdown_transmit_get(int unit, soc_port_t port, 
                                           uint32 *value)
{
    uint16      data;
    phy_ctrl_t *pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
            (READ_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc, &data));

    *value = (data & 
              (SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_BITS <<
               SERDESDIGITAL_CONTROL1000X2_FORCE_XMIT_DATA_ON_TXSIDE_SHIFT))
             ? 1 : 0;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_xgxs16g1l_control_set
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
phy_xgxs16g1l_control_set(int unit, soc_port_t port,
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
    case SOC_PHY_CONTROL_PREEMPHASIS:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        rv = _phy_xgxs16g1l_control_tx_driver_set(unit, pc, type, value);
        break;
    case SOC_PHY_CONTROL_EQUALIZER_BOOST:
        rv = MODIFY_XGXS16G_RX0_RX_ANALOGBIAS0r(unit, pc, value,
                                  RX_ALL_RX_EQ_BOOST_EQUALIZER_CONTROL_MASK);
        break;
    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        rv = _phy_xgxs16g1l_control_linkdown_transmit_set(unit, port, value);
        break;
    case SOC_PHY_CONTROL_PARALLEL_DETECTION:
        /* enable 1000X parallel detect */
        SOC_IF_ERROR_RETURN
            (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc,
            value? SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK:0,              SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK));
        rv = SOC_E_NONE;
        break;

    default:
        rv = (SOC_E_UNAVAIL);
        break;
    }
    return rv;
}

STATIC int
_phy_xgxs16g1l_control_tx_driver_get(int unit, phy_ctrl_t *pc,
                                soc_phy_control_t type, uint32 *value)
{
    uint16  data16;           /* Temporary holder of 16 bit reg value */

    /* Read preemphasis/driver/pre-driver current */
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_TX_ALL_TX_DRIVERr(unit, pc, &data16));

    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS:
         *value = (data16 & TX_ALL_TX_DRIVER_PREEMPHASIS_MASK) >>
                  TX_ALL_TX_DRIVER_PREEMPHASIS_SHIFT;
         break;
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
         *value = (data16 & TX_ALL_TX_DRIVER_IDRIVER_MASK) >> 
                  TX_ALL_TX_DRIVER_IDRIVER_SHIFT;
         break;
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
         *value = (data16 & TX_ALL_TX_DRIVER_IPREDRIVER_MASK) >>
                  TX_ALL_TX_DRIVER_IPREDRIVER_SHIFT;
         break;
    default:
         /* should never get here */
         return (SOC_E_PARAM);
    }
    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_control_get
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
phy_xgxs16g1l_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    int rv;
    phy_ctrl_t *pc;
    uint16 data16;

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return (SOC_E_PARAM);
    }

    pc = INT_PHY_SW_STATE(unit, port);
    rv = SOC_E_UNAVAIL;
    switch(type) {
    case SOC_PHY_CONTROL_PREEMPHASIS:
        /* fall through */
    case SOC_PHY_CONTROL_DRIVER_CURRENT:
        /* fall through */
    case SOC_PHY_CONTROL_PRE_DRIVER_CURRENT:
        rv = _phy_xgxs16g1l_control_tx_driver_get(unit, pc, type, value);
        break;
    case SOC_PHY_CONTROL_EQUALIZER_BOOST:
        SOC_IF_ERROR_RETURN
            (READ_XGXS16G_RX0_RX_ANALOGBIAS0r(unit, pc, &data16));
        *value = data16 & RX_ALL_RX_EQ_BOOST_EQUALIZER_CONTROL_MASK;
        rv = SOC_E_NONE;
        break;
    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        rv = _phy_xgxs16g1l_control_linkdown_transmit_get(unit, port, value);
        break;

    case SOC_PHY_CONTROL_PARALLEL_DETECTION:
        SOC_IF_ERROR_RETURN
            (READ_XGXS16G_SERDESDIGITAL_CONTROL1000X2r(unit, pc,&data16));
        *value =
           data16 & SERDESDIGITAL_CONTROL1000X2_ENABLE_PARALLEL_DETECTION_MASK?
             TRUE: FALSE;
        rv = SOC_E_NONE;
        break;

    default:
        rv = (SOC_E_UNAVAIL);
        break;
    }
    return rv;
}

/*
 * Function:
 *      phy_xgxs16g1l_reg_read
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
phy_xgxs16g1l_reg_read(int unit, soc_port_t port, uint32 flags,
                  uint32 phy_reg_addr, uint32 *phy_data)
{
    uint16               data;     /* Temporary holder for phy_data */
    phy_ctrl_t          *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_reg_aer_read(unit, pc, phy_reg_addr, &data));

   *phy_data = data;

    return (SOC_E_NONE);
}

/*
 * Function:
 *      phy_xgxs16g1l_reg_write
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
phy_xgxs16g1l_reg_write(int unit, soc_port_t port, uint32 flags,
                   uint32 phy_reg_addr, uint32 phy_data)
{
    uint16               data;     /* Temporary holder for phy_data */
    phy_ctrl_t          *pc;      /* PHY software state */

    pc = INT_PHY_SW_STATE(unit, port);

    data      = (uint16) (phy_data & 0x0000FFFF);

    SOC_IF_ERROR_RETURN
        (phy_reg_aer_write(unit, pc, phy_reg_addr, data));

    return (SOC_E_NONE);
}  

/*
 * Function:
 *      phy_xgxs16g1l_reg_modify
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
phy_xgxs16g1l_reg_modify(int unit, soc_port_t port, uint32 flags,
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
        (phy_reg_aer_modify(unit, pc, phy_reg_addr, data, mask));

    return (SOC_E_NONE);
}

STATIC int
phy_xgxs16g1l_probe(int unit, phy_ctrl_t *pc)
{
    uint16      serdes_id0;

    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_SERDESID_SERDESID0r(unit, pc, &serdes_id0));

    if ((serdes_id0 & SERDESID_SERDESID0_MODEL_NUMBER_MASK) !=
        SERDES_ID0_MODEL_NUMBER_XGXS_16G) {
        return SOC_E_NOT_FOUND;
    }

    if ((serdes_id0 & SERDESID_SERDESID0_TECH_PROC_MASK) !=
        SERDES_ID0_TECH_PROC_65NM) {
        return SOC_E_NOT_FOUND;
    }

    if ((serdes_id0 & SERDESID_SERDESID0_REV_NUMBER_MASK) !=
        SERDES_ID0_REV_NUMBER_0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}

STATIC int
phy_xgxs16g1l_notify(int unit, soc_port_t port,
                 soc_phy_event_t event, uint32 value)
{
    int             rv;

    if (event >= phyEventCount) {
        return SOC_E_PARAM;
    }

    rv = SOC_E_NONE;
    switch(event) {
    case phyEventInterface:
        rv = (_phy_xgxs16g1l_notify_interface(unit, port, value));
        break;
    case phyEventDuplex:
        rv = (_phy_xgxs16g1l_notify_duplex(unit, port, value));
        break;
    case phyEventSpeed:
        rv = (_phy_xgxs16g1l_notify_speed(unit, port, value));
        break;
    case phyEventStop:
        rv = (_phy_xgxs16g1l_notify_stop(unit, port, value));
        break;
    case phyEventResume:
        rv = (_phy_xgxs16g1l_notify_resume(unit, port, value));
        break;
    default:
        rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 * Function:
 *      phy_xgxs16g1l_duplex_set
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
phy_xgxs16g1l_duplex_set(int unit, soc_port_t port, int duplex)
{
    uint16       data16;
    phy_ctrl_t  *pc;
                                                                                
    pc = INT_PHY_SW_STATE(unit, port);
                                                                                
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_GP_STATUS_STATUS1000X1r(unit, pc, &data16));

    if (!(data16 & GP_STATUS_STATUS1000X1_SGMII_MODE_MASK)) {
        /* 1000X fiber mode, always full duplex */
        duplex = TRUE;
    }

    data16 = duplex? MII_CTRL_FD: 0;
 
    /* program the duplex setting */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, data16,MII_CTRL_FD));
                                                                                
    return SOC_E_NONE;
}

STATIC int
phy_xgxs16g1l_duplex_get(int unit, soc_port_t port, int *duplex)
{
    uint16       reg0_16;
    uint16       mii_ctrl;
    phy_ctrl_t  *pc;
                                                                               
    pc = INT_PHY_SW_STATE(unit, port);
                                                                               
    *duplex = TRUE;
                                                                               
    SOC_IF_ERROR_RETURN
        (READ_XGXS16G_GP_STATUS_STATUS1000X1r(unit, pc, &reg0_16));
                                                                               
    if (reg0_16 & GP_STATUS_STATUS1000X1_SGMII_MODE_MASK) {
                                                                               
    /* retrieve the duplex setting in SGMII mode */
        SOC_IF_ERROR_RETURN
            (READ_XGXS16G_COMBO_IEEE0_MIICNTLr(unit, pc, &mii_ctrl));
                                                                               
        if (mii_ctrl & MII_CTRL_AE) {
            SOC_IF_ERROR_RETURN
                (READ_XGXS16G_COMBO_IEEE0_AUTONEGLPABILr(unit,pc,&reg0_16));
                                                                               
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
 * Variable:
 *      phy_xgxs16g1l_drv
 * Purpose:
 *      Phy Driver for 10G (XAUI x 4) Serdes PHY. 
 */

phy_driver_t phy_xgxs16g1l_ge = {
    /* .drv_name                      = */ "XGXS16G/1 Unicore PHY Driver",
    /* .pd_init                       = */ phy_xgxs16g1l_init,
    /* .pd_reset                      = */ phy_null_reset,
    /* .pd_link_get                   = */ phy_xgxs16g1l_link_get,
    /* .pd_enable_set                 = */ phy_xgxs16g1l_enable_set,
    /* .pd_enable_get                 = */ phy_null_enable_get,
    /* .pd_duplex_set                 = */ phy_xgxs16g1l_duplex_set,
    /* .pd_duplex_get                 = */ phy_xgxs16g1l_duplex_get,
    /* .pd_speed_set                  = */ phy_xgxs16g1l_speed_set,
    /* .pd_speed_get                  = */ phy_xgxs16g1l_speed_get,
    /* .pd_master_set                 = */ phy_null_set,
    /* .pd_master_get                 = */ phy_null_zero_get,
    /* .pd_an_set                     = */ phy_xgxs16g1l_an_set,
    /* .pd_an_get                     = */ phy_xgxs16g1l_an_get,
    /* .pd_adv_local_set              = */ NULL, /* Deprecated */
    /* .pd_adv_local_get              = */ NULL, /* Deprecated */
    /* .pd_adv_remote_get             = */ NULL, /* Deprecated */ 
    /* .pd_lb_set                     = */ phy_xgxs16g1l_lb_set,
    /* .pd_lb_get                     = */ phy_xgxs16g1l_lb_get,
    /* .pd_interface_set              = */ phy_null_interface_set,
    /* .pd_interface_get              = */ phy_null_interface_get,
    /* .pd_ability                    = */ NULL, /* Deprecated */ 
    /* .pd_linkup_evt                 = */ NULL,
    /* .pd_linkdn_evt                 = */ NULL,
    /* .pd_mdix_set                   = */ phy_null_mdix_set,
    /* .pd_mdix_get                   = */ phy_null_mdix_get,
    /* .pd_mdix_status_get            = */ phy_null_mdix_status_get,
    /* .pd_medium_config_set          = */ NULL,
    /* .pd_medium_config_get          = */ NULL,
    /* .pd_medium_get                 = */ phy_null_medium_get,
    /* .pd_cable_diag                 = */ NULL,
    /* .pd_link_change                = */ NULL,
    /* .pd_control_set                = */ phy_xgxs16g1l_control_set,
    /* .pd_control_get                = */ phy_xgxs16g1l_control_get,
    /* .pd_reg_read                   = */ phy_xgxs16g1l_reg_read,
    /* .pd_reg_write                  = */ phy_xgxs16g1l_reg_write,
    /* .pd_reg_modify                 = */ phy_xgxs16g1l_reg_modify,
    /* .pd_notify                     = */ phy_xgxs16g1l_notify,
    /* .pd_probe                      = */ phy_xgxs16g1l_probe,
    /* .pd_ability_advert_set         = */ phy_xgxs16g1l_ability_advert_set, 
    /* .pd_ability_advert_get         = */ phy_xgxs16g1l_ability_advert_get,
    /* .pd_ability_remote_get         = */ phy_xgxs16g1l_ability_remote_get,
    /* .pd_ability_local_get          = */ phy_xgxs16g1l_ability_local_get
};

/***********************************************************************
 *
 * PASS-THROUGH NOTIFY ROUTINES
 *
 ***********************************************************************/

/*
 * Function:
 *      phy_xgxs16g1l_notify_duplex
 * Purpose:
 *      Program duplex if (and only if) 56xxx is an intermediate PHY.
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
 *      When used in SGMII mode, autoneg must be turned off and
 *      the speed/duplex forced to match that of the external PHY.
 */
STATIC int
_phy_xgxs16g1l_notify_duplex(int unit, soc_port_t port, uint32 duplex)
{
    int                 fiber;
    phy_ctrl_t  *pc;
                                                                               
    fiber = PHY_FIBER_MODE(unit, port);
    pc    = INT_PHY_SW_STATE(unit, port);
    SOC_DEBUG_PRINT((DK_PHY,
                     "_phy_xgxs16g1l_notify_duplex: "
                     "u=%d p=%d duplex=%d fiber=%d\n",
                     unit, port, duplex, fiber));
                                                                               
    if (SAL_BOOT_SIMULATION) {
        return SOC_E_NONE;
    }
                                                                               
    /* Put SERDES PHY in reset */
                                                                               
    SOC_IF_ERROR_RETURN
        (_phy_xgxs16g1l_notify_stop(unit, port, PHY_STOP_DUPLEX_CHG));

    SOC_IF_ERROR_RETURN
        (phy_xgxs16g1l_duplex_set(unit,port,duplex));

    /* Take SERDES PHY out of reset */
    SOC_IF_ERROR_RETURN
        (_phy_xgxs16g1l_notify_resume(unit, port, PHY_STOP_DUPLEX_CHG));
                                                                               
    /* Autonegotiation must be turned off to talk to external PHY if
     * SGMII autoneg is not enabled.
     */
    if (!PHY_SGMII_AUTONEG_MODE(unit, port)) {
        SOC_IF_ERROR_RETURN
            (phy_xgxs16g1l_an_set(unit, port, FALSE));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_xgxs16g1l_notify_speed
 * Purpose:
 *      Program duplex if (and only if) 56xxx is an intermediate PHY.
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
_phy_xgxs16g1l_notify_speed(int unit, soc_port_t port, uint32 speed)
{
    phy_ctrl_t  *pc;
                                                                               
    pc    = INT_PHY_SW_STATE(unit, port);

    /* Put SERDES PHY in reset */
    SOC_IF_ERROR_RETURN
        (_phy_xgxs16g1l_notify_stop(unit, port, PHY_STOP_SPEED_CHG));

    /* Update speed */
    SOC_IF_ERROR_RETURN
        (phy_xgxs16g1l_speed_set(unit, port, speed));

    /* Take SERDES PHY out of reset */
    SOC_IF_ERROR_RETURN
        (_phy_xgxs16g1l_notify_resume(unit, port, PHY_STOP_SPEED_CHG));

    /* Autonegotiation must be turned off to talk to external PHY */
    if (!PHY_SGMII_AUTONEG_MODE(unit, port) && PHY_EXTERNAL_MODE(unit, port)) {        SOC_IF_ERROR_RETURN
            (phy_xgxs16g1l_an_set(unit, port, FALSE));
    }

    return (SOC_E_NONE);
}

/*
 * Function:
 *      _phy_xgxs16g1l_stop
 * Purpose:
 *      Put 56xxx SERDES in or out of reset depending on conditions
 */

STATIC int
_phy_xgxs16g1l_stop(int unit, soc_port_t port)
{
    uint16 mask16,data16;
    int          stop, copper;
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

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
                   "phy_xgxs16g1l_stop: u=%d p=%d copper=%d stop=%d flg=0x%x\n",
                    unit, port, copper, stop,
                    pc->stop));

    mask16 = (1 << pc->lane_num);    /* rx lane */
    mask16 |= (mask16 << 4); /* add tx lane */
                                                                               
    if (stop) {
        mask16 |= 0x800;
        data16 = mask16;
    } else {
        data16 = 0;
    }
                                                                               
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_XGXSBLK1_LANECTRL3r(unit,pc,data16,mask16));
                                                                               
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_xgxs16g1l_notify_stop
 * Purpose:
 *      Add a reason to put 56xxx PHY in reset.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flags - Reason to stop
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_phy_xgxs16g1l_notify_stop(int unit, soc_port_t port, uint32 flags)
{
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    pc->stop |= flags;

    return _phy_xgxs16g1l_stop(unit, port);
}

/*
 * Function:
 *      phy_xgxs16g1l_notify_resume
 * Purpose:
 *      Remove a reason to put 56xxx PHY in reset.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flags - Reason to stop
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
_phy_xgxs16g1l_notify_resume(int unit, soc_port_t port, uint32 flags)
{
    phy_ctrl_t  *pc;

    pc = INT_PHY_SW_STATE(unit, port);

    pc->stop &= ~flags;

    return _phy_xgxs16g1l_stop(unit, port);
}


STATIC int
_phy_xgxs16g1l_notify_interface(int unit, soc_port_t port, uint32 intf)
{
    phy_ctrl_t  *pc;
    uint16       data16;

    pc = INT_PHY_SW_STATE(unit, port);

    data16 = (intf == SOC_PORT_IF_SGMII) ? 
                 0 : SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK;

    /* Disable 1000X/SGMII auto detect and force to 1000X.
     * 1000X mode is not supported in this driver.
     */
    SOC_IF_ERROR_RETURN
        (MODIFY_XGXS16G_SERDESDIGITAL_CONTROL1000X1r(unit, pc, data16,
                      SERDESDIGITAL_CONTROL1000X1_FIBER_MODE_1000X_MASK));

    return SOC_E_NONE;
}

#else
int _phy_xgxs16g1l_not_empty;
#endif /*  INCLUDE_XGXS_16G */

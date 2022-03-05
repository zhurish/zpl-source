/*
 * $Id: phy54680.c,v 1.75.2.16 Broadcom SDK $
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
 * File:        phy54680.c
 * Purpose:     PHY driver for BCM54680
 */
#include <sal/types.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>

#include "phydefs.h"      /* Must include before other phy related includes */

#if defined(INCLUDE_PHY_54680) || defined(INCLUDE_PHY_54680E) || defined(INCLUDE_PHY_52681E)
#include "phyconfig.h"    /* Must be the first phy include after phydefs.h */

#include "phyident.h"
#include "phyreg.h"
#include "phyfege.h"
#include "phynull.h"
#include "phy54680.h"

#define PHY_IS_BCM54680(_pc)      (PHY_MODEL_CHECK((_pc), \
                                                PHY_BCM54680_OUI, \
                                                PHY_BCM54680_MODEL))

#define PHY_IS_BCM54680_A0(_pc)   (PHY_MODEL_CHECK((_pc), \
                                                PHY_BCM54680_OUI, \
                                                PHY_BCM54680_MODEL) \
                                   && ((_pc)->phy_rev == 0x0 ))

#define PHY_BCM54680_A0_ID0    PHY_ID0(PHY_BCM54680_OUI,PHY_BCM54680_MODEL,0)
#define PHY_BCM54680_A0_ID1    PHY_ID1(PHY_BCM54680_OUI,PHY_BCM54680_MODEL,0)

#define PHY_IS_BCM54880E(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM54880E_OUI, \
                                 PHY_BCM54880E_MODEL))

#define PHY_IS_BCM54880E_A0(_pc) (PHY_MODEL_CHECK((_pc), \
                                  PHY_BCM54880E_OUI, \
                                  PHY_BCM54880E_MODEL) \
                                  && ((_pc)->phy_rev == 0x0 ))

#define PHY_IS_BCM54880E_A1(_pc) (PHY_MODEL_CHECK((_pc), \
                                  PHY_BCM54880E_OUI, \
                                  PHY_BCM54880E_MODEL) \
                                  && ((_pc)->phy_rev == 0x1))

#define PHY_IS_BCM54880E_A0A1(_pc) (PHY_MODEL_CHECK((_pc), \
                                  PHY_BCM54880E_OUI, \
                                  PHY_BCM54880E_MODEL) \
                                  && (((_pc)->phy_rev & 0xe) == 0x0))

#define PHY_IS_BCM54680E(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM54680E_OUI, \
                                 PHY_BCM54680E_MODEL) \
                                 && (((_pc)->phy_rev & 0x8) == 0x8))

#define PHY_IS_BCM54680E_A0(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM54680E_OUI, \
                                 PHY_BCM54680E_MODEL) \
                                 && ((_pc)->phy_rev == 0x8))

#define PHY_IS_BCM54680E_A1(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM54680E_OUI, \
                                 PHY_BCM54680E_MODEL) \
                                 && ((_pc)->phy_rev == 0x9))

#define PHY_IS_BCM54680E_A0A1(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM54680E_OUI, \
                                 PHY_BCM54680E_MODEL) \
                                 && (((_pc)->phy_rev & 0xe) == 0x8))

#define PHY_IS_BCM52681E(_pc)   (PHY_MODEL_CHECK((_pc), \
                                 PHY_BCM52681E_OUI, \
                                 PHY_BCM52681E_MODEL) \
                                 && (((_pc)->phy_rev & 0x8) == 0x8))

#ifndef DISABLE_CLK125
#define DISABLE_CLK125 0
#endif

#ifndef AUTO_MDIX_WHEN_AN_DIS
#define AUTO_MDIX_WHEN_AN_DIS 0
#endif

#define DISABLE_TEST_PORT 1
/* #define A0_AUTO_FILL_PORT_INDEX 1 */

STATIC int
phy_54680_speed_set(int unit, soc_port_t port, int speed);
STATIC int
phy_54680_duplex_set(int unit, soc_port_t port, int duplex);
STATIC int
phy_54680_master_set(int unit, soc_port_t port, int master);
STATIC int
phy_54680_ability_advert_set(int unit, soc_port_t port, soc_port_ability_t *ability);
STATIC int
phy_54680_autoneg_set(int unit, soc_port_t port, int autoneg);
STATIC int
phy_54680_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode);
STATIC int
phy_54680_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability);

STATIC int
phy_54680_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value);

#if defined(BCM_53125)
STATIC int
_phy_53125_eee_war(int unit, soc_port_t port);
#endif /* BCM_53125 */


/*
 * Function:
 *      _phy_54680e_cl45_reg_read
 * Purpose:
 *      Read Clause 45 Register using Clause 22 register access
 * Parameters:
 *      unit      - StrataSwitch unit #.
 *      pc        - Phy control
 *      flags     - Flags
 *      dev_addr  - Clause 45 Device
 *      reg_addr  - Register Address to read
 *      val       - Value Read
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_cl45_reg_read(int unit, phy_ctrl_t *pc, uint32 flags, 
                         uint8 dev_addr, uint16 reg_addr, uint16 *val)
{

    /* Write Device Address to register 0x0D (Set Function field to Address)*/
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0D, (dev_addr & 0x001f)));

    /* Select the register by writing to register address to register 0x0E */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0E, reg_addr));

    /* Write Device Address to register 0x0D (Set Function field to Data)*/
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0D, 
                           ((0x4000) | (dev_addr & 0x001f))));

    /* Read register 0x0E to get the value */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_READ(unit, pc, flags, 0x00, 0x0E, val));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680e_cl45_reg_write
 * Purpose:
 *      Write Clause 45 Register content using Clause 22 register access
 * Parameters:
 *      unit      - StrataSwitch unit #.
 *      pc        - Phy control
 *      flags     - Flags
 *      dev_addr  - Clause 45 Device
 *      reg_addr  - Register Address to read
 *      val       - Value to be written
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_cl45_reg_write(int unit, phy_ctrl_t *pc, uint32 flags, 
                         uint8 dev_addr, uint16 reg_addr, uint16 val)
{

    /* Write Device Address to register 0x0D (Set Function field to Address)*/
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0D, (dev_addr & 0x001f)));

    /* Select the register by writing to register address to register 0x0E */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0E, reg_addr));

    /* Write Device Address to register 0x0D (Set Function field to Data)*/
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0D, 
                           ((0x4000) | (dev_addr & 0x001f))));

    /* Write register 0x0E to write the value */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, 0x00, 0x0E, val));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680e_cl45_reg_modify
 * Purpose:
 *      Modify Clause 45 Register contents using Clause 22 register access
 * Parameters:
 *      unit      - StrataSwitch unit #.
 *      pc        - Phy control
 *      flags     - Flags
 *      reg_bank  - Register bank
 *      dev_addr  - Clause 45 Device
 *      reg_addr  - Register Address to read
 *      val       - Value 
 *      mask      - Mask for modifying the contents of the register
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_cl45_reg_modify(int unit, phy_ctrl_t *pc, uint32 flags, 
                          uint8 dev_addr, uint16 reg_addr, uint16 val, 
                          uint16 mask)
{

    uint16 value = 0;

    SOC_IF_ERROR_RETURN
        (PHY54680E_CL45_REG_READ(unit, pc, flags, dev_addr, reg_addr, &value));

    value = (val & mask) | (value & ~mask);

    SOC_IF_ERROR_RETURN
        (PHY54680E_CL45_REG_WRITE(unit, pc, flags, dev_addr, reg_addr, value));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680e_blk_top_lvl_reg_read
 * Purpose:
 *      New mechanism to Read Top Level Registers for EEE and 1588.
 * Parameters:
 *      unit       - StrataSwitch unit #.
 *      pc         - Phy control
 *      flags      - Flags
 *      reg_offset - Register Address to read
 *      val        - Value Read
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_blk_top_lvl_reg_read(int unit, phy_ctrl_t *pc, uint32 flags, 
                         uint16 reg_offset, uint16 *val)
{

    /* Write register 0x17 with the top level reg offset to read (handled in PHY54680_REG_READ) */
    /* Read register 0x15 to get the value */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_READ(unit, pc, flags, (0x0D00|(reg_offset & 0xff)), 0x15, val));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680e_blk_top_lvl_reg_write
 * Purpose:
 *      New mechanism to Write Top Level Registers for EEE and 1588
 * Parameters:
 *      unit      - StrataSwitch unit #.
 *      pc        - Phy control
 *      flags     - Flags
 *      reg_offset  - Register Address to read
 *      val       - Value to be written
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_blk_top_lvl_reg_write(int unit, phy_ctrl_t *pc, uint32 flags, 
                                  uint16 reg_offset, uint16 val)
{

    /* Write register 0x17 with the top level reg offset to write (handled in PHY54680_REG_WRITE) */
    /* Write register 0x15 the value to write to the top level register offset */
    SOC_IF_ERROR_RETURN
        (PHY54680_REG_WRITE(unit, pc, flags, (0x0D00|(reg_offset & 0xff)), 0x15, val));

    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680e_blk_top_lvl_reg_modify
 * Purpose:
 *      Modify Clause 45 Register contents using Clause 22 register access
 * Parameters:
 *      unit      - StrataSwitch unit #.
 *      pc        - Phy control
 *      flags     - Flags
 *      reg_bank  - Register bank
 *      dev_addr  - Clause 45 Device
 *      reg_addr  - Register Address to read
 *      val       - Value 
 *      mask      - Mask for modifying the contents of the register
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680e_blk_top_lvl_reg_modify(int unit, phy_ctrl_t *pc, uint32 flags,
                          uint16 reg_offset, uint16 val, uint16 mask)
{

    uint16 value = 0;

    /* Write register 0x17 with the top level reg offset to read */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc, flags, 
                                          reg_offset, &value));
    value = (val & mask) | (value & ~mask);

    /* Write register 0x17 with the top level reg offset to read */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc, flags, 
                                          reg_offset, value));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_medium_status
 * Purpose:
 *      Indicate the current active medium
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      medium - (OUT) One of:
 *              SOC_PORT_MEDIUM_COPPER
 *              SOC_PORT_MEDIUM_FIBER
 * Returns:
 *      SOC_E_NONE.
 */
STATIC int
phy_54680_medium_status(int unit, soc_port_t port, soc_port_medium_t *medium)
{

    *medium = SOC_PORT_MEDIUM_COPPER;

    return SOC_E_NONE;

}

/*
 * Function:
 *      phy_54680_medium_config_set
 * Purpose:
 *      Set the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - Operating parameters
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_medium_config_set(int unit, soc_port_t port, 
                           soc_port_medium_t  medium,
                           soc_phy_config_t  *cfg)
{
    COMPILER_REFERENCE(medium);

    SOC_IF_ERROR_RETURN
        (phy_54680_speed_set(unit, port, cfg->force_speed));
    SOC_IF_ERROR_RETURN
        (phy_54680_duplex_set(unit, port, cfg->force_duplex));
    SOC_IF_ERROR_RETURN
        (phy_54680_master_set(unit, port, cfg->master));
    SOC_IF_ERROR_RETURN
        (phy_54680_ability_advert_set(unit, port, &cfg->advert_ability));
    SOC_IF_ERROR_RETURN
        (phy_54680_autoneg_set(unit, port, cfg->autoneg_enable));
    SOC_IF_ERROR_RETURN
        (phy_54680_mdix_set(unit, port, cfg->mdix));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_medium_config_get
 * Purpose:
 *      Get the operating parameters that are automatically selected
 *      when medium switches type.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      port - Port number
 *      medium - SOC_PORT_MEDIUM_COPPER/FIBER
 *      cfg - (OUT) Operating parameters
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_medium_config_get(int unit, soc_port_t port, 
                           soc_port_medium_t medium,
                           soc_phy_config_t *cfg)
{
    phy_ctrl_t    *pc;

    COMPILER_REFERENCE(medium);

    pc = EXT_PHY_SW_STATE(unit, port);

    sal_memcpy(cfg, &pc->copper, sizeof (*cfg));

    return SOC_E_NONE;
}


#if defined(BCM_53125)
STATIC int
_phy_53125_eee_war(int unit, soc_port_t port)
{
    phy_ctrl_t        *pc;
    uint32 reg_val = 0;
    uint32  eee_phy_val = 0;
    uint16 phy_reg = 0;
    uint32  mac_low_power = 0;

    pc     = EXT_PHY_SW_STATE(unit, port);

    /* 1. Disable MAC EEE feature at StarFighter level */
    SOC_IF_ERROR_RETURN(
        REG_READ_EEE_EN_CTRLr(unit, &reg_val));
    reg_val &= ~(0x1 << port);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_EEE_EN_CTRLr(unit, &reg_val));
    SOC_IF_ERROR_RETURN(
        REG_READ_EEE_PHY_CTRLr(unit, &eee_phy_val));
    reg_val = 0;
    SOC_IF_ERROR_RETURN(
        REG_WRITE_EEE_PHY_CTRLr(unit, &reg_val));

    /* 2. Configure the GPHY registers */ 
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x18, 0x0c00));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x0ffe));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x0100));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x2022));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x01f0));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x4021));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x0887));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x2021));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x8983));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x0fff));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x4000));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x18, 0x0400));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0xd, 0x0007));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0xe, 0x003c));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0xd, 0x4007));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0xe, 0x0006));
    SOC_IF_ERROR_RETURN(
        READ_PHY_REG(unit, pc, 0xe, &phy_reg));

    /* 3. Enable MAC EEE feature at StarFighter level */
    SOC_IF_ERROR_RETURN(
        DRV_DEV_PROP_GET(unit,
        DRV_DEV_PROP_LOW_POWER_ENABLE, &mac_low_power));
    if (!mac_low_power) {
        /* Enable EEE if MAC LOW POWER is not enabled. */
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_EN_CTRLr(unit, &reg_val));
        reg_val |= (0x1 << port);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_EN_CTRLr(unit, &reg_val));
    }
    SOC_IF_ERROR_RETURN(
        REG_WRITE_EEE_PHY_CTRLr(unit, &eee_phy_val));

    /* 4. re-start auto-neg */
    SOC_IF_ERROR_RETURN(
        READ_PHY_REG(unit, pc, MII_CTRL_REG, &phy_reg));
    phy_reg |=  MII_CTRL_RAN;
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, MII_CTRL_REG, phy_reg));

    soc_cm_debug(DK_PORT,"_phy_53125_eee_war port %d\n",port);
    return SOC_E_NONE;
}
#endif /* BCM_53125 */

/*
 * Function:
 *      _phy_54680_reset_setup
 * Purpose:
 *      Function to reset the PHY and set up initial operating registers.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
_phy_54680_reset_setup(int unit, soc_port_t port)
{
    phy_ctrl_t        *int_pc;
    phy_ctrl_t        *pc;
    uint16             tmp;
    uint8              phy_addr;
    uint16             id0, id1;
    int                index;

    SOC_IF_ERROR_RETURN(phy_ge_init(unit, port));

    pc     = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    phy_addr = pc->phy_id;
    index = phy_addr & 7;

    /* set the default values that are valid for many boards */
    SOC_IF_ERROR_RETURN
        (phy_54680_control_set( unit, port, SOC_PHY_CONTROL_PORT_PRIMARY, 
        (port - index) < 0 ? 0 : (port - index)));
    SOC_IF_ERROR_RETURN
        (phy_54680_control_set( unit, port, SOC_PHY_CONTROL_PORT_OFFSET, index ));

#ifdef DISABLE_TEST_PORT
    if (PHY_IS_BCM54680_A0(pc))  {
        /* Disable test port */

        phy_addr = pc->phy_id + 1;

        SOC_IF_ERROR_RETURN
            (pc->read(unit, phy_addr, MII_PHY_ID0_REG, &id0));

        SOC_IF_ERROR_RETURN
            (pc->read(unit, phy_addr, MII_PHY_ID1_REG, &id1));

        if ( ( id0 != PHY_BCM54680_A0_ID0 ) || ( id1 != PHY_BCM54680_A0_ID1 ) ) {


            SOC_IF_ERROR_RETURN
                (pc->write(unit, phy_addr, 0x1f, 0xffd0));

            SOC_IF_ERROR_RETURN
                (pc->write(unit, phy_addr, 0x1e, 0x001f));

            SOC_IF_ERROR_RETURN
                (pc->write(unit, phy_addr, 0x1f, 0x8000));

            SOC_IF_ERROR_RETURN
                (pc->write(unit, phy_addr, 0x1d, 0x4002));

            SOC_IF_ERROR_RETURN
                (pc->write(unit, phy_addr, 0x00, MII_CTRL_RESET));

#ifdef A0_AUTO_FILL_PORT_INDEX
            {
                int i;

                for (i = 0; i < 8; i++) {
                    SOC_IF_ERROR_RETURN
                        (phy_54680_control_set( unit, port - i, SOC_PHY_CONTROL_PORT_PRIMARY, port - 7 ));
                    SOC_IF_ERROR_RETURN
                        (phy_54680_control_set( unit, port - i, SOC_PHY_CONTROL_PORT_OFFSET, 7 - i ));
                }
            }
#endif
        }
    }
#endif

    /* remove power down */
    if (pc->copper.enable) {
        tmp = PHY_DISABLED_MODE(unit, port) ? MII_CTRL_PD : 0;
    } else {
        tmp = MII_CTRL_PD;
    }
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_MII_CTRLr(unit, pc, tmp, MII_CTRL_PD));

    if (NULL != int_pc) {
        SOC_IF_ERROR_RETURN
            (PHY_INIT(int_pc->pd, unit, port));
    } 

    if (PHY_IS_BCM54880E(pc)) {
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 0x4f, (1U << 12), (1U << 12)));
    }

#if DISABLE_CLK125
    /* Reduce EMI emissions by disabling the CLK125 pin if not used */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_SPARE_CTRL_3r(unit, pc, 0, 1));
#endif


    /* Configure Extended Control Register */
    /* Enable LEDs to indicate traffic status */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_MII_ECRr(unit, pc, 0x0020, 0x0020));

#if AUTO_MDIX_WHEN_AN_DIS
    /* Enable Auto-MDIX When autoneg disabled */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200));
#endif

    /* Enable extended packet length (4.5k through 25k) */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_MII_AUX_CTRLr(unit, pc, 0x4000, 0x4000));

    /* Configure LED selectors */
    tmp = ((pc->ledmode[1] & 0xf) << 4) | (pc->ledmode[0] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY54680_LED_SELECTOR_1r(unit, pc, tmp));

    tmp = ((pc->ledmode[3] & 0xf) << 4) | (pc->ledmode[2] & 0xf);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY54680_LED_SELECTOR_2r(unit, pc, tmp));

    tmp = (pc->ledctrl & 0x3ff);
    SOC_IF_ERROR_RETURN
        (WRITE_PHY54680_LED_CTRLr(unit, pc, tmp));

    SOC_IF_ERROR_RETURN
        (WRITE_PHY54680_EXP_LED_SELECTORr(unit, pc, pc->ledselect));

    /*
     * Configure Auxiliary control register to turn off
     * carrier extension.  The Intel 7131 NIC does not accept carrier
     * extension and gets CRC errors.
     */
    /* Disable carrier extension */
    SOC_IF_ERROR_RETURN
        (MODIFY_PHY54680_AUX_CTRLr(unit, pc, 0x0040, 0x0040));

    /* BCM54880E is EEE capable */
    if (PHY_IS_BCM54680E(pc) || PHY_IS_BCM54880E(pc)) {
        if (pc->phy_rev == 0x1) { /* rev A1 */
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0c00));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0ffe));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0100));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2022));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x01f0));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x4021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0887));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x8983));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0400));
        }
        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0000, 0x0003));
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_EEE_CAPABLE);
    }

    /* BCM52681E is EEE capable */
    if (PHY_IS_BCM52681E(pc)) {
        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0000, 0x0003));
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_EEE_CAPABLE);
    }

    if (PHY_IS_BCM54680(pc)) {
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x0c00));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x17, 0x000e));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x15, 0x0752));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x17, 0x000f));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x15, 0xe04e));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x0400));
    }

#if defined(BCM_53314)
    if (PHY_IS_BCM53324(pc)) {
        /* BCM53324 internal GPHY is EEE capable */
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_EEE_CAPABLE);
    }
#endif /* BCM_53314 */    

#if defined(BCM_53125)
    /* BCM53125 internal GPHY is EEE capable */
    if (PHY_IS_BCM53125(pc)) {
        if (pc->phy_rev == 0x0) { /* rev A0 */
            /* Apply the EEE workaround */
            SOC_IF_ERROR_RETURN(
                _phy_53125_eee_war(unit, port));
        }
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_EEE_CAPABLE);
        
    }
#endif /* BCM_53125 */    

#if defined(BCM_53128)
    /* BCM53125 internal GPHY is EEE capable */
    if (PHY_IS_BCM53128(pc)) {
         PHY_FLAGS_SET(unit, port, PHY_FLAGS_EEE_CAPABLE);
    }
#endif /* BCM_53128 */    

    /* 100BASE-TX initialization change for EEE and autogreen mode */
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY_REG(unit, pc, 0x18, 0x0800, 0x0800));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x17, 0x4022));
    SOC_IF_ERROR_RETURN(
        WRITE_PHY_REG(unit, pc, 0x15, 0x017b));
    SOC_IF_ERROR_RETURN(
        MODIFY_PHY_REG(unit, pc, 0x18, 0x0000, 0x0800));

    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_54680_init
 * Purpose:
 *      Init function for 54680 PHY.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_init(int unit, soc_port_t port)
{
    phy_ctrl_t         *pc;

    pc     = EXT_PHY_SW_STATE(unit, port);

    pc->automedium = FALSE;
    pc->fiber_detect = FALSE;
    pc->fiber.enable = FALSE;

    pc->copper.enable = TRUE;
    pc->copper.preferred = TRUE;
    pc->copper.autoneg_enable = TRUE;
    pc->copper.force_speed = 1000;
    pc->copper.force_duplex = TRUE;
    pc->copper.master = SOC_PORT_MS_AUTO;
    pc->copper.mdix = SOC_PORT_MDIX_AUTO;


    /* Initially configure for the preferred medium. */

    PHY_FLAGS_SET(unit, port, PHY_FLAGS_COPPER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_PASSTHRU);

    /* Get Requested LED selectors (defaults are hardware defaults) */
    pc->ledmode[0] = soc_property_port_get(unit, port, spn_PHY_LED1_MODE, 0);
    pc->ledmode[1] = soc_property_port_get(unit, port, spn_PHY_LED2_MODE, 1);
    pc->ledmode[2] = soc_property_port_get(unit, port, spn_PHY_LED3_MODE, 3);
    pc->ledmode[3] = soc_property_port_get(unit, port, spn_PHY_LED4_MODE, 6);
    pc->ledctrl    = soc_property_port_get(unit, port, spn_PHY_LED_CTRL, 0x8);
    pc->ledselect  = soc_property_port_get(unit, port, spn_PHY_LED_SELECT, 0);

    SOC_IF_ERROR_RETURN
        (_phy_54680_reset_setup(unit, port));

    /* Advertise all possible by default */
    SOC_IF_ERROR_RETURN
        (phy_54680_ability_local_get(unit, port, &pc->copper.advert_ability));

    SOC_IF_ERROR_RETURN
        (phy_54680_medium_config_set(unit, port, 0, &pc->copper)); 

    return SOC_E_NONE;

}

/*
 * Function:
 *      phy_54680_enable_set
 * Purpose:
 *      Enable or disable the physical interface.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - Boolean, true = enable PHY, false = disable.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_enable_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t    *pc;
    uint16 power_down;

    pc = EXT_PHY_SW_STATE(unit, port);

    power_down = (enable) ? 0 : MII_CTRL_PD;

    if (pc->copper.enable) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_CTRLr(unit, pc, power_down, MII_CTRL_PD));
    }

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_enable_set(unit, port, enable));

    return SOC_E_NONE;

}

/*
 * Function:
 *      phy_54680_enable_get
 * Purpose:
 *      Enable or disable the physical interface for a 54680 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      enable - (OUT) Boolean, true = enable PHY, false = disable.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_enable_get(int unit, soc_port_t port, int *enable)
{
    SOC_IF_ERROR_RETURN
        (phy_fe_ge_enable_get(unit, port, enable));

    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_54680_link_get
 * Purpose:
 *      Determine the current link up/down status for a 54680 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      link - (OUT) Boolean, true indicates link established.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      If using automedium, also switches the mode.
 */

STATIC int
phy_54680_link_get(int unit, soc_port_t port, int *link)
{

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_link_get(unit, port, link));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_duplex_set
 * Purpose:
 *      Set the current duplex mode (forced).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - Boolean, true indicates full duplex, false indicates half.
 * Returns:
 *      SOC_E_XXX
 *      SOC_E_UNAVAIL - Half duplex requested, and not supported.
 * Notes:
 *      The duplex is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 *      Autonegotiation is not manipulated.
 */

STATIC int
phy_54680_duplex_set(int unit, soc_port_t port, int duplex)
{
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_duplex_set(unit, port, duplex));

    pc->copper.force_duplex = duplex;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_duplex_get
 * Purpose:
 *      Get the current operating duplex mode. If autoneg is enabled,
 *      then operating mode is returned, otherwise forced mode is returned.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - (OUT) Boolean, true indicates full duplex, false
 *              indicates half.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The duplex is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level. Autonegotiation is
 *      not manipulated.
 */

STATIC int
phy_54680_duplex_get(int unit, soc_port_t port, int *duplex)
{
    SOC_IF_ERROR_RETURN
        (phy_fe_ge_duplex_get(unit, port, duplex));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_speed_set
 * Purpose:
 *      Set the current operating speed (forced).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      speed - Requested speed.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The speed is set only for the ACTIVE medium.
 */

STATIC int
phy_54680_speed_set(int unit, soc_port_t port, int speed)
{
    SOC_IF_ERROR_RETURN
        (phy_fe_ge_speed_set(unit, port, speed));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_speed_get
 * Purpose:
 *      Get the current operating speed for a 54680 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      duplex - (OUT) Boolean, true indicates full duplex, false
 *              indicates half.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The speed is retrieved for the ACTIVE medium.
 */

STATIC int
phy_54680_speed_get(int unit, soc_port_t port, int *speed)
{
    SOC_IF_ERROR_RETURN
        (phy_fe_ge_speed_get(unit, port, speed));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_master_set
 * Purpose:
 *      Set the current master mode
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      master - SOC_PORT_MS_*
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The master mode is set only for the ACTIVE medium.
 */

STATIC int
phy_54680_master_set(int unit, soc_port_t port, int master)
{
    phy_ctrl_t    *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_master_set(unit, port, master));

    pc->copper.master = master;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_master_get
 * Purpose:
 *      Get the current master mode for a 54680 device.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      master - (OUT) SOC_PORT_MS_*
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The master mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_54680_master_get(int unit, soc_port_t port, int *master)
{
    SOC_IF_ERROR_RETURN
        (phy_fe_ge_master_get(unit, port, master));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_autoneg_set
 * Purpose:
 *      Enable or disable auto-negotiation on the specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      autoneg - Boolean, if true, auto-negotiation is enabled
 *              (and/or restarted). If false, autonegotiation is disabled.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is set only for the ACTIVE medium.
 */

STATIC int
phy_54680_autoneg_set(int unit, soc_port_t port, int autoneg)
{
    int           rv;
    phy_ctrl_t    *pc;

    pc             = EXT_PHY_SW_STATE(unit, port);
    rv             = SOC_E_NONE;

    /* Set auto-neg on PHY */
    rv = phy_fe_ge_an_set(unit, port, autoneg);
    if (SOC_SUCCESS(rv)) {
        pc->copper.autoneg_enable = autoneg ? 1 : 0;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_autoneg_get
 * Purpose:
 *      Get the current auto-negotiation status (enabled/busy).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      autoneg - (OUT) if true, auto-negotiation is enabled.
 *      autoneg_done - (OUT) if true, auto-negotiation is complete. This
 *              value is undefined if autoneg == FALSE.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The autoneg mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_54680_autoneg_get(int unit, soc_port_t port,
                     int *autoneg, int *autoneg_done)
{

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_an_get(unit, port, autoneg, autoneg_done));

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_ability_advert_set
 * Purpose:
 *      Set the current advertisement for auto-negotiation.
 * Parameters:
 *      unit    - StrataSwitch unit #.
 *      port    - StrataSwitch port #.
 *      ability - Port ability indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_ability_advert_set(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;
    uint16     eee_ability = 0;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_ability_advert_set(unit, port, ability));

    /* EEE settings */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        eee_ability = 0;
        if (ability->eee & SOC_PA_EEE_1GB_BASET) {
            eee_ability |= 0x4;
        }
        if (ability->eee & SOC_PA_EEE_100MB_BASETX) {
            eee_ability |= 0x2;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EEE_ADVr(unit, pc, eee_ability, 0x0006));
    }

    return SOC_E_NONE;

}

/*
 * Function:
 *      phy_54680_ability_advert_get
 * Purpose:
 *      Get the current advertisement for auto-negotiation.
 * Parameters:
 *      unit    - StrataSwitch unit #.
 *      port    - StrataSwitch port #.
 *      ability - Port ability indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_54680_ability_advert_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;
    uint16      eee_ability = 0;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_ability_advert_get(unit, port, ability));

    /* EEE settings */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EEE_ADVr(unit, pc, &eee_ability));
        if (eee_ability & 0x04) {
            ability->eee |= SOC_PA_EEE_1GB_BASET;
        }
        if (eee_ability & 0x02) {
            ability->eee |= SOC_PA_EEE_100MB_BASETX;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_ability_remote_get
 * Purpose:
 *      Get partners current advertisement for auto-negotiation.
 * Parameters:
 *      unit    - StrataSwitch unit #.
 *      port    - StrataSwitch port #.
 *      ability - Port ability indicating supported options/speeds.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The remote advertisement is retrieved for the ACTIVE medium.
 *      No synchronization performed at this level. If Autonegotiation is
 *      disabled or in progress, this routine will return an error.
 */

STATIC int
phy_54680_ability_remote_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t       *pc;
    uint16            eee_resolution;

    if (NULL == ability) {
        return (SOC_E_PARAM);
    }

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (phy_fe_ge_ability_remote_get(unit, port, ability));

    /* EEE settings */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EEE_RESOLUTION_STATr(unit, pc, &eee_resolution));
        if (eee_resolution & 0x04) {
            ability->eee |= SOC_PA_EEE_1GB_BASET;
        }
        if (eee_resolution & 0x02) {
            ability->eee |= SOC_PA_EEE_100MB_BASETX;
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_ability_local_get
 * Purpose:
 *      Get the device's complete abilities.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ability - return device's abilities.
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);


    if (NULL == ability) {
        return SOC_E_PARAM;
    }

    ability->speed_half_duplex  = SOC_PA_SPEED_100MB | SOC_PA_SPEED_10MB;
    ability->speed_full_duplex  = SOC_PA_SPEED_1000MB |SOC_PA_SPEED_100MB | 
                                  SOC_PA_SPEED_10MB;

                                  
    if (PHY_IS_BCM54880E_A0(pc)) { /* 880E A0 has a bug, 10Mbs doesn't work */
        ability->speed_half_duplex  = SOC_PA_SPEED_100MB;
        ability->speed_full_duplex  = SOC_PA_SPEED_1000MB |SOC_PA_SPEED_100MB;
    }

    if (PHY_IS_BCM52681E(pc)) {
        ability->speed_half_duplex  = SOC_PA_SPEED_100MB |SOC_PA_SPEED_10MB;
        ability->speed_full_duplex  = SOC_PA_SPEED_100MB |SOC_PA_SPEED_10MB;
    }

    ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
    ability->interface = SOC_PA_INTF_SGMII;
    if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || PHY_IS_BCM53324(pc)) {
    	/* No SGMII interface */
        ability->interface = SOC_PA_INTF_GMII;
    }
    ability->medium    = SOC_PA_MEDIUM_COPPER; 
    ability->loopback  = SOC_PA_LB_PHY;
    ability->flags     = SOC_PA_AUTONEG;
    
    /* EEE settings */
    if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        if (PHY_IS_BCM52681E(pc)) {
            ability->eee = SOC_PA_EEE_100MB_BASETX;
        } else {
            ability->eee = SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX;
        }
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *      phy_54680_lb_set
 * Purpose:
 *      Set the local PHY loopback mode.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      loopback - Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is set only for the ACTIVE medium.
 *      No synchronization performed at this level.
 */

STATIC int
phy_54680_lb_set(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || PHY_IS_BCM53324(pc)) {
        /* No SGMII interface */
        SOC_IF_ERROR_RETURN(phy_fe_ge_lb_set(unit, port, enable));

        /* 
         * Force link state machine into pass state.
         * Thus the PHY is forced to show link up without the link partner.
         * This is WA for no link-up issue about forcing 10/100 Mbps 
         * in PHY loopback. 
         */
        if(PHY_IS_BCM53324(pc)) {
            if(enable) {
                /* Force LSM to pass state */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY54680_TEST1r(unit, pc, (1U << 12), (1U << 12)));
            } else {
                /* Normal LSM operation */
                SOC_IF_ERROR_RETURN
                    (MODIFY_PHY54680_TEST1r(unit, pc, 0, (1U << 12)));
            }
        }    
        return SOC_E_NONE;
    }

    if (enable) {
        /* SGMII loopback with RX/TX on MDI suppressed */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY54680_EXP_SGMII_LINESIDE_LOOPBACK_CTRLr(unit, pc, (1U << 2) | (1U << 1) | (1U << 0)));
        /* Force LSM to pass state */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_TEST1r(unit, pc, (1U << 12), (1U << 12)));
        /* Disable TX */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_ECRr(unit, pc, (1U << 13), (1U << 13)));
    } else {
        /* no SGMII loopback */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY54680_EXP_SGMII_LINESIDE_LOOPBACK_CTRLr(unit, pc, 0x0));
        /* Normal LSM operation */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_TEST1r(unit, pc, 0, (1U << 12)));
        /* Enable TX */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_ECRr(unit, pc, 0, (1U << 13)));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_lb_get
 * Purpose:
 *      Get the local PHY loopback mode.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      loopback - (OUT) Boolean: true = enable loopback, false = disable.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The loopback mode is retrieved for the ACTIVE medium.
 */

STATIC int
phy_54680_lb_get(int unit, soc_port_t port, int *enable)
{
    phy_ctrl_t *pc;
    uint16 temp;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || PHY_IS_BCM53324(pc)) {
        /* No SGMII interface */
        SOC_IF_ERROR_RETURN(phy_fe_ge_lb_get(unit, port, enable));
        return SOC_E_NONE;
    }

    SOC_IF_ERROR_RETURN
        (READ_PHY54680_EXP_SGMII_LINESIDE_LOOPBACK_CTRLr(unit, pc, &temp));
    *enable = (temp & (1U<<2)) ? TRUE : FALSE;

    return SOC_E_NONE;
}

/*
 * Function:
 *      phy_54680_interface_set
 * Purpose:
 *      Set the current operating mode of the internal PHY.
 *      (Pertaining to the MAC/PHY interface, not the line interface).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      pif - one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(pif);

    return SOC_E_NONE;

}

/*
 * Function:
 *      phy_54680_interface_get
 * Purpose:
 *      Get the current operating mode of the internal PHY.
 *      (Pertaining to the MAC/PHY interface, not the line interface).
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      pif - (OUT) one of SOC_PORT_IF_*
 * Returns:
 *      SOC_E_XXX
 */

STATIC int
phy_54680_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    phy_ctrl_t *pc;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc)) {
        /* No SGMII interface */
        *pif = pc->interface;
        return (SOC_E_NONE);
    }
#if defined(BCM_53314)
    if(PHY_IS_BCM53324(pc)) {
        *pif = SOC_PORT_IF_GMII;
    }
#endif /* BCM_53314 */
    {
        *pif = SOC_PORT_IF_SGMII;
    }

    return(SOC_E_NONE);
}

/*
 * Function:
 *      phy_54680_mdix_set
 * Description:
 *      Set the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - One of:
 *              SOC_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              SOC_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              SOC_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              SOC_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{
    phy_ctrl_t    *pc;
    int            speed;

    pc = EXT_PHY_SW_STATE(unit, port);

    switch (mode) {
    case SOC_PORT_MDIX_AUTO:
        /* Clear bit 14 for automatic MDI crossover */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_ECRr(unit, pc, 0, 0x4000));

        /*
         * Write the result in the register 0x18, shadow copy 7
         */
        /* Clear bit 9 to disable forced auto MDI xover */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_MISC_CTRLr(unit, pc, 0, 0x0200));
        break;

    case SOC_PORT_MDIX_FORCE_AUTO:
        /* Clear bit 14 for automatic MDI crossover */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_ECRr(unit, pc, 0, 0x4000));

        /*
         * Write the result in the register 0x18, shadow copy 7
         */
        /* Set bit 9 to force automatic MDI crossover */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_MISC_CTRLr(unit, pc, 0x0200, 0x0200));
        break;

    case SOC_PORT_MDIX_NORMAL:
        SOC_IF_ERROR_RETURN(phy_54680_speed_get(unit, port, &speed));
        if (speed == 0 || speed == 10 || speed == 100) {
            /* Set bit 14 for manual MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_MII_ECRr(unit, pc, 0x4000, 0x4000));

            SOC_IF_ERROR_RETURN
                (WRITE_PHY54680_TEST1r(unit, pc, 0));
        } else {
            return SOC_E_UNAVAIL;
        }
        break;

    case SOC_PORT_MDIX_XOVER:
        SOC_IF_ERROR_RETURN(phy_54680_speed_get(unit, port, &speed));
        if (speed == 0 || speed == 10 || speed == 100) {
             /* Set bit 14 for manual MDI crossover */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_MII_ECRr(unit, pc, 0x4000, 0x4000));

            SOC_IF_ERROR_RETURN
                (WRITE_PHY54680_TEST1r(unit, pc, 0x0080));
        } else {
            return SOC_E_UNAVAIL;
        }
        break;

    default:
        return SOC_E_PARAM;
        break;
    }

    pc->copper.mdix = mode;
    return SOC_E_NONE;

}        

/*
 * Function:
 *      phy_54680_mdix_get
 * Description:
 *      Get the Auto-MDIX mode of a port/PHY
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (Out) One of:
 *              SOC_PORT_MDIX_AUTO
 *                      Enable auto-MDIX when autonegotiation is enabled
 *              SOC_PORT_MDIX_FORCE_AUTO
 *                      Enable auto-MDIX always
 *              SOC_PORT_MDIX_NORMAL
 *                      Disable auto-MDIX
 *              SOC_PORT_MDIX_XOVER
 *                      Disable auto-MDIX, and swap cable pairs
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mode)
{
    phy_ctrl_t    *pc;
    int            speed;

    pc = EXT_PHY_SW_STATE(unit, port);


    SOC_IF_ERROR_RETURN(phy_54680_speed_get(unit, port, &speed));
    if (speed == 1000) {
       *mode = SOC_PORT_MDIX_AUTO;
    } else {
        *mode = pc->copper.mdix;
    }

    return SOC_E_NONE;
}    

/*
 * Function:
 *      phy_54680_mdix_status_get
 * Description:
 *      Get the current MDIX status on a port/PHY
 * Parameters:
 *      unit    - Device number
 *      port    - Port number
 *      status  - (OUT) One of:
 *              SOC_PORT_MDIX_STATUS_NORMAL
 *                      Straight connection
 *              SOC_PORT_MDIX_STATUS_XOVER
 *                      Crossover has been performed
 * Return Value:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_mdix_status_get(int unit, soc_port_t port, 
                         soc_port_mdix_status_t *status)
{
    phy_ctrl_t    *pc;
    uint16               tmp;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_PHY54680_MII_ESRr(unit, pc, &tmp));
    if (tmp & 0x2000) {
        *status = SOC_PORT_MDIX_STATUS_XOVER;
    } else {
        *status = SOC_PORT_MDIX_STATUS_NORMAL;
    }

    return SOC_E_NONE;
}    

STATIC int
_phy_54680_power_mode_set (int unit, soc_port_t port, int mode)
{
    phy_ctrl_t    *pc;

    pc       = EXT_PHY_SW_STATE(unit, port);

    if (pc->power_mode == mode) {
        return SOC_E_NONE;
    }

    if (mode == SOC_PHY_CONTROL_POWER_LOW) {
        /* enable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_AUX_CTRLr(unit,pc,0x0c00,0x0c00));

        /* enable low power 136 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,0x80,0x80));

        /* reduce tx bias current to -20% */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x17, 0x0f75));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x15, 0x1555));

        /* disable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_AUX_CTRLr(unit,pc,0x0400,0x0c00));
        pc->power_mode = mode;

    } else if (mode == SOC_PHY_CONTROL_POWER_FULL) {

        /* back to normal mode */
        /* enable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_AUX_CTRLr(unit,pc,0x0c00,0x0c00));

        /* disable low power 136 */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,0x00,0x80));

        /* set tx bias current to nominal */
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x17, 0x0f75));
        SOC_IF_ERROR_RETURN
            (WRITE_PHY_REG(unit, pc, 0x15, 0x0));

        /* disable dsp clock */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MII_AUX_CTRLr(unit,pc,0x0400,0x0c00));

        /* disable the auto power mode */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,
                        0,
                        PHY_54680_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    } else if (mode == SOC_PHY_CONTROL_POWER_AUTO) {
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,
                        PHY_54680_AUTO_PWRDWN_EN,
                        PHY_54680_AUTO_PWRDWN_EN));
        pc->power_mode = mode;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      _phy_54680_eee_enable
 * Purpose:
 *      Enable or disable EEE (Native)
 * Parameters:
 *      unit   - StrataSwitch unit #.
 *      port   - StrataSwitch port #.
 *      enable - Enable Native EEE
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_phy_54680_eee_enable(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;
    int rv = SOC_E_NONE;
    
    pc = EXT_PHY_SW_STATE(unit, port);
    if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        rv = SOC_E_FAIL;
    }
    if (enable == 1) {
        if (PHY_IS_BCM54680E_A0A1(pc) || PHY_IS_BCM54880E_A0A1(pc)) {
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0c00));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0ffe));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0100));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0fff));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x4000));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2022));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x01f1));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x4021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0887));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x8983));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x4600));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0e40));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0000));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0400));

            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Br(unit, pc, 0xc000, 0xc000));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Ar(unit, pc, 0xf300, 0xf300));
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_803Dr(unit, pc, 0xc000, 0xc000)); /* 7.803d */
        }
        if (PHY_IS_BCM52681E(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0002, 0x0006));
            pc->copper.advert_ability.eee |= SOC_PA_EEE_100MB_BASETX;

        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0006, 0x0006));
            pc->copper.advert_ability.eee |= (SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX);
        }
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x1, 0x1)); /* exp af */
    } else {
        if (PHY_IS_BCM54680E_A0A1(pc) || PHY_IS_BCM54880E_A0A1(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Br(unit, pc, 0x0000, 0xc000));
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Ar(unit, pc, 0x0000, 0xf300));
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_803Dr(unit, pc, 0x0000, 0xc000)); /* 7.803d */
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0000, 0x0006));
        pc->copper.advert_ability.eee &= ~(SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX);

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 0x1)); /* exp af */
    }

    return rv;
}

/*
 * Function:
 *      _phy_54680_eee_auto_enable
 * Purpose:
 *      Enable or disable EEE (Native)
 * Parameters:
 *      unit   - StrataSwitch unit #.
 *      port   - StrataSwitch port #.
 *      enable - Enable Native EEE
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_phy_54680_eee_auto_enable(int unit, soc_port_t port, int enable)
{
    phy_ctrl_t *pc;
    int rv = SOC_E_NONE;
    
    pc = EXT_PHY_SW_STATE(unit, port);
    if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
        rv = SOC_E_FAIL;
    }

    if (enable == 1) {

        if (PHY_IS_BCM54680E_A0A1(pc) || PHY_IS_BCM54880E_A0A1(pc)) {

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0c00));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x18, 0x0007));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0ffe));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0900));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0fff));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x4000));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2022));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x01f1));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x2021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x8983));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x4021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0887));

            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0021));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x4600));

            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Br(unit, pc, 0xc000, 0xc000));

            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Ar(unit, pc, 0x1300, 0x1300));

        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_803Dr(unit, pc, 0xc000, 0xc000)); /* 7.803d */
        }
        if (PHY_IS_BCM52681E(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0002, 0x0006));
            pc->copper.advert_ability.eee |= SOC_PA_EEE_100MB_BASETX;
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0006, 0x0006));
            pc->copper.advert_ability.eee |= (SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX);
        }

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MODE_CTRLr(unit, pc, 0x00, 0x06));

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_SGMII_SLAVEr(unit, pc, 0x02, 0x02));

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0F7F,0x15, 0x7, 0x7)); /* exp 7f */

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x1, 0x1)); /* exp af */

        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0001, 0x0003));

    } else {
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 0x3)); /* exp af */
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EEE_ADVr(unit, pc, 0x0000, 0x0006));
        pc->copper.advert_ability.eee &= ~(SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX);

        if (PHY_IS_BCM54680E_A0A1(pc) || PHY_IS_BCM54880E_A0A1(pc)) {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Br(unit, pc, 0x0000, 0xc000)); /* 7.8031 */
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_TEST_CTRL_Ar(unit, pc, 0x0000, 0xf300)); /* 7.8030 2 ops */
        } else {
            SOC_IF_ERROR_RETURN
                (MODIFY_PHY54680_EEE_803Dr(unit, pc, 0x0000, 0xc000)); /* 7.803d */
        }
    }

    return rv;
}

/*
* Function:
 *      phy_54680_control_set
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
phy_54680_control_set(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 value)
{
    int rv;
    phy_ctrl_t *pc;
    uint16 data;
    soc_port_t primary;
    int offset, saved_autoneg;

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch(type) {
    case SOC_PHY_CONTROL_POWER:
            rv = _phy_54680_power_mode_set(unit,port,value);
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        if (value <= PHY_54680_AUTO_PWRDWN_WAKEUP_MAX) {

            /* at least one unit */
            if (value < PHY_54680_AUTO_PWRDWN_WAKEUP_UNIT) {
                value = PHY_54680_AUTO_PWRDWN_WAKEUP_UNIT;
            }
        } else { /* anything more then max, set to the max */
            value = PHY_54680_AUTO_PWRDWN_WAKEUP_MAX;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,
                      value/PHY_54680_AUTO_PWRDWN_WAKEUP_UNIT,
                      PHY_54680_AUTO_PWRDWN_WAKEUP_MASK));
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        /* sleep time configuration is either 2.7s or 5.4 s, default is 2.7s */
        if (value < PHY_54680_AUTO_PWRDWN_SLEEP_MAX) {
            data = 0; /* anything less than 5.4s, default to 2.7s */
        } else {
            data = PHY_54680_AUTO_PWRDWN_SLEEP_MASK;
        }
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_AUTO_POWER_DOWNr(unit,pc,
                      data,
                      PHY_54680_AUTO_PWRDWN_SLEEP_MASK));
        break;

        case SOC_PHY_CONTROL_PORT_PRIMARY:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_primary_set(unit, port, (soc_port_t)value));
        break;

    case SOC_PHY_CONTROL_PORT_OFFSET:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_offset_set(unit, port, (int)value));
        break;

    case SOC_PHY_CONTROL_CLOCK_ENABLE:
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_primary_get(unit, port, &primary));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_offset_get(unit, port, &offset));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_toplvl_reg_read(unit, port, primary, 0x0, &data));
        if ( value ) {
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_toplvl_reg_write(unit, port, primary, 0x0, 
                     ( data & 0xf0 ) | ( offset & 0x7 )));
        } else {
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_toplvl_reg_write(unit, port, primary, 0x0, 
                     ( data & 0xf0 ) | 0x8 ));
        }
        break;

    case SOC_PHY_CONTROL_CLOCK_SECONDARY_ENABLE:
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_primary_get(unit, port, &primary));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_offset_get(unit, port, &offset));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_toplvl_reg_read(unit, port, primary, 0x0, &data));
        if ( value ) {
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_toplvl_reg_write(unit, port, primary, 0x0, 
                     ( data & 0x0f ) | (( offset & 0x7 )<<4 )));
        } else {
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_toplvl_reg_write(unit, port, primary, 0x0, 
                     ( data & 0x0f ) | (0x8<<4) ));
        }
        break;

    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        /* Not supported for 10/100/1000BASE-T interfaces */
        rv = SOC_E_UNAVAIL;
        break;

    case SOC_PHY_CONTROL_EEE:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }

        if (value == 1) {
            /* BCM53125 didn't support Auto EEE */
            if ((!PHY_IS_BCM53125(pc)) && (!PHY_IS_BCM53128(pc)) 
                && (!PHY_IS_BCM53324(pc))) { 
                /* Disable Auto EEE */
                SOC_IF_ERROR_RETURN
                    (_phy_54680_eee_auto_enable(unit, port, 0));
            }
            /* Enable Native EEE */
            SOC_IF_ERROR_RETURN
                (_phy_54680_eee_enable(unit, port, 1));
            saved_autoneg = pc->copper.autoneg_enable;
            /* Initiate AN */
            SOC_IF_ERROR_RETURN
                (phy_54680_autoneg_set(unit, port, 1));
            pc->copper.autoneg_enable = saved_autoneg;
        } else {
            SOC_IF_ERROR_RETURN
                (_phy_54680_eee_enable(unit, port, 0));
                /* Initiate AN if administratively enabled */
                SOC_IF_ERROR_RETURN
                    (phy_54680_autoneg_set(unit, port, pc->copper.autoneg_enable ? 1 : 0));
        }


        break;

    case SOC_PHY_CONTROL_EEE_AUTO:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || 
            PHY_IS_BCM53324(pc)) {
            rv = SOC_E_UNAVAIL;
        }
        if (value == 1) {
            /* Disable Native EEE */
            SOC_IF_ERROR_RETURN
                (_phy_54680_eee_enable(unit, port, 0));
            /* Enable Auto EEE */
            SOC_IF_ERROR_RETURN
                (_phy_54680_eee_auto_enable(unit, port, 1));
            saved_autoneg = pc->copper.autoneg_enable;
            /* Initiate AN */
            SOC_IF_ERROR_RETURN
                (phy_54680_autoneg_set(unit, port, 1));
            pc->copper.autoneg_enable = saved_autoneg;
        } else {
            /* Disable Auto EEE */
            SOC_IF_ERROR_RETURN
                (_phy_54680_eee_auto_enable(unit, port, 0));
                /* Initiate AN if administratively enabled */
                SOC_IF_ERROR_RETURN
                    (phy_54680_autoneg_set(unit, port, pc->copper.autoneg_enable ? 1 : 0));
        }

        break;

    case SOC_PHY_CONTROL_EEE_AUTO_IDLE_THRESHOLD:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }

        if (value > 7) { /* Values needs to between 0 to 7 (inclusive) */
            rv = SOC_E_CONFIG;
        }

        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   value<<8, 0x0700));
        /* Setting GPHY core, MII buffer register as well */
        SOC_IF_ERROR_RETURN
           (MODIFY_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, 
                                                    value<<8, 0x0700));
        break;


    case SOC_PHY_CONTROL_EEE_AUTO_FIXED_LATENCY:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        if (value == 1) { /* Sets Fixed Latency */
            SOC_IF_ERROR_RETURN
               (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0000, 0x0004));
            /* Setting GPHY core, MII buffer register as well */
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, 
                                                        0x0000, 0x0004));
        } else { /* Sets Variable Latency */
            SOC_IF_ERROR_RETURN
               (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0004, 0x0004));
            /* Setting GPHY core, MII buffer register as well */
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, 
                                                        0x0004, 0x0004));
        }
        break;


    case SOC_PHY_CONTROL_EEE_AUTO_BUFFER_LIMIT:
        /* Buffer limit modification is not allowed */

    case SOC_PHY_CONTROL_EEE_TRANSMIT_WAKE_TIME:
    case SOC_PHY_CONTROL_EEE_RECEIVE_WAKE_TIME:
    case SOC_PHY_CONTROL_EEE_TRANSMIT_SLEEP_TIME:
    case SOC_PHY_CONTROL_EEE_RECEIVE_SLEEP_TIME:
    case SOC_PHY_CONTROL_EEE_TRANSMIT_QUIET_TIME:
    case SOC_PHY_CONTROL_EEE_RECEIVE_QUIET_TIME:
    case SOC_PHY_CONTROL_EEE_TRANSMIT_REFRESH_TIME:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }

        /* Time modification not allowed, IEEE EEE spec constants */
        rv = SOC_E_UNAVAIL;
        break;

    case SOC_PHY_CONTROL_EEE_STATISTICS_CLEAR:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        
        if (value == 1) {
            /* Clearing the stats: Enable clear on Read  */
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, 
                                                        0x1000, 0x1000));

            /* Read all the stats to clear */
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_TX_EVENTSr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_TX_EVENTSr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_TX_DURATIONr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_TX_DURATIONr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_RX_EVENTSr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_RX_EVENTSr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_RX_DURATIONr(unit, pc, &data));
            SOC_IF_ERROR_RETURN
                (READ_PHY54680_EXP_EEE_RX_DURATIONr(unit, pc, &data));

            /* Disable Clear on Read  */
            SOC_IF_ERROR_RETURN
               (MODIFY_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, 
                                                        0x0000, 0x1000));
        }

        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 * Function:
 *      phy_54680_control_get
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
phy_54680_control_get(int unit, soc_port_t port,
                     soc_phy_control_t type, uint32 *value)
{
    int rv;
    phy_ctrl_t *pc;
    uint16 data;
    uint32 temp;
    soc_port_t primary;
    int offset;

    if ((type < 0) || (type >= SOC_PHY_CONTROL_COUNT)) {
        return SOC_E_PARAM;
    }

    pc = EXT_PHY_SW_STATE(unit, port);
    rv = SOC_E_NONE;

    switch(type) {
    case SOC_PHY_CONTROL_POWER:
            *value = pc->power_mode;
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_SLEEP_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_AUTO_POWER_DOWNr(unit,pc, &data));

        if (data & PHY_54680_AUTO_PWRDWN_SLEEP_MASK) {
            *value = PHY_54680_AUTO_PWRDWN_SLEEP_MAX;
        } else {
            *value = 2700;
        }
        break;

    case SOC_PHY_CONTROL_POWER_AUTO_WAKE_TIME:
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_AUTO_POWER_DOWNr(unit,pc, &data));

        data &= PHY_54680_AUTO_PWRDWN_WAKEUP_MASK;
        *value = data * PHY_54680_AUTO_PWRDWN_WAKEUP_UNIT;
        break;

    case SOC_PHY_CONTROL_PORT_PRIMARY:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_primary_get(unit, port, &primary));
            *value = (uint32) primary;
        break;

    case SOC_PHY_CONTROL_PORT_OFFSET:
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_offset_get(unit, port, &offset));
            *value = (uint32) offset;
        break;

    case SOC_PHY_CONTROL_CLOCK_ENABLE:
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_primary_get(unit, port, &primary));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_offset_get(unit, port, &offset));
        if (soc_phyctrl_toplvl_reg_read(unit, port, primary, 0x0, &data) == SOC_E_NONE) {
            if (( data & 0x8 ) || (( data & 0x7 ) != offset)) {
                *value = FALSE;
            } else {
                *value = TRUE;
            }
        }
        break;

    case SOC_PHY_CONTROL_CLOCK_SECONDARY_ENABLE:
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_primary_get(unit, port, &primary));
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_offset_get(unit, port, &offset));
        if (soc_phyctrl_toplvl_reg_read(unit, port, primary, 0x0, &data) == SOC_E_NONE) {
            if (( data & (0x8<<4) ) || (( data & (0x7<<4) ) != (offset<<4) )) {
                *value = FALSE;
            } else {
                *value = TRUE;
            }
        }
        break;

    case SOC_PHY_CONTROL_LINKDOWN_TRANSMIT:
        /* Not supported for 10/100/1000BASE-T interfaces */
        rv = SOC_E_UNAVAIL;
        break;

    case SOC_PHY_CONTROL_EEE:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EEE_TEST_CTRL_Br(unit, pc, &data));

        if ((data & 0xc000) == 0xc000) {
            *value = 1;
        } else {
            *value = 0;
        }

        break;

    case SOC_PHY_CONTROL_EEE_AUTO:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || 
            PHY_IS_BCM53324(pc)) {
            rv = SOC_E_UNAVAIL;
        }

        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_read(unit, pc, 0, 7, &data));

        if    ((data & 0x0003) == 0x0001) {
            *value = 1;
        } else {
            *value = 0;
        }

        break;

    case SOC_PHY_CONTROL_EEE_AUTO_FIXED_LATENCY:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, &data));
        *value = ((data & 0x0004) > 0);
        break;

    case SOC_PHY_CONTROL_EEE_AUTO_IDLE_THRESHOLD:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_MII_BUF_CNTL_STAT1r(unit, pc, &data));
        *value = ((data & 0x0700) > 8);
        break;

    case SOC_PHY_CONTROL_EEE_TRANSMIT_EVENTS:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || 
            PHY_IS_BCM53324(pc)) {
            /* Internal GPHY of BCM53125 didn't support */
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_TX_EVENTSr(unit, pc, &data));
        temp = data;
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 1U<<14, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_TX_EVENTSr(unit, pc, &data));
        temp |= (data << 16);
        *value = temp;
        break;

    case SOC_PHY_CONTROL_EEE_TRANSMIT_DURATION:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        if (PHY_IS_BCM53125(pc) || PHY_IS_BCM53128(pc) || 
            PHY_IS_BCM53324(pc)) {
            /* Internal GPHY of BCM53125 didn't support */
            rv = SOC_E_UNAVAIL;
            return rv;
        }
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_TX_DURATIONr(unit, pc, &data));
        temp = data;
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 1U<<14, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_TX_DURATIONr(unit, pc, &data));
        temp |= (data << 16);
        *value = temp;
        break;

    case SOC_PHY_CONTROL_EEE_RECEIVE_EVENTS:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_RX_EVENTSr(unit, pc, &data));
        temp = data;
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 1U<<14, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_RX_EVENTSr(unit, pc, &data));
        temp |= (data << 16);
        *value = temp;
        break;

    case SOC_PHY_CONTROL_EEE_RECEIVE_DURATION:
        if (!PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE)) {
            rv = SOC_E_UNAVAIL;
        }
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 0x0, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_RX_DURATIONr(unit, pc, &data));
        temp = data;
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FAF,0x15, 1U<<14, 1U<<14)); /* exp af */
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_EEE_RX_DURATIONr(unit, pc, &data));
        temp |= (data << 16);
        *value = temp;
        break;

    default:
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 * Function:
 *      phy_54680_cable_diag
 * Purpose:
 *      Run 546x cable diagnostics
 * Parameters:
 *      unit - device number
 *      port - port number
 *      status - (OUT) cable diagnotic status structure
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
phy_54680_cable_diag(int unit, soc_port_t port,
                    soc_port_cable_diag_t *status)
{
    int                 rv, rv2, i;

    extern int phy_5464_cable_diag_sw(int, soc_port_t ,
                                      soc_port_cable_diag_t *);

    if (status == NULL) {
        return SOC_E_PARAM;
    }

    status->state = SOC_PORT_CABLE_STATE_OK;
    status->npairs = 4;
    status->fuzz_len = 0;
    for (i = 0; i < 4; i++) {
        status->pair_state[i] = SOC_PORT_CABLE_STATE_OK;
    }

    MIIM_LOCK(unit);    /* this locks out linkscan, essentially */
    rv = phy_5464_cable_diag_sw(unit,port, status);
    MIIM_UNLOCK(unit);
    rv2 = 0;
    if (rv <= 0) {      /* don't reset if > 0 -- link was up */
        rv2 = _phy_54680_reset_setup(unit, port);
    }
    if (rv >= 0 && rv2 < 0) {
        return rv2;
    }
    return rv;
}

/*
 * Function:
 *      phy_54680_probe
 * Purpose:
 *      Complement the generic phy probe routine to identify this phy when its
 *      phy id0 and id1 is same as some other phy's.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      pc   - phy ctrl descriptor.
 * Returns:
 *      SOC_E_NONE,SOC_E_NOT_FOUND and SOC_E_<error>
 */
STATIC int
phy_54680_probe(int unit, phy_ctrl_t *pc)
{
    uint16 id0, id1;
    soc_phy_info_t *pi;

    if (READ_PHY54680_MII_PHY_ID0r(unit, pc, &id0) < 0) {
        return SOC_E_NOT_FOUND;
    }
    if (READ_PHY54680_MII_PHY_ID1r(unit, pc, &id1) < 0) {
        return SOC_E_NOT_FOUND;
    }

    pi = &SOC_PHY_INFO(unit, pc->port);

    switch (PHY_MODEL(id0, id1)) {

    case PHY_BCM54680_MODEL:
    case PHY_BCM54880E_MODEL:
    case PHY_BCM53125_MODEL:
    case PHY_BCM53128_MODEL:
    case PHY_BCM53324_MODEL:
        return SOC_E_NONE;
    break;

    case PHY_BCM54680E_MODEL:
        if (id1 & 0x8) {
            return SOC_E_NONE;
        }
    break;

    case PHY_BCM52681E_MODEL:
        if (id1 & 0x8) {
            return SOC_E_NONE;
        }
    break;

    default:
    break;

    }
    return SOC_E_NOT_FOUND;
}

STATIC int
phy_54680_link_up(int unit, soc_port_t port)
{
    phy_ctrl_t *pc;
    uint16 eee_advert;

    pc = EXT_PHY_SW_STATE(unit, port);

    if (PHY_IS_BCM54880E(pc)) {
        int speed;
        uint32 eee_auto;

        SOC_IF_ERROR_RETURN
            (phy_54680_speed_get(unit, port, &speed));
        SOC_IF_ERROR_RETURN
            (phy_54680_control_get(unit, port,
                     SOC_PHY_CONTROL_EEE_AUTO, &eee_auto));
        if ((speed == 100) && (eee_auto == 1)) {
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0d10));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x00ff));

        } else {
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x17, 0x0d10));
            SOC_IF_ERROR_RETURN(
                WRITE_PHY_REG(unit, pc, 0x15, 0x0000));
        }
    }
    SOC_IF_ERROR_RETURN
        (READ_PHY54680_EEE_ADVr(unit, pc, &eee_advert));
    if (eee_advert & 0x4) {
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x4c00));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x17, 0x001a));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x15, 0x0000));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x4400));
    }

    return SOC_E_NONE;
}

STATIC int
phy_54680_link_down(int unit, soc_port_t port)
{
    phy_ctrl_t *pc;
    uint16 eee_advert;

    pc = EXT_PHY_SW_STATE(unit, port);

    SOC_IF_ERROR_RETURN
        (READ_PHY54680_EEE_ADVr(unit, pc, &eee_advert));
    if (eee_advert & 0x4) {
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x4c00));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x17, 0x001a));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x15, 0x0007));
        SOC_IF_ERROR_RETURN(
            WRITE_PHY_REG(unit, pc, 0x18, 0x4400));
    }

    return SOC_E_NONE;
}

void _phy_54680e_encode_egress_message_mode(soc_port_timesync_event_message_egress_mode_t mode,
                                            int offset, uint16 *value)
{

    switch (mode) {
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE:
        *value |= (0x0 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value |= (0x1 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN:
        *value |= (0x2 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP:
        *value |= (0x3 << offset);
        break;
    default:
        break;
    }

}

void _phy_54680e_encode_ingress_message_mode(soc_port_timesync_event_message_ingress_mode_t mode,
                                            int offset, uint16 *value)
{

    switch (mode) {
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE:
        *value |= (0x0 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value |= (0x1 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP:
        *value |= (0x2 << offset);
        break;
    case SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME:
        *value |= (0x3 << offset);
        break;
    default:
        break;
    }

}

void _phy_54680e_decode_egress_message_mode(uint16 value, int offset,
                                            soc_port_timesync_event_message_egress_mode_t *mode)
{

    switch ((value >> offset) & 0x3) {
    case 0x0:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN;
        break;
    case 0x3:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP;
        break;
    default:
        break;
    }

}

void _phy_54680e_decode_ingress_message_mode(uint16 value, int offset,
                                            soc_port_timesync_event_message_ingress_mode_t *mode)
{

    switch ((value >> offset) & 0x3) {
    case 0x0:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP;
        break;
    case 0x3:
        *mode = SOC_PORT_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME;
        break;
    default:
        break;
    }

}

STATIC int
phy_54680_timesync_config_set(int unit, soc_port_t port, soc_port_timesync_config_t *conf)
{
    phy_ctrl_t *pc, *pc_port1;
    soc_phy_chip_info_t *chip_info;
    soc_port_t primary_port;
    int offset;
    uint16 rx_control_reg = 0, tx_control_reg = 0, rx_crc_control_reg = 0,
           en_control_reg = 0, tx_capture_en_reg = 0, rx_capture_en_reg = 0,
           nse_nco_6 = 0, nse_nco_2c = 0, value;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_primary_get(unit, port, &primary_port));

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_offset_get(unit, port, &offset));

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    pc = EXT_PHY_SW_STATE(unit, port);
    pc_port1 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[0]);

    if ((!(PHY_IS_BCM54880E(pc) || PHY_IS_BCM52681E(pc))) || PHY_IS_BCM54880E_A0A1(pc)) {
        return SOC_E_UNAVAIL;
    }

    if (conf->flags & SOC_PORT_TIMESYNC_ENABLE) {
        en_control_reg |= ((1U << (offset + 8)) | (1U << offset));

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MODE_CTRLr(unit, pc, 0x00, 0x06));

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_SGMII_SLAVEr(unit, pc, 0x02, 0x02));

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EXT_SERDES_CTRLr(unit, pc, 0x00, 0x01));

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0F7F,0x15, 0x7, 0x7)); /* exp 7f */

        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0001, 0x0003));
        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FF5,0x15, 1U<<0, 1U<<0)); /* exp reg f5 */
    } else {

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FF5,0x15, 0, 1U<<0)); /* exp reg f5 */

    }

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_MODE_CTRLr(unit, pc, 0x00, 0x06));

        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_SGMII_SLAVEr(unit, pc, 0x02, 0x02));

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0F7F,0x15, 0x7, 0x7)); /* exp 7f */

        SOC_IF_ERROR_RETURN
           (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 7,
                                                   0x0001, 0x0003));


    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x10, en_control_reg, 
                                       ((1U << (offset + 8)) | (1U << offset))));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x10, &en_control_reg));

        SOC_IF_ERROR_RETURN
            (PHY54680_REG_MODIFY(unit, pc, 0x00, 0x0FF5,0x15, 1U<<0, 1U<<0)); /* exp reg f5 */

    if (conf->flags & SOC_PORT_TIMESYNC_CAPTURE_TS_ENABLE) {
        tx_capture_en_reg |= (1U << offset);
        rx_capture_en_reg |= (1U << (offset + 8));
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x12, tx_capture_en_reg, 
                                       (1U << offset)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x13, rx_capture_en_reg, 
                                       (1U << (offset + 8))));

    if (conf->flags & SOC_PORT_TIMESYNC_HEARTBEAT_TS_ENABLE) {
        nse_nco_6 |= (1U << 13);
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x4f, nse_nco_6, 
                                       (1U << 13)));

    /*
    if (conf->flags & SOC_PORT_TIMESYNC_ONE_STEP) {
    }
     */

    if (conf->flags & SOC_PORT_TIMESYNC_RX_CRC_ENABLE) {
        rx_crc_control_reg |= (1U << 3);
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x34, rx_crc_control_reg));

    if (conf->flags & SOC_PORT_TIMESYNC_8021AS_ENABLE) {
        rx_control_reg |= (1U << 3);
        tx_control_reg |= (1U << 3);
    }

    if (conf->flags & SOC_PORT_TIMESYNC_L2_ENABLE) {
        rx_control_reg |= (1U << 2);
        tx_control_reg |= (1U << 2);
    }

    if (conf->flags & SOC_PORT_TIMESYNC_IP4_ENABLE) {
        rx_control_reg |= (1U << 1);
        tx_control_reg |= (1U << 1);
    }

    if (conf->flags & SOC_PORT_TIMESYNC_IP6_ENABLE) {
        rx_control_reg |= (1U << 0);
        tx_control_reg |= (1U << 0);
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x32, tx_control_reg));
                                          
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x33, rx_control_reg));
                                          
    if (conf->flags & SOC_PORT_TIMESYNC_CLOCK_SRC_EXT) {
        nse_nco_2c &= ~(1U << 14);
    } else {
        nse_nco_2c |= (1U << 14);
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x47, nse_nco_2c, 
                                       (1U << 14)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x35, conf->itpid));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x36, conf->otpid));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x25, (uint16)(conf->original_timecode.nanoseconds & 0xffff)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x26, (uint16)((conf->original_timecode.nanoseconds >> 16) & 0xffff)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x27, (uint16)(COMPILER_64_LO(conf->original_timecode.seconds) & 0xffff)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x28, (uint16)((COMPILER_64_LO(conf->original_timecode.seconds) >> 16) & 0xffff)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x29, (uint16)(COMPILER_64_HI(conf->original_timecode.seconds) & 0xffff)));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x4f, ((conf->gmode + 1) << 14), 
                                       (3U << 14))); /* +1 does the encoding  for the chip */
    /* tx/rx_timestamp_offset */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x15 + offset, (uint16)(((conf->rx_timestamp_offset & 0xf0000) >> 4) |
                                          (conf->tx_timestamp_offset & 0xfff))));
    /* rx_timestamp_offset */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x1d + offset, (uint16)(conf->rx_timestamp_offset & 0xffff)));

    value = 0;
    _phy_54680e_encode_egress_message_mode(conf->tx_sync_mode, 0, &value);
    _phy_54680e_encode_egress_message_mode(conf->tx_delay_request_mode, 2, &value);
    _phy_54680e_encode_egress_message_mode(conf->tx_pdelay_request_mode, 4, &value);
    _phy_54680e_encode_egress_message_mode(conf->tx_pdelay_response_mode, 6, &value);
                                            
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 0x11, value, 0x00ff)); 

    value = 0;
    _phy_54680e_encode_ingress_message_mode(conf->rx_sync_mode, 0, &value);
    _phy_54680e_encode_ingress_message_mode(conf->rx_delay_request_mode, 2, &value);
    _phy_54680e_encode_ingress_message_mode(conf->rx_pdelay_request_mode, 4, &value);
    _phy_54680e_encode_ingress_message_mode(conf->rx_pdelay_response_mode, 6, &value);
                                            
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_modify(unit, pc, 0, 0x12, value << 8, 0xff00)); 

    return SOC_E_NONE;
}

STATIC int
phy_54680_timesync_config_get(int unit, soc_port_t port, soc_port_timesync_config_t *conf)
{
    phy_ctrl_t *pc, *pc_port1;
    soc_phy_chip_info_t *chip_info;
    soc_port_t primary_port;
    int offset;
    uint16 tx_control_reg = 0, rx_crc_control_reg = 0,
           en_control_reg = 0, tx_capture_en_reg = 0,
           nse_nco_6 = 0, nse_nco_2c = 0, temp1, temp2, temp3, value;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_primary_get(unit, port, &primary_port));

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_offset_get(unit, port, &offset));

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    pc = EXT_PHY_SW_STATE(unit, port);
    pc_port1 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[0]);

    if ((!(PHY_IS_BCM54880E(pc) || PHY_IS_BCM52681E(pc))) || PHY_IS_BCM54880E_A0A1(pc)) {
        return SOC_E_UNAVAIL;
    }

    conf->flags = 0;

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x10, &en_control_reg)); 

    if (en_control_reg & (1U << offset)) {
        conf->flags |= SOC_PORT_TIMESYNC_ENABLE;
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x12, &tx_capture_en_reg)); 

    if (tx_capture_en_reg & (1U << offset)) {
        conf->flags |= SOC_PORT_TIMESYNC_CAPTURE_TS_ENABLE;
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x4f, &nse_nco_6));

    if (nse_nco_6 & (1U << 13)) {
        conf->flags |= SOC_PORT_TIMESYNC_HEARTBEAT_TS_ENABLE;
    }

    conf->gmode = ((nse_nco_6 >> 14) - 1) & 0x3; /* -1 does the decoding for the chip */

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x34, &rx_crc_control_reg));

    if (rx_crc_control_reg & (1U << 3)) {
        conf->flags |= SOC_PORT_TIMESYNC_RX_CRC_ENABLE;
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x32, &tx_control_reg));
                                          
    if (tx_control_reg & (1U << 3)) {
        conf->flags |= SOC_PORT_TIMESYNC_8021AS_ENABLE;
    }

    if (tx_control_reg & (1U << 2)) {
        conf->flags |= SOC_PORT_TIMESYNC_L2_ENABLE;
    }

    if (tx_control_reg & (1U << 1)) {
        conf->flags |= SOC_PORT_TIMESYNC_IP4_ENABLE;
    }

    if (tx_control_reg & (1U << 0)) {
        conf->flags |= SOC_PORT_TIMESYNC_IP6_ENABLE;
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x47, &nse_nco_2c));

    if (!(nse_nco_2c & (1U << 14))) {
        conf->flags |= SOC_PORT_TIMESYNC_CLOCK_SRC_EXT;
    }

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x35, &conf->itpid));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x36, &conf->otpid));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x25, &temp1));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x26, &temp2));

    conf->original_timecode.nanoseconds = ((uint32)temp2 << 16) | temp1; 

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x27, &temp1));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x28, &temp2));

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x29, &temp3));

    /* conf->original_timecode.seconds = ((uint64)temp3 << 32) | ((uint32)temp2 << 16) | temp1; */

    COMPILER_64_SET(conf->original_timecode.seconds, ((uint32)temp3),  (((uint32)temp2<<16)|((uint32)temp1)));

    /* tx/rx_timestamp_offset */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x15 + offset, &temp1));
    /* rx_timestamp_offset */
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x1d + offset, &temp2));

    conf->tx_timestamp_offset = temp1 & 0xfff;
    conf->rx_timestamp_offset = (((uint32)(temp1 & 0xf000)) << 4) | temp2;

    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc, 0, 0x11, &value)); 

    _phy_54680e_decode_egress_message_mode(value, 0, &conf->tx_sync_mode);
    _phy_54680e_decode_egress_message_mode(value, 2, &conf->tx_delay_request_mode);
    _phy_54680e_decode_egress_message_mode(value, 4, &conf->tx_pdelay_request_mode);
    _phy_54680e_decode_egress_message_mode(value, 6, &conf->tx_pdelay_response_mode);
                                            
    SOC_IF_ERROR_RETURN
        (_phy_54680e_blk_top_lvl_reg_read(unit, pc, 0, 0x12, &value)); 

    value >>= 8;

    _phy_54680e_decode_ingress_message_mode(value, 0, &conf->rx_sync_mode);
    _phy_54680e_decode_ingress_message_mode(value, 2, &conf->rx_delay_request_mode);
    _phy_54680e_decode_ingress_message_mode(value, 4, &conf->rx_pdelay_request_mode);
    _phy_54680e_decode_ingress_message_mode(value, 6, &conf->rx_pdelay_response_mode);
                                            
    return SOC_E_NONE;
}

STATIC int
phy_54680_timesync_control_set(int unit, soc_port_t port, soc_port_control_timesync_t type, uint64 value)
{
    phy_ctrl_t *pc, *pc_port1;
    soc_phy_chip_info_t *chip_info;
    soc_port_t primary_port;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_primary_get(unit, port, &primary_port));

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    pc = EXT_PHY_SW_STATE(unit, port);
    pc_port1 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[0]);

    if ((!(PHY_IS_BCM54880E(pc) || PHY_IS_BCM52681E(pc))) || PHY_IS_BCM54880E_A0A1(pc)) {
        return SOC_E_UNAVAIL;
    }

    switch (type) {

    case SOC_PORT_CONTROL_TIMESYNC_NCOADDEND:
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x44, (uint16)COMPILER_64_LO(value)));
        break;

    case SOC_PORT_CONTROL_TIMESYNC_FRAMESYNC:
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x4f, COMPILER_64_LO(value) ? (1U << 5) : 0, 
                                       (0xF << 2)));
        break;

    case SOC_PORT_CONTROL_TIMESYNC_LOCAL_TIME:
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x45, (uint16)(COMPILER_64_LO(value) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_write(unit, pc_port1, 0, 0x46, (uint16)((COMPILER_64_LO(value) >> 16) & 0xffff)));
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_modify(unit, pc_port1, 0, 0x47, (uint16)(COMPILER_64_HI(value) & 0xfff), 0xfff));

    default:
        return SOC_E_FAIL;
        break;
    }

    return SOC_E_NONE;
}

STATIC int
phy_54680_timesync_control_get(int unit, soc_port_t port, soc_port_control_timesync_t type, uint64 *value)
{
    phy_ctrl_t *pc, *pc_port1;
    soc_phy_chip_info_t *chip_info;
    soc_port_t primary_port;
    uint16 value0;
    uint16 value1;
    uint16 value2;
    uint16 value3;

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_primary_get(unit, port, &primary_port));

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    pc = EXT_PHY_SW_STATE(unit, port);
    pc_port1 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[0]);

    if ((!(PHY_IS_BCM54880E(pc) || PHY_IS_BCM52681E(pc))) || PHY_IS_BCM54880E_A0A1(pc)) {
        return SOC_E_UNAVAIL;
    }

    switch (type) {
    case SOC_PORT_CONTROL_TIMESYNC_HEARTBEAT_TIMESTAMP:
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EXP_READ_START_END_CTRLr(unit, pc_port1, 0x1 << 0, 0x3 << 0 ));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_HEARTBEAT_TIMESTAMPr(unit, pc_port1, &value0));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_HEARTBEAT_TIMESTAMPr(unit, pc_port1, &value1));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_HEARTBEAT_TIMESTAMPr(unit, pc_port1, &value2));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_HEARTBEAT_TIMESTAMPr(unit, pc_port1, &value3));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EXP_READ_START_END_CTRLr(unit, pc_port1,0x2 << 0, 0x3 << 0 ));

        COMPILER_64_SET((*value), (((uint32)value3<<16)|((uint32)value2)),  (((uint32)value1<<16)|((uint32)value0)));

    /*    *value = (((uint64)value3) << 48) | (((uint64)value2) << 32) | (((uint64)value1) << 16) | ((uint64)value0); */
        break;

    case SOC_PORT_CONTROL_TIMESYNC_CAPTURE_TIMESTAMP:
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EXP_READ_START_END_CTRLr(unit, pc_port1, 0x1 << 2, 0x3 << 2 ));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_CAPTURE_TIMESTAMPr(unit, pc_port1, &value0));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_CAPTURE_TIMESTAMPr(unit, pc_port1, &value1));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_CAPTURE_TIMESTAMPr(unit, pc_port1, &value2));
        SOC_IF_ERROR_RETURN
            (READ_PHY54680_EXP_CAPTURE_TIMESTAMPr(unit, pc_port1, &value3));
        SOC_IF_ERROR_RETURN
            (MODIFY_PHY54680_EXP_READ_START_END_CTRLr(unit, pc_port1, 0x2 << 2, 0x3 << 2 ));

        COMPILER_64_SET((*value), (((uint32)value3<<16)|((uint32)value2)),  (((uint32)value1<<16)|((uint32)value0)));

    /*   *value = (((uint64)value3) << 48) | (((uint64)value2) << 32) | (((uint64)value1) << 16) | ((uint64)value0); */
        break;

    case SOC_PORT_CONTROL_TIMESYNC_NCOADDEND:
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x44, &value0));
        /* *value = value0; */
        COMPILER_64_SET((*value), 0,  (uint32)value0);
        break;

    case SOC_PORT_CONTROL_TIMESYNC_FRAMESYNC:
        SOC_IF_ERROR_RETURN
            (_phy_54680e_blk_top_lvl_reg_read(unit, pc_port1, 0, 0x4f, &value0));
        /* *value = (value0 & (1U << 5)) > 0; */
        COMPILER_64_SET((*value), 0,  (uint32)((value0 & (1U << 5)) > 0));
        break;

    default:
        return SOC_E_FAIL;
        break;
    }

    return SOC_E_NONE;
}

/*
 * Variable:    phy_54680drv_ge
 * Purpose:     PHY driver for 54680
 */

phy_driver_t phy_54680drv_ge = {
    "54680 Gigabit PHY Driver",
    phy_54680_init,
    phy_fe_ge_reset,
    phy_54680_link_get,
    phy_54680_enable_set,
    phy_54680_enable_get,
    phy_54680_duplex_set,
    phy_54680_duplex_get,
    phy_54680_speed_set,
    phy_54680_speed_get,
    phy_54680_master_set,
    phy_54680_master_get,
    phy_54680_autoneg_set,
    phy_54680_autoneg_get,
    NULL,
    NULL,
    NULL,
    phy_54680_lb_set,
    phy_54680_lb_get,
    phy_54680_interface_set,
    phy_54680_interface_get,
    NULL,                       /* Deprecated */
    phy_54680_link_up,          /* Link up event */
    phy_54680_link_down,
    phy_54680_mdix_set,
    phy_54680_mdix_get,
    phy_54680_mdix_status_get,
    phy_54680_medium_config_set,
    phy_54680_medium_config_get,
    phy_54680_medium_status,
    phy_54680_cable_diag,
    NULL,                        /* phy_link_change */
    phy_54680_control_set,       /* phy_control_set */ 
    phy_54680_control_get,       /* phy_control_get */
    phy_ge_reg_read,
    phy_ge_reg_write,
    phy_ge_reg_modify,
    NULL,                        /* Phy notify event */    
    phy_54680_probe,             /* pd_probe  */
    phy_54680_ability_advert_set,/* pd_ability_advert_set */
    phy_54680_ability_advert_get,/* pd_ability_advert_get */
    phy_54680_ability_remote_get,/* pd_ability_remote_get */
    phy_54680_ability_local_get, /* pd_ability_local_get  */
    NULL,                        /* pd_firmware_set */    
    phy_54680_timesync_config_set,
    phy_54680_timesync_config_get,
    phy_54680_timesync_control_set,
    phy_54680_timesync_control_get
};

#else /* INCLUDE_PHY_54680_ESW */
int _phy_54680_not_empty;
#endif /* INCLUDE_PHY_54680_ESW */

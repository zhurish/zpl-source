/*
 * $Id: phyctrl.c,v 1.89.4.6 Broadcom SDK $
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
 * StrataSwitch PHY control API
 * All access to PHY drivers should call the following soc functions.
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/port_ability.h>
#include <soc/phy.h>
#include <soc/phyreg.h>
#include <soc/phyctrl.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>
#include <soc/error.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif
#ifdef INCLUDE_MACSEC
#include <soc/macsecphy.h>
#endif

/* PHY address to port re-mapping */
int *phy_rmap[SOC_MAX_NUM_DEVICES];

/* Per port data structure to manage PHY drivers */
soc_phy_info_t *phy_port_info[SOC_MAX_NUM_DEVICES];

/* Per PHY data structure to manage PHY drivers */
phy_ctrl_t **int_phy_ctrl[SOC_MAX_NUM_DEVICES];
phy_ctrl_t **ext_phy_ctrl[SOC_MAX_NUM_DEVICES];

#define SOC_PHYCTRL_INIT_CHECK(_ext_pc, _int_pc) \
    if ((NULL == (_ext_pc)) && (NULL == (_int_pc))) { \
        return SOC_E_INIT; \
    } 

#define SOC_NULL_PARAM_CHECK(_param) \
    if (NULL == (_param)) { \
        return SOC_E_PARAM; \
    }

#ifdef PHYCTRL_DEBUG_PRINT
#define PHYCTRL_DEBUG(_message) soc_cm_debug _message
#else
#define PHYCTRL_DEBUG(_message) 
#endif 

int
soc_phyctrl_software_deinit(int unit)
{
    int port;

    if (NULL != phy_port_info[unit]) {
        PBMP_PORT_ITER(unit, port) {
            if (NULL != phy_port_info[unit][port].chip_info) {
                sal_free(phy_port_info[unit][port].chip_info);
                phy_port_info[unit][port].chip_info = NULL;
            }
        }
        sal_free(phy_port_info[unit]);
        phy_port_info[unit] = NULL;
    }

    if (NULL != int_phy_ctrl[unit]) {
        PBMP_PORT_ITER(unit, port) {
            if (NULL != int_phy_ctrl[unit][port]) {
                sal_free(int_phy_ctrl[unit][port]);
                int_phy_ctrl[unit][port] = NULL;
            }
        }
        sal_free(int_phy_ctrl[unit]);
        int_phy_ctrl[unit] = NULL;
    }

    if (NULL != ext_phy_ctrl[unit]) {
        PBMP_PORT_ITER(unit, port) {
            if (NULL != ext_phy_ctrl[unit][port]) {
                sal_free(ext_phy_ctrl[unit][port]);
                ext_phy_ctrl[unit][port] = NULL;
            }
        }
        sal_free(ext_phy_ctrl[unit]);
        ext_phy_ctrl[unit] = NULL;
    }

    if (NULL != phy_rmap[unit]) {
        sal_free(phy_rmap[unit]);
        phy_rmap[unit] = NULL;
    }

    SOC_IF_ERROR_RETURN (soc_phy_deinit(unit));

    return (SOC_E_NONE);
}

int
soc_phyctrl_software_init(int unit)
{
    int port;

    if ((phy_port_info[unit] != NULL) ||
        (int_phy_ctrl[unit] != NULL) ||
        (ext_phy_ctrl[unit] != NULL) ||
        (phy_rmap[unit] != NULL)) {
        soc_phyctrl_software_deinit(unit);
    }

    phy_port_info[unit] = sal_alloc(sizeof(soc_phy_info_t) * 
                                    SOC_MAX_NUM_PORTS,
                                    "phy_port_info");
    if (phy_port_info[unit] == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(phy_port_info[unit], 0, 
               sizeof(soc_phy_info_t) * SOC_MAX_NUM_PORTS);

    int_phy_ctrl[unit] = sal_alloc(sizeof(phy_ctrl_t *) *
                                   SOC_MAX_NUM_PORTS,
                                   "int_phy_ctrl");
    if (int_phy_ctrl[unit] == NULL) {
        soc_phyctrl_software_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(int_phy_ctrl[unit], 0, 
               sizeof(phy_ctrl_t *) * SOC_MAX_NUM_PORTS);


    ext_phy_ctrl[unit] = sal_alloc(sizeof(phy_ctrl_t *) *
                                   SOC_MAX_NUM_PORTS,
                                   "ext_phy_ctrl");
    if (ext_phy_ctrl[unit] == NULL) {
        soc_phyctrl_software_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(ext_phy_ctrl[unit], 0, 
               sizeof(phy_ctrl_t *) * SOC_MAX_NUM_PORTS);

    phy_rmap[unit] = sal_alloc(sizeof(int) * EXT_PHY_ADDR_MAX,
                               "phy_rmap");
    if (phy_rmap[unit] == NULL) {
        soc_phyctrl_software_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(phy_rmap[unit], -1, sizeof(int) * EXT_PHY_ADDR_MAX);

    /* Initialize PHY driver table and assign default PHY address */
    SOC_IF_ERROR_RETURN
        (soc_phy_init(unit));

    PBMP_PORT_ITER(unit, port) {
        if (PHY_ADDR(unit, port) >= EXT_PHY_ADDR_MAX ||
            PHY_ADDR_INT(unit, port) >= EXT_PHY_ADDR_MAX) {
            soc_cm_debug(DK_ERR, "soc_phyctrl_software_init: intPhyAddr 0x%x "
              "or extPhyAddr 0x%x exceeds max size u=%d p=%d FAILED ", 
               PHY_ADDR_INT(unit, port),PHY_ADDR(unit, port),unit, port);
            return SOC_E_PARAM;
        }
        PHY_ADDR_TO_PORT(unit, PHY_ADDR(unit, port)) = port;
        PHY_ADDR_TO_PORT(unit, PHY_ADDR_INT(unit, port)) = port;
    }

    PHYCTRL_DEBUG((DK_PHY | DK_VERBOSE, 
                 "soc_phyctrl_software_init Unit %d\n",
                      unit));

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phyctrl_drv_name_get
 * Purpose:
 *      Get PHY driver name.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      name - Buffer for PHY driver name.
 *      len  - Length of buffer.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_drv_name_get(int unit, soc_port_t port, char *name, int len)
{
    static char       unknown_driver[] = "unknown driver";
    phy_driver_t      *pd;
    int               string_len;

    pd = NULL;
    if (NULL != EXT_PHY_SW_STATE(unit, port)) {
        pd = EXT_PHY_SW_STATE(unit, port)->pd;
    } else if (NULL != INT_PHY_SW_STATE(unit, port)) {
        pd = INT_PHY_SW_STATE(unit, port)->pd;
    }
    
    if (NULL == pd) {
        string_len = (int)sizeof(unknown_driver);
        if (string_len <= len) {
            sal_strcpy(name, unknown_driver);
        }
        return SOC_E_NOT_FOUND;
    }

    string_len = (int)sal_strlen(pd->drv_name);
    if (string_len > len) {
        return SOC_E_MEMORY;
    }

    sal_strcpy(name, pd->drv_name);

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phyctrl_probe
 * Purpose:
 *      Probe for internal and external PHYs attached to the port.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_probe(int unit, soc_port_t port)
{
    int           rv;
    phy_driver_t *ext_pd;
    phy_driver_t *int_pd;
    phy_ctrl_t   ext_pc;
    phy_ctrl_t   int_pc;
    phy_ctrl_t   *pc;
    rv = SOC_E_NONE;
    sal_memset(&ext_pc, 0, sizeof(phy_ctrl_t));
    sal_memset(&int_pc, 0, sizeof(phy_ctrl_t));

    /* The soc layer probe function always uses the default PHY addresses
     * instead of PHY address stored in phy_info from previous probe.
     * This make sure that the external PHY probe works correctly even
     * when the device is hot plugged.  
     */
    int_pc.unit      = unit;
    int_pc.port      = port;
    int_pc.speed_max = SOC_INFO(unit).port_speed_max[port];
    ext_pc.unit      = unit;
    ext_pc.port      = port;
    ext_pc.speed_max =  SOC_INFO(unit).port_speed_max[port];

#ifdef BCM_SBX_SUPPORT
    if (SOC_IS_SBX(unit)) {
        int          phy_clause = 22;
        if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
            phy_clause = 45;
        }
        /* Clause 45 instead of Clause 22 MDIO access */
        phy_clause = soc_property_port_get(unit, port, spn_PORT_PHY_CLAUSE, 
                               phy_clause);
 
        /* SBX verson of register access */
        int_pc.read  = soc_sbx_miim_read;
	int_pc.write = soc_sbx_miim_write; 
        if (phy_clause == 45) {
	    ext_pc.read  = soc_sbx_miimc45_read;
	    ext_pc.write = soc_sbx_miimc45_write;
	} else {
	    ext_pc.read  = soc_sbx_miim_read;
	    ext_pc.write = soc_sbx_miim_write; 
	}
    } else
#endif /* BCM_SBX_SUPPORT */
#ifdef BCM_ROBO_SUPPORT
     if (SOC_IS_ROBO(unit)) {
        /* ROBO version of register access */
        int_pc.read  = soc_robo_miim_int_read;
        int_pc.write = soc_robo_miim_int_write;
        
        ext_pc.read  = soc_robo_miim_read;
        ext_pc.write = soc_robo_miim_write;
    } else 
#endif /* BCM_ROBO_SUPPORT */
    {
#ifdef BCM_ESW_SUPPORT
        int          phy_clause = 22;
        int_pc.read  = soc_esw_miim_read;
        int_pc.write = soc_esw_miim_write; 
        if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
            phy_clause = 45;
        }
        /* Clause 45 instead of Clause 22 MDIO access */
        phy_clause = soc_property_port_get(unit, port, spn_PORT_PHY_CLAUSE, 
                               phy_clause);

        if (phy_clause == 45) {
            ext_pc.read  = soc_esw_miimc45_read;
            ext_pc.write = soc_esw_miimc45_write;
        } else {
            ext_pc.read  = soc_esw_miim_read;
            ext_pc.write = soc_esw_miim_write; 
        }
#ifdef INCLUDE_I2C
        if (soc_property_port_get(unit, port,
                             spn_PHY_BUS_I2C, 0)) {
            ext_pc.read  = phy_i2c_miireg_read;
            ext_pc.write = phy_i2c_miireg_write;
        }
#endif
#endif /* BCM_ESW_SUPPORT */
    }

    SOC_IF_ERROR_RETURN
        (soc_phy_probe(unit, port, &ext_pc, &int_pc));

    ext_pd = ext_pc.pd;
    int_pd = int_pc.pd;
    if (ext_pd == int_pd) {
        /* If external PHY driver and internal PHY driver are the same,
         * config setting must have override the PHY driver selection.
         * In this case just attach internal PHY driver. 
         * Internal driver is needed for all devices with internal
         * SerDes because MAC driver performs notify calls to internal
         * PHY driver. 
         */
        ext_pd = NULL;
    }

    if (NULL != ext_pd) {
        if (NULL == EXT_PHY_SW_STATE(unit, port)) {
            EXT_PHY_SW_STATE(unit, port) =
                sal_alloc (sizeof (phy_ctrl_t) + ext_pc.size, ext_pd->drv_name);
            if (NULL == EXT_PHY_SW_STATE(unit, port)) {
                rv = SOC_E_MEMORY;
            }
        }
        if (SOC_SUCCESS(rv)) {
            pc = EXT_PHY_SW_STATE(unit, port);
            sal_memcpy(pc, &ext_pc, sizeof(phy_ctrl_t));
            rv = soc_phy_reset_register(unit, port, pc->pd->pd_reset, 
                                        NULL, TRUE);
            PHY_ADDR_TO_PORT(unit, PHY_ADDR(unit, port)) = port;
            soc_cm_debug(DK_PHY, 
                         "soc_phyctrl_probe external: u=%d p=%d %s\n",
                              unit, port, ext_pd->drv_name);
        } 
    } else {
        /* No external PHY detected. If there is allocated memory for
         * external PHY driver, free it.
         */ 
        if (NULL != EXT_PHY_SW_STATE(unit, port)) {
            sal_free(EXT_PHY_SW_STATE(unit, port));
            EXT_PHY_SW_STATE(unit, port) = NULL;
        }
    }

    if (SOC_SUCCESS(rv) && NULL != int_pd) {
        if (NULL == INT_PHY_SW_STATE(unit, port)) {
            INT_PHY_SW_STATE(unit, port) =
                sal_alloc (sizeof (phy_ctrl_t) + int_pc.size, int_pd->drv_name);
            if (NULL == INT_PHY_SW_STATE(unit, port)) {
                rv = SOC_E_MEMORY;
            }
        } else {
            phy_ctrl_t  *ppc = INT_PHY_SW_STATE(unit, port);
            if (ppc->driver_data) {
                /* If driver data allocated, must free it */
                sal_free(ppc->driver_data);
            }
        }
        if (SOC_SUCCESS(rv)) {
            pc = INT_PHY_SW_STATE(unit, port);
            sal_memcpy(pc, &int_pc, sizeof(phy_ctrl_t));
            PHY_ADDR_TO_PORT(unit, PHY_ADDR_INT(unit, port)) = port;
            if (NULL == ext_pd) {
                /* If there is no external PHY, the internal PHY 
                 * must be in fiber mode. 
                 */ 
                if (soc_property_port_get(unit, port,
                                              spn_SERDES_FIBER_PREF, 1)) {
                    PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
                } else {
                    PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
                }
                rv = soc_phy_reset_register(unit, port, pc->pd->pd_reset, 
                                            NULL, TRUE);
            }

            soc_cm_debug(DK_PHY, 
                         "soc_phyctrl_probe internal: u=%d p=%d %s\n",
                              unit, port, int_pd->drv_name);
        }
    } else {
        /* No internal PHY detected. If there is allocated memory for
         * internal PHY driver, free it.
         */ 
        if (NULL != INT_PHY_SW_STATE(unit, port)) {
            phy_ctrl_t  *ppc = INT_PHY_SW_STATE(unit, port);
            if (ppc->driver_data) {
                /* If driver data allocated, must free it */
                sal_free(ppc->driver_data);
            }
            sal_free(ppc);
            ppc = NULL;
        }
    }



    if (SOC_SUCCESS(rv)) {
        /* Set SOC related restriction/configuration/flags first */ 
        PHY_FLAGS_CLR(unit, port, PHY_FLAGS_SGMII_AUTONEG);
        if (soc_property_port_get(unit, port, spn_PHY_SGMII_AUTONEG, FALSE) &&
            soc_feature(unit, soc_feature_sgmii_autoneg)) {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_IS_TB(unit)){
                if (IS_GE_PORT(unit, port) && !IS_GMII_PORT(unit, port)){
                    /* for those GE port bounded with internal serdes */
                    if (IS_S_PORT(unit, port)){
                        soc_cm_debug(DK_PHY, 
                                "%s, port=%d(2.5G) have no Ext-PHY, "
                                "No SGMII_AUTONEG required!\n", 
                                FUNCTION_NAME(), port);
                    } else {
                        soc_cm_debug(DK_PHY,
                                "%s, port=%d, SGMII_AUTONEG flag set!!\n",
                                FUNCTION_NAME(), port);
                        PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
                    }
                }
            } else {
                PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
            }
#else   /* BCM_ROBO_SUPPORT */
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
#endif  /* BCM_ROBO_SUPPORT */
        }
    } else {
        if (NULL != EXT_PHY_SW_STATE(unit, port)) {
            sal_free(EXT_PHY_SW_STATE(unit, port));
            EXT_PHY_SW_STATE(unit, port) = NULL;
        }
 
        if (NULL != INT_PHY_SW_STATE(unit, port)) {
            phy_ctrl_t  *ppc = INT_PHY_SW_STATE(unit, port);
            if (ppc->driver_data) {
                /* If driver data allocated, must free it */
                sal_free(ppc->driver_data);
            }
            sal_free(ppc);
            ppc = NULL;
        }
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_init
 * Purpose:
 *      Initialize the PHY drivers attached to the port.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_init(int unit, soc_port_t port)
{
    int         rv;
    phy_ctrl_t *int_pc;
    phy_ctrl_t *ext_pc;

    int_pc = INT_PHY_SW_STATE(unit, port);
    ext_pc = EXT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

#ifdef BCM_SBX_SUPPORT
    if (SOC_IS_SBX(unit)) {
        /* Set Sandburst switch specific PHY flags */
    } else
#endif /* BCM_SBX_SUPPORT */
#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        /* Set ROBO switch specific PHY flags */
        
        if (SOC_IS_ROBODINO(unit)||SOC_IS_ROBO5395(unit) ||
                SOC_IS_ROBO5398(unit)||SOC_IS_ROBO5397(unit) ||
                SOC_IS_ROBO53115(unit)|| SOC_IS_ROBO53118(unit) ||
                SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
                SOC_IS_ROBO53128(unit)) {
                
            if (SOC_IS_ROBO53115(unit)){
                if (port == 5) {
                    /* Vulcan's port5 is built-in SerDes */
                    PHY_FLAGS_SET(unit, port, PHY_FLAGS_FORCED_SGMII);
                    PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
                } else {
                    /* set PHY_FLAGS_FORCED_COPPER for those ROBO chips with  
                     *  built-in GE PHY.
                     */
                    PHY_FLAGS_SET(unit, port, PHY_FLAGS_FORCED_COPPER);
                }
            } else {
                /* set PHY_FLAGS_FORCED_COPPER for those ROBO chips with 
                 *  built-in GE PHY.
                 */
                PHY_FLAGS_SET(unit, port, PHY_FLAGS_FORCED_COPPER);
            }
        }
        
        if (soc_feature(unit, soc_feature_sgmii_autoneg)) {
            if (SOC_IS_TB(unit)){
                if (IS_GE_PORT(unit, port) && !IS_GMII_PORT(unit, port)){
                    /* for those GE port bounded with internal serdes 
                       *
                       * P.S. TB's 2.5G need to double checked.
                       */
                    if (IS_S_PORT(unit, port)){
                        soc_cm_debug(DK_PHY, 
                                "%s, port=%d(2.5G) have no Ext-PHY, "
                                "No SGMII_AUTONEG required!\n", 
                                FUNCTION_NAME(), port);
                    } else {
                        soc_cm_debug(DK_PHY,
                                "%s, port=%d, SGMII_AUTONEG flag set!!\n",
                                FUNCTION_NAME(), port);
                        PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
                    }
                }
            } else {
                PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
            }
        }
    } else 
#endif /* BCM_ROBO_SUPPORT */
    {
#ifdef BCM_ESW_SUPPORT
        if (SOC_IS_DRACO(unit)) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_FORCED_SGMII);
        }
        if (SOC_IS_BRADLEY(unit)) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_SGMII_AUTONEG);
        }
        if (IS_LMD_ENABLED_PORT(unit, port)) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_SINGLE_LANE);
        }
#endif /* BCM_ESW_SUPPORT */
    }

    rv = SOC_E_NONE;
    /* arbiter lock */
    INT_MCU_LOCK(unit);
    if (NULL != ext_pc) {
        rv = (PHY_INIT(ext_pc->pd, unit, port));
        if (!SOC_SUCCESS(rv)) {
            soc_cm_debug(DK_ERR, "soc_phyctrl_probe: Init failed for"
                         " u=%d p=%d FAILED ", unit, port);
            return SOC_E_FAIL;
        }
    } 

    if (NULL != int_pc) {
        rv = (PHY_INIT(int_pc->pd, unit, port));
    }
    INT_MCU_UNLOCK(unit);

    PHY_FLAGS_SET(unit, port, PHY_FLAGS_INIT_DONE);

    PHYCTRL_DEBUG((DK_PHY | DK_VERBOSE, 
                 "soc_phyctrl_init: u=%d p=%d %s rv=%d\n",
                             unit, port, (ext_pc) ? "EXT" : "INT", rv));
    return rv;
}

int
soc_phyctrl_pd_get(int unit, soc_port_t port, phy_driver_t **pd)
{
    phy_ctrl_t *int_pc;
    phy_ctrl_t *ext_pc;

    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    if (NULL != ext_pc) {
        *pd = ext_pc->pd;
    } else {
        *pd = int_pc->pd;
    }

    return SOC_E_NONE;
}

int
soc_phyctrl_passthru_pd_get(int unit, soc_port_t port, phy_driver_t **pd)
{
    phy_ctrl_t   *int_pc;
    phy_ctrl_t   *ext_pc;

    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    if (PHY_PASSTHRU_MODE(unit, port)) {
        if (NULL != int_pc) {
            *pd = int_pc->pd;
        } else {
            *pd = ext_pc->pd;
        }
    } else {
        if (NULL != ext_pc) {
            *pd = ext_pc->pd;
        } else {
            *pd = int_pc->pd;
        }
    }
 
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phyctrl_reset
 * Purpose:
 *      Reset the PHY drivers attached to the port.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_reset(int unit, soc_port_t port, void *user_arg)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_RESET(pd, unit, port));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                      "soc_phyctrl_reset: u=%d p=%d rv=%d\n",
                       unit, port, rv));
    } 
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_drv_name
 * Purpose:
 *      Get pointer to driver name.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      name - driver name.
 * Returns:
 *      SOC_E_XXX
 */
char *
soc_phyctrl_drv_name(int unit, soc_port_t port)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    if (SOC_SUCCESS(rv)) {
        return pd->drv_name;
    }
    return NULL;
}


/*
 * Function:
 *      soc_phyctrl_link_get
 * Purpose:
 *      Read link status of the PHY driver.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      link - Link status
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_link_get(int unit, soc_port_t port, int *link)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(link);
    *link = FALSE;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_LINK_GET(pd, unit, port, link));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_link_get failed %d\n", rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_enable_set
 * Purpose:
 *      Enable/Disable the PHY driver attached to the port. 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      enable - enable/disable PHY driver
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_enable_set(int unit, soc_port_t port, int enable)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_ENABLE_SET(pd, unit, port, enable));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, 
                      "soc_phyctrl_enable_set: u=%d p=%d e=%d rv=%d\n",
                       unit, port, enable, rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_enable_get
 * Purpose:
 *      Get the enable/disable state of the PHY driver. 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      enable - Current enable/disable state.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_enable_get(int unit, soc_port_t port, int *enable)
{
    phy_driver_t *pd;
    int           rv;

    SOC_NULL_PARAM_CHECK(enable);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_ENABLE_GET(pd, unit, port, enable));
    }
    INT_MCU_UNLOCK(unit);
    
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_enable_get failed %d\n", rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_duplex_set
 * Purpose:
 *      Set duplex of the PHY.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      duplex -  (1) Full duplex
 *                (0) Half duplex
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_duplex_set(int unit, soc_port_t port, int duplex)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_DUPLEX_SET(pd, unit, port, duplex));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, 
                      "soc_phyctrl_duplex_set: u=%d p=%d d=%d rv=%d\n",
                      unit, port, duplex, rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_duplex_get
 * Purpose:
 *      Get current duplex setting of the PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      duplex -  (1) Full duplex
 *                (0) Half duplex
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_duplex_get(int unit, soc_port_t port, int *duplex)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_DUPLEX_GET(pd, unit, port, duplex));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, 
                      "soc_phyctrl_duplex_get:  u=%d p=%d rv=%d\n",
                      unit, port, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_speed_set
 * Purpose:
 *      Set PHY speed.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      speed - new speed of the PHY
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_speed_set(int unit, soc_port_t port, int speed)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_SPEED_SET(pd, unit, port, speed));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                      "soc_phyctrl_speed_set: u=%d p=%d s=%d rv=%d\n",
                      unit, port, speed, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_speed_get
 * Purpose:
 *      Read current PHY speed
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      speed - Current speed of PHY.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_speed_get(int unit, soc_port_t port, int *speed)
{
    int           rv;
    phy_driver_t *pd;

    SOC_NULL_PARAM_CHECK(speed);
    *speed = 0;

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_SPEED_GET(pd, unit, port, speed));
    }
    INT_MCU_UNLOCK(unit);
    
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_speed_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_master_set
 * Purpose:
 *      Ser Master/Slave configuration of the PHY.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      master - (1) master mode.
 *               (0) slave mode. 
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_master_set(int unit, soc_port_t port, int master)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MASTER_SET(pd, unit, port, master));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_PHY | DK_VERBOSE, 
                      "soc_phyctrl_master_set: u=%d p=%d m=%d rv=%d\n",
                      unit, port, master, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_master_get
 * Purpose:
 *      Read current Master/Slave setting
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      master - (1) master mode.
 *               (0) slave mode.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_master_get(int unit, soc_port_t port, int *master)
{
    int           rv;
    phy_driver_t *pd;

    SOC_NULL_PARAM_CHECK(master);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MASTER_GET(pd, unit, port, master));
    }
    INT_MCU_UNLOCK(unit);
    
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_master_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_auto_negotiate_set
 * Purpose:
 *      Enable/Disable autonegotiation of the PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      an   - new autoneg setting 
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_auto_negotiate_set(int unit, soc_port_t port, int an)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_AUTO_NEGOTIATE_SET(pd, unit, port, an));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                     "soc_phyctrl_auto_negotiate_set: u=%d p=%d an=%d rv=%d\n", 
                     unit, port, an, rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_auto_negotiate_get
 * Purpose:
 *      Get current auto neg setting
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      an   - current autoneg setting
 *      an_done - autoneg completed 
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_auto_negotiate_get(int unit, soc_port_t port, int *an, int *an_done)
{
    int           rv;
    phy_driver_t *pd;

    SOC_NULL_PARAM_CHECK(an);
    SOC_NULL_PARAM_CHECK(an_done);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);
    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_AUTO_NEGOTIATE_GET(pd, unit, port, an, an_done));
    }
    INT_MCU_UNLOCK(unit);
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_auto_negotiate_get failed %d\n", 
                       rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_adv_local_set
 * Purpose:
 *      Configure local advertising setting.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mode - Advertising mode
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_adv_local_set(int unit, soc_port_t port, soc_port_mode_t mode)
{
    int                   rv;
    phy_driver_t         *pd;
    soc_port_ability_t    ability;

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_ADV_SET(pd, unit, port, mode));
        if (SOC_E_UNAVAIL == rv) {
            rv = PHY_ABILITY_ADVERT_GET(pd, unit, port, &ability); /* Read the currently configured value. 
                                                                      So attributes are not available from *_PA_* */
            if (SOC_SUCCESS(rv)) {
                rv = soc_port_mode_to_ability(mode, &ability);
                if (SOC_SUCCESS(rv)) {
                    rv = (PHY_ABILITY_ADVERT_SET(pd, unit, port, &ability));
                }
            }
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_adv_local_set: u=%d p=%d "
                      "m=%x rv=%d\n", unit, port, mode, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_adv_local_get
 * Purpose:
 *      Get current local advertising setting
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mode - Current advertised mode.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_adv_local_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    int                   rv;
    phy_driver_t         *pd;
    soc_port_ability_t    ability;

    SOC_NULL_PARAM_CHECK(mode);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_ADV_GET(pd, unit, port, mode));
        if (rv == SOC_E_UNAVAIL) {
            rv = (PHY_ABILITY_ADVERT_GET(pd, unit, port, &ability));
            if (SOC_SUCCESS(rv)) {
                rv = soc_port_ability_to_mode(&ability, mode);
            }
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_adv_local_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_adv_remote_get
 * Purpose:
 *      Get link partner advertised mode
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mode - Link partner advertised mode
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_adv_remote_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    int                   rv;
    phy_driver_t         *pd;
    soc_port_ability_t    ability;

    SOC_NULL_PARAM_CHECK(mode);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);
    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_ADV_REMOTE_GET(pd, unit, port, mode));

        if (rv == SOC_E_UNAVAIL) {
            rv = (PHY_ABILITY_REMOTE_GET(pd, unit, port, &ability));
            if (SOC_SUCCESS(rv)) {
                rv = soc_port_ability_to_mode(&ability, mode);
            }
        }
    }
    INT_MCU_UNLOCK(unit);
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_adv_remote_get failed %d\n", rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_loopback_set
 * Purpose:
 *      Enable/disable loopback mode.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      enable - (1) Enable loopback mode.
 *               (0) Disable loopback mode.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_loopback_set(int unit, soc_port_t port, int enable)
{
    int           rv;
    phy_ctrl_t   *int_pc;
    phy_ctrl_t   *ext_pc;
    phy_driver_t *pd;

    rv     = SOC_E_NONE;
    int_pc = INT_PHY_SW_STATE(unit, port);
    ext_pc = EXT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    if (NULL != ext_pc) {
        pd = ext_pc->pd;
    } else {
        pd = int_pc->pd;
    }
    INT_MCU_LOCK(unit);
    rv = (PHY_LOOPBACK_SET(pd, unit, port, enable));

    /* Wait until link up if internal PHY is put into loopback */
    if (SOC_SUCCESS(rv) && enable && (NULL != int_pc)) { 
        if ((PHY_PASSTHRU_MODE(unit, port)) || (NULL == ext_pc)) {
            int           link;
            soc_timeout_t to;

            /* Wait up to 5000 msec for link up */
            soc_timeout_init(&to, 5000000, 0);
            link = 0;
            /*
             * Needs more than one read to clear Latched Link down bits.
             */
            rv = (PHY_LINK_GET(int_pc->pd, unit, port, &link)); 
            do {
                rv = (PHY_LINK_GET(int_pc->pd, unit, port, &link)); 
                if (link || SOC_FAILURE(rv)) {
                    break;
                }
            } while (!soc_timeout_check(&to));
            if (!link) {
                soc_cm_debug(DK_WARN,
                             "soc_phyctrl_loopback_set: u=%d p=%d TIMEOUT\n",
                             unit, port);
                rv = SOC_E_TIMEOUT;
            }
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, 
                      "soc_phyctrl_loopback_set: u=%d p=%d l=%d rv=%d\n",
                      unit, port, enable, rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_loopback_get
 * Purpose:
 *      Get current loopback setting
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      enable - (1) Enable loopback mode.
 *               (0) Disable loopback mode.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_loopback_get(int unit, soc_port_t port, int *enable)
{
    int           rv;
    phy_driver_t *pd;

    SOC_NULL_PARAM_CHECK(enable);

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_LOOPBACK_GET(pd, unit, port, enable));
    }
    INT_MCU_UNLOCK(unit);
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_loopback_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_interface_set
 * Purpose:
 *      Set the interface between MAC and PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      pif  - Interface type 
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_interface_set(int unit, soc_port_t port, soc_port_if_t pif)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_INTERFACE_SET(pd, unit, port, pif));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                      "soc_phyctrl_interface_set: u=%d p=%d i=%d rv=%d\n",
                      unit, port, pif, rv));
    }
    return rv; 
}

/*
 * Function:
 *      soc_phyctrl_interface_get
 * Purpose:
 *      Get current interface setting between MAC and PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      pif  - current interface setting
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_interface_get(int unit, soc_port_t port, soc_port_if_t *pif)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_INTERFACE_GET(pd, unit, port, pif));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_interface_get failed %d\n", rv));
    }
    return rv;
}

STATIC int
_soc_phy_ability_get(int unit, soc_port_t port, 
                         phy_driver_t *pd, soc_port_mode_t *mode)
{
    int                 rv;
    soc_port_ability_t  ability;

    INT_MCU_LOCK(unit);
    rv = (PHY_ABILITY_GET(pd, unit, port, mode));

    if (SOC_E_UNAVAIL == rv) {
        rv = (PHY_ABILITY_LOCAL_GET(pd, unit, port, &ability));
        if (SOC_SUCCESS(rv)) {
            rv = soc_port_ability_to_mode(&ability, mode);
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "_soc_phy_ability_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_ability_get
 * Purpose:
 *      Get PHY ability
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mode - PHY ability
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_ability_get(int unit, soc_port_t port, soc_port_mode_t *mode)
{
    int              rv;
    soc_port_mode_t  mode_speed_int;
    soc_port_mode_t  mode_speed_ext;
    phy_ctrl_t      *int_pc;
    phy_ctrl_t      *ext_pc;

    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);
    rv = SOC_E_NONE;

    mode_speed_int = mode_speed_ext = SOC_PM_SPEED_ALL;
    if (NULL != int_pc) {

        if (int_pc->speed_max > 16000) {
            soc_cm_debug(DK_ERR,
                    "soc_phyctrl_ability_get: Speed support above 16Gbps will"
                    "not work. Use soc_phyctrl_ability_local_get\n");
        }

        rv = _soc_phy_ability_get(unit, port, int_pc->pd, mode);
        mode_speed_int = *mode & SOC_PM_SPEED_ALL;
    }
    if (SOC_SUCCESS(rv) && NULL != ext_pc) {
        rv = _soc_phy_ability_get(unit, port, ext_pc->pd, mode);
        mode_speed_ext = *mode & SOC_PM_SPEED_ALL;
    }

    if (SOC_SUCCESS(rv)) {
        *mode &= ~(SOC_PM_SPEED_ALL);
        *mode |= (mode_speed_int & mode_speed_ext);
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_ability_get failed %d\n", rv));
    }
    soc_cm_debug(DK_PHY, "soc_phyctrl_ability_get E=%08x I=%08x C=%08x\n",
                        mode_speed_ext, mode_speed_int,*mode);
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_linkup_evt
 * Purpose:
 *      Force link up event to PHY.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_linkup_evt(int unit, soc_port_t port)
{
    int           rv;
    phy_driver_t *pd;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    if (SOC_SUCCESS(rv)) {
        rv = (PHY_LINKUP_EVT(pd, unit, port));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_linkdn_evt
 * Purpose:
 *      Force link down event to PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_linkdn_evt(int unit, soc_port_t port)
{
    int           rv;
    phy_driver_t *pd;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    if (SOC_SUCCESS(rv)) {
        rv = (PHY_LINKDN_EVT(pd, unit, port));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_mdix_set
 * Purpose:
 *      Set new mdix setting 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mdix - new mdix mode
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mdix)
{
    int           rv;
    phy_driver_t *pd=NULL;

    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MDIX_SET(pd, unit, port, mdix));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, 
                      "soc_phyctrl_mdix_set: u=%d p=%d m=%d rv=%d\n",
                      unit, port, mdix, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_mdix_get
 * Purpose:
 *      Get current mdix setting
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mdix - current mdix mode.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mdix)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(mdix);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MDIX_GET(pd, unit, port, mdix));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_mdix_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_mdix_status_get
 * Purpose:
 *      Current resolved mdix status.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      status - Current resolved mdix status.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_mdix_status_get(int unit, soc_port_t port,
                        soc_port_mdix_status_t *status)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(status);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MDIX_STATUS_GET(pd, unit, port, status));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_mdix_status_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_medium_config_set
 * Purpose:
 *      Set configuration of selected medium
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      medium - Selected medium
 *      cfg    - New configuration of the selected medium
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_medium_config_set(int unit, soc_port_t port, 
                          soc_port_medium_t medium,
                          soc_phy_config_t *cfg)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(cfg);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MEDIUM_CONFIG_SET(pd, unit, port, medium, cfg));
    }
    INT_MCU_UNLOCK(unit);
 
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                 "soc_phyctrl_medium_config_set: u=%d p=%d m=%d rv=%d\n",
                         unit, port, medium, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_medium_config_get
 * Purpose:
 *      Get current configuration of the selected medium
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      medium - Selected medium
 *      cfg    - Current configuration of the selected medium
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_medium_config_get(int unit, soc_port_t port, 
                          soc_port_medium_t medium,
                          soc_phy_config_t *cfg)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(cfg);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MEDIUM_CONFIG_GET(pd, unit, port, medium, cfg));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_medium_config_get failed %d\n", 
                       rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_medium_get
 * Purpose:
 *      Get active medium type
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      medium - active medium
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_medium_get(int unit, soc_port_t port, soc_port_medium_t *medium)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(medium);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_MEDIUM_GET(pd, unit, port, medium));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_medium_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_cable_diag
 * Purpose:
 *      Run cable diag on the PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      status - Cable status
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_cable_diag(int unit, soc_port_t port, soc_port_cable_diag_t *status)
{
    int         rv;
    phy_ctrl_t *ext_pc;

    SOC_NULL_PARAM_CHECK(status);

    rv     = SOC_E_UNAVAIL;
    ext_pc = EXT_PHY_SW_STATE(unit, port);

    if (NULL != ext_pc) {
        INT_MCU_LOCK(unit);
        rv = (PHY_CABLE_DIAG(ext_pc->pd, unit, port, status));
        INT_MCU_UNLOCK(unit);

        if (SOC_FAILURE(rv)) {
            PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_cable_diag failed %d\n", rv));
        }
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_link_change
 * Purpose:
 *      Force link change on the PHY
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      link - Link status to change
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_link_change(int unit, soc_port_t port, int *link)
{
    int           rv;
    phy_driver_t *pd=NULL;

    SOC_NULL_PARAM_CHECK(link);
    rv = soc_phyctrl_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_LINK_CHANGE(pd, unit, port, link));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_link_change failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_control_set
 * Purpose:
 *      Set PHY specific configuration
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      phy_ctrl - PHY control type to change
 *      value    - New setting for the PHY control
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_control_set(int unit, soc_port_t port, 
                    soc_phy_control_t phy_ctrl, uint32 value)
{

    int           rv;
    phy_driver_t *pd;

    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_CONTROL_SET(pd, unit, port, phy_ctrl, value));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN,
                 "soc_phyctrl_control_set: u=%d p=%d c=%d rv=%d\n",
                         unit, port, phy_ctrl, rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_control_get
 * Purpose:
 *      Get current setting of the PHY control
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      phy_ctrl - PHY control type to read 
 *      value    - Current setting for the PHY control
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_control_get(int unit, soc_port_t port, 
                    soc_phy_control_t phy_ctrl, uint32 *value)
{
    int           rv;
    phy_driver_t *pd;

    SOC_NULL_PARAM_CHECK(value);
    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = (PHY_CONTROL_GET(pd, unit, port, phy_ctrl, value));
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_control_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_reg_read
 * Purpose:
 *      Read PHY register
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      flags - Flags
 *      addr  - PHY register address
 *      data  - data read
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_reg_read(int unit, soc_port_t port, uint32 flags, 
                 uint32 addr, uint32 *data)
{
    int         rv;
    phy_ctrl_t *pc;

    SOC_NULL_PARAM_CHECK(data);

    rv = SOC_E_UNAVAIL;

    if (flags & SOC_PHY_INTERNAL) {
        pc = INT_PHY_SW_STATE(unit, port);
    } else {
        pc = EXT_PHY_SW_STATE(unit, port);
    }

    if (NULL != pc) {
        rv = PHY_REG_READ(pc->pd, unit, port, flags, addr, data);
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_reg_read failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_reg_write
 * Purpose:
 *      Write to PHY register
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      flags - Flags
 *      addr  - PHY register address
 *      data  - data to be written 
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_reg_write(int unit, soc_port_t port, uint32 flags, 
                  uint32 addr, uint32 data)
{
    int         rv;
    phy_ctrl_t *pc;

    rv = SOC_E_UNAVAIL;

    if (flags & SOC_PHY_INTERNAL) {
        pc = INT_PHY_SW_STATE(unit, port);
    } else {
        pc = EXT_PHY_SW_STATE(unit, port);
    }

    if (NULL != pc) {
        rv = PHY_REG_WRITE(pc->pd, unit, port, flags, addr, data);
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_reg_write failed %d\n", rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_reg_modify
 * Purpose:
 *      Modify PHY register
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      flags - Flags
 *      addr  - PHY register address
 *      data  - data to be written
 *      mask  - bit mask of data to be modified
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_reg_modify(int unit, soc_port_t port, uint32 flags, 
                   uint32 addr, uint32 data, uint32 mask)
{
    int         rv;
    phy_ctrl_t *pc;

    rv = SOC_E_UNAVAIL;

    if (flags & SOC_PHY_INTERNAL) {
        pc = INT_PHY_SW_STATE(unit, port);
    } else {
        pc = EXT_PHY_SW_STATE(unit, port);
    }

    if (NULL != pc) {
        INT_MCU_LOCK(unit);
        rv = PHY_REG_MODIFY(pc->pd, unit, port, flags, addr, data, mask);
        INT_MCU_UNLOCK(unit);
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_reg_modify failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_ability_advert_get
 * Purpose:
 *      Get local PHY advertised ability 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      ability - PHY ability
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_ability_advert_get(int unit, soc_port_t port, 
                            soc_port_ability_t * ability)
{
    int              rv;
    phy_driver_t    *pd;
    soc_port_mode_t  mode;

    SOC_NULL_PARAM_CHECK(ability);
    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = PHY_ABILITY_ADVERT_GET(pd, unit, port, ability);
    }

    if (SOC_E_UNAVAIL == rv) {
        rv = PHY_ADV_GET(pd, unit, port, &mode); 
        if (SOC_SUCCESS(rv)) {
            sal_memset(ability, 0, sizeof(*ability));
            rv = soc_port_mode_to_ability(mode, ability);
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_ability_advert_get failed %d\n", rv));
    }

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_ability_advert_set
 * Purpose:
 *      Set local PHY advertised ability 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      ability - PHY ability
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_ability_advert_set(int unit, soc_port_t port, 
                            soc_port_ability_t * ability)
{
    int              rv;
    phy_driver_t    *pd;
    soc_port_mode_t  mode;

    SOC_NULL_PARAM_CHECK(ability);
    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = PHY_ABILITY_ADVERT_SET(pd, unit, port, ability);
    }

    if (SOC_E_UNAVAIL == rv) {
        rv = soc_port_ability_to_mode(ability, &mode);
        if (SOC_SUCCESS(rv)) {
            rv = PHY_ADV_SET(pd, unit, port, mode); 
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_ability_advert_set failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_ability_remote_get
 * Purpose:
 *      Get remote PHY advertsied ability 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      ability - PHY ability
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_ability_remote_get(int unit, soc_port_t port, 
                                   soc_port_ability_t * ability)
{
    int              rv;
    phy_driver_t    *pd;
    soc_port_mode_t  mode;

    SOC_NULL_PARAM_CHECK(ability);
    rv = soc_phyctrl_passthru_pd_get(unit, port, &pd);

    INT_MCU_LOCK(unit);
    if (SOC_SUCCESS(rv)) {
        rv = PHY_ABILITY_REMOTE_GET(pd, unit, port, ability);
    }

    if (SOC_E_UNAVAIL == rv) {
        rv = PHY_ADV_REMOTE_GET(pd, unit, port, &mode); 
        if (SOC_SUCCESS(rv)) {
            sal_memset(ability, 0, sizeof(*ability));
            rv = soc_port_mode_to_ability(mode, ability);
        }
    }
    INT_MCU_UNLOCK(unit);

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_ability_remote_get failed %d\n", rv));
    }

    return rv;
}

STATIC int
_soc_phy_ability_local_get(int unit, soc_port_t port, 
                         phy_driver_t *pd, soc_port_ability_t *ability)
{
    int              rv;
    soc_port_mode_t  mode;

    INT_MCU_LOCK(unit);
    rv = PHY_ABILITY_LOCAL_GET(pd, unit, port, ability);

    if (SOC_E_UNAVAIL == rv) {
        rv = PHY_ABILITY_GET(pd, unit, port, &mode); 
        if (SOC_SUCCESS(rv)) {
            sal_memset(ability, 0, sizeof(*ability));
            rv = soc_port_mode_to_ability(mode, ability);
        }
    }
    INT_MCU_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      soc_phyctrl_ability_local_get
 * Purpose:
 *      Get PHY ability
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      mode - PHY ability
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_ability_local_get(int unit, soc_port_t port,
                             soc_port_ability_t *ability)
{
    int                 rv;
    soc_port_ability_t  ability_int;
    soc_port_ability_t  ability_ext;
    phy_ctrl_t         *int_pc;
    phy_ctrl_t         *ext_pc;

    SOC_NULL_PARAM_CHECK(ability);

    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    ability_int.speed_half_duplex = ability_ext.speed_half_duplex = SOC_PA_SPEED_ALL;
    ability_int.speed_full_duplex = ability_ext.speed_full_duplex = SOC_PA_SPEED_ALL; 

    rv = SOC_E_NONE;
    if (NULL != int_pc) {
        rv = _soc_phy_ability_local_get(unit, port, int_pc->pd, ability);
        ability_int.speed_full_duplex = ability->speed_full_duplex;
        ability_int.speed_half_duplex = ability->speed_half_duplex;
    }
    if (SOC_SUCCESS(rv) && NULL != ext_pc) {
        rv = _soc_phy_ability_local_get(unit, port, ext_pc->pd, ability);
        ability_ext.speed_full_duplex = ability->speed_full_duplex;
        ability_ext.speed_half_duplex = ability->speed_half_duplex;
    }

    if (SOC_SUCCESS(rv)) {
        ability->speed_half_duplex = ability_int.speed_half_duplex & ability_ext.speed_half_duplex;
        ability->speed_full_duplex = ability_int.speed_full_duplex & ability_ext.speed_full_duplex;
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_ability_get failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_firmware_set
 * Purpose:
 *      Update the phy device's firmware 
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      flags - Flags
 *      offset - offset to the data stream
 *      addr  - PHY register address
 *      data  - data to be written
 *      mask  - bit mask of data to be modified
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_firmware_set(int unit, soc_port_t port, uint32 flags,
                   int offset, uint8 *array, int len)
{
    int         rv;
    phy_ctrl_t *pc;
                                                                                
    rv = SOC_E_UNAVAIL;
                                                                                
    if (flags & SOC_PHY_INTERNAL) {
        pc = INT_PHY_SW_STATE(unit, port);
    } else {
        pc = EXT_PHY_SW_STATE(unit, port);
    }
                                                                                
    if (NULL != pc) {
        INT_MCU_LOCK(unit);
        rv = PHY_FIRMWARE_SET(pc->pd, unit, port, offset, array, len);
        INT_MCU_UNLOCK(unit);
    }
                                                                                
    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_firmware_set failed %d\n", rv));
    }
    return rv;
}

/*
 * Function:
 *      soc_phyctrl_primary_set
 * Purpose:
 *      Set primary port
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      primary  - primary port of the phy chip
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_primary_set(int unit, soc_port_t port, soc_port_t primary)
{
    soc_phy_chip_info_t *chip_info;
                                                                                

    if (SOC_PHY_INFO(unit, port).chip_info == NULL) {
        SOC_PHY_INFO(unit, port).chip_info = sal_alloc(sizeof(soc_phy_chip_info_t), 
                                             "phy_chip_info");
        if (SOC_PHY_INFO(unit, port).chip_info == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_PHY_INFO(unit, port).chip_info, -1, sizeof(soc_phy_chip_info_t));
    }

    chip_info = SOC_PHY_INFO(unit, port).chip_info;

    chip_info->primary = primary;
                                                                                
    return SOC_E_NONE;

}

/*
 * Function:
 *      soc_phyctrl_primary_get
 * Purpose:
 *      Get primary port
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      primary  - primary port of the phy chip
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_primary_get(int unit, soc_port_t port, soc_port_t *primary)
{
    soc_phy_chip_info_t *chip_info;
                                                                                
    chip_info = SOC_PHY_INFO(unit, port).chip_info;

    if ((chip_info == NULL) || (chip_info->primary == -1)) {
        return SOC_E_UNAVAIL;
    }
                                                                                
    *primary = chip_info->primary;

    return SOC_E_NONE;

}

/*
 * Function:
 *      soc_phyctrl_offset_set
 * Purpose:
 *      Set port offset
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      offset  - offset of the port
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_offset_set(int unit, soc_port_t port, int offset)
{
    soc_phy_chip_info_t *chip_info, *primary_chip_info;
                                                                                
    chip_info = SOC_PHY_INFO(unit, port).chip_info;

    if ((chip_info == NULL) || (chip_info->primary == -1) || (!PHY_OFFSET_VALID(offset))) {
        /* set primary first and make sure that the offset is valid */
        return SOC_E_UNAVAIL;
    }

    if (SOC_PHY_INFO(unit, chip_info->primary).chip_info == NULL) {
        SOC_PHY_INFO(unit, chip_info->primary).chip_info = sal_alloc(sizeof(soc_phy_chip_info_t), 
                                             "phy_chip_info");
        if (SOC_PHY_INFO(unit, chip_info->primary).chip_info == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_PHY_INFO(unit, chip_info->primary).chip_info, -1, sizeof(soc_phy_chip_info_t));
    }

    primary_chip_info = SOC_PHY_INFO(unit, chip_info->primary).chip_info;
    primary_chip_info->offset_to_port[offset] = port;
    chip_info->offset = offset;
                                                                                
    return SOC_E_NONE;

}

/*
 * Function:
 *      soc_phyctrl_offset_get
 * Purpose:
 *      Get port offset
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      offset  - offset of the port
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_offset_get(int unit, soc_port_t port, soc_port_t *offset)
{
    soc_phy_chip_info_t *chip_info;
                                                                                
    chip_info = SOC_PHY_INFO(unit, port).chip_info;

    if ((chip_info == NULL) || (!PHY_OFFSET_VALID(chip_info->offset))) {
        return SOC_E_UNAVAIL;
    }
                                                                                
    *offset = chip_info->offset;

    return SOC_E_NONE;

}

/*
 * Function:
 *      soc_phyctrl_toplvl_reg_read
 * Purpose:
 *      Read a top level register from a supporting chip
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      primary_port - Primary Port number.
 *      reg_offset  - Offset to the reg
 *      data  - Pointer to data returned
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_toplvl_reg_read(int unit, soc_port_t port, soc_port_t primary_port, 
                          uint8 reg_offset, uint16 *data)

{
    phy_ctrl_t    *pc, *pc_port3, *pc_port5;
    uint16 reg_data, status;
    soc_phy_chip_info_t *chip_info;
    int         rv = SOC_E_NONE;

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    if ((chip_info->offset_to_port[2] == -1) ||
        (chip_info->offset_to_port[4] == -1)) {

        return SOC_E_FAIL;
    }

    pc       = EXT_PHY_SW_STATE(unit, port);
    pc_port3 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[2]);
    pc_port5 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[4]);

    if ((!pc) || (!pc_port3) || (!pc_port5)) {
        return SOC_E_FAIL;
    }

    /* Write Reg address to Port 5's register 0x1C, shadow 0x0B */
    /* Status READ from Port 3's register 0x15 */

    /* Write Reg offset to Port 5's register 0x1C, shadow 0x0B */
    reg_data = (0xAC00 | reg_offset);
    INT_MCU_LOCK(unit);
    rv = pc->write(unit, pc_port5->phy_id, 0x1c, reg_data);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }

    /* Read data from Top level MII Status register(0x15h) */
    rv = pc->write(unit, pc_port3->phy_id, 0x17, 0x8F0B);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }
    rv = pc->read(unit, pc_port3->phy_id, 0x15, &status);
    INT_MCU_UNLOCK(unit);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    *data = (status & 0xff);

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phyctrl_toplvl_reg_write
 * Purpose:
 *      Write to a top level register from a supporting chip
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 *      primary_port - Primary Port number.
 *      reg_offset  - Offset to the reg
 *      data  - Data to be written
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_toplvl_reg_write(int unit, soc_port_t port, soc_port_t primary_port,
                           uint8 reg_offset, uint16 data)

{
    phy_ctrl_t    *pc, *pc_port3, *pc_port5, *pc_port6;
    uint16 reg_data;
#ifdef BROADCOM_DEBUG
    uint16 status;
#endif
    soc_phy_chip_info_t *chip_info;
    int         rv = SOC_E_NONE;

    if ((primary_port == -1) || (SOC_PHY_INFO(unit, primary_port).chip_info == NULL)) {
        return SOC_E_FAIL;
    }

    chip_info = SOC_PHY_INFO(unit, primary_port).chip_info;

    if ((chip_info->offset_to_port[2] == -1) ||
        (chip_info->offset_to_port[4] == -1) ||
        (chip_info->offset_to_port[5] == -1)) {

        return SOC_E_FAIL;
    }

    pc       = EXT_PHY_SW_STATE(unit, port);
    pc_port3 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[2]);
    pc_port5 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[4]);
    pc_port6 = EXT_PHY_SW_STATE(unit, chip_info->offset_to_port[5]);

    if ((!pc) || (!pc_port3) || (!pc_port5) || (!pc_port6)) {
        return SOC_E_FAIL;
    }

    /* Write Reg address to Port 5's register 0x1C, shadow 0x0B */
    /* Write data to Port 6's register 0x1C, shadow 0x0c */
    /* Status READ from Port 3's register 0x15 */

    /* Write Data to port6, register 0x1C, shadow 0x0c */
    INT_MCU_LOCK(unit);
    reg_data = (0xB000 | (data & 0xff));
    rv = pc->write(unit, pc_port6->phy_id, 0x1c, reg_data);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }

    /* Write Reg address to Port 5's register 0x1C, shadow 0x0B */
    /* Enable Write ( Port 5's register 0x1C, shadow 0x0B) Bit 7 = 1 */
    reg_data = (0xAC80 | reg_offset);
    rv = pc->write(unit, pc_port5->phy_id, 0x1c, reg_data);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }

    /* Disable Write ( Port 5's register 0x1C, shadow 0x0B) Bit 7 = 0 */
    reg_data = (0xAC00 | reg_offset);
    rv = pc->write(unit, pc_port5->phy_id, 0x1c, reg_data);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }

#ifdef BROADCOM_DEBUG
    /* Read data from Top level MII Status register(0x15h) */
    rv = pc->write(unit, pc_port3->phy_id, 0x17, 0x8F0B);
    if (SOC_FAILURE(rv)) {
        INT_MCU_UNLOCK(unit);
        return rv;
    }
    rv = pc->read(unit, pc_port3->phy_id, 0x15, &status);

#endif

    INT_MCU_UNLOCK(unit);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phyctrl_detach
 * Purpose:
 *      Remove PHY driver.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_detach(int unit, soc_port_t port)
{
    phy_driver_t  *phyd;
    phy_ctrl_t    *ext_pc;
    phy_ctrl_t    *int_pc;
 
    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);

    if (NULL != int_pc) {
        /* indicate to re-initialize the port. Needed for flex port op on wc */
        int_pc->flags &= ~PHYCTRL_INIT_DONE;
    }

    SOC_IF_ERROR_RETURN(soc_phy_nocxn(unit, &phyd));

    if (NULL == ext_pc) {
        /* This function is used for Hot Swapping external PHY driver.
         * Therefore, always attach no connection driver to external PHY. 
         */
        ext_pc = sal_alloc (sizeof (phy_ctrl_t), phyd->drv_name);
        if (NULL == ext_pc) {
            return SOC_E_MEMORY;
        }
        
        sal_memset(ext_pc, 0, sizeof(phy_ctrl_t));
        ext_pc->unit                 = unit;
        ext_pc->port                 = port;
        ext_pc->phy_id               = PHY_ADDR(unit, port);        
        EXT_PHY_SW_STATE(unit, port) = ext_pc;
    }

    ext_pc->pd   = phyd;

    /* don't clear the init done flag in independent lane mode */
    if (PHY_INDEPENDENT_LANE_MODE(unit, port)) {
        PHY_FLAGS_CLR(unit,port,~PHY_FLAGS_INIT_DONE);
    } else {
        /* Clear the PHY configuration flags */
        PHY_FLAGS_CLR_ALL(unit, port);
    }
    return SOC_E_NONE;
}

#define PHYDEV_TYPE_MAX  20
#define MDIO_BUS_NUM_MAX  6

STATIC int
_soc_phyctrl_bcst_init(int unit, pbmp_t pbmp,char *dev_name,int bus_num,int ctrl,int ext_bus)
{
    bcm_port_t port;
    phy_ctrl_t *pc;

    PBMP_ITER(pbmp, port) {
        pc = ext_bus? EXT_PHY_SW_STATE(unit, port):
                      INT_PHY_SW_STATE(unit, port);
        if (pc == NULL) {
            continue;
        }

        if (((pc->dev_name != NULL) && 
             (sal_strcmp(pc->dev_name, dev_name) == 0)) &&   /* match device name*/
            (PHY_ID_BUS_NUM(pc->phy_id) == bus_num) &&  /* match bus num*/
            (pc->flags & PHYCTRL_MDIO_BCST)) {
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_firmware_set(unit,port,ext_bus?0:SOC_PHY_INTERNAL,
                              ctrl,NULL,0));
             if (ctrl == PHYCTRL_UCODE_BCST_uC_SETUP ||
                 ctrl == PHYCTRL_UCODE_BCST_LOAD) {
                 break;
             }
             if (ctrl == PHYCTRL_UCODE_BCST_END) {
                 pc->flags |= PHYCTRL_UCODE_BCST_DONE;
             }
        }
    }
    return SOC_E_NONE;
}

STATIC int
_soc_phyctrl_ucode_bcst(int unit, pbmp_t pbmp, int ext_bus)
{
    bcm_port_t port;
    int dev_types;
    int i;
    int j;
    int bcst_num;
    phy_ctrl_t *pc;
    int rv = SOC_E_NONE;
    char *dev_name[PHYDEV_TYPE_MAX];

    /* first check if bcst is requested */
    bcst_num = 0;
    PBMP_ITER(pbmp, port) {
        pc = ext_bus? EXT_PHY_SW_STATE(unit, port):
                      INT_PHY_SW_STATE(unit, port);
        if ((pc != NULL) && (pc->flags & PHYCTRL_MDIO_BCST)) {
            bcst_num++;
        }
    }

    /* if number doesn't seem valid, exit */
    if (bcst_num < 2) {
        return SOC_E_NONE;
    }

    for (i = 0; i < PHYDEV_TYPE_MAX; i++) {
        dev_name[i] = NULL;
    }

    /* find out all PHY device types */
    dev_types = 0;
    PBMP_ITER(pbmp, port) {
        pc = ext_bus? EXT_PHY_SW_STATE(unit, port):
                      INT_PHY_SW_STATE(unit, port);
        if (pc == NULL) {
            continue;
        }
        for (i = 0; i < dev_types; i++) {
            if ((pc->dev_name == NULL) ||
                (dev_name[i] == NULL) ||
                (sal_strcmp(pc->dev_name, dev_name[i]) == 0)) {
                break;
            }
        }
        if (i >= dev_types) {
            if (pc->dev_name) {
                dev_name[dev_types++] = pc->dev_name;

                /* exceed device name array limit */
                if (dev_types >= PHYDEV_TYPE_MAX) {
                    break;
                }
            }
        }
    }

    /* walk through each device type */
    for (i = 0; i < dev_types; i++) {

        /* walk through each MDIO bus */
        for (j = 0; j < MDIO_BUS_NUM_MAX; j++) {

            /* pass 1: setup bcst mode for same type devices on the
             * same MDIO bus. go through each device
             */
            rv = _soc_phyctrl_bcst_init(unit,pbmp,dev_name[i],j,
                                  PHYCTRL_UCODE_BCST_SETUP,ext_bus);

            /* pass 2: first encounterd device's function is called.
             * bcst setup  
             */
            if (rv == SOC_E_NONE) {
                rv = _soc_phyctrl_bcst_init(unit,pbmp,dev_name[i],j,
                                  PHYCTRL_UCODE_BCST_uC_SETUP,ext_bus);
            }

            /* pass 3: all device's functions are called.
             * 84740 do a reset to signal uC start of download.
             * However this reset clears bcst configuration register
             */
            if (rv == SOC_E_NONE) {
                rv = _soc_phyctrl_bcst_init(unit,pbmp,dev_name[i],j,
                                  PHYCTRL_UCODE_BCST_ENABLE,ext_bus);
            }

            /* pass 4: first encounterd device's function is called.
             * bcst data to MDIO bus.
             */
            if (rv == SOC_E_NONE) {
                rv = _soc_phyctrl_bcst_init(unit,pbmp,dev_name[i],j,
                                  PHYCTRL_UCODE_BCST_LOAD,ext_bus);
            }

            /* pass 5: all device's functions are called.
             * read and check status. At the end, the bcst mode
             * must be turned off
             */
            SOC_IF_ERROR_RETURN
                (_soc_phyctrl_bcst_init(unit,pbmp,dev_name[i],j,
                                  PHYCTRL_UCODE_BCST_END,ext_bus));
        }  /* mdio bus loop */
    }  /* device type loop */

    return SOC_E_NONE;
}

STATIC int
soc_phyctrl_mdio_ucode_bcst(int unit, pbmp_t pbmp)
{
    int rv = SOC_E_NONE;
    
    /* do external PHY device first */
    rv = _soc_phyctrl_ucode_bcst(unit,pbmp,TRUE);

    /* do internal serdes */
    /* fow now skip the internal serdes */
    return rv;
}

int
soc_phyctrl_pbm_probe_init(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int rv = SOC_E_NONE;
    bcm_port_t port;
    phy_ctrl_t *int_pc;
    phy_ctrl_t *ext_pc;

    PBMP_ITER(pbmp, port) {
        soc_cm_debug(DK_PORT | DK_VERBOSE, "Init port %d PHY...\n", port);

        if ((rv = soc_phyctrl_probe(unit, port)) < 0) {
            soc_cm_debug(DK_WARN,
                     "Unit %d Port %s: Failed to probe PHY: %s\n",
                     unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
            break;
        }

        ext_pc = EXT_PHY_SW_STATE(unit, port);
        int_pc = INT_PHY_SW_STATE(unit, port);

        if (ext_pc) {
            PHYCTRL_INIT_STATE_SET(ext_pc,PHYCTRL_INIT_STATE_PASS1);
        }
        if (int_pc) {
            PHYCTRL_INIT_STATE_SET(int_pc,PHYCTRL_INIT_STATE_PASS1);
        }

        /* do PHY init pass1 */
        if ((rv = soc_phyctrl_init(unit, port)) < 0) {
            soc_cm_debug(DK_WARN,
                     "Unit %d Port %s: Failed to initialize PHY: %s\n",
                     unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv));
            break;
        }

        SOC_PBMP_PORT_ADD(*okay_pbmp, port);
    }

    /* check/perform PHY ucode broadcast if requested on valid pbmp */
    (void)soc_phyctrl_mdio_ucode_bcst(unit,*okay_pbmp);

    /* do PHY init pass2 if requested */
    PBMP_ITER(*okay_pbmp, port) {
        ext_pc = EXT_PHY_SW_STATE(unit, port);
        int_pc = INT_PHY_SW_STATE(unit, port);

        if (ext_pc) {
            if (PHYCTRL_INIT_STATE(ext_pc) == PHYCTRL_INIT_STATE_PASS2) {
                rv = (PHY_INIT(ext_pc->pd, unit, port));
            }
            PHYCTRL_INIT_STATE_SET(ext_pc,PHYCTRL_INIT_STATE_DEFAULT);
        }
        if (int_pc) {
            if (PHYCTRL_INIT_STATE(int_pc) == PHYCTRL_INIT_STATE_PASS2) {
                rv = (PHY_INIT(int_pc->pd, unit, port));
            }
            PHYCTRL_INIT_STATE_SET(int_pc,PHYCTRL_INIT_STATE_DEFAULT);
        }
    }

    return rv;
}

#ifdef PHYCTRL_DEBUG_PRINT
STATIC char * 
soc_phyctrl_event_string(soc_phy_event_t event)
{
     static char *phy_event[] = PHY_EVENT_STRING;

     assert((sizeof(phy_event) / sizeof(phy_event[0])) == phyEventCount);

     if (event >= phyEventCount) {
         return "Unknown Event";
     }

     return phy_event[event];
}
#endif /* PHYCTRL_DEBUG_PRINT */

/*
 * Function:
 *      soc_phyctrl_notify
 * Purpose:
 *      Notify events to internal PHY driver.
 * Parameters:
 *      unit - SOC Unit #.
 *      port - Port number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_phyctrl_notify(int unit, soc_port_t port,
                   soc_phy_event_t event, uint32 data)
{
    int         rv;
    phy_ctrl_t *int_pc;
    phy_ctrl_t *ext_pc;

    /* check events target to the internal serdes in all conditions */
    if (event == phyEventTxFifoReset) {
        int_pc = INT_PHY_SW_STATE(unit, port);
        if (NULL == int_pc) {
            return SOC_E_INIT;
        }
        rv = (PHY_NOTIFY(int_pc->pd, unit, port, event, data));
        return rv;
    }

    ext_pc = EXT_PHY_SW_STATE(unit, port);
    int_pc = INT_PHY_SW_STATE(unit, port);
    SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    rv     = SOC_E_NONE;

    if ((NULL != ext_pc) && (NULL != int_pc)) {
        if (!PHY_SGMII_AUTONEG_MODE(unit, port)) {
            rv = (PHY_NOTIFY(int_pc->pd, unit, port, event, data));

            PHYCTRL_DEBUG((DK_PHY | DK_VERBOSE, 
                           "u=%d p=%d event=%s data=0x%08x\n",
                           unit, port, soc_phyctrl_event_string(event), data));
        }
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_DEBUG((DK_WARN, "soc_phyctrl_notify failed %d\n", rv));
    }
 
    return rv;
}


STATIC void 
_soc_phyctrl_dump(phy_ctrl_t *pc)
{
    static char * if_string[] = {"NOCXN", "NULL",
                                 "MII", "GMII",
                                 "SGMII", "TBI",
                                 "XGMII", "RGMII",
                                 "RvMII", "1000X"};
                                      
    soc_cm_print("%s\n", pc->pd->drv_name);
    soc_cm_print("port         %d\n", pc->port);
    soc_cm_print("phy_id0      0x%04x\n", pc->phy_id0);
    soc_cm_print("phy_id1      0x%04x\n", pc->phy_id1);
    soc_cm_print("phy_model    0x%04x\n", pc->phy_model);
    soc_cm_print("phy_rev      0x%04x\n", pc->phy_rev);
    soc_cm_print("phy_oui      0x%04x\n", pc->phy_oui);
    soc_cm_print("phy_id       0x%02x\n", pc->phy_id);
    soc_cm_print("ledmode      0x%02x, 0x%02x, 0x%02x, 0x%02x\n", 
                               pc->ledmode[0], pc->ledmode[1],
                               pc->ledmode[2], pc->ledmode[3]);
    soc_cm_print("ledctrl      0x%04x\n", pc->ledctrl);
    soc_cm_print("ledselect    0x%04x\n", pc->ledselect);
    soc_cm_print("automedium   %s\n", pc->automedium ? "Y" : "N");
    soc_cm_print("tbi_capable  %s\n", pc->tbi_capable ? "Y" : "N");
    soc_cm_print("medium       %x\n", pc->medium);
    soc_cm_print("fiber_detect %d\n", pc->fiber_detect);
    soc_cm_print("halfout      %d\n", pc->halfout);
    soc_cm_print("interface    %s\n", if_string[pc->interface]);
}

STATIC void
_soc_phyinfo_dump(int unit, soc_port_t port) 
{
    soc_cm_print("phy_id0      0x%04x\n", PHY_ID0_REG(unit, port));
    soc_cm_print("phy_id1      0x%04x\n", PHY_ID1_REG(unit, port));
    soc_cm_print("phy_addr     0x%02x\n", PHY_ADDR(unit, port));
    soc_cm_print("phy_addr_int 0x%02x\n", PHY_ADDR_INT(unit, port));
    soc_cm_print("phy_name     %s\n", PHY_NAME(unit, port));
    soc_cm_print("phy_flags    %s%s%s%s%s%s\n",
                  PHY_COPPER_MODE(unit, port) ?  "COPPER\t" : "",
                  PHY_FIBER_MODE(unit, port) ?  "FIBER\t" : "",
                  PHY_PASSTHRU_MODE(unit, port) ?  "PASSTHRU\t" : "",
                  PHY_TBI_MODE(unit, port) ? "TBI\t" : "",
                  PHY_FIBER_100FX_MODE(unit, port) ? "100FX\t" : "",
                  PHY_SGMII_AUTONEG_MODE(unit, port) ?  "SGMII_AN\t" : "");
    soc_cm_print("phy_flags    %s%s%s%s%s%s\n", 
                  PHY_WAN_MODE(unit, port) ? "WAN\t" : "",
                  PHY_EXTERNAL_MODE(unit, port) ? "EXTERNAL\t" : "",
                  PHY_MEDIUM_CHANGED(unit, port) ?  "MEDIUM_CHANGED\t" : "",
                  PHY_SERDES_FIBER_MODE(unit, port) ?  "SERDES_FIBER\t" : "",
                  PHY_FORCED_SGMII_MODE(unit, port) ? "FORCED_SGMII\t" : "",
                  PHY_FORCED_COPPER_MODE(unit, port) ? "FORCED_COPPER\t" : "");
    soc_cm_print("phy_flags    %s%s\n", 
                  PHY_CLAUSE45_MODE(unit, port) ? "C45\t" : "",
                  PHY_DISABLED_MODE(unit, port) ? "DISABLED" : "");
    soc_cm_print("an_timeout   %d\n", PHY_AN_TIMEOUT(unit, port));
}

void
soc_phyctrl_port_dump(int unit, soc_port_t port)
{
    phy_ctrl_t *pc;

    if (NULL == phy_port_info[unit]) {
        soc_cm_print("----------------------\n");
        soc_cm_print("PHY SW not initialized\n");
        soc_cm_print("----------------------\n");
    } else {
        _soc_phyinfo_dump(unit, port);
        pc = INT_PHY_SW_STATE(unit, port);
        if (NULL != pc) {
            soc_cm_print("--------------------\n");
            soc_cm_print("Internal PHY Control\n");
            soc_cm_print("--------------------\n");
            _soc_phyctrl_dump(pc);
        }
    
        pc = EXT_PHY_SW_STATE(unit, port);
        if (NULL != pc) {
            soc_cm_print("--------------------\n");
            soc_cm_print("External PHY Control\n");
            soc_cm_print("--------------------\n");
            _soc_phyctrl_dump(pc);
        }
    }
} 
    

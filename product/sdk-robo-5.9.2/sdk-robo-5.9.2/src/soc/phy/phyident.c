/*
 * $Id: phyident.c,v 1.303.4.8 Broadcom SDK $
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
 * File:        phyident.c
 * Purpose:     These routines and structures are related to
 *              figuring out phy identification and correlating
 *              addresses to drivers
 */

#include <sal/types.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/phyreg.h>

#include <soc/phy.h>
#include <soc/phyctrl.h>
#include <soc/phy/phyctrl.h>
#include <soc/phy/drv.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif

#include "phydefs.h"  /* Must include before other phy related includes */
#include "phyident.h"
#include "phyaddr.h"
#include "physr.h"

#define _MAX_PHYS       128

typedef struct {
    uint16 int_addr;
    uint16 ext_addr;
} port_phy_addr_t;

/* Per port phy address map. */
static port_phy_addr_t *port_phy_addr[SOC_MAX_NUM_DEVICES];

static soc_phy_table_t  *phy_table[_MAX_PHYS];
static int              _phys_in_table = -1;

static int _chk_phy(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                    uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);

static int _chk_null(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                     uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);

static int _chk_sfp_phy(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                    uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);

#if defined(INCLUDE_PHY_SIMUL)
static int _chk_simul(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                      uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_SIMUL */

#if defined(INCLUDE_PHY_5690)
static int _chk_fiber5690(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_5690 */

#if defined(INCLUDE_XGXS_QSGMII65)
static int _chk_qsgmii53314(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_XGXS_QSGMII65 */

#if defined(INCLUDE_PHY_54680)
static int _chk_qgphy_5332x(int unit, soc_port_t port, 
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_54680 */

#if defined(INCLUDE_PHY_56XXX)
static int _chk_fiber56xxx(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_PHY_56XXX)
static int _chk_fiber56xxx_5601x(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_SERDES_COMBO)
static int _chk_serdes_combo_5601x(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_SERDES_COMBO */

#if defined(INCLUDE_PHY_XGXS6)
static int _chk_unicore(int unit, soc_port_t port,
                        soc_phy_table_t *my_entry,
                        uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_XGXS6 */

#if defined(INCLUDE_SERDES_ASSUMED)
static int _chk_serdes(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                       uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_SERDES_ASSUMED */

#if defined(INCLUDE_PHY_8706)
static int _chk_8706(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                     uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_8706 */

#if defined(INCLUDE_PHY_8072)
static int _chk_8072(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                     uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_8072 */

#if defined(INCLUDE_PHY_8040)
static int _chk_8040(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                     uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_8040 */

#if defined(INCLUDE_PHY_8481)
static int _chk_8481(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                     uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_8481 */

#if defined(INCLUDE_SERDES_COMBO65)
static int
_chk_serdescombo65(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);

#endif /* INCLUDE_SERDES_COMBO65 */

#if defined(INCLUDE_XGXS_16G)
static int
_chk_xgxs16g1l(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_XGXS_16G */

#if defined(INCLUDE_PHY_53XXX)
static int _chk_fiber53xxx(int unit, soc_port_t port,
                          soc_phy_table_t *my_entry,
                          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);
#endif /* INCLUDE_PHY_53XXX */

static int _chk_default(int unit, soc_port_t port, soc_phy_table_t *my_entry,
                        uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi);

static soc_known_phy_t
    _phy_ident_type_get(uint16 phy_id0, uint16 phy_id1);

static soc_phy_table_t _null_phy_entry =
    {_chk_null, _phy_id_NULL,   "Null",    &phy_null,          NULL};

#if defined(INCLUDE_PHY_SIMUL)
static soc_phy_table_t _simul_phy_entry =
    {_chk_simul, _phy_id_SIMUL, "Simulation", &phy_simul,      NULL};
#endif /* INCLUDE_PHY_SIMUL */

#if defined(INCLUDE_PHY_5690)
static soc_phy_table_t _fiber5690_phy_entry =
    {_chk_fiber5690, _phy_id_NULL, "Internal SERDES", &phy_5690drv_ge, NULL };
#endif /* INCLUDE_PHY_5690 */

#if defined(INCLUDE_PHY_56XXX)
static soc_phy_table_t _fiber56xxx_phy_entry =
    {_chk_fiber56xxx, _phy_id_NULL, "Internal SERDES", &phy_56xxxdrv_ge, NULL };
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_PHY_53XXX)
static soc_phy_table_t _fiber53xxx_phy_entry =
    {_chk_fiber53xxx, _phy_id_NULL, "Internal  SERDES", &phy_53xxxdrv_ge, NULL};
#endif /* INCLUDE_PHY_53XXX */

static soc_phy_table_t _default_phy_entry =
    {_chk_default, _phy_id_NULL, "Unknown", &phy_drv_fe, NULL};

/*
 * Variable:
 *      _standard_phy_table
 * Purpose:
 *      Defines the standard supported Broadcom PHYs, and the corresponding
 *      driver.
 */

static soc_phy_table_t _standard_phy_table[] = {

#ifdef INCLUDE_PHY_522X
    {_chk_phy, _phy_id_BCM5218, "BCM5218",     &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5220, "BCM5220/21",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5226, "BCM5226",     &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5228, "BCM5228",     &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5238, "BCM5238",     &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5248, "BCM5248",     &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5324, "BCM5324/FE",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM5348, "BCM5348/FE",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM53242, "BCM53242/FE",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM53262, "BCM53262/FE",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM53101, "BCM53101/FE",  &phy_522xdrv_fe, NULL},
    {_chk_phy, _phy_id_BCM53280, "BCM53280/FE",  &phy_522xdrv_fe, NULL},
#endif /* INCLUDE_PHY_522X */

    {_chk_phy, _phy_id_BCM5400, "BCM5400",     &phy_drv_ge, NULL},

#ifdef INCLUDE_PHY_54XX
    {_chk_phy, _phy_id_BCM5401, "BCM5401",     &phy_5401drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5402, "BCM5402",     &phy_5402drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5404, "BCM5404",     &phy_5404drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5424, "BCM5424/34",  &phy_5424drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5411, "BCM5411",     &phy_5411drv_ge, NULL},
#endif /* INCLUDE_PHY_54XX */
#if defined(INCLUDE_PHY_5464_ROBO)
    {_chk_phy, _phy_id_BCM5461, "BCM5461",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5464, "BCM5464",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5466, "BCM5466",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5478, "BCM5478",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5488, "BCM5488",     &phy_5464robodrv_ge, NULL},
#endif /* INCLUDE_PHY_5464_ROBO */

#if defined(INCLUDE_PHY_5482_ROBO)
    {_chk_phy, _phy_id_BCM5482, "BCM5482",     &phy_5482robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5481, "BCM5481",     &phy_5482robodrv_ge, NULL},
#endif /* INCLUDE_PHY_5482_ROBO */

#if defined(INCLUDE_PHY_5464_ESW)
    {_chk_phy, _phy_id_BCM5461, "BCM5461",     &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5464, "BCM5464",     &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5466, "BCM5466",     &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5478, "BCM5478",     &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM5488, "BCM5488",     &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54980, "BCM54980",   &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54980C, "BCM54980",  &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54980V, "BCM54980",  &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54980VC, "BCM54980", &phy_5464drv_ge, NULL},
    {_chk_phy, _phy_id_BCM53314, "BCM53314",   &phy_5464drv_ge, NULL},
#endif /* INCLUDE_PHY_5464 */

#if defined(INCLUDE_PHY_5464_ROBO)
    {_chk_phy, _phy_id_BCM5398, "BCM5398",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM5395, "BCM5395",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM53115, "BCM53115",     &phy_5464robodrv_ge, NULL},
    {_chk_phy, _phy_id_BCM53118, "BCM53118",     &phy_5464robodrv_ge, NULL},
#endif /* BCM_ROBO_SUPPORT && INCLUDE_PHY_5464 */

#if defined(INCLUDE_PHY_5482_ESW)
    {_chk_phy, _phy_id_BCM5482, "BCM5482/801x",     &phy_5482drv_ge, NULL},
#endif /* INCLUDE_PHY_5482 */

#if defined(INCLUDE_PHY_54684_ESW)
    {_chk_phy, _phy_id_BCM54684, "BCM54684", &phy_54684drv_ge, NULL},
#endif /* defined(INCLUDE_PHY_54684_ESW) */

#if defined(INCLUDE_PHY_54640_ESW)
    {_chk_phy, _phy_id_BCM54640, "BCM54640", &phy_54640drv_ge, NULL},
#endif /* defined(INCLUDE_PHY_54640_ESW) */

#if defined(INCLUDE_PHY_54682)
    {_chk_phy, _phy_id_BCM54682, "BCM54682E", &phy_54682drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54684E, "BCM54684E", &phy_54682drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54685, "BCM54685", &phy_54682drv_ge, NULL},
#endif /* defined(INCLUDE_PHY_54682) */

#ifdef INCLUDE_PHY_54616
    {_chk_phy, _phy_id_BCM54616, "BCM54616", &phy_54616drv_ge, NULL},
#endif /* INCLUDE_PHY_54616 */

#ifdef INCLUDE_MACSEC
#if defined(INCLUDE_PHY_54580)
    {_chk_phy, _phy_id_BCM54584, "BCM54584", &phy_54580drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54580, "BCM54580", &phy_54580drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54540, "BCM54540", &phy_54580drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54585, "BCM54584", &phy_54580drv_ge, NULL},
#endif /* defined(INCLUDE_PHY_54580) */
#ifdef INCLUDE_PHY_8729
    {_chk_phy, _phy_id_BCM8729, "BCM8729",  &phy_8729drv_gexe,  NULL},
#endif /* INCLUDE_PHY_8729 */
#endif  /* INCLUDE_MACSEC */

#ifdef INCLUDE_PHY_5421S
    {_chk_phy, _phy_id_BCM5421, "BCM5421S",    &phy_5421Sdrv_ge, NULL},
#endif /* INCLUDE_PHY_5421S */

#ifdef INCLUDE_PHY_54680
    {_chk_phy, _phy_id_BCM54680, "BCM54680", &phy_54680drv_ge, NULL},
    {_chk_qgphy_5332x, _phy_id_BCM53324, "BCM53324", &phy_54680drv_ge, NULL},
    {_chk_phy, _phy_id_BCM53125, "BCM53125", &phy_54680drv_ge, NULL},
    {_chk_phy, _phy_id_BCM53128, "BCM53128", &phy_54680drv_ge, NULL},
#endif /* INCLUDE_PHY_54680 */

#ifdef INCLUDE_PHY_54880
    {_chk_phy, _phy_id_BCM54880, "BCM54880", &phy_54880drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54881, "BCM54881", &phy_54880drv_ge, NULL},
    {_chk_phy, _phy_id_BCM54810, "BCM54810", &phy_54880drv_ge, NULL},
#endif /* INCLUDE_PHY_54880 */

#ifdef INCLUDE_PHY_54640E
    {_chk_phy, _phy_id_BCM54680E, "BCM54640E", &phy_54640drv_ge, NULL},
#endif /* INCLUDE_PHY_54640E */

#ifdef INCLUDE_PHY_54880E
    {_chk_phy, _phy_id_BCM54880E, "BCM54880E", &phy_54680drv_ge, NULL},
#endif /* INCLUDE_PHY_54880E */

#ifdef INCLUDE_PHY_54680E
    {_chk_phy, _phy_id_BCM54680E, "BCM54680E", &phy_54680drv_ge, NULL},
#endif /* INCLUDE_PHY_54680E */

#ifdef INCLUDE_PHY_52681E
    {_chk_phy, _phy_id_BCM52681E, "BCM52681E", &phy_54680drv_ge, NULL},
#endif /* INCLUDE_PHY_52681E */

#ifdef INCLUDE_PHY_8703
    {_chk_phy, _phy_id_BCM8703, "BCM8703",  &phy_8703drv_xe,  NULL},
    {_chk_phy, _phy_id_BCM8704, "BCM8704",  &phy_8703drv_xe,  NULL},
#endif /* INCLUDE_PHY_8703 */
#ifdef INCLUDE_PHY_8705
    {_chk_phy, _phy_id_BCM8705, "BCM8705/24/25",  &phy_8705drv_xe,  NULL},
#endif /* INCLUDE_PHY_8705 */
#if defined(INCLUDE_PHY_8706)
    /* BCM8706_A0 and BCM8705 has the same device ID. Therefore, the probe must
     * check for 8706 before 8705 to correctly attach BCM8706. For 8706,
     * phy_8706 config must be set.
     */
    {_chk_8706, _phy_id_BCM8706, "BCM8706/8726", &phy_8706drv_xe, NULL},
    {_chk_8706, _phy_id_BCM8727, "BCM8727", &phy_8706drv_xe, NULL},
    {_chk_8706, _phy_id_BCM8747, "BCM8728/8747", &phy_8706drv_xe, NULL},
#endif /* INCLUDE_PHY_8706 */
#if defined(INCLUDE_PHY_8072)
    {_chk_8072, _phy_id_BCM8072, "BCM8072", &phy_8072drv_xe, NULL},
    {_chk_8072, _phy_id_BCM8073, "BCM8073", &phy_8072drv_xe, NULL},
    {_chk_8072, _phy_id_BCM8074, "BCM8074", &phy_8074drv_xe, NULL},
#endif /* INCLUDE_PHY_8072 */

#if defined(INCLUDE_PHY_8040)
    {_chk_8040, _phy_id_BCM8040, "BCM8040", &phy_8040drv_xe, NULL},
#endif /* INCLUDE_PHY_8040 */

#if defined(INCLUDE_PHY_8481)
    {_chk_8481, _phy_id_BCM8481x, "BCM8481X", &phy_8481drv_xe, NULL},
    {_chk_8481, _phy_id_BCM84812ce, "BCM84812", &phy_8481drv_xe, NULL},
    {_chk_8481, _phy_id_BCM84821, "BCM84821", &phy_8481drv_xe, NULL},
    {_chk_8481, _phy_id_BCM84822, "BCM84822", &phy_8481drv_xe, NULL},
    {_chk_8481, _phy_id_BCM84823, "BCM84823", &phy_8481drv_xe, NULL},
#endif /* INCLUDE_PHY_8481 */
#ifdef INCLUDE_PHY_8750
    {_chk_phy, _phy_id_BCM8750, "BCM8750",  &phy_8750drv_xe,  NULL},
    {_chk_phy, _phy_id_BCM8752, "BCM8752",  &phy_8750drv_xe,  NULL},
    {_chk_phy, _phy_id_BCM8754, "BCM8754",  &phy_8750drv_xe,  NULL},
#endif /* INCLUDE_PHY_8750 */
#ifdef INCLUDE_PHY_84740
    {_chk_phy, _phy_id_BCM84740, "BCM84740",  &phy_84740drv_xe,  NULL},
#endif /* INCLUDE_PHY_84740 */

    {_chk_sfp_phy,0xffff,"copper sfp",&phy_copper_sfp_drv,NULL},
};

/* Internal PHY table */
static soc_phy_table_t _int_phy_table[] = {
#if defined(INCLUDE_SERDES_ASSUMED)
    {_chk_serdes, _phy_id_NULL, "Assumed SERDES", &phy_serdesassumed_ge, NULL},
#endif /* INCLUDE_SERDES_ASSUMED */

#ifdef BCM_DRACO_SUPPORT
    {_chk_fiber5690, _phy_id_NULL, "Internal SERDES", &phy_5690drv_ge,
     NULL },
#endif /* BCM_DRACO_SUPPORT */

#if defined(INCLUDE_PHY_56XXX)
    {_chk_fiber56xxx, _phy_id_NULL, "Internal SERDES",
     &phy_56xxxdrv_ge, NULL},
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_PHY_53XXX)
    {_chk_fiber53xxx, _phy_id_NULL, "Internal SERDES",
     &phy_53xxxdrv_ge, NULL},
#endif /* INCLUDE_PHY_53XXX */

#if defined(INCLUDE_SERDES_COMBO)
     {_chk_phy, _phy_id_SERDESCOMBO, "COMBO", &phy_serdescombo_ge, NULL},
#endif /* INCLUDE_SERDES_COMBO */

#if defined(INCLUDE_SERDES_100FX)
    {_chk_phy, _phy_id_SERDES100FX, "1000X/100FX", &phy_serdes100fx_ge, NULL},
#endif /* INCLUDE_SERDES_100FX */

#if defined(INCLUDE_SERDES_65LP)
    {_chk_phy, _phy_id_SERDES65LP, "65LP", &phy_serdes65lp_ge, NULL},
#endif /* INCLUDE_SERDES_65LP */

#ifdef INCLUDE_XGXS_QSGMII65
    {_chk_qsgmii53314, _phy_id_NULL, "QSGMII65", &phy_qsgmii65_ge,
     NULL },
#endif /* INCLUDE_XGXS_QSGMII65 */

#if defined(INCLUDE_SERDES_COMBO65)
    {_chk_serdescombo65, _phy_id_SERDESCOMBO65, "COMBO65", 
     &phy_serdescombo65_ge, NULL},
#endif /* INCLUDE_SERDES_COMBO65 */

#if defined(INCLUDE_PHY_XGXS1)
    {_chk_phy, _phy_id_BCMXGXS1, "XGXS1",      &phy_xgxs1_hg, NULL},
    {_chk_phy, _phy_id_BCMXGXS2, "XGXS2",      &phy_xgxs1_hg, NULL},
#endif /* INCLUDE_PHY_XGXS1 */

#if defined(INCLUDE_PHY_XGXS5)
    {_chk_phy, _phy_id_BCMXGXS5, "XGXS5",      &phy_xgxs5_hg, NULL},
#endif /* INCLUDE_PHY_XGXS5 */

#if defined(INCLUDE_PHY_XGXS6)
    {_chk_phy,     _phy_id_BCMXGXS6, "XGXS6",  &phy_xgxs6_hg, NULL},
    {_chk_unicore, _phy_id_BCMXGXS2, "XGXS6",  &phy_xgxs6_hg, NULL},
#endif /* INCLUDE_PHY_XGXS6 */

    /* Must probe for newer internal SerDes/XAUI first before probing for
     * older devices. Newer devices reuse the same device ID and introduce
     * a new mechanism to differentiate betwee devices. Therefore, newer
     * PHY drivers implement probe funtion to check for correct device.
     */
#if defined(INCLUDE_XGXS_WC40)
    {_chk_phy, _phy_id_XGXS_WC40, "WC40/4",    &phy_wc40_hg, NULL},
#endif /* INCLUDE_XGXS_HL65 */

#if defined(INCLUDE_XGXS_HL65)
    {_chk_phy, _phy_id_XGXS_HL65, "HL65/4",    &phy_hl65_hg, NULL},
#endif /* INCLUDE_XGXS_HL65 */

#if defined(INCLUDE_PHY_56XXX)
    {_chk_fiber56xxx_5601x, _phy_id_NULL, "Internal SERDES",
     &phy_56xxx_5601x_drv_ge, NULL},
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_SERDES_COMBO)
    {_chk_serdes_combo_5601x, _phy_id_NULL, "COMBO SERDES",
     &phy_serdescombo_5601x_ge, NULL},
#endif /* INCLUDE_SERDES_COMBO */

#if defined(INCLUDE_XGXS_16G)
    {_chk_phy, _phy_id_XGXS_16G, "XGXS16G",    &phy_xgxs16g_hg, NULL},
    {_chk_xgxs16g1l, _phy_id_XGXS_16G, "XGXS16G/1", &phy_xgxs16g1l_ge, NULL}
#endif /* INCLUDE_XGXS_16G */
};

/*
 * Check corrupted registers by writing zeroes
 * to block address register and making sure zeroes
 * are read back.
 */
STATIC INLINE int 
_is_corrupted_reg(int unit, uint8 phy_addr)
{
    int         rv;
    uint16      data;

    rv = soc_miim_write(unit, phy_addr, 0x1f, 0);
    if (rv != SOC_E_NONE) {
        return FALSE;
    }
    rv = soc_miim_read(unit, phy_addr, 0x1f, &data);
    if (rv != SOC_E_NONE) {
        return FALSE;
    }

    return (data != 0);
}

/*
 * Function:
 *      _init_phy_table(void)
 * Purpose:
 *      Initialize the phy table with known phys.
 * Parameters:
 *      None
 */

static void
_init_phy_table(void)
{
    uint32      i;

    for (i = 0; i < COUNTOF(_standard_phy_table) && i < _MAX_PHYS; i++) {
        phy_table[i] = &_standard_phy_table[i];
    }

    _phys_in_table = i;
}

#if defined(BCM_ENDURO_SUPPORT)
static void
_enduro_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_enduro_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x00, /* Port  1 (N/A)                */
        0x81, /* Port  2 (8SERDES_0)    IntBus=0 Addr=0x01 */
        0x82, /* Port  3 (8SERDES_0)    IntBus=0 Addr=0x02 */
        0x83, /* Port  4 (8SERDES_0)    IntBus=0 Addr=0x03 */
        0x84, /* Port  5 (8SERDES_0)    IntBus=0 Addr=0x04 */
        0x85, /* Port  6 (8SERDES_0)    IntBus=0 Addr=0x05 */
        0x86, /* Port  7 (8SERDES_0)    IntBus=0 Addr=0x06 */
        0x87, /* Port  8 (8SERDES_0)    IntBus=0 Addr=0x07 */
        0x88, /* Port  9 (8SERDES_0)    IntBus=0 Addr=0x08 */
        0x89, /* Port 10 (8SERDES_1)    IntBus=0 Addr=0x09 */
        0x8a, /* Port 11 (8SERDES_1)    IntBus=0 Addr=0x0a */
        0x8b, /* Port 12 (8SERDES_1)    IntBus=0 Addr=0x0b */
        0x8c, /* Port 13 (8SERDES_1)    IntBus=0 Addr=0x0c */
        0x8d, /* Port 14 (8SERDES_1)    IntBus=0 Addr=0x0d */
        0x8e, /* Port 15 (8SERDES_1)    IntBus=0 Addr=0x0e */
        0x8f, /* Port 16 (8SERDES_1)    IntBus=0 Addr=0x0f */
        0x90, /* Port 17 (8SERDES_1)    IntBus=0 Addr=0x10 */
        0x91, /* Port 18 (9SERDES)      IntBus=0 Addr=0x11 */
        0x92, /* Port 19 (9SERDES)      IntBus=0 Addr=0x12 */
        0x93, /* Port 20 (9SERDES)      IntBus=0 Addr=0x13 */
        0x94, /* Port 21 (9SERDES)      IntBus=0 Addr=0x14 */
        0x95, /* Port 22 (9SERDES)      IntBus=0 Addr=0x15 */
        0x96, /* Port 23 (9SERDES)      IntBus=0 Addr=0x16 */
        0x97, /* Port 24 (9SERDES)      IntBus=0 Addr=0x17 */
        0x98, /* Port 25 (9SERDES)      IntBus=0 Addr=0x18 */
        0x99, /* Port 26 (HC0)          IntBus=0 Addr=0x19 */
        0x9a, /* Port 27 (HC1)          IntBus=0 Addr=0x1a */
        0x9b, /* Port 28 (HC2)          IntBus=0 Addr=0x1b */
        0x9c, /* Port 29 (HC3)          IntBus=0 Addr=0x1c */
    };

    static const uint16 _soc_phy_addr_bcm56334[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A)                    */
        0x01, /* Port  2 ( ge0) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge1) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge2) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge3) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge4) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge5) ExtBus=0 Addr=0x06 */
        0x07, /* Port  8 ( ge6) ExtBus=0 Addr=0x07 */
        0x08, /* Port  9 ( ge7) ExtBus=0 Addr=0x08 */
        0x09, /* Port 10 ( ge8) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 11 ( ge9) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 12 (ge10) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 13 (ge11) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 14 (ge12) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 15 (ge13) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 16 (ge14) ExtBus=0 Addr=0x0f */
        0x10, /* Port 17 (ge15) ExtBus=0 Addr=0x10 */
        0x11, /* Port 18 (ge16) ExtBus=0 Addr=0x11 */
        0x12, /* Port 19 (ge17) ExtBus=0 Addr=0x12 */
        0x13, /* Port 20 (ge18) ExtBus=0 Addr=0x13 */
        0x14, /* Port 21 (ge19) ExtBus=0 Addr=0x14 */
        0x15, /* Port 22 (ge20) ExtBus=0 Addr=0x15 */
        0x16, /* Port 23 (ge21) ExtBus=0 Addr=0x16 */
        0x17, /* Port 24 (ge22) ExtBus=0 Addr=0x17 */
        0x18, /* Port 25 (ge23) ExtBus=0 Addr=0x18 */
        0x19, /* Port 26 ( hg0) ExtBus=0 Addr=0x19 */
        0x1a, /* Port 27 ( hg1) ExtBus=0 Addr=0x1a */
        0x1b, /* Port 28 ( hg2) ExtBus=0 Addr=0x1b */
        0x1c, /* Port 29 ( hg3) ExtBus=0 Addr=0x1c */
    };

    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    *phy_addr_int = _soc_enduro_int_phy_addr[port];
    switch (dev_id) {
    case BCM56331_DEVICE_ID:
    case BCM56333_DEVICE_ID:
    case BCM56334_DEVICE_ID:
    case BCM56338_DEVICE_ID:
    case BCM56320_DEVICE_ID:
    case BCM56321_DEVICE_ID:
    case BCM56132_DEVICE_ID:
    case BCM56134_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56334[port];
        break;
    }
}
#endif /* BCM_ENDURO_SUPPORT */

#if defined(BCM_HURRICANE_SUPPORT)
static void
_hurricane_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_hurricane_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x00, /* Port  1 (N/A)                */
        0x81, /* Port  2 (8SERDES_0)    IntBus=0 Addr=0x01 */
        0x81, /* Port  3 (8SERDES_0)    IntBus=0 Addr=0x02 */
        0x81, /* Port  4 (8SERDES_0)    IntBus=0 Addr=0x03 */
        0x81, /* Port  5 (8SERDES_0)    IntBus=0 Addr=0x04 */
        0x81, /* Port  6 (8SERDES_0)    IntBus=0 Addr=0x05 */
        0x81, /* Port  7 (8SERDES_0)    IntBus=0 Addr=0x06 */
        0x81, /* Port  8 (8SERDES_0)    IntBus=0 Addr=0x07 */
        0x81, /* Port  9 (8SERDES_0)    IntBus=0 Addr=0x08 */
        0x89, /* Port 10 (8SERDES_1)    IntBus=0 Addr=0x09 */
        0x89, /* Port 11 (8SERDES_1)    IntBus=0 Addr=0x0a */
        0x89, /* Port 12 (8SERDES_1)    IntBus=0 Addr=0x0b */
        0x89, /* Port 13 (8SERDES_1)    IntBus=0 Addr=0x0c */
        0x89, /* Port 14 (8SERDES_1)    IntBus=0 Addr=0x0d */
        0x89, /* Port 15 (8SERDES_1)    IntBus=0 Addr=0x0e */
        0x89, /* Port 16 (8SERDES_1)    IntBus=0 Addr=0x0f */
        0x89, /* Port 17 (8SERDES_1)    IntBus=0 Addr=0x10 */
        0x91, /* Port 18 (9SERDES)      IntBus=0 Addr=0x11 */
        0x91, /* Port 19 (9SERDES)      IntBus=0 Addr=0x12 */
        0x91, /* Port 20 (9SERDES)      IntBus=0 Addr=0x13 */
        0x91, /* Port 21 (9SERDES)      IntBus=0 Addr=0x14 */
        0x91, /* Port 22 (9SERDES)      IntBus=0 Addr=0x15 */
        0x91, /* Port 23 (9SERDES)      IntBus=0 Addr=0x16 */
        0x91, /* Port 24 (9SERDES)      IntBus=0 Addr=0x17 */
        0x91, /* Port 25 (9SERDES)      IntBus=0 Addr=0x18 */
        0x99, /* Port 26 (HC0)          IntBus=0 Addr=0x19 */
        0x99, /* Port 27 (HC0)          IntBus=0 Addr=0x19 */
        0x9a, /* Port 28 (HC1)          IntBus=0 Addr=0x1a */
        0x9a, /* Port 29 (HC1)          IntBus=0 Addr=0x1a */
    };

    static const uint16 _soc_phy_addr_bcm56142[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A)                    */
        0x01, /* Port  2 ( ge0) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge1) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge2) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge3) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge4) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge5) ExtBus=0 Addr=0x06 */
        0x07, /* Port  8 ( ge6) ExtBus=0 Addr=0x07 */
        0x08, /* Port  9 ( ge7) ExtBus=0 Addr=0x08 */
        0x0a, /* Port 10 ( ge8) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 11 ( ge9) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 12 (ge10) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 13 (ge11) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 14 (ge12) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 15 (ge13) ExtBus=0 Addr=0x0f */
        0x10, /* Port 16 (ge14) ExtBus=0 Addr=0x10 */
        0x11, /* Port 17 (ge15) ExtBus=0 Addr=0x11 */
        0x13, /* Port 18 (ge16) ExtBus=0 Addr=0x13 */
        0x14, /* Port 19 (ge17) ExtBus=0 Addr=0x14 */
        0x15, /* Port 20 (ge18) ExtBus=0 Addr=0x15 */
        0x16, /* Port 21 (ge19) ExtBus=0 Addr=0x16 */
        0x17, /* Port 22 (ge20) ExtBus=0 Addr=0x17 */
        0x18, /* Port 23 (ge21) ExtBus=0 Addr=0x18 */
        0x19, /* Port 24 (ge22) ExtBus=0 Addr=0x19 */
        0x1a, /* Port 25 (ge23) ExtBus=0 Addr=0x1a */
        0x1c, /* Port 26 ( hg0) ExtBus=0/1 Addr=0x1c */
        0,    /* Port 27 ( hg1) */
        0x1d, /* Port 28 ( hg2) ExtBus=0/1 Addr=0x1d */
        0,    /* Port 29 ( hg3) */
    };

    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    *phy_addr_int = _soc_hurricane_int_phy_addr[port];
    switch (dev_id) {
    case BCM56142_DEVICE_ID:
    case BCM56143_DEVICE_ID:
    case BCM56144_DEVICE_ID:
    case BCM56146_DEVICE_ID:
    case BCM56147_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56142[port];
        break;
    }
    /* Todo: If port 26-29 is GE, then Extbus=0, else Extbus=1 */
}
#endif /* BCM_HURRICANE_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT) || \
    defined(BCM_VALKYRIE2_SUPPORT)
static void
_triumph2_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_triumph2_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x99, /* Port  1 (9SERDES)      IntBus=0 Addr=0x19 */
        0x81, /* Port  2 (8SERDES_0)    IntBus=0 Addr=0x01 */
        0x82, /* Port  3 (8SERDES_0)    IntBus=0 Addr=0x02 */
        0x83, /* Port  4 (8SERDES_0)    IntBus=0 Addr=0x03 */
        0x84, /* Port  5 (8SERDES_0)    IntBus=0 Addr=0x04 */
        0x85, /* Port  6 (8SERDES_0)    IntBus=0 Addr=0x05 */
        0x86, /* Port  7 (8SERDES_0)    IntBus=0 Addr=0x06 */
        0x87, /* Port  8 (8SERDES_0)    IntBus=0 Addr=0x07 */
        0x88, /* Port  9 (8SERDES_0)    IntBus=0 Addr=0x08 */
        0x89, /* Port 10 (8SERDES_1)    IntBus=0 Addr=0x09 */
        0x8a, /* Port 11 (8SERDES_1)    IntBus=0 Addr=0x0a */
        0x8b, /* Port 12 (8SERDES_1)    IntBus=0 Addr=0x0b */
        0x8c, /* Port 13 (8SERDES_1)    IntBus=0 Addr=0x0c */
        0x8d, /* Port 14 (8SERDES_1)    IntBus=0 Addr=0x0d */
        0x8e, /* Port 15 (8SERDES_1)    IntBus=0 Addr=0x0e */
        0x8f, /* Port 16 (8SERDES_1)    IntBus=0 Addr=0x0f */
        0x90, /* Port 17 (8SERDES_1)    IntBus=0 Addr=0x10 */
        0x91, /* Port 18 (9SERDES)      IntBus=0 Addr=0x11 */
        0x92, /* Port 19 (9SERDES)      IntBus=0 Addr=0x12 */
        0x93, /* Port 20 (9SERDES)      IntBus=0 Addr=0x13 */
        0x94, /* Port 21 (9SERDES)      IntBus=0 Addr=0x14 */
        0x95, /* Port 22 (9SERDES)      IntBus=0 Addr=0x15 */
        0x96, /* Port 23 (9SERDES)      IntBus=0 Addr=0x16 */
        0x97, /* Port 24 (9SERDES)      IntBus=0 Addr=0x17 */
        0x98, /* Port 25 (9SERDES)      IntBus=0 Addr=0x18 */
        0xd9, /* Port 26 (HC0)          IntBus=2 Addr=0x19 */
        0xda, /* Port 27 (HC1)          IntBus=2 Addr=0x1a */
        0xdb, /* Port 28 (HC2)          IntBus=2 Addr=0x1b */
        0xdc, /* Port 29 (HC3)          IntBus=2 Addr=0x1c */
        0xc1, /* Port 30 (HL0)          IntBus=2 Addr=0x01 */
        0xc2, /* Port 31 (HL0)          IntBus=2 Addr=0x02 */
        0xc3, /* Port 32 (HL0)          IntBus=2 Addr=0x03 */
        0xc4, /* Port 33 (HL0)          IntBus=2 Addr=0x04 */
        0xc5, /* Port 34 (HL1)          IntBus=2 Addr=0x05 */
        0xc6, /* Port 35 (HL1)          IntBus=2 Addr=0x06 */
        0xc7, /* Port 36 (HL1)          IntBus=2 Addr=0x07 */
        0xc8, /* Port 37 (HL1)          IntBus=2 Addr=0x08 */
        0xc9, /* Port 38 (HL2)          IntBus=2 Addr=0x09 */
        0xca, /* Port 39 (HL2)          IntBus=2 Addr=0x0a */
        0xcb, /* Port 40 (HL2)          IntBus=2 Addr=0x0b */
        0xcc, /* Port 41 (HL2)          IntBus=2 Addr=0x0c */
        0xcd, /* Port 42 (HL3)          IntBus=2 Addr=0x0d */
        0xce, /* Port 43 (HL3)          IntBus=2 Addr=0x0e */
        0xcf, /* Port 44 (HL3)          IntBus=2 Addr=0x0f */
        0xd0, /* Port 45 (HL3)          IntBus=2 Addr=0x10 */
        0xd1, /* Port 46 (HL4)          IntBus=2 Addr=0x11 */
        0xd2, /* Port 47 (HL4)          IntBus=2 Addr=0x12 */
        0xd3, /* Port 48 (HL4)          IntBus=2 Addr=0x13 */
        0xd4, /* Port 49 (HL4)          IntBus=2 Addr=0x14 */
        0xd5, /* Port 50 (HL5)          IntBus=2 Addr=0x15 */
        0xd6, /* Port 51 (HL5)          IntBus=2 Addr=0x16 */
        0xd7, /* Port 52 (HL5)          IntBus=2 Addr=0x17 */
        0xd8, /* Port 53 (HL5)          IntBus=2 Addr=0x18 */
    };

    static const uint16 _soc_phy_addr_bcm56630[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=0 Addr=0x19 */
        0x00, /* Port  2 ( N/A)                    */
        0x00, /* Port  3 ( N/A)                    */
        0x00, /* Port  4 ( N/A)                    */
        0x00, /* Port  5 ( N/A)                    */
        0x00, /* Port  6 ( N/A)                    */
        0x00, /* Port  7 ( N/A)                    */
        0x00, /* Port  8 ( N/A)                    */
        0x00, /* Port  9 ( N/A)                    */
        0x00, /* Port 10 ( N/A)                    */
        0x00, /* Port 11 ( N/A)                    */
        0x00, /* Port 12 ( N/A)                    */
        0x00, /* Port 13 ( N/A)                    */
        0x00, /* Port 14 ( N/A)                    */
        0x00, /* Port 15 ( N/A)                    */
        0x00, /* Port 16 ( N/A)                    */
        0x00, /* Port 17 ( N/A)                    */
        0x00, /* Port 18 ( N/A)                    */
        0x00, /* Port 19 ( N/A)                    */
        0x00, /* Port 20 ( N/A)                    */
        0x00, /* Port 21 ( N/A)                    */
        0x00, /* Port 22 ( N/A)                    */
        0x00, /* Port 23 ( N/A)                    */
        0x00, /* Port 24 ( N/A)                    */
        0x00, /* Port 25 ( N/A)                    */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x01, /* Port 30 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port 31 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port 32 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port 33 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port 34 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port 35 ( ge6) ExtBus=0 Addr=0x06 */
        0x07, /* Port 36 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port 37 ( ge8) ExtBus=0 Addr=0x08 */
        0x09, /* Port 38 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 39 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 40 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 41 (ge12) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 42 (ge13) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 43 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 44 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 45 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 46 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 47 (ge18) ExtBus=0 Addr=0x12 */
        0x13, /* Port 48 (ge19) ExtBus=0 Addr=0x13 */
        0x14, /* Port 49 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 50 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 51 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 52 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 53 (ge24) ExtBus=0 Addr=0x18 */
    };

static const uint16 _soc_phy_addr_bcm56634[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=0 Addr=0x19 */
        0x09, /* Port  2 ( ge1) ExtBus=0 Addr=0x09 */
        0x0a, /* Port  3 ( ge2) ExtBus=0 Addr=0x0a */
        0x0b, /* Port  4 ( ge3) ExtBus=0 Addr=0x0b */
        0x0c, /* Port  5 ( ge4) ExtBus=0 Addr=0x0c */
        0x0d, /* Port  6 ( ge5) ExtBus=0 Addr=0x0d */
        0x0e, /* Port  7 ( ge6) ExtBus=0 Addr=0x0e */
        0x0f, /* Port  8 ( ge7) ExtBus=0 Addr=0x0f */
        0x10, /* Port  9 ( ge8) ExtBus=0 Addr=0x10 */
        0x15, /* Port 10 ( ge9) ExtBus=0 Addr=0x15 */ 
        0x16, /* Port 11 (ge10) ExtBus=0 Addr=0x16 */ 
        0x17, /* Port 12 (ge11) ExtBus=0 Addr=0x17 */
        0x18, /* Port 13 (ge12) ExtBus=0 Addr=0x18 */ 
        0x21, /* Port 14 (ge13) ExtBus=1 Addr=0x01 */
        0x22, /* Port 15 (ge14) ExtBus=1 Addr=0x02 */
        0x23, /* Port 16 (ge15) ExtBus=1 Addr=0x03 */
        0x24, /* Port 17 (ge16) ExtBus=1 Addr=0x04 */
        0x29, /* Port 18 (ge17) ExtBus=1 Addr=0x09 */
        0x2a, /* Port 19 (ge18) ExtBus=1 Addr=0x0a */
        0x2b, /* Port 20 (ge19) ExtBus=1 Addr=0x0b */
        0x2c, /* Port 21 (ge20) ExtBus=1 Addr=0x0c */
        0x2d, /* Port 22 (ge21) ExtBus=1 Addr=0x0d */
        0x2e, /* Port 23 (ge22) ExtBus=1 Addr=0x0e */
        0x2f, /* Port 24 (ge23) ExtBus=1 Addr=0x0f */
        0x30, /* Port 25 (ge24) ExtBus=1 Addr=0x10 */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x01, /* Port 30 (ge25) ExtBus=0 Addr=0x01 */
        0x02, /* Port 31 (ge26) ExtBus=0 Addr=0x02 */
        0x03, /* Port 32 (ge27) ExtBus=0 Addr=0x03 */
        0x04, /* Port 33 (ge28) ExtBus=0 Addr=0x04 */
        0x05, /* Port 34 (ge29) ExtBus=0 Addr=0x05 */
        0x06, /* Port 35 (ge30) ExtBus=0 Addr=0x06 */
        0x07, /* Port 36 (ge31) ExtBus=0 Addr=0x07 */
        0x08, /* Port 37 (ge32) ExtBus=0 Addr=0x08 */
        0x11, /* Port 38 (ge33) ExtBus=0 Addr=0x11 */
        0x12, /* Port 39 (ge34) ExtBus=0 Addr=0x12 */
        0x13, /* Port 40 (ge35) ExtBus=0 Addr=0x13 */
        0x14, /* Port 41 (ge36) ExtBus=0 Addr=0x14 */
        0x25, /* Port 42 (ge37) ExtBus=1 Addr=0x05 */
        0x26, /* Port 43 (ge38) ExtBus=1 Addr=0x06 */
        0x27, /* Port 44 (ge39) ExtBus=1 Addr=0x07 */
        0x28, /* Port 45 (ge40) ExtBus=1 Addr=0x08 */
        0x31, /* Port 46 (ge41) ExtBus=1 Addr=0x11 */
        0x32, /* Port 47 (ge42) ExtBus=1 Addr=0x12 */
        0x33, /* Port 48 (ge43) ExtBus=1 Addr=0x13 */
        0x34, /* Port 49 (ge44) ExtBus=1 Addr=0x14 */
        0x35, /* Port 50 (ge45) ExtBus=1 Addr=0x15 */
        0x36, /* Port 51 (ge46) ExtBus=1 Addr=0x16 */
        0x37, /* Port 52 (ge47) ExtBus=1 Addr=0x17 */
        0x38, /* Port 53 (ge48) ExtBus=1 Addr=0x18 */
    };


    static const uint16 _soc_phy_addr_bcm56636[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x09, /* Port  2 ( ge1) ExtBus=0 Addr=0x09 */
        0x0a, /* Port  3 ( ge2) ExtBus=0 Addr=0x0a */
        0x0b, /* Port  4 ( ge3) ExtBus=0 Addr=0x0b */
        0x0c, /* Port  5 ( ge4) ExtBus=0 Addr=0x0c */
        0x0d, /* Port  6 ( ge5) ExtBus=0 Addr=0x0d */
        0x0e, /* Port  7 ( ge6) ExtBus=0 Addr=0x0e */
        0x0f, /* Port  8 ( ge7) ExtBus=0 Addr=0x0f */
        0x10, /* Port  9 ( ge8) ExtBus=0 Addr=0x10 */
        0x15, /* Port 10 ( ge9) ExtBus=0 Addr=0x15 */
        0x16, /* Port 11 (ge10) ExtBus=0 Addr=0x16 */
        0x17, /* Port 12 (ge11) ExtBus=0 Addr=0x17 */
        0x18, /* Port 13 (ge12) ExtBus=0 Addr=0x18 */
        0x00, /* Port 14 ( N/A)                    */
        0x00, /* Port 15 ( N/A)                    */
        0x00, /* Port 16 ( N/A)                    */
        0x00, /* Port 17 ( N/A)                    */
        0x00, /* Port 18 ( N/A)                    */
        0x00, /* Port 19 ( N/A)                    */
        0x00, /* Port 20 ( N/A)                    */
        0x00, /* Port 21 ( N/A)                    */
        0x00, /* Port 22 ( N/A)                    */
        0x00, /* Port 23 ( N/A)                    */
        0x00, /* Port 24 ( N/A)                    */
        0x00, /* Port 25 ( N/A)                    */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x01, /* Port 30 (ge25) ExtBus=0 Addr=0x01 */
        0x02, /* Port 31 (ge26) ExtBus=0 Addr=0x02 */
        0x03, /* Port 32 (ge27) ExtBus=0 Addr=0x03 */
        0x04, /* Port 33 (ge28) ExtBus=0 Addr=0x04 */
        0x05, /* Port 34 (ge29) ExtBus=0 Addr=0x05 */
        0x06, /* Port 35 (ge30) ExtBus=0 Addr=0x06 */
        0x07, /* Port 36 (ge31) ExtBus=0 Addr=0x07 */
        0x08, /* Port 37 (ge32) ExtBus=0 Addr=0x08 */
        0x11, /* Port 38 (ge33) ExtBus=0 Addr=0x11 */
        0x12, /* Port 39 (ge34) ExtBus=0 Addr=0x12 */
        0x13, /* Port 40 (ge35) ExtBus=0 Addr=0x13 */
        0x14, /* Port 41 (ge36) ExtBus=0 Addr=0x14 */
        0x45, /* Port 42 (ge37) ExtBus=2 Addr=0x05 */
        0x00, /* Port 43 ( N/A)                    */
        0x00, /* Port 44 ( N/A)                    */
        0x00, /* Port 45 ( N/A)                    */
        0x00, /* Port 46 ( N/A)                    */
        0x00, /* Port 47 ( N/A)                    */
        0x00, /* Port 48 ( N/A)                    */
        0x00, /* Port 49 ( N/A)                    */
        0x46, /* Port 50 (ge45) ExtBus=2 Addr=0x06 */
        0x00, /* Port 51 ( N/A)                    */
        0x00, /* Port 52 ( N/A)                    */
        0x00, /* Port 53 ( N/A)                    */
    };

    static const uint16 _soc_phy_addr_bcm56638[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x00, /* Port  2 ( N/A)                    */
        0x00, /* Port  3 ( N/A)                    */
        0x00, /* Port  4 ( N/A)                    */
        0x00, /* Port  5 ( N/A)                    */
        0x00, /* Port  6 ( N/A)                    */
        0x00, /* Port  7 ( N/A)                    */
        0x00, /* Port  8 ( N/A)                    */
        0x00, /* Port  9 ( N/A)                    */
        0x00, /* Port 10 ( N/A)                    */
        0x00, /* Port 11 ( N/A)                    */
        0x00, /* Port 12 ( N/A)                    */
        0x00, /* Port 13 ( N/A)                    */
        0x00, /* Port 14 ( N/A)                    */
        0x00, /* Port 15 ( N/A)                    */
        0x00, /* Port 16 ( N/A)                    */
        0x00, /* Port 17 ( N/A)                    */
        0x00, /* Port 18 ( N/A)                    */
        0x00, /* Port 19 ( N/A)                    */
        0x00, /* Port 20 ( N/A)                    */
        0x00, /* Port 21 ( N/A)                    */
        0x00, /* Port 22 ( N/A)                    */
        0x00, /* Port 23 ( N/A)                    */
        0x00, /* Port 24 ( N/A)                    */
        0x00, /* Port 25 ( N/A)                    */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x45, /* Port 30 ( xe0) ExtBus=2 Addr=0x05 */
        0x00, /* Port 31 ( N/A)                    */
        0x00, /* Port 32 ( N/A)                    */
        0x00, /* Port 33 ( N/A)                    */
        0x00, /* Port 34 ( N/A)                    */
        0x00, /* Port 35 ( N/A)                    */
        0x00, /* Port 36 ( N/A)                    */
        0x00, /* Port 37 ( N/A)                    */
        0x46, /* Port 38 ( xe1) ExtBus=2 Addr=0x06 */
        0x00, /* Port 39 ( N/A)                    */
        0x00, /* Port 40 ( N/A)                    */
        0x00, /* Port 41 ( N/A)                    */
        0x47, /* Port 42 ( xe2) ExtBus=2 Addr=0x07 */
        0x00, /* Port 43 ( N/A)                    */
        0x00, /* Port 44 ( N/A)                    */
        0x00, /* Port 45 ( N/A)                    */
        0x00, /* Port 46 ( N/A)                    */
        0x00, /* Port 47 ( N/A)                    */
        0x00, /* Port 48 ( N/A)                    */
        0x00, /* Port 49 ( N/A)                    */
        0x48, /* Port 50 ( xe3) ExtBus=2 Addr=0x08 */
        0x00, /* Port 51 ( N/A)                    */
        0x00, /* Port 52 ( N/A)                    */
        0x00, /* Port 53 ( N/A)                    */
    };

    static const uint16 _soc_phy_addr_bcm56639[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x1d, /* Port  1 ( ge0) ExtBus=0 Addr=0x1d */
        0x01, /* Port  2 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge6) ExtBus=0 Addr=0x06 */
        0x07, /* Port  8 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port  9 ( ge8) ExtBus=0 Addr=0x08 */
        0x09, /* Port 10 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 11 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 12 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 13 (ge12) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 14 (ge13) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 15 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 16 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 17 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 18 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 19 (ge18) ExtBus=0 Addr=0x12 */
        0x13, /* Port 20 (ge19) ExtBus=0 Addr=0x13 */
        0x14, /* Port 21 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 22 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 23 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 24 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 25 (ge24) ExtBus=0 Addr=0x18 */
        0x45, /* Port 26 ( hg0) ExtBus=2 Addr=0x05 */
        0x46, /* Port 27 ( hg1) ExtBus=2 Addr=0x06 */
        0x47, /* Port 28 ( hg2) ExtBus=2 Addr=0x07 */
        0x48, /* Port 29 ( hg3) ExtBus=2 Addr=0x08 */
        0x41, /* Port 30 ( xe0) ExtBus=2 Addr=0x01 */
        0x00, /* Port 31 ( N/A)                    */
        0x00, /* Port 32 ( N/A)                    */
        0x00, /* Port 33 ( N/A)                    */
        0x00, /* Port 34 ( N/A)                    */
        0x00, /* Port 35 ( N/A)                    */
        0x00, /* Port 36 ( N/A)                    */
        0x00, /* Port 37 ( N/A)                    */
        0x42, /* Port 38 ( xe1) ExtBus=2 Addr=0x02 */
        0x00, /* Port 39 ( N/A)                    */
        0x00, /* Port 40 ( N/A)                    */
        0x00, /* Port 41 ( N/A)                    */
        0x43, /* Port 42 ( xe2) ExtBus=2 Addr=0x03 */
        0x00, /* Port 43 ( N/A)                    */
        0x00, /* Port 44 ( N/A)                    */
        0x00, /* Port 45 ( N/A)                    */
        0x19, /* Port 46 (ge25) ExtBus=0 Addr=0x19 */
        0x1a, /* Port 47 (ge26) ExtBus=0 Addr=0x1a */
        0x1b, /* Port 48 (ge27) ExtBus=0 Addr=0x1b */
        0x1c, /* Port 49 (ge28) ExtBus=0 Addr=0x1c */
        0x44, /* Port 50 ( xe3) ExtBus=2 Addr=0x04 */
        0x00, /* Port 51 ( N/A)                    */
        0x00, /* Port 52 ( N/A)                    */
        0x00, /* Port 53 ( N/A)                    */
    };

    static const uint16 _soc_phy_addr_bcm56521[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A)                    */
        0x01, /* Port  2 ( ge0) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge1) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge2) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge3) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge4) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge5) ExtBus=0 Addr=0x06 */
        0x07, /* Port  8 ( ge6) ExtBus=0 Addr=0x07 */
        0x08, /* Port  9 ( ge7) ExtBus=0 Addr=0x08 */
        0x09, /* Port 10 ( ge8) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 11 ( ge9) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 12 (ge10) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 13 (ge11) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 14 (ge12) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 15 (ge13) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 16 (ge14) ExtBus=0 Addr=0x0f */
        0x10, /* Port 17 (ge15) ExtBus=0 Addr=0x10 */
        0x11, /* Port 18 (ge16) ExtBus=0 Addr=0x11 */
        0x12, /* Port 19 (ge17) ExtBus=0 Addr=0x12 */
        0x13, /* Port 20 (ge18) ExtBus=0 Addr=0x13 */
        0x14, /* Port 21 (ge19) ExtBus=0 Addr=0x14 */
        0x15, /* Port 22 (ge20) ExtBus=0 Addr=0x15 */
        0x16, /* Port 23 (ge21) ExtBus=0 Addr=0x16 */
        0x17, /* Port 24 (ge22) ExtBus=0 Addr=0x17 */
        0x18, /* Port 25 (ge23) ExtBus=0 Addr=0x18 */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x00, /* Port 30 ( N/A)                    */
        0x00, /* Port 31 ( N/A)                    */
        0x00, /* Port 32 ( N/A)                    */
        0x00, /* Port 33 ( N/A)                    */
        0x00, /* Port 34 ( N/A)                    */
        0x00, /* Port 35 ( N/A)                    */
        0x00, /* Port 36 ( N/A)                    */
        0x00, /* Port 37 ( N/A)                    */
        0x00, /* Port 38 ( N/A)                    */
        0x00, /* Port 39 ( N/A)                    */
        0x00, /* Port 40 ( N/A)                    */
        0x00, /* Port 41 ( N/A)                    */
        0x00, /* Port 42 ( N/A)                    */
        0x00, /* Port 43 ( N/A)                    */
        0x00, /* Port 44 ( N/A)                    */
        0x00, /* Port 45 ( N/A)                    */
        0x00, /* Port 46 ( N/A)                    */
        0x00, /* Port 47 ( N/A)                    */
        0x00, /* Port 48 ( N/A)                    */
        0x00, /* Port 49 ( N/A)                    */
        0x00, /* Port 50 ( N/A)                    */
        0x00, /* Port 51 ( N/A)                    */
        0x00, /* Port 52 ( N/A)                    */
        0x00, /* Port 53 ( N/A)                    */
    };

    static const uint16 _soc_phy_addr_bcm56526[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x1d, /* Port  1 ( ge0) ExtBus=0 Addr=0x19 */
        0x01, /* Port  2 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge6) ExtBus=0 Addr=0x06 */
        0x07, /* Port  8 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port  9 ( ge8) ExtBus=0 Addr=0x08 */
        0x0d, /* Port 10 (ge13) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 11 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 12 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 13 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 14 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 15 (ge18) ExtBus=0 Addr=0x12 */
        0x13, /* Port 16 (ge19) ExtBus=0 Addr=0x13 */
        0x14, /* Port 17 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 18 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 19 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 20 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 21 (ge24) ExtBus=0 Addr=0x18 */
        0x19, /* Port 22 (ge25) ExtBus=0 Addr=0x19 */
        0x1a, /* Port 23 (ge26) ExtBus=0 Addr=0x1a */
        0x1b, /* Port 24 (ge27) ExtBus=0 Addr=0x1b */
        0x1c, /* Port 25 (ge28) ExtBus=0 Addr=0x1c */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x00, /* Port 30 ( N/A)                    */
        0x00, /* Port 31 ( N/A)                    */
        0x00, /* Port 32 ( N/A)                    */
        0x00, /* Port 33 ( N/A)                    */
        0x00, /* Port 34 ( N/A)                    */
        0x00, /* Port 35 ( N/A)                    */
        0x00, /* Port 36 ( N/A)                    */
        0x00, /* Port 37 ( N/A)                    */
        0x09, /* Port 38 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 39 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 40 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 41 (ge12) ExtBus=0 Addr=0x0c */
        0x00, /* Port 42 ( N/A)                    */
        0x00, /* Port 43 ( N/A)                    */
        0x00, /* Port 44 ( N/A)                    */
        0x00, /* Port 45 ( N/A)                    */
        0x45, /* Port 46 ( xe0) ExtBus=2 Addr=0x05 */
        0x00, /* Port 47 ( N/A)                    */
        0x00, /* Port 48 ( N/A)                    */
        0x00, /* Port 49 ( N/A)                    */
        0x46, /* Port 50 ( xe1) ExtBus=2 Addr=0x06 */
        0x00, /* Port 51 ( N/A)                    */
        0x00, /* Port 52 ( N/A)                    */
        0x00, /* Port 53 ( N/A)                    */
    };

    static const uint16 _soc_phy_addr_bcm56685[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x39, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x00, /* Port  2 ( N/A)                    */
        0x00, /* Port  3 ( N/A)                    */
        0x00, /* Port  4 ( N/A)                    */
        0x00, /* Port  5 ( N/A)                    */
        0x00, /* Port  6 ( N/A)                    */
        0x00, /* Port  7 ( N/A)                    */
        0x00, /* Port  8 ( N/A)                    */
        0x00, /* Port  9 ( N/A)                    */
        0x00, /* Port 10 ( N/A)                    */
        0x00, /* Port 11 ( N/A)                    */
        0x00, /* Port 12 ( N/A)                    */
        0x00, /* Port 13 ( N/A)                    */
        0x00, /* Port 14 ( N/A)                    */
        0x00, /* Port 15 ( N/A)                    */
        0x00, /* Port 16 ( N/A)                    */
        0x00, /* Port 17 ( N/A)                    */
        0x00, /* Port 18 ( N/A)                    */
        0x00, /* Port 19 ( N/A)                    */
        0x00, /* Port 20 ( N/A)                    */
        0x00, /* Port 21 ( N/A)                    */
        0x00, /* Port 22 ( N/A)                    */
        0x00, /* Port 23 ( N/A)                    */
        0x00, /* Port 24 ( N/A)                    */
        0x00, /* Port 25 ( N/A)                    */
        0x41, /* Port 26 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 27 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 28 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 29 ( hg3) ExtBus=2 Addr=0x04 */
        0x01, /* Port 30 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port 31 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port 32 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port 33 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port 34 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port 35 ( ge6) ExtBus=0 Addr=0x06 */
        0x07, /* Port 36 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port 37 ( ge8) ExtBus=0 Addr=0x08 */
        0x09, /* Port 38 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 39 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 40 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 41 (ge12) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 42 (ge13) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 43 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 44 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 45 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 46 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 47 (ge18) ExtBus=0 Addr=0x12 */
        0x13, /* Port 48 (ge19) ExtBus=0 Addr=0x13 */
        0x14, /* Port 49 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 50 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 51 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 52 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 53 (ge24) ExtBus=0 Addr=0x18 */
    };

    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    *phy_addr_int = _soc_triumph2_int_phy_addr[port];
    switch (dev_id) {
    case BCM56630_DEVICE_ID:
    case BCM56520_DEVICE_ID:
    case BCM56522_DEVICE_ID:
    case BCM56524_DEVICE_ID:
    case BCM56534_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56630[port];
        break;
    case BCM56636_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56636[port];
        break;
    case BCM56638_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56638[port];
        break;
    case BCM56639_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56639[port];
        break;
    case BCM56521_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56521[port];
        break;
    case BCM56526_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56526[port];
        break;
    case BCM56685_DEVICE_ID:
    case BCM56689_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56685[port];
        break;
    case BCM56634_DEVICE_ID:
    case BCM56538_DEVICE_ID:
    default:
        *phy_addr = _soc_phy_addr_bcm56634[port];
        break;
    }
}
#endif /* BCM_TRIUMPH2_SUPPORT || BCM_APOLLO_SUPPORT || BCM_VALKYRIE2_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
static void
_triumph_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_triumph_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x99, /* Port  1 (serdes2:8)    IntBus=0 Addr=0x19 */
        0xc1, /* Port  2 (hyperlite0:0) IntBus=2 Addr=0x01 */
        0xc2, /* Port  3 (hyperlite0:1) IntBus=2 Addr=0x02 */
        0xc3, /* Port  4 (hyperlite0:2) IntBus=2 Addr=0x03 */
        0xc4, /* Port  5 (hyperlite0:3) IntBus=2 Addr=0x04 */
        0xc5, /* Port  6 (hyperlite1:0) IntBus=2 Addr=0x05 */
        0xc6, /* Port  7 (hyperlite1:1) IntBus=2 Addr=0x06 */
        0x81, /* Port  8 (serdes0:0)    IntBus=0 Addr=0x01 */
        0x82, /* Port  9 (serdes0:1)    IntBus=0 Addr=0x02 */
        0x83, /* Port 10 (serdes0:2)    IntBus=0 Addr=0x03 */
        0x84, /* Port 11 (serdes0:3)    IntBus=0 Addr=0x04 */
        0x85, /* Port 12 (serdes0:4)    IntBus=0 Addr=0x05 */
        0x86, /* Port 13 (serdes0:5)    IntBus=0 Addr=0x06 */
        0xc9, /* Port 14 (hyperlite2:0) IntBus=2 Addr=0x09 */
        0xca, /* Port 15 (hyperlite2:1) IntBus=2 Addr=0x0a */
        0xcb, /* Port 16 (hyperlite2:2) IntBus=2 Addr=0x0b */
        0xcc, /* Port 17 (hyperlite2:3) IntBus=2 Addr=0x0c */
        0xc7, /* Port 18 (hyperlite1:2) IntBus=2 Addr=0x07 */
        0xc8, /* Port 19 (hyperlite1:3) IntBus=2 Addr=0x08 */
        0x87, /* Port 20 (serdes0:6)    IntBus=0 Addr=0x07 */
        0x88, /* Port 21 (serdes0:7)    IntBus=0 Addr=0x08 */
        0x89, /* Port 22 (serdes1:0)    IntBus=0 Addr=0x09 */
        0x8a, /* Port 23 (serdes1:1)    IntBus=0 Addr=0x0a */
        0x8b, /* Port 24 (serdes1:2)    IntBus=0 Addr=0x0b */
        0x8c, /* Port 25 (serdes1:3)    IntBus=0 Addr=0x0c */
        0xcd, /* Port 26 (hyperlite3:0) IntBus=2 Addr=0x0d */
        0xd5, /* Port 27 (hyperlite5:0) IntBus=2 Addr=0x15 */
        0xd9, /* Port 28 (unicore0)     IntBus=2 Addr=0x19 */
        0xda, /* Port 29 (unicore1)     IntBus=2 Addr=0x1a */
        0xdb, /* Port 30 (unicore2)     IntBus=2 Addr=0x1b */
        0xdc, /* Port 31 (unicore3)     IntBus=2 Addr=0x1c */
        0xce, /* Port 32 (hyperlite3:1) IntBus=2 Addr=0x0e */
        0xcf, /* Port 33 (hyperlite3:2) IntBus=2 Addr=0x0f */
        0xd0, /* Port 34 (hyperlite3:3) IntBus=2 Addr=0x10 */
        0xd1, /* Port 35 (hyperlite4:0) IntBus=2 Addr=0x11 */
        0xd2, /* Port 36 (hyperlite4:1) IntBus=2 Addr=0x12 */
        0x8d, /* Port 37 (serdes1:4)    IntBus=0 Addr=0x0d */
        0x8e, /* Port 38 (serdes1:5)    IntBus=0 Addr=0x0e */
        0x8f, /* Port 39 (serdes1:6)    IntBus=0 Addr=0x0f */
        0x90, /* Port 40 (serdes1:7)    IntBus=0 Addr=0x10 */
        0x91, /* Port 41 (serdes2:0)    IntBus=0 Addr=0x11 */
        0x92, /* Port 42 (serdes2:1)    IntBus=0 Addr=0x12 */
        0xd6, /* Port 43 (hyperlite5:1) IntBus=2 Addr=0x16 */
        0xd7, /* Port 44 (hyperlite5:2) IntBus=2 Addr=0x17 */
        0xd8, /* Port 45 (hyperlite5:3) IntBus=2 Addr=0x18 */
        0xd3, /* Port 46 (hyperlite4:2) IntBus=2 Addr=0x13 */
        0xd4, /* Port 47 (hyperlite4:3) IntBus=2 Addr=0x14 */
        0x93, /* Port 48 (serdes2:2)    IntBus=0 Addr=0x13 */
        0x94, /* Port 49 (serdes2:3)    IntBus=0 Addr=0x14 */
        0x95, /* Port 50 (serdes2:4)    IntBus=0 Addr=0x15 */
        0x96, /* Port 51 (serdes2:5)    IntBus=0 Addr=0x16 */
        0x97, /* Port 52 (serdes2:6)    IntBus=0 Addr=0x17 */
        0x98, /* Port 53 (serdes2:7)    IntBus=0 Addr=0x18 */
    };

    static const uint16 _soc_phy_addr_bcm56624[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x01, /* Port  2 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge6) ExtBus=0 Addr=0x06 */
        0x09, /* Port  8 ( ge7) ExtBus=0 Addr=0x09 */
        0x0a, /* Port  9 ( ge8) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 10 ( ge9) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 11 (ge10) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 12 (ge11) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 13 (ge12) ExtBus=0 Addr=0x0e */
        0x11, /* Port 14 (ge13) ExtBus=0 Addr=0x11 */
        0x12, /* Port 15 (ge14) ExtBus=0 Addr=0x12 */
        0x13, /* Port 16 (ge15) ExtBus=0 Addr=0x13 */
        0x14, /* Port 17 (ge16) ExtBus=0 Addr=0x14 */
        0x07, /* Port 18 (ge17) ExtBus=0 Addr=0x07 */
        0x08, /* Port 19 (ge18) ExtBus=0 Addr=0x08 */
        0x0f, /* Port 20 (ge19) ExtBus=0 Addr=0x0f */
        0x10, /* Port 21 (ge20) ExtBus=0 Addr=0x10 */
        0x15, /* Port 22 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 23 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 24 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 25 (ge24) ExtBus=0 Addr=0x18 */
        0x25, /* Port 26 (ge25) ExtBus=1 Addr=0x05 */
        0x35, /* Port 27 (ge26) ExtBus=1 Addr=0x15 */
        0x41, /* Port 28 ( xg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( xg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( xg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( xg3) ExtBus=2 Addr=0x04 */
        0x26, /* Port 32 (ge27) ExtBus=1 Addr=0x06 */
        0x27, /* Port 33 (ge28) ExtBus=1 Addr=0x07 */
        0x28, /* Port 34 (ge29) ExtBus=1 Addr=0x08 */
        0x31, /* Port 35 (ge30) ExtBus=1 Addr=0x11 */
        0x32, /* Port 36 (ge31) ExtBus=1 Addr=0x12 */
        0x21, /* Port 37 (ge32) ExtBus=1 Addr=0x01 */
        0x22, /* Port 38 (ge33) ExtBus=1 Addr=0x02 */
        0x23, /* Port 39 (ge34) ExtBus=1 Addr=0x03 */
        0x24, /* Port 40 (ge35) ExtBus=1 Addr=0x04 */
        0x29, /* Port 41 (ge36) ExtBus=1 Addr=0x09 */
        0x2a, /* Port 42 (ge37) ExtBus=1 Addr=0x0a */
        0x36, /* Port 43 (ge38) ExtBus=1 Addr=0x16 */
        0x37, /* Port 44 (ge39) ExtBus=1 Addr=0x17 */
        0x38, /* Port 45 (ge40) ExtBus=1 Addr=0x18 */
        0x33, /* Port 46 (ge41) ExtBus=1 Addr=0x13 */
        0x34, /* Port 47 (ge42) ExtBus=1 Addr=0x14 */
        0x2b, /* Port 48 (ge43) ExtBus=1 Addr=0x0b */
        0x2c, /* Port 49 (ge44) ExtBus=1 Addr=0x0c */
        0x2d, /* Port 50 (ge45) ExtBus=1 Addr=0x0d */
        0x2e, /* Port 51 (ge46) ExtBus=1 Addr=0x0e */
        0x2f, /* Port 52 (ge47) ExtBus=1 Addr=0x0f */
        0x30, /* Port 53 (ge48) ExtBus=1 Addr=0x10 */
    };

    static const uint16 _soc_phy_addr_bcm56626[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x01, /* Port  2 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge6) ExtBus=0 Addr=0x06 */
        0x09, /* Port  8 ( ge7) ExtBus=0 Addr=0x09 */
        0x0a, /* Port  9 ( ge8) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 10 ( ge9) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 11 (ge10) ExtBus=0 Addr=0x0c */
        0x0d, /* Port 12 (ge11) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 13 (ge12) ExtBus=0 Addr=0x0e */
        0x11, /* Port 14 (ge13) ExtBus=0 Addr=0x11 */
        0x12, /* Port 15 (ge14) ExtBus=0 Addr=0x12 */
        0x13, /* Port 16 (ge15) ExtBus=0 Addr=0x13 */
        0x14, /* Port 17 (ge16) ExtBus=0 Addr=0x14 */
        0x07, /* Port 18 (ge17) ExtBus=0 Addr=0x07 */
        0x08, /* Port 19 (ge18) ExtBus=0 Addr=0x08 */
        0x0f, /* Port 20 (ge19) ExtBus=0 Addr=0x0f */
        0x10, /* Port 21 (ge20) ExtBus=0 Addr=0x10 */
        0x15, /* Port 22 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 23 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 24 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 25 (ge24) ExtBus=0 Addr=0x18 */
        0x45, /* Port 26 ( xe0) ExtBus=2 Addr=0x05 */
        0x46, /* Port 27 ( xe1) ExtBus=2 Addr=0x06 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
    };

    static const uint16 _soc_phy_addr_bcm56628[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=1 Addr=0x01 */
        0x45, /* Port  2 ( xe0) ExtBus=2 Addr=0x05 */
        0x00, /* Port  3 ( N/A) */
        0x00, /* Port  4 ( N/A) */
        0x00, /* Port  5 ( N/A) */
        0x00, /* Port  6 ( N/A) */
        0x00, /* Port  7 ( N/A) */
        0x00, /* Port  8 ( N/A) */
        0x00, /* Port  9 ( N/A) */
        0x00, /* Port 10 ( N/A) */
        0x00, /* Port 11 ( N/A) */
        0x00, /* Port 12 ( N/A) */
        0x00, /* Port 13 ( N/A) */
        0x46, /* Port 14 ( xe1) ExtBus=2 Addr=0x06 */
        0x00, /* Port 15 ( N/A) */
        0x00, /* Port 16 ( N/A) */
        0x00, /* Port 17 ( N/A) */
        0x00, /* Port 18 ( N/A) */
        0x00, /* Port 19 ( N/A) */
        0x00, /* Port 20 ( N/A) */
        0x00, /* Port 21 ( N/A) */
        0x00, /* Port 22 ( N/A) */
        0x00, /* Port 23 ( N/A) */
        0x00, /* Port 24 ( N/A) */
        0x00, /* Port 25 ( N/A) */
        0x47, /* Port 26 ( xe2) ExtBus=2 Addr=0x07 */
        0x48, /* Port 27 ( xe3) ExtBus=2 Addr=0x08 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
    };

    static const uint16 _soc_phy_addr_bcm56629[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x19, /* Port  1 ( ge0) ExtBus=0 Addr=0x19 */
        0x41, /* Port  2 ( xe0) ExtBus=2 Addr=0x01 */
        0x00, /* Port  3 ( N/A) */
        0x00, /* Port  4 ( N/A) */
        0x00, /* Port  5 ( N/A) */
        0x00, /* Port  6 ( N/A) */
        0x00, /* Port  7 ( N/A) */
        0x01, /* Port  8 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  9 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port 10 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port 11 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port 12 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port 13 ( ge6) ExtBus=0 Addr=0x06 */
        0x42, /* Port 14 ( xe1) ExtBus=2 Addr=0x02 */
        0x00, /* Port 15 ( N/A) */
        0x00, /* Port 16 ( N/A) */
        0x00, /* Port 17 ( N/A) */
        0x00, /* Port 18 ( N/A) */
        0x00, /* Port 19 ( N/A) */
        0x07, /* Port 20 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port 21 ( ge8) ExtBus=0 Addr=0x08 */
        0x09, /* Port 22 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 23 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 24 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 25 (ge12) ExtBus=0 Addr=0x0c */
        0x43, /* Port 26 ( xe2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 27 ( xe3) ExtBus=2 Addr=0x04 */
        0x45, /* Port 28 ( hg0) ExtBus=2 Addr=0x05 */
        0x46, /* Port 29 ( hg1) ExtBus=2 Addr=0x06 */
        0x47, /* Port 30 ( hg2) ExtBus=2 Addr=0x07 */
        0x48, /* Port 31 ( hg3) ExtBus=2 Addr=0x08 */
        0x00, /* Port 32 ( N/A) */
        0x00, /* Port 33 ( N/A) */
        0x00, /* Port 34 ( N/A) */
        0x00, /* Port 35 ( N/A) */
        0x00, /* Port 36 ( N/A) */
        0x0d, /* Port 37 (ge13) ExtBus=0 Addr=0x0d */
        0x0e, /* Port 38 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 39 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 40 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 41 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 42 (ge18) ExtBus=0 Addr=0x12 */
        0x00, /* Port 43 ( N/A) */
        0x00, /* Port 44 ( N/A) */
        0x00, /* Port 45 ( N/A) */
        0x00, /* Port 46 ( N/A) */
        0x00, /* Port 47 ( N/A) */
        0x13, /* Port 48 (ge19) ExtBus=0 Addr=0x13 */
        0x14, /* Port 49 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 50 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 51 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 52 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 53 (ge24) ExtBus=0 Addr=0x18 */
    };

    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    *phy_addr_int = _soc_triumph_int_phy_addr[port];
    switch (dev_id) {
    case BCM56626_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56626[port];
        break;
    case BCM56628_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56628[port];
        break;
    case BCM56629_DEVICE_ID:
        if (soc_feature(unit, soc_feature_xgport_one_xe_six_ge)) {
            if (soc_property_get(unit, spn_BCM56629_40GE, 0)) {
                *phy_addr = _soc_phy_addr_bcm56624[port];
            } else {
                *phy_addr = _soc_phy_addr_bcm56629[port];
            }
        } else {
            *phy_addr = _soc_phy_addr_bcm56626[port];
        }
        break;
    case BCM56624_DEVICE_ID:
    default:
        *phy_addr = _soc_phy_addr_bcm56624[port];
        break;
    }
}
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_VALKYRIE_SUPPORT
static void
_valkyrie_phy_addr_default(int unit, int port,
                           uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_valkyrie_int_phy_addr[] = {
        0x00, /* Port  0 (cmic)         N/A */
        0x99, /* Port  1 (serdes2:8)    IntBus=0 Addr=0x19 */
        0xc1, /* Port  2 (hyperlite0:0) IntBus=2 Addr=0x01 */
        0xc2, /* Port  3 (hyperlite0:1) IntBus=2 Addr=0x02 */
        0xc3, /* Port  4 (hyperlite0:2) IntBus=2 Addr=0x03 */
        0xc4, /* Port  5 (hyperlite0:3) IntBus=2 Addr=0x04 */
        0xc5, /* Port  6 (hyperlite1:0) IntBus=2 Addr=0x05 */
        0xc6, /* Port  7 (hyperlite1:1) IntBus=2 Addr=0x06 */
        0x81, /* Port  8 (N/A) */
        0x82, /* Port  9 (N/A) */
        0x83, /* Port 10 (N/A) */
        0x84, /* Port 11 (N/A) */
        0x85, /* Port 12 (N/A) */
        0x86, /* Port 13 (N/A) */
        0xc9, /* Port 14 (hyperlite2:0) IntBus=2 Addr=0x09 */
        0xca, /* Port 15 (hyperlite2:1) IntBus=2 Addr=0x0a */
        0xcb, /* Port 16 (hyperlite2:2) IntBus=2 Addr=0x0b */
        0xcc, /* Port 17 (hyperlite2:3) IntBus=2 Addr=0x0c */
        0xc7, /* Port 18 (hyperlite1:2) IntBus=2 Addr=0x07 */
        0xc8, /* Port 19 (hyperlite1:3) IntBus=2 Addr=0x08 */
        0x87, /* Port 20 (N/A) */
        0x88, /* Port 21 (N/A) */
        0x89, /* Port 22 (N/A) */
        0x8a, /* Port 23 (N/A) */
        0x8b, /* Port 24 (N/A) */
        0x8c, /* Port 25 (N/A) */
        0xcd, /* Port 26 (hyperlite3:0) IntBus=2 Addr=0x0d */
        0xd5, /* Port 27 (hyperlite5:0) IntBus=2 Addr=0x15 */
        0xd9, /* Port 28 (unicore0)     IntBus=2 Addr=0x19 */
        0xda, /* Port 29 (unicore1)     IntBus=2 Addr=0x1a */
        0xdb, /* Port 30 (unicore2)     IntBus=2 Addr=0x1b */
        0xdc, /* Port 31 (unicore3)     IntBus=2 Addr=0x1c */
        0xce, /* Port 32 (hyperlite3:1) IntBus=2 Addr=0x0e */
        0xcf, /* Port 33 (hyperlite3:2) IntBus=2 Addr=0x0f */
        0xd0, /* Port 34 (hyperlite3:3) IntBus=2 Addr=0x10 */
        0xd1, /* Port 35 (hyperlite4:0) IntBus=2 Addr=0x11 */
        0xd2, /* Port 36 (hyperlite4:1) IntBus=2 Addr=0x12 */
        0x8d, /* Port 37 (N/A) */
        0x8e, /* Port 38 (N/A) */
        0x8f, /* Port 39 (N/A) */
        0x90, /* Port 40 (N/A) */
        0x91, /* Port 41 (N/A) */
        0x92, /* Port 42 (N/A) */
        0xd6, /* Port 43 (hyperlite5:1) IntBus=2 Addr=0x16 */
        0xd7, /* Port 44 (hyperlite5:2) IntBus=2 Addr=0x17 */
        0xd8, /* Port 45 (hyperlite5:3) IntBus=2 Addr=0x18 */
        0xd3, /* Port 46 (hyperlite4:2) IntBus=2 Addr=0x13 */
        0xd4, /* Port 47 (hyperlite4:3) IntBus=2 Addr=0x14 */
        0x93, /* Port 48 (N/A) */
        0x94, /* Port 49 (N/A) */
        0x95, /* Port 50 (N/A) */
        0x96, /* Port 51 (N/A) */
        0x97, /* Port 52 (N/A) */
        0x98, /* Port 53 (N/A) */
    };

    static const uint16 _soc_phy_addr_bcm56680[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x39, /* Port  1 ( ge0) ExtBus=1 Addr=0x19 */
        0x01, /* Port  2 ( ge1) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge2) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge3) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge4) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge5) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge6) ExtBus=0 Addr=0x06 */
        0x00, /* Port  8 ( N/A) */
        0x00, /* Port  9 ( N/A) */
        0x00, /* Port 10 ( N/A) */
        0x00, /* Port 11 ( N/A) */
        0x00, /* Port 12 ( N/A) */
        0x00, /* Port 13 ( N/A) */
        0x07, /* Port 14 ( ge7) ExtBus=0 Addr=0x07 */
        0x08, /* Port 15 ( ge8) ExtBus=0 Addr=0x08 */
        0x09, /* Port 16 ( ge9) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 17 (ge10) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 18 (ge11) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 19 (ge12) ExtBus=0 Addr=0x0c */
        0x00, /* Port 20 ( N/A) */
        0x00, /* Port 21 ( N/A) */
        0x00, /* Port 22 ( N/A) */
        0x00, /* Port 23 ( N/A) */
        0x00, /* Port 24 ( N/A) */
        0x00, /* Port 25 ( N/A) */
        0x0d, /* Port 26 (ge13) ExtBus=0 Addr=0x0d */
        0x13, /* Port 27 (ge19) ExtBus=0 Addr=0x13 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
        0x0e, /* Port 32 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 33 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 34 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 35 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 36 (ge18) ExtBus=0 Addr=0x12 */
        0x00, /* Port 37 ( N/A) */
        0x00, /* Port 38 ( N/A) */
        0x00, /* Port 39 ( N/A) */
        0x00, /* Port 40 ( N/A) */
        0x00, /* Port 41 ( N/A) */
        0x00, /* Port 42 ( N/A) */
        0x14, /* Port 43 (ge20) ExtBus=0 Addr=0x14 */
        0x15, /* Port 44 (ge21) ExtBus=0 Addr=0x15 */
        0x16, /* Port 45 (ge22) ExtBus=0 Addr=0x16 */
        0x17, /* Port 46 (ge23) ExtBus=0 Addr=0x17 */
        0x18, /* Port 47 (ge24) ExtBus=0 Addr=0x18 */
        0x00, /* Port 48 ( N/A) */
        0x00, /* Port 49 ( N/A) */
        0x00, /* Port 50 ( N/A) */
        0x00, /* Port 51 ( N/A) */
        0x00, /* Port 52 ( N/A) */
        0x00, /* Port 53 ( N/A) */
    };

    static const uint16 _soc_phy_addr_bcm56684[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A) */
        0x01, /* Port  2 ( ge0) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge1) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge2) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge3) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge4) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge5) ExtBus=0 Addr=0x06 */
        0x00, /* Port  8 ( N/A) */
        0x00, /* Port  9 ( N/A) */
        0x00, /* Port 10 ( N/A) */
        0x00, /* Port 11 ( N/A) */
        0x00, /* Port 12 ( N/A) */
        0x00, /* Port 13 ( N/A) */
        0x09, /* Port 14 ( ge6) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 15 ( ge7) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 16 ( ge8) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 17 ( ge9) ExtBus=0 Addr=0x0c */
        0x07, /* Port 18 (ge10) ExtBus=0 Addr=0x07 */
        0x08, /* Port 19 (ge11) ExtBus=0 Addr=0x08 */
        0x00, /* Port 20 ( N/A) */
        0x00, /* Port 21 ( N/A) */
        0x00, /* Port 22 ( N/A) */
        0x00, /* Port 23 ( N/A) */
        0x00, /* Port 24 ( N/A) */
        0x00, /* Port 25 ( N/A) */
        0x0d, /* Port 26 (ge12) ExtBus=0 Addr=0x0d */
        0x15, /* Port 27 (ge13) ExtBus=0 Addr=0x15 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
        0x0e, /* Port 32 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 33 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 34 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 35 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 36 (ge18) ExtBus=0 Addr=0x12 */
        0x00, /* Port 37 ( N/A) */
        0x00, /* Port 38 ( N/A) */
        0x00, /* Port 39 ( N/A) */
        0x00, /* Port 40 ( N/A) */
        0x00, /* Port 41 ( N/A) */
        0x00, /* Port 42 ( N/A) */
        0x16, /* Port 43 (ge19) ExtBus=0 Addr=0x16 */
        0x17, /* Port 44 (ge20) ExtBus=0 Addr=0x17 */
        0x18, /* Port 45 (ge21) ExtBus=0 Addr=0x18 */
        0x13, /* Port 46 (ge22) ExtBus=0 Addr=0x13 */
        0x14, /* Port 47 (ge23) ExtBus=0 Addr=0x14 */
        0x00, /* Port 48 ( N/A) */
        0x00, /* Port 49 ( N/A) */
        0x00, /* Port 50 ( N/A) */
        0x00, /* Port 51 ( N/A) */
        0x00, /* Port 52 ( N/A) */
        0x00, /* Port 53 ( N/A) */
    };

    static const uint16 _soc_phy_addr_bcm56686[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A) */
        0x45, /* Port  2 ( xe0) ExtBus=2 Addr=0x05 */
        0x00, /* Port  3 ( N/A) */
        0x00, /* Port  4 ( N/A) */
        0x00, /* Port  5 ( N/A) */
        0x00, /* Port  6 ( N/A) */
        0x00, /* Port  7 ( N/A) */
        0x00, /* Port  8 ( N/A) */
        0x00, /* Port  9 ( N/A) */
        0x00, /* Port 10 ( N/A) */
        0x00, /* Port 11 ( N/A) */
        0x00, /* Port 12 ( N/A) */
        0x00, /* Port 13 ( N/A) */
        0x00, /* Port 14 ( N/A) */
        0x00, /* Port 15 ( N/A) */
        0x00, /* Port 16 ( N/A) */
        0x00, /* Port 17 ( N/A) */
        0x00, /* Port 18 ( N/A) */
        0x00, /* Port 19 ( N/A) */
        0x00, /* Port 20 ( N/A) */
        0x00, /* Port 21 ( N/A) */
        0x00, /* Port 22 ( N/A) */
        0x00, /* Port 23 ( N/A) */
        0x00, /* Port 24 ( N/A) */
        0x00, /* Port 25 ( N/A) */
        0x00, /* Port 26 ( N/A) */
        0x46, /* Port 27 ( xe1) ExtBus=2 Addr=0x06 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
    };

    static const uint16 _soc_phy_addr_bcm56620[] = {
        0x00, /* Port  0 (cmic) N/A */
        0x00, /* Port  1 ( N/A) */
        0x01, /* Port  2 ( ge0) ExtBus=0 Addr=0x01 */
        0x02, /* Port  3 ( ge1) ExtBus=0 Addr=0x02 */
        0x03, /* Port  4 ( ge2) ExtBus=0 Addr=0x03 */
        0x04, /* Port  5 ( ge3) ExtBus=0 Addr=0x04 */
        0x05, /* Port  6 ( ge4) ExtBus=0 Addr=0x05 */
        0x06, /* Port  7 ( ge5) ExtBus=0 Addr=0x06 */
        0x00, /* Port  8 ( N/A) */
        0x00, /* Port  9 ( N/A) */
        0x00, /* Port 10 ( N/A) */
        0x00, /* Port 11 ( N/A) */
        0x00, /* Port 12 ( N/A) */
        0x00, /* Port 13 ( N/A) */
        0x09, /* Port 14 ( ge6) ExtBus=0 Addr=0x09 */
        0x0a, /* Port 15 ( ge7) ExtBus=0 Addr=0x0a */
        0x0b, /* Port 16 ( ge8) ExtBus=0 Addr=0x0b */
        0x0c, /* Port 17 ( ge9) ExtBus=0 Addr=0x0c */
        0x07, /* Port 18 (ge10) ExtBus=0 Addr=0x07 */
        0x08, /* Port 19 (ge11) ExtBus=0 Addr=0x08 */
        0x00, /* Port 20 ( N/A) */
        0x00, /* Port 21 ( N/A) */
        0x00, /* Port 22 ( N/A) */
        0x00, /* Port 23 ( N/A) */
        0x00, /* Port 24 ( N/A) */
        0x00, /* Port 25 ( N/A) */
        0x0d, /* Port 26 (ge12) ExtBus=0 Addr=0x0d */
        0x15, /* Port 27 (ge13) ExtBus=0 Addr=0x15 */
        0x41, /* Port 28 ( hg0) ExtBus=2 Addr=0x01 */
        0x42, /* Port 29 ( hg1) ExtBus=2 Addr=0x02 */
        0x43, /* Port 30 ( hg2) ExtBus=2 Addr=0x03 */
        0x44, /* Port 31 ( hg3) ExtBus=2 Addr=0x04 */
        0x0e, /* Port 32 (ge14) ExtBus=0 Addr=0x0e */
        0x0f, /* Port 33 (ge15) ExtBus=0 Addr=0x0f */
        0x10, /* Port 34 (ge16) ExtBus=0 Addr=0x10 */
        0x11, /* Port 35 (ge17) ExtBus=0 Addr=0x11 */
        0x12, /* Port 36 (ge18) ExtBus=0 Addr=0x12 */
        0x00, /* Port 37 ( N/A) */
        0x00, /* Port 38 ( N/A) */
        0x00, /* Port 39 ( N/A) */
        0x00, /* Port 40 ( N/A) */
        0x00, /* Port 41 ( N/A) */
        0x00, /* Port 42 ( N/A) */
        0x16, /* Port 43 (ge19) ExtBus=0 Addr=0x16 */
        0x17, /* Port 44 (ge20) ExtBus=0 Addr=0x17 */
        0x18, /* Port 45 (ge21) ExtBus=0 Addr=0x18 */
        0x13, /* Port 46 (ge22) ExtBus=0 Addr=0x13 */
        0x14, /* Port 47 (ge23) ExtBus=0 Addr=0x14 */
        0x00, /* Port 48 ( N/A) */
        0x00, /* Port 49 ( N/A) */
        0x00, /* Port 50 ( N/A) */
        0x00, /* Port 51 ( N/A) */
        0x00, /* Port 52 ( N/A) */
        0x00, /* Port 53 ( N/A) */
    };
    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    *phy_addr_int = _soc_valkyrie_int_phy_addr[port];
    switch (dev_id) {
    case BCM56620_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56620[port];
        break;
    case BCM56684_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56684[port];
        break;
    case BCM56686_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56686[port];
        break;
    case BCM56680_DEVICE_ID:
    default:
        *phy_addr = _soc_phy_addr_bcm56680[port];
        break;
    }
}
#endif /* BCM_VALKYRIE_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC void
_trident_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    soc_info_t *si;
    int phy_port;

    si = &SOC_INFO(unit);

    phy_port = si->port_l2p_mapping[port];
    if (phy_port == -1) {
        *phy_addr_int = 0;
        *phy_addr = 0;
        return;
    }

    /* Physical port number starts from 1
     * Internal/external MDIO bus number = (physical port number - 1) / 24
     * Internal MDIO address = (physical port number - 1) % 24 + 1
     */
    *phy_addr_int = 0x80 | (((phy_port - 1) / 24) << 5) | 
                           ((((phy_port - 1) % 24)/4)*4 + 1);

    /* external MDIO address */
    *phy_addr = (((phy_port - 1) / 24) << 5) | 
                ((phy_port - 1) % 24 + 4);
}
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
STATIC void
_shadow_phy_addr_default(int unit, int port,
                          uint16 *phy_addr, uint16 *phy_addr_int)
{
    soc_info_t *si;
    int phy_port;
    static const uint16 _soc_phy_addr_bcm88732_a0[] = {
        0,
        1, 1, 1, 1,       /* Radian port 1-4 => WC0 MDIO Port Address 1 */ 
        5, 5, 5, 5,       /* Radian port 5-8 => WC0 MDIO Port Address 5 */
        9, 9, 9, 9,       /* Radian port 9-10 => WC0 MDIO Port Address 9 */ 
        13, 13, 13, 13,   /* Radian port 11-12 => WC0 MDIO Port Address 13 */ 
        17, 17, 17, 17,   /* Radian port 13-14 => WC0 MDIO Port Address 17 */
        21, 21, 21, 21    /* Radian port 15-16 => WC0 MDIO Port Address 21 */
    };
    static const uint16 _soc_phy_addr_switch_link_xaui[] = {
        9,  9,       /* Radian port 9-10 => WC0 MDIO Port Address 9 */ 
        13, 13,    /* Radian port 11-12 => WC0 MDIO Port Address 13 */ 
        17, 17,    /* Radian port 13-14 => WC0 MDIO Port Address 17 */
        21, 21    /* Radian port 15-16 => WC0 MDIO Port Address 21 */
    };
    si = &SOC_INFO(unit);

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = si->port_l2p_mapping[port];
    } else {
        phy_port = port;
    }
    if (phy_port < 1 || phy_port > 16) {
        *phy_addr_int = 0;
        *phy_addr = 0;
        return;
    }
    if (phy_port < 9) {
       *phy_addr_int = 0x80 | (_soc_phy_addr_bcm88732_a0[phy_port]);
       *phy_addr = *phy_addr_int; /* no external PHYs */
    } else {  /* switch link side */
        if (soc_property_get(unit, spn_BCM88732_1X40_4X10, 0) || 
            soc_property_get(unit, spn_BCM88732_4X10_4X10, 0)) {
            *phy_addr_int = 0x80 | (_soc_phy_addr_switch_link_xaui[phy_port-9]);
            *phy_addr = *phy_addr_int; /* no external PHYs */
        } else {
            *phy_addr_int = 0x80 | (_soc_phy_addr_bcm88732_a0[phy_port]);
            if (soc_property_get(unit, spn_BCM88732_2X40_2X40, 0) ||
                soc_property_get(unit, spn_BCM88732_8X10_2X40, 0) ||
                (soc_property_get(unit, "IL3125", 0)))  { /* for 16Lanes */
                /* 
                 * Use the MDIO address assignment similar to that of 
                 * Switch link port configuration. 
                 */
                *phy_addr_int = 0x80 | (_soc_phy_addr_switch_link_xaui[phy_port-9]);
            } else { 
                 *phy_addr_int = 0x80 | (_soc_phy_addr_bcm88732_a0[phy_port]);
            }
            *phy_addr = *phy_addr_int; /* no external PHYs */
        }
    }
}
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_SCORPION_SUPPORT
static int
_scorpion_phy_addr_default(int unit, int port,
                           uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_phy_addr_bcm56820_a0[] = {
 /* extPhy intPhy          */
    0x00, 0x00, /* Port  0 (cmic) N/A */
    0x42, 0xc1, /* Port  1 ( xe0) */
    0x43, 0xc2, /* Port  2 ( xe1) */
    0x44, 0xc3, /* Port  3 ( xe2) */
    0x45, 0xc4, /* Port  4 ( xe3) */
    0x46, 0xc5, /* Port  5 ( xe4) */
    0x47, 0xc6, /* Port  6 ( xe5) */
    0x48, 0xc7, /* Port  7 ( xe6) */
    0x49, 0xc8, /* Port  8 ( xe7) */
    0x4a, 0xc9, /* Port  9 ( xe8) */
    0x4b, 0xca, /* Port 10 ( xe9) */
    0x4c, 0xcb, /* Port 11 (xe10) */
    0x4d, 0xcc, /* Port 12 (xe11) */
    0x4e, 0xcd, /* Port 13 (xe12) */
    0x4f, 0xce, /* Port 14 (xe13) */
    0x50, 0xcf, /* Port 15 (xe14) */
    0x51, 0xd0, /* Port 16 (xe15) */
    0x52, 0xd1, /* Port 17 (xe16) */
    0x53, 0xd2, /* Port 18 (xe17) */
    0x54, 0xd3, /* Port 19 (xe18) */
    0x55, 0xd4, /* Port 20 (xe19) */
    0x21, 0xd5, /* Port 21 (xe20) */
    0x26, 0xd6, /* Port 22 (xe21) */
    0x2b, 0xd7, /* Port 23 (xe22) */
    0x30, 0xd8, /* Port 24 (xe23) */
    0x59, 0xd9, /* Port 25 (ge0) */
    0x5a, 0xda, /* Port 26 (ge1) */
    0x5b, 0xdb, /* Port 27 (ge2) */
    0x5c, 0xdc  /* Port 28 (ge3) */
    };
    int rv = TRUE;
    uint16 dev_id;
    uint8 rev_id;
                                                                                
    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (dev_id) {
    case BCM56820_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm56820_a0[port * 2];
        *phy_addr_int = _soc_phy_addr_bcm56820_a0[port * 2 + 1];
        break;
    default:
        rv = FALSE;
        break;
    }
    return rv;
}
#endif /* BCM_SCORPION_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
static void
_xgs3_phy_addr_default(int unit, int port,
                      uint16 *phy_addr, uint16 *phy_addr_int)
{
    int phy_addr_adjust = 0;

    if (SOC_IS_GOLDWING(unit)) {
        /* In Goldwing due to the hardware port remapping, we need to adjust
         * the PHY addresses. The PHY addresses are remapped as follow.
         * Original : 0  ... 13 14 15 16 17 18 19
         * Remapped : 0  ... 13 16 17 18 19 14 15
         */
        if (port == 14 || port == 15) {
            phy_addr_adjust = 4;
        } else if (port > 15) {
            phy_addr_adjust = -2;
        }
    }
    if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port) ||
        IS_GX_PORT(unit, port)) {
        pbmp_t pbm;
        int    temp_port;
        int    found     = 0;
        int    mdio_addr = 1;
        /*
         * Internal XAUI (Internal sel bit 0x80) on XPORT MDIO bus(0x40)
         * External Phy  (Internal sel bit 0x00) on XPORT MDIO bus(0x40)
         * Assume External MDIO address starts at 1 as this is on a
         * seperate BUS.
         */

        /*
         * First, assign lowest addresses to GE ports which are also
         * GX ports.
         */
        pbm = PBMP_GX_ALL(unit);
        SOC_PBMP_AND(pbm, PBMP_GE_ALL(unit));

        PBMP_ITER(pbm, temp_port) {
            if (temp_port == port) {
                found = 1;
                break;
            }
            mdio_addr++;
        }

        /*
         * Second, assign external addresses for XE ports.
         */
        if (!found) {
            PBMP_XE_ITER(unit, temp_port) {
                if  (temp_port == port) {
                    found = 1;
                    break;
                }
                mdio_addr++;
            }
        }

        /*
         * Finally, assign external adddresses for HG ports.
         */
        if (!found) {
            PBMP_HG_ITER(unit, temp_port) {
                if  (temp_port == port) {
                    found = 1;
                    break;
                }
                mdio_addr++;
            }
        }

        *phy_addr = mdio_addr + 0x40;
        *phy_addr_int = port + 0xc0 + phy_addr_adjust;

        if (SOC_IS_SC_CQ(unit)) {
            *phy_addr_int = port + 0xc0;
            *phy_addr = port + 0x41;
        }
        if (SAL_BOOT_QUICKTURN) {
            *phy_addr = port + 0x41;
            if (SOC_IS_SCORPION(unit)) {
                /* Skip over CMIC at port 0 */
                *phy_addr -= 1;
            }
        }
    } else {
        /*
         * Internal Serdes (Internal sel bit 0x80) on GPORT MDIO bus(0x00)
         * External Phy    (Internal sel bit 0x00) on GPORT MDIO bus(0x00)
         */
        *phy_addr = port + 1 + phy_addr_adjust;
        *phy_addr_int = port + 0x80 + phy_addr_adjust;
        if (SOC_IS_SC_CQ(unit)) {
            *phy_addr_int = port + 0xc0;
            *phy_addr = port + 0x40;
        }
    }
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_SBX_SUPPORT
typedef struct _block_phy_addr_s {
    int     block_type;
    int     block_number;
    int     block_index;
    uint16  addr;
    uint16  addr_int;
} _block_phy_addr_t;

#ifdef BCM_BM9600_SUPPORT
static _block_phy_addr_t   _soc_phy_addr_bcm88130_a0[] = {
    { SOC_BLK_GXPORT,    0x0,    0,   0x0,    0x0 },  /* sfi0  */
    { SOC_BLK_GXPORT,    0x0,    1,   0x0,    0x0 },  /* sfi1  */
    { SOC_BLK_GXPORT,    0x0,    2,   0x0,    0x0 },  /* sfi2  */
    { SOC_BLK_GXPORT,    0x0,    3,   0x0,    0x0 },  /* sfi3  */
    { SOC_BLK_GXPORT,    0x1,    4,   0x1,    0x1 },  /* sfi4  */
    { SOC_BLK_GXPORT,    0x1,    5,   0x1,    0x1 },  /* sfi5  */
    { SOC_BLK_GXPORT,    0x1,    6,   0x1,    0x1 },  /* sfi6  */
    { SOC_BLK_GXPORT,    0x1,    7,   0x1,    0x1 },  /* sfi7  */
    { SOC_BLK_GXPORT,    0x2,    8,   0x2,    0x2 },  /* sfi8  */
    { SOC_BLK_GXPORT,    0x2,    9,   0x2,    0x2 },  /* sfi9  */
    { SOC_BLK_GXPORT,    0x2,   10,   0x2,    0x2 },  /* sfi10 */
    { SOC_BLK_GXPORT,    0x2,   11,   0x2,    0x2 },  /* sfi11 */
    { SOC_BLK_GXPORT,    0x3,   12,   0x3,    0x3 },  /* sfi12 */
    { SOC_BLK_GXPORT,    0x3,   13,   0x3,    0x3 },  /* sfi13 */
    { SOC_BLK_GXPORT,    0x3,   14,   0x3,    0x3 },  /* sfi14 */
    { SOC_BLK_GXPORT,    0x3,   15,   0x3,    0x3 },  /* sfi15 */
    { SOC_BLK_GXPORT,    0x4,   16,   0x4,    0x4 },  /* sfi16 */
    { SOC_BLK_GXPORT,    0x4,   17,   0x4,    0x4 },  /* sfi17 */
    { SOC_BLK_GXPORT,    0x4,   18,   0x4,    0x4 },  /* sfi18 */
    { SOC_BLK_GXPORT,    0x4,   19,   0x4,    0x4 },  /* sfi19 */
    { SOC_BLK_GXPORT,    0x5,   20,   0x5,    0x5 },  /* sfi20 */
    { SOC_BLK_GXPORT,    0x5,   21,   0x5,    0x5 },  /* sfi21 */
    { SOC_BLK_GXPORT,    0x5,   22,   0x5,    0x5 },  /* sfi22 */
    { SOC_BLK_GXPORT,    0x5,   23,   0x5,    0x5 },  /* sfi23 */
    { SOC_BLK_GXPORT,    0x6,   24,   0x6,    0x6 },  /* sfi24  */
    { SOC_BLK_GXPORT,    0x6,   25,   0x6,    0x6 },  /* sfi25  */
    { SOC_BLK_GXPORT,    0x6,   26,   0x6,    0x6 },  /* sfi26  */
    { SOC_BLK_GXPORT,    0x6,   27,   0x6,    0x6 },  /* sfi27  */
    { SOC_BLK_GXPORT,    0x7,   28,   0x7,    0x7 },  /* sfi28 */
    { SOC_BLK_GXPORT,    0x7,   29,   0x7,    0x7 },  /* sfi29  */
    { SOC_BLK_GXPORT,    0x7,   30,   0x7,    0x7 },  /* sfi30  */
    { SOC_BLK_GXPORT,    0x7,   31,   0x7,    0x7 },  /* sfi31  */
    { SOC_BLK_GXPORT,    0x8,   32,   0x8,    0x8 },  /* sfi32  */
    { SOC_BLK_GXPORT,    0x8,   33,   0x8,    0x8 },  /* sfi33  */
    { SOC_BLK_GXPORT,    0x8,   34,   0x8,    0x8 },  /* sfi34  */
    { SOC_BLK_GXPORT,    0x8,   35,   0x8,    0x8 },  /* sfi35  */
    { SOC_BLK_GXPORT,    0x9,   36,   0x9,    0x9 },  /* sfi36 */
    { SOC_BLK_GXPORT,    0x9,   37,   0x9,    0x9 },  /* sfi37 */
    { SOC_BLK_GXPORT,    0x9,   38,   0x9,    0x9 },  /* sfi38 */
    { SOC_BLK_GXPORT,    0x9,   39,   0x9,    0x9 },  /* sfi39 */
    { SOC_BLK_GXPORT,    0xa,   40,   0xa,    0xa },  /* sfi40 */
    { SOC_BLK_GXPORT,    0xa,   41,   0xa,    0xa },  /* sfi41 */
    { SOC_BLK_GXPORT,    0xa,   42,   0xa,    0xa },  /* sfi42 */
    { SOC_BLK_GXPORT,    0xa,   43,   0xa,    0xa },  /* sfi43 */
    { SOC_BLK_GXPORT,    0xb,   44,   0xb,    0xb },  /* sfi44 */
    { SOC_BLK_GXPORT,    0xb,   45,   0xb,    0xb },  /* sfi45 */
    { SOC_BLK_GXPORT,    0xb,   46,   0xb,    0xb },  /* sfi46 */
    { SOC_BLK_GXPORT,    0xb,   47,   0xb,    0xb },  /* sfi47 */
    { SOC_BLK_GXPORT,    0xc,   48,   0xc,    0xc },  /* sfi48 */
    { SOC_BLK_GXPORT,    0xc,   49,   0xc,    0xc },  /* sfi49 */
    { SOC_BLK_GXPORT,    0xc,   50,   0xc,    0xc },  /* sfi50  */
    { SOC_BLK_GXPORT,    0xc,   51,   0xc,    0xc },  /* sfi51  */
    { SOC_BLK_GXPORT,    0xd,   52,   0xd,    0xd },  /* sfi52  */
    { SOC_BLK_GXPORT,    0xd,   53,   0xd,    0xd },  /* sfi53  */
    { SOC_BLK_GXPORT,    0xd,   54,   0xd,    0xd },  /* sfi54  */
    { SOC_BLK_GXPORT,    0xd,   55,   0xd,    0xd },  /* sfi55  */
    { SOC_BLK_GXPORT,    0xe,   56,   0xe,    0xe },  /* sfi56  */
    { SOC_BLK_GXPORT,    0xe,   57,   0xe,    0xe },  /* sfi57  */
    { SOC_BLK_GXPORT,    0xe,   58,   0xe,    0xe },  /* sfi58  */
    { SOC_BLK_GXPORT,    0xe,   59,   0xe,    0xe },  /* sfi59  */
    { SOC_BLK_GXPORT,    0xf,   60,   0xf,    0xf },  /* sfi60  */
    { SOC_BLK_GXPORT,    0xf,   61,   0xf,    0xf },  /* sfi61  */
    { SOC_BLK_GXPORT,    0xf,   62,   0xf,    0xf },  /* sfi62 */
    { SOC_BLK_GXPORT,    0xf,   63,   0xf,    0xf },  /* sfi63 */
    { SOC_BLK_GXPORT,    0x10,  64,   0x10,   0x10 },  /* sfi64 */
    { SOC_BLK_GXPORT,    0x10,  65,   0x10,   0x10 },  /* sfi65 */
    { SOC_BLK_GXPORT,    0x10,  66,   0x10,   0x10 },  /* sfi66 */
    { SOC_BLK_GXPORT,    0x10,  67,   0x10,   0x10 },  /* sfi67 */
    { SOC_BLK_GXPORT,    0x11,  68,   0x11,   0x11 },  /* sfi68 */
    { SOC_BLK_GXPORT,    0x11,  69,   0x11,   0x11 },  /* sfi69 */
    { SOC_BLK_GXPORT,    0x11,  70,   0x11,   0x11 },  /* sfi70 */
    { SOC_BLK_GXPORT,    0x11,  71,   0x11,   0x11 },  /* sfi71 */
    { SOC_BLK_GXPORT,    0x12,  72,   0x12,   0x12 },  /* sfi72 */
    { SOC_BLK_GXPORT,    0x12,  73,   0x12,   0x12 },  /* sfi73 */
    { SOC_BLK_GXPORT,    0x12,  74,   0x12,   0x12 },  /* sfi74 */
    { SOC_BLK_GXPORT,    0x12,  75,   0x12,   0x12 },  /* sfi75 */
    { SOC_BLK_GXPORT,    0x13,  76,   0x13,   0x13 },  /* sfi76  */
    { SOC_BLK_GXPORT,    0x13,  77,   0x13,   0x13 },  /* sfi77  */
    { SOC_BLK_GXPORT,    0x13,  78,   0x13,   0x13 },  /* sfi78  */
    { SOC_BLK_GXPORT,    0x13,  79,   0x13,   0x13 },  /* sfi79  */
    { SOC_BLK_GXPORT,    0x14,  80,   0x14,   0x14 },  /* sfi80  */
    { SOC_BLK_GXPORT,    0x14,  81,   0x14,   0x14 },  /* sfi81  */
    { SOC_BLK_GXPORT,    0x14,  82,   0x14,   0x14 },  /* sfi82  */
    { SOC_BLK_GXPORT,    0x14,  83,   0x14,   0x14 },  /* sfi83  */
    { SOC_BLK_GXPORT,    0x15,  84,   0x15,   0x15 },  /* sfi84  */
    { SOC_BLK_GXPORT,    0x15,  85,   0x15,   0x15 },  /* sfi85  */
    { SOC_BLK_GXPORT,    0x15,  86,   0x15,   0x15 },  /* sfi86  */
    { SOC_BLK_GXPORT,    0x15,  87,   0x15,   0x15 },  /* sfi87  */
    { SOC_BLK_GXPORT,    0x16,  88,   0x16,   0x16 },  /* sfi88 */
    { SOC_BLK_GXPORT,    0x16,  89,   0x16,   0x16 },  /* sfi89 */
    { SOC_BLK_GXPORT,    0x16,  90,   0x16,   0x16 },  /* sfi90 */
    { SOC_BLK_GXPORT,    0x16,  91,   0x16,   0x16 },  /* sfi91 */
    { SOC_BLK_GXPORT,    0x17,  92,   0x17,   0x17 },  /* sfi92 */
    { SOC_BLK_GXPORT,    0x17,  93,   0x17,   0x17 },  /* sfi93 */
    { SOC_BLK_GXPORT,    0x17,  94,   0x17,   0x17 },  /* sfi94 */
    { SOC_BLK_GXPORT,    0x17,  95,   0x17,   0x17 },  /* sfi95 */
    {            -1,   -1,   -1,   0x0,   0x00 }   /* Last */
};

static void
_bm9600_phy_addr_default(int unit, int port,
                         uint16 *phy_addr, uint16 *phy_addr_int)
{
  _block_phy_addr_t  *phy_info;

  *phy_addr = *phy_addr_int = 0x0;
  phy_info = &_soc_phy_addr_bcm88130_a0[0];

  while (phy_info->block_type != -1) {
    if (phy_info->block_index == port) {
      /* Found */
      *phy_addr     = phy_info->addr;
      *phy_addr_int = phy_info->addr_int;
      break;
    }

    phy_info++;
  }
}
#endif /* BCM_BM9600_SUPPORT */

static _block_phy_addr_t   _soc_phy_addr_bcm88020_a0[] = {
    { SOC_BLK_GPORT,    0,    0,   0x0,    0x80 },  /* ge0  */
    { SOC_BLK_GPORT,    0,    1,   0x1,    0x81 },  /* ge1  */
    { SOC_BLK_GPORT,    0,    2,   0x2,    0x82 },  /* ge2  */
    { SOC_BLK_GPORT,    0,    3,   0x3,    0x83 },  /* ge3  */
    { SOC_BLK_GPORT,    0,    4,   0x4,    0x84 },  /* ge4  */
    { SOC_BLK_GPORT,    0,    5,   0x5,    0x85 },  /* ge5  */
    { SOC_BLK_GPORT,    0,    6,   0x6,    0x86 },  /* ge6  */
    { SOC_BLK_GPORT,    0,    7,   0x7,    0x87 },  /* ge7  */
    { SOC_BLK_GPORT,    0,    8,   0x8,    0x88 },  /* ge8  */
    { SOC_BLK_GPORT,    0,    9,   0x9,    0x89 },  /* ge9  */
    { SOC_BLK_GPORT,    0,   10,   0xa,    0x8a },  /* ge10 */
    { SOC_BLK_GPORT,    0,   11,   0xb,    0x8b },  /* ge11 */
    { SOC_BLK_GPORT,    1,    0,   0xc,    0x8c },  /* ge12 */
    { SOC_BLK_GPORT,    1,    1,   0xd,    0x8d },  /* ge13 */
    { SOC_BLK_GPORT,    1,    2,   0xe,    0x8e },  /* ge14 */
    { SOC_BLK_GPORT,    1,    3,   0xf,    0x8f },  /* ge15 */
    { SOC_BLK_GPORT,    1,    4,  0x10,    0x90 },  /* ge16 */
    { SOC_BLK_GPORT,    1,    5,  0x11,    0x91 },  /* ge17 */
    { SOC_BLK_GPORT,    1,    6,  0x12,    0x92 },  /* ge18 */
    { SOC_BLK_GPORT,    1,    7,  0x13,    0x93 },  /* ge19 */
    { SOC_BLK_GPORT,    1,    8,  0x14,    0x94 },  /* ge20 */
    { SOC_BLK_GPORT,    1,    9,  0x15,    0x95 },  /* ge21 */
    { SOC_BLK_GPORT,    1,   10,  0x16,    0x96 },  /* ge22 */
    { SOC_BLK_GPORT,    1,   11,  0x17,    0x97 },  /* ge23 */
    { SOC_BLK_XPORT,    0,    0,  0x58,    0xd8 },  /* xe0  */
    { SOC_BLK_XPORT,    1,    0,  0x59,    0xd9 },  /* xe1  */
    {            -1,   -1,   -1,   0x0,    0x00 }   /* Last */
};

/*  Note:
 *  ge8-ge11 share the same internal phy as xe2
 *  ge20-ge23 share the same internal phy as xe3 
 */

static _block_phy_addr_t   _soc_phy_addr_bcm88025_a0[] = {
    { SOC_BLK_GPORT,    0,    0,   0x0,    0x80 },   /* ge0  */
    { SOC_BLK_GPORT,    0,    1,   0x1,    0x81 },   /* ge1  */
    { SOC_BLK_GPORT,    0,    2,   0x2,    0x82 },   /* ge2  */
    { SOC_BLK_GPORT,    0,    3,   0x3,    0x83 },   /* ge3  */
    { SOC_BLK_GPORT,    0,    4,   0x4,    0x84 },   /* ge4  */
    { SOC_BLK_GPORT,    0,    5,   0x5,    0x85 },   /* ge5  */
    { SOC_BLK_GPORT,    0,    6,   0x6,    0x86 },   /* ge6  */
    { SOC_BLK_GPORT,    0,    7,   0x7,    0x87 },   /* ge7  */
    { SOC_BLK_GPORT,    0,    8,   0x8,    0xd2 },   /* ge8  */
    { SOC_BLK_GPORT,    0,    9,   0x9,    0xd2 },   /* ge9  */
    { SOC_BLK_GPORT,    0,   10,   0xa,    0xd2 },   /* ge10 */
    { SOC_BLK_GPORT,    0,   11,   0xb,    0xd2 },   /* ge11 */
    { SOC_BLK_XPORT,    0,    0,  0x50,    0xd0 },   /* xe0  */
    { SOC_BLK_XPORT,    1,    0,  0x51,    0xd1 },   /* xe1  */
    { SOC_BLK_XPORT,    2,    0,  0x52,    0xd2 },   /* xe2  */
    { SOC_BLK_XPORT,    3,    0,  0x53,    0xd3 },   /* xe3  */
    { SOC_BLK_GPORT,    1,    0,   0xc,    0x88 },   /* ge12 */
    { SOC_BLK_GPORT,    1,    1,   0xd,    0x89 },   /* ge13 */
    { SOC_BLK_GPORT,    1,    2,   0xe,    0x8a },   /* ge14 */
    { SOC_BLK_GPORT,    1,    3,   0xf,    0x8b },   /* ge15 */
    { SOC_BLK_GPORT,    1,    4,  0x10,    0x8c },   /* ge16 */
    { SOC_BLK_GPORT,    1,    5,  0x11,    0x8d },   /* ge17 */
    { SOC_BLK_GPORT,    1,    6,  0x12,    0x8e },   /* ge18 */
    { SOC_BLK_GPORT,    1,    7,  0x13,    0x8f },   /* ge19 */
    { SOC_BLK_GPORT,    1,    8,  0x14,    0xd3 },   /* ge20 */
    { SOC_BLK_GPORT,    1,    9,  0x15,    0xd3 },   /* ge21 */
    { SOC_BLK_GPORT,    1,   10,  0x16,    0xd3 },   /* ge22 */
    { SOC_BLK_GPORT,    1,   11,  0x17,    0xd3 },   /* ge23 */
    {            -1,   -1,   -1,   0x0,    0x00 }    /* Last */
};

static void
_fe2000_phy_addr_default(int unit, int port,
                         uint16 *phy_addr, uint16 *phy_addr_int)
{
    _block_phy_addr_t  *phy_info;
    int                block_type;
    int                block_number;
    int                block_index;

    *phy_addr = *phy_addr_int = 0x0;

    if (SOC_IS_SBX_FE2KXT(unit)) {
      phy_info = &_soc_phy_addr_bcm88025_a0[0];
    } else {
      phy_info = &_soc_phy_addr_bcm88020_a0[0];
    }

    /* Get physical block number and port offset */
    block_type   = SOC_PORT_BLOCK_TYPE(unit, port);
    block_number = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_index  = SOC_PORT_BLOCK_INDEX(unit, port);

    while (phy_info->block_type != -1) {
        if ((phy_info->block_type   == block_type)   &&
            (phy_info->block_number == block_number) ) {
            if ((block_type == SOC_BLK_XPORT) ||
                (phy_info->block_index == block_index)) {
                /* Found */
                *phy_addr     = phy_info->addr;
                *phy_addr_int = phy_info->addr_int;
                break;
            }
        }

        phy_info++;
    }
}

#ifdef BCM_SIRIUS_SUPPORT
/* Hypercores for hg port are on internal mdio bus only 
 *   Hg0-3 use Hc0-3
 *   Hg4-7 use Hc6-9
 *   SCI0/1 use Hc4, lane0/1
 *   SFI0/21 use Hc4, Lan2/3 to Hc9
 */
static _block_phy_addr_t   _soc_phy_addr_bcm88230[] = {
    { SOC_BLK_GXPORT,   0,    0,   0x0,    0x80 },  /* hg0  */
    { SOC_BLK_GXPORT,   1,    0,   0x0,    0x81 },  /* hg1  */
    { SOC_BLK_GXPORT,   2,    0,   0x0,    0x82 },  /* hg2  */
    { SOC_BLK_GXPORT,   3,    0,   0x0,    0x83 },  /* hg3  */
    { SOC_BLK_GXPORT,   4,    0,   0x0,    0x86 },  /* hg4  */
    { SOC_BLK_GXPORT,   5,    0,   0x0,    0x87 },  /* hg5  */
    { SOC_BLK_GXPORT,   6,    0,   0x0,    0x88 },  /* hg6  */
    { SOC_BLK_GXPORT,   7,    0,   0x0,    0x89 },  /* hg7  */
    { SOC_BLK_CMIC,     0,    0,   0x0,    0x00 },  /* cpu N/A */
    { SOC_BLK_SC_TOP,   0,    2,   0x0,    0x84 },  /* sfi0 */
    { SOC_BLK_SC_TOP,   0,    3,   0x0,    0x84 },  /* sfi1 */
    { SOC_BLK_SC_TOP,   0,    4,   0x0,    0x85 },  /* sfi2 */
    { SOC_BLK_SC_TOP,   0,    5,   0x0,    0x85 },  /* sfi3 */
    { SOC_BLK_SC_TOP,   0,    6,   0x0,    0x85 },  /* sfi4 */
    { SOC_BLK_SC_TOP,   0,    7,   0x0,    0x85 },  /* sfi5 */
    { SOC_BLK_SC_TOP,   0,    8,   0x0,    0x86 },  /* sfi6 */
    { SOC_BLK_SC_TOP,   0,    9,   0x0,    0x86 },  /* sfi7 */
    { SOC_BLK_SC_TOP,   0,   10,   0x0,    0x86 },  /* sfi8 */
    { SOC_BLK_SC_TOP,   0,   11,   0x0,    0x86 },  /* sfi9 */
    { SOC_BLK_SF_TOP,   0,    0,   0x0,    0x87 },  /* sfi10*/
    { SOC_BLK_SF_TOP,   0,    1,   0x0,    0x87 },  /* sfi11*/
    { SOC_BLK_SF_TOP,   0,    2,   0x0,    0x87 },  /* sfi12*/
    { SOC_BLK_SF_TOP,   0,    3,   0x0,    0x87 },  /* sfi13*/
    { SOC_BLK_SF_TOP,   0,    4,   0x0,    0x88 },  /* sfi14*/
    { SOC_BLK_SF_TOP,   0,    5,   0x0,    0x88 },  /* sfi15*/
    { SOC_BLK_SF_TOP,   0,    6,   0x0,    0x88 },  /* sfi16*/
    { SOC_BLK_SF_TOP,   0,    7,   0x0,    0x88 },  /* sfi17*/
    { SOC_BLK_SF_TOP,   0,    8,   0x0,    0x89 },  /* sfi18*/
    { SOC_BLK_SF_TOP,   0,    9,   0x0,    0x89 },  /* sfi19*/
    { SOC_BLK_SF_TOP,   0,   10,   0x0,    0x89 },  /* sfi20*/
    { SOC_BLK_SF_TOP,   0,   11,   0x0,    0x89 },  /* sfi21*/
    { SOC_BLK_SC_TOP,   0,    0,   0x0,    0x84 },  /* sci0 */
    { SOC_BLK_SC_TOP,   0,    1,   0x0,    0x84 },  /* sci1 */
    { SOC_BLK_EP,       0,    0,   0x0,    0x00 },  /* req0 N/A */
    { SOC_BLK_EP,       0,    1,   0x0,    0x00 },  /* req1 N/A */
    {             -1,   -1,    -1,   0x0,    0x0 }   /* Last */
};

static void
_sirius_phy_addr_default(int unit, int port,
                         uint16 *phy_addr, uint16 *phy_addr_int)
{
    _block_phy_addr_t  *phy_info;
    int                block_type;
    int                block_number;
    int                block_index;

    *phy_addr = *phy_addr_int = 0x0;

    phy_info = &_soc_phy_addr_bcm88230[0];

    /* Get physical block number and port offset */
    block_type   = SOC_PORT_BLOCK_TYPE(unit, port);
    block_number = SOC_PORT_BLOCK_NUMBER(unit, port);
    block_index  = SOC_PORT_BLOCK_INDEX(unit, port);

    soc_cm_debug(DK_PHY | DK_VERBOSE, "block_type %d, block_num %d, block_index %d\n",
		 block_type, block_number, block_index);

    while (phy_info->block_type != -1) {
        if ((phy_info->block_type   == block_type)   &&
            (phy_info->block_number == block_number) &&
            (phy_info->block_index  == block_index)) {
            /* Found */
            *phy_addr     = phy_info->addr;
            *phy_addr_int = phy_info->addr_int;
            break;
        }

        phy_info++;
    }
}
#endif /* BCM_SIRIUS_SUPPORT */

#endif /* BCM_SBX_SUPPORT */

#ifdef BCM_HAWKEYE_SUPPORT
static void
_hawkeye_phy_addr_default(int unit, int port,
                           uint16 *phy_addr, uint16 *phy_addr_int)
{
    static const uint16 _soc_phy_addr_bcm53314_a0[] = {
    0x80, 0x0, /* Port  0 (cmic) N/A */
    0x81, 0x0, /* Port  1 ( ge0) ExtBus=0 ExtAddr=0x01 */
    0x82, 0x0, /* Port  2 ( ge1) ExtBus=0 ExtAddr=0x02 */
    0x83, 0x0, /* Port  3 ( ge2) ExtBus=0 ExtAddr=0x03 */
    0x84, 0x0, /* Port  4 ( ge3) ExtBus=0 ExtAddr=0x04 */
    0x85, 0x0, /* Port  5 ( ge4) ExtBus=0 ExtAddr=0x05 */
    0x86, 0x0, /* Port  6 ( ge5) ExtBus=0 ExtAddr=0x06 */
    0x87, 0x0, /* Port  7 ( ge6) ExtBus=0 ExtAddr=0x07 */
    0x88, 0x0, /* Port  8 ( ge7) ExtBus=0 ExtAddr=0x08 */
    0x09, 0x89, /* Port  9 ( ge8) ExtBus=0 ExtAddr=0x09 IntBus=0 IntAddr=0x09 */
    0x0a, 0x89, /* Port 10 ( ge9) ExtBus=0 ExtAddr=0x0a IntBus=0 IntAddr=0x09 */
    0x0b, 0x89, /* Port 11 (ge10) ExtBus=0 ExtAddr=0x0b IntBus=0 IntAddr=0x09 */
    0x0c, 0x89, /* Port 12 (ge11) ExtBus=0 ExtAddr=0x0c IntBus=0 IntAddr=0x09*/
    0x0d, 0x89, /* Port 13 (ge12) ExtBus=0 ExtAddr=0x0d IntBus=0 IntAddr=0x09 */
    0x0e, 0x89, /* Port 14 (ge13) ExtBus=0 ExtAddr=0x0e IntBus=0 IntAddr=0x09 */
    0x0f, 0x89, /* Port 15 (ge14) ExtBus=0 ExtAddr=0x0f IntBus=0 IntAddr=0x09 */
    0x10, 0x89, /* Port 16 (ge15) ExtBus=0 ExtAddr=0x10 IntBus=0 IntAddr=0x09 */
    0x12, 0x91, /* Port 17 (ge16) ExtBus=0 ExtAddr=0x12 IntBus=0 IntAddr=0x11 */
    0x13, 0x91, /* Port 18 (ge17) ExtBus=0 ExtAddr=0x13 IntBus=0 IntAddr=0x11 */
    0x14, 0x91, /* Port 19 (ge18) ExtBus=0 ExtAddr=0x14 IntBus=0 IntAddr=0x11 */
    0x15, 0x91, /* Port 20 (ge19) ExtBus=0 ExtAddr=0x15 IntBus=0 IntAddr=0x11 */
    0x16, 0x91, /* Port 21 (ge20) ExtBus=0 ExtAddr=0x16 IntBus=0 IntAddr=0x11 */
    0x17, 0x91, /* Port 22 (ge21) ExtBus=0 ExtAddr=0x17 IntBus=0 IntAddr=0x11 */
    0x18, 0x91, /* Port 23 (ge22) ExtBus=0 ExtAddr=0x18 IntBus=0 IntAddr=0x11 */
    0x19, 0x91, /* Port 24 (ge23) ExtBus=0 ExtAddr=0x19 IntBus=0 IntAddr=0x11 */
    };

    uint16 dev_id;
    uint8 rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    switch (dev_id) {
    case BCM53312_DEVICE_ID:
    case BCM53313_DEVICE_ID:
    case BCM53314_DEVICE_ID:
    case BCM53322_DEVICE_ID:
    case BCM53323_DEVICE_ID:
    case BCM53324_DEVICE_ID:
        *phy_addr = _soc_phy_addr_bcm53314_a0[port * 2];
        *phy_addr_int = _soc_phy_addr_bcm53314_a0[port * 2 + 1];
        break;
    }
}
#endif /* BCM_HAWKEYE_SUPPORT */

#ifdef BCM_ROBO_SUPPORT

static void
_robo_phy_addr_default(int unit, int port,
                         uint16 *phy_addr, uint16 *phy_addr_int)
{

    /* assigning the initial phy_addr */
    _ROBO_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    
    /* Specific assinging section for those ROBO devices within built-in 
     * SerDes design. (like, bcm5396/5348/5347/53115)
     *  - For Robo device, here we assign a specifal flag at the highest bit
     *      to indicate that there is a internal SerDes built-in on this port.
     *      And the physical phy_addr on ext/int site will be retrived again 
     *      when doing miim_read/write.
     * 
     * Note : 
     *  1. The usage for internal phy_addr flag causes the max available 
     *      phy_addr been reduced from 255 to 127. In case a ROBO device might 
     *      consist of more than 127 ports on a single unit than this design 
     *      for internal phy_add flag have to change.
     *  2. The phy_addr is a logical phy_addr reflect to the PHY-ID of a port 
     *      on a device.
     */
    if (IS_ROBO_SPECIFIC_INT_SERDES(unit, port)){
        *phy_addr_int |= PHY_ADDR_ROBO_INT_SERDES; 
    }
}
#endif /* BCM_ROBO_SUPPORT */

/*
 * Function:
 *      _soc_phy_addr_default
 * Purpose:
 *      Return the default PHY addresses used to initialize the PHY map
 *      for a port.
 * Parameters:
 *      unit - StrataSwitch unit number
 *      phy_addr - (OUT) Outer PHY address
 *      phy_addr_int - (OUT) Intermediate PHY address, 0xff if none
 */

static void
_soc_phy_addr_default(int unit, int port,
                      uint16 *phy_addr, uint16 *phy_addr_int)
{

#ifdef BCM_XGS12_FABRIC_SUPPORT
    if (SOC_IS_XGS12_FABRIC(unit)) {
        _XGS12_FABRIC_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_XGS12_FABRIC_SUPPORT */

#ifdef BCM_DRACO_SUPPORT
    if (SOC_IS_DRACO(unit)) {
        _DRACO_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_DRACO_SUPPORT */

#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        _LYNX_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_LYNX_SUPPORT */

#ifdef BCM_TUCANA_SUPPORT
    if (SOC_IS_TUCANA(unit)) {
        _TUCANA_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_TUCANA_SUPPORT */

#ifdef BCM_RAPTOR_SUPPORT
    if (SOC_IS_RAPTOR(unit) || SOC_IS_RAVEN(unit)) {
        _RAPTOR_PHY_ADDR_DEFAULT(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_RAPTOR_SUPPORT */

#if defined(BCM_ENDURO_SUPPORT)
    if (SOC_IS_ENDURO(unit)) {
        _enduro_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_ENDURO_SUPPORT */

#if defined(BCM_HURRICANE_SUPPORT)
    if (SOC_IS_HURRICANE(unit)) {
        _hurricane_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_HURRICANE_SUPPORT */

#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT) || \
    defined(BCM_VALKYRIE2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
        SOC_IS_VALKYRIE2(unit)) {
        _triumph2_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_IS_TRIUMPH(unit)) {
        _triumph_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_VALKYRIE_SUPPORT
    if (SOC_IS_VALKYRIE(unit)) {
        _valkyrie_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_VALKYRIE_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        _trident_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        _shadow_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_SCORPION_SUPPORT
    if (SOC_IS_SCORPION(unit) &&
        _scorpion_phy_addr_default(unit, port, phy_addr, phy_addr_int)) {
    } else
#endif /* BCM_SCORPION_SUPPORT */

#ifdef BCM_HAWKEYE_SUPPORT
    if (SOC_IS_HAWKEYE(unit)) {
        _hawkeye_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_HAWKEYE_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        _xgs3_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        _robo_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_ROBO_SUPPORT */

#ifdef BCM_SBX_SUPPORT
    if (SOC_IS_SBX_FE2000(unit) ||
	SOC_IS_SBX_FE2KXT(unit) ) {
        _fe2000_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_FE2000_SUPPORT */

#ifdef BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)) {
        _bm9600_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_BM9600_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        _sirius_phy_addr_default(unit, port, phy_addr, phy_addr_int);
    } else
#endif /* BCM_SIRIUS_SUPPORT */

    {
        *phy_addr     = port + 1;
        *phy_addr_int = 0xff;
    }

    /*
     * Override the calculated address(es) with the per-port properties
     */
    *phy_addr = soc_property_port_get(unit, port,
                                      spn_PORT_PHY_ADDR,
                                      *phy_addr);
}

int
soc_phy_deinit(int unit)
{
    sal_free(port_phy_addr[unit]);
    port_phy_addr[unit] = NULL;
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_init
 * Purpose:
 *      Initialize PHY software subsystem.
 * Parameters:
 *      unit - StrataSwitch unit number
 * Returns:
 *      SOC_E_XXX
 */

int
soc_phy_init(int unit)
{
    uint16              phy_addr=0, phy_addr_int=0;
    soc_port_t          port;

    if (_phys_in_table < 0) {
        _init_phy_table();
    }

    if (port_phy_addr[unit] == NULL) {
        port_phy_addr[unit] = sal_alloc(sizeof(port_phy_addr_t) *
                                        SOC_MAX_NUM_PORTS,
                                        "port_phy_addr");
        if (port_phy_addr[unit] == NULL) {
            return SOC_E_MEMORY;
        }
    }

    sal_memset(port_phy_addr[unit], 0,
               sizeof(port_phy_addr_t) * SOC_MAX_NUM_PORTS);

    PBMP_PORT_ITER(unit, port) {
        _soc_phy_addr_default(unit, port, &phy_addr, &phy_addr_int);

        SOC_IF_ERROR_RETURN
            (soc_phy_cfg_addr_set(unit,port,0, phy_addr));
        SOC_IF_ERROR_RETURN
            (soc_phy_cfg_addr_set(unit,port,SOC_PHY_INTERNAL, phy_addr_int));

        PHY_ADDR(unit, port)     = phy_addr;
        PHY_ADDR_INT(unit, port) = phy_addr_int;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_add_entry
 * Purpose:
 *      Add an entry to the PHY table
 * Parameters:
 *      entry - pointer to the entry
 * Returns:
 *      SOC_E_NONE - no error
 *      SOC_E_INIT - not initialized
 *      SOC_E_MEMORY - not no more space in table.
 */

int
soc_phy_add_entry(soc_phy_table_t *entry)
{
    assert(_phys_in_table >= 0);        /* Fatal if not already inited */

    if (_phys_in_table >= _MAX_PHYS) {
        return SOC_E_MEMORY;
    }

    phy_table[_phys_in_table++] = entry;

    return SOC_E_NONE;
}

#if defined(INCLUDE_SERDES_COMBO65)
/*
 * Function:
 *      _chk_serdescombo65
 * Purpose:
 *      Check function for Raven Combo SerDes PHYs
 * Parameters:
 *      See soc_phy_ident_f
 *      *pi is an output parameter filled with PHY info.
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      Sets the phy_info structure for this port.
 */

static int
_chk_serdescombo65(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(unit);

    if (SOC_IS_RAVEN(unit)) {
        if (port == 1 || port == 2 || port == 4 || port == 5) {
            pi->phy_name = my_entry->phy_name;
            return TRUE;
        }
    }

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        if(SOC_IS_TB(unit) && IS_S_PORT(unit, port)){
            pi->phy_name = my_entry->phy_name;
            return TRUE;
        }
    }
#endif /* BCM_ROBO_SUPPORT */

    return FALSE;
}

#endif /* INCLUDE_SERDES_COMBO65 */

#if defined(INCLUDE_XGXS_16G)
/*
 * Function:
 *      _chk_phy
 * Purpose:
 *      Standard check function for PHYs (see soc_phy_ident_f)
 * Parameters:
 *      See soc_phy_ident_f
 *      *pi is an output parameter filled with PHY info.
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      Sets the phy_info structure for this port.
 */

static int
_chk_xgxs16g1l(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1)) {
        if (SOC_IS_SCORPION(unit) && !IS_GX_PORT(unit, port)) {
            pi->phy_name = my_entry->phy_name;
            return TRUE;
        }

#ifdef BCM_SBX_SUPPORT
	if (SOC_IS_SBX_FE2KXT(unit) && IS_GE_PORT(unit,port)) {
            pi->phy_name = my_entry->phy_name;
            return TRUE;	  
	}
#endif /* BCM_SBX_SUPPORT */

#ifdef BCM_ENDURO_SUPPORT
	if (SOC_IS_ENDURO(unit) && (port > 25) && IS_GE_PORT(unit,port)) {
            pi->phy_name = my_entry->phy_name;
            return TRUE;	  
	}
#endif /* BCM_ENDURO_SUPPORT */
    }
    return FALSE;
}
#endif /* INCLUDE_XGXS_16G */

/*
 * Function:
 *      _chk_phy
 * Purpose:
 *      Standard check function for PHYs (see soc_phy_ident_f)
 * Parameters:
 *      See soc_phy_ident_f
 *      *pi is an output parameter filled with PHY info.
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      Sets the phy_info structure for this port.
 */

static int
_chk_phy(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(unit);

    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }

    return FALSE;
}

#if defined(INCLUDE_PHY_SIMUL)
#if defined(SIM_ALL_PHYS)
#define USE_SIMULATION_PHY(unit, port)  (TRUE)
#else
#define USE_SIMULATION_PHY(unit, port) \
     (soc_property_port_get(unit, port, spn_PHY_SIMUL, 0))
#endif
#else
#define USE_SIMULATION_PHY(unit, port)  (FALSE)
#endif

/*
 * Function:
 *      _chk_null
 * Purpose:
 *      Check function for NULL phys.
 * Returns:
 *      True if this phy matches.
 *      False otherwise.
 */

static int
_chk_null(int unit, soc_port_t port,  soc_phy_table_t *my_entry,
          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if ((SAL_BOOT_PLISIM && (!SAL_BOOT_RTLSIM &&
        !USE_SIMULATION_PHY(unit, port))) ||
        !soc_property_get(unit, spn_PHY_ENABLE, 1) ||
        soc_property_port_get(unit, port, spn_PHY_NULL, 0)) {
        pi->phy_name = my_entry->phy_name;

        return TRUE;
    }

    return FALSE;
}

#if defined(BCM_XGS3_SWITCH_SUPPORT)
/*
 * Function:
 *      _chk_gmii
 * Purpose:
 *      Check function for GMII port.
 * Returns:
 *      True if this phy matches.
 *      False otherwise.
 */

static int
_chk_gmii(int unit, soc_port_t port,  soc_phy_table_t *my_entry,
          uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (IS_GMII_PORT(unit, port)) {
        pi->phy_name = my_entry->phy_name;

        return TRUE;
    }

    return FALSE;
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(INCLUDE_PHY_SIMUL)

/*
 * Function:
 *      _chk_simul
 * Purpose:
 *      Check function for simulation phys.
 * Returns:
 *      True if this phy matches.
 *      False otherwise.
 */

static int
_chk_simul(int unit, soc_port_t port,  soc_phy_table_t *my_entry,
           uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (USE_SIMULATION_PHY(unit, port)) {
        pi->phy_name = my_entry->phy_name;

        return TRUE;
    }

    return FALSE;
}
#endif /* include phy sim */

#undef USE_SIMULATION_PHY

#ifdef BCM_DRACO_SUPPORT
/*
 * Function:
 *      _chk_fiber5690
 * Purpose:
 *      Check for using Internal 5690 SERDES Device for fiber
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_fiber5690(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);
    COMPILER_REFERENCE(pi);

    if (SOC_IS_DRACO(unit) && IS_GE_PORT(unit, port)) {
        pi->phy_name = "Phy5690";
        return TRUE;
    }

    return FALSE;
}
#endif /* BCM_DRACO_SUPPORT */

#ifdef INCLUDE_XGXS_QSGMII65
/*
 * Function:
 *      _chk_qsgmii53314
 * Purpose:
 *      Check for using Internal 53314 SERDES Device 
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_qsgmii53314(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);
    COMPILER_REFERENCE(pi);

    if (SOC_IS_HAWKEYE(unit) && port >= 9) {
        pi->phy_name = "Phy53314";
        return TRUE;
    }

    if (IS_GE_PORT(unit, port) && (SOC_IS_HURRICANE(unit) && (port >= 2) && (port <= 25))) {
        pi->phy_name = "Phy53314";
        return TRUE;
    }

    return FALSE;
}
#endif /* INCLUDE_XGXS_QSGMII65 */

#ifdef INCLUDE_PHY_54680
/*
 * Function:
 *      _chk_qgphy_5332x
 * Purpose:
 *      Check for using 5332x QGPHY Device 
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      Because The Id of QGPHY device of HKEEE is same as BCM54682E,
 *      the _chk_phy can be used for QGPHY device of HKEEE
 */

static int
_chk_qgphy_5332x(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);
    COMPILER_REFERENCE(pi);

    if (SOC_IS_HAWKEYE(unit) && 
        soc_feature (unit, soc_feature_eee) && 
        (port <= 8) && 
        (port != 0)) {
        pi->phy_name = "BCM53324";
        return TRUE;
    }

    return FALSE;
}
#endif /* INCLUDE_PHY_54680 */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
/*
 * Function:
 *      _chk_fiber56xxx
 * Purpose:
 *      Check for using Internal 56XXX SERDES Device for fiber
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_fiber56xxx(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);

    if (IS_GE_PORT(unit, port) && !IS_GMII_PORT(unit, port) &&
        !SOC_IS_HAWKEYE(unit) &&
        soc_feature(unit, soc_feature_dodeca_serdes)) {
        pi->phy_name = "Phy56XXX";

        return TRUE;
    }

    return FALSE;
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#if defined(INCLUDE_PHY_XGXS6)
/*
 * Function:
 *      _chk_unicore
 * Purpose:
 *      Check for Unicore, which may return two different PHY IDs
 *      depending on the current IEEE register mapping. One of
 *      these PHY IDs conflicts with the BCM5673/74 PHY ID, so
 *      we need to check the port type here as well.
 * Parameters:
 *      See soc_phy_ident_f
 *      *pi is an output parameter filled with PHY info.
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      Sets the phy_info structure for this port.
 */

static int
_chk_unicore(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(unit);

    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1) &&
        soc_feature(unit, soc_feature_xgxs_v6)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }

    return FALSE;
}
#endif /* INCLUDE_PHY_XGXS6 */

#if defined(INCLUDE_SERDES_ASSUMED)
/*
 * Function:
 *      _chk_serdes
 * Purpose:
 *      Check for Assumed SERDES Device
 * Returns:
 *      True if this PHY matches.
 *      False otherwise.
 */

static int
_chk_serdes(int unit, soc_port_t port, soc_phy_table_t *my_entry,
            uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);
    COMPILER_REFERENCE(pi);

    if (IS_GE_PORT(unit, port) && (!SOC_IS_ROBO(unit))) {
        if (!SOC_IS_XGS(unit) && !SOC_IS_DRACO(unit)) {
            pi->phy_flags |= PHY_FLAGS_10B;
            pi->phy_name = "Serdes";
            return TRUE;
        } else
        {
            return FALSE;
        }
    }

    return FALSE;
}
#endif /* INCLUDE_SERDES_ASSUMED */

static int
_chk_sfp_phy(int unit, soc_port_t port, soc_phy_table_t *my_entry,
         uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(unit);

    if (soc_property_port_get(unit, port, spn_PHY_COPPER_SFP, 0)) {
        if (!(phy_id0 == (uint16)0xFFFF && phy_id1 == (uint16)0xFFFF)) {
            SOC_DEBUG_PRINT((DK_PHY,
                "_chk_sfp_phy: u=%d p=%d id0=0x%x, id1=0x%x,"
                " oui=0x%x,model=0x%x,rev=0x%x\n",
                         unit, port,phy_id0,phy_id1,PHY_OUI(phy_id0, phy_id1),
                         PHY_MODEL(phy_id0, phy_id1),PHY_REV(phy_id0, phy_id1)));
            pi->phy_name = my_entry->phy_name;
            return TRUE;
        }
    }
    return FALSE;
}

#if defined(INCLUDE_PHY_8706)
static int
_chk_8706(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1) ||
        soc_property_port_get(unit, port, spn_PHY_8706, FALSE)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }
    return FALSE;
}

#endif /* INCLUDE_PHY_8706 */

#if defined(INCLUDE_PHY_8040)
static int
_chk_8040(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }
    return FALSE;
}
#endif /* INCLUDE_PHY_8040 */

#if defined(INCLUDE_PHY_8072)
static int
_chk_8072(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1) ||
        soc_property_port_get(unit, port, spn_PHY_8072, FALSE)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }
    return FALSE;
}

#endif /* INCLUDE_PHY_8072 */

#if defined(INCLUDE_PHY_8481)
static int
_chk_8481(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{

    if (!IS_XE_PORT(unit, port) && !IS_HG_PORT(unit, port)) {
        return FALSE;
    }

    if (my_entry->myNum == _phy_ident_type_get(phy_id0, phy_id1)) {
        pi->phy_name = my_entry->phy_name;
        return TRUE;
    }

    return FALSE;

}
#endif /* INCLUDE_PHY_8481 */

#if defined(INCLUDE_PHY_56XXX)
/*
 * Function:
 *      _chk_fiber56xxx_5601x
 * Purpose:
 *      Check for using Internal SERDES Device for fiber with shadow registers
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_fiber56xxx_5601x(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    uint8       phy_addr;

    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);

    phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);

    if (SOC_IS_RAPTOR(unit) && 
        soc_feature(unit, soc_feature_fe_ports) &&
        ((port == 4) || (port == 5)) &&
        (soc_property_port_get(unit, port, spn_SERDES_SHADOW_DRIVER, FALSE) ||
         _is_corrupted_reg(unit, phy_addr))) {
        pi->phy_name = "Phy56XXX5601x";
        return TRUE;
    }

    return FALSE;
}
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_SERDES_COMBO)
/*
 * Function:
 *      _chk_serdes_combo_5601x
 * Purpose:
 *      Check for using Internal SERDES Device for fiber with shadow registers
 * Returns:q
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_serdes_combo_5601x(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    uint8       phy_addr;

    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);

    phy_addr = PORT_TO_PHY_ADDR_INT(unit, port);

    if (SOC_IS_RAPTOR(unit) && 
        soc_feature(unit, soc_feature_fe_ports) &&
        ((port == 1) || (port == 2)) &&
        (soc_property_port_get(unit, port, spn_SERDES_SHADOW_DRIVER, FALSE) ||
         _is_corrupted_reg(unit, phy_addr))) {
        pi->phy_name = "COMBO5601x";
        return TRUE;
    }    

    return FALSE;
}
#endif /* INCLUDE_SERDES_COMBO */

#if defined(INCLUDE_PHY_53XXX)
/*
 * Function:
 *      _chk_fiber53xxx
 * Purpose:
 *      Check for using Internal 53XXX SERDES Device for fiber
 * Returns:
 *      TRUE if this PHY matches.
 *      FALSE otherwise.
 * Notes:
 *      This routine should be called after checking for external
 *      PHYs because it will default the PHY driver to the internal
 *      SERDES in the absense of any other PHY.
 */

static int
_chk_fiber53xxx(int unit, soc_port_t port, soc_phy_table_t *my_entry,
               uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    COMPILER_REFERENCE(my_entry);
    COMPILER_REFERENCE(phy_id0);
    COMPILER_REFERENCE(phy_id1);

    if (IS_GE_PORT(unit, port) && !IS_GMII_PORT(unit, port) &&
        soc_feature(unit, soc_feature_dodeca_serdes) && 
        SOC_IS_ROBO(unit)) {
        pi->phy_name = "Phy53XXX";

        return TRUE;
    }

    return FALSE;
}
#endif /* INCLUDE_PHY_53XXX */
/*
 * Function:
 *      _chk_default
 * Purpose:
 *      Select a default PHY driver.
 * Returns:
 *      TRUE
 * Notes:
 *      This routine always "finds" a default PHY driver and can
 *      be the last entry in the PHY table (or called explicitly).
 */

static int
_chk_default(int unit, soc_port_t port, soc_phy_table_t *my_entry,
             uint16 phy_id0, uint16 phy_id1, soc_phy_info_t *pi)
{
    pi->phy_name = my_entry->phy_name;

    return TRUE;
}

STATIC int
_forced_phy_probe(int unit, soc_port_t port,
                  soc_phy_info_t *pi, phy_ctrl_t *ext_pc)
{
    phy_driver_t   *phyd;
    char *s;

    phyd = NULL;
#if defined(INCLUDE_PHY_SIMUL)
    /* Similarly, check for simulation driver */
    if (phyd == NULL &&
        _chk_simul(unit, port, &_simul_phy_entry, 0xffff, 0xffff, pi)) {
        ext_pc->pd  = _simul_phy_entry.driver;
        pi->phy_id0 = 0xffff;
        pi->phy_id1 = 0xffff;
    }
#endif

#ifdef BCM_DRACO_SUPPORT
    /* Check for property forcing fiber mode on Draco */
    if (phyd == NULL &&
        soc_property_port_get(unit, port, spn_PHY_5690, FALSE) &&
        _chk_fiber5690(unit, port, &_fiber5690_phy_entry,
                       0xffff, 0xffff, pi)) {
        ext_pc->pd  = _fiber5690_phy_entry.driver;
        pi->phy_id0 = 0xffff;
        pi->phy_id1 = 0xffff;
    }
#endif  /* BCM_DRACO_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    /* Check for property forcing fiber mode on XGS3 switch */
    if (phyd == NULL &&
        soc_property_port_get(unit, port, spn_PHY_56XXX, FALSE) &&
        _chk_fiber56xxx(unit, port, &_fiber56xxx_phy_entry,
                        0xffff, 0xffff, pi)) {
        /* Correct internal SerDes PHY driver is already attached by
         * internal PHY probe. Therefore, just assign NULL to external PHY
         * driver.
         */
        ext_pc->pd   = NULL;
        pi->phy_id0  = 0xffff;
        pi->phy_id1  = 0xffff;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        /* force the external PHY driver on TB's FE port in FPGA board. 
         *  - MAC is fe ports but GE PHY ports designed in FPGA board
         */
        if (phyd == NULL && IS_FE_PORT(unit, port)){
            soc_cm_print("_forced_phy_probe(),[FPGA]:port %d, ", port);
            if (ext_pc->pd == NULL){
                soc_cm_print("No external PHY connected!\n");
            } 
#if defined(INCLUDE_PHY_5464_ROBO) && defined(INCLUDE_PHY_522X)
            else if (ext_pc->pd == &phy_5464robodrv_ge){
                ext_pc->pd   = &phy_522xdrv_fe;
                soc_cm_print("FE driver for bcm5464 PHY!\n");
            } 
#endif /* defined(INCLUDE_PHY_5464_ROBO) && defined(INCLUDE_PHY_522X) */
            else {
                soc_cm_print("Unexpected PHY connected!\n");
            }
        }
    }
    /* Forced PHY will have internal PHY device ID */
    return SOC_E_NONE;
}

STATIC int
_int_phy_probe(int unit, soc_port_t port,
               soc_phy_info_t *pi, phy_ctrl_t *int_pc)
{
    uint16               phy_addr;
    uint16               phy_id0, phy_id1;
    int                  i;
    int                  rv;
    phy_driver_t         *int_phyd;

    phy_addr = int_pc->phy_id;
    int_phyd = NULL;
    i = sizeof(_int_phy_table) / sizeof(_int_phy_table[0]);

    /* Make sure page 0 is mapped before reading for PHY dev ID */
    (void)int_pc->write(unit, phy_addr, 0x1f, 0);

    (void)int_pc->read(unit, phy_addr, MII_PHY_ID0_REG, &phy_id0);
    (void)int_pc->read(unit, phy_addr, MII_PHY_ID1_REG, &phy_id1);

    
    pi->phy_id0       = phy_id0;
    pi->phy_id1       = phy_id1;
    pi->phy_addr_int  = phy_addr;
    int_pc->phy_id0   = phy_id0;
    int_pc->phy_id1   = phy_id1;
    int_pc->phy_oui   = PHY_OUI(phy_id0, phy_id1);
    int_pc->phy_model = PHY_MODEL(phy_id0, phy_id1);
    int_pc->phy_rev   = PHY_REV(phy_id0, phy_id1);

    for (i = i - 1; i >= 0; i--) {
        if ((_int_phy_table[i].checkphy)(unit, port, &_int_phy_table[i],
                                         phy_id0, phy_id1, pi)) {
            /* Device ID matches. Calls driver probe routine to confirm
             * that the driver is the appropriate one.
             * Many PHY devices has the same device ID but they are
             * actually different.
             */
            rv = PHY_PROBE(_int_phy_table[i].driver, unit, int_pc);
            if ((rv == SOC_E_NONE) || (rv == SOC_E_UNAVAIL)) {
                SOC_DEBUG_PRINT((DK_PHY, "<%d> Index = %d Mynum = %d %s\n",
                    rv, i, _int_phy_table[i].myNum, _int_phy_table[i].phy_name));
                int_phyd = _int_phy_table[i].driver;
                break;
            }
        }
    }
#if defined(INCLUDE_XGXS_16G)
    if (IS_GE_PORT(unit, port) && (&phy_xgxs16g1l_ge == int_phyd)) {
        /* using XGXS16G in independent lane mode on GE port.  */
#if defined(BCM_FE2000_SUPPORT)
        if (SOC_IS_SBX_FE2KXT(unit)) {
	  int_pc->flags |= PHYCTRL_MDIO_ADDR_SHARE;
            switch(port) {
            case 20:
            case 8:
                int_pc->lane_num = 0;
                break;
                                                                                
            case 21:
	    case 9:
                int_pc->lane_num = 1;
                break;
            case 22:
	    case 10:
                int_pc->lane_num = 2;
                break;
            case 23:
	    case 11:
                int_pc->lane_num = 3;
                break;
            default:
                break;
            }
        }
#endif
#if defined(BCM_SCORPION_SUPPORT)
        if (SOC_IS_SCORPION(unit)) {
            switch(port) {
            case 25:
                pi->phy_name = "XGXS16G/1/0";
                int_pc->lane_num = 0;
                break;
            case 26:
                pi->phy_name = "XGXS16G/1/1";
                int_pc->lane_num = 1;      
                break;
            case 27:
                pi->phy_name = "XGXS16G/1/2";
                int_pc->lane_num = 2;
                break;
            case 28:
                pi->phy_name = "XGXS16G/1/3";
                int_pc->lane_num = 3;
                break;
            default:
                break;
            }
        }
#endif
    }
#endif

#if defined(INCLUDE_XGXS_HL65)
    if (IS_GE_PORT(unit, port) && (&phy_hl65_hg == int_phyd)) {
        /* If using HyperLite on GE port, use the HyperLite in independent
         * lane mode.
         */
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_INDEPENDENT_LANE);
        pi->phy_name = "HL65/1";
#if defined(BCM_HURRICANE_SUPPORT)
        if (SOC_IS_HURRICANE(unit)) {
            int_pc->phy_mode = PHYCTRL_DUAL_LANE_PORT;
            int_pc->flags |= PHYCTRL_MDIO_ADDR_SHARE;
            switch (port) {
                case 26:
                    pi->phy_name = "HL65/0/0";
                    int_pc->lane_num = 0;
                    break;
                case 27:
                    pi->phy_name = "HL65/0/2";
                    int_pc->lane_num = 2;
                    break;
                case 28:
                    pi->phy_name = "HL65/1/0";
                    int_pc->lane_num = 0;
                    break;
                case 29:
                    pi->phy_name = "HL65/1/2";
                    int_pc->lane_num = 2;
                    break;
                default:
                    break;
            }
        }
#endif
#if defined(BCM_VALKYRIE_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
        if (SOC_IS_VALKYRIE(unit) || SOC_IS_TRIUMPH(unit)) {
            switch(port) {
            case 2:
            case 6:
            case 14:
            case 26:
            case 27:
            case 35:
                pi->phy_name = "HL65/1/0";
                int_pc->lane_num = 0;
                break;

            case 3:
            case 7:
            case 15:
            case 32:
            case 36:
            case 43:
                pi->phy_name = "HL65/1/1";
                int_pc->lane_num = 1;
                break;

            case 4:
            case 16:
            case 18:
            case 33:
            case 44:
            case 46:
                pi->phy_name = "HL65/1/2";
                int_pc->lane_num = 2;
                break;

            case 5:
            case 17:
            case 19:
            case 34:
            case 45:
            case 47:
                pi->phy_name = "HL65/1/3";
                int_pc->lane_num = 3;
                break;

            default:
                break;

            }
        }
#endif /* BCM_VALKYRIE_SUPPORT */
#if defined(BCM_TRIUMPH2_SUPPORT) || defined(BCM_APOLLO_SUPPORT) || \
    defined(BCM_VALKYRIE2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit)) {
            switch(port) {
            case 30:
            case 34:
            case 38:
            case 42:
            case 46:
            case 50:
                pi->phy_name = "HL65/1/0";
                int_pc->lane_num = 0;
                break;

            case 31:
            case 35:
            case 39:
            case 43:
            case 47:
            case 51:
                pi->phy_name = "HL65/1/1";
                int_pc->lane_num = 1;
                break;

            case 32:
            case 36:
            case 40:
            case 44:
            case 48:
            case 52:
                pi->phy_name = "HL65/1/2";
                int_pc->lane_num = 2;
                break;

            case 33:
            case 37:
            case 41:
            case 45:
            case 49:
            case 53:
                pi->phy_name = "HL65/1/3";
                int_pc->lane_num = 3;
                break;

            default:
                break;

            }
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

#if defined(BCM_HURRICANE_SUPPORT)
        if ((&phy_hl65_hg == int_phyd) && SOC_IS_HURRICANE(unit) && ((IS_HG_PORT(unit, port)) || IS_XE_PORT(unit, port))) {
            if ((port == 26) && (!SOC_PORT_VALID(unit, 27))) {
                SOC_DEBUG_PRINT((DK_PHY, "Port 26 in combo mode\n"));
                pi->phy_name = "HL65/0";
            } else if ((port == 28) && (!SOC_PORT_VALID(unit, 29))) {
                SOC_DEBUG_PRINT((DK_PHY, "Port 28 in combo mode\n"));
                pi->phy_name = "HL65/1";
            } else {
                SOC_DEBUG_PRINT((DK_PHY, "Port %d in HGd mode\n", port));
                PHY_FLAGS_SET(unit, port, PHY_FLAGS_INDEPENDENT_LANE);
                int_pc->phy_mode = PHYCTRL_DUAL_LANE_PORT;
                int_pc->flags |= PHYCTRL_MDIO_ADDR_SHARE;
                switch (port) {
                    case 26:
                        pi->phy_name = "HL65/0/0";
                        int_pc->lane_num = 0;
                        break;
                    case 27:
                        pi->phy_name = "HL65/0/2";
                        int_pc->lane_num = 2;
                        break;
                    case 28:
                        pi->phy_name = "HL65/1/0";
                        int_pc->lane_num = 0;
                        break;
                    case 29:
                        pi->phy_name = "HL65/1/2";
                        int_pc->lane_num = 2;
                        break;
                    default:
                        break;
                }
            }
        }
#endif /* BCM_HURRICANE_SUPPORT */

#if defined(BCM_BM9600_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (&phy_hl65_hg == int_phyd) {
        if (SOC_IS_SBX_BM9600(unit)) {
            static char *bm9600_phy_names[] = {
                "HC65/0/0",  "HC65/0/1",  "HC65/0/2",  "HC65/0/3",  
                "HC65/1/0",  "HC65/1/1",  "HC65/1/2",  "HC65/1/3",  
                "HC65/2/0",  "HC65/2/1",  "HC65/2/2",  "HC65/2/3",  
                "HC65/3/0",  "HC65/3/1",  "HC65/3/2",  "HC65/3/3",  
                "HC65/4/0",  "HC65/4/1",  "HC65/4/2",  "HC65/4/3",  
                "HC65/5/0",  "HC65/5/1",  "HC65/5/2",  "HC65/5/3",  
                "HC65/6/0",  "HC65/6/1",  "HC65/6/2",  "HC65/6/3",  
                "HC65/7/0",  "HC65/7/1",  "HC65/7/2",  "HC65/7/3",  
                "HC65/8/0",  "HC65/8/1",  "HC65/8/2",  "HC65/8/3",  
                "HC65/9/0",  "HC65/9/1",  "HC65/9/2",  "HC65/9/3",  
                "HC65/10/0", "HC65/10/1", "HC65/10/2", "HC65/10/3", 
                "HC65/11/0", "HC65/11/1", "HC65/11/2", "HC65/11/3", 
                "HC65/12/0", "HC65/12/1", "HC65/12/2", "HC65/12/3", 
                "HC65/13/0", "HC65/13/1", "HC65/13/2", "HC65/13/3", 
                "HC65/14/0", "HC65/14/1", "HC65/14/2", "HC65/14/3", 
                "HC65/15/0", "HC65/15/1", "HC65/15/2", "HC65/15/3", 
                "HC65/16/0", "HC65/16/1", "HC65/16/2", "HC65/16/3", 
                "HC65/17/0", "HC65/17/1", "HC65/17/2", "HC65/17/3", 
                "HC65/18/0", "HC65/18/1", "HC65/18/2", "HC65/18/3", 
                "HC65/19/0", "HC65/19/1", "HC65/19/2", "HC65/19/3", 
                "HC65/20/0", "HC65/20/1", "HC65/20/2", "HC65/20/3", 
                "HC65/21/0", "HC65/21/1", "HC65/21/2", "HC65/21/3", 
                "HC65/22/0", "HC65/22/1", "HC65/22/2", "HC65/22/3", 
                "HC65/23/0", "HC65/23/1", "HC65/23/2", "HC65/23/3" 
            }; 
            pi->phy_name = bm9600_phy_names[port];
            int_pc->lane_num = port % 4;
            int_pc->flags |= PHYCTRL_MDIO_ADDR_SHARE;
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_HC65_FABRIC);
        } else {
            if (SOC_IS_SIRIUS(unit)) {
                static struct {
                    char  *name;
                    int    lane;
                } _sirius_phy_port_info[] = {
                    { "HC65/4/2", 2 },  /* port 9 , sfi0,  lane 2 */
                    { "HC65/4/3", 3 },  /* port 10, sfi1,  lane 3 */
                    { "HC65/5/0", 0 },  /* port 11, sfi2,  lane 0 */
                    { "HC65/5/1", 1 },  /* port 12, sfi3,  lane 1 */
                    { "HC65/5/2", 2 },  /* port 13, sfi4,  lane 2 */
                    { "HC65/5/3", 3 },  /* port 14, sfi5,  lane 3 */
                    { "HC65/6/0", 0 },  /* port 15, sfi6,  lane 0 */
                    { "HC65/6/1", 1 },  /* port 16, sfi7,  lane 1 */
                    { "HC65/6/2", 2 },  /* port 17, sfi8,  lane 2 */
                    { "HC65/6/3", 3 },  /* port 18, sfi9,  lane 3 */
                    { "HC65/7/0", 0 },  /* port 19, sfi10, lane 0 */
                    { "HC65/7/1", 1 },  /* port 20, sfi11, lane 1 */
                    { "HC65/7/2", 2 },  /* port 21, sfi12, lane 2 */
                    { "HC65/7/3", 3 },  /* port 22, sfi13, lane 3 */
                    { "HC65/8/0", 0 },  /* port 23, sfi14, lane 0 */
                    { "HC65/8/1", 1 },  /* port 24, sfi15, lane 1 */
                    { "HC65/8/2", 2 },  /* port 25, sfi16, lane 2 */
                    { "HC65/8/3", 3 },  /* port 26, sfi17, lane 3 */
                    { "HC65/9/0", 0 },  /* port 27, sfi18, lane 0 */
                    { "HC65/9/1", 1 },  /* port 28, sfi19, lane 1 */
                    { "HC65/9/2", 2 },  /* port 29, sfi20, lane 2 */
                    { "HC65/9/3", 3 },  /* port 30, sfi21, lane 3 */
                    { "HC65/4/0", 0 },  /* port 31, sci0,  lane 0 */
                    { "HC65/4/1", 1 }   /* port 32, sci1,  lane 1 */
                };
                /*
                 * Make sure port is SFI or SCI. 
                 *  0..7  : Higig 
                 *  8     : CPU
                 *  9..30 : SFI
                 *  31-32 : SCI
                 */
                if (port >= 9 && port <= 32) {
                    int port_idx = port - 9;
                    pi->phy_name = _sirius_phy_port_info[port_idx].name;
                    int_pc->lane_num = _sirius_phy_port_info[port_idx].lane;
                    int_pc->flags |= PHYCTRL_MDIO_ADDR_SHARE;
                    PHY_FLAGS_SET(unit, port, PHY_FLAGS_HC65_FABRIC);
                }
            }
        }
    }
#endif /* BCM_BM9600_SUPPORT || BCM_SIRIUS_SUPPORT */
#endif /* INCLUDE_XGXS_HL65 */

#if defined(INCLUDE_XGXS_QSGMII65)
    if (IS_GE_PORT(unit, port) && (&phy_qsgmii65_ge == int_phyd)) {
        pi->phy_name = "QSGMII65";
#if defined(BCM_HAWKEYE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit)) {
            switch(port) {
            case 9:
            case 17:
                pi->phy_name = "QSGMII65/0";
                int_pc->lane_num = 0;
                break;

            case 10:
            case 18:
                pi->phy_name = "QSGMII65/1";
                int_pc->lane_num = 1;
                break;

            case 11:
            case 19:
                pi->phy_name = "QSGMII65/2";
                int_pc->lane_num = 2;
                break;

            case 12:
            case 20:
                pi->phy_name = "QSGMII65/3";
                int_pc->lane_num = 3;
                break;

            case 13:
            case 21:
                pi->phy_name = "QSGMII65/4";
                int_pc->lane_num = 4;
                break;

            case 14:
            case 22:
                pi->phy_name = "QSGMII65/5";
                int_pc->lane_num = 5;
                break;

            case 15:
            case 23:
                pi->phy_name = "QSGMII65/6";
                int_pc->lane_num = 6;
                break;

            case 16:
            case 24:
                pi->phy_name = "QSGMII65/7";
                int_pc->lane_num = 7;
                break;

            default:
                break;

            }
        }
#endif /* SOC_IS_HAWKEYE */
#if defined(BCM_HURRICANE_SUPPORT)
        if (SOC_IS_HURRICANE(unit)) {
            switch(port) {
            case 2:
            case 10:
            case 18:
                pi->phy_name = "QSGMII65/0";
                int_pc->lane_num = 0;
                break;

            case 3:
            case 11:
            case 19:
                pi->phy_name = "QSGMII65/1";
                int_pc->lane_num = 1;
                break;

            case 4:
            case 12:
            case 20:
                pi->phy_name = "QSGMII65/2";
                int_pc->lane_num = 2;
                break;

            case 5:
            case 13:
            case 21:
                pi->phy_name = "QSGMII65/3";
                int_pc->lane_num = 3;
                break;

            case 6:
            case 14:
            case 22:
                pi->phy_name = "QSGMII65/4";
                int_pc->lane_num = 4;
                break;

            case 7:
            case 15:
            case 23:
                pi->phy_name = "QSGMII65/5";
                int_pc->lane_num = 5;
                break;

            case 8:
            case 16:
            case 24:
                pi->phy_name = "QSGMII65/6";
                int_pc->lane_num = 6;
                break;

            case 9:
            case 17:
            case 25:
                pi->phy_name = "QSGMII65/7";
                int_pc->lane_num = 7;
                break;

            default:
                break;

            }
        }
#endif /* BCM_HURRICANE_SUPPORT */
    }
#endif /* INCLUDE_XGXS_QSGMII65 */

#if defined(INCLUDE_PHY_56XXX) && defined(INCLUDE_PHY_XGXS6)
    /* If we detetcted a Unicore driver for a GE port, attach internal SerDes
     * driver.
     * Current Unicore driver does not support external GE PHY.
     */
    if (IS_GE_PORT(unit, port) &&  (&phy_xgxs6_hg == int_phyd)) {
        if (_chk_fiber56xxx(unit, port, &_fiber56xxx_phy_entry,
                       phy_id0, phy_id1, pi)) {
            int_phyd = &phy_56xxxdrv_ge;
        }
    }
#endif /* INCLUDE_PHY_56XXX && INCLUDE_PHY_XGXS6 */


#if defined(INCLUDE_PHY_56XXX) 
    /* If we detected a shadow register driver, allocate driver data */
    if (&phy_56xxx_5601x_drv_ge == int_phyd) {
        serdes_5601x_sregs_t *sr;

        /* Allocate shadow registers */
        sr = sal_alloc(sizeof(serdes_5601x_sregs_t), 
                       "SERDES_COMBO shadow regs");
        if (sr == NULL) {
            return SOC_E_MEMORY;
        }
        int_pc->driver_data = sr;
    }
#endif /* INCLUDE_PHY_56XXX */

#if defined(INCLUDE_SERDES_COMBO)
    /* If we detected a shadow register driver, allocate driver data */
    if (&phy_serdescombo_5601x_ge == int_phyd) {
        serdescombo_5601x_sregs_t *sr;

        /* Allocate shadow registers */
        sr = sal_alloc(sizeof(serdescombo_5601x_sregs_t), 
                       "SERDES_COMBO shadow regs");
        if (sr == NULL) {
            return SOC_E_MEMORY;
        }
        int_pc->driver_data = sr;
    }
#endif /* INCLUDE_SERDES_COMBO */

#if defined(INCLUDE_PHY_53XXX)
    /* If we detetcted a Unicore driver for a GE port, attach internal SerDes
     * driver.
     * Current Unicore driver does not support external GE PHY.
     */
    if (IS_GE_PORT(unit, port)) {
        if (_chk_fiber53xxx(unit, port, &_fiber53xxx_phy_entry,
                       phy_id0, phy_id1, pi)) {
            int_phyd = &phy_53xxxdrv_ge;
        }
    }
#endif /* INCLUDE_PHY_53XXX */

#ifdef BCM_XGS_SUPPORT
    if (int_phyd == NULL) {
        if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
            /* If no appropriate driver is installed in the phy driver table
             * use a default higig driver for XE port */
#if defined (INCLUDE_PHY_XGXS6)
            if (soc_feature(unit, soc_feature_xgxs_v6)) {
                int_phyd = &phy_xgxs6_hg;
            } else
#endif /* INCLUDE_PHY_XGXS6*/
#if defined(INCLUDE_PHY_XGXS5)
            if (soc_feature(unit, soc_feature_xgxs_v5)) {
                int_phyd = &phy_xgxs5_hg;
            } else
#endif /* INCLUDE_PHY_XGXS5 */
            {
                int_phyd = &phy_xgxs1_hg;
            }
        }
    }
#endif /* BCM_XGS_SUPPORT */

#ifdef BCM_XGS3_SWITCH_SUPPORT
    /* If GMII port, attach NULL PHY driver to
     * internal PHY driver. GMII port does not have SerDes.
     */
    if (_chk_gmii(unit, port, &_null_phy_entry, 0xffff, 0xffff, pi)) {
        int_phyd = _null_phy_entry.driver;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    int_pc->pd = int_phyd;

    return SOC_E_NONE;
}

int
_ext_phy_probe(int unit, soc_port_t port,
               soc_phy_info_t *pi, phy_ctrl_t *ext_pc)
{
    uint16               phy_addr;
    uint32               id0_addr, id1_addr;
    uint16               phy_id0=0, phy_id1=0;
    int                  i;
    phy_driver_t        *phyd;
    int                  rv;
    int                  cl45_override = 0;

    phy_addr = ext_pc->phy_id;
    phyd     = NULL;


    /* Clause 45 instead of Clause 22 MDIO access */
    if (soc_property_port_get(unit, port, spn_PORT_PHY_CLAUSE, 22) == 45) {
        cl45_override = 1;
    }


    if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port) || cl45_override) {
        id0_addr = SOC_PHY_CLAUSE45_ADDR(PHY_C45_DEV_PMA_PMD,
                                         MII_PHY_ID0_REG);
        id1_addr = SOC_PHY_CLAUSE45_ADDR(PHY_C45_DEV_PMA_PMD,
                                         MII_PHY_ID1_REG);
    } else {
#ifdef BCM_SIRIUS_SUPPORT
        if (SOC_IS_SIRIUS(unit)) {
            /* Do not probe GE ports in Sirius */
            ext_pc->pd  = NULL;
            return SOC_E_NONE;
        }
#endif /* BCM_SIRIUS_SUPPORT */
        id0_addr = MII_PHY_ID0_REG;
        id1_addr = MII_PHY_ID1_REG;
    }
    phy_id0 = 0xFFFF;
    phy_id1 = 0xFFFF;
    (void)ext_pc->read(unit, phy_addr, id0_addr, &phy_id0);
    (void)ext_pc->read(unit, phy_addr, id1_addr, &phy_id1);
    phy_id0 = soc_property_port_get(unit, port, spn_PORT_PHY_ID0, phy_id0);
    phy_id1 = soc_property_port_get(unit, port, spn_PORT_PHY_ID1, phy_id1);



    /* Look through table for match */

    for (i = _phys_in_table - 1; i >= 0; i--) {
        if ((phy_table[i]->checkphy)(unit, port, phy_table[i],
                                     phy_id0, phy_id1, pi)) {

            /* Device ID matches. Calls driver probe routine to confirm
             * that the driver is the appropriate one.
             * Many PHY devices has the same device ID but they are
             * actually different.
             */
            rv = PHY_PROBE(phy_table[i]->driver, unit, ext_pc);
            if ((rv == SOC_E_NONE) || (rv == SOC_E_UNAVAIL)) {
                SOC_DEBUG_PRINT((DK_PHY, "<%d> Index = %d Mynum = %d %s\n",
                    rv, i, phy_table[i]->myNum, phy_table[i]->phy_name));
            phyd = phy_table[i]->driver;
            pi->phy_id0       = phy_id0;
            pi->phy_id1       = phy_id1;
            pi->phy_addr      = phy_addr;
            if (ext_pc->dev_name) {
                pi->phy_name      = ext_pc->dev_name;
            }
            ext_pc->phy_id0   = phy_id0;
            ext_pc->phy_id1   = phy_id1;
            ext_pc->phy_oui   = PHY_OUI(phy_id0, phy_id1);
            ext_pc->phy_model = PHY_MODEL(phy_id0, phy_id1);
            ext_pc->phy_rev   = PHY_REV(phy_id0, phy_id1);

            PHY_FLAGS_SET(unit, port, PHY_FLAGS_EXTERNAL_PHY);

            break;
            }
        }
    }

#if defined(INCLUDE_PHY_5464_ESW) && defined(INCLUDE_PHY_5464_ROBO)
    
    if (SOC_IS_ROBO(unit) && (phyd == &phy_5464drv_ge)) {
        phyd = &phy_5464robodrv_ge;
    }
#endif /* INCLUDE_PHY_5464_ESW && INCLUDE_PHY_5464_ROBO */

#if defined(INCLUDE_PHY_5482_ESW) && defined(INCLUDE_PHY_5482_ROBO)
    
    if (SOC_IS_ROBO(unit) && (phyd == &phy_5482drv_ge)) {
        phyd = &phy_5482robodrv_ge;
    }
#endif /* INCLUDE_PHY_5464_ESW && INCLUDE_PHY_5464_ROBO */

#if defined(INCLUDE_PHY_54684_ESW)
    if (IS_GE_PORT(unit, port) && (phyd == &phy_54684drv_ge)) {
#if defined(BCM_HAWKEYE_SUPPORT)
        if (SOC_IS_HAWKEYE(unit)) {
            switch(port) {
            case 9:
            case 17:
                ext_pc->lane_num = 0;
                break;

            case 10:
            case 18:
                ext_pc->lane_num = 1;
                break;

            case 11:
            case 19:
                ext_pc->lane_num = 2;
                break;

            case 12:
            case 20:
                ext_pc->lane_num = 3;
                break;

            case 13:
            case 21:
                ext_pc->lane_num = 4;
                break;

            case 14:
            case 22:
                ext_pc->lane_num = 5;
                break;

            case 15:
            case 23:
                ext_pc->lane_num = 6;
                break;

            case 16:
            case 24:
                ext_pc->lane_num = 7;
                break;

            default:
                break;

            }
        }
#endif /* SOC_IS_HAWKEYE */
    }
#endif /* INCLUDE_PHY_54684_ESW */

    ext_pc->pd = phyd;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_probe
 * Purpose:
 *      Probe the PHY on the specified port and return a pointer to the
 *      drivers for the device found.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      phyd_ptr - (OUT) Pointer to PHY driver (NULL on error)
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      Loop thru table making callback for each known PHY.
 *      We loop from the table from top to bottom so that user additions
 *      take precedence over default values.  The first checkphy function
 *      returning TRUE is used as the driver.
 */
int
soc_phy_probe(int unit, soc_port_t port, phy_ctrl_t *ext_pc,
              phy_ctrl_t *int_pc)
{
    soc_phy_info_t      *pi;
    uint16               phy_addr;
    uint16               phy_addr_int;

    /* Always use default addresses for probing.
     * This make sure that the external PHY probe works correctly even
     * when the device is hot plugged or the external PHY address is
     * overriden from previous probe.
     */
    SOC_IF_ERROR_RETURN
        (soc_phy_cfg_addr_get(unit,port,0,&phy_addr));
    SOC_IF_ERROR_RETURN
        (soc_phy_cfg_addr_get(unit,port,SOC_PHY_INTERNAL,&phy_addr_int));
    int_pc->phy_id = phy_addr_int;
    ext_pc->phy_id = phy_addr;

    /*
     * Characterize PHY by reading MII registers.
     *
     * SERDES NOTE: soc_miim_read does not fail even though there's no
     * MII.  A PHY ID of "approximately" 0xffff is returned.
     * Unfortunately, it is sometimes a value like 0x3fff or 0x1fff.  We
     * are cheating by using an "assumed SERDES" if the PHY ID does not
     * match any other PHY.
     */
    pi       = &SOC_PHY_INFO(unit, port);

    /* Probe for null PHY configuration first to avoid MII timeouts */
    if (_chk_null(unit, port, &_null_phy_entry, 0xffff, 0xffff, pi)) {
        ext_pc->pd     = _null_phy_entry.driver;
        int_pc->pd     = _null_phy_entry.driver;
    }

    /* Search for internal phy */
    if (NULL == int_pc->pd) {
        SOC_IF_ERROR_RETURN
            (_int_phy_probe(unit, port, pi, int_pc));
    }

    /* Search for external PHY */
    if (NULL == ext_pc->pd) {
        SOC_IF_ERROR_RETURN
            (_ext_phy_probe(unit, port, pi, ext_pc));
    }
    /* Override external PHY driver according to config settings */
    SOC_IF_ERROR_RETURN
        (_forced_phy_probe(unit, port, pi, ext_pc));


    if (ext_pc->pd != NULL) {
        if (IS_GMII_PORT(unit, port)) {
            /* If GMII port has external PHY, remove the NULL PHY driver
             * attached to internal PHY in _int_phy_probe().
             */
            int_pc->pd = NULL;
        }
#if defined(BCM_ESW_SUPPORT) && defined(INCLUDE_PHY_SERDES)
        if (SOC_IS_ESW(unit)) {
        if (&phy_serdesassumed_ge == int_pc->pd) {
            /* Do not use Assumed SerDes driver if external PHY is present */
            pi->phy_flags &= ~PHY_FLAGS_10B;
            int_pc->pd = NULL;
        }
        }
#endif /* BCM_ESW_SUPPORT */
        if ((int_pc->pd == _null_phy_entry.driver) &&
            (ext_pc->pd == _null_phy_entry.driver)) {
            /* Attach NULL PHY driver as external PHY driver */
            int_pc->pd = NULL;
        }
    }

   if ((ext_pc->pd == NULL) && (int_pc->pd == NULL) &&
        _chk_default(unit, port, &_default_phy_entry, 0xffff, 0xffff, pi)) {
        ext_pc->pd = _default_phy_entry.driver;
    }

    assert((ext_pc->pd != NULL) || (int_pc->pd != NULL));

    if (ext_pc->pd == NULL ||        /* No external PHY */
        ext_pc->pd == int_pc->pd) {  /* Forced PHY */
        /* Use internal address when application trying to access
         * external PHY.
         */
        pi->phy_addr = pi->phy_addr_int;

        /* If there is no external PHY, the internal PHY must be in
         * fiber mode.
         */
        if (soc_property_port_get(unit, port,
                                      spn_SERDES_FIBER_PREF, 1)) {
            PHY_FLAGS_SET(unit, port, PHY_FLAGS_FIBER);
        } else {
            PHY_FLAGS_CLR(unit, port, PHY_FLAGS_FIBER);
        }
    }

    /*
     * The property if_tbi_port<X> can be used to force TBI mode on a
     * port.  The individual PHY drivers should key off this flag.
     */
    if (soc_property_port_get(unit, port, spn_IF_TBI, 0)) {
        PHY_FLAGS_SET(unit, port, PHY_FLAGS_10B);
    }

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        
        /* specific for BCM5324 only on detecting if GE_PHY is at 10B interface.*/
        if(SOC_IS_ROBO5324(unit) && IS_GE_PORT(unit, port)){
            uint32  addr, temp, ifset;
            addr = drv_reg_addr(unit, STRAP_STSr, 0, 0);
            SOC_IF_ERROR_RETURN(
                (DRV_SERVICES(unit)->reg_read)(unit, addr, &temp, 4));
            if (port == 25) {
                drv_reg_field_get(unit, STRAP_STSr, &temp, GIGA0_IFSELf, &ifset);
            }else if (port == 26) {
                drv_reg_field_get(unit, STRAP_STSr, &temp, GIGA1_IFSELf, &ifset);
            } else {
                return SOC_E_PARAM;
            }
        
            if (ifset == 2){
                pi->phy_flags |= PHY_FLAGS_10B;
            }
        }
    }
#endif /* BCM_ROBO_SUPPORT */

    pi->an_timeout =
        soc_property_port_get(unit, port,
                              spn_PHY_AUTONEG_TIMEOUT, 250000);

    SOC_DEBUG_PRINT((DK_PHY,
                     "soc_phy_probe: port=%d addr=0x%x "
                     "id0=0x%x id1=0x%x flg=0x%x driver=\"%s\"\n",
                     port,
                     pi->phy_addr, pi->phy_id0, pi->phy_id1,
                     pi->phy_flags, pi->phy_name));

    return SOC_E_NONE;
}


/*
 * Variable:
 *      phy_id_map
 * Purpose:
 *      Map the PHY identifier register (OUI and device ID) into
 *      enumerated PHY type for prototypical devices.
 */

typedef struct phy_id_map_s {
    soc_known_phy_t     phy_num;        /* Enumerated PHY type */
    uint16              oui;            /* Device OUI */
    uint16              model;          /* Device Model */
    uint16              rev_map;        /* Device Revision */
} phy_id_map_t;

#define PHY_REV_ALL   (0xffff)
#define PHY_REV_0     (1 << 0)
#define PHY_REV_1     (1 << 1)
#define PHY_REV_2     (1 << 2)
#define PHY_REV_3     (1 << 3)
#define PHY_REV_4     (1 << 4)
#define PHY_REV_5     (1 << 5)
#define PHY_REV_6     (1 << 6)
#define PHY_REV_7     (1 << 7)
#define PHY_REV_8     (1 << 8)
#define PHY_REV_9     (1 << 9)
#define PHY_REV_10    (1 << 10)
#define PHY_REV_11    (1 << 11)
#define PHY_REV_12    (1 << 12)

STATIC phy_id_map_t phy_id_map[] = {
    { _phy_id_BCM5218,      PHY_BCM5218_OUI,        PHY_BCM5218_MODEL,
      PHY_REV_ALL },
    { _phy_id_BCM5220,      PHY_BCM5220_OUI,        PHY_BCM5220_MODEL,
      PHY_REV_ALL }, /* & 5221 */
    { _phy_id_BCM5226,      PHY_BCM5226_OUI,        PHY_BCM5226_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5228,      PHY_BCM5228_OUI,        PHY_BCM5228_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5238,      PHY_BCM5238_OUI,        PHY_BCM5238_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5248,      PHY_BCM5248_OUI,        PHY_BCM5248_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5324,      PHY_BCM5324_OUI,        PHY_BCM5324_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5400,      PHY_BCM5400_OUI,        PHY_BCM5400_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5401,      PHY_BCM5401_OUI,        PHY_BCM5401_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5402,      PHY_BCM5402_OUI,        PHY_BCM5402_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5404,      PHY_BCM5404_OUI,        PHY_BCM5404_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5411,      PHY_BCM5411_OUI,        PHY_BCM5411_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5421,      PHY_BCM5421_OUI,        PHY_BCM5421_MODEL,
      PHY_REV_ALL}, /* & 5421S */
    { _phy_id_BCM5424,      PHY_BCM5424_OUI,        PHY_BCM5424_MODEL,
      PHY_REV_ALL}, /* & 5434 */
    { _phy_id_BCM5464,      PHY_BCM5464_OUI,        PHY_BCM5464_MODEL,
      PHY_REV_ALL}, /* & 5464S */
    { _phy_id_BCM5466,       PHY_BCM5466_OUI,       PHY_BCM5466_MODEL,
      PHY_REV_ALL}, /* & 5466S */
    { _phy_id_BCM5461,       PHY_BCM5461_OUI,       PHY_BCM5461_MODEL,
      PHY_REV_ALL}, /* & 5461S */
    { _phy_id_BCM5461,       PHY_BCM5462_OUI,       PHY_BCM5462_MODEL,
      PHY_REV_ALL}, /* & 5461D */
    { _phy_id_BCM5478,       PHY_BCM5478_OUI,       PHY_BCM5478_MODEL,
      PHY_REV_ALL}, /*   5478 */
    { _phy_id_BCM5488,       PHY_BCM5488_OUI,       PHY_BCM5488_MODEL,
      PHY_REV_ALL}, /*   5488 */
    { _phy_id_BCM5482,       PHY_BCM5482_OUI,       PHY_BCM5482_MODEL,
      PHY_REV_ALL}, /* & 5482S */
    { _phy_id_BCM5481,       PHY_BCM5481_OUI,       PHY_BCM5481_MODEL,
      PHY_REV_ALL}, /* & 5481 */
    { _phy_id_BCM54980,      PHY_BCM54980_OUI,      PHY_BCM54980_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54980C,     PHY_BCM54980C_OUI,     PHY_BCM54980C_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54980V,     PHY_BCM54980V_OUI,     PHY_BCM54980V_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54980VC,    PHY_BCM54980VC_OUI,    PHY_BCM54980VC_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54680,      PHY_BCM54680_OUI,      PHY_BCM54680_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54880,      PHY_BCM54880_OUI,      PHY_BCM54880_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54880E,     PHY_BCM54880E_OUI,     PHY_BCM54880E_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54680E,     PHY_BCM54680E_OUI,     PHY_BCM54680E_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM52681E,     PHY_BCM52681E_OUI,     PHY_BCM52681E_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54881,      PHY_BCM54881_OUI,      PHY_BCM54881_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54810,      PHY_BCM54810_OUI,      PHY_BCM54810_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54684,      PHY_BCM54684_OUI,      PHY_BCM54684_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54684E,     PHY_BCM54684E_OUI,     PHY_BCM54684E_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54682,      PHY_BCM54682E_OUI,     PHY_BCM54682E_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54685,      PHY_BCM54685_OUI,      PHY_BCM54685_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54640,      PHY_BCM54640_OUI,      PHY_BCM54640_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54616,      PHY_BCM54616_OUI,      PHY_BCM54616_MODEL,
      PHY_REV_ALL},
#ifdef INCLUDE_MACSEC
#if defined(INCLUDE_PHY_54580)
    { _phy_id_BCM54584,      PHY_BCM54580_OUI,      PHY_BCM54584_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54580,      PHY_BCM54580_OUI,      PHY_BCM54580_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54540,      PHY_BCM54580_OUI,      PHY_BCM54540_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM54585,      PHY_BCM54580_OUI,      PHY_BCM54585_MODEL,
      PHY_REV_ALL},
#endif
#endif /* INCLUDE_MACSEC */
    { _phy_id_BCM8011,       PHY_BCM8011_OUI,       PHY_BCM8011_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM8040,       PHY_BCM8040_OUI,       PHY_BCM8040_MODEL,
      PHY_REV_0 | PHY_REV_1 | PHY_REV_2},
    { _phy_id_BCM8703,       PHY_BCM8703_OUI,       PHY_BCM8703_MODEL,
      PHY_REV_0 | PHY_REV_1 | PHY_REV_2},
    { _phy_id_BCM8704,       PHY_BCM8704_OUI,       PHY_BCM8704_MODEL,
      PHY_REV_3},
    { _phy_id_BCM8705,       PHY_BCM8705_OUI,       PHY_BCM8705_MODEL,
      PHY_REV_4},
    { _phy_id_BCM8706,       PHY_BCM8706_OUI,       PHY_BCM8706_MODEL,
      PHY_REV_5},
    { _phy_id_BCM8750,       PHY_BCM8750_OUI,       PHY_BCM8750_MODEL,
      PHY_REV_0},
    { _phy_id_BCM8752,       PHY_BCM8752_OUI,       PHY_BCM8752_MODEL,
      PHY_REV_0},
    { _phy_id_BCM8754,       PHY_BCM8754_OUI,       PHY_BCM8754_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM8072,       PHY_BCM8072_OUI,       PHY_BCM8072_MODEL,
      PHY_REV_5},
    { _phy_id_BCM8727,       PHY_BCM8727_OUI,       PHY_BCM8727_MODEL,
      PHY_REV_6},
    { _phy_id_BCM8073,       PHY_BCM8073_OUI,       PHY_BCM8073_MODEL,
      PHY_REV_6},
    { _phy_id_BCM8747,       PHY_BCM8747_OUI,       PHY_BCM8747_MODEL,
      PHY_REV_7},
    { _phy_id_BCM84740,      PHY_BCM84740_OUI,      PHY_BCM84740_MODEL,
      PHY_REV_0},
#ifdef INCLUDE_MACSEC
#ifdef INCLUDE_PHY_8729
    { _phy_id_BCM8729,       PHY_BCM5927_OUI,       PHY_BCM5927_MODEL,
      PHY_REV_4},
    { _phy_id_BCM8729,       PHY_BCM8729_OUI,       PHY_BCM8729_MODEL,
      PHY_REV_12},
#endif
#endif  /* INCLUDE_MACSEC */
    { _phy_id_BCM8481x,      PHY_BCM8481X_OUI,      PHY_BCM8481X_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM84812ce,    PHY_BCM84812CE_OUI,    PHY_BCM84812CE_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM84821,      PHY_BCM84821_OUI,      PHY_BCM84821_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM84822,      PHY_BCM84822_OUI,      PHY_BCM84822_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM84823,      PHY_BCM84823_OUI,      PHY_BCM84823_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCMXGXS1,      PHY_BCMXGXS1_OUI,      PHY_BCMXGXS1_MODEL,
      PHY_REV_ALL},
    /*
     * HL65 has the same device ID as XGXS1.
     *{ _phy_id_XGXS_HL65,   PHY_XGXS_HL65_OUI,     PHY_XGXS_HL65_MODEL,
     * PHY_REV_ALL }
     */
    { _phy_id_BCMXGXS2,      PHY_BCMXGXS2_OUI,      PHY_BCMXGXS2_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCMXGXS5,      PHY_BCMXGXS5_OUI,      PHY_BCMXGXS5_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCMXGXS6,      PHY_BCMXGXS6_OUI,      PHY_BCMXGXS6_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5398,       PHY_BCM5398_OUI,       PHY_BCM5398_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5348,       PHY_BCM5348_OUI,       PHY_BCM5348_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM5395,       PHY_BCM5395_OUI,       PHY_BCM5395_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53242,      PHY_BCM53242_OUI,     PHY_BCM53242_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53262,      PHY_BCM53262_OUI,     PHY_BCM53262_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53115,      PHY_BCM53115_OUI,     PHY_BCM53115_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53118,      PHY_BCM53118_OUI,     PHY_BCM53118_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53280,      PHY_BCM53280_OUI,     PHY_BCM53280_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53101,      PHY_BCM53101_OUI,     PHY_BCM53101_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53314,      PHY_BCM53314_OUI,     PHY_BCM53314_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53324,      PHY_BCM53324_OUI,     PHY_BCM53324_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53125,      PHY_BCM53125_OUI,     PHY_BCM53125_MODEL,
      PHY_REV_ALL},
    { _phy_id_BCM53128,      PHY_BCM53128_OUI,     PHY_BCM53128_MODEL,
      PHY_REV_ALL},
    { _phy_id_SERDES100FX,   PHY_SERDES100FX_OUI,   PHY_SERDES100FX_MODEL,
      PHY_REV_4 | PHY_REV_5},
    { _phy_id_SERDES65LP,    PHY_SERDES65LP_OUI,    PHY_SERDES65LP_MODEL,
      PHY_REV_ALL},
    { _phy_id_SERDESCOMBO,   PHY_SERDESCOMBO_OUI,   PHY_SERDESCOMBO_MODEL,
      PHY_REV_8 },
    { _phy_id_XGXS_16G,      PHY_XGXS_16G_OUI,      PHY_XGXS_16G_MODEL,
      PHY_REV_0 },
};

/*
 * Function:
 *      _phy_ident_type_get
 * Purpose:
 *      Check the PHY ID and return an enumerated value indicating
 *      the PHY.  This looks very redundant, but in the future, more
 *      complicated PHY detection may be necessary.  In addition, the
 *      enum value could be used as an index.
 * Parameters:
 *      phy_id0 - PHY ID register 0 (MII register 2)
 *      phy_id1 - PHY ID register 1 (MII register 3)
 */

static soc_known_phy_t
_phy_ident_type_get(uint16 phy_id0, uint16 phy_id1)
{
    int                 i;
    phy_id_map_t        *pm;
    uint16              oui, model, rev_map;

    oui       = PHY_OUI(phy_id0, phy_id1);
    model     = PHY_MODEL(phy_id0, phy_id1);
    rev_map   = 1 << PHY_REV(phy_id0, phy_id1);

    SOC_DEBUG_PRINT((DK_PHY,
        "phy_id0 = %04x phy_id1 %04x oui = %04x model = %04x rev_map = %04x\n",
        phy_id0, phy_id1, oui, model, rev_map));
    for (i = 0; i < COUNTOF(phy_id_map); i++) {
        pm = &phy_id_map[i];
        if ((pm->oui == oui) && (pm->model == model)) {
            if (pm->rev_map & rev_map) {
                return pm->phy_num;
            }
        }
    }

    return _phy_id_unknown;
}

/*
 * Function:
 *      soc_phy_nocxn
 * Purpose:
 *      Return the no_cxn PHY driver
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      phyd_ptr - (OUT) Pointer to PHY driver.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_phy_nocxn(int unit, phy_driver_t **phyd_ptr)
{
    *phyd_ptr = &phy_nocxn;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_info_get
 * Purpose:
 *      Accessor function to copy out PHY info structure
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      pi - (OUT) Pointer to output structure.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_phy_info_get(int unit, soc_port_t port, soc_phy_info_t *pi)
{
    soc_phy_info_t *source;

    source = &SOC_PHY_INFO(unit, port);

    sal_memcpy(pi, source, sizeof(soc_phy_info_t));
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_an_timeout_get
 * Purpose:
 *      Return autonegotiation timeout for a specific port
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      Timeout in usec
 */

sal_usecs_t
soc_phy_an_timeout_get(int unit, soc_port_t port)
{
    return PHY_AN_TIMEOUT(unit, port);
}

/*
 * Function:
 *      soc_phy_addr_of_port
 * Purpose:
 *      Return PHY ID of the PHY attached to a specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      PHY ID
 */

uint8
soc_phy_addr_of_port(int unit, soc_port_t port)
{
    return PHY_ADDR(unit, port);
}

/*
 * Function:
 *      soc_phy_addr_int_of_port
 * Purpose:
 *      Return PHY ID of a intermediate PHY on specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      PHY ID
 * Notes:
 *      Only applies to chip ports that have an intermediate PHY.
 */

uint8
soc_phy_addr_int_of_port(int unit, soc_port_t port)
{
    return PHY_ADDR_INT(unit, port);
}

/*
 * Function:
 *      soc_phy_cfg_addr_get
 * Purpose:
 *      Get the configured PHY address for this port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flag - internal phy or external phy
 *      addr_ptr - hold the retrieved address
 * Returns:
 *      SOC_E_NONE 
 */

int
soc_phy_cfg_addr_get(int unit, soc_port_t port, int flags, uint16 *addr_ptr)
{
    if (flags & SOC_PHY_INTERNAL) {
        *addr_ptr = port_phy_addr[unit][port].int_addr;
    } else {
        *addr_ptr = soc_property_port_get(unit, port,
                                      spn_PORT_PHY_ADDR,
                                      port_phy_addr[unit][port].ext_addr);
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_cfg_addr_set
 * Purpose:
 *      Configure the port with the given PHY address.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      flag - internal phy or external phy
 *      addr - the address to set
 * Returns:
 *      SOC_E_NONE
 */

int
soc_phy_cfg_addr_set(int unit, soc_port_t port, int flags, uint16 addr)
{
    if (flags & SOC_PHY_INTERNAL) {
        port_phy_addr[unit][port].int_addr = addr;
    } else {
        port_phy_addr[unit][port].ext_addr = addr;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_phy_addr_to_port
 * Purpose:
 *      Return the port to which a given PHY ID corresponds.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      Port number
 */

soc_port_t
soc_phy_addr_to_port(int unit, uint8 phy_id)
{
    return PHY_ADDR_TO_PORT(unit, phy_id);
}

/*
 * Function:
 *      soc_phy_id1reg_get
 * Purpose:
 *      Return the PHY ID1 field from the PHY on a specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      PHY ID
 */

uint16
soc_phy_id1reg_get(int unit, soc_port_t port)
{
    return PHY_ID1_REG(unit, port);
}

/*
 * Function:
 *      soc_phy_id1reg_get
 * Purpose:
 *      Return the PHY ID0 field from the PHY on a specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      PHY ID
 */

uint16
soc_phy_id0reg_get(int unit, soc_port_t port)
{
    return PHY_ID0_REG(unit, port);
}

/*
 * Function:
 *      soc_phy_is_c45_miim
 * Purpose:
 *      Return TRUE  if Phy uses Clause 45 MIIM, FALSE otherwise
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      Return TRUE  if Phy uses Clause 45 MIIM, FALSE otherwise
 */

int
soc_phy_is_c45_miim(int unit, soc_port_t port)
{
    return PHY_CLAUSE45_MODE(unit, port);
}

/*
 * Function:
 *      soc_phy_name_get
 * Purpose:
 *      Return name of PHY driver corresponding to specified port.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 * Returns:
 *      Static pointer to string.
 */

char *
soc_phy_name_get(int unit, soc_port_t port)
{
    if (PHY_NAME(unit, port) != NULL) {
        return PHY_NAME(unit, port);
    } else {
        return "<Unnamed PHY>";
    }
}

#ifdef BCM_ESW_SUPPORT
#ifdef BROADCOM_DEBUG
/*
 * Function:
 *      soc_phy_dump
 * Purpose:
 *      Display the phy dirvers that are compiled into the driver.
 * Parameters:
 *      unit - StrataSwitch unit #.
 * Returns:
 *      none
 */

void soc_phy_dump(void)
{
    int idx1, idx2;

    for(idx1 = 0; idx1 < _phys_in_table; idx1 += 4) {
        if ( idx1 == 0 ) {
            soc_cm_print("PHYs: ");
        } else {
            soc_cm_print("      ");
        }
        for(idx2 = idx1; idx2 < idx1 + 4 && idx2 < _phys_in_table; idx2++) {
            soc_cm_print("\t%s%s", phy_table[idx2]->phy_name,
                         idx2 < _phys_in_table? "," : "");
        }
        soc_cm_print("\n");
    }

    return;
}
#endif /* BROADCOM_DEBUG */

int
soc_phy_list_get(char *phy_list[],int phy_list_size,int *phys_in_list)
{
    int       idx;

    if (phy_list == NULL) {
        return SOC_E_PARAM;
    }

    if (phys_in_list == NULL) {
        return SOC_E_PARAM;
    }

    if (phy_list_size < _phys_in_table) {
        *phys_in_list = phy_list_size;
    } else {
        *phys_in_list = _phys_in_table;
    }

    for (idx = 0; idx < *phys_in_list; idx++) {
        phy_list[idx] = phy_table[idx]->phy_name;
    }

    return SOC_E_NONE;
}
#endif /* BCM_ESW_SUPPORT */

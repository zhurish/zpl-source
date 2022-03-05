/*
 * $Id: drv.c,v 1.110 Broadcom SDK $
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
 * StrataSwitch driver
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/boot.h>
#include <sal/core/dpc.h>

#include <soc/cm.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/error.h>

#include <soc/counter.h>
#include <soc/mcm/robo/driver.h>
#include <soc/mcm/robo/allenum.h>
#include <soc/mcm/robo/intenum.h>
#include <soc/drv.h>
#include <soc/feature.h>
#include <soc/robo.h>
#include <soc/counter.h>
#include <soc/arl.h>
#include <soc/phyctrl.h>

/*
 * Driver global variables
 *
 *   soc_control: per-unit driver control structure
 *   soc_ndev: the number of units created
 *   soc_ndev_attached: the number of units attached
 */

int soc_eth_unit = -1;
int soc_mii_unit = -1;

sal_mutex_t spiMutex = NULL;   /* SPI mutual exclusion */

#ifdef MDC_MDIO_SUPPORT
sal_mutex_t mdc_data_lock = NULL;
int access_page[SOC_MAX_NUM_SWITCH_DEVICES];
#endif /* MDC_MDIO_SUPPORT */

#if defined(BCM_53125)
#define BCM53125_STRAP_PIN_8051_MASK    0x10000
#endif

/*
 * Function:
 *  soc_robo_power_down_config
 * Purpose:
 *      Disable/Enable PORTX power down mode 
 * Parameters:
 *  valid_pbmp - pbmp to disable power down mode (PHY is enable)
 */
 int
soc_robo_power_down_config(int unit, soc_pbmp_t valid_pbmp) {
    int rv;
    soc_pbmp_t pbmp;
    uint32 reg_val;
    
    /* Get power down support pbmp from chip specific dev_prop 
     * Do nothing if the return value is unavail.
     */
    rv = DRV_DEV_PROP_GET(unit, 
        DRV_DEV_PROP_POWER_DOWN_SUPPORT_PBMP, (uint32 *)&pbmp);    

    if (SOC_SUCCESS(rv)) {
        rv = REG_READ_PWR_DOWN_MODEr(unit, &reg_val);
        if (rv == SOC_E_UNAVAIL) {
            return SOC_E_NONE;
        }
        SOC_PBMP_NEGATE(valid_pbmp, valid_pbmp);
        SOC_PBMP_AND(pbmp, valid_pbmp);
        soc_PWR_DOWN_MODEr_field_set(unit, &reg_val, PORTX_PWR_DOWNf, (uint32 *)&pbmp);
        rv = REG_WRITE_PWR_DOWN_MODEr(unit, &reg_val);
        if (rv == SOC_E_UNAVAIL) {
            return SOC_E_NONE;
        }    
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *  robo_soc_chip_driver_find
 * Purpose:
 *      Return the soc_driver for a chip if found.
 *      If not found, return NULL.
 * Parameters:
 *  pci_dev_id - PCI dev ID to find (exact match)
 *  pci_rev_id - PCI dev ID to find (exact match)
 * Returns:
 *  Pointer to static driver structure
 */
static soc_driver_t *
soc_robo_chip_driver_find(uint16 pci_dev_id, uint8 pci_rev_id)
{
    int         i;
    soc_driver_t    *d;
    uint16      driver_dev_id;
    uint8       driver_rev_id;

    if (soc_cm_get_id_driver(pci_dev_id, pci_rev_id,
                 &driver_dev_id, &driver_rev_id) < 0) {
    return NULL;
    }

    /*
     * Find driver in table.  In theory any IDs returned by
     * soc_cm_id_to_driver_id() should have a driver in the table.
     */
    for (i = 0; i < SOC_ROBO_NUM_SUPPORTED_CHIPS; i++) {
    d = soc_robo_base_driver_table[i];
    if ((d != NULL) &&
        (d->block_info != NULL) &&
        (d->pci_device == driver_dev_id) &&
        (d->pci_revision == driver_rev_id)) {
        return d;
    }
    }

    SOC_ERROR_PRINT((DK_ERR,
             "soc_chip_driver_find: driver in devid table "
             "not in soc_robo_base_driver_table\n"));

    return NULL;
}

/*
 * Function:
 *  soc_info_config
 * Parameters:
 *  unit - RoboSwitch unit number.
 *  soc  - soc_control_t associated with this unit
 * Purpose:
 *  Fill in soc_info structure for a newly attached unit.
 *  Generates bitmaps and various arrays based on block and
 *  ports that the hardware has enabled.
 */

static void
soc_robo_info_config(int unit, soc_control_t *soc)
{
    soc_info_t      *si;
    soc_pbmp_t      pbmp_valid;
    uint16      dev_id;
    uint8       rev_id;
    uint16      drv_dev_id;
    uint8       drv_rev_id;
    int         port, blk = 0, bindex = 0, pno = 0, mem;
    char        *bname;
    int         blktype;
    int                 disabled_port, i;
    uint32      value[SOC_PBMP_WORD_MAX];
#ifdef BCM_TB_SUPPORT
    char        *s = NULL;
#endif
    
    si = &soc->info;
    sal_memset((void *)si, 0, sizeof(soc_info_t));

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &drv_dev_id, &drv_rev_id);

    if (CMDEV(unit).dev.info->dev_type & SOC_SPI_DEV_TYPE) {
        si->spi_device = TRUE;
    }

    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];
    si->num_cpu_cosq = 1;
    si->port_addr_max = 31;
    si->modid_count = 1;
    /* there is no Stacking solution for ROBO device so far */
    si->modid_max = 1;    /* See SOC_MODID_MAX(unit) */

    SOC_PBMP_CLEAR(si->s_pbm);  /* 10/100/1000/2500 Mbps comboserdes */
    SOC_PBMP_CLEAR(si->gmii_pbm);

    /*
     * pbmp_valid is a bitmap of all ports that exist on the unit.
     */
    pbmp_valid = soc_property_get_pbmp(unit, spn_PBMP_VALID, 1);

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (drv_dev_id) {
    case BCM5324_PHYID_LOW:
        si->chip = SOC_INFO_ROBO5324;
        if ((drv_rev_id == BCM5324_A1_REV_ID) ||
            (drv_rev_id == BCM5324_A2_REV_ID) ) {
            si->chip |= SOC_INFO_ROBO5324_A1;
        }
        break;  
    case BCM5396_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5396;
        break;
    case BCM5389_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5389;
        break;
    case BCM5398_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5398;
        break;
    case BCM5348_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5348;
        si->port_addr_max = 63;
        break;
    case BCM5397_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5397;
        break;
    case BCM5347_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5347;
        break;
    case BCM5395_DEVICE_ID:
        si->chip = SOC_INFO_ROBO5395;
        si->num_cpu_cosq = 4;
        break;
    case BCM53242_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53242;
        si->num_cpu_cosq = 4;
        break;
    case BCM53262_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53262;
        si->num_cpu_cosq = 4;
        break;
    case BCM53115_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53115;
        si->num_cpu_cosq = 4;
        break;
    case BCM53118_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53118;
        si->num_cpu_cosq = 4;
        break;
#ifdef BCM_TB_SUPPORT
    case BCM53280_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53280;
        si->num_cpu_cosq = 8;
        if (drv_rev_id == BCM53280_B0_REV_ID) {
            si->chip |= SOC_INFO_ROBO53280_B0;
        }
        switch (dev_id) {
            case BCM53284_DEVICE_ID:
                /* 24FE + 2GE */
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 27);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 28);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 25);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 26);
                break;
            case BCM53286_DEVICE_ID:        
                s = soc_property_get_str(unit, "board_name");
                if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)){
                    /* for FPGA  fe0-fe1, imp, ge0-ge1 */
                    SOC_PBMP_CLEAR(pbmp_valid);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 0);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 1);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 2);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 3);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 24);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 25);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 26);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 27);
                    SOC_PBMP_PORT_ADD(pbmp_valid, 28);                    
                }
                /* 24FE + 4GE (4 built-in SerDes) */
                break;
            case BCM53288_DEVICE_ID:
                /* 24FE + 2GE(2 built-in SerDes) + 1 2.5G(GPONE) */
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 27);
                SOC_PBMP_PORT_ADD(si->s_pbm, 28);
                break;
            case BCM53283_DEVICE_ID:
                /* 16FE + 2GE */
                for (port = 16; port < 24; port ++) {
                    SOC_PBMP_PORT_REMOVE(pbmp_valid, port);
                }
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 27);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 28);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 25);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 26);
                break;
            case BCM53282_DEVICE_ID:
                 /* 8FE + 2GE */
                for (port = 8; port < 24; port ++) {
                    SOC_PBMP_PORT_REMOVE(pbmp_valid, port);
                }                     
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 27);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 28);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 25);
                SOC_PBMP_PORT_ADD(si->gmii_pbm, 26);

                break;
        }
        break;
#endif  /* BCM_TB_SUPPORT */
	case BCM53101_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53101;
        si->num_cpu_cosq = 4;
        break;
       case BCM53125_DEVICE_ID:
        si->chip = SOC_INFO_ROBO53125;
        si->num_cpu_cosq = 4;
        break;
    case BCM53128_DEVICE_ID:    
        si->chip = SOC_INFO_ROBO53128;
        si->num_cpu_cosq = 4;
        /* Check the bonding */
        if (spiMutex) {
            value[0] = 0;
            if (REG_READ_BONDING_PAD_STATUSr(unit, &value[0]) != SOC_E_NONE) {
                SOC_ERROR_PRINT((DK_ERR,
                    "soc_info_config: can not get the bonding of Blackbird2\n"));
            }
            if (value[0] & 0x1) {
                /* BCM53128V */
                si->chip |= SOC_INFO_ROBO53128V;
            }
        }
        
        break;
    default:
        si->chip = 0;
        SOC_ERROR_PRINT((DK_WARN,
                 "soc_info_config: driver device %04x unexpected\n",
                 drv_dev_id));
        break;
    }



    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        /* Register access is only available when spiMutex be created. */
        if (spiMutex) {
            uint32 data;
            int rv;

            rv = REG_READ_BONDING_PADr(unit, &data);
            if (SOC_FAILURE(rv)) {
                SOC_ERROR_PRINT((DK_ERR,
                     "soc_info_config: can not get the bonding of Harrier\n"));
                return;
            }
            data &= 0x1f;
            data >>= 1;
            if (data == 0x5) {
                /* BCM53212, 16FE+2GE */
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 16);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 17);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 18);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 19);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 20);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 21);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 22);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 23);
            } else if (data == 0x3) {
                /* BCM53202, 8FE+2GE */
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 8);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 9);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 10);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 11);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 12);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 13);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 14);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 15);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 16);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 17);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 18);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 19);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 20);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 21);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 22);
                SOC_PBMP_PORT_REMOVE(pbmp_valid, 23);
            }
        }
    }

    /*
     * For BCM5324 family.
     * BCM5321: 16 ports
     * BCM5320: 8 ports
     */
    if (SOC_IS_ROBO5324(unit)) {
        if ( soc_property_get(unit, spn_BCM5321, 0)) {
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 16);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 17);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 18);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 19);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 20);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 21);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 22);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 23);
        }
        if ( soc_property_get(unit, spn_BCM5320, 0)) {
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 8);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 9);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 10);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 11);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 12);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 13);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 14);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 15);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 16);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 17);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 18);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 19);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 20);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 21);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 22);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 23);
        }
    }

    if (SOC_IS_ROBO5389(unit)) {
        if ((dev_id == BCM5389_A1_DEVICE_ID) && 
            (rev_id == BCM5389_A1_REV_ID)) {
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 5);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 6);
            SOC_PBMP_PORT_REMOVE(pbmp_valid, 7);
        }
    }
    si->ipic_port = -1;
    si->ipic_block = -1;
    si->exp_port = -1;
    si->exp_block = -1;
    si->cmic_port = -1;
    si->cmic_block = -1;
    si->spi_port = -1;
    si->spi_block = -1;
    si->fe.min = si->fe.max = -1;
    si->ge.min = si->ge.max = -1;
    si->xe.min = si->xe.max = -1;
    si->hg.min = si->hg.max = -1;
    si->ether.min = si->ether.max = -1;
    si->port.min = si->port.max = -1;
    si->all.min = si->all.max = -1;

    for (blk = 0; blk < SOC_ROBO_MAX_NUM_BLKS; blk++) {
        si->block_port[blk] = REG_PORT_ANY;
    }

    for (port = 0; ; port++) {
        disabled_port = FALSE;
        blk = SOC_PORT_INFO(unit, port).blk;
        bindex = SOC_PORT_INFO(unit, port).bindex;
        if (blk < 0 && bindex < 0) {            /* end of list */
            break;
        }
        if (blk < 0) {                  /* empty slot */
            disabled_port = TRUE;
            blktype = 0;
        } else {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
            if (!SOC_PBMP_MEMBER(pbmp_valid, port)) {   /* disabled port */
                if (blktype & SOC_BLK_CPU) {
                    SOC_ERROR_PRINT((DK_WARN,
                             "soc_info_config: "
                             "cannot disable cpu port\n"));
                } else {
                    disabled_port = TRUE;
                }
            }
        }

        if (disabled_port) {
            sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                 "?%d", port);
            si->port_offset[port] = port;
            continue;
        }

#define ADD_PORT(ptype, port) \
            si->ptype.port[si->ptype.num++] = port; \
            if (si->ptype.min < 0) { \
            si->ptype.min = port; \
            } \
            if (port > si->ptype.max) { \
            si->ptype.max = port; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.bitmap, port);

        bname = soc_block_port_name_lookup_ext(blktype, unit);
        switch (blktype) {
        case SOC_BLK_EPIC:
            pno = si->fe.num;
            ADD_PORT(fe, port);
            ADD_PORT(ether, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_BLK_GPIC:
                pno = si->ge.num;           
                ADD_PORT(ge, port);         
                ADD_PORT(ether, port);          
                ADD_PORT(port, port);           
                ADD_PORT(all, port);        
            break;
        case SOC_BLK_XPIC:
            pno = si->xe.num;
            ADD_PORT(xe, port);
            ADD_PORT(ether, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_BLK_IPIC:
            si->ipic_port = port;
            si->ipic_block = blk;
            /* FALLTHROUGH */
        case SOC_BLK_HPIC:
            pno = si->hg.num;
            ADD_PORT(hg, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            break;
        case SOC_BLK_CPIC:
        case SOC_BLK_CMIC:
            pno = 0;
            si->cmic_port = port;
            si->cmic_block = blk;
            SOC_PBMP_PORT_ADD(si->cmic_bitmap, port);
            ADD_PORT(all, port);
            break;
        case SOC_BLK_SPI:
            pno = 0;
            si->spi_port = port;
            si->spi_block = blk;
            SOC_PBMP_PORT_ADD(si->spi_bitmap, port);
            ADD_PORT(all, port);
            break;
        case SOC_BLK_EXP:
            pno = 0;
            si->exp_port = port;
            si->exp_block = blk;
            ADD_PORT(all, port);
            break;
        default:
            pno = 0;
            break;
        }
#undef  ADD_PORT
        if (bname[0] == '?') {
            pno = port;
        }
        sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                 "%s%d", bname, pno);
        si->port_type[port] = blktype;
        si->port_offset[port] = pno;
        si->block_valid[blk] += 1;
        if (si->block_port[blk] < 0) {
            si->block_port[blk] = port;
        }
        SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
    }
    si->port_num = port;

    /* some things need to be found in the block table */
    si->arl_block = -1;
    si->mmu_block = -1;
    si->mcu_block = -1;
    si->inter_block = -1;
    si->exter_block = -1;
    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
        blktype = SOC_BLOCK_INFO(unit, blk).type;
        si->has_block |= blktype;
        switch (blktype) {
        case SOC_BLK_ARL:/* No use in robo */
            si->arl_block = blk;
            si->block_valid[blk] += 1;
            break;
        case SOC_BLK_MMU:/* No use in robo */
            si->mmu_block = blk;
            si->block_valid[blk] += 1;
            break;
        case SOC_BLK_MCU:/* No use in robo */
            si->mcu_block = blk;
            si->block_valid[blk] += 1;
            break;
        case SOC_BLK_INTER:
            si->inter_block = blk;
            si->block_valid[blk] += 1;
            if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
                DRV_SERVICES(unit)->dev_prop_get
                    (unit, DRV_DEV_PROP_INTERNAL_MII_PBMP, &value[0]);
                for (i=0; i < SOC_PBMP_WORD_MAX; i++){
                    SOC_PBMP_WORD_SET(si->block_bitmap[blk], i, value[i]);
                }
            }

            break;
        case SOC_BLK_EXTER:
            si->exter_block = blk;
            si->block_valid[blk] += 1;
           if (SOC_IS_ROBO5348(unit)) {
                DRV_SERVICES(unit)->dev_prop_get
                    (unit, DRV_DEV_PROP_EXTERNAL_MII_PBMP, &value[0]);
                for (i=0; i < SOC_PBMP_WORD_MAX; i++){
                    SOC_PBMP_WORD_SET(si->block_bitmap[blk], i, value[i]);
                }
            }
            break;
            
            
        }
        sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                 "%s%d",
                 soc_block_name_lookup_ext(blktype, unit),
                 SOC_BLOCK_INFO(unit, blk).number);
    }
    si->block_num = blk;

    /*
     * Calculate the mem_block_any array for this configuration
     * The "any" block is just the first one enabled
     */
    for (mem = 0; mem < NUM_SOC_ROBO_MEM; mem++) {
        si->mem_block_any[mem] = -1;
        if(&SOC_MEM_INFO(unit, mem)==NULL){
            continue;
        }
        if (SOC_MEM_IS_VALID(unit, mem)) {
            SOC_MEM_BLOCK_ITER(unit, mem, blk) {
            si->mem_block_any[mem] = blk;
            break;
            }
        }
    }
}

static int
soc_chip_reset(int unit){

    int rv = SOC_E_NONE;
    int i;
    uint32      temp;
    uint32 data;
    uint32        bypass_reset;
#if defined(BCM_53125) || defined(BCM_53128)
    uint32  model = 0;
#endif /* BCM_53125 || BCM_53128 */

    bypass_reset = 
            soc_property_get(unit, spn_SOC_SKIP_RESET, 0);
    if (bypass_reset) {
        return rv;
    }
    if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit) ||
        SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) ||
        SOC_IS_TB(unit)) {

      do {
        SOC_IF_ERROR_RETURN(REG_READ_CHIP_RST_CTLr(unit, &data));

        temp = 1; /* Enable reset register */
        soc_CHIP_RST_CTLr_field_set(unit, &data, RST_CHIPf, &temp); 
        SOC_IF_ERROR_RETURN(REG_WRITE_CHIP_RST_CTLr(unit, &data));

        sal_usleep(500000); /* 500 ms */

        /* Wait for chip reset complete */
        for (i=0; i<100000; i++) {
            data = 0;
            if ((rv = REG_READ_CHIP_RST_CTLr(unit, &data)) < 0) {
                /*
                 * Can't check reset status from register.
                 * Anyway, sleep for 3 seconds
                 */
                sal_sleep(3);
                break;
            } 
            temp = 0;
            soc_CHIP_RST_CTLr_field_get(unit, &data, RST_CHIPf, &temp); 
            if (!temp) {
                /* Reset is complete */
                break;
            }
        }

        if (SOC_IS_ROBO5348(unit) || SOC_IS_ROBO5347(unit)) {
            /*
             * BCM5347/BCM5348 need to
             * check for any error found in the BIST
             */
            uint64 bist_data;

            if((rv = REG_READ_BIST_STS0r(unit, (uint32 *)&bist_data)) < 0){
                /* Can't read BIST_STS0 register, rerun reset procedure */
                continue;
            }
            if (COMPILER_64_IS_ZERO(bist_data)) {
                /* The first BIST_STS0r is ok, check the second BIST_STS1 */
                if((rv = REG_READ_BIST_STS1r(unit, (uint32 *)&bist_data)) < 0){
                    /* Can't read BIST_STS1 register, rerun reset procedure */
                    continue;
                }
                if (COMPILER_64_IS_ZERO(bist_data)) {
                    /* Both BIST_STS0 and BIST_STS1 are ok. Quit now */
                    break;
                }
            }
            /* Found error in any of two BIST_STS0 and BIST_STS1. Retry again */
        } else {
            /* BCM53242 and BCM53262 needn't do the workaround */
            break;
        }
      } while (1);
    } else if (SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
        SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) ||
        SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||SOC_IS_ROBO53128(unit)) {
        SOC_IF_ERROR_RETURN(REG_READ_WATCH_DOG_CTRLr(unit, &data));
        temp = 1;
        if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) || 
            SOC_IS_ROBO53118(unit) ||SOC_IS_ROBO53101(unit) ||
            SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
            soc_WATCH_DOG_CTRLr_field_set(unit, &data, EN_SW_RESETf, &temp); 
        } else {
            soc_WATCH_DOG_CTRLr_field_set(unit, &data, EN_HW_RESETf, &temp); 
        } 
        soc_WATCH_DOG_CTRLr_field_set(unit, &data, SOFTWARE_RESETf, &temp); 

        SOC_IF_ERROR_RETURN(REG_WRITE_WATCH_DOG_CTRLr(unit, &data));

        /* Wait for chip reset complete */
        for (i=0; i<100000; i++) {
            data = 0;
            if ((rv = REG_READ_WATCH_DOG_CTRLr(unit, &data)) < 0) {
                /*
                 * Can't check reset status from register.
                 * Anyway, sleep for 3 seconds
                 */
                sal_sleep(3);
                break;
            } 
            temp = 0;
            soc_WATCH_DOG_CTRLr_field_get(unit, &data, SOFTWARE_RESETf, &temp); 
            if (!temp) {
                /* Reset is complete */
                break;
            }
        }
        
        if (soc_feature(unit, soc_feature_int_cpu_arbiter)) {
#if defined(BCM_53125) || defined(BCM_53128)             
            if (SOC_IS_ROBO53125(unit)) {
                model = BCM53125_MODEL_ID;
            } else  if (SOC_IS_ROBO53128(unit)) {
                model = BCM53128_MODEL_ID;
            }
            /* wait internal cpu boot up completely */
            do {
                REG_READ_MODEL_IDr(unit, &data);
            } while (data != model);
#endif /* BCM_53125 || BCM_53128 */            

#if defined (BCM_53125)
            if (SOC_IS_ROBO53125(unit)) {
            /* Check if the internal 8051 is enabled or not */
            REG_READ_STRAP_PIN_STATUSr(unit, &data);
            if (data & BCM53125_STRAP_PIN_8051_MASK) {
                /* Bit 16 is en_8051 */
                SOC_CONTROL(unit)->int_cpu_enabled = TRUE;
            } else {
                SOC_CONTROL(unit)->int_cpu_enabled = FALSE;
            }
            }
#endif /* BCM_53125 */

#if defined (BCM_53128)
            if (SOC_IS_ROBO53128(unit)) {
            /* Check if the internal 8051 is enabled or not */
            REG_READ_STRAP_PIN_STATUSr(unit, &data);
            soc_STRAP_PIN_STATUSr_field_get(unit, 
                &data, GREEN_ENf, &temp);
            SOC_CONTROL(unit)->int_cpu_enabled = (temp) ? TRUE : FALSE;
            }
#endif /* BCM_53128 */
        }
    }
    return rv;

}

static int
soc_robo_board_led_init(int unit)
{
    uint16      dev_id, chip_dev_id;
    uint8       rev_id, chip_rev_id;
    int         dma_unit = 1;
    uint32  data;
    uint64  reg_value64;
    int rv = SOC_E_NONE;
    
    /* check if the robo chip is BCM53242 */
    soc_cm_get_id(dma_unit, &dev_id, &rev_id);

    /* Keystone GMAC core ID */
    if (dev_id == BCM53000_GMAC_DEVICE_ID) {
        soc_cm_get_id(unit, &chip_dev_id, &chip_rev_id);
        if ((SOC_IS_ROBO53242(unit)) ||
            (SOC_IS_TB(unit) && (chip_dev_id == BCM53284_DEVICE_ID))){
            data = 0x104;
            SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC0_CTLr(unit,&data));
            data = 0x106;
            SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC1_CTLr(unit,&data));
            data = 0x6000000;

            if((rv = REG_READ_LED_FUNC_MAPr(unit, (uint32 *)&reg_value64)) < 0){
                return rv;
            }
            soc_LED_FUNC_MAPr_field_set(unit, (uint32 *)&reg_value64, 
                LED_FUNC_MAPf, &data);
            if((rv = REG_WRITE_LED_FUNC_MAPr(unit, (uint32 *)&reg_value64)) < 0){
                return rv;
            }       
        }
    }

    return rv;
}


/*
 * Function:
 *  soc_robo_do_init
 * Purpose:
 *  Optionally reset, and initialize a StrataSwitch.
 * Parameters:
 *  unit  - RoboSwitch unit number.
 *  reset - Boolean, if TRUE, device is reset.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *
 *  This routine may be called after a device is attached
 *  or whenever a chip reset is required.
 *
 * IMPORTANT NOTE:
 *  Only the quickest, most basic things should be initialized here.
 *  This routine may NOT do anything time consuming like scanning ports,
 *  clearing counters, etc.  Such things should be done as commands in
 *  the rc script.  Otherwise, Verilog and Quickturn simulations take
 *  too long.
 */
int
soc_robo_do_init(int unit, int reset)
{
    soc_control_t        *soc;
    int     arl_mod = 0;
#if defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128)    
    uint32  enable;
#endif /* BCM_53101 */
#if defined(BCM_53125)
    uint32 reg_val = 0;
#endif /* BCM_53125 */

    assert(SOC_UNIT_VALID(unit));

    soc = SOC_CONTROL(unit);

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        SOC_ERROR_PRINT((DK_ERR, "soc_robo_do_init: Unit %d not attached\n", unit));
        return(SOC_E_UNIT);
    }

#ifdef ETH_MII_DEBUG
    if (soc_cm_get_dev_type(unit) & SOC_ETHER_DEV_TYPE) {
        return(SOC_E_NONE);
    }
#endif

    if (soc->soc_flags & SOC_F_INITED) {
        /********************************************************************/
        /* If the device has already been initialized before, perform some     */
        /* de-initialization to avoid stomping on existing activities.         */
        /********************************************************************/

        soc->soc_flags &= ~SOC_F_INITED;
    }

    /* Set bitmaps according to which type of device it is */
    soc_robo_info_config(unit, soc);
    
    /* Chip HW Reset */
    if (reset) {
        SOC_IF_ERROR_RETURN(soc_chip_reset(unit));
    }
    /*
     * Configure DMA channels.
     */
    if (soc_robo_dma_init(unit) != 0) {
        SOC_ERROR_PRINT((DK_ERR,
                 "Unit %d: DMA initialization failed, unavailable\n",
                 unit));
        return(SOC_E_INTERNAL);
    }

    /*
     * Chip depended misc init.
     */
    if (soc_misc_init(unit) != 0) {
        SOC_ERROR_PRINT((DK_ERR,
                 "Unit %d: Chip Misc initialization failed, unavailable\n",
                 unit));
        return(SOC_E_INTERNAL);
    }
    
    /* Set feature cache */
    soc_feature_init(unit);

    /*soc_dcb_unit_init(unit);*/ /* Maybe have this function, will be evaluatied */

    /***********************************************************************/
    /* Begin initialization from a known state (reset).                    */
    /***********************************************************************/
    /*
     * PHY drivers and ID map
     */

    SOC_IF_ERROR_RETURN(soc_phyctrl_software_init(unit));

    /*
     * Update saved chip state to reflect values after reset.
     */

    soc->soc_flags &= SOC_F_RESET;
    /*
    Will fill more needed fields in soc_control_t.
    Ex. soc->xxx = xxx;
    */

    /*
     * Configure DMA channels.
     */
    /* Maybe have this function, will be evaluatied */

    /*
    Any SPI Register needed be set ?
    */

    if (reset) {
        SOC_IF_ERROR_RETURN(soc_robo_reset(unit));
        /* It will be implemented when receive chip 
        init default setting from DVT. */
    }
    /* Configure board LED */
    soc_robo_board_led_init(unit);

#if defined(BCM_53125)
    if (SOC_IS_ROBO53125(unit)) {
        /*
         * Add the workaround of crystal jitter issue.
         * Increasing the link delay timer to avoid this issue.
         */
        /* 1. Disable MAC EEE feature at StarFighter level */
        SOC_IF_ERROR_RETURN(
            REG_READ_EEE_EN_CTRLr(unit, &reg_val));
        enable = 0;
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_EN_CTRLr(unit, &enable));

        /* 2. Configure the link delay timer to 1 second */
        enable = 1000000; /* 1000000 us */
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_LINK_DLY_TIMERr(unit,CMIC_PORT(unit),&enable));

        /* 3. Restore MAC EEE feature at StarFighter level */
        SOC_IF_ERROR_RETURN(
            REG_WRITE_EEE_EN_CTRLr(unit, &reg_val));
    }
#endif /* BCM_53125 */

    arl_mod = soc_property_get(unit, spn_L2XMSG_MODE, 1);
    if(arl_mod) {
        SOC_IF_ERROR_RETURN(soc_robo_arl_mode_set(unit,ARL_MODE_ROBO_POLL));
    } else {
        SOC_IF_ERROR_RETURN(soc_robo_arl_mode_set(unit,ARL_MODE_NONE));
    }

#ifdef IMP_SW_PROTECT
    
    soc_imp_prot_init(unit);

#endif /* IMP_SW_PROTECT */

#ifdef BCM5324_SUPPORT_LACP
    /* 
     * Enable this bit for BCM5324 to allow LACP packets pass to CPU
     * when STP status is blocking/disable/learning 
     */
    if (SOC_IS_ROBO5324(unit)) {
        uint32 reg_value, temp;        
        reg_value = 0;
        SOC_IF_ERROR_RETURN(REG_READ_SPECIAL_MNGTr(unit, &reg_value));
        temp = 1;
        soc_SPECIAL_MNGTr_field_set(unit, &reg_value, PASS_BPDU_REVMCASTf, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_SPECIAL_MNGTr(unit, &reg_value));

    }
#endif /* BCM5324_SUPPORT_LACP */

#if defined(BCM_53101) || defined(BCM_53125) || defined(BCM_53128)
    /* Check the MAC LOW POWER MODE enable */
    enable = soc_property_get(unit, spn_AUTO_ENABLE_MAC_LOW_POWER, 0);
    SOC_AUTO_MAC_LOW_POWER(unit) = enable;
    if (!enable) {
        return SOC_E_NONE;
    }
    
    if (SOC_IS_ROBO53101(unit) || SOC_IS_ROBO53125(unit) ||
        SOC_IS_ROBO53128(unit)) {
        uint32 temp = 0;

        /* Get current Low Power Mode */
        SOC_IF_ERROR_RETURN(
            DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_LOW_POWER_ENABLE, &temp));
        
        MAC_LOW_POWER_LOCK(unit);
        
        if (temp) {
            SOC_MAC_LOW_POWER_ENABLED(unit) = 1;
        } else {
            SOC_MAC_LOW_POWER_ENABLED(unit) = 0;
            soc->all_link_down_detected = 0;
        }
        
        MAC_LOW_POWER_UNLOCK(unit);
            
    }
#endif /* BCM_53101 */
    return(SOC_E_NONE);
}

/*
 * Function:
 *    soc_reset_init
 * Purpose:
 *    Reset and initialize a RoboSwitch.
 * Parameters:
 *    unit - RoboSwitch unit number.
 * Returns:
 *    SOC_E_XXX
 */
int
soc_robo_reset_init(int unit)
{
    return(soc_robo_do_init(unit, TRUE));
}

/*
 * Function:
 *    soc_robo_init
 * Purpose:
 *    Initialize the device to a known state. Writes some minor initial 
 *    operating settings to the internal registers (e.g. interrupt 
 *    config, channel config). Initializes the soc_control [unit] 
 *    structure to be consistent with the chip reset state.
 * Parameters:
 *    unit - RoboSwitch unit number.
 * Returns:
 *    SOC_E_XXX
 * Notes:
 *    soc_robo_init provides a way for diagnostics to bring the chip 
 *    to a known state.
 */
int
soc_robo_init(int unit)
{
    return(soc_robo_do_init(unit, FALSE));
}

/*
 * Function:
 *    soc_robo_reset
 * Purpose:
 *    Reset some registers in unit to default working values.
 * Parameters:
 *    unit - RoboSwitch unit number.
 */
int
soc_robo_reset(int unit)
{
    /* It will be implemented when receive chip 
       init default setting from DVT. */
    int rv = SOC_E_NONE;
    soc_port_t port;
    pbmp_t bm;
    uint32 data;
    uint32      temp;

    /* Reset register */
    if (SOC_IS_ROBO5389(unit) || SOC_IS_ROBO5395(unit) ||
        SOC_IS_ROBO5398(unit) || SOC_IS_ROBO5397(unit) ||
        SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)) {

        SOC_IF_ERROR_RETURN(REG_READ_WATCH_DOG_CTRLr(unit,&data));
        temp = 1; /* Enable reset register */
        soc_WATCH_DOG_CTRLr_field_set(unit, &data, EN_RST_REGFILEf, &temp); 
        SOC_IF_ERROR_RETURN(REG_WRITE_WATCH_DOG_CTRLr(unit,&data));
    } 
    SOC_PBMP_ASSIGN(bm, PBMP_ALL(unit));
    PBMP_ITER(bm, port) {

        if (IS_GE_PORT(unit, port) || SOC_IS_ROBO53101(unit)){
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &data));
            if (SOC_IS_TB(unit)) {
                temp = 3; /* Forwarding State*/
                soc_G_PCTLr_field_set(unit, &data, G_STP_STATEf, &temp);
            } else {
                temp = 5; /* Forwarding State */
                soc_G_PCTLr_field_set(unit, &data, G_MISTP_STATEf, &temp);
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr(unit, port, &data));
        } else {
            if(SOC_IS_ROBO5324(unit) || SOC_IS_ROBO5348(unit) ||
                SOC_IS_ROBO5347(unit) || SOC_IS_ROBO53242(unit) ||
                SOC_IS_ROBO53262(unit)){
                temp = 5;
                SOC_IF_ERROR_RETURN( REG_READ_TH_PCTLr(unit, port, &data));
                soc_TH_PCTLr_field_set(unit, &data, MISTP_STATEf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_TH_PCTLr(unit, port, &data));                
            }
            if (SOC_IS_ROBO5396(unit)||
                SOC_IS_ROBO5398(unit)|| SOC_IS_ROBO5397(unit)){
                temp = 5;
                SOC_IF_ERROR_RETURN( REG_READ_MII_PCTLr(unit, port, &data));
                if(SOC_IS_ROBO5396(unit)){
                    soc_MII_PCTLr_field_set(unit, &data, MII_MISTP_STATEf, &temp);
                } else {
                    soc_MII_PCTLr_field_set(unit, &data, STP_STSf, &temp); 
                }
                SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, port, &data));
            }
            if (SOC_IS_TB(unit)) {
                temp = 3;
                SOC_IF_ERROR_RETURN( REG_READ_TH_PCTLr(unit, port, &data));
                soc_TH_PCTLr_field_set(unit, &data, STP_STATEf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_TH_PCTLr(unit, port, &data));                
            }
            if (SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) ||
                SOC_IS_ROBO53118(unit) || SOC_IS_ROBO53101(unit) ||
                SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)){
                continue;
            }
        }
    }

   /* Enable Frame Forwarding, Set Managed Mode */
    SOC_IF_ERROR_RETURN(REG_READ_SWMODEr(unit, &data));
    temp = 1;
    soc_SWMODEr_field_set(unit, &data, SW_FWDG_ENf, &temp);
    soc_SWMODEr_field_set(unit, &data, SW_FWDG_MODEf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_SWMODEr(unit, &data));

    /* LED initialization for BCM5395 */
    if(SOC_IS_ROBO5395(unit) || SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit)
        || SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
        data = 0x4320;
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_FUNC1_CTLr(unit, &data));        
    }
    if(SOC_IS_ROBO53115(unit) || SOC_IS_ROBO53118(unit) || 
        SOC_IS_ROBO53125(unit) || SOC_IS_ROBO53128(unit)) {
        data = SOC_PBMP_WORD_GET(PBMP_PORT_ALL(unit), 0);
        SOC_IF_ERROR_RETURN(REG_WRITE_LED_EN_MAPr(unit, &data));        
        }

    if(SOC_IS_ROBO5324(unit)) {
        /* Clear ARL entries */
        (DRV_SERVICES(unit)->mem_delete)
            (unit, DRV_MEM_MARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL);
    }
    
    if (SOC_IS_ROBO53101(unit)) {
    	/* Enable MDC timing enhancement bit */
        SOC_IF_ERROR_RETURN(
            REG_READ_CTRL_REGr(unit, &data));
        temp = 1;
        soc_CTRL_REGr_field_set(unit, &data, MDC_TIMING_ENHf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_CTRL_REGr(unit, &data));
    }
    
    /* power down the invalid port*/
    soc_robo_power_down_config(unit, bm);

    return rv;
}

/*
 * Function:
 *    soc_detach
 * Purpose:
 *    Detach a SOC device and deallocate all resources allocated.
 * Parameters:
 *    unit - RoboSwitch unit number.
 */
int
soc_robo_detach(int unit)
{
    soc_control_t   *soc;
    soc_mem_t       mem;

    SOC_DEBUG_PRINT((DK_PCI, "Detaching\n"));

    soc = SOC_CONTROL(unit);

    if (NULL == soc) {
        return SOC_E_NONE;
    }

    if (0 == (soc->soc_flags & SOC_F_ATTACHED)) {
        return SOC_E_NONE;
    }

    /* Free up DMA memory */

    /* Will be added or changed later */

    /* Clear all outstanding DPCs owned by this unit */
    sal_dpc_cancel(INT_TO_PTR(unit));
    /* Will be added or changed later */

    /*
     * Call soc_robo_init to cancel link scan task, counter DMA task,
     * outstanding DMAs, interrupt generation, and anything else the
     * driver or chip may be doing.
     */

#if !defined(PLISIM)
    SOC_IF_ERROR_RETURN(soc_robo_reset_init(unit));
#endif

    /*
     * PHY drivers and ID map
     */
    SOC_IF_ERROR_RETURN(soc_phyctrl_software_deinit(unit));

    /* Detach interrupt handler, if we installed one */
    /* unit # is ISR arg */
    if (soc_cm_interrupt_disconnect(unit) < 0) {
        soc_cm_print("soc_detach: could not disconnect interrupt line\n");
        return SOC_E_INTERNAL;
    }

    /*
     * When detaching, take care to free up only resources that were
     * actually allocated, in case we are cleaning up after an attach
     * that failed part way through.
     */

    /* Will be added or changed later */


    /* Terminate counter module; frees allocated space */
    if(soc_robo_counter_detach(unit)){
        soc_cm_print("soc_detach: could not detach counter thread!\n");
        return SOC_E_INTERNAL;
    }


    /* Will be added or changed later */
    /* for ex. 
        if (soc->schanMutex) {
            sal_mutex_destroy(soc->schanMutex);
            soc->schanMutex = NULL;
        }
    */
    if (soc->miimMutex) {
        sal_mutex_destroy(soc->miimMutex);
        soc->miimMutex = NULL;
    }

    if (soc->overrideMutex) {
        sal_mutex_destroy(soc->overrideMutex);
        soc->overrideMutex = NULL;
    }


    SOC_FLAGS_CLR(soc, SOC_F_ATTACHED);

    if(soc_robo_arl_mode_set(unit,ARL_MODE_NONE)){
        soc_cm_print("soc_detach: could not detach arl thread!\n");
        return SOC_E_INTERNAL;
    }

    if (soc->arl_table_Mutex){
        sal_mutex_destroy(soc->arl_table_Mutex);
        soc->arl_table_Mutex = NULL;
    }
    
    if (soc->arl_mem_search_Mutex) {
        sal_mutex_destroy(soc->arl_mem_search_Mutex);
        soc->arl_mem_search_Mutex = NULL;
    }

    if (soc->mem_rwctrl_reg_Mutex) {
        sal_mutex_destroy(soc->mem_rwctrl_reg_Mutex);
        soc->mem_rwctrl_reg_Mutex = NULL;
    }

    for (mem = 0; mem < NUM_SOC_ROBO_MEM; mem++) {
        if (SOC_MEM_IS_VALID(unit, mem)) {
            
            if (soc->memState[mem].lock != NULL) {
                sal_mutex_destroy(soc->memState[mem].lock);
                soc->memState[mem].lock = NULL;
            }
        }
    }
    
    sal_free(SOC_CONTROL(unit));
    SOC_CONTROL(unit) = NULL;

    if (--soc_ndev_attached == 0) {
    /* Work done after the last SOC device is detached. */
    /* (currently nothing) */
        if (spiMutex) {
            sal_mutex_destroy(spiMutex);
            spiMutex = NULL;
        }
#ifdef MDC_MDIO_SUPPORT
        if (mdc_data_lock) {
            sal_mutex_destroy(mdc_data_lock);
            mdc_data_lock = NULL;
        }
#endif /* MDC_MDIO_SUPPORT */
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *    _soc_functions_find
 * Purpose:
 *    Return function array corresponding to the driver
 * Returns:
 *    Pointer to static function array soc_functions_t
 */

static soc_functions_t *
_soc_functions_find(soc_driver_t *drv)
{
    switch (drv->type) {

#ifdef BCM_ROBO_SUPPORT
    case SOC_CHIP_BCM5324_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5324_A1:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5324_A2:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5396_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5389_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5398_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5348_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5397_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5347_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM5395_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53242_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53262_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53115_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53118_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53280_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53280_B0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53101_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53125_A0:  return &soc_robo_drv_funs;
    case SOC_CHIP_BCM53128_A0:  return &soc_robo_drv_funs;
#endif

    default:
        break;
    }

    soc_cm_print("_soc_functions_find: no functions for this chip\n");
    assert(0);
    return NULL;
}

/*
 * Function:
 *    _soc_device_created
 * Purpose:
 *    Called by soc_cm_device_create() to tell the driver a
 *    device is being created.
 * Notes:
 *    This function may perform management initialization only.
 *    It may not touch the chip, as its accessors will not
 *    have been setup yet.
 *
 *    This function is a result of the prior organization,
 *    and is here to make the transition easier.
 *    and this should all probably be restructured to match
 *    the new configuration code.
 */

int
_soc_robo_device_created(int unit)
{
    soc_control_t        *soc;
    int         chip;
    uint16        dev_id;
    uint8        rev_id;

    if (SOC_CONTROL(unit) != NULL) {
        return SOC_E_EXISTS;
    }
    soc = SOC_CONTROL(unit) = sal_alloc(sizeof(soc_control_t), "soc_control");
    if (soc == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(soc, 0, sizeof(soc_control_t));

    if (soc_cm_get_dev_type(unit) & SOC_ETHER_DEV_TYPE) {
        if (soc_eth_unit == -1) {
            soc_eth_unit = unit;
        }
        soc_eth_ndev++;

#ifdef ETH_MII_DEBUG
        /*
         * Instantiate the driver -- Verify chip revision matches driver
         * compilation revision.
         */

        soc_cm_get_id(unit, &dev_id, &rev_id);

        soc->chip_driver = soc_robo_chip_driver_find(dev_id, rev_id);
        soc->soc_functions = NULL;

#endif
        return 0;
    }

    /*
     * Instantiate the driver -- Verify chip revision matches driver
     * compilation revision.
     */

    soc_cm_get_id(unit, &dev_id, &rev_id);

    soc->chip_driver = soc_robo_chip_driver_find(dev_id, rev_id);
    if (soc->chip_driver == NULL) {
        SOC_ERROR_PRINT((DK_ERR, 
            "_soc_robo_device_created: unit %d has no driver "
                         "(device 0x%04x rev 0x%02x)\n",
                         unit, dev_id, rev_id));
        return SOC_E_UNAVAIL;
    }
    soc->soc_functions = _soc_functions_find(soc->chip_driver);

    if (soc->chip_driver == NULL) {
        SOC_ROBO_FIRST_ACTIVE_DRIVER(chip);
        soc->chip_driver = soc_robo_base_driver_table[chip];
        soc_cm_print("#### \n");
        soc_cm_print("#### Chip not recognized. \n");
        soc_cm_print("#### Device id:  0x%04x.  Rev id:  0x%02x\n",
                 dev_id, rev_id);
        soc_cm_print("#### Installing default driver for device %s.\n",
                 SOC_CHIP_NAME(soc->chip_driver->type));
        soc_cm_print("#### Unpredictable behavior may result.\n");
        soc_cm_print("#### \n");
    }

    return 0;
}

/*
 * Function:
 *    soc_robo_attach
 * Purpose:
 *    Initialize the soc_control_t structure for a device, allocating all memory
 *    and semaphores required.
 * Parameters:
 *    unit - StrataSwitch unit #.
 *    detach - Callback function called on detach.
 * Returns:
 *     SOC_E_XXX
 * Notes:
 *    No chip initialization is done other than masking all interrupts,
 *    see soc_init or soc_robo_reset_init.
 */
int
soc_robo_attach(int unit)
{
    soc_control_t        *soc;
    soc_mem_t        mem;
#ifdef MDC_MDIO_SUPPORT
    int i;
#endif /* MDC_MDIO_SUPPORT */

    SOC_DEBUG_PRINT((DK_PCI, "soc_robo_attach: unit=%d\n", unit));

    _soc_robo_device_created(unit);

    soc = SOC_CONTROL(unit);

    /* Install the Interrupt Handler */
    /* Make sure interrupts are masked before connecting line. */

    /* Will be added or changed later */

    /*
     * Attached flag must be true during initialization.
     * If initialization fails, the flag is cleared by soc_detach (below).
     */

    soc->soc_flags |= SOC_F_ATTACHED;

    if (soc_ndev_attached++ == 0) {
        int            chip;

        /* Work to be done before the first SOC device is attached. */
        for (chip = 0; chip < SOC_ROBO_NUM_SUPPORTED_CHIPS; chip++) {

            /* Call each chip driver's init function */
            if (soc_robo_base_driver_table[chip]->init) {

                (soc_robo_base_driver_table[chip]->init)();
            }

        }
    }

    if (spiMutex == NULL) {
        if ((spiMutex = sal_mutex_create("SPI")) == NULL) {
            goto error;
        }
    }
#ifdef MDC_MDIO_SUPPORT
    if (mdc_data_lock == NULL) {
        if ((mdc_data_lock = sal_mutex_create("MDC data lock")) == NULL) {
            goto error;
        }
    }
    /* Initialize global variable "access_page" for Pseudo Phy access commands. */
    for (i = 0; i < SOC_MAX_NUM_SWITCH_DEVICES; i++) {
        access_page[i] = 0xff;
    }
#endif /* MDC_MDIO_SUPPORT */

    /*
     * Set up port bitmaps.  They are also set up on each soc_robo_init so
     * they can be changed from a CLI without rebooting.
     */
    soc_robo_info_config(unit, soc);
    /* Set feature cache */
    soc_feature_init(unit);
    /*soc_dcb_unit_init(unit);*/
    /* Will be added or changed later */

    /*
     * Initialize memory index_maxes. Chip specific overrides follow.
     */
    /* Will be added or changed later */
        /*
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        soc->memState[mem].index_max = SOC_MEM_INFO(unit, mem).index_max;
        }
        */
    if ((soc->miimMutex = sal_mutex_create("MIIM")) == NULL) {
        goto error;
    }

    if ((soc->overrideMutex = sal_mutex_create("Override")) == NULL) {
        goto error;
    }

    /* Allocate counter module resources */
    if (soc_robo_counter_attach(unit)) {
        goto error;
    }

    /*
     * Create mutices
     */
    for (mem = 0; mem < NUM_SOC_ROBO_MEM; mem++) {
        /*
         * should only create mutexes for valid memories.
         */
        if ((soc->memState[mem].lock =
             sal_mutex_create(SOC_ROBO_MEM_NAME(unit, mem))) == NULL) {
            goto error;
        }
    }

    if ((soc->arl_table_Mutex = sal_mutex_create("ARL_SW_TABLE")) == NULL) {
        goto error;
    }
    
    if ((soc->arl_mem_search_Mutex = sal_mutex_create("ARL_MEM_SEARCH")) == 
        NULL) {
        goto error;
    }

    if ((soc->mem_rwctrl_reg_Mutex = sal_mutex_create("MEM_RWCTRL_REG")) == 
        NULL) {
        goto error;
    }
    
    if ((soc->arl_notify =
         sal_sem_create("ARL interrupt", sal_sem_BINARY, 0)) == NULL) {
        goto error;
    }

#if defined(BCM_53101)
    if ((soc->mac_low_power_mutex = sal_mutex_create("MAC_LOW_POWER")) ==
        NULL) {
        goto error;
    }
#endif /* BCM_53101 */

#if defined(BCM_53125) || defined(BCM_53128)
    if ((soc->arbiter_mutex = sal_mutex_create("ARBITER")) ==
        NULL) {
        goto error;
    }
#endif /* BCM_53125  || BCM_53128 */

    soc->arl_pid = SAL_THREAD_ERROR;

    /* Initialize DMA */
    /* Will be added or changed later */

    /* Clear statistics */

    sal_memset(&soc->stat, 0, sizeof(soc->stat));

    /*
     * Configure nominal IPG register settings.
     * Also make some chip-dependent adjustments.
     * These defaults may be overridden by properties in mac.c.
     */
    /* Will be added or changed later */

    soc->ipg.fe_hd_10 = 0x12;
    soc->ipg.fe_hd_100 = 0x12;
    soc->ipg.fe_fd_10 = 0x15;
    soc->ipg.fe_fd_100 = 0x15;
    soc->ipg.gth_hd_10 = 0x12;
    soc->ipg.gth_hd_100 = 0x12;
    soc->ipg.gth_fd_10 = 0x15;
    soc->ipg.gth_fd_100 = 0x15;
    soc->ipg.ge_hd_1000 = 0x0c;
    soc->ipg.ge_hd_2500 = 0x0c;
    soc->ipg.ge_fd_1000 = 0x0c;
    soc->ipg.ge_fd_2500 = 0x0c;
    soc->ipg.bm_fd_10000 = 0x08;

    if (soc_cm_get_dev_type(unit) & SOC_PCI_DEV_TYPE) {
        soc_cm_print("PCI device %s attached as unit %d.\n",
             soc_dev_name(unit), unit);
    } else if (soc_cm_get_dev_type(unit) & SOC_SPI_DEV_TYPE) {
        soc_cm_print("SPI device %s attached as unit %d.\n",
             soc_dev_name(unit), unit);
    }

    return(SOC_E_NONE);

 error:
    soc_cm_debug(DK_ERR, "soc_robo_attach: unit %d failed\n", unit);

    soc_robo_detach(unit);        /* Perform necessary clean-ups on error */
    return SOC_E_MEMORY;

}


#if defined(BROADCOM_DEBUG)
#define P soc_cm_print
/*
 * Function:
 *    soc_dump
 * Purpose:
 *    Dump useful information from the soc structure.
 * Parameters:
 *    unit - unit number to dump
 *    pfx - character string to prefix output line.
 * Returns:
 *    SOC_E_XXX
 */
int
soc_robo_dump(int unit, const char *pfx)
{
    soc_control_t       *soc;
    soc_stat_t      *stat;
    uint16          dev_id;
    uint8           rev_id;

    if (!SOC_UNIT_VALID(unit)) {
        return(SOC_E_UNIT);
    }

    soc = SOC_CONTROL(unit);

    stat = &soc->stat;

    P("%sUnit %d Driver Control Structure:\n", pfx, unit);

    soc_cm_get_id(unit, &dev_id, &rev_id);

    P("%sChip=%s Rev=0x%02x Driver=%s\n",
      pfx,
      soc_dev_name(unit),
      rev_id,
      SOC_CHIP_NAME(soc->chip_driver->type));
    P("%sFlags=0x%x:",
      pfx, soc->soc_flags);
    if (soc->soc_flags & SOC_F_ATTACHED)        P(" attached");
    if (soc->soc_flags & SOC_F_INITED)          P(" initialized");
    if (soc->soc_flags & SOC_F_LSE)             P(" link-scan");
    P("\n");

    P("%s", pfx);
    soc_cm_dump(unit);

    P("%sCounter: int=%dus per=%dus \n",
      pfx,
      soc->counter_interval,
      SAL_USECS_SUB(soc->counter_coll_cur, soc->counter_coll_prev));

    return(0);
}

/*
 * Function:
 *    soc_chip_dump
 * Purpose:
 *    Display driver and chip information
 * Notes:
 *    Pass unit -1 to avoid referencing unit number.
 */
void
soc_robo_chip_dump(int unit, soc_driver_t *d)
{
    soc_info_t          *si;
    int                 i;
    soc_port_t          port;
    char                pfmt[SOC_PBMP_FMT_LEN];
    uint16              dev_id;
    uint8               rev_id;
    int                 blk, bindex;
    char                *bname = NULL, *bpname = NULL;

    if (d == NULL) {
        P("unit %d: no driver attached\n", unit);
        return;
    }

    P("driver %s (%s)\n", SOC_CHIP_NAME(d->type), d->chip_string);
    P("\tregsfile\t\t%s\n", d->origin);
    P("\tpci identifier\t\tvendor 0x%04x device 0x%04x rev 0x%02x\n",
      d->pci_vendor, d->pci_device, d->pci_revision);
    P("\tclasses of service\t%d\n", d->num_cos);
    P("\tmaximums\t\tblock %d ports %d mem_bytes %d\n",
      SOC_ROBO_MAX_NUM_BLKS, SOC_ROBO_MAX_NUM_PORTS, SOC_ROBO_MAX_MEM_BYTES);

    for (blk = 0; d->block_info[blk].type >= 0; blk++) {
        if (unit < 0) {
            for (i = 0; soc_block_names[i].blk != SOC_BLK_NONE; i++) {
                if (soc_block_names[i].blk == d->block_info[blk].type) {
                    bname = soc_block_names[i].name;
                    break;
                }
            }
        } else {
            bname = soc_block_name_lookup_ext(d->block_info[blk].type, unit);
        }
        P("\tblk %d\t\t%s%d\tcmic %d\n",
          blk,
          bname,
          d->block_info[blk].number,
          d->block_info[blk].cmic);
    }
    for (port = 0; ; port++) {
        blk = d->port_info[port].blk;
        bindex = d->port_info[port].bindex;
        if (blk < 0 && bindex < 0) {            /* end of list */
            break;
        }
        if (blk < 0) {                          /* empty slot */
            continue;
        }
        if (unit < 0) {
            for (i = 0; soc_block_port_names[i].blk != SOC_BLK_NONE; i++) {
                if (soc_block_port_names[i].blk == d->block_info[blk].type) {
                    bpname = soc_block_port_names[i].name;
                }
            }
            
            for (i = 0; soc_block_names[i].blk != SOC_BLK_NONE; i++) {
                if (soc_block_names[i].blk == d->block_info[blk].type) {
                    bname = soc_block_names[i].name;
                }
            }
        } else {
            bpname = soc_block_port_name_lookup_ext(d->block_info[blk].type, unit);
            bname = soc_block_name_lookup_ext(d->block_info[blk].type, unit);
        }
        P("\tport %d\t\t%s\tblk %d %s%d.%d\n",
          port,
          bpname,
          blk,
          bname,
          d->block_info[blk].number,
          bindex);
    }

    if (unit < 0) {
        return;
    }
    si = &SOC_INFO(unit);
    soc_cm_get_id(unit, &dev_id, &rev_id);
    P("unit %d:\n", unit);
    P("\tpci\t\t\tdevice %04x rev %02x\n", dev_id, rev_id);
    P("\tdriver\t\t\ttype %d (%s) group %d (%s)\n",
      si->driver_type, SOC_CHIP_NAME(si->driver_type),
      si->driver_group, soc_chip_group_names[si->driver_group]);
    P("\tchip\t\t\t%s\n",
      SOC_IS_ROBO(unit) ? "robo " : "");
    P("\tnum ports\t\t%d\n", si->port_num);
    P("\tnum blocks\t\t%d\n", si->block_num);
    P("\tFE ports\t%d\t%s (%d:%d)\n",
      si->fe.num, SOC_PBMP_FMT(si->fe.bitmap, pfmt),
      si->fe.min, si->fe.max);
    P("\tGE ports\t%d\t%s (%d:%d)\n",
      si->ge.num, SOC_PBMP_FMT(si->ge.bitmap, pfmt),
      si->ge.min, si->ge.max);
    P("\tXE ports\t%d\t%s (%d:%d)\n",
      si->xe.num, SOC_PBMP_FMT(si->xe.bitmap, pfmt),
      si->xe.min, si->xe.max);
    P("\tHG ports\t%d\t%s (%d:%d)\n",
      si->hg.num, SOC_PBMP_FMT(si->hg.bitmap, pfmt),
      si->hg.min, si->hg.max);
    P("\tETHER ports\t%d\t%s (%d:%d)\n",
      si->ether.num, SOC_PBMP_FMT(si->ether.bitmap, pfmt),
      si->ether.min, si->ether.max);
    P("\tPORT ports\t%d\t%s (%d:%d)\n",
      si->port.num, SOC_PBMP_FMT(si->port.bitmap, pfmt),
      si->port.min, si->port.max);
    P("\tALL ports\t%d\t%s (%d:%d)\n",
      si->all.num, SOC_PBMP_FMT(si->all.bitmap, pfmt),
      si->all.min, si->all.max);
    P("\tIPIC port\t%d\tblock %d\n", si->ipic_port, si->ipic_block);
    P("\tCMIC port\t%d\t%s block %d\n", si->cmic_port,
      SOC_PBMP_FMT(si->cmic_bitmap, pfmt), si->cmic_block);

    P("\thas blocks\t0x%x\t", si->has_block);
    for (i = 0; soc_block_names[i].blk != SOC_BLK_NONE; i++) {
        if (soc_block_names[i].isalias) {
            continue;
        }
        if (soc_block_names[i].blk & si->has_block) {
            P("%s ", soc_block_names[i].name);
        }
    }
    P("\n");
    P("\tport names\t\t");
    for (port = 0; port < si->port_num; port++) {
        if (port > 0 && (port % 5) == 0) {
            P("\n\t\t\t\t");
        }
        P("%d=%s\t",
          port, si->port_name[port]);
    }
    P("\n");
    i = 0;
    for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
        if (SOC_PBMP_IS_NULL(si->block_bitmap[blk])) {
            continue;
        }
        if (++i == 1) {
            P("\tblock bitmap\t");
        } else {
            P("\n\t\t\t");
        }
        P("%d\t%s\t%s (%d ports)",
          blk,
          si->block_name[blk],
          SOC_PBMP_FMT(si->block_bitmap[blk], pfmt),
          si->block_valid[blk]);
    }
    if (i > 0) {
        P("\n");
    }

    {
        soc_feature_t f;

        P("\tfeatures\t");
        i = 0;
        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                if (++i > 3) {
                    P("\n\t\t\t");
                    i = 1;
                }
                P("%s ", soc_feature_name[f]);
            }
        }
        P("\n");
    }
}

#undef P

#endif /* BROADCOM_DEBUG */

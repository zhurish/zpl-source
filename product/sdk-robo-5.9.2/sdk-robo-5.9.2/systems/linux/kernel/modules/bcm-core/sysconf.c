/*
 * $Id: sysconf.c,v 1.11.74.1 Broadcom SDK $
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

#include <sal/types.h>

#include <soc/cmext.h>
#include <soc/drv.h>

#include <ibde.h>
#include <bcm-core.h>

#include <gmodule.h>
#include <kconfig.h>

static int bcore_sysconf_probe_done;

extern int bde_create(void);

/* SOC Configuration Manager device vectors */

static char *
_config_var_get(soc_cm_dev_t *dev, const char *property)
{
    COMPILER_REFERENCE(dev);

    return kconfig_get(property);
}

static void
_write(soc_cm_dev_t *dev, uint32 addr, uint32 data)
{
    bde->write(dev->dev, addr, data);
}

static uint32
_read(soc_cm_dev_t *dev, uint32 addr)
{
    return bde->read(dev->dev, addr);
}

static void
_pci_conf_write(soc_cm_dev_t *dev, uint32 addr, uint32 data)
{
    bde->pci_conf_write(dev->dev, addr, data);
}

static uint32
_pci_conf_read(soc_cm_dev_t *dev, uint32 addr)
{
    return bde->pci_conf_read(dev->dev, addr);
}

static void *
_salloc(soc_cm_dev_t *dev, int size, const char *name)
{
    COMPILER_REFERENCE(name);
    return bde->salloc(dev->dev, size, name);
}

static void
_sfree(soc_cm_dev_t *dev, void *ptr)
{
    bde->sfree(dev->dev, ptr);
}

static int
_sflush(soc_cm_dev_t *dev, void *addr, int length)
{
    return (bde->sflush) ? bde->sflush(dev->dev, addr, length) : 0;
}

static int
_sinval(soc_cm_dev_t *dev, void *addr, int length)
{
    return (bde->sinval) ? bde->sinval(dev->dev, addr, length) : 0;
}

static uint32 
_l2p(soc_cm_dev_t *dev, void *addr)
{
    return (bde->l2p) ? bde->l2p(dev->dev, addr) : 0;
}

static void*
_p2l(soc_cm_dev_t *dev, uint32 addr)
{
    return (bde->p2l) ? bde->p2l(dev->dev, addr) : 0;
}

static int
_interrupt_connect(soc_cm_dev_t *dev, soc_cm_isr_func_t handler, void *data)
{
    return bde->interrupt_connect(dev->dev, handler, data);
}

static int
_interrupt_disconnect(soc_cm_dev_t *dev)
{
    return 0;
}
#ifdef BCM_ROBO_SUPPORT
static void
_spi_read(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len)
{
    bde->spi_read(dev->dev, addr, buf, len);
}

static void
_spi_write(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len)
{
    bde->spi_write(dev->dev, addr, buf, len);
}
#ifdef ROBO_I2C
static void
_i2c_read(soc_cm_dev_t *dev, uint16 addr, uint8 *buf, int len)
{
    bde->i2c_read(dev->dev, addr, buf, len);
}

static void
_i2c_write(soc_cm_dev_t *dev, uint16 addr, uint8 *buf, int len)
{
    bde->i2c_write(dev->dev, addr, buf, len);
}

static void
_i2c_read_intr(soc_cm_dev_t *dev, uint8 chipid, uint8 *buf, int len)
{
    bde->i2c_read_intr(dev->dev, chipid, buf, len);
}

static void
_i2c_read_ARA(soc_cm_dev_t *dev, uint8 *chipid, int len)
{
    bde->i2c_read_ARA(dev->dev, chipid, len);
}
#endif
#endif
/*
 * Function: bcore_sysconf_probe
 *
 * Purpose:
 *    Searches for known devices and creates Configuration 
 *    Manager instances.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
int
bcore_sysconf_probe(void)
{
    int u;
    int cm_dev = 0;
    uint16 devID;
    uint8 revID;
#ifdef KEYSTONE
    uint8 find_robo=0;
#endif

    if (bcore_sysconf_probe_done) {
	return -1;
    }
    bcore_sysconf_probe_done = 1;

    /* Initialize system BDE */
    if (bde_create()) {
	return -1;
    }

#ifdef KEYSTONE
    for (u = 0; u < bde->num_devices(BDE_ALL_DEVICES) && u < SOC_MAX_NUM_DEVICES; u++) {
        if (bde->get_dev_type(u) & BDE_SPI_DEV_TYPE) {
            find_robo = 1;
        }
    }        
#endif
    /* Iterate over devices */
    for (u = 0; u < bde->num_devices(BDE_ALL_DEVICES) && u < SOC_MAX_NUM_DEVICES; u++) {
	const ibde_dev_t *dev = bde->get_dev(u);
	devID = dev->device;
	revID = dev->rev;

	if (soc_cm_device_supported(devID, revID) < 0) {
	    /* Not a switch chip; continue probing other devices */
	    return 0;
	}
#ifdef KEYSTONE
       if (bde->get_dev_type(u) & BDE_ETHER_DEV_TYPE) {
            if (!find_robo) {
                continue;
            }
       }
#endif
	/* Create a device handle, but don't initialize yet. */
	cm_dev = soc_cm_device_create(devID, revID, NULL);

	assert(cm_dev >= 0);
	assert(cm_dev == u);
    }

    return 0;
}

/*
 * Function: bcore_sysconf_attach
 *
 * Purpose:
 *    Install SOC Configuration Manager device vectors and
 *    initialize SOC device.
 * Parameters:
 *    unit - SOC device
 * Returns:
 *    SOC_E_XXX
 */
int
bcore_sysconf_attach(int unit)
{
    soc_cm_device_vectors_t vectors;

    assert(unit >= 0 && unit < bde->num_devices(BDE_ALL_DEVICES));

    bde->pci_bus_features(unit, &vectors.big_endian_pio,
			  &vectors.big_endian_packet,
			  &vectors.big_endian_other);

    vectors.config_var_get = _config_var_get;
    vectors.interrupt_connect = _interrupt_connect;
    vectors.interrupt_disconnect= _interrupt_disconnect;
#if defined(PCI_DECOUPLED) || defined(BCM_ICS)
    vectors.base_address = 0;
#else
    vectors.base_address = bde->get_dev(unit)->base_address;
#endif
    vectors.read = _read;
    vectors.write = _write;
    vectors.pci_conf_read = _pci_conf_read;
    vectors.pci_conf_write = _pci_conf_write;
    vectors.salloc = _salloc;
    vectors.sfree = _sfree;
    vectors.sinval = _sinval;
    vectors.sflush = _sflush;
    vectors.l2p = _l2p;
    vectors.p2l = _p2l;
#ifdef BCM_ROBO_SUPPORT    
    vectors.spi_read = _spi_read;
    vectors.spi_write = _spi_write;
    vectors.bus_type = bde->get_dev_type(unit);
#ifdef ROBO_I2C
    vectors.i2c_read = _i2c_read;
    vectors.i2c_write = _i2c_write;
    vectors.i2c_read_intr = _i2c_read_intr;
    vectors.i2c_read_ARA = _i2c_read_ARA;
#endif
#endif
    /* Initialize default config values */
    kconfig_set("os", "linux");

    return soc_cm_device_init(unit, &vectors);
}

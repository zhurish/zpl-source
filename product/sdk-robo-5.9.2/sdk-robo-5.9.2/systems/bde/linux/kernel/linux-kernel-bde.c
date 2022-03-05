/*
 * $Id: linux-kernel-bde.c,v 1.226.4.9 Broadcom SDK $
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
 * Linux Kernel BDE
 *
 *
 * DMA memory allocation modes
 * ===========================
 *
 * 1. Using private pool in kernel memory
 * --------------------------------------
 * In this mode the BDE module will try to assemble a physically contiguous
 * of memory using the kernel page allocator. This memory block is then
 * administered by the mpool allocation functions. Note that once a system
 * has been running for a while, the memory fragmentation may prevent the
 * allocator from assembling a contiguous memory block, however, if the
 * module is loaded shortly after system startup, it is very unlikely to
 * fail.
 *
 * This allocation method is used by default.
 *
 * 2. Using private pool in high memory
 * ------------------------------------
 * In this mode the BDE module will assume that unused physical memory is
 * present at the high_memory address, i.e. memory not managed by the Linux
 * memory manager. This memory block is mapped into kernel space and
 * administered by the mpool allocation functions. High memory must be
 * reserved using either the mem=xxx kernel parameter (recommended), or by
 * hardcoding the memory limit in the kernel image.
 *
 * The module parameter himem=1 enables this allocation mode.
 *
 * 3. Using kernel allocators (kmalloc, __get_free_pages)
 * ------------------------------------------------------
 * In this mode all DMA memory is allocated from the kernel on the fly, i.e.
 * no private DMA memory pool will be created. If large memory blocks are
 * only allocated at system startup (or not at all), this allocation method
 * is the most flexible and memory-efficient, however, it is not recommended
 * for non-coherent memory platforms due to an overall system performance
 * degradation arising from the use of cache flush/invalidate instructions.
 *
 * The module parameter dmasize=0M enables this allocation mode, however if
 * DMA memory is requested from a user mode application, a private memory
 * pool will be created and used irrespectively.
 */

#include <gmodule.h>
#include <linux-bde.h>
#include <mpool.h>
#include <linux/delay.h>
#include <sdk_config.h>

#ifdef BCM_PLX9656_LOCAL_BUS
#include <asm/cacheflush.h>
#endif

#ifdef BCM_ROBO_SUPPORT
/* robo/et related header files */
#include <shared/et/typedefs.h>
#include <shared/et/bcmdevs.h>

#include <shared/et/osl.h>
#include <shared/et/sbconfig.h>
#include <shared/et/bcmenet47xx.h>

#ifdef KEYSTONE
#include <shared/et/aiutils.h>
#include <sbchipc.h>
#include <etc_robo_spi.h>
#include <soc/gmac0_core.h>
#else /* !KEYSTONE */
#include <shared/et/sbutils.h>
#include <etc_robo.h>
#endif /* KEYSTONE */
#endif /* BCM_ROBO_SUPPORT */

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Kernel BDE");
MODULE_LICENSE("Proprietary");

/* DMA memory pool size */
static char *dmasize;
LKM_MOD_PARAM(dmasize, "s", charp, 0);
MODULE_PARM_DESC(dmasize,
"Specify DMA memory size (default 4MB)");

/* Use high memory for DMA */
static char *himem;
LKM_MOD_PARAM(himem, "s", charp, 0);
MODULE_PARM_DESC(himem,
"Use high memory for DMA (default no)");

/* Allow override of default value */
#ifndef BDE_PCIEFIXUP
#define BDE_PCIEFIXUP 1
#endif

/* Disable PCIe fixups required for some XGS4 devices */
int pciefixup;
LKM_MOD_PARAM(pciefixup, "i", int, BDE_PCIEFIXUP);
MODULE_PARM_DESC(pciefixup,
"Limit payload size and disable relaxed ordering");

/* Ignore all recognized devices (for debug purposes) */
int nodevices;
LKM_MOD_PARAM(nodevices, "i", int, 0);
MODULE_PARM_DESC(nodevices,
"Ignore all recognized devices (default no)");

/*
 * This usually is defined at /usr/include/linux/pci_ids.h
 * But this ID is newer.
 */
#ifndef PCI_DEVICE_ID_PLX_9656
#define PCI_DEVICE_ID_PLX_9656 0x9656
#endif

#ifndef PCI_DEVICE_ID_PLX_9056
#define PCI_DEVICE_ID_PLX_9056 0x9056
#endif

/* For 2.4.x kernel support */
#ifndef IRQF_SHARED
#define IRQF_SHARED     SA_SHIRQ
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
typedef unsigned long resource_size_t;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18) */

#ifdef BCM_ICS
#define BCM_ICS_CMIC_BASE       0x08000000
#else

/* Force interrupt line */
static int forceirq = -1;
static uint32_t forceirqubm = 0xffffffff;
LKM_MOD_PARAM(forceirq, "i", int, 0);
LKM_MOD_PARAM(forceirqubm, "i", uint, 0);
MODULE_PARM_DESC(forceirq,
"Override IRQ line assigned by boot loader");
MODULE_PARM_DESC(forceirqubm,
"Bitmap for overriding the IRQ line assigned by boot loader for given units");

#endif /* BCM_ICS */

/* Debug output */
static int debug;
LKM_MOD_PARAM(debug, "i", int, 0);
MODULE_PARM_DESC(debug,
"Set debug level (default 0");
/* Use high memory for DMA */

/* module param for probing EB devices. */
static char *eb_bus;
LKM_MOD_PARAM(eb_bus, "s", charp, 0);
MODULE_PARM_DESC(eb_bus,
"List of EB devices on platform. Input format (BA=%x IRQ=%d RD16=%d WR16=%d");


#ifndef MDC_MDIO_SUPPORT
#ifdef KEYSTONE
/* Force SPI Frequency */
static int spifreq = 0;
LKM_MOD_PARAM(spifreq, "i", int, 0);
MODULE_PARM_DESC(spifreq,
"Force SPI Frequency for Keystone CPU (0 for default frequency)");
#endif
#endif

/* Compatibility */
#ifdef LKM_2_4
#define _ISR_RET void
#define IRQ_HANDLED
#define MEM_MAP_RESERVE mem_map_reserve
#define MEM_MAP_UNRESERVE mem_map_unreserve
#else /* LKM_2_6 */
#define _ISR_RET int
#define MEM_MAP_RESERVE SetPageReserved
#define MEM_MAP_UNRESERVE ClearPageReserved
char * ___strtok;
char * strtok(char * s,const char * ct)
{
    char *sbegin, *send;
    sbegin  = s ? s : ___strtok;
    if (!sbegin) {
        return NULL;
    }
    sbegin += strspn(sbegin,ct);
    if (*sbegin == '\0') {
        ___strtok = NULL;
        return( NULL );
    }
    send = strpbrk( sbegin, ct);
    if (send && *send != '\0')
        *send++ = '\0';
    ___strtok = send;
    return (sbegin);
}
LKM_EXPORT_SYM(___strtok);
LKM_EXPORT_SYM(strtok);
#endif /* LKM_2_x */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#define VIRT_TO_PAGE(p)     virt_to_page((void*)(p))
#else
#define VIRT_TO_PAGE(p)     virt_to_page((p))
#endif

#if defined(CONFIG_IDT_79EB334) || defined(CONFIG_BCM4702)
/* ioremap is broken in kernel */
#define IOREMAP(addr, size) ((void *)KSEG1ADDR(addr))
#else
#define IOREMAP(addr, size) ioremap_nocache(addr, size)
#endif

#if defined (__mips__)
#if defined(CONFIG_NONCOHERENT_IO) || defined(CONFIG_DMA_NONCOHERENT)
/* Use flush/invalidate for cached memory */
#define NONCOHERENT_DMA_MEMORY
/* Remap virtual DMA addresses to non-cached segment */
#define REMAP_DMA_NONCACHED
#endif /* CONFIG_NONCOHERENT_IO || CONFIG_DMA_NONCOHERENT */
#endif /* __mips__ */


/* Structure of private spi device */
struct spi_dev {
    uint8          cid;         /* Chip ID */
    uint32          part;        /* Part number of the chip */
    uint8          rev;         /* Revision of the chip */
    void           *robo;       /* ptr to robo info required to access SPI */
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
};

struct bde_spi_device_id {
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
};

/* Control Data */
typedef struct bde_ctrl_s {
    struct list_head list;

    /* Specify the type of device, pci, spi, switch, ether ... */
    uint32 dev_type;

    int be_pio;

    union {
        /* Linux PCI device pointer */
        struct pci_dev* _pci_dev;

        /* SPI device pointer */
        struct spi_dev* _spi_dev;
    } dev;
#define pci_device  dev._pci_dev
#define spi_device  dev._spi_dev

    /* PCI I/O mapped base address */
    sal_vaddr_t base_address;

    /* Physical address */
    resource_size_t phys_address;

    /* BDE device description */
    ibde_dev_t bde_dev;

    /* Interrupt Handling */
    int     iLine; /* Interrupt line */
    void (*isr)(void *);
    void *isr_data;

    /*
     * Controls to allow two drivers to share a single set of
     * hardware registers. Typically a kernel driver will handle
     * a subset of hardware interrupts and a user mode driver
     * will handle the remaining interrupts.
     */
    void (*isr2)(void *);
    void *isr2_data;
    uint32_t fmask;  /* Interrupts controlled by secondary handler */
    uint32_t imask;  /* Enabled interrupts for primary handler */
    uint32_t imask2; /* Enabled interrupts for secondary handler */

} bde_ctrl_t;

static bde_ctrl_t _devices[LINUX_BDE_MAX_DEVICES];
static int _ndevices = 0;
static int _switch_ndevices = 0;
static int _ether_ndevices = 0;
static int  robo_switch = 0;

#define VALID_DEVICE(_n) ((_n >= 0) && (_n < _ndevices))
#define DEVICE_INDEX(_n) ((_n < _switch_ndevices) ? _n : \
                  (LINUX_BDE_MAX_SWITCH_DEVICES+_n-_switch_ndevices))

#ifdef BCM_ROBO_SUPPORT

/* for SPI access via bcm4710 core */
static void *robo = NULL;
static void *sbh = NULL;

#ifdef ALTA_ROBO_SPI

extern void *alta_eth_spi_ctrl;

extern int
robo_spi_read(void *cookie, uint16_t reg, uint8_t *buf, int len);

extern int
robo_spi_write(void *cookie, uint16_t reg, uint8_t *buf, int len);

#define ROBO_RREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_read(_dev ? NULL : alta_eth_spi_ctrl, (_page << 8) | (_reg), _buf, _len)
#define ROBO_WREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_write(_dev ? NULL : alta_eth_spi_ctrl, (_page << 8) | (_reg), _buf, _len)

#else /* !ALTA_ROBO_SPI */

#define ROBO_RREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_rreg(_robo, _dev, _page, _reg, _buf, _len)
#define ROBO_WREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_wreg(_robo, _dev, _page, _reg, _buf, _len)

#endif /* ALTA_ROBO_SPI */

#endif /* BCM_ROBO_SUPPORT */

/* Broadcom BCM4704 */
#define BCM4704_VENDOR_ID 0x14E4
#define BCM4704_DEVICE_ID 0x4704

/* SiByte PCI Host */
#define SIBYTE_PCI_VENDOR_ID 0x166D
#define SIBYTE_PCI_DEVICE_ID 0x0001

/* Intel 21150 PCI-PCI Bridge */
#define DC21150_VENDOR_ID 0x1011
#define DC21150_DEVICE_ID 0x0022

/* HiNT HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID 0x3388
#define HINT_HB4_DEVICE_ID 0x0022

/* Pericom PI7C8150 PCI-PCI Bridge (21150 clone) */
#define PI7C8150_VENDOR_ID 0x12D8
#define PI7C8150_DEVICE_ID 0x8150

/* Pericom PI7C9X130 PCI-PCIE Bridge */
#define PCI_VNDID_PERICOM     0x12D8
#define PCI_DEVID_PI7C9X130   0xE130
#define DEV_CTRL_REG           0xb8

#define MAX_PAYLOAD_256B       (1 << 5)
#define MAX_PAYLOAD_512B       (2 << 5)
#define MAX_READ_REQ_256B      (1 << 12)


/* Freescale 8548 PCI-E  host Bridge */
#define FSL_VENDOR_ID                   0x1957
#define FSL8548PCIE_DEVICE_ID           0x0013
#define FSL8548PCIE_DEV_CTRL_REG        0x54

/* 4716 PCI-E  host Bridge */
#define BCM4716_VENDOR_ID                   0x14e4
#define BCM4716PCIE_DEVICE_ID           0x4716
#define BCM4716PCIE_DEV_CAP_REG        0xd4
#define BCM4716PCIE_DEV_CTRL_REG        0xd8
#define BCM53000_VENDOR_ID                  0x14e4
#define BCM53000PCIE_DEVICE_ID              0x5300

#define BCM53000PCIE_DEV(port) ((port==0) ? pcie0:pcie1)
#define BCM53000PCIE_BASE(port) ((port==0) ? 0xb8005000 : 0xb800e000)
#define BCM53000PCIE_FUNC0_COFIG_SPACE 0x400
#define BCM53000PCIE_SROM_SPACE 0x800
#define BCM53000PCIE_DEV_CAP_REG  0xd4
#define BCM53000PCIE_DEV_CTRL_REG 0xd8
#define BCM53000PCIE_MAX_PAYLOAD_MASK  0x7
#define BCM53000PCIE_CAP_MAX_PAYLOAD_256B  (1 << 0)

/* 16bit wide register. offset 14, 14*2 = 0x1c */
#define BCM53000PCIE_SPROM_OFFSET 0x1c  
/* bit 15:13 spromData.MaxPayloadSize. 1: 256 bytes */
#define BCM53000PCIE_SPROM_MAX_PAYLOAD_MASK 0xe000
#define BCM53000PCIE_SPROM_MAX_PAYLOAD_256B (1 << 13)


/* Intel 21150, HiNT HB4 and other 21150-compatible */
#define PCI_CFG_DEC21150_SEC_CLK 0x68

#define BCM4704_ENUM_BASE     0x18000000
#define BCM4704_MEMC_BASE     (BCM4704_ENUM_BASE+0x8000)
#define BCM4704_MEMC_PRIORINV 0x18

/* PLX PCI-E Switch */
#define PLX_PEX8608_DEV_ID         0x8608
#define PLX_PEX8608_DEV_CTRL_REG   0x70


#define BDE_DEV_MEM_MAPPED(d)                               \
            ((d) & (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE |   \
                    BDE_EB_DEV_TYPE))


static uint32_t _read(int d, uint32_t addr);

#ifdef BCM_ICS
#else
/* Used to determine overall memory limits across all devices */
static uint32_t _pci_mem_start = 0xFFFFFFFF;
static uint32_t _pci_mem_end = 0;
#endif

#ifdef BCM_PLX9656_LOCAL_BUS


#define CPLD_OFFSET             0x00800000
#define CPLD_REVISION_REG       0x0000
#define CPLD_REVISION_MASK      0xffff
#define CPLD_RESET_REG          0x0004
#define CPLD_RESET_NONE         0x0000

#define PL0_OFFSET      0x00800000
#define PL0_SIZE        0x00040000
#define PL0_REVISION_REG    0x0000


/* Assume there's only one PLX PCI-to-Local bus bridge if any */
static bde_ctrl_t plx_ctrl;
static int num_plx = 0;

#endif /* BCM_PLX9656_LOCAL_BUS */
static spinlock_t bus_lock;

static int
_parse_eb_args(char *str, char * format, ...)
{
    va_list args;

    va_start(args, format);
    vsscanf(str, format, args);
    va_end(args);

    return 0;
}

static int
_eb_device_create(resource_size_t baddr, int irq, int rd_hw, int wr_hw)
{
    bde_ctrl_t* ctrl;
    uint32  dev_rev_id = 0x0, dev_id;

    ctrl = _devices + _switch_ndevices++;
    dev_id = _ndevices;
    _ndevices++;

    ctrl->dev_type |= BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;
    ctrl->pci_device = NULL; /* No PCI bus */

    if(rd_hw) {
        ctrl->dev_type |= BDE_DEV_BUS_RD_16BIT;
    }

    if (wr_hw) {
        ctrl->dev_type |= BDE_DEV_BUS_WR_16BIT;
    }

    /* Map in the device */
    ctrl->phys_address = baddr;
    ctrl->base_address = (sal_vaddr_t) IOREMAP(baddr, 0x10000);
    ctrl->bde_dev.base_address = ctrl->base_address;

    dev_rev_id = _read(dev_id, 0x178);  /* CMIC_DEV_REV_ID */

    ctrl->bde_dev.device = dev_rev_id & 0xFFFF;
    ctrl->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;

    ctrl->iLine = irq;
    ctrl->isr = NULL;
    ctrl->isr_data = NULL;

    gprintk("Created EB device at BA=%x IRQ=%d RD16=%d WR16=%d device=0x%x\n",
           baddr, irq, rd_hw, wr_hw, ctrl->bde_dev.device);

    return 0;
}


#ifdef BCM_ICS
static int
_ics_bde_create(void)
{
    bde_ctrl_t* ctrl;
    uint32  dev_rev_id = 0x0;

    if (_ndevices == 0) {
        ctrl = _devices + _switch_ndevices++;
        _ndevices++;

        ctrl->dev_type |= BDE_ICS_DEV_TYPE | BDE_SWITCH_DEV_TYPE;
        ctrl->pci_device = NULL; /* No PCI bus */

        /* Map in the device */
        ctrl->phys_address = BCM_ICS_CMIC_BASE;
        ctrl->base_address = (sal_vaddr_t)
            IOREMAP(BCM_ICS_CMIC_BASE, 0x10000);
        ctrl->bde_dev.base_address = ctrl->base_address;

        dev_rev_id =
            *((unsigned int*)(KSEG1ADDR(BCM_ICS_CMIC_BASE + 0x178)));

        ctrl->bde_dev.device = dev_rev_id & 0xFFFF;
        ctrl->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;

        ctrl->iLine = 5; /* From raptor linux BSP */

        ctrl->isr = NULL;
        ctrl->isr_data = NULL;
        printk("Creaed ICS device ..%x\n", ctrl->base_address);
    }

    return 0;
}
#else

#ifdef BCM_MODENA_SUPPORT
#include <soc/modena/devids.h>
#else
#include <soc/devids.h>
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
#define LINUX_PCI_FIND_DEVICE(d,v,fr)       \
                pci_find_device((d), (v), (fr))
#else
#define LINUX_PCI_FIND_DEVICE(d,v,fr)       \
                pci_get_device((d), (v), (fr))
#endif

/*
 * Function: fixup_p2p_bridge
 *
 * Purpose:
 *    Finalize initialization secondary PCI-PCI bridge.
 * Parameters:
 *    membase - start of memory address space for secondary PCI bus
 * Returns:
 *    0
 * Notes:
 *    The membase depends on the processor architecture, and is
 *    derived from the memory space addresses set up by the kernel.
 */
static int
fixup_p2p_bridge(void)
{
    struct pci_dev *dev;
    uint16 cmd;
    uint16 mem_base;
    uint16 mem_limit;
    uint8 bridge_ctrl;

    if ((dev = LINUX_PCI_FIND_DEVICE(DC21150_VENDOR_ID, DC21150_DEVICE_ID, NULL)) != NULL ||
        (dev = LINUX_PCI_FIND_DEVICE(HINT_HB4_VENDOR_ID, HINT_HB4_DEVICE_ID, NULL)) != NULL ||
        (dev = LINUX_PCI_FIND_DEVICE(PI7C8150_VENDOR_ID, PI7C8150_DEVICE_ID, NULL)) != NULL) {

        if (debug >= 1) gprintk("fixing up PCI-to-PCI bridge\n");
        /* Fixup command register */
        pci_read_config_word(dev, PCI_COMMAND, &cmd);
        cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
        /* Disable device */
        pci_write_config_word(dev, PCI_COMMAND, 0);
        /* Initialize non-prefetchable memory window if needed */
        pci_read_config_word(dev, PCI_MEMORY_BASE, &mem_base);
        if (mem_base == 0) {
            mem_base = (uint16)((_pci_mem_start & 0xFFF00000) >> 16);
            mem_limit = (uint16)((_pci_mem_end & 0xFFF00000) >> 16);
            pci_write_config_word(dev, PCI_MEMORY_BASE, mem_base);
            pci_write_config_word(dev, PCI_MEMORY_LIMIT, mem_limit);
        }
        /* Enable PCI clocks on remote end */
        pci_write_config_word(dev, PCI_CFG_DEC21150_SEC_CLK, 0);
        /* Re-enable config space */
        pci_write_config_word(dev, PCI_COMMAND, cmd);

        /* Avoid DMA data corruption */
        if (dev->vendor == HINT_HB4_VENDOR_ID) {
            /* Fix for HiNT bridge and BCM4704 DMA problem */
            if ((dev = LINUX_PCI_FIND_DEVICE(BCM4704_VENDOR_ID, BCM4704_DEVICE_ID, NULL)) != NULL) {
                                /* Reset PrefetchEn (PE) */
                pci_write_config_dword(dev, 0x8c, 1);
                if (debug >= 1) gprintk("reset PrefetchEn on BCM4704 when HiNT bridge is present\n");
            }
        }
    }
    /* Enable fast back-to-back read/write */
    if ((dev = LINUX_PCI_FIND_DEVICE(PI7C8150_VENDOR_ID, PI7C8150_DEVICE_ID, NULL)) != NULL) {
        pci_read_config_word(dev, PCI_COMMAND, &cmd);
        cmd |= PCI_COMMAND_FAST_BACK;
        pci_read_config_byte(dev, PCI_BRIDGE_CONTROL, &bridge_ctrl);
        bridge_ctrl |= PCI_BRIDGE_CTL_FAST_BACK;
        pci_write_config_word(dev, PCI_COMMAND, 0);
        pci_write_config_byte(dev, PCI_BRIDGE_CONTROL, bridge_ctrl);
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    }

    if ((dev = LINUX_PCI_FIND_DEVICE(PCI_VNDID_PERICOM, PCI_DEVID_PI7C9X130,
        NULL)) != NULL) {
        /* configure the PCIE cap: Max payload size: 256, Max Read
         * Request size: 256, disabling relax ordering.
         * Writes to the PCIE capability device control register
         */

        pci_write_config_dword(dev,DEV_CTRL_REG,MAX_PAYLOAD_256B |
                                                MAX_READ_REQ_256B);
    }

    if ((dev = LINUX_PCI_FIND_DEVICE(FSL_VENDOR_ID, FSL8548PCIE_DEVICE_ID,
        NULL)) != NULL) {
        /* configure the PCIE cap: Max payload size: 256, Max Read
         * Request size: 256, disabling relax ordering.
         * Writes to the PCIE capability device control register
         */

        pci_write_config_dword(dev,FSL8548PCIE_DEV_CTRL_REG,
                                MAX_PAYLOAD_256B | MAX_READ_REQ_256B);
    }
    if ((dev = LINUX_PCI_FIND_DEVICE(BCM4716_VENDOR_ID, 
            BCM4716PCIE_DEVICE_ID, NULL)) != NULL ||
        (dev = LINUX_PCI_FIND_DEVICE(BCM53000_VENDOR_ID, 
            BCM53000PCIE_DEVICE_ID, NULL)) != NULL) {
        
        uint32 tmp, fixup_bmp=0, mask;
        unsigned long addr;
        uint16 tmp16, tmp161;
        int i, bus0 = -1, bus1 = -1, port;        
        struct pci_dev *pcie0, *pcie1;
        
        pcie0 = dev;
        bus0 = dev->bus->number;
        if ((pcie1 = LINUX_PCI_FIND_DEVICE(BCM53000_VENDOR_ID, 
                BCM53000PCIE_DEVICE_ID, pcie0)) != NULL) {
            bus1 = pcie1->bus->number;
        }

        for(i = 0; i < _ndevices; i++) {
            int _i;
            bde_ctrl_t* ctrl;

            _i = DEVICE_INDEX(i);
            ctrl = _devices + _i;
            if (ctrl->dev_type & (BDE_SWITCH_DEV_TYPE|BDE_PCI_DEV_TYPE)){
                if (ctrl->pci_device->bus->number == bus0) {
                    fixup_bmp |= 1 << 0;
                }
                if (ctrl->pci_device->bus->number == bus1) {
                    fixup_bmp |= 1 << 1;
                }
            }
        }        

        /* configure the PCIE cap: Max payload size: 256, Max Read
         * Request size: 256, disabling relax ordering.
         * Writes to the PCIE capability device control register
         */

        i = 0;
        while(fixup_bmp) {
            if (fixup_bmp & (1 << i)){
                port = i ;                                  
                pci_read_config_dword(BCM53000PCIE_DEV(port), 
                    BCM53000PCIE_DEV_CAP_REG, &tmp);
                if (debug >= 1) {
                    gprintk("port %d\n",port);
                    gprintk("DevCap (@%x): 0x%x%c\n", BCM53000PCIE_DEV_CAP_REG, 
                        tmp, ((tmp & BCM53000PCIE_MAX_PAYLOAD_MASK) != 
                        BCM53000PCIE_CAP_MAX_PAYLOAD_256B) ? ' ':'\n');
                }
                if ((tmp & BCM53000PCIE_MAX_PAYLOAD_MASK) != 
                        BCM53000PCIE_CAP_MAX_PAYLOAD_256B) {
                    addr = BCM53000PCIE_BASE(port) | BCM53000PCIE_SROM_SPACE |
                        BCM53000PCIE_SPROM_OFFSET;
                    tmp16 = *((uint16 *)addr);                                                       
                    if (debug >= 1){
                        gprintk("addr %x spromData.MaxPayloadSize: 0x%x\n",addr,tmp16);                                    
                    }
                    mask = BCM53000PCIE_SPROM_MAX_PAYLOAD_MASK;
                    if ((tmp16 & mask) != 
                            BCM53000PCIE_SPROM_MAX_PAYLOAD_256B) {
                        tmp161 = (tmp16 & ~mask) | 
                            BCM53000PCIE_SPROM_MAX_PAYLOAD_256B;
                        *((uint16 *)addr) = tmp161;                                                  
                        if (debug >= 1) {
                            tmp16 = 0;                                                                         
                            tmp16 = *((uint16 *)addr);                                                   
                            gprintk("Enable spromData.MaxPayloadSize to 1 (256 bytes): 0x%x (%s w/ 0x%x)\n",
                                tmp161, ((tmp16 & mask) == 
                                BCM53000PCIE_SPROM_MAX_PAYLOAD_256B) ? 
                                "Success":"Fail", tmp16);
                        }
                    }                                                                                      
                    pci_read_config_dword(BCM53000PCIE_DEV(port), 
                        BCM53000PCIE_DEV_CAP_REG, &tmp);                            
                    if (debug >= 1){
                        gprintk("DevCap (@%x): now is 0x%x\n\n", 
                            BCM53000PCIE_DEV_CAP_REG, tmp);        
                    }
                }                                                                                          

                addr =  BCM53000PCIE_BASE(port) | BCM53000PCIE_FUNC0_COFIG_SPACE |
                    BCM53000PCIE_DEV_CTRL_REG;
                tmp16 = *((uint16 *)addr);                                                           
                if (debug >= 1) gprintk("DevControl (@%x): 0x%x\n", BCM53000PCIE_DEV_CTRL_REG, tmp16);                      
                if (!(tmp16 & MAX_PAYLOAD_256B) || !(tmp16 & MAX_READ_REQ_256B)) {                         
                    tmp161 = tmp16 | MAX_PAYLOAD_256B | MAX_READ_REQ_256B;                                 
                    *((uint16 *)addr) = tmp161;                                                                                                          
                    if (debug >= 1) {
                        tmp16 = 0;                                                                             
                        tmp16 = *((uint16 *)addr);   
                        gprintk("addr %x Enable DevControl MaxPayloadSize to 1 (256 bytes): 0x%x (%s w/ 0x%x)\n",      
                            addr,tmp161, (tmp16 & MAX_PAYLOAD_256B) ? "Success":"Fail", tmp16);                     
                        gprintk("Enable DevControl MaxReadRequestSize to 1 (256 bytes): 0x%x (%s w/ 0x%x)\n\n",
                            tmp161, (tmp16 & MAX_READ_REQ_256B) ? "Success":"Fail", tmp16);                    
                    }                    
                }             
                fixup_bmp &= ~(1 << i);
            }
            i++;
        }
    }

    /* 
     * Configure max payload to 512 on all ports in the PLX8608.
     * The device supports 128, 512, and 1024 max payload sizes. 
     */
    dev = NULL;
    while ((dev = LINUX_PCI_FIND_DEVICE(PCI_VENDOR_ID_PLX,
                                        PLX_PEX8608_DEV_ID, 
                                        dev)) != NULL) {
        uint16 ctrl_reg;
        pci_read_config_word(dev, PLX_PEX8608_DEV_CTRL_REG, &ctrl_reg);
        ctrl_reg = (ctrl_reg & ~(7<<5)) | MAX_PAYLOAD_512B;
        pci_write_config_word(dev, PLX_PEX8608_DEV_CTRL_REG, ctrl_reg);
    }
    return 0;
}

#ifdef BCM_PLX9656_LOCAL_BUS

#define PLX_LAS0_BA         0x00000004  /* LAS0 Local Base Address Remap   */
#define PLX_LAS1_BA         0x000000f4  /* LAS1 Local Base Address Remap   */
#define PLX_LAS_EN          0x00000001  /* Space Enable bit                */

#define PLX_MMAP_PCIBAR0    0           /* Memory-Mapped Config (PCIBAR0)  */
#define PLX_LAS0_PCIBAR2    2           /* Local Address Space 0 (PCIBAR2) */
#define PLX_LAS1_PCIBAR3    3           /* Local Address Space 1 (PCIBAR3) */

STATIC int 
_plx_las_bar_get(struct pci_dev *dev)
{
    void           *local_config_addr;
    int             bar = -1;

    local_config_addr = IOREMAP(pci_resource_start(dev, PLX_MMAP_PCIBAR0),
                                pci_resource_len(dev, PLX_MMAP_PCIBAR0));
    if (local_config_addr) {
        uint32          las_remap_reg;        
        /* 
         * Make sure LAS0BA or LAS1BA is enabled before returning
         * BAR that will be used to access the Local Bus
         */
        las_remap_reg = ioread32(local_config_addr + PLX_LAS0_BA);
        if (las_remap_reg & PLX_LAS_EN) {
            bar = PLX_LAS0_PCIBAR2;
        } else {
            las_remap_reg = ioread32(local_config_addr + PLX_LAS1_BA);
            if (las_remap_reg & PLX_LAS_EN) {
                bar = PLX_LAS1_PCIBAR3;
            }
        } 
    }
    iounmap(local_config_addr);
    return bar;
}
#endif /* BCM_PLX9656_LOCAL_BUS */


/*
 * Function: _pci_probe
 *
 * Purpose:
 *    Device initialization callback used by the Linux PCI
 *    subsystem. Called as a result of pci_register_driver().
 * Parameters:
 *    dev - Linux PCI device structure
 * Returns:
 *    0
 */
static int
_pci_probe(struct pci_dev *dev, const struct pci_device_id *ent)
{
    bde_ctrl_t* ctrl;
    uint16  cmd = 0;
    int baroff = 0;
    uint16 cap_base = 0;
    uint16 rval = 0;
    uint16 size_code = 0;

    if (nodevices == 1) {
        return 0; 
    }

#ifdef BCM_ROBO_SUPPORT
#ifdef KEYSTONE
	if ((dev->device == BCM47XX_ENET_ID) || (dev->device == BCM53000_GMAC_ID)) {
#else /* !KEYSTONE */
    if (dev->device == BCM47XX_ENET_ID){
#endif /* KEYSTONE */    	
        ctrl = _devices + LINUX_BDE_MAX_SWITCH_DEVICES +
          _ether_ndevices++;
        ctrl->pci_device = dev;
        ctrl->iLine = ctrl->pci_device->irq;
        ctrl->dev_type |= BDE_ETHER_DEV_TYPE;
        if (debug >= 1)
            gprintk("found pci device %04x:%04x as ether_dev\n",
                    dev->vendor, dev->device);
    } else
#endif
    {
#ifdef BCM_PLX9656_LOCAL_BUS
    /* PLX chip itself won't be part of _devices[]. */
    if ((dev->vendor == PCI_VENDOR_ID_PLX) &&
       ((dev->device == PCI_DEVICE_ID_PLX_9656) ||
        (dev->device == PCI_DEVICE_ID_PLX_9056))) {
        baroff = _plx_las_bar_get(dev);
        if (baroff == -1) {
            gprintk("No Local Address Space enabled in PLX\n");
            return 0;
        }
        ctrl = &plx_ctrl;
        num_plx++;
    } else {
        ctrl = _devices + _switch_ndevices++;
    }
#else
        ctrl = _devices + _switch_ndevices++;
#endif
        ctrl->pci_device = dev;

        /* The device ID for 56500, 56100, 56300 and 56600 family of devices may be
         * incorrect just after a PCI HW reset. It needs to be read again after
         * stabilization.
         */
        if (dev->vendor == BROADCOM_VENDOR_ID && (dev->device == BCM56102_DEVICE_ID ||
            dev->device == BCM56504_DEVICE_ID || dev->device == BCM56304_DEVICE_ID ||
            dev->device == BCM56314_DEVICE_ID || dev->device == BCM56112_DEVICE_ID ||
            dev->device == BCM56601_DEVICE_ID || dev->device == BCM56602_DEVICE_ID)) {
            /* Get the device ID */
            pci_read_config_word(ctrl->pci_device,
                                 PCI_DEVICE_ID,
                                 &ctrl->bde_dev.device);
            dev->device = ctrl->bde_dev.device;
        }

        if (pci_enable_device(ctrl->pci_device)) {
            gprintk("Cannot enable pci device : vendor_id = %x, device_id = %x\n",dev->vendor, dev->device);
        }

        
        /*
         * These are workarounds to get around some existing
         * kernel problems :(
         */

        /*
         * While probing we determine the overall limits for the PCI
         * memory windows across all devices. These limits are used
         * later on by the PCI-PCI bridge fixup code.
         */
        if (pci_resource_start(ctrl->pci_device, baroff) < _pci_mem_start) {
            _pci_mem_start = pci_resource_start(ctrl->pci_device, baroff);
        }
        if (pci_resource_end(ctrl->pci_device, baroff) > _pci_mem_end) {
            _pci_mem_end = pci_resource_end(ctrl->pci_device, baroff);
        }

#ifdef CONFIG_SANDPOINT
        /*
         * Something wrong with the PCI subsystem in the mousse kernel.
         * The device is programmed correctly, but the irq in the pci
         * structure is hosed. This checks for the hosed-ness and fixes it.
         */
        if (ctrl->pci_device->irq > 100) {
            ctrl->pci_device->irq = 2;
            gprintk("irq problem: setting irq = %d\n", ctrl->pci_device->irq);
        }
#endif

#ifdef CONFIG_BMW
        /*
         * PCI subsystem does not always program the system correctly.
         */
        if (ctrl->pci_device->irq < 16 && ctrl->pci_device->irq != 2) {
            ctrl->pci_device->irq = 2;
            gprintk("irq problem: setting irq = %d\n", ctrl->pci_device->irq);
        }
#endif

#ifdef CONFIG_IDT_79EB334
        /*
         * IDT kernel is not currently mapping interrupts correctly
         * Hardwired to core mips interrupt, irq 3 for kernel 2.4.18
         */
        if (ctrl->pci_device->irq != 3) {
            ctrl->pci_device->irq = 3;
            gprintk("irq problem: setting irq = %d\n", ctrl->pci_device->irq);
        }
#endif

#ifdef CONFIG_BCM94702_CPCI
        /*
         * Hardwired to core mips interrupt irq 6 on MBZ
         */
        if (ctrl->pci_device->irq != 6) {
            ctrl->pci_device->irq = 6;
            gprintk("irq problem: setting irq = %d\n", ctrl->pci_device->irq);
        }
#endif

        if ((LINUX_PCI_FIND_DEVICE(BCM4704_VENDOR_ID, BCM4704_DEVICE_ID, NULL)) != NULL) {
            /*
             * Decrease the PCI bus priority for the CPU for better overall
             * system performance. This change significantly reduces the
             * number of PCI retries from other devices on the PCI bus.
             */
            void * _mc_vbase = IOREMAP(BCM4704_MEMC_BASE, 0x1000);
            int priorinv = 0x80;
            static int done = 0;
            if (!done) {
                done = 1;
                writel(priorinv, _mc_vbase + BCM4704_MEMC_PRIORINV);
                if (debug >= 1)
                    gprintk("set BCM4704 PriorInvTim register to 0x%x\n", priorinv);
                iounmap(_mc_vbase);
            }
        }

        if ((LINUX_PCI_FIND_DEVICE(SIBYTE_PCI_VENDOR_ID, SIBYTE_PCI_DEVICE_ID, NULL)) != NULL) {
            /*
             * The BCM91125CPCI CPU boards with a PCI-PCI bridge use the same
             * interrupt line for all switch ships behind the bridge.
             */
            if (LINUX_PCI_FIND_DEVICE(DC21150_VENDOR_ID, DC21150_DEVICE_ID, NULL) ||
                LINUX_PCI_FIND_DEVICE(HINT_HB4_VENDOR_ID, HINT_HB4_DEVICE_ID, NULL) ||
                LINUX_PCI_FIND_DEVICE(PI7C8150_VENDOR_ID, PI7C8150_DEVICE_ID, NULL)) {
                                /*
                                 * By default we try to guess the correct IRQ based on the design.
                                 * For now we only look at the bridge vendor, but it may be necessary
                                 * to look at the switch chip configuration as well.
                                 */
                if (forceirq == -1) {
                    if ((LINUX_PCI_FIND_DEVICE(HINT_HB4_VENDOR_ID, HINT_HB4_DEVICE_ID, NULL)) ||
                        ((dev->device == BCM5674_DEVICE_ID) &&
                         (LINUX_PCI_FIND_DEVICE(PI7C8150_VENDOR_ID, PI7C8150_DEVICE_ID, NULL)))) {
                        forceirq = 58;
                    } else {
                        forceirq = 56;
                    }
                }
            }
        }

        if ((forceirq > 0) && (forceirqubm & (1U<<(_switch_ndevices-1))) && ctrl->pci_device->irq != (uint32) forceirq) {
            ctrl->pci_device->irq = forceirq;
            if (debug >= 1) gprintk("force irq to %d\n", forceirq);
        }

        ctrl->iLine = ctrl->pci_device->irq;

        pci_read_config_word(ctrl->pci_device,
                             PCI_CAPABILITY_LIST,
                             &cap_base);

        if (cap_base) {
            while (cap_base) {
                pci_read_config_word(ctrl->pci_device,
                                     cap_base,
                                     &rval);

                /*
                 * PCIE spec 1.1 section 7.8.1
                 * PCI-Express capability ID = 0x10
                 * Locate PCI-E capability structure
                 */
                if ((rval & 0xff) != 0x10) {
                    cap_base = (rval >> 8) & 0xff;
                    continue;
                }
                /*
                 * PCIE spec 1.1 section 7.8.1
                 * offset 0x04 : Device capabilities register
                 * bit 2-0 : Max_payload_size supported
                 * 000 - 128 Bytes max payload size
                 * 001 - 256 Bytes max payload size
                 * 010 - 512 Bytes max payload size
                 * 011 - 1024 Bytes max payload size
                 * 100 - 2048 Bytes max payload size
                 * 101 - 4096 Bytes max payload size
                 */
                pci_read_config_word(ctrl->pci_device,
                                     cap_base + 0x04,
                                     &size_code);

                size_code &= 0x07;
                if (size_code > 1) {
                    /*
                     * Restrict Max_payload_size to 256
                     */
                    size_code = 1;
                }
                /*
                 * PCIE spec 1.1 section 7.8.4
                 * offset 0x08 : Device control register
                 * bit 4-4 : Enable relaxed ordering (Disable)
                 * bit 5-7 : Max_payload_size        (256)
                 * bit 12-14 : Max_Read_Request_Size (256)
                 */

                rval = 0;
                pci_read_config_word(ctrl->pci_device,
                                     cap_base + 8,
                                     &rval);

                /* Max_Payload_Size = 1 (256 bytes) */
                rval &= ~0x00e0;
                rval |= size_code << 5;

                /* Max_Read_Request_Size = 1 (256 bytes) */
                rval &= ~0x7000;
                rval |= size_code << 12;

                rval &= ~0x0010; /* Disable Relaxed Ordering */
                pci_write_config_word(ctrl->pci_device, cap_base + 0x08, rval);
                break;
            }
        } else {
            /* Set PCI retry to infinite on switch device */
            pci_write_config_word(ctrl->pci_device, 0x40, 0x0080);
            if (debug >= 1) gprintk("set DMA retry to infinite on switch device\n");
        }
    }

    _ndevices++;

    ctrl->be_pio = 0;
    ctrl->dev_type |= BDE_PCI_DEV_TYPE;
    ctrl->pci_device = dev;
    pci_set_drvdata(dev, ctrl);

    /* Get the device revision */
    pci_read_config_byte(ctrl->pci_device,
                         PCI_REVISION_ID,
                         &ctrl->bde_dev.rev);

    /* Map in the device */
    ctrl->bde_dev.device = ctrl->pci_device->device;
    ctrl->phys_address = pci_resource_start(ctrl->pci_device, baroff);

    ctrl->base_address = (sal_vaddr_t)
      IOREMAP(pci_resource_start(ctrl->pci_device, baroff),
              pci_resource_len(ctrl->pci_device, baroff));
    
#ifdef BCM_PLX9656_LOCAL_BUS
#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    if (num_plx) {
        sal_vaddr_t base_address;
        uint32 intr_enable;
        base_address = (sal_vaddr_t)
              IOREMAP(pci_resource_start(ctrl->pci_device, 0),
                  pci_resource_len(ctrl->pci_device, 0));
        intr_enable = readl((uint32 *)(base_address + 0x68));
        gprintk("PLX Interrupt ENABLE: %x\n", intr_enable);
        intr_enable |= 0x00080000;
        writel(intr_enable, (uint32 *)(base_address + 0x68));
        gprintk("PLX Interrupt ENABLE: %x\n", intr_enable);
    }
#endif
#endif

#ifdef BCM_ROBO_SUPPORT
#ifdef KEYSTONE 
    if (dev->device == BCM53000_GMAC_ID) {
        /*
         * Since the GMAC driver of Robo chips need access the ChipCommon, 
         * and Wrapper registers, we will set the base address as
         * enumeration base address.
         * And its size as 3MB to cover all wrapper register region. 
         */
        ctrl->phys_address = SB_ENUM_BASE;
        ctrl->base_address = (sal_vaddr_t)
            IOREMAP(ctrl->phys_address,
              0x300000);
    }
#endif /* KEYSTONE */
#endif /* BCM_ROBO_SUPPORT */

    /* Use direct PCI access */
    ctrl->bde_dev.base_address = ctrl->base_address;

    /* workaround bug in FE2K A1 part; shows as A0 part in PCI config space,
     * read the FE's regs directly to get the true revision
     */
    if (ctrl->bde_dev.device == BCM88020_DEVICE_ID && ctrl->bde_dev.rev == 0) {
#define FE2000_REVISION_OFFSET      (0x0)
        uint32_t fe_rev = *((uint32_t*)(ctrl->base_address + FE2000_REVISION_OFFSET));
        if( (fe_rev >> 16) ==  BCM88020_DEVICE_ID) {
            fe_rev &= 0xFF;
        } else {
            fe_rev = (fe_rev >> 24) & 0xFF;
        }

        ctrl->bde_dev.rev = fe_rev;
    }

    ctrl->isr = NULL;
    ctrl->isr_data = NULL;

    pci_read_config_word(ctrl->pci_device, PCI_COMMAND, &cmd);
    if (!(cmd & PCI_COMMAND_MEMORY) || !(cmd & PCI_COMMAND_MASTER)) {
        cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
        pci_write_config_word(ctrl->pci_device, PCI_COMMAND, cmd);
        if (debug >= 1) gprintk("enable PCI resources 0x%x (PCI_COMMAND)\n", cmd);
    }

#ifndef ALTA_ROBO_SPI
#ifdef BCM_ROBO_SUPPORT
    /* MDC/MDIO path for pseudo PHY access to ROBO register on BCM5836/4704 */
    if (dev->device == BCM47XX_ENET_ID) {
#ifdef MDC_MDIO_SUPPORT
        ((uint32 *)ctrl->bde_dev.base_address)[0x410 / 4] = 0x14;
#else
        ((uint32 *)ctrl->bde_dev.base_address)[0x410 / 4] = 0;
#endif
    }
#endif /* BCM_ROBO_SUPPORT */
#endif

    /* Let's boogie */
    return 0;
}

/*
 * Function: _pci_remove
 *
 * Purpose:
 *    Detach driver from device. Called from pci_unregister_driver().
 * Parameters:
 *    dev - Linux PCI device structure
 * Returns:
 *    0
 */
static void
_pci_remove(struct pci_dev* dev)
{
    bde_ctrl_t* ctrl;

    if (nodevices == 1) {
        return; 
    }

    ctrl = (bde_ctrl_t *) pci_get_drvdata(dev);

    if (ctrl->base_address) {
        iounmap((void *)ctrl->base_address);
    }

    /* Free our interrupt handler, if we have one */
    if (ctrl->isr != NULL) {
        free_irq(dev->irq, ctrl);
#if defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT)
        pci_disable_msi(ctrl->pci_device);
#endif /* defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT) */
        ctrl->isr = NULL;
        ctrl->isr_data = NULL;
    }
}


/*
 * PCI device table.
 * Populated from the include/soc/devids.h file.
 */

static struct pci_device_id _id_table[] = {
#ifdef BCM_MODENA_SUPPORT
    { BROADCOM_VENDOR_ID, BCM5345_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
#else
    { BROADCOM_VENDOR_ID, BCM5690_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5691_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5692_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5693_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5695_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5696_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5697_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5698_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5670_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5671_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5675_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5676_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5673_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5674_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5665_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5655_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM5650_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56218X_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56218_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56219_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56218R_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56219R_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56214_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56215_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56214R_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56215R_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56216_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56217_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56212_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56213_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53718_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53714_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53716_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56018_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56014_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56224_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56225_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56226_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56227_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56228_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56229_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56024_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56025_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53724_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53726_DEVICE_ID,  PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56100_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56101_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56102_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56105_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56106_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56107_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56110_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56111_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56112_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56115_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56116_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56117_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56300_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56301_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56302_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56303_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56304_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56404_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56305_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56306_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56307_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56308_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56309_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56310_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56311_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56312_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56313_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56314_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56315_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56316_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56317_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56318_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56319_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
#ifndef EXCLUDE_BCM56324
    { BROADCOM_VENDOR_ID, BCM56322_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56324_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
#endif /* EXCLUDE_BCM56324 */
    { BROADCOM_VENDOR_ID, BCM53312_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53313_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53314_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53324_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53300_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53301_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM53302_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56500_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56501_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56502_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56503_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56504_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56505_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56506_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56507_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56508_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56509_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56510_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56511_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56512_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56513_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56514_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56516_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56517_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56518_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56519_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56580_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56600_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56601_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56602_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56603_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56605_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56606_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56607_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56608_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56620_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56624_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56626_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56628_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56629_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56680_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56684_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56700_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56701_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56720_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56721_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56725_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56800_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56801_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56802_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56803_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56820_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56821_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56822_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56823_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56825_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56630_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56634_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56636_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56638_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56639_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56538_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56520_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56521_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56522_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56524_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56526_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56534_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56685_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56689_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56331_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56333_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56334_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56338_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56320_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56321_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56132_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56134_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88732_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56142_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56143_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56144_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56146_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56147_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56613_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56930_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56931_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56935_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56936_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56939_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56840_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56841_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56842_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56843_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56844_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56845_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56846_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56549_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56743_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56744_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56745_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM56746_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88230_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88231_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88235_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88236_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { BROADCOM_VENDOR_ID, BCM88239_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
#ifdef BCM_ROBO_SUPPORT
    { BROADCOM_VENDOR_ID, BCM47XX_ENET_ID, PCI_ANY_ID, PCI_ANY_ID },
#ifdef KEYSTONE 
    { BROADCOM_VENDOR_ID, BCM53000_GMAC_ID, PCI_ANY_ID, PCI_ANY_ID },
#endif
#endif
    { SANDBURST_VENDOR_ID, QE2000_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { SANDBURST_VENDOR_ID, BCM88020_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { SANDBURST_VENDOR_ID, BCM88025_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID },
    { PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9656, PCI_ANY_ID, PCI_ANY_ID },
    { PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9056, PCI_ANY_ID, PCI_ANY_ID },
#endif
    { 0, 0, 0, 0 }
};

static struct pci_driver _device_driver = {
    probe: _pci_probe,
    remove: _pci_remove,
    id_table: _id_table,
    /* The rest are dynamic */
};

#endif /* BCM_ICS */

#ifdef BCM_ROBO_SUPPORT
static struct bde_spi_device_id _spi_id_table[] = {
    { BROADCOM_PHYID_HIGH, BCM5338_PHYID_LOW },
    { BCM5324_PHYID_HIGH, BCM5324_PHYID_LOW },
    { BCM5324_A1_PHYID_HIGH, BCM5324_PHYID_LOW },
    { BROADCOM_PHYID_HIGH, BCM5380_PHYID_LOW },
    { BROADCOM_PHYID_HIGH, BCM5388_PHYID_LOW },
    { BCM5396_PHYID_HIGH, BCM5396_PHYID_LOW },
    { BCM5389_PHYID_HIGH, BCM5389_PHYID_LOW },
    { BCM5398_PHYID_HIGH, BCM5398_PHYID_LOW },
    { BCM5325_PHYID_HIGH, BCM5325_PHYID_LOW },
    { BCM5348_PHYID_HIGH, BCM5348_PHYID_LOW },
    { BCM5395_PHYID_HIGH, BCM5395_PHYID_LOW },
    { BCM53242_PHYID_HIGH, BCM53242_PHYID_LOW },
    { BCM53262_PHYID_HIGH, BCM53262_PHYID_LOW },
    { BCM53115_PHYID_HIGH, BCM53115_PHYID_LOW },
    { BCM53118_PHYID_HIGH, BCM53118_PHYID_LOW },
    { BCM53280_PHYID_HIGH, BCM53280_PHYID_LOW },
    { BCM53101_PHYID_HIGH, BCM53101_PHYID_LOW },
    { BCM53125_PHYID_HIGH, BCM53125_PHYID_LOW },
    { BCM53128_PHYID_HIGH, BCM53128_PHYID_LOW },
    { 0, 0 },
};
#endif
/* DMA memory allocation */

#define ONE_KB 1024
#define ONE_MB (1024*1024)

/* Default DMA memory size */
#define DMA_MEM_DEFAULT (8 * ONE_MB)
#define DMA_MEM_DEFAULT_ROBO (4 * ONE_MB)


/* We try to assemble a contiguous segment from chunks of this size */
#define DMA_BLOCK_SIZE (512 * ONE_KB)

typedef struct _dma_segment {
    struct list_head list;
    unsigned long req_size;     /* Requested DMA segment size */
    unsigned long blk_size;     /* DMA block size */
    unsigned long blk_order;    /* DMA block size in alternate format */
    unsigned long seg_size;     /* Current DMA segment size */
    unsigned long seg_begin;    /* Logical address of segment */
    unsigned long seg_end;      /* Logical end address of segment */
    unsigned long *blk_ptr;     /* Array of logical DMA block addresses */
    int blk_cnt_max;            /* Maximum number of block to allocate */
    int blk_cnt;                /* Current number of blocks allocated */
} dma_segment_t;

static unsigned int _dma_mem_size = DMA_MEM_DEFAULT;
static mpool_handle_t _dma_pool = NULL;
static unsigned int* _dma_vbase = NULL;
static unsigned int _dma_pbase = 0;
static int _use_himem = 0;
static LIST_HEAD(_dma_seg);

/*
 * Function: _find_largest_segment
 *
 * Purpose:
 *    Find largest contiguous segment from a pool of DMA blocks.
 * Parameters:
 *    dseg - DMA segment descriptor
 * Returns:
 *    0 on success, < 0 on error.
 * Notes:
 *    Assembly stops if a segment of the requested segment size
 *    has been obtained.
 *
 *    Lower address bits of the DMA blocks are used as follows:
 *       0: Untagged
 *       1: Discarded block
 *       2: Part of largest contiguous segment
 *       3: Part of current contiguous segment
 */
static int
_find_largest_segment(dma_segment_t *dseg)
{
    int i, j, blks, found;
    unsigned long b, e, a;

    blks = dseg->blk_cnt;
    /* Clear all block tags */
    for (i = 0; i < blks; i++) {
        dseg->blk_ptr[i] &= ~3;
    }
    for (i = 0; i < blks && dseg->seg_size < dseg->req_size; i++) {
        /* First block must be an untagged block */
        if ((dseg->blk_ptr[i] & 3) == 0) {
            /* Initial segment size is the block size */
            b = dseg->blk_ptr[i];
            e = b + dseg->blk_size;
            dseg->blk_ptr[i] |= 3;
            /* Loop looking for adjacent blocks */
            do {
                found = 0;
                for (j = i + 1; j < blks && (e - b) < dseg->req_size; j++) {
                    a = dseg->blk_ptr[j];
                    /* Check untagged blocks only */
                    if ((a & 3) == 0) {
                        if (a == (b - dseg->blk_size)) {
                            /* Found adjacent block below current segment */
                            dseg->blk_ptr[j] |= 3;
                            b = a;
                            found = 1;
                        } else if (a == e) {
                            /* Found adjacent block above current segment */
                            dseg->blk_ptr[j] |= 3;
                            e += dseg->blk_size;
                            found = 1;
                        }
                    }
                }
            } while (found);
            if ((e - b) > dseg->seg_size) {
                                /* The current block is largest so far */
                dseg->seg_begin = b;
                dseg->seg_end = e;
                dseg->seg_size = e - b;
                                /* Re-tag current and previous largest segment */
                for (j = 0; j < blks; j++) {
                    if ((dseg->blk_ptr[j] & 3) == 3) {
                        /* Tag current segment as the largest */
                        dseg->blk_ptr[j] &= ~1;
                    } else if ((dseg->blk_ptr[j] & 3) == 2) {
                        /* Discard previous largest segment */
                        dseg->blk_ptr[j] ^= 3;
                    }
                }
            } else {
                                /* Discard all blocks in current segment */
                for (j = 0; j < blks; j++) {
                    if ((dseg->blk_ptr[j] & 3) == 3) {
                        dseg->blk_ptr[j] &= ~2;
                    }
                }
            }
        }
    }
    return 0;
}

/*
 * Function: _alloc_dma_blocks
 *
 * Purpose:
 *    Allocate DMA blocks and add them to the pool.
 * Parameters:
 *    dseg - DMA segment descriptor
 *    blks - number of DMA blocks to allocate
 * Returns:
 *    0 on success, < 0 on error.
 * Notes:
 *    DMA blocks are allocated using the page allocator.
 */
static int
_alloc_dma_blocks(dma_segment_t *dseg, int blks)
{
    int i, start;
    unsigned long addr;

    if (dseg->blk_cnt + blks > dseg->blk_cnt_max) {
        gprintk("No more DMA blocks\n");
        return -1;
    }
    start = dseg->blk_cnt;
    dseg->blk_cnt += blks;
    for (i = start; i < dseg->blk_cnt; i++) {
        /*
         * Note that we cannot use pci_alloc_consistent when we
         * want to be able to map DMA memory to user space.
         *
         * The GFP_DMA flag is omitted as this imposes the ISA
         * addressing limitations on x86 platforms. As long as
         * we have less than 1GB of memory, we can do PCI DMA
         * to all physical RAM locations.
         */
        addr = __get_free_pages(GFP_ATOMIC, dseg->blk_order);
        if (addr) {
            dseg->blk_ptr[i] = addr;
        } else {
            gprintk("DMA allocation failed\n");
            return -1;
        }
    }
    return 0;
}

/*
 * Function: _dma_segment_alloc
 *
 * Purpose:
 *    Allocate large physically contiguous DMA segment.
 * Parameters:
 *    size - requested DMA segment size
 *    blk_size - assemble segment from blocks of this size
 * Returns:
 *    DMA segment descriptor.
 * Notes:
 *    Since we cannot allocate large blocks of contiguous
 *    memory from the kernel, we simply keep allocating
 *    smaller chunks until we can assemble a contiguous
 *    block of the desired size.
 *
 *    When system allowed maximum bytes of memory has been allocated
 *    without a successful assembly of a contiguous DMA
 *    segment, the allocation function will return the
 *    largest contiguous segment found so far. It is up
 *    to the calling function to decide whether this
 *    amount is sufficient to proceed.
 */
static dma_segment_t *
_dma_segment_alloc(size_t size, size_t blk_size)
{
    dma_segment_t *dseg;
    int i, blk_ptr_size;
    unsigned long page_addr;
    struct sysinfo si;

    /* Sanity check */
    if (size == 0 || blk_size == 0) {
        return NULL;
    }
    /* Allocate an initialize DMA segment descriptor */
    if ((dseg = kmalloc(sizeof(dma_segment_t), GFP_ATOMIC)) == NULL) {
        return NULL;
    }
    memset(dseg, 0, sizeof(dma_segment_t));
    dseg->req_size = size;
    dseg->blk_size = PAGE_ALIGN(blk_size);
    while ((PAGE_SIZE << dseg->blk_order) < dseg->blk_size) {
        dseg->blk_order++;
    }

    si_meminfo(&si);
    dseg->blk_cnt_max = (si.totalram << PAGE_SHIFT) / dseg->blk_size;
    blk_ptr_size = dseg->blk_cnt_max * sizeof(unsigned long);
    /* Allocate an initialize DMA block pool */
    dseg->blk_ptr = kmalloc(blk_ptr_size, GFP_KERNEL);
    if (dseg->blk_ptr == NULL) {
        kfree(dseg);
        return NULL;
    }
    memset(dseg->blk_ptr, 0, blk_ptr_size);
    /* Allocate minimum number of blocks */
    _alloc_dma_blocks(dseg, dseg->req_size / dseg->blk_size);
    /* Allocate more blocks until we have a complete segment */
    do {
        _find_largest_segment(dseg);
        if (dseg->seg_size >= dseg->req_size) {
            break;
        }
    } while (_alloc_dma_blocks(dseg, 8) == 0);
    /* Reserve all pages in the DMA segment and free unused blocks */
    for (i = 0; i < dseg->blk_cnt; i++) {
        if ((dseg->blk_ptr[i] & 3) == 2) {
            dseg->blk_ptr[i] &= ~3;
            for (page_addr = dseg->blk_ptr[i];
                 page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                 page_addr += PAGE_SIZE) {
                MEM_MAP_RESERVE(VIRT_TO_PAGE(page_addr));
            }
        } else if (dseg->blk_ptr[i]) {
            dseg->blk_ptr[i] &= ~3;
            free_pages(dseg->blk_ptr[i], dseg->blk_order);
            dseg->blk_ptr[i] = 0;
        }
    }
    return dseg;
}

/*
 * Function: _dma_segment_free
 *
 * Purpose:
 *    Release resources used by DMA segment.
 * Parameters:
 *    dseg - DMA segment descriptor
 * Returns:
 *    Nothing.
 */
static void
_dma_segment_free(dma_segment_t *dseg)
{
    int i;
    unsigned long page_addr;

    if (dseg->blk_ptr) {
        for (i = 0; i < dseg->blk_cnt; i++) {
            if (dseg->blk_ptr[i]) {
                for (page_addr = dseg->blk_ptr[i];
                     page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                     page_addr += PAGE_SIZE) {
                    MEM_MAP_UNRESERVE(VIRT_TO_PAGE(page_addr));
                }
                free_pages(dseg->blk_ptr[i], dseg->blk_order);
            }
        }
        kfree(dseg->blk_ptr);
        kfree(dseg);
    }
}

/*
 * Function: _pgalloc
 *
 * Purpose:
 *    Allocate DMA memory using page allocator
 * Parameters:
 *    size - number of bytes to allocate
 * Returns:
 *    Pointer to allocated DMA memory or NULL if failure.
 * Notes:
 *    For any sizes less than DMA_BLOCK_SIZE, we ask the page
 *    allocator for the entire memory block, otherwise we try
 *    to assemble a contiguous segment ourselves.
 */
static void *
_pgalloc(size_t size)
{
    dma_segment_t *dseg;
    size_t blk_size;

    blk_size = (size < DMA_BLOCK_SIZE) ? size : DMA_BLOCK_SIZE;
    if ((dseg = _dma_segment_alloc(size, blk_size)) == NULL) {
        return NULL;
    }
    if (dseg->seg_size < size) {
        /* If we didn't get the full size then forget it */
        _dma_segment_free(dseg);
        return NULL;
    }
    list_add(&dseg->list, &_dma_seg);
    return (void *)dseg->seg_begin;
}

/*
 * Function: _pgfree
 *
 * Purpose:
 *    Free memory allocated by _pgalloc
 * Parameters:
 *    ptr - pointer returned by _pgalloc
 * Returns:
 *    0 if succesfully freed, otherwise -1.
 */
static int
_pgfree(void *ptr)
{
    struct list_head *pos;
    list_for_each(pos, &_dma_seg) {
        dma_segment_t *dseg = list_entry(pos, dma_segment_t, list);
        if (ptr == (void *)dseg->seg_begin) {
            list_del(&dseg->list);
            _dma_segment_free(dseg);
            return 0;
        }
    }
    return -1;
}

/*
 * Function: _pgcleanup
 *
 * Purpose:
 *    Free all memory allocated by _pgalloc
 * Parameters:
 *    None
 * Returns:
 *    Nothing.
 */
static void
_pgcleanup(void)
{
    struct list_head *pos, *tmp;
    list_for_each_safe(pos, tmp, &_dma_seg) {
        dma_segment_t *dseg = list_entry(pos, dma_segment_t, list);
        list_del(&dseg->list);
        _dma_segment_free(dseg);
    }
}

/*
 * Function: _alloc_mpool
 *
 * Purpose:
 *    Allocate DMA memory pool
 * Parameters:
 *    size - size of DMA memory pool
 * Returns:
 *    Nothing.
 * Notes:
 *    If set up to use high memory, we simply map the memory into
 *    kernel space.
 */
static void
_alloc_mpool(size_t size)
{
    if (_use_himem) {
        /* Use high memory for DMA */
        _dma_pbase = virt_to_bus(high_memory);
        _dma_vbase = IOREMAP(_dma_pbase, size);
    } else {
        /* Get DMA memory from kernel */
        _dma_vbase = _pgalloc(size);
        _dma_pbase = virt_to_bus(_dma_vbase);
#ifdef REMAP_DMA_NONCACHED
        _dma_vbase = IOREMAP(_dma_pbase, size);
#endif
    }
}
#ifdef BCM_ROBO_SUPPORT
#ifdef MDC_MDIO_SUPPORT
#define PHYRD_TIMEOUT 500
#define PSEUDO_PHY_ADDR 0x1e
static int page_number = 0xff; /* init value */

static uint32_t _read(int d, uint32_t addr);
static int _write(int d, uint32_t addr, uint32_t data);

uint16
_chipphyrd(uint8 cid, uint phyaddr, uint reg)
{
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;
    uint16 v;

    /* local phy: our emac controls our phy */

    /* clear mii_int */
    _write(cid, 0x41c, EI_MII);

    /* issue the read */
    _write(cid, 0x414,  (MD_SB_START | MD_OP_READ |
                         (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID));

    /* wait for it to complete */
    while (process) {
        if ((_read(cid, 0x41c) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;

        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }

    if ((_read(cid, 0x41c) & EI_MII) == 0) {
        gprintk("_chipphyrd: did not complete\n");
    }

    v = _read(cid, 0x414) & MD_DATA_MASK;
    return v;
}

void
_chipphywr(uint8 cid, uint phyaddr, uint reg, uint16 v)
{
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;

    /* local phy: our emac controls our phy */

    /* clear mii_int */
    _write(cid, 0x41c, EI_MII);

    /* issue the write */
    _write(cid, 0x414,  (MD_SB_START | MD_OP_WRITE
                         | (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID | v));

    /* wait for it to complete */
    while (process) {
        if ((_read(cid, 0x41c) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;

        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }

    if ((_read(cid, 0x41c) & EI_MII) == 0) {
        gprintk("_chipphywr: did not complete\n");
    }
}

int
_soc_reg_read(uint8 cid, uint32 addr, uint8 *buf, int len)
{
    uint8 page, offset;
    uint16 phywr_val = 0, phyrd_val = 0;
    uint16 process,phyrd_timeout_count;
    uint16 *rd_buf = NULL;

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    /* If accessing register is in another page*/
    if (page != page_number) {
        phywr_val = (page << 8) | (cid & 0x3) << 1 | 0x1;
        _chipphywr(cid, PSEUDO_PHY_ADDR, 16, phywr_val);
        page_number = page;
    }

    phywr_val = (offset << 8) | 0x2; /*OP code read.*/
    _chipphywr(cid, PSEUDO_PHY_ADDR, 17, phywr_val);

    process = 1;
    phyrd_timeout_count = 0;

    while (process) {
        phyrd_val = _chipphyrd(cid, PSEUDO_PHY_ADDR, 17);
        if (!(phyrd_val & 0x03)) {
            process = 0;
        }
        phyrd_timeout_count ++;

        if (phyrd_timeout_count > PHYRD_TIMEOUT) {
            return -ETIMEDOUT;
        }
    }

    rd_buf = (uint16 *)buf;
    *rd_buf = _chipphyrd(cid, PSEUDO_PHY_ADDR, 24);
    *(rd_buf + 1) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 25);
    *(rd_buf + 2) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 26);
    *(rd_buf + 3) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 27);
#ifdef BE_HOST
    *rd_buf = (*rd_buf >> 8)|(*rd_buf << 8);
    *(rd_buf + 1) = (*(rd_buf + 1) >> 8)|(*(rd_buf + 1) << 8);
    *(rd_buf + 2) = (*(rd_buf + 2) >> 8)|(*(rd_buf + 2) << 8);
    *(rd_buf + 3) = (*(rd_buf + 3) >> 8)|(*(rd_buf + 3) << 8);
#endif

    return 0;
}

int
_soc_reg_write(uint8 cid, uint32 addr, uint8 *buf, int len)
{
    uint8 page, offset;
    uint16 phywr_val = 0, phyrd_val = 0;
    uint16 process,phyrd_timeout_count;
    uint16 *wr_buf;

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    /* If accessing register is in another page*/
    if (page != page_number) {
        phywr_val = (page << 8) | (cid & 0x3) << 1 | 0x1;
        _chipphywr(cid, PSEUDO_PHY_ADDR, 16, phywr_val);
        page_number = page;
    }

    wr_buf = (uint16 *)buf;
#ifdef BE_HOST
    *wr_buf = (*wr_buf >> 8)|(*wr_buf << 8);
    *(wr_buf + 1) = (*(wr_buf + 1) >> 8)|(*(wr_buf + 1) << 8);
    *(wr_buf + 2) = (*(wr_buf + 2) >> 8)|(*(wr_buf + 2) << 8);
    *(wr_buf + 3) = (*(wr_buf + 3) >> 8)|(*(wr_buf + 3) << 8);
#endif
    _chipphywr(cid, PSEUDO_PHY_ADDR, 24, *wr_buf);
    _chipphywr(cid, PSEUDO_PHY_ADDR, 25, *(wr_buf + 1));
    _chipphywr(cid, PSEUDO_PHY_ADDR, 26, *(wr_buf + 2));
    _chipphywr(cid, PSEUDO_PHY_ADDR, 27, *(wr_buf + 3));

    phywr_val = (offset << 8) | 0x1; /*OP code write.*/
    _chipphywr(cid, PSEUDO_PHY_ADDR, 17, phywr_val);

    process = 1;
    phyrd_timeout_count = 0;
    while (process) {
        phyrd_val = _chipphyrd(cid, PSEUDO_PHY_ADDR, 17);
        if (!(phyrd_val & 0x03)) {
            process = 0;
        }
        phyrd_timeout_count ++;

        if (phyrd_timeout_count > PHYRD_TIMEOUT) {
            return -ETIMEDOUT;
        }
    }
    return 0;
}

int
robo_mdio_reg_read(uint8 cid, uint32 addr, void *data, int len)
{
#ifdef BE_HOST
    uint32  i;
#endif
    uint32      *data_ptr;
    uint64      data_rw = 0;
    uint8           *data8_ptr;
#ifdef BE_HOST
    uint8       tmp;
#endif

    _soc_reg_read(cid, addr, (uint8 *)&data_rw, len);

    /* endian translation */
    data8_ptr = (uint8 *)&data_rw;
#ifdef BE_HOST
    if (len > 4) {
        for (i=0; i < 4; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[7-i];
            data8_ptr[7-i] = tmp;
        }
    } else {
        for (i = 0; i < 2; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[3-i];
            data8_ptr[3-i] = tmp;
        }
    }
#endif
    
    data_ptr = (uint32 *)data;
 
    if (len > 4) {
        sal_memcpy(data, data8_ptr, 8);
    } else {
        sal_memcpy(data, data8_ptr, 4);
    }
    return 0;
}

#endif

static int
_spi_device_valid_check(unsigned short phyidh)
{
    struct bde_spi_device_id *_ids;

    for (_ids = _spi_id_table;
         _ids->phyid_high && _ids->phyid_low; _ids++) {
            if (_ids->phyid_high == phyidh) {
                return 0;
            }
    }

    /* No valid spi devices found */
    return 1;
}

static int
probe_robo_switch(void){
    /*
     * No Strata device, which is of PCI device.
     * Try to probe robo devices, which if of SPI device.
     */

    int dev;
    int eb_bus=0;
#ifndef MDC_MDIO_SUPPORT
#ifdef KEYSTONE
    uint32 spi_freq = SPI_FREQ_20MHZ; /* The hightest supported SPI frequency, as of now 20MHz */
#endif
#endif

    spin_lock_init(&bus_lock);

    if(_switch_ndevices) {
        /*
         * currently skip probe robo if esw chips were found
         * FIX this while combined plateform support.
         * Robo will access CS0 in BCM5836_ROBO for EB_BUS
         * but CS0 in BCM5836_CPCI is used for reset register.
              */
        return robo_switch;
    }

#ifndef MDC_MDIO_SUPPORT
#ifdef KEYSTONE
	if (!(sbh = (void *)ai_soc_kattach(NULL))) {
#else /* !KEYSTONE */
    if (!(sbh = (void *)sb_soc_kattach())) {
#endif /* KEYSTONE */    	
        return -ENODEV;
    }
    /* Use GPIO 2-5 to operate on SPI interface */
#ifdef KEYSTONE
	    if (!(robo = (void *)robo_attach(sbh,
                                     (1<<1), (1<<0), (1<<2), (1<<3), 0))) {
#else /* !KEYSTONE */    
    if (!(robo = (void *)robo_attach(sbh,
                                     (1<<2), (1<<3), (1<<4), (1<<5)))) {
#endif /* KEYSTONE */                                     	
        return -ENODEV;
    }
#endif
    for (dev = 0; (dev<LINUX_BDE_MAX_SWITCH_DEVICES) &&
           (_switch_ndevices<LINUX_BDE_MAX_SWITCH_DEVICES); dev++) {
        unsigned short phyidh = 0, phyidl = 0;
        struct bde_spi_device_id *_ids;
        bde_ctrl_t* ctrl;
        uint8 part_8 = 0, rev = 0;
        uint16 part_16 = 0;
        uint32 part_32 = 0;
        uint64 part_64 = 0;

        /*
         * Read MII PHY Identifier registers for checking
         * the vendor/device id of robo chips.
         * Assume Port 0 MII page number (0x10) are fixed for all
         * robo chips.
         */
#ifdef MDC_MDIO_SUPPORT
        phyidh = _chipphyrd(0, 0, 2);
        gprintk("Pseudo Phy access mode.\n");
#else

#ifndef ALTA_ROBO_SPI
        /* first try EB_BUS */
        ROBO_RREG(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#endif

#ifdef BE_HOST
        phyidh = (phyidh>>8)|(phyidh<<8);
#endif
        if (_spi_device_valid_check(phyidh) && (dev<1)) {
            /*second try pseudo phy */
            robo_switch_bus(robo, 2);
            ROBO_RREG(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#ifdef BE_HOST
            phyidh = (phyidh>>8)|(phyidh<<8);
#endif
            if (_spi_device_valid_check(phyidh) && (dev<1)) {
                /*then try SPI */
                robo_switch_bus(robo, 1);
                ROBO_RREG(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#ifdef BE_HOST
                phyidh = (phyidh>>8)|(phyidh<<8);
#endif
            }
        } else {
            /* reg_access via EB bus*/
            eb_bus = 1;
        }
#endif

        /* Check if valid phyid */
        if (_spi_device_valid_check(phyidh)) {
            if (debug >= 1)
                gprintk("found %d robo device(s).\n", robo_switch);
            break;
        } else {
#ifdef MDC_MDIO_SUPPORT
            phyidl = _chipphyrd(0, 0, 3);
#else
            ROBO_RREG(robo, dev, 0x10, 0x06, (uint8 *)&phyidl, (uint)2);
#ifdef BE_HOST
            phyidl = (phyidl >> 8) | (phyidl << 8);
#endif
#endif
            if ((phyidl == 0xFFFF) || (phyidl == 0x0)) {
                if (debug >= 1)
                    gprintk("found %d robo device(s).\n", robo_switch);
                break;
            }
        }
#ifdef MDC_MDIO_SUPPORT
        if ((phyidh == BCM5348_PHYID_HIGH) && \
            ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
            robo_mdio_reg_read(0,0x388, &part_32, 1);
            part_8 = (part_32 & 0xFF);
            part_16 = (part_32 & 0xFFFF);
        } else if (((phyidh == BCM53242_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53242_PHYID_LOW)) || \
                ((phyidh == BCM53262_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53262_PHYID_LOW))){
                part_8 = 0;
                part_16 = part_8;
                rev = 0;
        } else {
            /* Register Model ID len = 4 for BCM53115, BCM53118 */
            if (((phyidh == BCM53115_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53115_PHYID_LOW)) || \
                ((phyidh == BCM53118_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53118_PHYID_LOW)) || \
                ((phyidh == BCM53101_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) || \
                 ((phyidh == BCM53125_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
                ((phyidh == BCM53128_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) {
                robo_mdio_reg_read(0,0x230, &part_32, 4);
#ifdef BE_HOST
                part_32 = (((part_32 >> 24) & 0x000000FF)|((part_32 >> 8) & 0x0000FF00)|
                               ((part_32 << 8) & 0x00FF0000)|((part_32 << 24) & 0xFF000000));
#endif
                part_16 = (part_32 & 0x0000FFFF);
            } else if ((phyidh == BCM53280_PHYID_HIGH) && 
                ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) {
                _soc_reg_read(0, 0xe8, (uint8 *)&part_64, 8);
                part_8 =  *(uint8 *)&part_64;
                part_16 = part_8;
            } else {
                robo_mdio_reg_read(0,0x230, &part_32, 1);
                part_16 = (part_32 & 0xFFFF);
            }

            robo_mdio_reg_read(0,0x240, &part_32, 1);
            rev = (part_32 & 0xFF);
        }
#else
        if ((phyidh == BCM5348_PHYID_HIGH) && \
            ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
            ROBO_RREG(robo, dev, 0x3, 0x88, (uint8 *)&part_8, (uint)1);
            part_16 = part_8;
        } else if (((phyidh == BCM53242_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53242_PHYID_LOW)) || \
                ((phyidh == BCM53262_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53262_PHYID_LOW))){
                part_8 = 0;
                part_16 = part_8;
                rev = 0;
        } else if (((phyidh == BCM53115_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53115_PHYID_LOW)) || \
                ((phyidh == BCM53118_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53118_PHYID_LOW)) || \
                ((phyidh == BCM53101_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) || \
                ((phyidh == BCM53125_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
                ((phyidh == BCM53128_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) {
            /* Register Model ID len = 4 for BCM53115, BCM53118 */
                ROBO_RREG(robo, dev, 0x2, 0x30, (uint8 *)&part_32, (uint)4);
#ifdef BE_HOST
                part_32 = (((part_32 >> 24) & 0x000000FF)|((part_32 >> 8) & 0x0000FF00)|
                               ((part_32 << 8) & 0x00FF0000)|((part_32 << 24) & 0xFF000000));
#endif
                part_16 = (part_32 & 0x0000FFFF);
                ROBO_RREG(robo, dev, 0x2, 0x40, (uint8 *)&rev, (uint)1);         
        }else if ((phyidh == BCM53280_PHYID_HIGH) && 
            ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) {
            ROBO_RREG(robo, dev, 0x0, 0xe8, (uint8 *)&part_64, (uint)8); 
            part_8 =  *(uint8 *)&part_64;
            part_16 = part_8;
        } else {
            ROBO_RREG(robo, dev, 0x2, 0x30, (uint8 *)&part_8, (uint)1);
            part_16 = part_8;
            
            ROBO_RREG(robo, dev, 0x2, 0x40, (uint8 *)&rev, (uint)1);
        }
#endif

        gprintk("found robo device with %d:%04x:%04x:%04x:%02x\n",
                dev, phyidh, phyidl, part_16, rev);
        robo_select_device(robo, phyidh, phyidl);

        for (_ids = _spi_id_table;
             _ids->phyid_high && _ids->phyid_low; _ids++) {

            if ((_ids->phyid_high == phyidh) &&    /* exclude revision */
                (_ids->phyid_low == (phyidl & 0xFFF0))) {

                                /* Match supported chips */
                ctrl = _devices + _switch_ndevices++;

                if (NULL == (ctrl->spi_device = (struct spi_dev *)
                             kmalloc(sizeof(struct spi_dev), GFP_KERNEL))) {
                    gprintk("no memory available");
                    return -ENOMEM;
                }

                ctrl->dev_type = (BDE_SPI_DEV_TYPE | BDE_SWITCH_DEV_TYPE);
                if (eb_bus) {
                    ctrl->dev_type |= BDE_EB_DEV_TYPE;
                }
                ctrl->spi_device->cid = dev;
                ctrl->spi_device->part = part_16;
                ctrl->spi_device->rev = rev;
                ctrl->spi_device->robo = robo;
                ctrl->spi_device->phyid_high = phyidh;
                ctrl->spi_device->phyid_low = phyidl;
                ctrl->bde_dev.device = phyidl & 0xFFF0;
                ctrl->bde_dev.rev = phyidl & 0xF;
                if ((phyidh == BCM5396_PHYID_HIGH) && \
                    (phyidl == BCM5396_PHYID_LOW)) {
                    ctrl->bde_dev.device = part_16;
                    ctrl->bde_dev.rev = rev;
                }
                if ((phyidh == BCM5398_PHYID_HIGH) && \
                    (phyidl == BCM5398_PHYID_LOW)) {
                    ctrl->bde_dev.device = part_16;
                    ctrl->bde_dev.rev = rev;
                }
                if ((phyidh == BCM5324_A1_PHYID_HIGH) && \
                    (phyidl == BCM5324_PHYID_LOW)) {
                    ctrl->bde_dev.rev = 0x1;
                }
                if ((phyidh == BCM5325_PHYID_HIGH) && \
                    (phyidl == BCM5325_PHYID_LOW)) {
                    ctrl->bde_dev.rev = 0x1;
                }
                if ((phyidh == BCM5348_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
                    ctrl->bde_dev.device = part_16;
                }
                if ((phyidh == BCM53280_PHYID_HIGH) && \
                    ((phyidl&0xFFF0) == BCM53280_PHYID_LOW)) {
                    ctrl->bde_dev.device = part_16;
                    part_8 = (uint8)(phyidl&0xF);
                    ctrl->bde_dev.rev =part_8 ;
                }
                if (((phyidh == BCM53128_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW)) || \
                    ((phyidh == BCM53125_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
                    ((phyidh == BCM53101_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW))) {
                    ctrl->bde_dev.rev = rev;
                }
                ctrl->bde_dev.base_address = (sal_vaddr_t)NULL;
                ctrl->isr = NULL;
                ctrl->isr_data = NULL;
                robo_switch++;
                _ndevices++;
                break;
            }
        }
#ifndef MDC_MDIO_SUPPORT
#if defined(KEYSTONE)
        /*
         * Thunderbolt and Lotus can support SPI Frequency up to 20MHz 
         */
        if (((phyidh == BCM53280_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) ||
            ((phyidh == BCM53101_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) ||
            ((phyidh == BCM53125_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || 
            ((phyidh == BCM53128_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) {
            if (SPI_FREQ_20MHZ < spi_freq) {
                spi_freq = SPI_FREQ_20MHZ;
            }
        } else {
            spi_freq = SPI_FREQ_DEFAULT;
        }

        /* 
         * Override the SPI frequency from user configuration
         */ 
        if (spifreq != 0) {
            spi_freq = spifreq;
        }
#endif
#endif
    }

#ifndef MDC_MDIO_SUPPORT
#if defined(KEYSTONE)
    /* The underlying chip can support the SPI frequency higher than default (2MHz) */
    if (spi_freq != SPI_FREQ_DEFAULT) {
        chipc_spi_set_freq(robo, 0, spi_freq); 
    }
#endif
#endif

    return robo_switch;
}
#endif

#if defined(BCM_METROCORE_LOCAL_BUS)
static bde_ctrl_t*
map_local_bus(uint64_t addr, uint32_t size)
{
    bde_ctrl_t* ctrl;

    ctrl = _devices + _switch_ndevices++;
    _ndevices++;

    /*
     * For now: use EB type as `local bus'
     * (memory mapped, no DMA, no interrupts)
     * metrocore local bus supports interrupts, but we don't use them.
     */
    ctrl->dev_type |= BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE
        | (size > 64 * 1024 ? BDE_128K_REG_SPACE : 0);
    ctrl->pci_device = NULL; /* No PCI bus */

    /* Map in the device */
    ctrl->phys_address = addr;
    ctrl->base_address = (sal_vaddr_t)
        IOREMAP(addr, size);
    ctrl->bde_dev.base_address = ctrl->base_address;

    return(ctrl);
}

#define BME_REVISION_OFFSET     (0x0)

#endif


#ifdef BCM_METROCORE_LOCAL_BUS
    /*
     * SBX platform has both PCI- and local bus-attached devices
     * The local bus devices have fixed address ranges (and don't
     * support or require DMA), but are otherwise the same as PCI devices
     */
#define FPGA_IRQ                37
#define FPGA_PHYS               0x100E0000
#define BME_PHYS                0x100C0000
#define SE_PHYS                 0x100D0000
#define FPGA_SIZE               0x00004000
#define BME_SIZE                0x00004000
#define MAC0_PHYS               0x100B0000
#define MAC1_PHYS               0x100B8000
#define MAC_SIZE                0x800


/*
 * Please refer to "Supervisor Fabric Module (SFM) Specification"
 * page 23 for the following registers.
 */
#define FPGA_LC_POWER_DISABLE_OFFSET             0x4
#define FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK    0x1e

#define FPGA_LC_POWER_RESET_OFFSET               0x5
#define FPGA_LC_POWER_RESET_ENABLE_ALL_MASK      0x1e

#define FPGA_SW_SFM_MASTER_MODE_OFFSET           0x14
#define FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK      0x10


static int
probe_metrocore_local_bus(void) {
    bde_ctrl_t* ctrl;
    uint32_t dev_rev_id;
    VOL uint8_t *fpga;

    /*
     * Write the FPGA on the fabric card, to let metrocore
     * line cards out of reset.  We actually don't bother to determine whether
     * the card is a line card or a fabric card because when we do
     * this on the line cards, it has no effect.
     */
    fpga = (uint8_t *) IOREMAP(FPGA_PHYS, FPGA_SIZE);
    fpga[FPGA_SW_SFM_MASTER_MODE_OFFSET]
        |= FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK;
    fpga[FPGA_LC_POWER_DISABLE_OFFSET]
        |= FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK;
    fpga[FPGA_LC_POWER_RESET_OFFSET]
        |= FPGA_LC_POWER_RESET_ENABLE_ALL_MASK;

    ctrl = map_local_bus(BME_PHYS, BME_SIZE);

    dev_rev_id =
        *((uint32_t *)
          (((uint8_t *) ctrl->base_address) + BME_REVISION_OFFSET));
    ctrl->bde_dev.device = dev_rev_id >> 16;
    ctrl->bde_dev.rev = (dev_rev_id & 0xFF);

    if ((ctrl->bde_dev.device != BME3200_DEVICE_ID) &&
    (ctrl->bde_dev.device != BCM88130_DEVICE_ID)) {
        gprintk("probe_metrocore_local_bus: wrong BME type: "
                "0x%x (vs 0x%x or 0x%x)\n",
                ctrl->bde_dev.device, BME3200_DEVICE_ID, BCM88130_DEVICE_ID);
        return -1;
    }

    ctrl->iLine = FPGA_IRQ;
    ctrl->isr = NULL;
    ctrl->isr_data = NULL;

    /*
     * <BME-- 64k --><SE -- 64k --><FPGA -- 64k -->
     * We start from SE & include the FPGA, which is 128k
     */
    ctrl = map_local_bus(SE_PHYS, 128 * 1024);

    dev_rev_id =
        *((uint32_t *)
          (((uint8_t *) ctrl->base_address) + BME_REVISION_OFFSET));
    ctrl->bde_dev.device = dev_rev_id >> 16;
    ctrl->bde_dev.rev = (dev_rev_id & 0xFF);

    if ((ctrl->bde_dev.device != BME3200_DEVICE_ID) &&
    (ctrl->bde_dev.device != BCM88130_DEVICE_ID)) {
        gprintk("probe_metrocore_local_bus: wrong SE (BME) type: "
                "0x%x (vs 0x%x)\n",
                ctrl->bde_dev.device, BME3200_DEVICE_ID);
        return -1;
    }

    ctrl->iLine = FPGA_IRQ;
    ctrl->isr = NULL;
    ctrl->isr_data = NULL;

    return 0;
}
#endif

#ifdef BCM_PLX9656_LOCAL_BUS

#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
#define DEV_REG_BASE_OFFSET     0x800000          /* Register base local bus offset */
#define DEV_REG_DEVID           0x178             /* Device ID is CMID devid */
#else
#define DEV_REG_BASE_OFFSET     PL0_OFFSET /* Polaris register base */
#define DEV_REG_DEVID           0          /* Device ID is first register */
#endif

/*
 * The difference at map_local_bus2:
 *
 * The PLX9656 PCI-to-LOCAL bridge chip already has been iomapped the whole address space.
 * So the devices off local bus don't need to be mapped again.  They only need to claim
 * their own sub-space.
 */
static bde_ctrl_t*
map_local_bus2(bde_ctrl_t* plx_ctrl, uint32_t dev_base, uint32_t size)
{
    uint32_t dev_rev_id;
    uint8_t *addr;

#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    uint32_t val;
#endif

    bde_ctrl_t* ctrl = _devices + _switch_ndevices++;
    _ndevices++;

    /*
     * For now: use EB type as `local bus'
     * (memory mapped, no DMA, no interrupts)
     * metrocore local bus supports interrupts, but we don't use them.
     */
    ctrl->dev_type |= BDE_EB_DEV_TYPE |  BDE_SWITCH_DEV_TYPE | BDE_320K_REG_SPACE; /* polaris 18 bits address + FPGA*/
    ctrl->pci_device = NULL; /* No PCI bus */

    /* Map in the device */
    ctrl->phys_address = plx_ctrl->phys_address + (resource_size_t)dev_base;
    ctrl->base_address = plx_ctrl->base_address + dev_base;
    ctrl->bde_dev.base_address = ctrl->base_address;

#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    addr = (uint8_t *)(ctrl->base_address) + 0x174;
    writel(0x0, (uint32 *)addr);
    addr = (uint8_t *)ctrl->base_address + DEV_REG_DEVID;
#else 
    addr = (uint8_t *)ctrl->base_address + PL0_REVISION_REG;
#endif
    dev_rev_id = readl(addr);
    ctrl->bde_dev.device = dev_rev_id >> 16;
    ctrl->bde_dev.rev = (dev_rev_id & 0xFF);

#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    val = dev_rev_id & 0xffff; /* Lower 16 bits */
    ctrl->bde_dev.rev = (val << 8 | val >> 8);
    val = (dev_rev_id & 0xffff0000) >> 16 ; /* upper 16 bits */
    ctrl->bde_dev.device = val << 8 | val >> 8;
#endif
    switch (ctrl->bde_dev.device) {
    case BCM88732_DEVICE_ID:
    case BCM88130_DEVICE_ID:
    case BME3200_DEVICE_ID:
        break;
    default:
        gprintk("wrong BME type: 0x%x (vs 0x%x or 0x%x)\n",
                    ctrl->bde_dev.device, BME3200_DEVICE_ID, BCM88130_DEVICE_ID);
        return 0;
    }
    return(ctrl);
}

static int
probe_plx_local_bus(void)
{
    bde_ctrl_t *ctrl;
    uint32_t val;
    uint8_t *addr;

    if (num_plx > 1) {
            printk(KERN_ERR "There's more than one PLX 9656/9056 chip\n");
        return -1;
    }
#ifdef CONFIG_RESOURCES_64BIT
    printk(KERN_ERR "Found PLX %04x:%04x vir: 0x%08x phy: 0x%08x%08x\n",
           plx_ctrl.bde_dev.device, plx_ctrl.bde_dev.rev,
           plx_ctrl.base_address, 
           (uint32_t)(plx_ctrl.phys_address >> 32), 
           (uint32_t)(plx_ctrl.phys_address)); 
#else
    printk(KERN_ERR "Found PLX %04x:%04x vir: 0x%08x phy: 0x%08x\n",
           plx_ctrl.bde_dev.device, plx_ctrl.bde_dev.rev,
           plx_ctrl.base_address, plx_ctrl.phys_address); 
#endif /* CONFIG_RESOURCES_64BIT */
    addr = (uint8_t *)plx_ctrl.base_address + CPLD_OFFSET + CPLD_REVISION_REG;
    val = readl(addr);
    printk(KERN_ERR "plx: CPLD revision %d\n", val & CPLD_REVISION_MASK);
#if 000
    addr = (uint8_t *)plx_ctrl.base_address + CPLD_OFFSET + CPLD_RESET_REG;
    writel(CPLD_RESET_NONE, addr);
#endif
#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    ctrl = map_local_bus2(&plx_ctrl, DEV_REG_BASE_OFFSET, PL0_SIZE);
#else
    ctrl = map_local_bus2(&plx_ctrl, PL0_OFFSET, PL0_SIZE);
#endif
    if (ctrl == 0)
        return -1;

    /* Uses PLX IRQ for Polaris LC */
    ctrl->iLine = 48;
    ctrl->isr = NULL;
    ctrl->isr_data = NULL;

    return 0;
}

#endif /* BCM_PLX9656_LOCAL_BUS */

/*
 * Generic module functions
 */

/*
 * Function: _init
 *
 * Purpose:
 *    Module initialization.
 *    Attaches to kernel BDE.
 * Parameters:
 *    None
 * Returns:
 *    0 on success, < 0 on error.
 */
static int
_init(void)
{

#ifdef BCM_ICS
    _ics_bde_create();
#else /* PCI */
    /* Register our goodies */
    _device_driver.name = LINUX_KERNEL_BDE_NAME;

    if (pci_register_driver(&_device_driver) < 0) {
        return -ENODEV;
    }

    /* Note: PCI-PCI bridge fixup uses results from pci_register_driver */
    fixup_p2p_bridge();
#ifdef BCM_ROBO_SUPPORT
    probe_robo_switch();
#endif
#ifdef BCM_METROCORE_LOCAL_BUS
    if (probe_metrocore_local_bus()) {
        return -1;
    }
#endif
#ifdef BCM_PLX9656_LOCAL_BUS
    if (num_plx > 0) {
        probe_plx_local_bus();
    }
#endif
#endif /* BCM_ICS */

    /*
     * Probe for EB Bus devices.
     */
    if (eb_bus) {
        char  *tok;
        uint   irq = -1, eb_rd16bit=0, eb_wr16bit =0;
        resource_size_t eb_ba = 0x0;

        gprintk("EB bus info: %s\n", eb_bus);
        tok = strtok(eb_bus,",");
        while (tok) {
            _parse_eb_args(tok, "BA=%x IRQ=%d RD16=%d WR16=%d",
                    &eb_ba, &irq, &eb_rd16bit, &eb_wr16bit);
            _eb_device_create(eb_ba, irq, eb_rd16bit, eb_wr16bit);
            tok = strtok(NULL,",");
        }
    }

    /* DMA Setup */
    if (dmasize) {
        if ((dmasize[strlen(dmasize)-1] & ~0x20) == 'M') {
            _dma_mem_size = simple_strtoul(dmasize, NULL, 0);
            _dma_mem_size *= ONE_MB;
        } else {
            gprintk("DMA memory size must be specified as e.g. dmasize=8M\n");
        }
        if (_dma_mem_size & (_dma_mem_size-1)) {
            gprintk("dmasize must be a power of 2 (1M, 2M, 4M, 8M etc.)\n");
            _dma_mem_size = 0;
        }
    } else {
        if(robo_switch){
            _dma_mem_size =  DMA_MEM_DEFAULT_ROBO;
        }
    }

    if (himem) {
        if ((himem[0] & ~0x20) == 'Y' || himem[0] == '1') {
            _use_himem = 1;
        } else if ((himem[0] & ~0x20) == 'N' || himem[0] == '0') {
            _use_himem = 0;
        }
    }

    if (_dma_mem_size) {
        _alloc_mpool(_dma_mem_size);
        if (_dma_vbase == NULL) {
            gprintk("no DMA memory available\n");
        }
        else {
            mpool_init();
            _dma_pool = mpool_create(_dma_vbase, _dma_mem_size);
        }
    }

    return 0;
}

/*
 * Function: _cleanup
 *
 * Purpose:
 *    Module cleanup function.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_cleanup(void)
{
    int i;

    if (_dma_vbase) {
        mpool_destroy(_dma_pool);
        if (_use_himem) {
            iounmap(_dma_vbase);
        } else {
#ifdef REMAP_DMA_NONCACHED
            iounmap(_dma_vbase);
#endif
            _pgcleanup();
        }
        _dma_vbase = NULL;
        _dma_pbase = 0;

    }
#ifdef BCM_ROBO_SUPPORT
    if (robo) {
        robo_detach(robo);
    }
    if (sbh) {
#ifdef KEYSTONE
	ai_soc_detach(sbh);
#else /* !KEYSTONE */    	
        sb_soc_detach(sbh);
#endif /* KEYSTONE */        
    }
#endif
    for(i = 0; i < _ndevices; i++) {
        int _i;
        bde_ctrl_t* ctrl;

        _i = DEVICE_INDEX(i);
        ctrl = _devices + _i;

        /* free allocated kernel space memory */
        if (ctrl->dev_type & BDE_SPI_DEV_TYPE) {
            kfree(ctrl->spi_device);
        }
    }
#ifdef BCM_ICS
#else
    pci_unregister_driver(&_device_driver);
#endif /* BCM_ICS */
    return 0;
}

/*
 * Function: _pprint
 *
 * Purpose:
 *    Print proc filesystem information.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_pprint(void)
{
    int i = 0;

    pprintf("Broadcom Device Enumerator (%s)\n", LINUX_KERNEL_BDE_NAME);
    pprintf("DMA Memory (%s): %d bytes, %d used, %d free\n",
            (_use_himem) ? "high" : "kernel",
            (_dma_vbase) ? _dma_mem_size : 0,
            (_dma_vbase) ? mpool_usage(_dma_pool) : 0,
            (_dma_vbase) ? _dma_mem_size - mpool_usage(_dma_pool) : 0);

    pprintf("Devices:\n");
    for (i = 0; i < _ndevices; i++) {
        int _i;
        bde_ctrl_t* ctrl;

        _i = DEVICE_INDEX(i);
        ctrl = _devices + _i;

        if (ctrl->dev_type & BDE_SWITCH_DEV_TYPE) {
            pprintf("Switch Device %d:\n", _i);
        } else if (ctrl->dev_type & BDE_ETHER_DEV_TYPE) {
            pprintf("Ethernet Device %d:\n", _i);
        }

        if (ctrl->dev_type & BDE_PCI_DEV_TYPE) {
            pprintf("\tPCI device %d:0x%x:0x%x:%d:0x%.8lx:%d\n",
                    _i,
                    ctrl->pci_device->vendor,
                    ctrl->pci_device->device,
                    ctrl->bde_dev.rev,
                    pci_resource_start(ctrl->pci_device, 0),
                    ctrl->pci_device->irq);
        } else if (ctrl->dev_type & BDE_SPI_DEV_TYPE) {
            pprintf("\tSPI Device %d:%d:%x:%x:0x%x:0x%x:%d\n",
                    _i,
                    ctrl->spi_device->cid,
                    ctrl->spi_device->part,
                    ctrl->spi_device->rev,
                    ctrl->spi_device->phyid_high,
                    ctrl->spi_device->phyid_low,
                    ctrl->bde_dev.rev);
        } else if (ctrl->dev_type & BDE_ICS_DEV_TYPE) {
            pprintf("\tICS Device %d:0x%x:0x%x\n",
                    _i,
                    ctrl->bde_dev.device,
                    ctrl->bde_dev.rev);
        } else if (ctrl->dev_type & BDE_EB_DEV_TYPE) {
            pprintf("\tEB Bus Device %d:0x%x:0x%x\n",
                    _i,
                    ctrl->bde_dev.device,
                    ctrl->bde_dev.rev);
        }
        if (debug >= 1) {
            pprintf("\timask:imask2:fmask 0x%x:0x%x:0x%x\n",
                    ctrl->imask,
                    ctrl->imask2,
                    ctrl->fmask);
        }
    }
    return 0;
}

/* Workaround for broken Busybox/PPC insmod */
static char _modname[] = LINUX_KERNEL_BDE_NAME;

static gmodule_t _gmodule = {
    name: LINUX_KERNEL_BDE_NAME,
    major: LINUX_KERNEL_BDE_MAJOR,
    init: _init,
    cleanup: _cleanup,
    pprint: _pprint,
};

gmodule_t *
gmodule_get(void)
{
    _gmodule.name = _modname;
    return &_gmodule;
}


/*
 * BDE Interface
 */

static const char *
_name(void)
{
    return LINUX_KERNEL_BDE_NAME;
}

static int
_num_devices(int type)
{
    switch (type) {
     case BDE_ALL_DEVICES:
        return _ndevices;
     case BDE_SWITCH_DEVICES:
        return _switch_ndevices;
     case BDE_ETHER_DEVICES:
        return _ether_ndevices;
    }

    return 0;
}

static const ibde_dev_t *
_get_dev(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        gprintk("_get_dev: Invalid device index %d\n", d);
        return NULL;
    }
    _d = DEVICE_INDEX(d);

    return &_devices[_d].bde_dev;
}

static uint32
_get_dev_type(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        gprintk("_get_dev: Invalid device index %d\n", d);
        return 0;
    }
    _d = DEVICE_INDEX(d);

    return _devices[_d].dev_type;
}

static uint32
_pci_conf_read(int d, uint32 addr)
{
#ifdef BCM_ICS
    return 0xFFFFFFFF;
#else
    uint32 rc = 0;
    int _d;

    if (!VALID_DEVICE(d)) {
        gprintk("_pci_conf_read: Invalid device index %d\n", d);
        return 0xFFFFFFFF;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        gprintk("_pci_conf_read: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return 0xFFFFFFFF;
    }

    pci_read_config_dword(_devices[_d].pci_device, addr, &rc);
    return rc;
#endif /* BCM_ICS */
}

static int
_pci_conf_write(int d, uint32 addr, uint32 data)
{
#ifdef BCM_ICS
    return -1;
#else
    int _d;

    if (!VALID_DEVICE(d)) {
        gprintk("_pci_conf_write: Invalid device index %d\n", d);
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        gprintk("_pci_conf_write: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    pci_write_config_dword(_devices[_d].pci_device, addr, data);
    return 0;
#endif /* BCM_ICS */
}

/* Initialized when the bde is created */
static linux_bde_bus_t _bus;

static void
_pci_bus_features(int unit, int *be_pio, int *be_packet, int *be_other)
{
    if ((_devices[unit].bde_dev.device & 0xFF00) != 0x5600 &&
        (_devices[unit].bde_dev.device & 0xF000) != 0xc000 &&
        (_devices[unit].bde_dev.device & 0xF000) != 0xb000 &&
        (_devices[unit].bde_dev.device & 0xFFF0) != 0x0230) {
        if (be_pio) *be_pio = 0;
        if (be_packet) *be_packet = 0;
        if (be_other) *be_other = 0;
    } else {
        if (be_pio) *be_pio = _bus.be_pio;
        if (be_packet) *be_packet = _bus.be_packet;
        if (be_other) *be_other = _bus.be_other;
    }
#if defined(BCM_METROCORE_LOCAL_BUS)
    if (_devices[unit].dev_type & BDE_EB_DEV_TYPE && be_pio) {
        *be_pio = 1;
    }
#endif

}

static uint32_t
_read(int d, uint32_t addr)
{
    int _d;
    unsigned long flags;
    volatile uint16  msb, lsb;
    uint32  sl_addr;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].dev_type & BDE_DEV_BUS_RD_16BIT) {
        /*
         * Adjust the address presented to Eb slave. Move A15:A0 to A16:A1.
         */
        sl_addr = (addr & 0xffff0000) | ((addr & 0xffff) << 1);
        /* Disable interrupts */
        spin_lock_irqsave(&bus_lock, flags);

        lsb = *((uint16 *)(_devices[_d].bde_dev.base_address + sl_addr));
        msb = *((uint16 *)(_devices[_d].bde_dev.base_address + sl_addr));
        spin_unlock_irqrestore(&bus_lock, flags);

        return (msb << 16) | lsb;
    } else {
        return ((VOL uint32_t *)_devices[_d].bde_dev.base_address)[addr / 4];
    }
}

static int
_write(int d, uint32_t addr, uint32_t data)
{
    int _d;
    unsigned long flags;
    uint32  sl_addr;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].dev_type & BDE_DEV_BUS_WR_16BIT) {
        /*
         * Adjust the address presented to Eb slave. Move A15:A0 to A16:A1.
         */
        sl_addr = (addr & 0xffff0000) | ((addr & 0xffff) << 1);

        /* Disable interrupts */
        spin_lock_irqsave(&bus_lock, flags);

        *((VOL uint16 *)(_devices[_d].bde_dev.base_address + sl_addr)) =
                                                             data & 0xffff;
        *((VOL uint16 *)(_devices[_d].bde_dev.base_address + sl_addr)) =
                                                     (data >> 16) & 0xffff;
        spin_unlock_irqrestore(&bus_lock, flags);
    } else {
        ((VOL uint32_t *)_devices[_d].bde_dev.base_address)[addr / 4] = data;
    }
    return 0;

}

static uint32_t *
_salloc(int d, int size, const char *name)
{
    void *ptr;

    if (_dma_mem_size) {
        return mpool_alloc(_dma_pool, size);
    }
    if ((ptr = kmalloc(size, GFP_ATOMIC)) == NULL) {
        ptr = _pgalloc(size);
    }
    return ptr;
}

static void
_sfree(int d, void *ptr)
{
    if (_dma_mem_size) {
        return mpool_free(_dma_pool, ptr);
    }
    if (_pgfree(ptr) < 0) {
        kfree(ptr);
    }
}

static int
_sinval(int d, void *ptr, int length)
{
#if defined(dma_cache_wback_inv)
     dma_cache_wback_inv((unsigned long)ptr, length);
#else
    dma_cache_sync(NULL, ptr, length, DMA_BIDIRECTIONAL);
#endif
    return 0;
}

static int
_sflush(int d, void *ptr, int length)
{
#if defined(dma_cache_wback_inv)
    dma_cache_wback_inv((unsigned long)ptr, length);
#else
    dma_cache_sync(NULL, ptr, length, DMA_BIDIRECTIONAL);
#endif

    return 0;
            

}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
static _ISR_RET
_isr(int irq, void *dev_id)
{
    bde_ctrl_t *ctrl = (bde_ctrl_t *) dev_id;
#ifdef BCM_ROBO_SUPPORT
#ifdef KEYSTONE 
    gmac0regs_t *regs;
    if ((ctrl->bde_dev.device == BCM53000_GMAC_ID) && 
        (ctrl->dev_type & BDE_ETHER_DEV_TYPE)) {
        if ((regs = (gmac0regs_t *)ai_soc_setcore(sbh, GMAC_CORE_ID, 0)) 
            == NULL) {
            return IRQ_NONE;
        }
        if (!ai_soc_iscoreup(sbh)) {
            return IRQ_NONE;
        }
 
        /* Return if not its events */
        if ((regs->intstatus & regs->intmask) == 0) {
            return IRQ_NONE;
        }
        /* Disable interrupt */
        regs->intmask = 0;
        
    }       
#endif /* KEYSTONE */
#endif /* BCM_ROBO_SUPPORT */

    if (ctrl && ctrl->isr) {
        ctrl->isr(ctrl->isr_data);
    }
    if (ctrl && ctrl->isr2) {
        ctrl->isr2(ctrl->isr2_data);
    }
    return IRQ_HANDLED;
}

#else
static _ISR_RET
_isr(int irq, void *dev_id,
     struct pt_regs *regs)
{
    bde_ctrl_t *ctrl = (bde_ctrl_t *) dev_id;

    if (ctrl && ctrl->isr) {
        ctrl->isr(ctrl->isr_data);
    }
    if (ctrl && ctrl->isr2) {
        ctrl->isr2(ctrl->isr2_data);
    }
    return IRQ_HANDLED;
}
#endif /* LINUX_VERSION_CODE */

static int
_interrupt_connect(int d,
                   void (*isr)(void *),
                   void *isr_data)
{
    bde_ctrl_t* ctrl;
    int _d, use_msi;
    int isr2_dev;
    int isr_active;

    isr2_dev = d & LKBDE_ISR2_DEV;
    d &= ~LKBDE_ISR2_DEV;

    if (!VALID_DEVICE(d)) {
        gprintk("_interrupt_connect: Invalid device index %d\n", d);
        return -1;
    }
    _d = DEVICE_INDEX(d);
    if (debug >= 1) {
        gprintk("_interrupt_connect d %d _d %d\n", d, _d);
    }
    if (!(_devices[_d].dev_type & (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE |
                                   BDE_EB_DEV_TYPE))) {
        gprintk("_interrupt_connect: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    isr_active = (ctrl->isr || ctrl->isr2) ? 1 : 0;

    if (isr2_dev) {
        if (debug >= 1) {
            gprintk("connect secondary isr\n");
        }
        ctrl->isr2_data = isr_data;
        ctrl->isr2 = isr;
        if (isr_active) {
            /* Main handler (_isr) already installed */
            return 0;
        }
    } else {
        if (debug >= 1) {
            gprintk("connect primary isr\n");
        }
        ctrl->isr = isr;
        ctrl->isr_data = isr_data;
        if (isr_active) {
            /* Main handler (_isr) already installed */
            return 0;
        }
    }

    if (ctrl->iLine != -1) {
#if defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT)
        use_msi = 1;
        if (pci_enable_msi(ctrl->pci_device)) {
            use_msi = 0;
        }
#else
        use_msi = 0;
#endif /* defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT) */
        if(request_irq( use_msi ? ctrl->pci_device->irq : ctrl->iLine,
                       _isr,
                       (use_msi ? 0 : IRQF_SHARED),
                       LINUX_KERNEL_BDE_NAME,
                       ctrl) < 0) {
            gprintk("could not request irq %d for device %d\n",
                    ctrl->pci_device->irq, _d);

            ctrl->isr = NULL;
            ctrl->isr_data = NULL;
            ctrl->isr2 = NULL;
            ctrl->isr2_data = NULL;
#if defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT)
            if (use_msi) {
                pci_disable_msi(ctrl->pci_device);
            }
#endif /* defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT) */
            return -1;
        }
    }

    return 0;
}

static int
_interrupt_disconnect(int d)
{
    bde_ctrl_t* ctrl;
    int _d;
    int isr2_dev;
    int isr_active;

    isr2_dev = d & LKBDE_ISR2_DEV;
    d &= ~LKBDE_ISR2_DEV;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (debug >= 1) {
        gprintk("_interrupt_disconnect d %d _d %d\n", d, _d);
    }
    if (!(_devices[_d].dev_type &
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
        gprintk("_interrupt_disconnect: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    isr_active = (ctrl->isr || ctrl->isr2) ? 1 : 0;

    if (isr2_dev) {
        if (debug >= 1) {
            gprintk("disconnect secondary isr\n");
        }
        ctrl->isr2 = NULL;
        ctrl->isr2_data = NULL;
        ctrl->fmask = 0;
        if (ctrl->isr) {
            /* Primary handler still active */
            return 0;
        }
    } else {
        if (debug >= 1) {
            gprintk("disconnect primary isr\n");
        }
        ctrl->isr = NULL;
        ctrl->isr_data = NULL;
        if (ctrl->isr2) {
            /* Secondary handler still active */
            return 0;
        }
    }

    if (isr_active) {
        free_irq(ctrl->iLine, ctrl);
#if defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT)
        pci_disable_msi(ctrl->pci_device);
#endif /* defined(CONFIG_PCI_MSI) && defined(BDE_LINUX_USE_MSI_INTERRUPT) */
    }

    return 0;
}

static sal_paddr_t
_l2p(int d, void *vaddr)
{
    if (_dma_mem_size) {
        /* dma memory is a contiguous block */
        if (vaddr) {
            return _dma_pbase + (PTR_TO_UINTPTR(vaddr) - PTR_TO_UINTPTR(_dma_vbase));
        }
        return 0;
    }
    return virt_to_bus(vaddr);
}

static uint32_t *
_p2l(int d, sal_paddr_t paddr)
{
    if (_dma_mem_size) {
        /* dma memory is a contiguous block */
        return paddr ? (void *)_dma_vbase + (paddr - _dma_pbase) : NULL;
    }
    return bus_to_virt(paddr);
}

#ifdef BCM_ROBO_SUPPORT
#define SOC_ROBO_PAGE_BP        8    /* for Robo Chip only */
static int
_spi_read(int d, uint32 addr, uint8 *buf, int len)
{
    bde_ctrl_t* ctrl;
    int _d;
    uint8 page, offset;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_spi_read: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    ROBO_RREG(ctrl->spi_device->robo, ctrl->spi_device->cid,
              page, offset, buf, (uint)len);

    return 0;
}

static int
_spi_write(int d, uint32 addr, uint8 *buf, int len)
{
    bde_ctrl_t* ctrl;
    int _d;
    uint8 page, offset;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_spi_write: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    ROBO_WREG(ctrl->spi_device->robo, ctrl->spi_device->cid,
              page, offset, buf, (uint)len);

    return 0;
}

#ifdef INCLUDE_ROBO_I2C
static int
_i2c_read(int d, uint16 addr, uint8 *buf, int len)
{
    bde_ctrl_t* ctrl;
    int _d;
    uint8 chipid, reg_addr;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_i2c_read: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    chipid = (addr >> 8) & 0xFF;
    reg_addr = addr & 0xFF;

    robo_i2c_rreg(ctrl->spi_device->robo, chipid, reg_addr, buf, (uint)len);

    return 0;
}

static int
_i2c_write(int d, uint16 addr, uint8 *buf, int len)
{
    bde_ctrl_t* ctrl;
    int _d;
    uint8 chipid, reg_addr;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_i2c_write: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    chipid = (addr >> 8) & 0xFF;
    reg_addr = addr & 0xFF;

    robo_i2c_wreg(ctrl->spi_device->robo, chipid, reg_addr, buf, (uint)len);

    return 0;
}

static int
_i2c_read_intr(int d, uint8 chipid, uint8 *buf, int len)
{
    bde_ctrl_t* ctrl;
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_i2c_read: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    gprintk("_i2c_read_intr: chipid %x\n", chipid);
    robo_i2c_rreg_intr(ctrl->spi_device->robo, chipid, buf);

    return 0;
}

static int
_i2c_read_ARA(int d, uint8 *chipid, int len)
{
    bde_ctrl_t* ctrl;
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        gprintk("_i2c_read: Not spi device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return -1;
    }

    ctrl = _devices + _d;

    robo_i2c_read_ARA(ctrl->spi_device->robo, chipid);
    gprintk("_i2c_read_ARA: chipid %x\n", *chipid);

    return 0;
}
#endif
#endif
static ibde_t _ibde = {
    name: _name,
    num_devices: _num_devices,
    get_dev: _get_dev,
    get_dev_type: _get_dev_type,
    pci_conf_read: _pci_conf_read,
    pci_conf_write: _pci_conf_write,
    pci_bus_features: _pci_bus_features,
    read: _read,
    write: _write,
    salloc: _salloc,
    sfree: _sfree,
    sinval: _sinval,
    sflush: _sflush,
    interrupt_connect: _interrupt_connect,
    interrupt_disconnect: _interrupt_disconnect,
    l2p: _l2p,
    p2l: _p2l,
#ifdef BCM_ROBO_SUPPORT
    spi_read: _spi_read,
    spi_write: _spi_write,
#ifdef INCLUDE_ROBO_I2C
    i2c_read: _i2c_read,
    i2c_write: _i2c_write,
    i2c_read_intr: _i2c_read_intr,
    i2c_read_ARA: _i2c_read_ARA,
#endif
#endif
};

/*
 * Function: linux_bde_create
 *
 * Purpose:
 *    Creator function for this BDE interface.
 * Parameters:
 *    bus - pointer to the bus features structure you want this
 *          bde to export. Depends on the system.
 *    ibde - pointer to a location to recieve the bde interface pointer.
 * Returns:
 *    0 on success
 *    -1 on failure.
 * Notes:
 *    This is the main BDE create function for this interface.
 *    Used by the external system initialization code.
 */
int
linux_bde_create(linux_bde_bus_t *bus, ibde_t **ibde)
{

    memset(&_bus, 0, sizeof(_bus));

    if (bus) {
        _bus = *bus;
    }
#ifdef NONCOHERENT_DMA_MEMORY
#ifdef REMAP_DMA_NONCACHED
    /*
     * If we have a non-cached DMA memory pool
     * there is no need to flush and invalidate.
     */
    if (_dma_vbase != NULL) {
        _ibde.sinval = NULL;
        _ibde.sflush = NULL;
    }
#endif
#else
    _ibde.sinval = NULL;
    _ibde.sflush = NULL;
#endif
    *ibde = &_ibde;
    return 0;
}

/*
 * Function: linux_bde_destroy
 *
 * Purpose:
 *    destroy this bde
 * Parameters:
 *    BDE interface pointer
 * Returns:
 *    0 on success, < 0 on error.
 */
int
linux_bde_destroy(ibde_t *ibde)
{
    /* nothing */
    return 0;
}

/*
 *  Backdoors provided by the kernel bde
 *
 */


/*
 * Some of the driver malloc's are too large for
 * kmalloc(), so 'sal_alloc' and 'sal_free' in the
 * linux kernel sal cannot be implemented with kmalloc().
 *
 * Instead, they expect someone to provide an allocator
 * that can handle the gimongous size of some of the
 * allocations, and we provide it here, by allocating
 * this memory out of the boot-time dma pool.
 *
 * These are the functions in question:
 */

void* kmalloc_giant(int sz)
{
    return mpool_alloc(_dma_pool, sz);
}

void kfree_giant(void* ptr)
{
    return mpool_free(_dma_pool, ptr);
}

/*
 *  Backdoors provided by the kernel bde
 */

uint32_t
lkbde_get_dev_phys(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type &
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
        gprintk("lkbde_get_dev_phys: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return 0;
    }
    return _devices[_d].phys_address;
}

#ifdef CONFIG_RESOURCES_64BIT
uint32_t
lkbde_get_dev_phys_hi(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type &
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
        gprintk("lkbde_get_dev_phys: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return 0;
    }
    return (uint32_t)(_devices[_d].phys_address >> 32);
}
#endif /* CONFIG_RESOURCES_64BIT */

void *
lkbde_get_dev_virt(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) return NULL;

    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type &
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
    gprintk("lkbde_get_dev_phys: Not pci device %d (%d), type %x\n",
                d, _d, _devices[_d].dev_type);
        return 0;
    }
    return (void *)_devices[_d].base_address;
}

int
lkbde_get_dma_info(uint32_t *pbase, uint32_t *size)
{
    if (_dma_pbase == 0) {
        if (_dma_mem_size == 0) {
            _dma_mem_size = DMA_MEM_DEFAULT;
        }
        _alloc_mpool(_dma_mem_size);
    }
    *pbase = _dma_pbase;
    *size = _dma_mem_size;
    return 0;
}

void *
lkbde_get_hw_dev(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) return NULL;

    _d = DEVICE_INDEX(d);

    return (void *)_devices[_d].pci_device;
}

/*
 * When a secondary interrupt handler is installed this function
 * is used for synchronizing hardware access to the IRQ mask
 * register. The secondary driver will supply a non-zero fmask
 * (filter mask) to indicate which interrupt bits it controls.
 * The fmask is ignored for the primary driver.
 */
int
lkbde_irq_mask_set(int d, uint32_t addr, uint32_t mask, uint32_t fmask)
{
    int _d;
    bde_ctrl_t *ctrl;
    int isr2_dev;

    isr2_dev = d & LKBDE_ISR2_DEV;
    d &= ~LKBDE_ISR2_DEV;

    if (!VALID_DEVICE(d)) return -1;

    _d = DEVICE_INDEX(d);
    ctrl = _devices + _d;

    /* Lock should not be required since no shared data is written */
    if (isr2_dev) {
        /* This is the secondary interrupt handler */
        ctrl->fmask = fmask;
        ctrl->imask2 = mask & ctrl->fmask;
    } else {
        /* This is the primary interrupt handler */
        ctrl->imask = mask & ~ctrl->fmask;
    }
    _write(d, addr, ctrl->imask | ctrl->imask2);
#ifdef KEYSTONE
    /* Enforce PCIe transaction ordering. Commit the write transaction */
    __asm__ __volatile__("sync");
#endif
    return 0;
}


/*
 * Export functions
 */
LKM_EXPORT_SYM(linux_bde_create);
LKM_EXPORT_SYM(linux_bde_destroy);
LKM_EXPORT_SYM(kmalloc_giant);
LKM_EXPORT_SYM(kfree_giant);
LKM_EXPORT_SYM(lkbde_get_dev_phys);
LKM_EXPORT_SYM(lkbde_get_dev_virt);
LKM_EXPORT_SYM(lkbde_get_dma_info);
LKM_EXPORT_SYM(lkbde_get_hw_dev);
LKM_EXPORT_SYM(lkbde_irq_mask_set);
#ifdef CONFIG_RESOURCES_64BIT
LKM_EXPORT_SYM(lkbde_get_dev_phys_hi);
#endif

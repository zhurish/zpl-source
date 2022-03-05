/* vxbUsrCmdLine.c - source file for vxBus command-line builds */

/**********************************************
*
* Copyright (C) 2007  Wind River Systems, Inc. All rights are reserved.
*
* The right to copy, distribute, modify, or
* otherwise make use of this software may be
* licensed only pursuant to the terms of an
* applicable Wind River license agreement.
*/

/*
 *
 *                 DO NOT EDIT
 *
 *
 * This file is automatically generated.
 *
 * If you have added/modified files in
 * /target/config/comps/src/hwif folder,
 * you need to re-create vxbUsrCmdLine.c.
 * Move to /target/config/comps/src/hwif and 
 * execute make vxbUsrCmdLine.c 
 */

#include <vxWorks.h>
#include "config.h"

#ifndef INCLUDE_VXBUS

/* vxBusIncluded: prevent compiler warning */

BOOL vxBusIncluded = FALSE;

#else /* INCLUDE_VXBUS */

BOOL vxBusIncluded = TRUE;

#ifndef PRJ_BUILD

#include <vxBusLib.h>
#include <hwif/vxbus/vxBus.h>

IMPORT STATUS vxbDevInitInternal (void);
IMPORT STATUS vxbDevConnectInternal (void);
#include <vxbus/vxBus.c>
#include <util/vxbDelayLib.c>
#include <vxbus/vxbArchAccess.c>
#include <vxbus/vxbPlbAccess.c>
#include <vxbus/vxbPlb.c>

/* vxBus Device Driver */

IMPORT void lnPciRegister(void);

IMPORT void bmtPhyRegister(void);

IMPORT void brgPhyRegister(void);

#ifdef INCLUDE_VXB_CPM
#include "../src/hwif/h/resource/cpm.h"
#endif


IMPORT void dmPhyRegister(void);

#ifdef INCLUDE_FCC_VXB_END
#include "../src/hwif/h/end/fccVxbEnd.h"
#endif


IMPORT void fecRegister(void);

IMPORT void feiRegister(void);

#ifdef INCLUDE_GT64120A_PCI
extern void g64120aPciRegister (void);
#endif /* INCLUDE_GT64120A_PCI */

#ifdef INCLUDE_GT64120A_MF
extern void g64120aMfRegister (void);
#endif /* INCLUDE_GT64120A_MF */

IMPORT void geiRegister(void);

#ifdef	INCLUDE_GEI_HEND
extern void geiHEndRegister (void);
#endif	/* INCLUDE_GEI_HEND */

IMPORT void geiTbiPhyRegister(void);

IMPORT void genPhyRegister(void);

IMPORT void genTbiRegister(void);

#ifdef DRV_PCIBUS_IXP4XX
extern void ixPciRegister (void);
#endif /* DRV_PCIBUS_IXP4XX */

IMPORT void lxtPhyRegister(void);

#ifdef DRV_RESOURCE_M85XXCCSR
extern void m85xxCCSRRegister(void);
#endif /* DRV_RESOURCE_M85XXCCSR */

#ifdef INCLUDE_M85XX_CPU
extern void m85xxCpuRegister(void);
#endif /* INCLUDE_M85XX_CPU */

#ifdef INCLUDE_M85XX_RAPIDIO
extern void m85xxRioRegister(void);
#endif /* INCLUDE_M85XX_RAPIDIO */

#ifdef INCLUDE_MCF5475_PCI
extern void mcf5475PciRegister (void);
#endif /* INCLUDE_MCF5475_PCI */

#ifdef INCLUDE_MDIO
#include "../src/hwif/h/mii/mdio.h"
#endif /* INCLUDE_MDIO */

IMPORT void motEtsecHEndRegister(void);

IMPORT void motFecHEndRegister(void);

IMPORT void motTsecHEndRegister(void);

IMPORT void mvfPhyRegister(void);

IMPORT void mvPhyRegister(void);

IMPORT void ynRegister(void);

IMPORT void eneRegister(void);

IMPORT void nicRegister(void);

#ifdef INCLUDE_PENTIUM_PCI
extern void pentiumPciRegister (void);
#endif /* INCLUDE_PENTIUM_PCI */

#ifdef INCLUDE_PPC440GP_PCI
#include <busCtlr/ppc440gpPci.c>
#endif /* INCLUDE_PPC440GP_PCI */

IMPORT void qeFccHEndRegister(void);

IMPORT void rtlRegister(void);

IMPORT void rtgPhyRegister(void);

IMPORT void rtgRegister(void);

IMPORT void rtlPhyRegister(void);

IMPORT void sbeRegister(void);

#ifdef INCLUDE_SCC_VXB_END
IMPORT void sccRegister (void);
#endif


#ifdef INCLUDE_TSEC_MDIO
#include "../src/hwif/h/mii/tsecMdio.h"
#endif /* INCLUDE_MDIO */

IMPORT void tsecRegister(void);

IMPORT void vigPhyRegister(void);

#ifdef DRV_TIMER_CN3XXX
#include <timer/vxbCn3xxxTimer.c>
#endif


#ifdef DRV_SIO_COLDFIRE
extern void coldFireSioRegister (void);
#endif /* DRV_SIO_COLDFIRE */


#ifdef DRV_EB_GIC
#include <intCtlr/vxbEbGenIntrCtl.c>
#endif /* DRV_EB_GIC */

#ifdef INCLUDE_EMAC_VXB_NET
#include "../src/hwif/h/end/vxbEmacEnd.h"
#endif


#ifdef DRV_INTCTLR_EPIC
extern void vxbEpicIntCtlrRegister (void);
#endif /* DRV_INTCTLR_EPIC */

IMPORT void etsecRegister(void);

#ifdef DRV_NVRAM_FILE
IMPORT void vxbFileNvRamRegister(void);
#endif /* DRV_NVRAM_FILE */


#ifdef DRV_KBD_I8042
extern void i8042vxbRegister (void);
#endif

#ifdef DRV_TIMER_I8253
    extern void vxbI8253TimerDrvRegister (void);
#endif /* DRV_TIMER_I8253 */

#ifdef DRV_INTCTLR_I8259
extern void vxbI8259IntCtlrRegister (void);
#endif /* DRV_INTCTLR_I8259 */


#ifdef INCLUDE_IPIIX4_MF
extern void vxbIPiix4MfRegister (void);
#endif


#ifdef INCLUDE_VXB_IBM_MAL
extern void vxbMalRegister(void);
#endif /* INCLUDE_VXB_IBM_MAL */

#ifdef INCLUDE_DRV_STORAGE_INTEL_ICH
    extern void vxbIntelIchStorageRegister (void);
#endif /* INCLUDE_DRV_STORAGE_INTEL_ICH */

#ifdef INCLUDE_DRV_STORAGE_INTEL_ICH_SHOW
    extern STATUS ichAtaShowInit (void);
#endif /* INCLUDE_DRV_STORAGE_INTEL_ICH_SHOW */

#ifdef DRV_TIMER_IA_TIMESTAMP
extern void vxbIaTimestampDrvRegister(void);
#endif


#ifdef DRV_INTCTLR_IOAPIC
extern void vxbIoApicIntrDrvRegister (void);
#endif /* DRV_INTCTLR_IOAPIC */

#ifdef DRV_SIO_IXP400
extern void ixp400SioRegister (void);
#endif /* DRV_SIO_IXP400 */

#ifdef DRV_TIMER_IXP400
extern void ixp400TimerDrvRegister (void);
#endif

#ifdef DRV_INTCTLR_LOAPIC
extern void vxbLoApicIntrDrvRegister (void);
#endif /* DRV_INTCTLR_LOAPIC */

#ifdef DRV_TIMER_LOAPIC
extern void vxbLoApicTimerDrvRegister(void);
#endif


#ifdef DRV_DMA_COLDFIRE
IMPORT void m548xDmaDrvRegister (void);
#endif

#ifdef DRV_TIMER_COLDFIRE
extern void m54x5TimerDrvRegister (void);
#endif

#ifdef DRV_VGA_M6845
extern void m6845vxbRegister (void);
#endif

#ifdef DRV_TIMER_M85XX
extern void m85xxTimerDrvRegister (void);
#endif /* DRV_TIMER_M85XX */   

#ifdef DRV_TIMER_MC146818
extern void vxbMc146818RtcDrvRegister (void);
#endif /* DRV_TIMER_MC146818 */   

#ifdef DRV_INTCTLR_MIPS_CAV
#include <intCtlr/vxbMipsCavIntCtlr.c>
#endif /* DRV_INTCTLR_MIPS_CAV */

#ifdef DRV_INTCTLR_MIPS
extern void  vxbMipsIntCtlrRegister (void);
#endif /* DRV_INTCTLR_MIPS */

#ifdef DRV_TIMER_MIPSR4K
    extern void vxbR4KTimerDrvRegister (void);
#endif /* DRV_TIMER_MIPSR4K */

#ifdef DRV_INTCTLR_MIPS_SBE
extern void vxbMipsSbIntCtlrRegister (void);
#endif /* DRV_INTCTLR_MIPS_SBE */

#ifdef DRV_INTCTLR_MPAPIC
extern void vxbMpApicDrvRegister (void);
#endif /* DRV_INTCTLR_MPAPIC */

#ifdef INCLUDE_MSC01_PCI
extern void vxbMsc01PciRegister(void);
#endif /* INCLUDE_MSC01_PCI */


#ifdef DRV_SIO_NS16550
extern void ns16550SioRegister (void);
#endif /* DRV_SIO_NS16550 */

#ifdef INCLUDE_OCTEON_MDIO
#include "../config/cav_cn3xxx_mipsi64r2sf/vxbOcteonMdio.h"
#endif


#ifdef INCLUDE_OCTEON_RGMII_VXB_END
#include "../config/cav_cn3xxx_mipsi64r2sf/vxbOcteonRgmiiEnd.h"
#endif


#ifdef DRV_SIO_OCTEON
#include <sio/vxbOcteonSio.c>
#endif /* DRV_SIO_OCTEON */

#ifdef DRV_TIMER_OPENPIC
IMPORT void openPicTimerDrvRegister(void);
#endif

#ifdef DRV_TIMER_DEC_PPC
extern void ppcDecTimerDrvRegister (void);
#endif

#ifdef DRV_INTCTLR_PPC
extern void ppcIntCtlrRegister (void);
#endif /* DRV_INTCTLR_PPC */

#ifdef DRV_TIMER_QUICC_PPC
extern void quiccTimerDrvRegister (void);
#endif

#ifdef	DRV_SIO_PRIMECELL
    extern void vxbPrimeCellSioRegister (void);
#endif	/* DRV_SIO_PRIMECELL */

#ifdef DRV_INTCTLR_QE
#include <hwif/intCtlr/vxbQeIntCtlr.h>
#endif /* DRV_INTCTLR_QE */


#ifdef DRV_INTCTLR_QUICC
#include <hwif/intCtlr/vxbQuiccIntCtlr.h>
#endif /* DRV_INTCTLR_QUICC */


#ifdef DRV_STORAGE_SI31XX
    extern void vxbSI31xxStorageRegister (void);
#endif /* DRV_STORAGE_SI31XX */

#ifdef DRV_SIO_SB1
    extern void vxbSb1DuartSioRegister (void);
#endif /* DRV_SIO_SB1 */


#ifdef DRV_TIMER_SB1
extern void vxbSb1TimerDrvRegister (void);   
#endif


#ifdef DRV_TIMER_SH7700
extern void sh7700TimerDrvRegister (void);
#endif

#ifdef DRV_SIO_SHSCIF
extern void shScifSioRegister (void);
#endif /* DRV_SIO_SHSCIF */

#ifdef	DRV_SUPERIO_SMCFDC37X
    extern void vxbSmcFdc37xRegister (void);
#endif	/* DRV_SUPERIO_SMCFDC37X */

IMPORT void smeRegister(void);

#ifdef INCLUDE_EHCI
extern void usbEhcdInstantiate (void );
extern int usbEhcdInit (void);
extern void vxbUsbEhciRegister (void);
#endif

#ifdef INCLUDE_OHCI
extern void usbOhciInstantiate (void );
extern STATUS usbOhcdInit (void);
extern void vxbUsbOhciRegister (void);
#endif

#ifdef INCLUDE_UHCI
extern void usbUhcdInstantiate (void );
extern int usbUhcdInit (void);
extern void vxbUsbUhciRegister (void);
#endif

/*
 * USB Initialization - the USBD must be initted before the host controller
 */
#ifdef INCLUDE_USB_INIT
IMPORT STATUS usbInit (void); 
#endif

IMPORT void vxbVxSimIntCtlrRegister(void);



/* vxBus Bus Controller Drivers */

#ifdef DRV_PCIBUS_M83XX
extern void m83xxPciRegister (void);
#endif /* DRV_PCIBUS_M83XX */


#ifdef DRV_PCIBUS_M85XX
extern void m85xxPciRegister (void);
#endif /* DRV_PCIBUS_M85XX */

#ifdef INCLUDE_MII_BUS
#include <mii/miiBus.h>
#endif

#ifdef INCLUDE_PCI_BUS
#include <vxbus/vxbPciAccess.c>
#include <vxbus/vxbPci.c>
#endif /* INCLUDE_PCI_BUS */

#ifdef INCLUDE_PPC440GX_PCI
extern void vxbPpc440gxPciRegister(void);
#endif /* INCLUDE_PPC440GX_PCI */

#ifdef INCLUDE_RAPIDIO_BUS
#include <vxbus/vxbRapidIO.c>
#ifdef RAPIDIO_BUS_STATIC_TABLE
#include <vxbus/vxbRapidIOCfgTable.c>
#endif /* RAPIDIO_BUS_STATIC_TABLE */
#endif /* INCLUDE_RAPIDIO_BUS */

#ifdef DRV_PCI_SH77XX
    extern void sh77xxPciRegister (void);
#endif /* DRV_PCI_SH77XX */




/* vxBus Utility Modules */

#ifdef INCLUDE_DOWNLOADABLE_DRIVERS
#  include <util/drvDownLoad.c>
#endif /* INCLUDE_DOWNLOADABLE_DRIVERS */

/* include hardware interface memory allocation scheme */
#ifdef INCLUDE_HWMEM_ALLOC
#include <util/hwMemLib.c>
#ifndef HWMEM_POOL_SIZE
#define HWMEM_POOL_SIZE 50000
#endif /* HWMEM_POOL_SIZE */
LOCAL char  pHwMemPool[HWMEM_POOL_SIZE];
#endif /* INCLUDE_HWMEM_ALLOC */

#ifdef INCLUDE_QUICC_ENGINE_UTILS
extern void quiccEngineRegister(void);
#endif

#ifdef INCLUDE_SIO_UTILS
#  include <util/sioChanUtil.c>
#endif /* INCLUDE_SIO_UTILS */

IMPORT char *  pAuxClkName;
IMPORT UINT32  auxClkDevUnitNo;
IMPORT UINT32  auxClkTimerNo;

#ifndef AUXCLK_TIMER_NAME
#define AUXCLK_TIMER_NAME NULL
#endif /* AUXCLK_TIMER_NAME */

#ifndef AUXCLK_TIMER_UNIT
#define AUXCLK_TIMER_UNIT 0
#endif /* AUXCLK_TIMER_UNIT */

#ifndef AUXCLK_TIMER_NUM
#define AUXCLK_TIMER_NUM 0
#endif /* AUXCLK_TIMER_NUM */

#ifdef VXBUS_TABLE_CONFIG
#include <util/vxbDevTable.c>
#endif

/* Support for DMA device drivers */
#ifdef INCLUDE_DMA_SYS
#  include <util/vxbDmaLib.c>
#endif /* INCLUDE_DMA_SYS */

/* Buffer support for drivers with on-device DMA engines */
#ifdef INCLUDE_DMA_SYS
IMPORT void vxbDmaBufInit (void);
IMPORT void vxbDmaLibInit (void);
#endif /* INCLUDE_DMA_SYS */

#ifdef INCLUDE_INTCTLR_DYNAMIC_LIB
extern void vxbIntDynaCtlrInit();
#endif /* INCLUDE_INTCTLR_DYNAMIC_LIB */

IMPORT void vxbLegacyIntInit();

#ifdef INCLUDE_NON_VOLATILE_RAM
extern void vxbNonVolLibInit (void);
#endif /* INCLUDE_NON_VOLATILE_RAM */

/* vxBus Parameter Subsystem */
#ifdef INCLUDE_PARAM_SYS
#  include <util/vxbParamSys.c>
#endif /* INCLUDE_PARAM_SYS */

#ifdef INCLUDE_SHOW_ROUTINES
#include <vxbus/vxbShow.c>
#endif /* INCLUDE_SHOW_ROUTINES */

#ifdef INCLUDE_VXBUS_SM_SUPPORT
#   include <util/vxbSmSupport.c>
#endif /* INCLUDE_VXBUS_SM_SUPPORT */

IMPORT char *  pSysClkName;
IMPORT UINT32  sysClkDevUnitNo;
IMPORT UINT32  sysClkTimerNo ;

#ifndef SYSCLK_TIMER_NAME
#define SYSCLK_TIMER_NAME NULL
#endif /* SYSCLK_TIMER_NAME */

#ifndef SYSCLK_TIMER_UNIT
#define SYSCLK_TIMER_UNIT 0
#endif /* SYSCLK_TIMER_UNIT */

#ifndef SYSCLK_TIMER_NUM
#define SYSCLK_TIMER_NUM 0
#endif /* SYSCLK_TIMER_NUM */

#ifdef INCLUDE_TIMER_SYS
#include <util/vxbTimerLib.c>
#else  /* INCLUDE_TIMER_SYS */
#  ifdef INCLUDE_TIMER_STUB
#    include <util/vxbTimerStub.c>
#  endif /* INCLUDE_TIMER_STUB */
#endif /* INCLUDE_TIMER_SYS */

IMPORT char *  pTimestampTimerName;
IMPORT UINT32  timestampDevUnitNo;
IMPORT UINT32  timestampTimerNo;

#ifndef TIMESTAMP_TIMER_NAME
#define TIMESTAMP_TIMER_NAME NULL
#endif /* TIMESTAMP_TIMER_NAME */

#ifndef TIMESTAMP_TIMER_UNIT
#define TIMESTAMP_TIMER_UNIT 0
#endif /* TIMESTAMP_TIMER_UNIT */

#ifndef TIMESTAMP_TIMER_NUM
#define TIMESTAMP_TIMER_NUM 0
#endif /* TIMESTAMP_TIMER_NUM */



/*******************************************
*
* hardWareInterFaceBusInit - initialize bus
*/

void hardWareInterFaceBusInit (void)
    {
    /* initialize bus subsystem */

    vxbLibInit();

#ifdef INCLUDE_QUICC_ENGINE_UTILS
    quiccEngineRegister();
#endif

#ifdef INCLUDE_DMA_SYS
    vxbDmaBufInit();
    vxbDmaLibInit();
#endif

#ifdef INCLUDE_NON_VOLATILE_RAM
    vxbNonVolLibInit();
#endif /* INCLUDE_NON_VOLATILE_RAM */

#ifdef INCLUDE_VXBUS_SM_SUPPORT
    smEndRegister();
#endif



    /*
     * initialize processor local bus
     * PLB: always included
     */

    plbRegister();

    /* bus registration */

#ifdef DRV_PCIBUS_M83XX
    m83xxPciRegister ();		/* M83xx PCI host controller */
#endif /* DRV_PCIBUS_M83XX */

#ifdef DRV_PCIBUS_M85XX
    m85xxPciRegister ();		/* M85xx PCI host controller */
#endif /* DRV_PCIBUS_M85XX */

#ifdef INCLUDE_MII_BUS
    miiBusRegister();
#endif

#ifdef INCLUDE_PCI_BUS
    pciRegister();
#endif /* INCLUDE_PCI_BUS */

#ifdef INCLUDE_PPC440GX_PCI
    vxbPpc440gxPciRegister();   /* PowerPC 440GX PCI host controller */
#endif /* INCLUDE_PPC440GX_PCI */

#ifdef INCLUDE_RAPIDIO_BUS
    rapidIoRegister();
#endif /* INCLUDE_RAPIDIO_BUS */

#ifdef DRV_PCI_SH77XX
    sh77xxPciRegister();
#endif /* DRV_PCI_SH77XX */



    /* driver registration */

#ifdef INCLUDE_AM79C97X_VXB_END
    lnPciRegister();
#endif

#ifdef INCLUDE_BCM52XXPHY
    bmtPhyRegister();
#endif

#ifdef INCLUDE_BCM54XXPHY
    brgPhyRegister();
#endif

#ifdef INCLUDE_VXB_CPM
    cpmRegister();
#endif


#ifdef INCLUDE_DM9161PHY
    dmPhyRegister();
#endif

#ifdef INCLUDE_FCC_VXB_END
   fccRegister();
#endif


#ifdef INCLUDE_FEC_VXB_END
    fecRegister();
#endif /* INCLUDE_FEC_VXB_END */

#ifdef INCLUDE_FEI8255X_VXB_END
    feiRegister();
#endif

#ifdef INCLUDE_GT64120A_PCI
    g64120aPciRegister ();
#endif /* INCLUDE_GT64120A_PCI */

#ifdef INCLUDE_GT64120A_MF
    g64120aMfRegister ();
#endif /* INCLUDE_GT64120A_MF */

#ifdef INCLUDE_GEI825XX_VXB_END
    geiRegister();
#endif

#ifdef INCLUDE_GEI_HEND
    geiHEndRegister();
#endif /* INCLUDE_GEI_HEND */

#ifdef INCLUDE_GEITBIPHY
    geiTbiPhyRegister();
#endif

#ifdef INCLUDE_GENERICPHY
    genPhyRegister();
#endif

#ifdef INCLUDE_GENERICTBIPHY
    genTbiRegister();
#endif

#ifdef DRV_PCIBUS_IXP4XX
    ixPciRegister ();		/* IXP4xx PCI host controller */
#endif /* DRV_PCIBUS_IXP4XX */

#ifdef INCLUDE_LXT972PHY
    lxtPhyRegister();
#endif

#ifdef DRV_RESOURCE_M85XXCCSR
    m85xxCCSRRegister();
#endif /* DRV_RESOURCE_M85XXCCSR */

#ifdef INCLUDE_M85XX_CPU
    m85xxCpuRegister();
#endif /* INCLUDE_M85XX_CPU */

#ifdef INCLUDE_M85XX_RAPIDIO
    m85xxRioRegister();
#endif /* INCLUDE_M85XX_RAPIDIO */

#ifdef INCLUDE_MCF5475_PCI
    mcf5475PciRegister();
#endif /* INCLUDE_MCF5475_PCI */

#ifdef INCLUDE_MDIO
    mdioRegister();
#endif /* mdioRegister */


#ifdef INCLUDE_MOT_ETSEC_HEND
    motEtsecHEndRegister();
#endif /* INCLUDE_MOT_ETSEC_HEND */

#ifdef INCLUDE_MOT_FEC_HEND
    motFecHEndRegister();
#endif /* INCLUDE_MOT_FEC_HEND */

#ifdef INCLUDE_MOT_TSEC_HEND
    motTsecHEndRegister();
#endif /* INCLUDE_MOT_TSEC_HEND */

#ifdef INCLUDE_MV88E1113PHY
    mvfPhyRegister();
#endif

#ifdef INCLUDE_MV88E1X11PHY
    mvPhyRegister();
#endif

#ifdef INCLUDE_MVYUKONII_VXB_END
    ynRegister();
#endif

#ifdef INCLUDE_NE2000_VXB_END
    eneRegister();
#endif

#ifdef INCLUDE_NS83902_VXB_END
    nicRegister();
#endif /* INCLUDE_NS83902_VXB_END */

#ifdef INCLUDE_PENTIUM_PCI
    pentiumPciRegister();       /* pcPentium PCI host controller */
#endif /* INCLUDE_PENTIUM_PCI */

#ifdef INCLUDE_PPC440GP_PCI
    ppc440gpPciRegister();              /* PowerPC 440GP PCI host controller */
#endif /* INCLUDE_PPC440GP_PCI */

#ifdef INCLUDE_QE_FCC_HEND
    qeFccHEndRegister();
#endif /* INCLUDE_QE_FCC_HEND */

#ifdef INCLUDE_RTL8139_VXB_END
    rtlRegister();
#endif

#ifdef INCLUDE_RTL8169PHY
    rtgPhyRegister();
#endif

#ifdef INCLUDE_RTL8169_VXB_END
    rtgRegister();
#endif

#ifdef INCLUDE_RTL8201PHY
    rtlPhyRegister();
#endif

#ifdef INCLUDE_SBE_VXB_END
    sbeRegister();
#endif

#ifdef INCLUDE_SCC_VXB_END
   sccRegister();
#endif /* INCLUDE_SCC_VXB_END */


#ifdef INCLUDE_TSEC_MDIO
    tmRegister();
#endif /* mdioRegister */


#ifdef INCLUDE_TSEC_VXB_END
    tsecRegister();
#endif /* INCLUDE_TSEC_VXB_END */

#ifdef INCLUDE_VSC82XXPHY
    vigPhyRegister();
#endif

#ifdef DRV_TIMER_CN3XXX
    vxbCn3xxxTimerDrvRegister ();
#endif
    

#ifdef DRV_SIO_COLDFIRE
    coldFireSioRegister();
#endif

#ifdef DRV_EB_GIC
    vxbEbGenIntrCtlRegister ();
#endif /* DRV_EB_GIC C*/

#ifdef INCLUDE_EMAC_VXB_NET
    vxbEmacRegister();
#endif /* INCLUDE_EMAC_VXB_END */

#ifdef DRV_INTCTLR_EPIC
    vxbEpicIntCtlrRegister();
#endif /* DRV_INTCTLR_EPIC */

#ifdef INCLUDE_ETSEC_VXB_END
    etsecRegister();
#endif /* INCLUDE_ETSEC_VXB_END */

#ifdef DRV_NVRAM_FILE
    vxbFileNvRamRegister();
#endif /* DRV_NVRAM_FILE */


#ifdef DRV_KBD_I8042
    i8042vxbRegister ();
#endif /* DRV_KBD_I8042 */

#ifdef DRV_TIMER_I8253
    vxbI8253TimerDrvRegister ();
#endif


#ifdef DRV_INTCTLR_I8259
    vxbI8259IntCtlrRegister  ();
#endif /* DRV_INTCTLR_I8259 */ 
    

#ifdef INCLUDE_IPIIX4_MF
    vxbIPiix4MfRegister ();
#endif /* INCLUDE_PIIX4_MF */


#ifdef INCLUDE_VXB_IBM_MAL
    vxbMalRegister();
#endif /* INCLUDE_VXB_IBM_MAL */

#ifdef INCLUDE_DRV_STORAGE_INTEL_ICH
    vxbIntelIchStorageRegister ();
#endif

#ifdef INCLUDE_DRV_STORAGE_INTEL_ICH_SHOW
    ichAtaShowInit ();
#endif

#ifdef DRV_TIMER_IA_TIMESTAMP
    vxbIaTimestampDrvRegister ();
#endif

#ifdef DRV_INTCTLR_IOAPIC
    vxbIoApicIntrDrvRegister ();
#endif /* DRV_INTCTLR_IOAPI C*/

#ifdef DRV_SIO_IXP400
    ixp400SioRegister();
#endif /* DRV_SIO_IXP400 */

#ifdef DRV_TIMER_IXP400
    ixp400TimerDrvRegister ();
#endif

#ifdef DRV_INTCTLR_LOAPIC
    vxbLoApicIntrDrvRegister ();
#endif /* DRV_INTCTLR_LOAPIC */

#ifdef DRV_TIMER_LOAPIC
    vxbLoApicTimerDrvRegister ();
#endif

#ifdef DRV_DMA_COLDFIRE
    m548xDmaDrvRegister ();
#endif /* DRV_DMA_COLDFIRE */

#ifdef DRV_TIMER_COLDFIRE
    m54x5TimerDrvRegister ();
#endif

#ifdef DRV_VGA_M6845
    m6845vxbRegister ();
#endif /* DRV_VGA_M6845 */

#ifdef DRV_TIMER_M85XX
    m85xxTimerDrvRegister ();
#endif /* DRV_TIMER_M85XX */  
    

#ifdef DRV_TIMER_MC146818
    vxbMc146818RtcDrvRegister ();
#endif /* DRV_TIMER_MC146818 */  
    

#ifdef DRV_INTCTLR_MIPS_CAV
    vxbMipsCavIntCtlrRegister ();
#endif /* DRV_INTCTLR_MIPS_CAV */
    

#ifdef DRV_INTCTLR_MIPS
    vxbMipsIntCtlrRegister ();
#endif /* DRV_INTCTLR_MIPS */
    

#ifdef DRV_TIMER_MIPSR4K
    vxbR4KTimerDrvRegister ();
#endif /* DRV_TIMER_MIPSR4K */

#ifdef DRV_INTCTLR_MIPS_SBE
    vxbMipsSbIntCtlrRegister ();
#endif /* DRV_INTCTLR_MIPS_SBE */
    

#ifdef DRV_INTCTLR_MPAPIC
    vxbMpApicDrvRegister ();
#endif /* DRV_INTCTLR_MPAPIC */

#ifdef INCLUDE_MSC01_PCI
    vxbMsc01PciRegister ();
#endif



#ifdef DRV_SIO_NS16550
    ns16550SioRegister();
#endif /* DRV_SIO_NS16550 */

#ifdef INCLUDE_OCTEON_MDIO
    octeonMdioRegister();
#endif


#ifdef INCLUDE_OCTEON_RGMII_VXB_END
    octRgmiiRegister();
#endif


#ifdef DRV_SIO_OCTEON
    octeonSioRegister();
#endif /* DRV_SIO_OCTEON */

#ifdef DRV_TIMER_OPENPIC
    openPicTimerDrvRegister ();
#endif

#ifdef DRV_TIMER_DEC_PPC
    ppcDecTimerDrvRegister ();
#endif

#ifdef DRV_INTCTLR_PPC
    ppcIntCtlrRegister();
#endif /* DRV_INTCTLR_PPC */

#ifdef DRV_TIMER_QUICC_PPC
    quiccTimerDrvRegister ();
#endif
    

#ifdef	DRV_SIO_PRIMECELL
    vxbPrimeCellSioRegister();
#endif	/* DRV_SIO_PRIMECELL */

#ifdef DRV_INTCTLR_QE
    vxbQeIntCtlrRegister();
#endif /* DRV_INTCTLR_QE */

#ifdef DRV_INTCTLR_QUICC
    vxbQuiccIntCtlrRegister();
#endif /* DRV_INTCTLR_QUICC  */

#ifdef DRV_STORAGE_SI31XX
    vxbSI31xxStorageRegister ();
#endif


#ifdef DRV_SIO_SB1
    vxbSb1DuartSioRegister();
#endif /* DRV_SIO_SB1 */
    

#ifdef DRV_TIMER_SB1
    vxbSb1TimerDrvRegister ();
#endif
    

#ifdef DRV_TIMER_SH7700
    sh7700TimerDrvRegister ();
#endif

#ifdef DRV_SIO_SHSCIF
    shScifSioRegister();
#endif /* DRV_SIO_SHSCIF */

#ifdef	DRV_SUPERIO_SMCFDC37X
    vxbSmcFdc37xRegister();
#endif	/* DRV_SUPERIO_SMCFDC37X */

#ifdef INCLUDE_SMSCLAN9118_VXB_END
    smeRegister();
#endif

#ifdef INCLUDE_EHCI
	usbEhcdInstantiate();
#endif
#ifdef INCLUDE_EHCI_INIT
	/* Register the Ehci with vxBus */
	vxbUsbEhciRegister ();
#endif

#ifdef INCLUDE_OHCI
	usbOhciInstantiate();
#endif
#ifdef INCLUDE_OHCI_INIT
	/* Register the Ohci with vxBus */
	vxbUsbOhciRegister ();
#endif

#ifdef INCLUDE_UHCI
	usbUhcdInstantiate();
#endif
#ifdef INCLUDE_UHCI_INIT
	/* Register the Uhci with vxBus */
	vxbUsbUhciRegister ();
#endif

#ifdef DRV_INTCTLR_VXSIM
    vxbVxSimIntCtlrRegister();
#endif /* DRV_INTCTLR_VXSIM */




    /* probe devices and create instances */

    vxbInit();

    }

/*********************************************
*
* hardWareInterFaceInit - HWIF Pre-Kernel Init
*/

void hardWareInterFaceInit (void)
    {

#ifdef INCLUDE_HWMEM_ALLOC
    /* Pre-Kernel Memory Allocation */
    hwMemLibInit();
    hwMemPoolCreate(pHwMemPool, HWMEM_POOL_SIZE);
#endif /* INCLUDE_HWMEM_ALLOC */

#ifdef INCLUDE_TIMER_SYS
#ifdef INCLUDE_AUX_CLK
    pAuxClkName = AUXCLK_TIMER_NAME;
    auxClkDevUnitNo = AUXCLK_TIMER_UNIT;
    auxClkTimerNo = AUXCLK_TIMER_NUM;
#endif /* INCLUDE_AUX_CLK */
#endif /* INCLUDE_TIMER_SYS */


#ifdef INCLUDE_INTCTLR_DYNAMIC_LIB
	vxbIntDynaCtlrInit();
#endif /* INCLUDE_INTCTLR_DYNAMIC_LIB */

#ifdef INCLUDE_TIMER_SYS
    pSysClkName = SYSCLK_TIMER_NAME;
    sysClkDevUnitNo = SYSCLK_TIMER_UNIT;
    sysClkTimerNo = SYSCLK_TIMER_NUM;
#endif /* INCLUDE_TIMER_SYS */


#ifdef INCLUDE_TIMER_SYS
#ifdef INCLUDE_TIMESTAMP
    pTimestampTimerName = TIMESTAMP_TIMER_NAME;
    timestampDevUnitNo = TIMESTAMP_TIMER_UNIT;
    timestampTimerNo = TIMESTAMP_TIMER_NUM;
#endif /* INCLUDE_TIMESTAMP */
#endif /* INCLUDE_TIMER_SYS */




    /* bus subsystem initialization */
    hardWareInterFaceBusInit();

#ifdef INCLUDE_TIMER_SYS
    vxbTimerLibInit();
#else  /* INCLUDE_TIMER_SYS */
#  ifdef INCLUDE_TIMER_STUB
    vxbTimerStubInit();
#  endif /* INCLUDE_TIMER_STUB */
#endif /* INCLUDE_TIMER_SYS */



    }


/*********************************************
*
* vxbDevInit - HWIF Post-Kernel Init
*/

STATUS vxbDevInit (void)
    {
    int status;
#ifdef INCLUDE_USB_INIT
	usbInit ();

	/* 
 	 * The following initializatino routines perform initialization of 
 	 * data structures only.  vxBus will invoke the internal initialization.
 	 *
 	 * The host controller driver must be initialized after the USBD.
 	 *
 	 */
	#ifdef INCLUDE_EHCI_INIT
		/* Initialize global data structures */
		usbEhcdInit ();
	#endif
	#ifdef INCLUDE_OHCI_INIT
		/* Initialize global data structures */
		usbOhcdInit ();
	#endif
	#ifdef INCLUDE_UHCI_INIT
		/* Initialize global data structures */
		usbUhcdInit ();
	#endif
#endif 			/* INCLUDE_USB_INIT */



    /* bus Device Initialization */
    status = vxbDevInitInternal();

#ifdef INCLUDE_VXB_LEGACY_INTERRUPTS
    vxbLegacyIntInit();
#endif /* INCLUDE_VXB_LEGACY_INTERRUPTS */



    return (status);
    }

/*********************************************
*
* vxbDevConnect - HWIF Post-Kernel Connection
*/

STATUS vxbDevConnect (void)
    {
    int status;


    /* Bus Device Connection */
    status = vxbDevConnectInternal();


    return (status);
    }

#endif /* PRJ_BUILD */
#endif /* INCLUDE_VXBUS */

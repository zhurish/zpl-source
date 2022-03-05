/*
 * $Id: chip.h,v 1.31 Broadcom SDK $
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
 * File:        chip.h
 * Purpose:     Defines for chip types, etc.
 * Requires:    soctypes.h, memory, register and feature defs.
 *
 * System on Chip (SOC) basic structures and typedefs
 * Each chip consists of a number of blocks.  Each block can be
 * a port interface controller (PIC) that contains ports.
 * The mcm/bcm*.i files contain definitions of each block and port
 * in the associated chip.  The structures used in those files
 * are defined here.  They are used to build internal use data
 * structures defined in soc/drv.h
 */

#ifndef _SOC_CHIP_H
#define _SOC_CHIP_H

#include <soc/types.h>
#include <soc/memory.h>
#include <soc/register.h>
#include <soc/feature.h>

/*
 * Arrays of soc_block_info_t are built by the registers program.
 * Each array is indexed by block number (per chip).
 * The last entry has type -1.
 * Used in bcm*.i files.
 */
typedef struct {
    int                 type;		/* SOC_BLK_* */
    int			number;		/* instance of type */
    int			schan;		/* pic number for schan commands */
    int			cmic;		/* pic number for cmic r/w commands */
} soc_block_info_t;

/*
 * Arrays of soc_port_info_t are built by the registers program.
 * Each array is indexed by port number (per chip).
 * Unused ports have blk -1 and bindex 0
 * The last entry has blk -1 and bindex -1
 * Used in bcm*.i files.
 */
typedef struct {
    int			blk;		/* index into soc_block_info array */
    int			bindex;		/* index of port within block */
} soc_port_info_t;

/*
 * Block types
 */
#define SOC_BLK_NONE      (0)         /* no port (used for empty port numbers) */
#define SOC_BLK_EPIC      (1<<0)      /* 10/100M ethernet ports */
#define SOC_BLK_GPIC      (1<<1)      /* 1G ethernet ports */
#define SOC_BLK_HPIC      (1<<2)      /* 10G higig ports (herc style) */
#define SOC_BLK_IPIC      (1<<3)      /* 10G higig ports (draco/lynx style) */
#define SOC_BLK_XPIC      (1<<4)      /* 10G ethernet ports */
#define SOC_BLK_CMIC      (1<<5)      /* CPU (XGS CMIC or SBX NIC) */
#define SOC_BLK_CPIC      (1<<6)      /* CPU (herc style) */
#define SOC_BLK_ARL       (1<<7)      /* Address Resolution Logic */
#define SOC_BLK_MMU       (1<<8)      /* Memory Management Unit */
#define SOC_BLK_MCU       (1<<9)      /* Memory Control Unit */
#define SOC_BLK_GPORT     (1<<10)     /* 1G Ethernet ports (FB/ER) */
#define SOC_BLK_XPORT     (1<<11)     /* 10G Ethernet/higig port(FB/ER)*/
#define SOC_BLK_IPIPE     (1<<12)     /* Ingress Pipeline (firebolt) */
#define SOC_BLK_IPIPE_HI  (1<<13)     /* Ingress Pipeline (Higig only) (FB) */
#define SOC_BLK_EPIPE     (1<<14)     /* Egress Pipeline (FB) */
#define SOC_BLK_EPIPE_HI  (1<<15)     /* Egress Pipeline (Higig only) (FB) */
#define SOC_BLK_IGR       (1<<16)     /* Ingress (ER) */
#define SOC_BLK_EGR       (1<<17)     /* Egress (ER) */
#define SOC_BLK_BSE       (1<<18)     /* Binary Search Engine (ER) */
#define SOC_BLK_CSE       (1<<19)     /* CAM Search Engine (ER) */
#define SOC_BLK_HSE       (1<<20)     /* Hash Search Engine (ER) */
#define SOC_BLK_BSAFE     (1<<21)     /* Broadsafe */
#define SOC_BLK_GXPORT    (1<<22)     /* 10/2.5/1G port */
#define SOC_BLK_SPI       (1<<23)     /* SPI port for ROBO or SBX */
#define SOC_BLK_EXP       (1<<24)     /* Expansion port for ROBO */
#define SOC_BLK_SYS       (1<<25)     /* SYS for ROBO */
#define SOC_BLK_XGPORT    (1<<26)     /* 10/100/1G/2.5/10G port (TR) */
#define SOC_BLK_SPORT     (1<<27)     /* 1G SGMII port (TR) */
#define SOC_BLK_INTER     (1<<28)     /* Internal MII for ROBO*/
#define SOC_BLK_EXTER     (1<<29)     /* External MII for ROBO*/
#define SOC_BLK_ESM       (1<<30)     /* External Search Engine (TR) */

#define SOC_BLK_OTPC      SOC_BLK_BSAFE /* OTP Controller (to be removed before release) */
#define SOC_BLK_QGPORT    SOC_BLK_GPORT /* 10/100/1G/2.5 port (SC) Alias */
#define SOC_BLK_XQPORT    SOC_BLK_XGPORT
#define SOC_BLK_XLPORT    SOC_BLK_GXPORT
#define SOC_BLK_LBPORT    SOC_BLK_SYS
#define SOC_BLK_PORT_GROUP4 SOC_BLK_INTER
#define SOC_BLK_PORT_GROUP5 SOC_BLK_EXTER

/* Shadow definitions */
#define SOC_BLK_IL         SOC_BLK_BSE
#define SOC_BLK_MS_ISEC    SOC_BLK_CSE
#define SOC_BLK_MS_ESEC    SOC_BLK_HSE
#define SOC_BLK_CW         SOC_BLK_SYS

/* Sirius regs */
#define SOC_BLK_BP       (1<<0) 
#define SOC_BLK_CI       (1<<1) 
#define SOC_BLK_CS       (1<<2) 
#define SOC_BLK_EB       (1<<3) 
#define SOC_BLK_EP       (1<<4) 
/* #define SOC_BLK_CMIC  (1<<5) */
#define SOC_BLK_ES       (1<<6) 
#define SOC_BLK_FD       (1<<7) 
#define SOC_BLK_FF       (1<<8) 
#define SOC_BLK_FR       (1<<9) 
#define SOC_BLK_TX       (1<<10)
/* #define SOC_BLK_XPORT (1<<11) */
#define SOC_BLK_QMA      (1<<12)
#define SOC_BLK_QMB      (1<<13)
#define SOC_BLK_QMC      (1<<14)
#define SOC_BLK_QSA      (1<<15)
#define SOC_BLK_QSB      (1<<16)
#define SOC_BLK_RB       (1<<17)
#define SOC_BLK_SC_TOP   (1<<18)
#define SOC_BLK_SF_TOP   (1<<19)
#define SOC_BLK_TS       (1<<20)
/* #define SOC_BLK_OTPC  (1<<21) */
/* #define SOC_BLK_GXPORT(1<<22) */
#define SOC_BLK_CI0      (SOC_BLK_CI)

#define SOC_BLK_PORT    (SOC_BLK_EPIC | \
                         SOC_BLK_GPIC | \
                         SOC_BLK_HPIC | \
                         SOC_BLK_IPIC | \
                         SOC_BLK_XPIC | \
                         SOC_BLK_GPORT | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_XGPORT | \
                         SOC_BLK_QGPORT | \
                         SOC_BLK_SPORT | \
                         SOC_BLK_XQPORT | \
                         SOC_BLK_CPIC)
#define SOC_BLK_CPU     (SOC_BLK_CMIC | \
                         SOC_BLK_CPIC)
#define SOC_BLK_ETHER   (SOC_BLK_EPIC | \
                         SOC_BLK_GPIC | \
                         SOC_BLK_GPORT | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_XGPORT | \
                         SOC_BLK_QGPORT | \
                         SOC_BLK_SPORT | \
                         SOC_BLK_XPIC)
#define SOC_BLK_HIGIG   (SOC_BLK_HPIC | \
                         SOC_BLK_XPORT | \
                         SOC_BLK_GXPORT | \
                         SOC_BLK_XGPORT | \
                         SOC_BLK_QGPORT | \
                         SOC_BLK_IPIC)
#define SOC_BLK_FABRIC  (SOC_BLK_SC_TOP | \
                         SOC_BLK_SF_TOP | \
			 SOC_BLK_SPI)
#define SOC_BLK_NET     (SOC_BLK_ETHER | \
                         SOC_BLK_HIGIG | \
                         SOC_BLK_FABRIC)
#define SOC_BLK_HGPORT  (SOC_BLK_IPIPE_HI)

#define SOC_BLK_SBX_PORT (SOC_BLK_GXPORT)


/*
 * Naming of blocks (there are two such arrays, one for
 * block based naming and one for port based naming)
 * Last entry has blk of SOC_BLK_NONE.
 */
typedef struct {
    soc_block_t     blk;        /* block bits to match */
    int             isalias;    /* this name is an alias */
    char            *name;      /* printable name */
} soc_block_name_t;

/* used to intialize soc_block_name_t soc_block_port_names[] */
#define SOC_BLOCK_PORT_NAMES_INITIALIZER    {  \
            /*    blk  , isalias, name */      \
            { SOC_BLK_EPIC,     0,  "fe"    }, \
            { SOC_BLK_GPIC,     0,  "ge"    }, \
            { SOC_BLK_GPORT,    0,  "ge"    }, \
            { SOC_BLK_GXPORT,   0,  "hg"    }, \
            { SOC_BLK_XGPORT,   0,  "ge"    }, \
            { SOC_BLK_QGPORT,   0,  "ge"    }, \
            { SOC_BLK_XQPORT,   0,  "ge"    }, \
            { SOC_BLK_SPORT,    0,  "ge"    }, \
            { SOC_BLK_HPIC,     0,  "hg"    }, \
            { SOC_BLK_IPIC,     0,  "hg"    }, \
            { SOC_BLK_XPIC,     0,  "xe"    }, \
            { SOC_BLK_XPORT,    0,  "hg"    }, \
            { SOC_BLK_CMIC,     0,  "cpu"   }, \
            { SOC_BLK_CPIC,     0,  "cpu"   }, \
            { SOC_BLK_SPI,      0,  "spi"   }, \
            { SOC_BLK_EXP,      0,  "exp"   }, \
            { SOC_BLK_LBPORT,   0,  "lb"   }, \
            { SOC_BLK_CPU,      0,  "cpu"   }, \
            { SOC_BLK_NONE,     0,  ""  } }

/* used to intialize soc_block_name_t soc_block_names[] */
#define SOC_BLOCK_NAMES_INITIALIZER {  \
    /*    blk  , isalias, name */      \
    { SOC_BLK_EPIC,     0,  "epic"  }, \
    { SOC_BLK_GPIC,     0,  "gpic"  }, \
    { SOC_BLK_HPIC,     0,  "hpic"  }, \
    { SOC_BLK_IPIC,     0,  "ipic"  }, \
    { SOC_BLK_XPIC,     0,  "xpic"  }, \
    { SOC_BLK_CMIC,     0,  "cmic"  }, \
    { SOC_BLK_CPIC,     0,  "cpic"  }, \
    { SOC_BLK_ARL,      0,  "arl"   }, \
    { SOC_BLK_MMU,      0,  "mmu"   }, \
    { SOC_BLK_MCU,      0,  "mcu"   }, \
    { SOC_BLK_GPORT,    0,  "gport" }, \
    { SOC_BLK_XPORT,    0,  "xport" }, \
    { SOC_BLK_GXPORT,   0,  "gxport" }, \
    { SOC_BLK_XLPORT,   0,  "xlport" }, \
    { SOC_BLK_XGPORT,   0,  "xgport" }, \
    { SOC_BLK_QGPORT,   0,  "qgport" }, \
    { SOC_BLK_XQPORT,   0,  "xqport" }, \
    { SOC_BLK_SPORT,    0,  "sport" }, \
    { SOC_BLK_IPIPE,    0,  "ipipe" }, \
    { SOC_BLK_IPIPE_HI, 0,  "ipipe_hi" }, \
    { SOC_BLK_EPIPE,    0,  "epipe" }, \
    { SOC_BLK_EPIPE_HI, 0,  "epipe_hi" }, \
    { SOC_BLK_IGR,      0,  "igr"  }, \
    { SOC_BLK_EGR,      0,  "egr"  }, \
    { SOC_BLK_BSE,      0,  "bse"  }, \
    { SOC_BLK_MS_ISEC,  0,  "ms_isec"  }, \
    { SOC_BLK_MS_ESEC,  0,  "ms_esec"  }, \
    { SOC_BLK_CSE,      0,  "cse"  }, \
    { SOC_BLK_HSE,      0,  "hse"  }, \
    { SOC_BLK_BSAFE,    0,  "bsafe" }, \
    { SOC_BLK_ESM,      0,  "esm"}, \
    { SOC_BLK_EPIC,     1,  "e"     }, \
    { SOC_BLK_GPIC,     1,  "g"     }, \
    { SOC_BLK_HPIC,     1,  "h"     }, \
    { SOC_BLK_IPIC,     1,  "i"     }, \
    { SOC_BLK_XPIC,     1,  "x"     }, \
    { SOC_BLK_CMIC,     1,  "cpu"   }, \
    { SOC_BLK_OTPC,     1,  "otpc"  }, \
    { SOC_BLK_SPI,      0,  "spi"   }, \
    { SOC_BLK_LBPORT,   0,  "lb"    }, \
    { SOC_BLK_PORT_GROUP4, 0, "port_group4" }, \
    { SOC_BLK_PORT_GROUP5, 0, "port_group5" }, \
    { SOC_BLK_CPU,      0,  "cpu"   }, \
    { SOC_BLK_NONE,     0,  ""      } }

/* used to intialize soc_block_name_t soc_sbx_block_port_names[] */
#define SOC_BLOCK_SBX_PORT_NAMES_INITIALIZER    {  \
            /*    blk  , isalias, name */      \
            { SOC_BLK_CMIC,     0,  "cpu"   }, \
	    { SOC_BLK_GXPORT,   0,  "gxport"}, \
            { SOC_BLK_NONE,     0,  ""  } }

/* used to intialize soc_block_name_t soc_sbx_block_names[] */
#define SOC_BLOCK_SBX_NAMES_INITIALIZER {  \
    /*    blk  , isalias, name */      \
    { SOC_BLK_BP,       0,  "bp"  }, \
    { SOC_BLK_CI,       0,  "ci"  }, \
    { SOC_BLK_CS,       0,  "cs"  }, \
    { SOC_BLK_EB,       0,  "eb"  }, \
    { SOC_BLK_EP,       0,  "ep"  }, \
    { SOC_BLK_CMIC,     0,  "cmic"}, \
    { SOC_BLK_ES,       0,  "es"  }, \
    { SOC_BLK_FD,       0,  "fd"   }, \
    { SOC_BLK_FF,       0,  "ff"   }, \
    { SOC_BLK_FR,       0,  "fr"   }, \
    { SOC_BLK_TX,       0,  "tx"   }, \
    { SOC_BLK_GXPORT,   0,  "gxport"}, \
    { SOC_BLK_QMA,      0,  "qma"  }, \
    { SOC_BLK_QMB,      0,  "qmb"  }, \
    { SOC_BLK_QMC,      0,  "qmc"  }, \
    { SOC_BLK_QSA,      0,  "qsa"  }, \
    { SOC_BLK_QSB,      0,  "qsb"  }, \
    { SOC_BLK_RB,       0,  "rb"   }, \
    { SOC_BLK_SC_TOP,   0,  "sc_top" }, \
    { SOC_BLK_SF_TOP,   0,  "sf_top" }, \
    { SOC_BLK_OTPC,     1,  "otpc"  }, \
    { SOC_BLK_TS,       0,  "ts"   }, \
    { SOC_BLK_NONE,     0,  ""      } }

#define SOC_BLK_BP       (1<<0) 
#define SOC_BLK_CI       (1<<1) 
#define SOC_BLK_CS       (1<<2) 
#define SOC_BLK_EB       (1<<3) 
#define SOC_BLK_EP       (1<<4) 
/* #define SOC_BLK_CMIC  (1<<5) */
#define SOC_BLK_ES       (1<<6) 
#define SOC_BLK_FD       (1<<7) 
#define SOC_BLK_FF       (1<<8) 
#define SOC_BLK_FR       (1<<9) 
#define SOC_BLK_TX       (1<<10)
/* #define SOC_BLK_XPORT (1<<11) */
#define SOC_BLK_QMA      (1<<12)
#define SOC_BLK_QMB      (1<<13)
#define SOC_BLK_QMC      (1<<14)
#define SOC_BLK_QSA      (1<<15)
#define SOC_BLK_QSB      (1<<16)
#define SOC_BLK_RB       (1<<17)
#define SOC_BLK_SC_TOP   (1<<18)
#define SOC_BLK_SF_TOP   (1<<19)
#define SOC_BLK_TS       (1<<20)
/* #define SOC_BLK_OTPC  (1<<21) */
/* #define SOC_BLK_GXPORT  (1<<22) */
#define SOC_BLK_CI0      (SOC_BLK_CI)

/*
 * soc_feature_fun_t: boolean function indicating if feature is supported
 * Used in bcm*.i files.
 */
typedef int (*soc_feature_fun_t) (int unit, soc_feature_t feature);

/*
 * soc_init_chip_fun_t: chip initialization function
 * Used in bcm*.i files.
 */
typedef void (*soc_init_chip_fun_t) (void);

/* Use macros to access */
extern soc_chip_groups_t soc_chip_type_map[SOC_CHIP_TYPES_COUNT];
extern char *soc_chip_type_names[SOC_CHIP_TYPES_COUNT];
extern char *soc_chip_group_names[SOC_CHIP_GROUPS_COUNT];

#define SOC_CHIP_NAME(type)	(soc_chip_type_names[(type)])

#endif	/* !_SOC_CHIP_H */

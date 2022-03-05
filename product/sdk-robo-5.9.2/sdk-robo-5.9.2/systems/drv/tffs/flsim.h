/******************************************************************************* 
 *                                                                             * 
 *                         M-Systems Confidential                              * 
 *           Copyright (C) M-Systems Flash Disk Pioneers Ltd. 1995-2002        * 
 *                         All Rights Reserved                                 * 
 *                                                                             * 
 ******************************************************************************* 
 *                                                                             * 
 *                         NOTICE OF M-SYSTEMS OEM                             * 
 *                         SOFTWARE LICENSE AGREEMENT                          * 
 *                                                                             * 
 *  THE USE OF THIS SOFTWARE IS GOVERNED BY A SEPARATE LICENSE AGREEMENT       * 
 *  BETWEEN THE OEM AND M-SYSTEMS. REFER TO THAT AGREEMENT FOR THE SPECIFIC    * 
 *  TERMS AND CONDITIONS OF USE, OR CONTACT M-SYSTEMS FOR LICENSE              * 
 *  ASSISTANCE:                                                                * 
 *  E-MAIL = info@m-sys.com                                                    * 
 *                                                                             * 
 *******************************************************************************
 *                                                                             * 
 *                                                                             * 
 *                         Module: FLSIM                                       * 
 *                                                                             * 
 *  This module implements simulation of the DiskOnChip on RAM.                *
 *                                                                             * 
 *******************************************************************************/

/*
 * $Log: flsim.h,v $
 * Revision 1.2  2004/11/12 09:33:27  kevinwu
 * Change for TFFS 5.1.4
 *
 * 
 *    Rev 1.1   12 Feb 2002 01:05:30   andreyk
 * TrueFFS-5.1
 */




#ifndef FLSIM_H
#define FLSIM_H



/* 
 * includes 
 */

#include "flflash.h"




/*
 * Types
 */

/* simDoc configuration; standard initializers are provided below */
typedef struct {    
    char     * mem_addr;          /* memory buffer to contain simDoc       */
    long       mem_size;          /* size of memory buffer                 */
    unsigned   flags;             /* NFTL_ENABLED or INFTL_ENABLED         */
    int        ppp_limit;         /* Partial Page Programming limit        */
    int        page_size;         /* size of flash page in bytes           */
    int        page_extra_size;   /* size of page's extra area in bytes    */
    long       block_size;        /* size of flash erase block in bytes    */
    long       chip_size;         /* size of flash chip in bytes           */
    int        chips;             /* total flash chips                     */
    int        floors;            /* total floors                          */
    int        interlv;           /* interleave (1, 2 or 4)                */
    int        ecc_size;          /* number of bytes in EDC/ECC checksum   */ 
} FLSimConf_t;


/* MTD statistics */
typedef struct {    
    long  blocks_erased;
    long  pages_read; 
    long  pages_written; 
    long  extra_read; 
    long  extra_written; 
    long  bytes_read;
    long  bytes_written;
    long  bytes_extra_read;
    long  bytes_extra_written;
} FLSimStat_t;


/* SimDoc */
typedef struct {    
    FLSimConf_t   conf;           /* simDoc configuration                  */
    FLSimStat_t   stats;          /* MTD statistics                        */
    char        * flash;          /* flash pages + extra areas             */
    char        * bbt;            /* Bad Block Table                       */
    char        * ppp_count;      /* Partial Page Programming counters     */
    void        * map_buffer;     /* MTD's map buffer                      */
} FLSim_t;




/* 
 * Macros
 */

/* DiskOnChip Millennium initializer for struct FLSimConf_t */
#define FL_SIM_CONF_MDOC  { NULL, 0, (unsigned)NFTL_ENABLED, 5, 512, 16, 0x2000L, 0x800000L, 1, 1, 1, 6 }

/* Millennium Plus initializer for struct FLSimConf_t */
#define FL_SIM_CONF_MPLUS { NULL, 0, (unsigned)INFTL_ENABLED, 5, 1024, 32, 0x8000L, 0x2000000L, 1, 1, 2, 6 }

/* limits on simDoc configuration */
#define FL_SIM_MAX_FLASH_ARRAY_SIZE    (0x80000000L) /* 2 GBytes */
#define FL_SIM_MAX_PPP_LIMIT           15
#define FL_SIM_MIN_PPP_LIMIT           1
#define FL_SIM_MAX_ECC_SIZE            12




/*
 * simDoc's API 
 */

/* TrueFFS's standard socket installation routine */
extern FLStatus  flRegisterSimSoc (FLSimConf_t FAR1 *conf);

/* TrueFFS's standard MTD installation routine */
extern FLStatus  flRegisterSimMTD (void);

/* define default simDoc configuration */
extern int  flsimDefaultConfig (FLSimConf_t FAR1 *conf);

/* memory required by given simDoc configuration */
extern long  flsimMemNeeded (FLSimConf_t FAR1 *conf);

/* retrieve simDoc's statistics */
extern int  flsimRetrieveStatistics (int socket, FLSimStat_t FAR1 *stats);

/* clear simDoc's statistics */
extern int  flsimClearStatistics (int socket);

/* change entry in simDoc' Bad Block Table */
extern int  flsimChangeBBT (int socket, long addr, char new, char *old);

/* default simDoc configuration */
extern FLSimConf_t  flsimDefaultConf;


#endif /* FLSIM_H */


/******************************************************************************* 
 *                                                                             * 
 *                        M-Systems Confidential                               * 
 *           Copyright (C) M-Systems Flash Disk Pioneers Ltd. 1995-2001        * 
 *                         All Rights Reserved                                 * 
 *                                                                             * 
 ******************************************************************************* 
 *                                                                             * 
 *                            NOTICE OF M-SYSTEMS OEM                          * 
 *                           SOFTWARE LICENSE AGREEMENT                        * 
 *                                                                             * 
 *      THE USE OF THIS SOFTWARE IS GOVERNED BY A SEPARATE LICENSE             * 
 *      AGREEMENT BETWEEN THE OEM AND M-SYSTEMS. REFER TO THAT AGREEMENT       * 
 *      FOR THE SPECIFIC TERMS AND CONDITIONS OF USE,                          * 
 *      OR CONTACT M-SYSTEMS FOR LICENSE ASSISTANCE:                           * 
 *      E-MAIL = info@m-sys.com                                                * 
 *                                                                             * 
 ******************************************************************************* 
 *                                                                             * 
 *                         Module: FLDRVVXW                                    * 
 *                                                                             * 
 *  This module implements VxWorks driver layer for TFFS.                      *
 *                                                                             * 
 *******************************************************************************/

/* 
 * $Log: fldrvvxw.h,v $
 * Revision 1.2  2004/11/12 09:33:26  kevinwu
 * Change for TFFS 5.1.4
 *  
   
      Rev 1.14   16 May 2003 19:58:58   andrayk
   TrueFFS-5.1.4
   
      Rev 1.13   03 May 2002 20:11:06   andreyk
   removed routines installing access handlers
   
      Rev 1.12   24 Apr 2002 02:36:10   andreyk
   TrueFFS-5.1 update
   
      Rev 1.11   13 Feb 2002 21:16:32   andreyk
   comments fixed
   
      Rev 1.10   12 Feb 2002 01:05:30   andreyk
   TrueFFS-5.1
   
      Rev 1.9   Nov 28 2001 22:08:22   andreyk
   TrueFFS-5.04
   
      Rev 1.8   03 Sep 2001 03:46:00   andreyk
   alignment of file system's buffers
   
      Rev 1.7   Jun 20 2001 19:52:54   oris
   
      Rev 1.6   17 May 2001 02:38:30   andreyk
   bug fixes in osak-5
   
      Rev 1.5   19 Feb 2001 20:42:16   andreyk
   dosFs-2 support added
   
      Rev 1.4   Dec 31 2000 16:42:12   vadimk
   OSAK-4.2.1: support for dosFs's long filenames
   
      Rev 1.3   May 28 2000 11:09:04   vadimk
   OSAK-4.1 with IOCTL support
   
      Rev 1.2   May 11 2000 19:34:02   vadimk
   "99 -> 98% media use"
   
      Rev 1.1   08 Mar 2000 14:03:32   dimitrys
   OSAK-4.1/VxWorks
 *  
 *    Rev 1.0   Jul 21 1999 16:28:52   Administrator
 * Initial Revision 
 */




#ifndef FLDRVVXW_H
#define FLDRVVXW_H




#ifdef __cplusplus
extern "C" {
#endif




/* VxWorks */
#include "blkIo.h"

/* M-Systems */
#include "flflash.h"
#include "blockdev.h"
#include "flformat.h"




/*
 *  TFFS device ("disk")
 */ 

typedef struct
{
    BLK_DEV  tffsBlkdev;         /* VxWorks block device descriptor       */
    int      tffsHandle;         /* TFFS handle (socketNo/driveNo)        */
    long     tffsOffset;         /* first sector of the 1st FAT partition */ 
    int      tffsFlags;          /* tffsDevCreate() flags saved here      */
    int      tffsReadCnt;        /* sector read statistics                */
    int      tffsWriteCnt;       /* sector write statistics               */
    char   * tffsAlignBuf;       /* aligned buffer                        */
    int      tffsUnalignedRead;  /* unaligned sectors read                */
    int      tffsUnalignedWrite; /* unaligned sectors written             */
} TFFS_DEV;


/*
 * partition of TFFS device ("part")
 */
typedef struct {
    int    type;                 /* FAT12_PARTIT, FAT16_PARTIT etc.       */
    long   startSecNo;           /* sectorNo where partition starts       */
    long   sectors;              /* total sectors in partition            */
} tffsPartition;


/*
 * TFFS handles
 */

#define tffsMakeHandle(socketNo,diskNo)  ((int)((((diskNo) & 0xf) << 4) | ((socketNo) & 0xf)))
#define tffsHandle2Soc(handle)           (((int)(handle)) & 0xf)
#define tffsHandle2Disk(handle)          ((((int)(handle)) >> 4) & 0xf)

extern TFFS_DEV* tffsHandle2Dev (int handle);          




/*
 * function declarations
 */

/* tell driver where to look for DiskOnChip socket(s) */ 
extern void      tffsSetup (int sockets, long *addressRange);

/* driver initialization routine */
extern STATUS    tffsDrv (void);

/* standard VxWorks block device creation routine */
extern BLK_DEV*  tffsDevCreate (int handle, int flags);

/* low-level socket format routine */
extern STATUS    tffsDevFormat (int socNo, int arg);

/* access to driver's advanced features */
extern STATUS    tffsSysCall (void *arg);

/* verify low level flash format on TFFS device */
extern STATUS    tffsDevVerify (int  handle, int flags);

/* find out how many DiskOnChip sockets have been found */ 
extern int       tffsSockets (void);

/* find out how many "disks" has been detected on given socket */
extern int       tffsDisksOnSocket (int socNo);

/* set various TFFS-wide runtime config. options */
extern STATUS    tffsSetOption (int option, void *pVal);

/* set various options per socket */
extern STATUS    tffsSetSocketOption (int socNo, int option, void *pVal);

/* set various options per disk */
extern STATUS    tffsSetDiskOption (int handle, int option, void *pVal);

/* to see driver's diagnostic log */
extern char*     tffsViewDebugLog (void);

/* flash "garbage collection" routine */
extern STATUS    tffsRecycle (int handle, int mode);

/* low-level disk format routine */
extern STATUS    tffsDiskFormat (int handle, int arg);

/* OBSOLETE: old-style (pre-version 5.0) format routine */
extern STATUS    tffsDevFormatOld (int handle, int arg);

/* find out how many filesystem partitions are on given "disk" */
extern STATUS    tffsHowManyParts (int handle, int *parts);

/* find out layout of filesystem partition */
extern STATUS    tffsPartInfo (int handle, int partNo, tffsPartition *info);

/* enable/disable FAT monitoring on filesystem partition */
extern STATUS    tffsPartCtrl (int handle, int partNo, int action);

/* OBSOLETE: enable multi-doc */ 
extern STATUS    tffsMultidoc (int op, int *pVal);

extern void      tffsAnnonceDiskChange (int handle);


/*
 * tffsDevCreate() flags
 */

#define FL_DOSFS_LONGNAMES   0x010  /* support older dosFs's long filenames  */
#define FL_TRUE_BLKDEV       0x020  /* dosFs-2 compatibility                 */
#define FL_DOSFS2            FL_TRUE_BLKDEV
#define FL_VERIFY_WRITE      0x040  /* verify every 'write sector' call      */
#define FL_VERIFY_WRITE2     0x080  /* veryify low-level flash 'writes'      */
#define FL_VERIFY_WRITE3     0x100  /* more rigorous than FL_VERIFY_WRITE2   */
#define FL_CHK_FORMAT        0x200  /* verify low-level format               */ 


/*
 * tffsDevVerify() flags
 */

#define TFFS_DEV_VERIFY_JUST_CHECK   0x01   /* only check, don't repair      */
#define TFFS_DEV_VERIFY_AND_FIX      0x02   /* standard repair mode          */
#define TFFS_DEV_VERIFY_AND_FIX2     0x04   /* rigorous repair mode          */


/*
 * tffsSetOption()/tffsSetSocketOption()/tffsSetDiskOption() operations
 */

#define  TFFS_OPT_NFTL_CACHE           1    /* TRUE (default) or FALSE       */ 
#define  TFFS_OPT_8BIT_ACCESS          2    /* TRUE or FALSE (default)       */
#define  TFFS_OPT_FAST_DEFRAG          3    /* TRUE or FALSE (default)       */
#define  TFFS_OPT_MARK_DELETE          4    /* TRUE (default) or FALSE       */
#define  TFFS_OPT_NFTL_CHAIN           5    
#define  TFFS_OPT_MTL_ALTDEFRAG        6    /* cancelled in version 5.1      */
#define  TFFS_OPT_MTD_VERIFY           7    /* cancelled in version 5.1      */
#define  TFFS_OPT_SUSPEND_MODE         8    /* see allowed values below      */
#define  TFFS_OPT_SEC_PER_FOLD         9    /* sectors to verify per folding */
#define  TFFS_OPT_VERIFY_WRITE_BDTL   10    /* see allowed values below      */
#define  TFFS_OPT_VERIFY_WRITE_BINARY 11  
#define  TFFS_OPT_VERIFY_WRITE_OTHER  12  
#define  TFFS_OPT_ACCESS_TYPE         13    /* see allowed values below      */


/*
 * values for tffsSetOption(TFFS_OPT_VERIFY_WRITE_BINARY)
 */

#define  TFFS_OPT_VERIFY_WRITE_BDTL_OFF  0  /* verify low-level flash writes */
#define  TFFS_OPT_VERIFY_WRITE_BDTL_ON   1  /* verify rigorously             */
#define  TFFS_OPT_VERIFY_WRITE_BDTL_UPS  2  /* (default) do not verify       */


/*
 * values for tffsSetOption(TFFS_OPT_SUSPEND_MODE)
 */

#define  TFFS_OPT_SUSPEND_OFF            0  /* normal I/O mode                  */
#define  TFFS_OPT_SUSPEND_WRITE          1  /* do not allow writing             */
#define  TFFS_OPT_SUSPEND_IO             3  /* do not allow reading and writing */


/*
 * values for tffsSetSocketOption(TFFS_OPT_ACCESS_TYPE)
 */

#define  TFFS_OPT_ACCESS_USER_DEFINED     0x00001000

#define  TFFS_OPT_NO_ADDR_SHIFT           0x00000000
#define  TFFS_OPT_SINGLE_ADDR_SHIFT       0x00000010
#define  TFFS_OPT_DOUBLE_ADDR_SHIFT       0x00000020

#define  TFFS_OPT_BUS_8BIT_ACCESS         0x00000001
#define  TFFS_OPT_BUS_16BIT_ACCESS        0x00000002
#define  TFFS_OPT_BUS_32BIT_ACCESS        0x00000004


/*
 * values for tffsPartCtrl()
 */

#define FL_FATFILT_WATCH                 1  /* enable FAT monitor            */
#define FL_FATFILT_DONT_WATCH            2  /* disable FAT monitor           */
#define FL_FATFILT_RESET                 3  /* reset FAT monitor             */


/*
 * OBSOLETE: tffsMultidoc() operations
 */

#define  TFFS_MULTIDOC_ENABLE            1
#define  TFFS_MULTIDOC_DISABLE           2
#define  TFFS_MULTIDOC_STATUS            3


/* 
 * disk formatting (routine tffsDiskFormat())
 */

/* struct to pass to tffsDiskFormat() */
typedef  BDTLPartitionFormatParams  tffsDiskFormatParams;

/* Standard initializer for struct tffsDiskFormatParams */
#define  TFFS_STD_DISK_FORMAT_PARAMS    STD_BDTL_PARAMS


/*
 * socket formatting (routine tffsSocketFormat())
 */

typedef  FormatParams2  tffsFormatParams2;

/* struct to pass to tffsSocketFormat() */
typedef struct {
    tffsFormatParams2  formatParams;
    unsigned           formatFlags;
} tffsSocketFormatParams;

/* Standard initializer for struct tffsSocketFormatParams */
#define  TFFS_STD_SOCKET_FORMAT_PARAMS  { STD_FORMAT_PARAMS2, TL_LEAVE_BINARY_AREA }


/*
 * obsolete: device formatting. Use socket or disk formatting instead.
 */

typedef  FormatParams  tffsFormatParams;

/* obsolete: struct to pass to tffsDevFormat() */
typedef struct {
  tffsFormatParams  formatParams;
  unsigned          formatFlags;
} tffsDevFormatParams;

/* obsolete: standard initializer for struct tffsDevFormatParams */
#define  TFFS_STD_FORMAT_PARAMS  { STD_FORMAT_PARAMS, TL_FORMAT_IF_NEEDED }

/* obsolete: values for tffsDevFormatParams.formatFlags */
#define  FTL_FORMAT            TL_FORMAT         
#define  FTL_FORMAT_IF_NEEDED  TL_FORMAT_IF_NEEDED
#define  NO_FTL_FORMAT         FAT_ONLY_FORMAT


/* buffer alignment, 1 means no aligment */ 
extern int  flBufferAlignment;




#ifdef __cplusplus
  }
#endif

#endif /* FLDRVVXW_H */

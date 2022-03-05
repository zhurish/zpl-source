
/*
 * $Log: flcustom.h,v $
 * Revision 1.2  2004/11/12 09:33:25  kevinwu
 * Change for TFFS 5.1.4
 *
 * 
 *    Rev 1.11   16 May 2003 20:00:44   andrayk
 * TrueFFS-5.1.4
 * 
 *    Rev 1.10   23 Jul 2002 21:36:12   andreyk
 * updated TrueFFS and driver version string
 * 
 *    Rev 1.9   24 Apr 2002 02:37:42   andreyk
 * TrueFFS-5.1 update
 * 
 *    Rev 1.8   12 Feb 2002 01:56:22   andreyk
 * TrueFFS-5.1
 * 
 *    Rev 1.7   Nov 28 2001 22:10:18   andreyk
 * TrueFFS-5.04
 * 
 *    Rev 1.6   17 Jul 2001 17:38:12   andreyk
 * number of binary partitions increased from 1 to 3
 * 
 *    Rev 1.5   Jun 20 2001 19:55:10   oris
 * minor change to copyright statement
 * 
 *    Rev 1.4   17 May 2001 02:37:02   andreyk
 * bug fixes in osak-5
 * 
 *    Rev 1.3   01 Mar 2001 21:52:22   andreyk
 * OSAK-4.3 and MD23xx support
 * 
 *    Rev 1.2   19 Feb 2001 20:50:58   andreyk
 * dosFs-2 support added
 * 
 *    Rev 1.1   May 28 2000 11:42:56   vadimk
 * OSAK-4.1 with IOCTL support
 * 
 */



/*********************************************************************************** 
 *                                                                                 * 
 *                        M-Systems Confidential                                   * 
 *           Copyright (C) M-Systems Flash Disk Pioneers Ltd. 1995-2001            * 
 *                         All Rights Reserved                                     * 
 *                                                                                 * 
 *********************************************************************************** 
 *                                                                                 * 
 *                            NOTICE OF M-SYSTEMS OEM                              * 
 *                           SOFTWARE LICENSE AGREEMENT                            * 
 *                                                                                 * 
 *      THE USE OF THIS SOFTWARE IS GOVERNED BY A SEPARATE LICENSE                 * 
 *      AGREEMENT BETWEEN THE OEM AND M-SYSTEMS. REFER TO THAT AGREEMENT           * 
 *      FOR THE SPECIFIC TERMS AND CONDITIONS OF USE,                              * 
 *      OR CONTACT M-SYSTEMS FOR LICENSE ASSISTANCE:                               * 
 *      E-MAIL = info@m-sys.com                                                    * 
 *                                                                                 * 
 ***********************************************************************************/



#ifndef FLCUSTOM_H
#define FLCUSTOM_H



/* 
 * Driver & OSAK Version strings 
 */

#define driverVersion   "5.1.4"
#define OSAKVersion     "5.1.4"



/* Number of sockets
 *
 * Defines the maximum number of physical drives supported.
 *
 * The actual number of sockets depends on which socket controllers are
 * actually registered and the numbe of sockets in the systems.
 */

#define SOCKETS 4



/* Number of volumes
 *
 * Defines the maximum number of logical drives supported.
 *
 * The actual number of drives depends on which socket controllers are
 * actually registered , the amount of devices in the system and the TL format of
 * each device
 */

#define VOLUMES  (SOCKETS * 4)  /* '4' is MAX_VOLUMES_PER_DOC (see inftl.h) */



/* Number of open files
 *
 * Defines the maximum number of files that may be open at a time.
 */

#define FILES   0



/* Low level operations
 *
 * Uncomment the following line if you want to do low level operations
 * (i.e. read from a physical address, write to a physical address,
 * erase a unit according to its physical unit number.
 */

#define FL_LOW_LEVEL



/* Placing EXB files
 *
 * Uncomment the following line if you need to place EXB on the media.
 */

#define WRITE_EXB_IMAGE



/* Formatting
 *
 * Uncomment the following line if you need to format with flFormatVolume.
 */

#define FORMAT_VOLUME



/* Defragmentation
 *
 * Uncomment the following line if you need to defragment with
 * flDefragmentVolume.
 */

#define DEFRAGMENT_VOLUME



/* 12-bit FAT support
 *
 * Comment the following line if you do not need support for DOS media with
 * 12-bit FAT (typically media of 8 MBytes or less).
 */

#define FAT_12BIT



/* Parse path function
 *
 * Uncomment the following line if you need to parse DOS-like path names
 * with flParsePath.
 */

#define MAX_VOLUME_MBYTES       1024L



/* Absolute read & write
 *
 * Uncomment the following line if you want to be able to read & write
 * sectors by absolute sector number (i.e. without regard to files and
 * directories).
 */

#define ABS_READ_WRITE



/* Application exit
 *
 * If the FLite application ever exits, it needs to call flEXit before
 * exitting. Uncomment the following line to enable this.
 */

#define EXIT



/* Number of sectors per FAT cluster
 *
 * Define the minimum cluster size in sectors.
 */

#define MIN_CLUSTER_SIZE   4



/* Fixed or removable media
 *
 * If your Flash media is fixed, uncomment the following line.
 */

#define FIXED_MEDIA



/* Interval timer
 *
 * The following defines a timer polling interval in milliseconds. If the
 * value is 0, an interval timer is not installed.
 *
 * If you select an interval timer, you should provide an implementation
 * for 'flInstallTimer' defined in flsysfun.h.
 *
 * An interval timer is not a must, but it is recommended. The following
 * will occur if an interval timer is absent:
 *
 * - Card changes can be recognized only if socket hardware detects them.
 * - The Vpp delayed turn-off procedure is not applied. This may downgrade
 *   write performance significantly if the Vpp switching time is slow.
 * - The watchdog timer that guards against certain operations being stuck
 *   indefinitely will not be active.
 */

#define POLLING_INTERVAL 0           /* no polling is done */



/* Maximum MTD's and Translation Layers
 *
 * Define here the maximum number of Memory Technology Drivers and
 * Translation Layers that may be installed. Note that the actual
 * number installed is determined by which components are installed in
 * 'flRegisterComponents' (flcustom.c)
 */

#define MTDS    3       /* DiskOnChip MTD, M+ MTD, one spare */
#define TLS     4       /* NFTL, iNFTL, MTL, and one spare */



/* NFTL cache
 *
 * Enable NFTL-cache
 * Turning on this option improves performance but requires additional RAM resources.
 * The NAND Flash Translation Layer (NFTL) is a specification for storing data on DiskOnChip
 *   in a way that enables to access that data as a Virtual Block Device.
 * If this option is on then NFTL keeps in RAM table of following format:
 *   Physical Unit number    Virtual Unit number   Replacement Unit number
 *         ppp                            vvv                rrr
 * Whenever it is needed to change table entry,
 *   NFTL updates it in the RAM table and on the DiskOnChip.
 * If NFTL has to read table entry then you can save time on reading sector from DiskOnChip.
 * Accessing data described in the table is done when user read/write API function is called.
 */

#define NFTL_CACHE



/* 
 * Environment variables
 */

#define ENVIRONMENT_VARS



/* 
 * Support standard IOCTL interface.
 */

#define IOCTL_INTERFACE



/* 
 * Enable S\W write protection of the device
 */

#define WRITE_PROTECTION

#define SCRAMBLE_KEY_1  647777603l
#define SCRAMBLE_KEY_2  232324057l



/* 
 * Enable H\W one time programing capablity
 */

#define HW_OTP



/*
 * Enable H\W protection of the device
 */

#define HW_PROTECTION



/* Read after write
 *
 * Add read after every write verifing data integrity. Make sure that
 * flVerifyWrite variable is also set to 1.
 *
 */

#define VERIFY_WRITE



/* Make sure a page is erased before writing it
 *
 * Partial write of a page, due to power failures, might make TrueFFS
 * consider a the page as free (sector flags were not written properly).
 * As a result the next write operation might write over an already written
 * area. The result might be either bad data of exesive PPP.
 * Defining this flag causes TrueFFS to verfit the sector prior to reading it.
 */

#define VERIFY_ERASED_SECTOR



/* Verify entire volume
 *
 * Scan the entire disk partition for power failures symptoms and correct them.
 *
 */

#define VERIFY_VOLUME



/* 
 * Enables access to the BDK partition 
 */

#define BDK_ACCESS



/* Number of binary partitions in the systems
 *
 * Defines Maximum Number of binary partitions in the system.
 *
 * The actual number of partitions depends on the format placed
 * on each device.
 */

#define BINARY_PARTITIONS 3



/* Multi_Doc TL
 *
 * This define will compile the code for registering the Multi-Doc TL
 * The TL will cause all TrueFFS devices in the system to be combined into
 * a single large meida.
 */

/* #define MULTI_DOC */



#endif /* FLCUSTOM_H */

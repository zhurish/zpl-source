
/*
 * $Log: _dosfrmt.h,v $
 * Revision 1.1  2004/11/12 09:33:22  kevinwu
 * Change for TFFS 5.1.4
 *
 * 
 *    Rev 1.0   May 14 2002 14:59:24   oris
 * Initial revision.
 */

/***********************************************************************************/
/*                        M-Systems Confidential                                   */
/*           Copyright (C) M-Systems Flash Disk Pioneers Ltd. 1995-2001            */
/*                         All Rights Reserved                                     */
/***********************************************************************************/
/*                            NOTICE OF M-SYSTEMS OEM                              */
/*                           SOFTWARE LICENSE AGREEMENT                            */
/*                                                                                 */
/*      THE USE OF THIS SOFTWARE IS GOVERNED BY A SEPARATE LICENSE                 */
/*      AGREEMENT BETWEEN THE OEM AND M-SYSTEMS. REFER TO THAT AGREEMENT           */
/*      FOR THE SPECIFIC TERMS AND CONDITIONS OF USE,                              */
/*      OR CONTACT M-SYSTEMS FOR LICENSE ASSISTANCE:                               */
/*      E-MAIL = info@m-sys.com                                                    */
/***********************************************************************************/

#ifndef _DOSFRMT_H
#define _DOSFRMT_H

#ifdef FORMAT_VOLUME

extern FLStatus flDosFormat(TL *, BDTLPartitionFormatParams FAR1 *formatParams);

#endif /* FORMAT_VOLUME */

#endif /* _DOSFRMT_H */

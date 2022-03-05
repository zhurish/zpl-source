/*
 * $Log: _flioctl.h,v $
 * Revision 1.1  2004/11/12 09:33:23  kevinwu
 * Change for TFFS 5.1.4
 *
 * 
 *    Rev 1.0   May 14 2002 14:59:32   oris
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

#ifndef _FLIOCTL_H
#define _FLIOCTL_H

#ifdef IOCTL_INTERFACE

FLStatus flIOctl(IOreq FAR2 *);

#endif /* IOCTL_INTERFACE */

#endif /* _FLIOCTL_H */

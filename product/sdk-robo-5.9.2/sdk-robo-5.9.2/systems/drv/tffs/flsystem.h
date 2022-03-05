
/*
 * $Log: flsystem.h,v $
 * Revision 1.2  2004/11/12 09:33:27  kevinwu
 * Change for TFFS 5.1.4
 *
 * 
 *    Rev 1.9   16 May 2003 20:02:42   andrayk
 * TrueFFS-5.1.4
 * 
 *    Rev 1.8   03 May 2002 20:12:32   andreyk
 * added FL_NO_INIT_MMU_PAGES
 * 
 *    Rev 1.7   21 Sep 2001 17:47:40   andreyk
 * BIG_ENDIAN changed to FL_BIG_ENDIAN
 * flMsecToYieldCPU added
 * 
 *    Rev 1.6   03 Sep 2001 03:43:08   andreyk
 * alignment of file system's buffers
 * 
 *    Rev 1.5   16 Jul 2001 16:06:52   andreyk
 * DFORMAT_PRINT added
 * 
 *    Rev 1.4   Jun 20 2001 19:56:26   oris
 * minor change to copyright statement
 * 
 *    Rev 1.3   17 May 2001 02:34:10   andreyk
 * bug fixes in osak-5
 * 
 *    Rev 1.2   May 28 2000 11:15:00   vadimk
 * OSAK-4.1 with IOCTL support
 * 
 *    Rev 1.1   Apr 27 2000 11:52:10   vadimk
 * remove stdmem*** definitions

      Rev 1.5   03 Nov 1997 16:27:14   danig
   pointerToPhysical

      Rev 1.4   11 Sep 1997 14:14:22   danig
   physicalToPointer receives drive no. when FAR == 0

      Rev 1.3   04 Sep 1997 13:58:30   danig
   DEBUG_PRINT

      Rev 1.2   28 Aug 1997 16:39:32   danig
   include stdlib.h instead of malloc.h

      Rev 1.1   19 Aug 1997 20:05:06   danig
   Andray's changes

      Rev 1.0   24 Jul 1997 18:13:06   amirban
   Initial revision.
 */

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
 *                         Module: FLSYSTEM                                    * 
 *                                                                             * 
 *  This module implements VxWorks-to-TFFS bindings.                           *
 *                                                                             * 
 *******************************************************************************/




#ifndef FLSYSTEM_H
#define FLSYSTEM_H




#ifdef __cplusplus
  extern "C" {
#endif




/* VxWorks #includes */
#include <vxWorks.h>
#include <tickLib.h>
#include <sysLib.h>
#include <semLib.h>
#include <ioLib.h>
#include <errnoLib.h>
#include <taskLib.h>
#include <memLib.h>

/* ANSI C #includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>




/*
 *          signed/unsigned char
 *
 * It is assumed that 'char' is signed. If this is not your compiler
 * default, use compiler switches, or insert a #pragma here to define this.
 *
 */

/* char is signed by default in GNU C */


/*          CPU target
 *
 * Use compiler switches or insert a #pragma here to select the CPU type
 * you are targeting.
 *
 * If the target is an Intel 80386 or above, also uncomment the CPU_i386
 * definition.
 */

/* should be defined in Makefile */


/*          NULL constant
 *
 * Some compilers require a different definition for the NULL pointer
 */

/* using VxWorks default */


/*          Little-endian/big-endian
 *
 * FAT and translation layers structures use the little-endian (Intel)
 * format for integers.
 * If your machine uses the big-endian (Motorola) format, uncomment the
 * following line.
 * Note that even on big-endian machines you may omit the FL_BIG_ENDIAN
 * definition for smaller code size and better performance, but your media
 * will not be compatible with standard FAT and FTL.
 */

/* we are using VxWorks #define _BYTE_ORDER here */
#ifndef _BYTE_ORDER
#error "byte order is not defined"
#else  /* _BYTE_ORDER */
#if (_BYTE_ORDER == _BIG_ENDIAN)
#define FL_BIG_ENDIAN
#else  /* (_BYTE_ORDER == _BIG_ENDIAN) */
#undef FL_BIG_ENDIAN
#endif /* (_BYTE_ORDER == _BIG_ENDIAN) */
#endif /* _BYTE_ORDER */


/*          Far pointers
 *
 * Specify here which pointers may be far, if any.
 * Far pointers are usually relevant only to 80x86 architectures.
 *
 * Specify FAR_LEVEL:
 *   0 -    if using a flat memory model or having no far pointers.
 *   1 -        if only the socket window may be far
 *   2 -    if only the socket window and caller's read/write buffers
 *      may be far.
 *   3 -    if socket window, caller's read/write buffers and the
 *      caller's I/O request packet may be far
 */

#define FAR_LEVEL       0


/*          Pointer arithmetic
 *
 * The following macros define machine- and compiler-dependent macros for
 * handling pointers to physical window addresses. The definitions below are
 * for PC real-mode Borland-C.
 *
 * 'physicalToPointer' translates a physical flat address to a (far) pointer.
 * Note that if when your processor uses virtual memory, the code should
 * map the physical address to virtual memory, and return a pointer to that
 * memory (the size parameter tells how much memory should be mapped).
 *
 * 'addToFarPointer' adds an increment to a pointer and returns a new
 * pointer. The increment may be as large as your window size. The code
 * below assumes that the increment may be larger than 64 KB and so performs
 * huge pointer arithmetic.
 */

#define physicalToPointer(physical,size,drive)          \
    ((void *) (physical))

#define pointerToPhysical(ptr)  ((unsigned long)(ptr))

#define addToFarPointer(base,increment)     \
    ((void *) ((unsigned char *) (base) + (increment)))

#define freePointer(ptr,size) 


/*          Default calling convention
 *
 * C compilers usually use the C calling convention to routines (cdecl), but
 * often can also use the pascal calling convention, which is somewhat more
 * economical in code size. Some compilers also have specialized calling
 * conventions which may be suitable. Use compiler switches or insert a
 * #pragma here to select your favorite calling convention.
 */

/* use compiler default calling convention */



/*          Mutex type
 *
 * If you intend to access the FLite API in a multi-tasking environment,
 * you may need to implement some resource management and mutual-exclusion
 * of FLite with mutex & semaphore services that are available to you. In
 * this case, define here the Mutex type you will use, and provide your own
 * implementation of the Mutex functions incustom.c
 *
 * By default, a Mutex is defined as a simple counter, and the Mutex
 * functions in custom.c implement locking and unlocking by incrementing
 * and decrementing the counter. This will work well on all single-tasking
 * environment, as well as on many multi-tasking environments.
 */

typedef SEM_ID FLMutex;

#define flStartCriticalSection(mutexPtr)  flTakeMutex(mutexPtr)
#define flEndCriticalSection(mutexPtr)    flFreeMutex(mutexPtr)


/*          Memory allocation
 *
 * The translation layers (e.g. FTL) need to allocate memory to handle
 * Flash media. The size needed depends on the media being handled.
 *
 * You may choose to use the standard 'malloc' and 'free' to handle such
 * memory allocations, provide your own equivalent routines, or you may
 * choose not to define any memory allocation routine. In this case, the
 * memory will be allocated statically at compile-time on the assumption of
 * the largest media configuration you need to support. This is the simplest
 * choice, but may cause your RAM requirements to be larger than you
 * actually need.
 *
 * If you define routines other than malloc & free, they should have the
 * same parameters and return types as malloc & free. You should either code
 * these routines in flcustom.c or include them when you link your application.
 */

extern void* flmalloc (size_t nBytes);
extern void  flfree (void *p);

#define FL_MALLOC(bytes)  flmalloc((size_t)(bytes))
#define FL_FREE(p)        flfree((p))


/*          Debug mode
 *
 * Uncomment the following lines if you want debug messages to be printed
 * out. Messages will be printed at initialization key points, and when
 * low-level errors occure.
 * You may choose to use 'printf' or provide your own routine.
 */

extern void  tffsSaveMsg(char *msg);   /* flsysvxw.c */
extern int fl_useDebug;                /* flcustom.c */

#define DEBUG_PRINT(str)  tffsSaveMsg str
#define DFORMAT_PRINT(str) {}


/*          Alignment of structure members
 *
 *
 *  Pack all the structures for now.
 */

/* should be defined in Makefile */


/*
 *          First IOCTL code
 *
 * When using TrueFFS IOCTL functions you have to define the code of the first TrueFFS IOCTL
 * function (after that the functions get consecutive increasing numbers). This numer should 
 * be out of the range of the standard IOCTL codes used by the operating system that is using 
 * the TrueFFS IOCTLs.
 *
 */

#define FL_IOCTL_START  0x9000  /* flvxwioctl.h */


/*
 *          32-bit memory routines
 *
 */

extern  void *  flcpy (void *dest, const void *src, size_t count);
extern  void *  flset (void *dest, int val, size_t count);
extern  int     flcmp (const void *dest, const void *src, size_t count);


/*
 * These definitions are not actually used, but must be present
 */

#define NAMING_CONVENTION 


/*
 * Overrides respective definitions in MTDs
 */

extern int  flMsecToYieldCPU;    /* flcustom.c */

#define YIELD_CPU  flMsecToYieldCPU


/*                                                                   
 * Uncomment the FL_NO_INIT_MMU_PAGES definition for:
 *                                                                        
 * In order to skip initialization of first and last byte of the given buffer.
 * When the user buffer resides on separated memory pages the read        
 * operation may cause a page fault. Some CPU's return from a page        
 * fault (after loading the new page) and reread the bytes that caused    
 * the page fault from the new loaded page. In order to prevent such a    
 * case the first and last bytes of the buffer are written.
 *                                                                        
 */

#define FL_NO_INIT_MMU_PAGES


#ifdef __cplusplus
  }
#endif




#endif /* FLSYSTEM_H */

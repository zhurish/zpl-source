/*
 * $Id: zassert.h,v 1.2 2004/12/03 18:01:04 ajs Exp $
 *
 * This file is part of Quagga.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _QUAGGA_ASSERT_H
#define _QUAGGA_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void _zlog_assert_failed (const char *assertion, const char *file,
				 zpl_uint32  line, const char *function)
				 __attribute__ ((noreturn));

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION    __func__
#endif
#elif defined(__GNUC__)
#ifndef __ASSERT_FUNCTION
#define __ASSERT_FUNCTION    __FUNCTION__
#endif
#else
#define __ASSERT_FUNCTION    NULL
#endif

#define zassert(EX) ((void)((EX) ?  0 :	\
			    (_zlog_assert_failed(#EX, __FILE__, __LINE__, \
						 __ASSERT_FUNCTION), 0)))

#define plzassert(EX) if((EX)){} else {	\
				   _zlog_assert_failed(#EX, __FILE__, __LINE__, \
						 __ASSERT_FUNCTION); }

#undef assert
#define assert(EX) zassert(EX)
#define zpl_assert(EX) zassert(EX)

#ifdef __cplusplus
}
#endif

#endif /* _QUAGGA_ASSERT_H */

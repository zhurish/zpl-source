/* jhash.h: Jenkins hash support.
 *
 * Copyright (C) 1996 Bob Jenkins (bob_jenkins@burtleburtle.net)
 *
 * http://burtleburtle.net/bob/hash/
 *
 * These are the credits from Bob's sources:
 *
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 * hash(), hash2(), hash3, and mix() are externally useful functions.
 * Routines to test the hash are included if SELF_TEST is defined.
 * You can use this free for any purpose.  It has no warranty.
 *
 * Copyright (C) 2003 David S. Miller (davem@redhat.com)
 *
 * I've modified Bob's hash to be useful in the Linux kernel, and
 * any bugs present are surely my fault.  -DaveM
 */

#ifndef __LIB_JHASH_H
#define __LIB_JHASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* The most generic version, hashes an arbitrary sequence
 * of bytes.  No alignment or length assumptions are made about
 * the input key.
 */
extern zpl_uint32 jhash(const void *key, zpl_uint32 length, zpl_uint32 initval);

/* A special optimized version that handles 1 or more of zpl_uint32s.
 * The length parameter here is the number of zpl_uint32s in the key.
 */
extern zpl_uint32 jhash2(const zpl_uint32 *k, zpl_uint32 length, zpl_uint32 initval);

/* A special ultra-optimized versions that knows they are hashing exactly
 * 3, 2 or 1 word(s).
 *
 * NOTE: In partilar the "c += length; __jhash_mix(a,b,c);" normally
 *       done at the end is not done here.
 */
extern zpl_uint32 jhash_3words(zpl_uint32 a, zpl_uint32 b, zpl_uint32 c, zpl_uint32 initval);
extern zpl_uint32 jhash_2words(zpl_uint32 a, zpl_uint32 b, zpl_uint32 initval);
extern zpl_uint32 jhash_1word(zpl_uint32 a, zpl_uint32 initval);
 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_JHASH_H */

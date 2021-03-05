/* Hash routine.
   Copyright (C) 1998 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2, or (at your
option) any later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _ZEBRA_HASH_H
#define _ZEBRA_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Default hash table size.  */ 
#define HASH_INITIAL_SIZE     256	/* initial number of backets. */
#define HASH_THRESHOLD	      10	/* expand when backet. */

struct hash_backet
{
  /* Linked list.  */
  struct hash_backet *next;

  /* Hash key. */
  ospl_uint32  key;

  /* Data.  */
  void *data;
};

struct hash
{
  /* Hash backet. */
  struct hash_backet **index;

  /* Hash table size. Must be power of 2 */
  ospl_uint32  size;

  /* If expansion failed. */
  ospl_uint32 no_expand;

  /* Key make function. */
  ospl_uint32  (*hash_key) (void *);

  /* Data compare function. */
  int (*hash_cmp) (const void *, const void *);

  /* Backet alloc. */
  ospl_ulong count;
};

extern struct hash *hash_create (ospl_uint32  (*) (void *), 
				 int (*) (const void *, const void *));
extern struct hash *hash_create_size (ospl_uint32 , ospl_uint32  (*) (void *), 
                                             int (*) (const void *, const void *));

extern void *hash_get (struct hash *, void *, void * (*) (void *));
extern void *hash_alloc_intern (void *);
extern void *hash_lookup (struct hash *, void *);
extern void *hash_release (struct hash *, void *);

extern void hash_iterate (struct hash *, 
		   void (*) (struct hash_backet *, void *), void *);

extern void hash_clean (struct hash *, void (*) (void *));
extern void hash_free (struct hash *);

extern ospl_uint32  string_hash_make (const char *);
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_HASH_H */

/* Memory management routine
   Copyright (C) 1998 Kunihiro Ishiguro

This file is part of GNU Zebra.

GNU Zebra is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Zebra is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Zebra; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef __ZMEMORY_H__
#define __ZMEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

#define array_size(ar) (sizeof(ar) / sizeof(ar[0]))

#define MEMLIST_NAME_LEN  32

/* For pretty printing of memory allocate information. */
struct memory_list
{
  zpl_int32 index;
  //const char *format;
  const char format[MEMLIST_NAME_LEN];
};

struct mlist {
  struct memory_list *list;
  const char *name;
};
 
#ifdef MEMORY_LOG
struct mstat
{
  const char *name;
  long alloc;
  zpl_ulong t_malloc;
  zpl_ulong c_malloc;
  zpl_ulong t_calloc;
  zpl_ulong c_calloc;
  zpl_ulong t_realloc;
  zpl_ulong t_free;
  zpl_ulong c_strdup;
};
#else 
struct mstat
{
  zpl_char *name;
  long alloc;
};
#endif


#include "memtypes.h"

extern struct mlist mlists[];
extern struct mstat mstat[MTYPE_MAX];

/* #define MEMORY_LOG */
#ifdef MEMORY_LOG
#define XMALLOC(mtype, size) \
  mtype_zmalloc (__FILE__, __LINE__, (mtype), (size))
#define XCALLOC(mtype, size) \
  mtype_zcalloc (__FILE__, __LINE__, (mtype), (size))
#define XREALLOC(mtype, ptr, size)  \
  mtype_zrealloc (__FILE__, __LINE__, (mtype), (ptr), (size))
#define XFREE(mtype, ptr) \
  do { \
    mtype_zfree (__FILE__, __LINE__, (mtype), (ptr)); \
    ptr = NULL; } \
  while (0)
#define XSTRDUP(mtype, str) \
  mtype_zstrdup (__FILE__, __LINE__, (mtype), (str))
#else
#define XMALLOC(mtype, size)       zmalloc ((mtype), (size))
#define XCALLOC(mtype, size)       zzcalloc ((mtype), (size))
#define XREALLOC(mtype, ptr, size) zrealloc ((mtype), (ptr), (size))
#define XFREE(mtype, ptr)          do { \
                                     zfree ((mtype), (ptr)); \
                                     ptr = NULL; } \
                                   while (0)
#define XSTRDUP(mtype, str)        zstrdup ((mtype), (str))
#endif /* MEMORY_LOG */

/* Prototypes of memory function. */
extern void *zmalloc (zpl_uint32 type, zpl_size_t size);
extern void *zzcalloc (zpl_uint32 type, zpl_size_t size);
extern void *z_zcalloc (zpl_uint32 type, zpl_uint32 n, zpl_size_t size);
extern void *zrealloc (zpl_uint32 type, void *ptr, zpl_size_t size);
extern void  zfree (zpl_uint32 type, void *ptr);
extern zpl_char *zstrdup (zpl_uint32 type, const char *str);
extern void *cjson_malloc (zpl_size_t size);
extern void cjson_free (void *ptr);

extern void *mtype_zmalloc (const char *file, zpl_uint32 line, zpl_uint32 type, zpl_size_t size);

extern void *mtype_zcalloc (const char *file, zpl_uint32 line, zpl_uint32 type, zpl_size_t size);

extern void *mtype_zrealloc (const char *file, zpl_uint32 line, zpl_uint32 type, void *ptr,
		             zpl_size_t size);

extern void mtype_zfree (const char *file, zpl_uint32 line, zpl_uint32 type,
		         void *ptr);

extern zpl_char *mtype_zstrdup (const char *file, zpl_uint32 line, zpl_uint32 type,
		            const char *str);
extern void memory_init (void);
extern void log_memstats_stderr (const char *);

/* return number of allocations outstanding for the type */
extern zpl_ulong mtype_stats_alloc (zpl_uint32);

/* Human friendly string for given byte count */
#define MTYPE_MEMSTR_LEN 20
extern const char *mtype_memstr (zpl_char *, zpl_size_t, zpl_ulong);

extern void cmd_memory_init (void);

 
#ifdef __cplusplus
}
#endif

#endif /* __ZMEMORY_H__ */

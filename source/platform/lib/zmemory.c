/*
 * Memory management routine
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "command.h"
#ifdef ZPL_OS_JSON
#include "cJSON.h"
#endif

/* malloc.h is generally obsolete, however GNU Libc mallinfo wants it. */
#if !defined(HAVE_STDLIB_H) || (defined(GNU_LINUX) && defined(HAVE_MALLINFO))
#include <malloc.h>
#endif /* !HAVE_STDLIB_H || HAVE_MALLINFO */

static void alloc_inc(zpl_uint32);
static void alloc_dec(zpl_uint32);
static void log_memstats(zpl_uint32 log_priority);

static const struct message mstr[] =
    {
        {MTYPE_THREAD, "thread"},
        {MTYPE_THREAD_MASTER, "thread_master"},
        {MTYPE_VECTOR, "vector"},
        {MTYPE_VECTOR_INDEX, "vector_index"},
        {MTYPE_IF, "interface"},
        {0, NULL},
};

/* Fatal memory allocation error occured. */
static void __attribute__((noreturn))
zerror(const char *fname, zpl_uint32 type, zpl_size_t size)
{
  zlog_err(MODULE_LIB, "%s : can't allocate memory for `%s' size %d: %s\n",
           fname, lookup(mstr, type), (zpl_uint32)size, ipstack_strerror(ipstack_errno));
  log_memstats(ZLOG_LEVEL_WARNING);
  /* N.B. It might be preferable to call zlog_backtrace_sigsafe here, since
     that function should definitely be safe in an OOM condition.  But
     unfortunately zlog_backtrace_sigsafe does not support syslog logging at
     this time... */
  zlog_backtrace(ZLOG_LEVEL_WARNING);
  abort();
}

/*
 * Allocate memory of a given size, to be tracked by a given type.
 * Effects: Returns a pointer to usable memory.  If memory cannot
 * be allocated, aborts execution.
 */
void *
zmalloc(zpl_uint32 type, zpl_size_t size)
{
  void *memory;

  memory = malloc(size);

  if (memory == NULL)
    zerror("malloc", type, size);
  os_memset(memory, 0, size);
  alloc_inc(type);

  return memory;
}

/*
 * Allocate memory as in zmalloc, and also clear the memory.
 * Add an extra 'z' prefix to function name to avoid collision when linking
 * statically with zlib that exports the 'zcalloc' symbol.
 */
void *
zzcalloc(zpl_uint32 type, zpl_size_t size)
{
  void *memory;

  memory = calloc(1, size);

  if (memory == NULL)
    zerror("calloc", type, size);
  os_memset(memory, 0, size);
  alloc_inc(type);

  return memory;
}

void *
z_zcalloc(zpl_uint32 type, zpl_uint32 n, zpl_size_t size)
{
  void *memory;

  memory = calloc(n, size);

  if (memory == NULL)
    zerror("calloc", type, size);
  os_memset(memory, 0, size);
  alloc_inc(type);
  //mstat[type].alloc++;
  return memory;
}
/* 
 * Given a pointer returned by zmalloc or zzcalloc, free it and
 * return a pointer to a new size, basically acting like realloc().
 * Requires: ptr was returned by zmalloc, zzcalloc, or zrealloc with the
 * same type.
 * Effects: Returns a pointer to the new memory, or aborts.
 */
void *
zrealloc(zpl_uint32 type, void *ptr, zpl_size_t size)
{
  void *memory;

  if (ptr == NULL) /* is really alloc */
    return zzcalloc(type, size);

  memory = realloc(ptr, size);
  if (memory == NULL)
    zerror("realloc", type, size);
  if (ptr == NULL)
    alloc_inc(type);

  return memory;
}

/*
 * Free memory allocated by z*alloc or zstrdup.
 * Requires: ptr was returned by zmalloc, zzcalloc, or zrealloc with the
 * same type.
 * Effects: The memory is freed and may no longer be referenced.
 */
void zfree(zpl_uint32 type, void *ptr)
{
  if (ptr != NULL)
  {
    alloc_dec(type);
    free(ptr);
  }
}

void *cjson_malloc(zpl_size_t size)
{
  return zmalloc(MTYPE_CJSON, size);
}
void cjson_free(void *ptr)
{
  zfree(MTYPE_CJSON, ptr);
}
/*
 * Duplicate a string, counting memory usage by type.
 * Effects: The string is duplicated, and the return value must
 * eventually be passed to zfree with the same type.  The function will
 * succeed or abort.
 */
zpl_char *
zstrdup(zpl_uint32 type, const char *str)
{
  void *dup;

  dup = strdup(str);
  if (dup == NULL)
    zerror("strdup", type, strlen(str));
  alloc_inc(type);
  return dup;
}

#ifdef MEMORY_LOG
struct mstat mstat[MTYPE_MAX];

static void
mtype_log(zpl_char *func, void *memory, const char *file, zpl_uint32 line, zpl_uint32 type)
{
  zlog_debug(MODULE_LIB, "%s: %s %p %s %d", func, lookup(mstr, type), memory, file, line);
}

void *
mtype_zmalloc(const char *file, zpl_uint32 line, zpl_uint32 type, zpl_size_t size)
{
  void *memory;

  mstat[type].c_malloc++;
  mstat[type].t_malloc++;

  memory = zmalloc(type, size);
  mtype_log("zmalloc", memory, file, line, type);

  return memory;
}

void *
mtype_zcalloc(const char *file, zpl_uint32 line, zpl_uint32 type, zpl_size_t size)
{
  void *memory;

  mstat[type].c_calloc++;
  mstat[type].t_calloc++;

  memory = zzcalloc(type, size);
  mtype_log("xcalloc", memory, file, line, type);

  return memory;
}

void *
mtype_zrealloc(const char *file, zpl_uint32 line, zpl_uint32 type, void *ptr, zpl_size_t size)
{
  void *memory;

  /* Realloc need before allocated pointer. */
  mstat[type].t_realloc++;

  memory = zrealloc(type, ptr, size);

  mtype_log("xrealloc", memory, file, line, type);

  return memory;
}

/* Important function. */
void mtype_zfree(const char *file, zpl_uint32 line, zpl_uint32 type, void *ptr)
{
  mstat[type].t_free++;

  mtype_log("xfree", ptr, file, line, type);

  zfree(type, ptr);
}

zpl_char *
mtype_zstrdup(const char *file, zpl_uint32 line, zpl_uint32 type, const char *str)
{
  zpl_char *memory;

  mstat[type].c_strdup++;

  memory = zstrdup(type, str);

  mtype_log("xstrdup", memory, file, line, type);

  return memory;
}
#else
struct mstat mstat[MTYPE_MAX];
#endif /* MEMORY_LOG */

/* Increment allocation counter. */
static void
alloc_inc(zpl_uint32 type)
{
  mstat[type].alloc++;
}

/* Decrement allocation counter. */
static void
alloc_dec(zpl_uint32 type)
{
  mstat[type].alloc--;
}

/* Looking up memory status from vty interface. */

static void
log_memstats(zpl_uint32 pri)
{
  struct mlist *ml;

  for (ml = mlists; ml->list; ml++)
  {
    struct memory_list *m;

    zlog(MODULE_LIB, pri, "Memory utilization in module %s:", ml->name);
    for (m = ml->list; m->index >= 0; m++)
      if (m->index && mstat[m->index].alloc && m->format)
        zlog(MODULE_LIB, pri, "  %-30s: %10ld", m->format, mstat[m->index].alloc);
  }
}

void log_memstats_stderr(const char *prefix)
{
  struct mlist *ml;
  struct memory_list *m;
  zpl_uint32 i;
  zpl_uint32 j = 0;

  for (ml = mlists; ml->list; ml++)
  {
    i = 0;

    for (m = ml->list; m->index >= 0; m++)
      if (m->index && mstat[m->index].alloc && m->format)
      {
        if (!i)
          fprintf(stderr,
                  "%s: memstats: Current memory utilization in module %s:\r\n",
                  prefix,
                  ml->name);
        fprintf(stderr,
                "%s: memstats:  %-30s: %10ld%s\r\n",
                prefix,
                m->format,
                mstat[m->index].alloc,
                mstat[m->index].alloc < 0 ? " (REPORT THIS BUG!)" : "");
        i = j = 1;
      }
  }

  if (j)
    fprintf(stderr,
            "%s: memstats: NOTE: If configuration exists, utilization may be "
            "expected.\r\n",
            prefix);
  else
    fprintf(stderr,
            "%s: memstats: No remaining tracked memory utilization.\r\n",
            prefix);
}


void memory_init(void)
{
  #ifdef ZPL_OS_JSON
  cJSON_Hooks cJSONhooks;
  cJSONhooks.malloc_fn = cjson_malloc;
  cJSONhooks.free_fn = cjson_free;
  cJSON_InitHooks(&cJSONhooks);
  #endif
  memset(mstat, 0, sizeof(mstat));
}

/* Stats querying from users */
/* Return a pointer to a human friendly string describing
 * the byte count passed in. E.g:
 * "0 bytes", "2048 bytes", "110kB", "500MiB", "11GiB", etc.
 * Up to 4 significant figures will be given.
 * The pointer returned may be NULL (indicating an error)
 * or point to the given buffer, or point to static storage.
 */
const char *
mtype_memstr(zpl_char *buf, zpl_size_t len, zpl_ulong bytes)
{
  zpl_uint32 m, k;

  /* easy cases */
  if (!bytes)
    return "0 bytes";
  if (bytes == 1)
    return "1 byte";

  /*
   * When we pass the 2gb barrier mallinfo() can no longer report
   * correct data so it just does something odd...
   * Reporting like Terrabytes of data.  Which makes users...
   * edgy.. yes edgy that's the term for it.
   * So let's just give up gracefully
   */
  if (bytes > 0x7fffffff)
    return "> 2GB";

  m = bytes >> 20;
  k = bytes >> 10;

  if (m > 10)
  {
    if (bytes & (1 << 19))
      m++;
    snprintf(buf, len, "%d MiB", m);
  }
  else if (k > 10)
  {
    if (bytes & (1 << 9))
      k++;
    snprintf(buf, len, "%d KiB", k);
  }
  else
    snprintf(buf, len, "%ld bytes", bytes);

  return buf;
}

zpl_ulong
mtype_stats_alloc(zpl_uint32 type)
{
  return mstat[type].alloc;
}

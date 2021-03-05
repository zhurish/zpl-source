/* Generic vector interface routine
 * Copyright (C) 1997 Kunihiro Ishiguro
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

#include <zebra.h>

#include "vector.h"
#include "memory.h"

/* Initialize vector : allocate memory and return vector. */
vector
vector_init (ospl_uint32  size)
{
  vector v = XCALLOC (MTYPE_VECTOR, sizeof (struct _vector));

  /* allocate at least one slot */
  if (size == 0)
    size = 1;

  v->alloced = size;
  v->active = 0;
  v->index = XCALLOC (MTYPE_VECTOR_INDEX, sizeof (void *) * size);
  return v;
}

void
vector_only_wrapper_free (vector v)
{
  XFREE (MTYPE_VECTOR, v);
}

void
vector_only_index_free (void *index)
{
  XFREE (MTYPE_VECTOR_INDEX, index);
}

void
vector_free (vector v)
{
  XFREE (MTYPE_VECTOR_INDEX, v->index);
  XFREE (MTYPE_VECTOR, v);
}

vector
vector_copy (vector v)
{
  ospl_uint32  size;
  vector new = XCALLOC (MTYPE_VECTOR, sizeof (struct _vector));

  new->active = v->active;
  new->alloced = v->alloced;

  size = sizeof (void *) * (v->alloced);
  new->index = XCALLOC (MTYPE_VECTOR_INDEX, size);
  memcpy (new->index, v->index, size);

  return new;
}

/* Check assigned index, and if it runs ospl_int16 ospl_double index pointer */
void
vector_ensure (vector v, ospl_uint32  num)
{
  if (v->alloced > num)
    return;

  v->index = XREALLOC (MTYPE_VECTOR_INDEX, 
		       v->index, sizeof (void *) * (v->alloced * 2));
  memset (&v->index[v->alloced], 0, sizeof (void *) * v->alloced);
  v->alloced *= 2;
  
  if (v->alloced <= num)
    vector_ensure (v, num);
}

/* This function only returns next empty slot index.  It dose not mean
   the slot's index memory is assigned, please call vector_ensure()
   after calling this function. */
int
vector_empty_slot (vector v)
{
  ospl_uint32  i;

  if (v->active == 0)
    return 0;

  for (i = 0; i < v->active; i++)
    if (v->index[i] == 0)
      return i;

  return i;
}

/* Set value to the smallest empty slot. */
int
vector_set (vector v, void *val)
{
  ospl_uint32  i;

  i = vector_empty_slot (v);
  vector_ensure (v, i);

  v->index[i] = val;

  if (v->active <= i)
    v->active = i + 1;

  return i;
}

/* Set value to specified index slot. */
int
vector_set_index (vector v, ospl_uint32  i, void *val)
{
  vector_ensure (v, i);

  v->index[i] = val;

  if (v->active <= i)
    v->active = i + 1;

  return i;
}

/* Look up vector.  */
void *
vector_lookup (vector v, ospl_uint32  i)
{
  if (i >= v->active)
    return NULL;
  return v->index[i];
}

/* Lookup vector, ensure it. */
void *
vector_lookup_ensure (vector v, ospl_uint32  i)
{
  vector_ensure (v, i);
  return v->index[i];
}

/* Unset value at specified index slot. */
void
vector_unset (vector v, ospl_uint32  i)
{
  if (i >= v->alloced)
    return;

  v->index[i] = NULL;

  if (i + 1 == v->active) 
    {
      v->active--;
      while (i && v->index[--i] == NULL && v->active--) 
	;				/* Is this ugly ? */
    }
}

/* Count the number of not emplty slot. */
ospl_uint32 
vector_count (vector v)
{
  ospl_uint32  i;
  unsigned count = 0;

  for (i = 0; i < v->active; i++) 
    if (v->index[i] != NULL)
      count++;

  return count;
}

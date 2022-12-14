/* Priority queue functions.
   Copyright (C) 2003 Yasuhiro Ohara

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

#ifndef __LIB_PQUEUE_H
#define __LIB_PQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

struct pqueue
{
  void **array;
  zpl_uint32 array_size;
  zpl_uint32 size;

  int (*cmp) (void *, void *);
  void (*update) (void * node, zpl_uint32 actual_position);
};

#define PQUEUE_INIT_ARRAYSIZE  32

extern struct pqueue *pqueue_create (void);
extern void pqueue_delete (struct pqueue *queue);

extern void pqueue_enqueue (void *data, struct pqueue *queue);
extern void *pqueue_dequeue (struct pqueue *queue);
extern void pqueue_remove_at (zpl_uint32 index, struct pqueue *queue);

extern void trickle_down (zpl_uint32 index, struct pqueue *queue);
extern void trickle_up (zpl_uint32 index, struct pqueue *queue);

extern zpl_bool pqueue_empty (struct pqueue *queue);
 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_PQUEUE_H */

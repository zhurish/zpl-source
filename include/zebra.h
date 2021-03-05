/* Zebra common header.
   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002 Kunihiro Ishiguro

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

#ifndef _ZEBRA_H
#define _ZEBRA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "os_definc.h"
#include "ospl_type.h"
#include "ospl_def.h"
#include "ospl_ret.h"

#include "rtplat_cfg.h"


#include "os_memory.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_job.h"
#include "os_task.h"
#include "os_time.h"
#include "os_util.h"
#include "os_socket.h"
#include "os_tlv.h"
#include "os_ansync.h"
#include "os_queue.h"
#include "os_rng.h"
#include "cJSON.h"
#include "avl.h"

#include "list.h"
#include "dlst.h"
#include "list_tree.h"

#include "zassert.h"
#include "str.h"
#include "bitmap.h"

#include "module.h"


#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_H */

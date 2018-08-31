/*
 * cmd_memory.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */

#include "zebra.h"
/* malloc.h is generally obsolete, however GNU Libc mallinfo wants it. */
#if !defined(HAVE_STDLIB_H) || (defined(GNU_LINUX) && defined(HAVE_MALLINFO))
#include <malloc.h>
#endif /* !HAVE_STDLIB_H || HAVE_MALLINFO */

#include "log.h"
#include "memory.h"
#include "vector.h"
#include "vty.h"
#include "command.h"



DEFUN (show_memory,
       show_memory_cmd,
       "show memory",
       "Show running system information\n"
       "Memory statistics\n")
{
  return vty_show_memory_cmd(vty);
}


void
cmd_memory_init (void)
{
/*  install_element (RESTRICTED_NODE, &show_memory_cmd);*/

  install_element (VIEW_NODE, &show_memory_cmd);
}

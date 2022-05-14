/*
 * cmd_memory.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "command.h"
#include "vty.h"


static void show_memory_info_separator(struct vty *vty)
{
  vty_out(vty, "-----------------------------%s",VTY_NEWLINE);
}

static int show_memory_info_detail(struct vty *vty, struct memory_list *ml)
{
  struct memory_list *m = NULL;
  zpl_uint32 needsep = 0;
  zpl_char bufstr[MEMLIST_NAME_LEN];
  for (m = ml; m->index >= 0; m++)
  {
    if (m->index == 0)
    {
      if (needsep)
      {
        show_memory_info_separator(vty);
        needsep = 0;
      }
    }
    if (m->index > 0 && mstat[m->index].alloc && m->format != NULL)
    {
      //vty_out(vty, "%-30s: %10ld %s", m->format, mstat[m->index].alloc, VTY_NEWLINE);
	    memset(bufstr, '\0', sizeof(bufstr));
	    strncpy(bufstr, m->format, MEMLIST_NAME_LEN);
	    vty_out(vty, "%-30s: %10ld %s", bufstr, mstat[m->index].alloc, VTY_NEWLINE);
      needsep = 1;
    }
  }
  return needsep;
}

#ifdef HAVE_MALLINFO
static int show_memory_mallinfo(struct vty *vty)
{
  struct mallinfo minfo = mallinfo();
  zpl_char buf[MTYPE_MEMSTR_LEN];

  vty_out(vty, "System allocator statistics:%s", VTY_NEWLINE);
  vty_out(vty, "  Total heap allocated:  %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.arena),
          VTY_NEWLINE);
  vty_out(vty, "  Holding block headers: %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.hblkhd),
          VTY_NEWLINE);
  vty_out(vty, "  Used small blocks:     %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.usmblks),
          VTY_NEWLINE);
  vty_out(vty, "  Used ordinary blocks:  %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.uordblks),
          VTY_NEWLINE);
  vty_out(vty, "  Free small blocks:     %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.fsmblks),
          VTY_NEWLINE);
  vty_out(vty, "  Free ordinary blocks:  %s%s",
          mtype_memstr(buf, MTYPE_MEMSTR_LEN, minfo.fordblks),
          VTY_NEWLINE);
  vty_out(vty, "  Ordinary blocks:       %ld%s",
          (zpl_ulong)minfo.ordblks,
          VTY_NEWLINE);
  vty_out(vty, "  Small blocks:          %ld%s",
          (zpl_ulong)minfo.smblks,
          VTY_NEWLINE);
  vty_out(vty, "  Holding blocks:        %ld%s",
          (zpl_ulong)minfo.hblks,
          VTY_NEWLINE);
  vty_out(vty, "(see system documentation for 'mallinfo' for meaning)%s",
          VTY_NEWLINE);
  return 1;
}
#endif /* HAVE_MALLINFO */




DEFUN (show_memory_info,
       show_memory_info_cmd,
       "show memory",
       "Show running system information\n"
       "Memory statistics\n")
{
  struct mlist *ml = NULL;
  zpl_uint32 needsep = 0;

#ifdef HAVE_MALLINFO
  needsep = show_memory_mallinfo(vty);
#endif /* HAVE_MALLINFO */

  for (ml = mlists; ml->list; ml++)
  {
    if (needsep)
      show_memory_info_separator(vty);
    if(ml && ml->list)  
      needsep = show_memory_info_detail(vty, ml->list);
  }

  return CMD_SUCCESS;
}

DEFUN (show_system_clock,
		show_system_clock_cmd,
		"show system clock",
		SHOW_STR
		"Displays system information\n"
		"Displays Clock information\n")
{
	vty_out(vty, "%s", VTY_NEWLINE);
	vty_out(vty, "system current time  : %s %s", os_time_fmt("/",os_time (NULL)), VTY_NEWLINE);
	vty_out(vty, "system running time  : %s %s", os_time_string(os_monotonic_time ()), VTY_NEWLINE);
	vty_out(vty, "system hw-clock time : %s %s", os_time_fmt("/",os_monotonic_time ()), VTY_NEWLINE);

	if(argc == 1)
	{
		struct timeval tv;
		vty_out(vty, "os time              : %u %s", os_time (NULL), VTY_NEWLINE);
		vty_out(vty, "os monotonic time    : %u %s", os_monotonic_time (), VTY_NEWLINE);
		os_gettimeofday (&tv);
		vty_out(vty, "os timeofday         : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
		os_get_realtime (&tv);
		vty_out(vty, "os realtime          : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
		os_get_monotonic (&tv);
		vty_out(vty, "os monotonic         : %u.%u %s", tv.tv_sec, tv.tv_usec/1000, VTY_NEWLINE);
	}
	vty_out(vty, "%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}

ALIAS_HIDDEN(show_system_clock,
		show_system_clock_detal_cmd,
		"show system clock (detail|)",
		SHOW_STR
		"Displays system information\n"
		"Displays Clock information\n"
		"Dtail information\n");


void
cmd_memory_init (void)
{
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_memory_info_cmd);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_system_clock_cmd);
  install_element (VIEW_NODE, CMD_VIEW_LEVEL, &show_system_clock_detal_cmd);
}

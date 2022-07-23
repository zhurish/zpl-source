/*
 * cmd_log.c
 *
 *  Created on: Jan 2, 2018
 *      Author: zhurish
 */




#include "auto_include.h"
#include <zplos_include.h>
#include "if.h"
#include "command.h"
#include "prefix.h"
#include "template.h"
#include "vty.h"
#include "nsm_debug.h"

/* Debug node. */
static struct cmd_node debug_node =
{
  DEBUG_NODE,
  "",				/* Debug node has no interface. */
  1
};

DEFUN (show_debuging_config,
	   show_debuging_config_cmd,
		"show debugging",
		SHOW_STR
		 "Debugging information\n")
{
	nsm_template_debug_show_config (vty, zpl_true);
	return CMD_SUCCESS;
}


#ifdef ZPL_NSM_MODULE
static int config_nsm_write_debug (struct vty *vty)
{
  int write = 0;

  if (IS_NSM_DEBUG_EVENT)
    {
      vty_out (vty, "debug zebra events%s", VTY_NEWLINE);
      write++;
    }
  if (IS_NSM_DEBUG_PACKET)
    {
      if (IS_NSM_DEBUG_SEND && IS_NSM_DEBUG_RECV)
	{
	  vty_out (vty, "debug zebra packet%s%s",
		   IS_NSM_DEBUG_DETAIL ? " detail" : "",
		   VTY_NEWLINE);
	  write++;
	}
      else
	{
	  if (IS_NSM_DEBUG_SEND)
	    vty_out (vty, "debug zebra packet send%s%s",
		     IS_NSM_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  else
	    vty_out (vty, "debug zebra packet recv%s%s",
		     IS_NSM_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  write++;
	}
    }
  if (IS_NSM_DEBUG_KERNEL)
    {
      vty_out (vty, "debug zebra kernel%s", VTY_NEWLINE);
      write++;
    }
  if (IS_NSM_DEBUG_RIB)
    {
      vty_out (vty, "debug zebra rib%s", VTY_NEWLINE);
      write++;
    }
  if (IS_NSM_DEBUG_RIB_Q)
    {
      vty_out (vty, "debug zebra rib queue%s", VTY_NEWLINE);
      write++;
    }

  //extern int ospf_config_write_debug (struct vty *vty);
  //ospf_config_write_debug (vty);
//  extern void show_debug_hook_all(struct vty *);
//  show_debug_hook_all(vty);
  vty_out (vty, "!%s", VTY_NEWLINE);
  return write;
}
#endif

static int config_write_debug (struct vty *vty)
{
  #ifdef ZPL_NSM_MODULE
	config_nsm_write_debug (vty);
  #endif
	nsm_template_debug_write_config (vty);
	return OK;
}


int cmd_debug_init(void)
{
	install_node (&debug_node, config_write_debug);

	nsm_debug_init ();

	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_debuging_config_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_debuging_config_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_debuging_config_cmd);
	return OK;
}

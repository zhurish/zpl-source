/*
 * Zebra debug related function
 * Copyright (C) 1999 Kunihiro Ishiguro
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "nsm_debug.h"
#include "zmemory.h"
#include "if.h"
#include "vty.h"
#include "command.h"

/* For debug statement. */
zpl_ulong nsm_debug_event;
zpl_ulong nsm_debug_packet = 0xffffff;
zpl_ulong nsm_debug_kernel = 0xffffff;
zpl_ulong nsm_debug_rib = 0xffffff;
zpl_ulong nsm_debug_nht;

#ifdef ZPL_SHELL_MODULE
DEFUN(show_debugging_nsm,
      show_debugging_nsm_cmd,
      "show debugging zebra",
      SHOW_STR
      "Debugging information\n"
      "Zebra configuration\n")
{
    vty_out(vty, "Zebra debugging status:%s", VTY_NEWLINE);

    if (IS_NSM_DEBUG_EVENT)
        vty_out(vty, "  Zebra event debugging is on%s", VTY_NEWLINE);

    if (IS_NSM_DEBUG_PACKET)
    {
        if (IS_NSM_DEBUG_SEND && IS_NSM_DEBUG_RECV)
        {
            vty_out(vty, "  Zebra packet%s debugging is on%s",
                    IS_NSM_DEBUG_DETAIL ? " detail" : "",
                    VTY_NEWLINE);
        }
        else
        {
            if (IS_NSM_DEBUG_SEND)
                vty_out(vty, "  Zebra packet send%s debugging is on%s",
                        IS_NSM_DEBUG_DETAIL ? " detail" : "",
                        VTY_NEWLINE);
            else
                vty_out(vty, "  Zebra packet receive%s debugging is on%s",
                        IS_NSM_DEBUG_DETAIL ? " detail" : "",
                        VTY_NEWLINE);
        }
    }

    if (IS_NSM_DEBUG_KERNEL)
        vty_out(vty, "  Zebra kernel debugging is on%s", VTY_NEWLINE);

    if (IS_NSM_DEBUG_RIB)
        vty_out(vty, "  Zebra RIB debugging is on%s", VTY_NEWLINE);
    if (IS_NSM_DEBUG_RIB_Q)
        vty_out(vty, "  Zebra RIB queue debugging is on%s", VTY_NEWLINE);

    if (IS_NSM_DEBUG_NHT)
        vty_out(vty, "  Zebra next-hop tracking debugging is on%s", VTY_NEWLINE);

    return CMD_SUCCESS;
}

DEFUN(debug_nsm_events,
      debug_nsm_events_cmd,
      "debug zebra events",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra events\n")
{
    nsm_debug_event = NSM_DEBUG_EVENT;
    return CMD_WARNING;
}

DEFUN(debug_nsm_nht,
      debug_nsm_nht_cmd,
      "debug zebra nht",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra next hop tracking\n")
{
    nsm_debug_nht = NSM_DEBUG_NHT;
    return CMD_WARNING;
}

DEFUN(debug_nsm_packet,
      debug_nsm_packet_cmd,
      "debug zebra packet",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra packet\n")
{
    nsm_debug_packet = NSM_DEBUG_PACKET;
    SET_FLAG(nsm_debug_packet, NSM_DEBUG_SEND);
    SET_FLAG(nsm_debug_packet, NSM_DEBUG_RECV);
    return CMD_SUCCESS;
}

DEFUN(debug_nsm_packet_direct,
      debug_nsm_packet_direct_cmd,
      "debug zebra packet (recv|send|detail)",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra packet\n"
      "Debug option set for receive packet\n"
      "Debug option set for send packet\n")
{
    nsm_debug_packet = NSM_DEBUG_PACKET;
    if (strncmp("send", argv[0], strlen(argv[0])) == 0)
        SET_FLAG(nsm_debug_packet, NSM_DEBUG_SEND);
    if (strncmp("recv", argv[0], strlen(argv[0])) == 0)
        SET_FLAG(nsm_debug_packet, NSM_DEBUG_RECV);
    if (strncmp("detail", argv[0], strlen(argv[0])) == 0)
        SET_FLAG(nsm_debug_packet, NSM_DEBUG_DETAIL);
    return CMD_SUCCESS;
}

DEFUN(debug_nsm_packet_detail,
      debug_nsm_packet_detail_cmd,
      "debug zebra packet (recv|send) detail",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra packet\n"
      "Debug option set for receive packet\n"
      "Debug option set for send packet\n"
      "Debug option set detailed information\n")
{
    nsm_debug_packet = NSM_DEBUG_PACKET;
    if (strncmp("send", argv[0], strlen(argv[0])) == 0)
        SET_FLAG(nsm_debug_packet, NSM_DEBUG_SEND);
    if (strncmp("recv", argv[0], strlen(argv[0])) == 0)
        SET_FLAG(nsm_debug_packet, NSM_DEBUG_RECV);
    SET_FLAG(nsm_debug_packet, NSM_DEBUG_DETAIL);
    return CMD_SUCCESS;
}

DEFUN(debug_nsm_kernel,
      debug_nsm_kernel_cmd,
      "debug zebra kernel",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra between kernel interface\n")
{
    nsm_debug_kernel = NSM_DEBUG_KERNEL;
    return CMD_SUCCESS;
}

DEFUN(debug_nsm_rib,
      debug_nsm_rib_cmd,
      "debug zebra rib",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug RIB events\n")
{
    SET_FLAG(nsm_debug_rib, NSM_DEBUG_RIB);
    return CMD_SUCCESS;
}

DEFUN(debug_nsm_rib_q,
      debug_nsm_rib_q_cmd,
      "debug zebra rib queue",
      DEBUG_STR
      "Zebra configuration\n"
      "Debug RIB events\n"
      "Debug RIB queueing\n")
{
    SET_FLAG(nsm_debug_rib, NSM_DEBUG_RIB_Q);
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_events,
      no_debug_nsm_events_cmd,
      "no debug zebra events",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra events\n")
{
    nsm_debug_event = 0;
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_nht,
      no_debug_nsm_nht_cmd,
      "no debug zebra nht",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra next hop tracking\n")
{
    nsm_debug_nht = 0;
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_packet,
      no_debug_nsm_packet_cmd,
      "no debug zebra packet",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra packet\n")
{
    nsm_debug_packet = 0;
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_packet_direct,
      no_debug_nsm_packet_direct_cmd,
      "no debug zebra packet (recv|send)",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra packet\n"
      "Debug option set for receive packet\n"
      "Debug option set for send packet\n")
{
    if (strncmp("send", argv[0], strlen(argv[0])) == 0)
        UNSET_FLAG(nsm_debug_packet, NSM_DEBUG_SEND);
    if (strncmp("recv", argv[0], strlen(argv[0])) == 0)
        UNSET_FLAG(nsm_debug_packet, NSM_DEBUG_RECV);
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_kernel,
      no_debug_nsm_kernel_cmd,
      "no debug zebra kernel",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug option set for zebra between kernel interface\n")
{
    nsm_debug_kernel = 0;
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_rib,
      no_debug_nsm_rib_cmd,
      "no debug zebra rib",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug zebra RIB\n")
{
    nsm_debug_rib = 0;
    return CMD_SUCCESS;
}

DEFUN(no_debug_nsm_rib_q,
      no_debug_nsm_rib_q_cmd,
      "no debug zebra rib queue",
      NO_STR
          DEBUG_STR
      "Zebra configuration\n"
      "Debug zebra RIB\n"
      "Debug RIB queueing\n")
{
    UNSET_FLAG(nsm_debug_rib, NSM_DEBUG_RIB_Q);
    return CMD_SUCCESS;
}

#endif

int nsm_debug_init(void)
{
    nsm_debug_event = 0;
    nsm_debug_packet = 0;
    nsm_debug_kernel = 0;
    nsm_debug_rib = 0;

#ifdef ZPL_SHELL_MODULE
    // install_node (&debug_node, config_write_debug);

    install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_debugging_nsm_cmd);

    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_events_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_nht_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_direct_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_detail_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_kernel_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_rib_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_nsm_rib_q_cmd);

    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_events_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_nht_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_packet_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_kernel_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_rib_cmd);
    install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_rib_q_cmd);

    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_events_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_nht_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_direct_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_packet_detail_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_kernel_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_rib_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_nsm_rib_q_cmd);

    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_events_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_nht_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_packet_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_kernel_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_rib_cmd);
    install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_nsm_rib_q_cmd);

#endif
    return OK;
}

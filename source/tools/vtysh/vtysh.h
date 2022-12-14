/* Virtual terminal interface shell.
 * Copyright (C) 2000 Kunihiro Ishiguro
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

#ifndef VTYSH_H
#define VTYSH_H


#define VTYSH_ALL	  0xff

/* vtysh local configuration file. */
#define VTYSH_DEFAULT_CONFIG "vtysh.conf"


struct vtysh_user
{
  char *name;
  u_char nopassword;
};

struct vtysh_client
{
  zpl_socket_t fd;
  char name[64];
  int flag;
  char path[64];

  int complete_status;
  struct vty *vty;
  struct list *userlist;
};

extern struct vtysh_client vtysh_client;



extern void vtysh_init (void);
extern void vtysh_cmd_init (void);
extern int vtysh_connect (const char *optional_daemon_name);
extern int vtysh_execute (const char *);

extern char *vtysh_prompt (void);



#endif /* VTYSH_H */

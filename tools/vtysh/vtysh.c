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

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"

#include <readline/readline.h>
#include <readline/history.h>
#include "vtysh.h"

struct vtysh_client vtysh_client;

/* We don't care about the point of the cursor when '?' is typed. */
static int vtysh_rl_describe(void)
{
  int ret;
  unsigned int i;
  vector vline;
  vector describe;
  int width;
  struct cmd_token *token;

  vline = cmd_make_strvec(rl_line_buffer);

  /* In case of '> ?'. */
  if (vline == NULL)
  {
    vline = vector_init(1);
    vector_set(vline, NULL);
  }
  else if (rl_end && isspace((int)rl_line_buffer[rl_end - 1]))
    vector_set(vline, NULL);

  describe = cmd_describe_command(vline, vtysh_client.vty, &ret);

  fprintf(stdout, "\n");

  /* Ambiguous and no match error. */
  switch (ret)
  {
  case CMD_ERR_AMBIGUOUS:
    cmd_free_strvec(vline);
    fprintf(stdout, "%% Ambiguous command.\n");
    rl_on_new_line();
    return 0;
    break;
  case CMD_ERR_NO_MATCH:
    cmd_free_strvec(vline);
    fprintf(stdout, "%% There is no matched command.\n");
    rl_on_new_line();
    return 0;
    break;
  }

  /* Get width of command string. */
  width = 0;
  for (i = 0; i < vector_active(describe); i++)
    if ((token = vector_slot(describe, i)) != NULL)
    {
      int len;

      if (token->cmd[0] == '\0')
        continue;

      len = strlen(token->cmd);
      if (token->cmd[0] == '.')
        len--;

      if (width < len)
        width = len;
    }

  for (i = 0; i < vector_active(describe); i++)
    if ((token = vector_slot(describe, i)) != NULL)
    {
      if (token->cmd[0] == '\0')
        continue;

      if (!token->desc)
        fprintf(stdout, "  %-s\n",
                token->cmd[0] == '.' ? token->cmd + 1 : token->cmd);
      else
        fprintf(stdout, "  %-*s  %s\n",
                width,
                token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
                token->desc);
    }

  cmd_free_strvec(vline);
  vector_free(describe);

  rl_on_new_line();

  return 0;
}

/* Result of cmd_complete_command() call will be stored here
 * and used in new_completion() in order to put the space in
 * correct places only. */
static char *command_generator(const char *text, int state)
{
  vector vline;
  static char **matched = NULL;
  static int index = 0;

  /* First call. */
  if (!state)
  {
    index = 0;

    if (vtysh_client.vty->node == AUTH_NODE || vtysh_client.vty->node == AUTH_ENABLE_NODE)
      return NULL;

    vline = cmd_make_strvec(rl_line_buffer);
    if (vline == NULL)
      return NULL;

    if (rl_end && isspace((int)rl_line_buffer[rl_end - 1]))
      vector_set(vline, NULL);

    matched = cmd_complete_command(vline, vtysh_client.vty, &vtysh_client.complete_status);
  }

  if (matched && matched[index])
    return matched[index++];

  return NULL;
}

static char **new_completion(char *text, int start, int end)
{
  char **matches;

  matches = rl_completion_matches(text, command_generator);

  if (matches)
  {
    rl_point = rl_end;
    if (vtysh_client.complete_status != CMD_COMPLETE_FULL_MATCH)
      /* only append a space on full match */
      rl_completion_append_character = '\0';
  }

  return matches;
}

/* Making connection to protocol daemon. */
static int vtysh_connect_node(struct vtysh_client *vclient)
{
  int ret;
  int sock, len;
  struct sockaddr_un addr;
  struct stat s_stat;

  /* Stat socket to see if we have permission to access it. */
  ret = stat(vclient->path, &s_stat);
  if (ret < 0 && errno != ENOENT)
  {
    fprintf(stderr, "vtysh_connect(%s): stat = %s\n",
            vclient->path, strerror(errno));
    exit(1);
  }

  if (ret >= 0)
  {
    if (!S_ISSOCK(s_stat.st_mode))
    {
      fprintf(stderr, "vtysh_connect(%s): Not a socket\n",
              vclient->path);
      exit(1);
    }
  }

  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
  {
#ifdef DEBUG
    fprintf(stderr, "vtysh_connect(%s): socket = %s\n", vclient->path,
            strerror(errno));
#endif /* DEBUG */
    return -1;
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, vclient->path, strlen(vclient->path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

  ret = connect(sock, (struct sockaddr *)&addr, len);
  if (ret < 0)
  {
#ifdef DEBUG
    fprintf(stderr, "vtysh_connect(%s): connect = %s\n", vclient->path,
            strerror(errno));
#endif /* DEBUG */
    close(sock);
    return -1;
  }
  vclient->fd = sock;
  vclient->flag = 1;
  return 0;
}

static int vtysh_read_result(struct vtysh_client *vclient)
{
  // int ret;
  zpl_socket_t sock;
  zpl_uint32 nbytes = 0, already = 0;
  zpl_uchar *buf = NULL;
  vtysh_result_t msg;
  sock.stack = OS_STACK;
  sock._fd = vclient->fd;
  zpl_osmsg_reset(vclient->vty->vtysh_msg);
  while (1)
  {
    already = zpl_osmsg_get_endp(vclient->vty->vtysh_msg);
    nbytes = zpl_osmsg_read_try(vclient->vty->vtysh_msg, sock, sizeof(vtysh_result_t) - already);
    if (nbytes == -1)
    {
      zpl_osmsg_reset(vclient->vty->vtysh_msg);
      return -1;
    }
    if (nbytes == (ssize_t)(sizeof(vtysh_msghdr_t) - already))
    {
      already = sizeof(vtysh_msghdr_t);
      break;
    }
  }

  /* Reset to read from the beginning of the incoming packet. */
  zpl_osmsg_set_getp(vclient->vty->vtysh_msg, 0);

  /* Fetch header values */
  msg.type = zpl_osmsg_getl(vclient->vty->vtysh_msg);
  msg.retcode = zpl_osmsg_getl(vclient->vty->vtysh_msg);
  msg.retlen = zpl_osmsg_getl(vclient->vty->vtysh_msg);

  if (msg.retlen > zpl_osmsg_get_size(vclient->vty->vtysh_msg))
  {
    zpl_osmsg_reset(vclient->vty->vtysh_msg);
    return -1;
  }

  /* Read rest of data. */
  if (already < msg.retlen)
  {
    while (1)
    {
      ssize_t nbyte;
      if (((nbyte = zpl_osmsg_read_try(vclient->vty->vtysh_msg, sock,
                                       msg.retlen - already)) == 0) ||
          (nbyte == -1))
      {
        zpl_osmsg_reset(vclient->vty->vtysh_msg);
        return -1;
      }
      if (nbyte == (ssize_t)(msg.retlen - already))
      {
        break;
      }
    }
  }
  buf = zpl_osmsg_pnt(vclient->vty->vtysh_msg);
  if (msg.retcode == CMD_SUCCESS)
    return 0;
  else
  {
    if (strlen(buf))
    {
      fprintf(stdout, "%s", buf);
      fflush(stdout);
    }
  }
  return 0;
}

static int vtysh_write_cmd(const char *cmd)
{
  int rc = 0, mlen = 0, offset = 0;
  char cmdbuf[1024];
  vtysh_msghdr_t *vtyshhdr = (vtysh_msghdr_t *)cmdbuf;
  memset(cmdbuf, 0, sizeof(cmdbuf));
  vtyshhdr->type = htonl(VTYSH_MSG_CMD);
  vtyshhdr->msglen = htonl(strlen(cmd));
  mlen = sizeof(vtysh_msghdr_t) + strlen(cmd);
  while (vtysh_client.fd)
  {
    rc = write(vtysh_client.fd, cmdbuf + offset, mlen - offset);
    if (rc == mlen)
    {
      break;
    }
    if (rc <= 0)
    {
      break;
    }
    else
      offset += rc;
  }
  rc = vtysh_read_result(&vtysh_client);
  return rc;
}

int vtysh_connect(const char *daemon_name)
{
  int rc = 0;
  strcpy(vtysh_client.name, "zebra");
  // vtysh_client.flag;
  strcpy(vtysh_client.path, daemon_name);
  rc = vtysh_connect_node(&vtysh_client);
  return rc;
}

/* To disable readline's filename completion. */
static char *
vtysh_completion_entry_function(const char *ignore, int invoking_key)
{
  return NULL;
}

static void vtysh_readline_init(void)
{
  rl_initialize();
  /* readline related settings. */
  rl_bind_key('?', (rl_command_func_t *)vtysh_rl_describe);
  rl_completion_entry_function = vtysh_completion_entry_function;
  rl_attempted_completion_function = (rl_completion_func_t *)new_completion;
}
/*
extern struct cmd_element config_end_cmd;
extern struct cmd_element config_exit_cmd;
extern struct cmd_element config_quit_cmd;
extern struct cmd_element config_help_cmd;
extern struct cmd_element config_list_cmd;
extern struct cmd_element config_end_cmd;
extern struct cmd_element config_exit_cmd;
extern struct cmd_element config_quit_cmd;
extern struct cmd_element config_help_cmd;
extern struct cmd_element config_list_cmd;
extern struct cmd_element config_write_terminal_cmd;
extern struct cmd_element config_write_file_cmd;
extern struct cmd_element config_write_memory_cmd;
extern struct cmd_element config_write_cmd;
extern struct cmd_element show_running_config_cmd;
*/
static void
vtysh_install_default (enum node_type node)
{
  install_element (node, CMD_VIEW_LEVEL, &config_list_cmd);
}

/* Standard command node structures. */
static struct cmd_node auth_node =
{
  AUTH_NODE,
  "Password: ",
};

static struct cmd_node view_node =
{
  VIEW_NODE,
  "%s> ",
};



static struct cmd_node auth_enable_node =
{
  AUTH_ENABLE_NODE,
  "Password: ",
};

static struct cmd_node enable_node =
{
  ENABLE_NODE,
  "%s# ",
};

static struct cmd_node config_node =
{
  CONFIG_NODE,
  "%s(config)# ",
  1
};
static struct cmd_node bgp_node =
{
  BGP_NODE,
  "%s(config-router)# ",
};

static struct cmd_node rip_node =
{
  RIP_NODE,
  "%s(config-router)# ",
};

static struct cmd_node isis_node =
{
  ISIS_NODE,
  "%s(config-router)# ",
};

static struct cmd_node interface_node =
{
  INTERFACE_NODE,
  "%s(config-if)# ",
};

static struct cmd_node rmap_node =
{
  RMAP_NODE,
  "%s(config-route-map)# "
};


static struct cmd_node bgp_vpnv4_node =
{
  BGP_VPNV4_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_vpnv6_node =
{
  BGP_VPNV6_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_encap_node =
{
  BGP_ENCAP_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_encapv6_node =
{
  BGP_ENCAPV6_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_ipv4_node =
{
  BGP_IPV4_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_ipv4m_node =
{
  BGP_IPV4M_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_ipv6_node =
{
  BGP_IPV6_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node bgp_ipv6m_node =
{
  BGP_IPV6M_NODE,
  "%s(config-router-af)# "
};

static struct cmd_node ospf_node =
{
  OSPF_NODE,
  "%s(config-router)# "
};

static struct cmd_node ripng_node =
{
  RIPNG_NODE,
  "%s(config-router)# "
};

static struct cmd_node ospf6_node =
{
  OSPF6_NODE,
  "%s(config-ospf6)# "
};

static struct cmd_node babel_node =
{
  BABEL_NODE,
  "%s(config-babel)# "
};

static struct cmd_node keychain_node =
{
  KEYCHAIN_NODE,
  "%s(config-keychain)# "
};

static struct cmd_node keychain_key_node =
{
  KEYCHAIN_KEY_NODE,
  "%s(config-keychain-key)# "
};

struct cmd_node link_params_node =
{
  LINK_PARAMS_NODE,
  "%s(config-link-params)# ",
};
int vtysh_execute(const char *aa)
{
  return vtysh_write_cmd(aa);
}

void vtysh_init(void)
{
  memset(&vtysh_client, 0, sizeof(vtysh_client));
  vtysh_readline_init();
  vty_init_vtysh();
  /* Make vty structure. */
  vtysh_client.vty = vty_new();
  vtysh_client.vty->type = VTY_SHELL;
  vtysh_client.vty->node = VIEW_NODE;
  vtysh_client.vty->vtysh_msg = zpl_osmsg_new(VTY_BUFSIZ);
  // vtysh_client.vty->fd = sock;
  // vtysh_client.vty->wfd = sock;
  // vtysh_client.vty->type = VTY_SHELL_SERV;
  // vtysh_client.vty->node = VIEW_NODE;
  vtysh_client.vty->login_type = VTY_LOGIN_VTYSH;
  /* Initialize commands. */
  cmd_init(0);


  install_node (&view_node, NULL);
  install_node (&enable_node, NULL);
  install_node (&auth_node, NULL);
  install_node (&auth_enable_node, NULL);
  //install_node (&restricted_node, NULL);
  install_node (&config_node, NULL);
  install_node (&bgp_node, NULL);
  install_node (&rip_node, NULL);
  install_node (&interface_node, NULL);
  install_node (&link_params_node, NULL);
  install_node (&rmap_node, NULL);

  install_node (&bgp_vpnv4_node, NULL);
  install_node (&bgp_vpnv6_node, NULL);
  install_node (&bgp_encap_node, NULL);
  install_node (&bgp_encapv6_node, NULL);
  install_node (&bgp_ipv4_node, NULL);
  install_node (&bgp_ipv4m_node, NULL);
/* #ifdef ZPL_BUILD_IPV6 */
  install_node (&bgp_ipv6_node, NULL);
  install_node (&bgp_ipv6m_node, NULL);
/* #endif */
  install_node (&ospf_node, NULL);
/* #ifdef ZPL_BUILD_IPV6 */
  install_node (&ripng_node, NULL);
  install_node (&ospf6_node, NULL);
/* #endif */
  install_node (&babel_node, NULL);
  install_node (&keychain_node, NULL);
  install_node (&keychain_key_node, NULL);
  install_node (&isis_node, NULL);
  //install_node (&vty_node, NULL);

  vtysh_install_default (VIEW_NODE);
  vtysh_install_default (ENABLE_NODE);
  vtysh_install_default (CONFIG_NODE);
  vtysh_install_default (BGP_NODE);
  vtysh_install_default (RIP_NODE);
  vtysh_install_default (INTERFACE_NODE);
  vtysh_install_default (LINK_PARAMS_NODE);
  vtysh_install_default (RMAP_NODE);

  vtysh_install_default (BGP_VPNV4_NODE);
  vtysh_install_default (BGP_VPNV6_NODE);
  vtysh_install_default (BGP_ENCAP_NODE);
  vtysh_install_default (BGP_ENCAPV6_NODE);
  vtysh_install_default (BGP_IPV4_NODE);
  vtysh_install_default (BGP_IPV4M_NODE);
  vtysh_install_default (BGP_IPV6_NODE);
  vtysh_install_default (BGP_IPV6M_NODE);
  vtysh_install_default (OSPF_NODE);
  vtysh_install_default (RIPNG_NODE);
  vtysh_install_default (OSPF6_NODE);
  vtysh_install_default (BABEL_NODE);
  vtysh_install_default (ISIS_NODE);
  vtysh_install_default (KEYCHAIN_NODE);
  vtysh_install_default (KEYCHAIN_KEY_NODE);
  //vtysh_install_default (VTY_NODE);

}

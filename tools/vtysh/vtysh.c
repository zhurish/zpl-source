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

#include <os_include>
#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <readline/readline.h>
#include <readline/history.h>

/* VTY shell client structure. */
struct vtysh_client
{
  int fd;
  const char *name;
  int flag;
  const char *path;
  char *pbuf;
  int plen;
} vtysh_client =
    {
        .fd = -1, .name = "zebra", .flag = 0, .path = ZEBRA_VTYSH_PATH};

static void vtysh_client_close(struct vtysh_client *vtysh_client)
{
  if (vtysh_client->pbuf)
  {
    free(vtysh_client->pbuf);
    vtysh_client->pbuf = NULL;
    vtysh_client->plen = 0;
  }
  if (vtysh_client->fd >= 0)
  {
    fprintf(stderr,
            "Warning: closing connection to %s because of an I/O error!\n",
            vtysh_client->name);
    close(vtysh_client->fd);
    vtysh_client->fd = -1;
  }
}

/* Return true if str begins with prefix, else return false */
static int vtysh_client_execute(struct vtysh_client *vtysh_client, const char *line, FILE *fp)
{
  int ret = 0;
  int nbytes = 0, rlen = 0;
  vtysh_hdr_t header;

  os_bzero(&header, sizeof(vtysh_hdr_t));

  if (vtysh_client->fd < 0)
    return ERROR;

  ret = write(vtysh_client->fd, line, strlen(line) + 1);
  if (ret <= 0)
  {
    vtysh_client_close(vtysh_client);
    return ERROR;
  }
  while (1)
  {
    nbytes = read(vtysh_client->fd, &header, sizeof(vtysh_hdr_t));
    if (nbytes <= 0)
    {
      if (ipstack_errno == EINTR)
        continue;

      fprintf(stderr, "(%u)", ipstack_errno);
      perror("");

      if (ipstack_errno == EAGAIN || ipstack_errno == EIO)
        continue;

      vtysh_client_close(vtysh_client);
      return ERROR;
    }
    if (nbytes == sizeof(vtysh_hdr_t))
    {
      if (header.retcode == CMD_SUCCESS)
      {
        if (!strstr(line, "show"))
        {
          return OK;
        }
      }
      if (vtysh_client->pbuf == NULL)
      {
        vtysh_client->pbuf = malloc(header.retlen + 8);
        if (vtysh_client->pbuf)
          vtysh_client->plen = header.retlen + 8;
      }
      else
      {
        if ((header.retlen + 8) > vtysh_client->plen)
        {
          vtysh_client->pbuf = realloc(vtysh_client->pbuf, header.retlen + 8);
          if (vtysh_client->pbuf)
            vtysh_client->plen = header.retlen + 8;
        }
      }
    }
    rlen = header.retlen - sizeof(vtysh_hdr_t);
    while (1)
    {
      nbytes = read(vtysh_client->fd, vtysh_client->pbuf, rlen);
      if (nbytes <= 0)
      {
        if (ipstack_errno == EINTR)
          continue;

        fprintf(stderr, "(%u)", ipstack_errno);
        perror("");

        if (ipstack_errno == EAGAIN || ipstack_errno == EIO)
          continue;

        vtysh_client_close(vtysh_client);
        return ERROR;
      }
      if (nbytes == rlen)
      {
        if (fp)
        {
          fputs(vtysh_client->pbuf, fp);
          fflush(fp);
        }
        return OK;
      }
      else
      {
        if (fp)
        {
          fputs(vtysh_client->pbuf, fp);
          fflush(fp);
        }
        rlen = rlen - nbytes;
      }
    }
  }
  return OK;
}

int vtysh_execute(const char *line)
{
  return vtysh_execute_func(line, 1);
}

/* Configration make from file. */
int vtysh_config_from_file(struct vty *vty, FILE *fp)
{
  int ret;
  struct cmd_element *cmd;

  while (fgets(vty->buf, vty->max, fp))
  {
    ret = command_config_read_one_line(vty, &cmd, 1);

    switch (ret)
    {
    case CMD_WARNING:
      if (vty->type == VTY_FILE)
        fprintf(stdout, "Warning...\n");
      break;
    case CMD_ERR_AMBIGUOUS:
      fprintf(stdout, "%% Ambiguous command.\n");
      break;
    case CMD_ERR_NO_MATCH:
      fprintf(stdout, "%% Unknown command: %s", vty->buf);
      break;
    case CMD_ERR_INCOMPLETE:
      fprintf(stdout, "%% Command incomplete.\n");
      break;
    case CMD_SUCCESS_DAEMON:
    {
      u_int i;
      int cmd_stat = CMD_SUCCESS;

      for (i = 0; i < array_size(vtysh_client); i++)
      {
        if (cmd->daemon & vtysh_client[i].flag)
        {
          cmd_stat = vtysh_client_execute(&vtysh_client[i],
                                          vty->buf, stdout);
          if (cmd_stat != CMD_SUCCESS)
            break;
        }
      }
      if (cmd_stat != CMD_SUCCESS)
        break;

      if (cmd->func)
        (*cmd->func)(cmd, vty, 0, NULL);
    }
    }
  }
  return CMD_SUCCESS;
}

/* Making connection to protocol daemon. */
int vtysh_connect(struct vtysh_client *vclient)
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
            vclient->path, safe_strerror(errno));
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
            safe_strerror(errno));
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
            safe_strerror(errno));
#endif /* DEBUG */
    close(sock);
    return -1;
  }
  vclient->fd = sock;

  return 0;
}

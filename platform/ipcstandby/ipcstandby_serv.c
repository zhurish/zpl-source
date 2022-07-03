/* ipcstandby daemon server routine.
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
 *
 * This file is part of GNU ipcstandby.
 *
 * GNU ipcstandby is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU ipcstandby is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcstandby; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"

#include "thread.h"
#include "eloop.h"
#include "stream.h"
#include "network.h"
#include "sockunion.h"
#include "log.h"
#include "linklist.h"
#include "sockopt.h"
#include "buffer.h"
#include "vty_include.h"
#include "ipcstandby_serv.h"

/* Event list of ipcstandby. */

struct ipcstandby_server_t ipcstandby_server;

static void ipcstandby_serv_event(enum ipcmsg_event event, zpl_socket_t sock, struct ipcstandby_serv *client);

static void ipcstandby_serv_client_close(struct ipcstandby_serv *client);


int ipcstandby_serv_callback(struct ipcstandby_callback cli, struct ipcstandby_callback msg, struct ipcstandby_callback res)
{
  ipcstandby_server.cli_callback.ipcstandby_callback = cli.ipcstandby_callback;
  ipcstandby_server.cli_callback.pVoid = cli.pVoid;
  ipcstandby_server.msg_callback.ipcstandby_callback = msg.ipcstandby_callback;
  ipcstandby_server.msg_callback.pVoid = msg.pVoid;
  ipcstandby_server.res_callback.ipcstandby_callback = res.ipcstandby_callback;
  ipcstandby_server.res_callback.pVoid = res.pVoid;
  return OK;
}


static int ipcstandby_serv_server_send_message(struct ipcstandby_serv *client)
{
  int ret = 0, len = stream_get_endp(client->obuf);
  char *buf = STREAM_DATA(client->obuf);
  stream_putw_at(client->obuf, 0, stream_get_endp(client->obuf));
  while (1)
  {
    ret = ipstack_write(client->sock, buf, len);
    if (ret > 0)
    {
      len -= ret;
      buf += ret;
    }
    else
    {
      if (IPSTACK_ERRNO_RETRY(ipstack_errno))
      {
        continue;
      }
      client->send_faild_cnt++;
      ipcstandby_serv_client_close(client);
      return ERROR;
    }
    if (len == 0)
    {
      client->send_cnt++;
      client->last_write_time = quagga_time(NULL);
      return OK;
    }
  }
  return ERROR;
}

static void ipcstandby_serv_create_header(struct stream *s, zpl_uint16 module, zpl_uint16 cmd)
{
  stream_reset(s);
  /* length placeholder, caller can update */
  stream_putw(s, ZPL_IPCMSG_HEADER_SIZE);
  stream_putc(s, ZPL_IPCMSG_HEADER_MARKER);
  stream_putc(s, ZPL_IPCMSG_VERSION);
  stream_putw(s, module);
  stream_putw(s, cmd);
}

int ipcstandby_serv_result(struct ipcstandby_serv *client, int module, int process, int ret, char *data, int len)
{
  ipcstandby_serv_create_header(client->obuf, module, ZPL_IPCSTANBY_ACK);
  stream_putl(client->obuf, process);
  stream_putl(client->obuf, ret);
  if (data)
  {
    stream_putl(client->obuf, sizeof(struct ipcstanby_result) + len);
    stream_put(client->obuf, data, len);
  }
  else
    stream_putl(client->obuf, sizeof(struct ipcstanby_result));
  return ipcstandby_serv_server_send_message(client);
}

/* Tie up route-type and client->sock */
static void ipcstandby_serv_read_hello(struct ipcstandby_serv *client)
{
  /* type of protocol (lib/zplos_include.h) */
  client->slot = stream_getl(client->ibuf);
  client->hello++;
}

static void ipcstandby_serv_read_register(struct ipcstandby_serv *client)
{
  client->slot = stream_getl(client->ibuf);
  stream_get(client->version, client->ibuf, sizeof(client->version));
  _host_standby.state++;
}

/* Close ipcstandby client. */
static void
ipcstandby_serv_client_close(struct ipcstandby_serv *client)
{
  /* Close file descriptor. */
  if (!ipstack_invalid(client->sock))
  {
    ipstack_close(client->sock);
  }
  if(_host_standby.state)
    _host_standby.state--;
  /* Free stream buffers. */
  if (client->ibuf)
    stream_free(client->ibuf);
  if (client->obuf)
    stream_free(client->obuf);
  /* Release threads. */
  if (client->t_read)
    thread_cancel(client->t_read);
  listnode_delete(ipcstandby_server.client_list, client);
  XFREE(0, client);
}

/* Make new client. */
static void
ipcstandby_serv_client_create(zpl_socket_t sock, struct ipstack_sockaddr_in *remote)
{
  struct ipcstandby_serv *client;
  zpl_uint32 i = 0;

  client = XCALLOC(MTYPE_TMP, sizeof(struct ipcstandby_serv));

  /* Make client input/output buffer. */
  client->sock = sock;
  client->ibuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  client->obuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);

  client->connect_time = quagga_time(NULL);
  listnode_add(ipcstandby_server.client_list, client);
  /* Make new read thread. */
  ipcstandby_serv_event(ZPL_IPCMSG_READ, sock, client);
}

static int
ipcstandby_serv_client_timeout(struct thread *thread)
{
  struct ipcstandby_serv *client;
  client = THREAD_ARG(thread);
  client->t_timeout = NULL;
  if(client->hello == 0)
  {
    client->state = zpl_false;
    ipcstandby_serv_client_close(client);
    //host_standby(zpl_bool val);
    return OK;
  }
  client->t_timeout = thread_add_timer(ipcstandby_server.master, ipcstandby_serv_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);  
  client->hello--; 
  return OK; 
}

/* Handler of ipcstandby service request. */
static int
ipcstandby_serv_client_read(struct thread *thread)
{
  zpl_socket_t sock;
  struct ipcstandby_serv *client;
  zpl_size_t already;
  zpl_uint16 length, command;
  zpl_uint8 marker, version;
  zpl_uint16 module;
  int ret = 0;
  /* Get thread data.  Reset reading thread because I'm running. */
  sock = THREAD_FD(thread);
  client = THREAD_ARG(thread);
  client->t_read = NULL;

  /* Read length and command (if we don't have it already). */
  if ((already = stream_get_endp(client->ibuf)) < ZPL_IPCMSG_HEADER_SIZE)
  {
    ssize_t nbyte;
    if (((nbyte = stream_read_try(client->ibuf, sock,
                                  ZPL_IPCMSG_HEADER_SIZE - already)) == 0) ||
        (nbyte == -1))
    {
      client->recv_faild_cnt++;
      if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->debug))
        zlog_debug(MODULE_NSM, "connection closed ipstack_socket [%d]", sock);
      ipcstandby_serv_client_close(client);
      return -1;
    }
    if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - already))
    {
      /* Try again later. */
      ipcstandby_serv_event(ZPL_IPCMSG_READ, sock, client);
      return 0;
    }
    already = ZPL_IPCMSG_HEADER_SIZE;
  }

  /* Reset to read from the beginning of the incoming packet. */
  stream_set_getp(client->ibuf, 0);

  /* Fetch header values */
  length = stream_getw(client->ibuf);
  marker = stream_getc(client->ibuf);
  version = stream_getc(client->ibuf);
  module = stream_getw(client->ibuf);
  command = stream_getw(client->ibuf);

  if (marker != ZPL_IPCMSG_HEADER_MARKER || version != ZPL_IPCMSG_VERSION)
  {
    client->pkt_err_cnt++;
    zlog_err(MODULE_NSM, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
             __func__, sock, marker, version);
    ipcstandby_serv_client_close(client);
    return -1;
  }
  if (length < ZPL_IPCMSG_HEADER_SIZE)
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u is less than header size %d",
              __func__, sock, length, ZPL_IPCMSG_HEADER_SIZE);
    ipcstandby_serv_client_close(client);
    return -1;
  }
  if (length > STREAM_SIZE(client->ibuf))
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
              __func__, sock, length, (u_long)STREAM_SIZE(client->ibuf));
    ipcstandby_serv_client_close(client);
    return -1;
  }

  /* Read rest of data. */
  if (already < length)
  {
    ssize_t nbyte;
    if (((nbyte = stream_read_try(client->ibuf, sock,
                                  length - already)) == 0) ||
        (nbyte == -1))
    {
      client->recv_faild_cnt++;
      if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->debug))
        zlog_debug(MODULE_NSM, "connection closed [%d] when reading ipcstandby data", sock);
      ipcstandby_serv_client_close(client);
      return -1;
    }
    if (nbyte != (ssize_t)(length - already))
    {
      /* Try again later. */
      ipcstandby_serv_event(ZPL_IPCMSG_READ, sock, client);
      return 0;
    }
  }
  client->recv_cnt++;
  length -= ZPL_IPCMSG_HEADER_SIZE;

  /* Debug packet information. */
  if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->debug))
    zlog_debug(MODULE_NSM, "ipcstandby message comes from ipstack_socket [%d]", sock);

  if (IS_ZPL_IPCMSG_DEBUG_PACKET(client->debug) && IS_ZPL_IPCMSG_DEBUG_RECV(client->debug))
    zlog_debug(MODULE_NSM, "ipcstandby message received [%d] %d in VRF %u",
               (command), length, module);

  client->last_read_time = quagga_time(NULL);

  switch (command)
  {
  case ZPL_IPCSTANBY_HELLO:
    ipcstandby_serv_read_hello(client);
    ret = OK;
    break;
  case ZPL_IPCSTANBY_REGISTER:
    ipcstandby_serv_read_register(client);
    ret = OK;
    break;    
    
  case ZPL_IPCSTANBY_CLI:
    if(ipcstandby_server.cli_callback.ipcstandby_callback)
      ret = (ipcstandby_server.cli_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, ipcstandby_server.cli_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCSTANBY_MSG:
    if(ipcstandby_server.msg_callback.ipcstandby_callback)
      ret = (ipcstandby_server.msg_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, ipcstandby_server.msg_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCSTANBY_RES:
    if(ipcstandby_server.res_callback.ipcstandby_callback)
      ret = (ipcstandby_server.res_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, client);
    else
      ret = OS_NO_CALLBACK;  
    break;

  default:
    zlog_info(MODULE_NSM, "ipcstandby received unknown command %d", command);
    ret = OS_UNKNOWN_CMD; 
    break;
  }
  if(command != ZPL_IPCSTANBY_RES)
  {
    if (ret == OK)
      ipcstandby_serv_result(client, module, 0, ret, NULL, 0);
    else
      ipcstandby_serv_result(client, module, 0, ret, zpl_strerror(ret), strlen(zpl_strerror(ret)));
  }
  stream_reset(client->ibuf);
  ipcstandby_serv_event(ZPL_IPCMSG_READ, sock, client);
  return 0;
}

/* Accept code of ipcstandby server ipstack_socket. */
static int
ipcstandby_serv_accept(struct thread *thread)
{
  zpl_socket_t accept_sock;
  zpl_socket_t client_sock;
  struct ipstack_sockaddr_in client;
  socklen_t len;

  accept_sock = THREAD_FD(thread);

  /* Reregister myself. */
  ipcstandby_serv_event(ZPL_IPCMSG_ACCEPT, accept_sock, NULL);

  len = sizeof(struct ipstack_sockaddr_in);
  client_sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client, &len);

  if (ipstack_invalid(client_sock))
  {
    zlog_warn(MODULE_NSM, "Can't ipstack_accept ipcstandby ipstack_socket: %s", ipstack_strerror(ipstack_errno));
    return -1;
  }

  /* Make client ipstack_socket non-blocking.  */
  ipstack_set_nonblocking(client_sock);

  /* Create new ipcstandby client. */
  ipcstandby_serv_client_create(client_sock, &client);

  return 0;
}

/* Make ipcstandby's server ipstack_socket. */
static zpl_socket_t ipcstandby_serv_tcp(char *ip, int port)
{
  int ret;
  zpl_socket_t accept_sock;
  struct ipstack_sockaddr_in addr;

  accept_sock = ipstack_socket(IPCOM_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);

  if (ipstack_invalid(accept_sock))
  {
    zlog_warn(MODULE_NSM, "Can't create zserv stream ipstack_socket: %s",
              ipstack_strerror(ipstack_errno));
    // zlog_warn (MODULE_NSM, "ipcstandby can't provice full functionality due to above error");
    return accept_sock;
  }

  memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
  addr.sin_family = IPSTACK_AF_INET;
  addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  if (ip)
    addr.sin_addr.s_addr = inet_addr(ip);

  sockopt_reuseaddr(accept_sock);
  sockopt_reuseport(accept_sock);

  ret = ipstack_bind(accept_sock, (struct ipstack_sockaddr *)&addr,
                     sizeof(struct ipstack_sockaddr_in));
  if (ret < 0)
  {
    zlog_warn(MODULE_NSM, "Can't ipstack_bind to stream ipstack_socket: %s",
              ipstack_strerror(ipstack_errno));
    // zlog_warn (MODULE_NSM, "ipcstandby can't provice full functionality due to above error");
    ipstack_close(accept_sock); /* Avoid sd leak. */
    return accept_sock;
  }

  ret = ipstack_listen(accept_sock, 1);
  if (ret < 0)
  {
    zlog_warn(MODULE_NSM, "Can't ipstack_listen to stream ipstack_socket: %s",
              ipstack_strerror(ipstack_errno));
    // zlog_warn (MODULE_NSM, "ipcstandby can't provice full functionality due to above error");
    ipstack_close(accept_sock); /* Avoid sd leak. */
    return accept_sock;
  }

  // ipcstandby_serv_event(ZPL_IPCMSG_ACCEPT, accept_sock, NULL);
  return accept_sock;
}

static void
ipcstandby_serv_event(enum ipcmsg_event event, zpl_socket_t sock, struct ipcstandby_serv *client)
{
  switch (event)
  {
  case ZPL_IPCMSG_ACCEPT:
    ipcstandby_server.t_accept = thread_add_read(ipcstandby_server.master, ipcstandby_serv_accept, client, sock);
    break;
  case ZPL_IPCMSG_READ:
    client->t_read = thread_add_read(ipcstandby_server.master, ipcstandby_serv_client_read, client, sock);
    break;
  case ZPL_IPCMSG_TIMEOUT:
    client->t_timeout = thread_add_timer(ipcstandby_server.master, ipcstandby_serv_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);
    break;
    
  case ZPL_IPCMSG_WRITE:
    /**/
    break;
  }
}

/* Make ipcstandby server ipstack_socket, wiping any existing one (see bug #403). */
void ipcstandby_serv_init(void *m)
{
  memset(&ipcstandby_server, 0, sizeof(ipcstandby_server));
  ipcstandby_server.master = m;
  ipcstandby_server.client_list = list_new();
  ipcstandby_server.serv_sock = ipcstandby_serv_tcp(NULL, 0);
	ipcstandby_server.vty = vty_new();
	ipstack_init(OS_STACK, ipcstandby_server.vty->fd);
	ipstack_init(OS_STACK, ipcstandby_server.vty->fd);
	ipcstandby_server.vty->wfd._fd = STDOUT_FILENO;
	ipcstandby_server.vty->fd._fd = STDIN_FILENO;
	ipcstandby_server.vty->type = VTY_STABDVY;
	ipcstandby_server.vty->node = ENABLE_NODE;
	memset(ipcstandby_server.vty->buf, 0, (VTY_BUFSIZ));  
  ipcstandby_serv_event(ZPL_IPCMSG_ACCEPT, ipcstandby_server.serv_sock, NULL);
}

void ipcstandby_serv_exit(void)
{
  struct listnode *node;
  struct ipcstandby_serv *client;

  for (ALL_LIST_ELEMENTS_RO(ipcstandby_server.client_list, node, client))
    ipcstandby_serv_client_close(client);
  if (ipcstandby_server.t_accept)
    thread_cancel(ipcstandby_server.t_accept);
  if (!ipstack_invalid(ipcstandby_server.serv_sock))
  {
    ipstack_close(ipcstandby_server.serv_sock);
  }
}
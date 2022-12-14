/* ipcbc daemon server routine.
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
 *
 * This file is part of GNU ipcbc.
 *
 * GNU ipcbc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU ipcbc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcbc; see the file COPYING.  If not, write to the
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
#include "ipcbc_serv.h"

/* Event list of ipcbc. */

struct ipcbc_server_t ipcbc_server;

static void ipcbc_serv_event(enum ipcmsg_event event, zpl_socket_t sock, struct ipcbc_serv *client);
static void ipcbc_serv_client_close(struct ipcbc_serv *client);


int ipcbc_serv_callback(struct ipcbc_callback msg, struct ipcbc_callback res)
{
  ipcbc_server.set_callback.ipcbc_callback = msg.ipcbc_callback;
  ipcbc_server.set_callback.pVoid = msg.pVoid;
  ipcbc_server.get_callback.ipcbc_callback = res.ipcbc_callback;
  ipcbc_server.get_callback.pVoid = res.pVoid;
  return OK;
}


static int ipcbc_serv_server_send_message(struct ipcbc_serv *client)
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
      ipcbc_serv_client_close(client);
      return ERROR;
    }
    if (len == 0)
    {
      client->send_cnt++;
      client->last_write_time = os_time(NULL);
      return OK;
    }
  }
  return ERROR;
}


int ipcbc_serv_result(struct ipcbc_serv *client, int ret, char *data, int len)
{
  ipcbc_create_header(client->obuf, ZPL_IPCBC_ACK);
  stream_putl(client->obuf, ret);
  if (data)
  {
    stream_putl(client->obuf, sizeof(struct ipcbc_result) + len);
    stream_put(client->obuf, data, len);
  }
  else
    stream_putl(client->obuf, sizeof(struct ipcbc_result));
  return ipcbc_serv_server_send_message(client);
}

static int ipcbc_serv_msg_req(struct ipcbc_serv *client, int cmd, char *data, int len)
{
  ipcbc_create_header(client->obuf, cmd);
  if (data)
  {
    stream_put(client->obuf, data, len);
  }
  else
    stream_putl(client->obuf, 0);
  return ipcbc_serv_server_send_message(client);
}

int ipcbc_serv_msg_set(struct ipcbc_serv *client, char *data, int len)
{
  return ipcbc_serv_msg_req(client, ZPL_IPCBC_SET, data, len);
}
int ipcbc_serv_msg_get(struct ipcbc_serv *client, char *data, int len)
{
  return ipcbc_serv_msg_req(client, ZPL_IPCBC_GET, data, len);
}


/* Tie up route-type and client->sock */
static void ipcbc_serv_read_hello(struct ipcbc_serv *client)
{
  /* type of protocol (lib/zplos_include.h) */
  client->slot = stream_getl(client->ibuf);
  client->proto = stream_getl(client->ibuf);
  client->hello++;
}

static void ipcbc_serv_read_register(struct ipcbc_serv *client)
{
  client->slot = stream_getl(client->ibuf);
  client->proto = stream_getl(client->ibuf); 
  stream_get(client->version, client->ibuf, sizeof(client->version));
}

/* Close ipcbc client. */
static void
ipcbc_serv_client_close(struct ipcbc_serv *client)
{
  /* Close file descriptor. */
  if (!ipstack_invalid(client->sock))
  {
    ipstack_close(client->sock);
  }
  /* Free stream buffers. */
  if (client->ibuf)
    stream_free(client->ibuf);
  if (client->obuf)
    stream_free(client->obuf);
  /* Release threads. */
  if (client->t_read)
    thread_cancel(client->t_read);
  listnode_delete(ipcbc_server.client_list, client);
  XFREE(MTYPE_IPCBCCLIENT, client);
}

/* Make new client. */
static void
ipcbc_serv_client_create(zpl_socket_t sock, struct ipstack_sockaddr_in *remote)
{
  struct ipcbc_serv *client;
  client = XCALLOC(MTYPE_IPCBCCLIENT, sizeof(struct ipcbc_serv));

  /* Make client input/output buffer. */
  client->sock = sock;
  client->ibuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  client->obuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);

  client->connect_time = os_time(NULL);
  listnode_add(ipcbc_server.client_list, client);
  /* Make new read thread. */
  ipcbc_serv_event(ZPL_IPCMSG_READ, sock, client);
}

static int
ipcbc_serv_client_timeout(struct thread *thread)
{
  struct ipcbc_serv *client;
  client = THREAD_ARG(thread);
  client->t_timeout = NULL;
  if(client->hello == 0)
  {
    client->state = zpl_false;
    ipcbc_serv_client_close(client);
    return OK;
  }
  client->t_timeout = thread_add_timer(ipcbc_server.master, ipcbc_serv_client_timeout, client, ZPL_IPCBC_TIMEOUT);  
  client->hello--; 
  return OK; 
}

static int ipcbc_serv_recv_ack_async(struct ipcbc_serv *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout)
{
  int nbyte;
  int timeoutval = timeout;
  zpl_size_t already;
  zpl_uint16 length, command;
  zpl_uint8 marker, version;
  int current_timeval = 0;
  int start_timeval = 0;
  zpl_char cbuf[128], recvtmp[4];
  union prefix46constptr pu;
  pu.p = &client->remote;
  memset(cbuf, 0, sizeof(cbuf));

  start_timeval = os_get_monotonic_msec();

  while (1)
  {
    already = ipstack_read_timeout(ipcbc_server.ack_sock[0], recvtmp + nbyte, 2 - nbyte, timeoutval);
    if (already == OS_TIMEOUT)
    {
      zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
      return OS_TIMEOUT;
    }
    else if (already == ERROR)
    {
      client->recv_faild_cnt++;
      zlog_err(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is ERROR:%s", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)), ipstack_strerror(ipstack_errno));
      return OS_CLOSE;
    }
    if (already != 2)
    {
      nbyte += already;
      /* Try again later. */
      current_timeval = os_get_monotonic_msec();
      timeoutval -= (current_timeval - start_timeval);
      if (timeoutval <= 0)
      {
        zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
        return OS_TIMEOUT;
      }
      continue;
    }
    nbyte = already;
    current_timeval = os_get_monotonic_msec();
    timeoutval -= (current_timeval - start_timeval);
    break;
  }
  if (nbyte == 2)
  {
    stream_set_getp(ipcbc_server.ackbuf, 0);
    /* Fetch header values. */
    length = stream_getw(ipcbc_server.ackbuf);
    marker = stream_getc(ipcbc_server.ackbuf);
    version = stream_getc(ipcbc_server.ackbuf);
    command = stream_getw(ipcbc_server.ackbuf);

    if (command == ZPL_IPCBC_ACK)
    {
      client->recv_cnt++;
      length -= ZPL_IPCMSG_HEADER_SIZE;
      client->last_read_time = os_time(NULL);
      if (ack)
      {
        ack->result = stream_getl(ipcbc_server.ackbuf);
        ack->msglen = stream_getl(ipcbc_server.ackbuf);
        length += sizeof(struct ipcbc_result);
      }
      if (callback && callback->ipcbc_callback && ack->result == OK)
      {
        (callback->ipcbc_callback)(0, stream_pnt(ipcbc_server.ackbuf), length, callback->pVoid);
        stream_reset(ipcbc_server.ackbuf);
        return ack->result;
      }
      client->pkt_err_cnt++;
      stream_reset(ipcbc_server.ackbuf);
      return OK;
    }
    stream_reset(ipcbc_server.ackbuf);
    return ERROR;
  }
  stream_reset(ipcbc_server.ackbuf);
  return ERROR;
}

int ipcbc_serv_recv_ack(struct ipcbc_serv *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout)
{
    os_get_monotonic(&ipcbc_server.wait_timeval);
    ipcbc_server.wait_timeval.tv_sec += (timeout/1000);
    ipcbc_server.wait_timeval.tv_usec += ((timeout%1000)*1000);  
  return ipcbc_serv_recv_ack_async(client, ack, callback, timeout);
}
/* Handler of ipcbc service request. */
static int ipcbc_serv_client_read(struct thread *thread)
{
  zpl_socket_t sock;
  struct ipcbc_serv *client;
  zpl_size_t already;
  zpl_uint16 length, command;
  zpl_uint8 marker, version;
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
        zlog_debug(MODULE_NSM, "connection closed ipstack_socket [%d]", ipstack_fd(sock));
      ipcbc_serv_client_close(client);
      return -1;
    }
    if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - already))
    {
      /* Try again later. */
      ipcbc_serv_event(ZPL_IPCMSG_READ, sock, client);
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
  command = stream_getw(client->ibuf);

  if (marker != ZPL_IPCMSG_HEADER_MARKER || version != ZPL_IPCMSG_VERSION)
  {
    client->pkt_err_cnt++;
    zlog_err(MODULE_NSM, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
             __func__, ipstack_fd(sock), marker, version);
    ipcbc_serv_client_close(client);
    return -1;
  }
  if (length < ZPL_IPCMSG_HEADER_SIZE)
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u is less than header size %d",
              __func__, ipstack_fd(sock), length, ZPL_IPCMSG_HEADER_SIZE);
    ipcbc_serv_client_close(client);
    return -1;
  }
  if (length > STREAM_SIZE(client->ibuf))
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
              __func__, ipstack_fd(sock), length, (u_long)STREAM_SIZE(client->ibuf));
    ipcbc_serv_client_close(client);
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
        zlog_debug(MODULE_NSM, "connection closed [%d] when reading ipcbc data", ipstack_fd(sock));
      ipcbc_serv_client_close(client);
      return -1;
    }
    if (nbyte != (ssize_t)(length - already))
    {
      /* Try again later. */
      ipcbc_serv_event(ZPL_IPCMSG_READ, sock, client);
      return 0;
    }
  }
  client->recv_cnt++;
  length -= ZPL_IPCMSG_HEADER_SIZE;

  /* Debug packet information. */
  if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->debug))
    zlog_debug(MODULE_NSM, "ipcbc message comes from ipstack_socket [%d]", ipstack_fd(sock));

  if (IS_ZPL_IPCMSG_DEBUG_PACKET(client->debug) && IS_ZPL_IPCMSG_DEBUG_RECV(client->debug))
    zlog_debug(MODULE_NSM, "ipcbc message received [%d] %d",
               (command), length);

  client->last_read_time = os_time(NULL);

  switch (command)
  {
  case ZPL_IPCBC_HELLO:
    ipcbc_serv_read_hello(client);
    ret = OK;
    break;
  case ZPL_IPCBC_REGISTER:
    ipcbc_serv_read_register(client);
    ret = OK;
    break;    

  case ZPL_IPCBC_SET:
    if(ipcbc_server.set_callback.ipcbc_callback)
      ret = (ipcbc_server.set_callback.ipcbc_callback)(ZPL_IPCBC_SET, stream_pnt(client->ibuf), 0, ipcbc_server.set_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCBC_GET:
    if(ipcbc_server.get_callback.ipcbc_callback)
      ret = (ipcbc_server.get_callback.ipcbc_callback)(ZPL_IPCBC_GET, stream_pnt(client->ibuf), 0, ipcbc_server.get_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCBC_ACK:
  {
    struct timeval current_timeval;
    os_get_monotonic(&current_timeval);
    stream_copy(ipcbc_server.ackbuf, client->ibuf);
    if (os_timeval_cmp(ipcbc_server.wait_timeval, current_timeval) && !ipstack_invalid(ipcbc_server.ack_sock[1]))
      ipstack_send(ipcbc_server.ack_sock[1], &command, 2, 0);
  }
    break;  
  default:
    zlog_info(MODULE_NSM, "ipcbc received unknown command %d", command);
    ret = OS_UNKNOWN_CMD; 
    break;
  }
  if(command == ZPL_IPCBC_SET)
  {
    if (ret == OK)
      ipcbc_serv_result(client, ret, NULL, 0);
    else
      ipcbc_serv_result(client, ret, zpl_strerror(ret), strlen(zpl_strerror(ret)));
  }
  stream_reset(client->ibuf);
  ipcbc_serv_event(ZPL_IPCMSG_READ, sock, client);
  return 0;
}

/* Accept code of ipcbc server ipstack_socket. */
static int
ipcbc_serv_accept(struct thread *thread)
{
  zpl_socket_t accept_sock;
  zpl_socket_t client_sock;
  struct ipstack_sockaddr_in client;
  socklen_t len;

  accept_sock = THREAD_FD(thread);

  /* Reregister myself. */
  ipcbc_serv_event(ZPL_IPCMSG_ACCEPT, accept_sock, NULL);

  len = sizeof(struct ipstack_sockaddr_in);
  client_sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client, &len);

  if (ipstack_invalid(client_sock))
  {
    zlog_warn(MODULE_NSM, "Can't ipstack_accept ipcbc ipstack_socket: %s", ipstack_strerror(ipstack_errno));
    return -1;
  }

  /* Make client ipstack_socket non-blocking.  */
  ipstack_set_nonblocking(client_sock);

  /* Create new ipcbc client. */
  ipcbc_serv_client_create(client_sock, &client);

  return 0;
}

/* Make ipcbc's server ipstack_socket. */
static zpl_socket_t ipcbc_serv_tcp(char *ip, int port)
{
  int ret;
  zpl_socket_t accept_sock;
  struct ipstack_sockaddr_in addr;

  accept_sock = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);

  if (ipstack_invalid(accept_sock))
  {
    zlog_warn(MODULE_NSM, "Can't create zserv stream ipstack_socket: %s",
              ipstack_strerror(ipstack_errno));
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
    ipstack_close(accept_sock); /* Avoid sd leak. */
    return accept_sock;
  }

  ret = ipstack_listen(accept_sock, 1);
  if (ret < 0)
  {
    zlog_warn(MODULE_NSM, "Can't ipstack_listen to stream ipstack_socket: %s",
              ipstack_strerror(ipstack_errno));
    ipstack_close(accept_sock); /* Avoid sd leak. */
    return accept_sock;
  }
  return accept_sock;
}

static void
ipcbc_serv_event(enum ipcmsg_event event, zpl_socket_t sock, struct ipcbc_serv *client)
{
  switch (event)
  {
  case ZPL_IPCMSG_ACCEPT:
    ipcbc_server.t_accept = thread_add_read(ipcbc_server.master, ipcbc_serv_accept, client, sock);
    break;
  case ZPL_IPCMSG_READ:
    client->t_read = thread_add_read(ipcbc_server.master, ipcbc_serv_client_read, client, sock);
    break;
  case ZPL_IPCMSG_TIMEOUT:
    client->t_timeout = thread_add_timer(ipcbc_server.master, ipcbc_serv_client_timeout, client, ZPL_IPCBC_TIMEOUT);
    break;
    
  case ZPL_IPCMSG_WRITE:
    /**/
    break;
  }
}

/* Make ipcbc server ipstack_socket, wiping any existing one (see bug #403). */
void ipcbc_serv_init(void *m)
{
  memset(&ipcbc_server, 0, sizeof(ipcbc_server));
  ipcbc_server.master = m;
  ipcbc_server.client_list = list_new();
  ipcbc_server.serv_sock = ipcbc_serv_tcp(NULL, 0);
  ipcbc_server.ackbuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  if (ipstack_socketpair (IPSTACK_OS, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0, ipcbc_server.ack_sock)<0)
  {
      ipcbc_serv_exit();
      return;
  }
  ipstack_set_nonblocking(ipcbc_server.ack_sock[0]);
  ipstack_set_nonblocking(ipcbc_server.ack_sock[1]);  
  ipcbc_serv_event(ZPL_IPCMSG_ACCEPT, ipcbc_server.serv_sock, NULL);
}

void ipcbc_serv_exit(void)
{
  struct listnode *node;
  struct ipcbc_serv *client;

  for (ALL_LIST_ELEMENTS_RO(ipcbc_server.client_list, node, client))
    ipcbc_serv_client_close(client);
  if(ipcbc_server.ackbuf)
  {
    stream_free(ipcbc_server.ackbuf);
    ipcbc_server.ackbuf = NULL;
  }  
  if (!ipstack_invalid(ipcbc_server.ack_sock[0]))
    ipstack_close(ipcbc_server.ack_sock[0]);
  if (!ipstack_invalid(ipcbc_server.ack_sock[1]))
    ipstack_close(ipcbc_server.ack_sock[1]); 

  if (ipcbc_server.t_accept)
    thread_cancel(ipcbc_server.t_accept);
  if (!ipstack_invalid(ipcbc_server.serv_sock))
  {
    ipstack_close(ipcbc_server.serv_sock);
  }
}
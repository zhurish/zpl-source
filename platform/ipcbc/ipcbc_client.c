/* ipcbc's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
 *
 * This file is part of GNU ipcbc.
 *
 * GNU ipcbc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU ipcbc is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcbc; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"

#include "thread.h"
#include "stream.h"
#include "network.h"
#include "sockunion.h"
#include "log.h"
#include "linklist.h"
#include "network.h"
#include "buffer.h"
#include "vty_include.h"
#include "ipcbc_client.h"


/* Prototype for event manager. */
static void ipcbc_client_event(enum ipcmsg_event, struct ipcbc_client *);
static zpl_socket_t ipcbc_client_socket_connect(struct ipcbc_client *client);


/* Allocate client structure. */
struct ipcbc_client *
ipcbc_client_new(void *m)
{
  struct ipcbc_client *client;
  client = XCALLOC(MTYPE_IPCBCCLIENT, sizeof(struct ipcbc_client));

  client->ibuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  client->obuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  client->ackbuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
  client->master = m;
  if (ipstack_socketpair (OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0, client->ack_sock)<0)
  {
      ipcbc_client_free(client);
      client = NULL;
      return NULL;
  }
  ipstack_set_nonblocking(client->ack_sock[0]);
  ipstack_set_nonblocking(client->ack_sock[1]);  
  return client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free client structure. */
void ipcbc_client_free(struct ipcbc_client *client)
{
  if (!ipstack_invalid(client->ack_sock[0]))
    ipstack_close(client->ack_sock[0]);
  if (!ipstack_invalid(client->ack_sock[1]))
    ipstack_close(client->ack_sock[1]);  
  if (client->ibuf)
    stream_free(client->ibuf);
  if (client->obuf)
    stream_free(client->obuf);
  if(client->ackbuf)
  {
    stream_free(client->ackbuf);
    client->ackbuf = NULL;
  } 
  XFREE(MTYPE_IPCBCCLIENT, client);
}

/* Initialize ipcbc client.  Argument module is unwanted
   redistribute route type. */
void ipcbc_client_init(struct ipcbc_client *client, zpl_uint32 slot, zpl_uint32 proto)
{
  /* Schedule first client connection. */
  if (client->debug)
    zlog_debug(MODULE_DEFAULT, "client start scheduled");
  client->slot = slot;
  client->proto = proto;
  ipcbc_client_event(ZPL_IPCMSG_SCHEDULE, client);
}

/* Stop ipcbc client services. */
void ipcbc_client_stop(struct ipcbc_client *client)
{
  if (client->debug)
    zlog_debug(MODULE_DEFAULT, "client stopped");

  /* Stop threads. */
  THREAD_OFF(client->t_connect);
  THREAD_OFF(client->t_write);

  /* Reset streams. */
  stream_reset(client->ibuf);
  stream_reset(client->obuf);

  /* Close ipstack_socket. */
  if (!ipstack_invalid(client->sock))
  {
    ipstack_close(client->sock);
  }
  client->fail = 0;
  client->recv_cnt = 0;
  client->recv_faild_cnt = 0;
  client->pkt_err_cnt = 0;
  client->send_cnt = 0;
  client->send_faild_cnt = 0;
  client->connect_cnt = 0;
}

void ipcbc_client_reset(struct ipcbc_client *client)
{
  ipcbc_client_stop(client);
  ipcbc_client_init(client, client->slot, client->proto);
}

/* Make ipstack_socket to ipcbc daemon. Return ipcbc ipstack_socket. */
static zpl_socket_t
ipcbc_client_socket_tcp(char *ip, int port)
{
  zpl_socket_t sock;
  int ret;
  struct ipstack_sockaddr_in serv;

  /* We should think about IPv6 connection. */
  sock = ipstack_socket(IPCOM_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);
  if (ipstack_invalid(sock))
    return sock;

  /* Make server ipstack_socket. */
  memset(&serv, 0, sizeof(struct ipstack_sockaddr_in));
  serv.sin_family = IPSTACK_AF_INET;
  serv.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  serv.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  if (ip)
    serv.sin_addr.s_addr = inet_addr(ip);

  /* Connect to ipcbc. */
  ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
  if (ret < 0)
  {
    ipstack_close(sock);
    return sock;
  }
  return sock;
}

/**
 * Connect to ipcbc daemon.
 * @param client a pointer to client structure
 * @return ipstack_socket fd just to make sure that connection established
 * @see ipcbc_client_init
 * @see ipcbc_client_new
 */
static zpl_socket_t
ipcbc_client_socket_connect(struct ipcbc_client *client)
{
  client->sock = ipcbc_client_socket_tcp(NULL, 0);
  return client->sock;
}

static int
ipcbc_client_failed(struct ipcbc_client *client)
{
  client->fail++;
  ipcbc_client_stop(client);
  ipcbc_client_event(ZPL_IPCMSG_CONNECT, client);
  return OK;
}



static int ipcbc_client_register_send(struct ipcbc_client *client)
{
  ipcbc_create_header(client->obuf, ZPL_IPCBC_REGISTER);
  stream_putl(client->obuf, client->slot);
  stream_putl(client->obuf, client->proto);
  stream_put(client->obuf, "", 64);
  // stream_putw_at(s, 0, stream_get_endp(s));
  return ipcbc_client_send_message(client);
}


static int ipcbc_client_hello_send(struct ipcbc_client *client)
{
  ipcbc_create_header(client->obuf, ZPL_IPCBC_HELLO);
  stream_putl(client->obuf, client->slot);
  stream_putl(client->obuf, client->proto);
  // stream_putw_at(s, 0, stream_get_endp(s));
  return ipcbc_client_send_message(client);
}

static int
ipcbc_client_timeout(struct thread *t)
{
  struct ipcbc_client *client;
  client = THREAD_ARG(t);
  client->t_timeout = NULL;
  ipcbc_client_hello_send(client);
  client->t_timeout = thread_add_timer(client->master, ipcbc_client_timeout, client, ZPL_IPCBC_TIMEOUT);  
  return OK;
}

/* Make connection to ipcbc daemon. */
int ipcbc_client_start(struct ipcbc_client *client)
{
  if (client->debug)
    zlog_debug(MODULE_DEFAULT, "ipcbc_client_start is called");

  /* If already connected to the ipcbc. */
  if (!ipstack_invalid(client->sock))
    return 0;

  /* Check ipstack_connect thread. */
  if (client->t_connect)
    return 0;

  if (ipstack_invalid(ipcbc_client_socket_connect(client)))
  {
    if (client->debug)
      zlog_debug(MODULE_DEFAULT, "client connection fail");
    client->fail++;
    ipcbc_client_event(ZPL_IPCMSG_CONNECT, client);
    return -1;
  }

  if (ipstack_set_nonblocking(client->sock) < 0)
    zlog_warn(MODULE_DEFAULT, "%s: set_nonblocking(%d) failed", __func__, ipstack_fd(client->sock));

  /* Clear fail count. */
  client->fail = 0;
  if (client->debug)
    zlog_debug(MODULE_DEFAULT, "client ipstack_connect success with ipstack_socket [%d]", ipstack_fd(client->sock));

  ipcbc_client_register_send(client);
  ipcbc_client_event(ZPL_IPCMSG_TIMEOUT, client);

  return 0;
}

/* This function is a wrapper function for calling ipcbc_client_start from
   timer or event thread. */
static int
ipcbc_client_connect(struct thread *t)
{
  struct ipcbc_client *client;

  client = THREAD_ARG(t);
  client->t_connect = NULL;

  if (client->debug)
    zlog_debug(MODULE_DEFAULT, "ipcbc_client_connect is called");

  return ipcbc_client_start(client);
}

static int __ipcbc_client_recv_message(struct ipcbc_client *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout)
{
  int nbyte;
  int timeoutval = timeout;
  zpl_size_t already;
  zpl_uint16 length, command;
  zpl_uint8 marker, version;
  int current_timeval = 0;
  int start_timeval = 0;
  zpl_char cbuf[128];
  union prefix46constptr pu;
  pu.p = &client->remote;
  memset(cbuf, 0, sizeof(cbuf));

  start_timeval = os_get_monotonic_msec();
  already = stream_get_endp(client->ibuf);
  while (already < ZPL_IPCMSG_HEADER_SIZE)
  {
    nbyte = ipstack_read_timeout(client->sock, STREAM_DATA(client->ibuf) + client->ibuf->endp,
                                 ZPL_IPCMSG_HEADER_SIZE - client->ibuf->endp, timeoutval);
    if (nbyte == OS_TIMEOUT)
    {
      stream_reset(client->ibuf);
      zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
      return OS_TIMEOUT;
    }
    else if (nbyte == ERROR)
    {
      client->recv_faild_cnt++;
      stream_reset(client->ibuf);
      zlog_err(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is ERROR:%s", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)), ipstack_strerror(ipstack_errno));
      return OS_CLOSE;
    }
    if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - client->ibuf->endp))
    {
      /* Try again later. */
      current_timeval = os_get_monotonic_msec();
      timeoutval -= (current_timeval - start_timeval);
      if (timeoutval <= 0)
      {
        stream_reset(client->ibuf);
        zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
        return OS_TIMEOUT;
      }
      continue;
    }
    client->ibuf->endp = ZPL_IPCMSG_HEADER_SIZE;
    already = ZPL_IPCMSG_HEADER_SIZE;
    current_timeval = os_get_monotonic_msec();
    timeoutval -= (current_timeval - start_timeval);
    break;
  }
  if (client->ibuf->endp == ZPL_IPCMSG_HEADER_SIZE)
  {
    stream_set_getp(client->ibuf, 0);

    /* Fetch header values. */
    length = stream_getw(client->ibuf);
    marker = stream_getc(client->ibuf);
    version = stream_getc(client->ibuf);
    command = stream_getw(client->ibuf);

    if (marker != ZPL_IPCMSG_HEADER_MARKER || version != ZPL_IPCMSG_VERSION)
    {
      client->pkt_err_cnt++;
      stream_reset(client->ibuf);
      zlog_err(MODULE_DEFAULT, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
               __func__, ipstack_fd(client->sock), marker, version);
      return ERROR;
    }

    if (length < ZPL_IPCMSG_HEADER_SIZE)
    {
      client->pkt_err_cnt++;
      stream_reset(client->ibuf);
      zlog_err(MODULE_DEFAULT, "%s: ipstack_socket %d message length %u is less than %d ",
               __func__, ipstack_fd(client->sock), length, ZPL_IPCMSG_HEADER_SIZE);
      return ERROR;
    }

    /* Length check. */
    if (length > STREAM_SIZE(client->ibuf))
    {
      struct stream *ns;
      zlog_warn(MODULE_DEFAULT, "%s: message size %u exceeds buffer size %lu, expanding...",
                __func__, length, (u_long)STREAM_SIZE(client->ibuf));
      ns = stream_new(length);
      stream_copy(ns, client->ibuf);
      stream_free(client->ibuf);
      client->ibuf = ns;
    }
    while (1)
    {
      /* Read rest of ipcbc packet. */
      if (already < length)
      {
        nbyte = ipstack_read_timeout(client->sock, STREAM_DATA(client->ibuf) + client->ibuf->endp,
                                     (length)-already, timeoutval);
        if (nbyte == OS_TIMEOUT)
        {
          stream_reset(client->ibuf);
          zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
          return OS_TIMEOUT;
        }
        else if (nbyte == ERROR)
        {
          stream_reset(client->ibuf);
          client->recv_faild_cnt++;
          zlog_err(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is ERROR:%s", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)), ipstack_strerror(ipstack_errno));
          return OS_CLOSE;
        }
        if (nbyte != (ssize_t)(length - already))
        {
          /* Try again later. */
          current_timeval = os_get_monotonic_msec();
          timeoutval -= (current_timeval - start_timeval);
          if (timeoutval <= 0)
          {
            stream_reset(client->ibuf);
            zlog_warn(MODULE_DEFAULT, "Server Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), prefix2str(pu, cbuf, sizeof(cbuf)));
            return OS_TIMEOUT;
          }
          continue;
        }
        already += nbyte;
        client->ibuf->endp += nbyte;
        if (already == length)
          break;
      }
    }
    if (command == ZPL_IPCBC_ACK)
    {
      client->recv_cnt++;
      length -= ZPL_IPCMSG_HEADER_SIZE;
      client->last_read_time = os_time(NULL);
      if (ack)
      {
        ack->result = stream_getl(client->ibuf);
        ack->msglen = stream_getl(client->ibuf);
        length += sizeof(struct ipcbc_result);
      }
      if (callback && callback->ipcbc_callback && ack->result == OK)
      {
        (callback->ipcbc_callback)(0, stream_pnt(client->ibuf), length, callback->pVoid);
      }
      client->pkt_err_cnt++;
      stream_reset(client->ibuf);
      return OK;
    }
    stream_reset(client->ibuf);
    return ERROR;
  }
  stream_reset(client->ibuf);
  return ERROR;
}

static int ipcbc_client_recv_ack_async(struct ipcbc_client *client, struct ipcbc_result *ack,
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
    already = ipstack_read_timeout(client->ack_sock[0], recvtmp + nbyte, 2 - nbyte, timeoutval);
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
    stream_set_getp(client->ackbuf, 0);
    /* Fetch header values. */
    length = stream_getw(client->ackbuf);
    marker = stream_getc(client->ackbuf);
    version = stream_getc(client->ackbuf);
    command = stream_getw(client->ackbuf);

    if (command == ZPL_IPCBC_ACK)
    {
      client->recv_cnt++;
      length -= ZPL_IPCMSG_HEADER_SIZE;
      client->last_read_time = os_time(NULL);
      if (ack)
      {
        ack->result = stream_getl(client->ackbuf);
        ack->msglen = stream_getl(client->ackbuf);
        length += sizeof(struct ipcbc_result);
      }
      if (callback && callback->ipcbc_callback && ack->result == OK)
      {
        (callback->ipcbc_callback)(0, stream_pnt(client->ackbuf), length, callback->pVoid);
        stream_reset(client->ackbuf);
        return ack->result;
      }
      client->pkt_err_cnt++;
      stream_reset(client->ackbuf);
      return OK;
    }
    stream_reset(client->ackbuf);
    return ERROR;
  }
  stream_reset(client->ackbuf);
  return ERROR;
}



static int ipcbc_client_send_message_raw(struct ipcbc_client *client, int timeout)
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
      ipcbc_client_failed(client);
      return ERROR;
    }
    if (len == 0)
    {
      return OK;
    }
  }
  return ERROR;
}


int ipcbc_client_recv_message(struct ipcbc_client *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout)
{
    os_get_monotonic(&client->wait_timeval);
    client->wait_timeval.tv_sec += (timeout/1000);
    client->wait_timeval.tv_usec += ((timeout%1000)*1000);  
  return __ipcbc_client_recv_message(client, ack, callback, timeout);
}

int ipcbc_client_recv_ack(struct ipcbc_client *client, struct ipcbc_result *ack,
                                          struct ipcbc_callback *callback, int timeout)
{
    os_get_monotonic(&client->wait_timeval);
    client->wait_timeval.tv_sec += (timeout/1000);
    client->wait_timeval.tv_usec += ((timeout%1000)*1000);  
  return ipcbc_client_recv_ack_async(client, ack, callback, timeout);
}

int ipcbc_client_send_message(struct ipcbc_client *client)
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
      ipcbc_client_failed(client);
      return ERROR;
    }
    if (len == 0)
    {
      client->send_cnt++;
      client->last_write_time = os_time(NULL);
      /*ret = ipcbc_client_recv_message(client, ack, callback, timeout);
      if (ret == ERROR)
        return ERROR;
      if (ret == OS_CLOSE)
      {
        ipcbc_client_failed(client);
      }*/
      return OK;
    }
  }
  return ERROR;
}



int ipcbc_client_sendmsg(struct ipcbc_client *client, int cmd, char *msg, int len)
{
  ipcbc_create_header(client->obuf, cmd);
  stream_put(client->obuf, msg, len);
  return ipcbc_client_send_message(client);
}


int ipcbc_client_result(struct ipcbc_client *client, int ret, char *data, int len)
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
  return ipcbc_client_send_message_raw(client, 500);
}





static int ipcbc_client_read(struct thread *thread)
{
  zpl_socket_t sock;
  struct ipcbc_client *client;
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
      ipcbc_client_failed(client);
      return -1;
    }
    if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - already))
    {
      /* Try again later. */
      ipcbc_client_event(ZPL_IPCMSG_READ, client);
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
    ipcbc_client_failed(client);
    return -1;
  }
  if (length < ZPL_IPCMSG_HEADER_SIZE)
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u is less than header size %d",
              __func__, ipstack_fd(sock), length, ZPL_IPCMSG_HEADER_SIZE);
    ipcbc_client_failed(client);
    return -1;
  }
  if (length > STREAM_SIZE(client->ibuf))
  {
    client->pkt_err_cnt++;
    zlog_warn(MODULE_NSM, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
              __func__, ipstack_fd(sock), length, (u_long)STREAM_SIZE(client->ibuf));
    ipcbc_client_failed(client);
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
      ipcbc_client_failed(client);
      return -1;
    }
    if (nbyte != (ssize_t)(length - already))
    {
      /* Try again later. */
      ipcbc_client_event(ZPL_IPCMSG_READ, client);
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
  case ZPL_IPCBC_SET:
    if(client->set_callback.ipcbc_callback)
      ret = (client->set_callback.ipcbc_callback)(ZPL_IPCBC_SET, stream_pnt(client->ibuf), 0, client->set_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCBC_GET:
    if(client->get_callback.ipcbc_callback)
      ret = (client->get_callback.ipcbc_callback)(ZPL_IPCBC_GET, stream_pnt(client->ibuf), 0, client->get_callback.pVoid);
    else
      ret = OS_NO_CALLBACK;  
    break;
  case ZPL_IPCBC_ACK:
  {
    struct timeval current_timeval;
    os_get_monotonic(&current_timeval);
    stream_copy(client->ackbuf, client->ibuf);
    if (os_timeval_cmp(client->wait_timeval, current_timeval) && !ipstack_invalid(client->ack_sock[1]))
      ipstack_send(client->ack_sock[1], &command, 2, 0);
  }
    break;  
  default:
    zlog_info(MODULE_NSM, "ipcbc received unknown command %d", command);
    ret = OS_UNKNOWN_CMD; 
    break;
  }
  if(command == ZPL_IPCBC_SET )
  {
    if (ret == OK)
      ipcbc_client_result(client, ret, NULL, 0);
    else
      ipcbc_client_result(client, ret, zpl_strerror(ret), strlen(zpl_strerror(ret)));
  }
  stream_reset(client->ibuf);
  ipcbc_client_event(ZPL_IPCMSG_READ, client);
  return 0;
}

static void
ipcbc_client_event(enum ipcmsg_event event, struct ipcbc_client *client)
{
  switch (event)
  {
  case ZPL_IPCMSG_SCHEDULE:
    if (!client->t_connect)
      client->t_connect =
          thread_add_event(client->master, ipcbc_client_connect, client, 0);
    break;
  case ZPL_IPCMSG_CONNECT:
    if (client->fail >= 10)
      return;
    if (client->debug)
      zlog_debug(MODULE_DEFAULT, "client ipstack_connect schedule interval is %d",
                 client->fail < 3 ? 10 : 60);
    if (!client->t_connect)
      client->t_connect =
          thread_add_timer(client->master, ipcbc_client_connect, client,
                           client->fail < 3 ? 10 : 60);
    break;
  case ZPL_IPCMSG_TIMEOUT:
    if (!client->t_timeout)
      client->t_timeout = thread_add_timer(client->master, ipcbc_client_timeout, client, ZPL_IPCBC_TIMEOUT); 
       break;
  case ZPL_IPCMSG_READ:
    client->t_read = thread_add_read(client->master, ipcbc_client_read, client, client->sock);
    break;
  default:
    break;
  }
}


int ipcbc_client_callback(struct ipcbc_client *client, struct ipcbc_callback msg, struct ipcbc_callback res)
{
  client->set_callback.ipcbc_callback = msg.ipcbc_callback;
  client->set_callback.pVoid = msg.pVoid;
  client->get_callback.ipcbc_callback = res.ipcbc_callback;
  client->get_callback.pVoid = res.pVoid;
  return OK;
}
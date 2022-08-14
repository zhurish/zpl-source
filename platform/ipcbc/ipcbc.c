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
#include "host.h"
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
#include "ipcbc_serv.h"




struct ipcbc  _host_standby;


void ipcbc_create_header(struct stream *s, zpl_uint16 cmd)
{
  stream_reset(s);
  /* length placeholder, caller can update */
  stream_putw(s, ZPL_IPCMSG_HEADER_SIZE);
  stream_putc(s, ZPL_IPCMSG_HEADER_MARKER);
  stream_putc(s, ZPL_IPCMSG_VERSION);
  stream_putw(s, cmd);
}

static int ipcbc_callback_msg(zpl_uint8 *data, zpl_uint32 len, void *pVoid)
{
  int ret = 0;
 #ifdef ZPL_ACTIVE_STANDBY 
  if(host_isactive())
    return OK;  
#endif
  return ret;
}

static int ipcbc_callback_res(zpl_uint8 *data, zpl_uint32 len, void *pVoid)
{
  int ret = 0;
  struct ipcbc_reskey reskey;
  if(ret == OK)
    ipcbc_serv_result(pVoid, OK, &reskey, sizeof(struct ipcbc_reskey));
   else 
    ipcbc_serv_result(pVoid, ret, zpl_strerror(ret), strlen(zpl_strerror(ret)));
  return OK;
}


int ipcbc_sendto_msg(char *msg, int len)
{
  struct ipcbc_result ack;
  int ret = ipcbc_client_sendmsg(ipcbc_client, ZPL_IPCBC_SET, msg, len);

  if(ret == OK) 
  {
    if(ack.result == OK)
      return OK;
  }        
  return ERROR;                        
}



static int ipcbc_callback_resources(zpl_uint8 *data, zpl_uint32 len, void *pVOid)
{
  struct ipcbc_reskey *reskey = pVOid;
  struct ipcbc_reskey *res = (struct ipcbc_reskey *)data;
  if(len >= sizeof(struct ipcbc_reskey))
  {
    memcpy(reskey, res, sizeof(struct ipcbc_reskey));
  	reskey->key1 = ntohl(res->key1);
	  reskey->key2 = ntohl(res->key2);
	  reskey->key3 = ntohl(res->key3);
	  reskey->key4 = ntohl(res->key4);
	  reskey->key5 = ntohl(res->key5);
	  reskey->key6 = ntohl(res->key6);
    return OK;
  }
  return ERROR;
}


int ipcbc_request_res(int type, struct ipcbc_reskey *reskey)
{
  struct ipcbc_result ack;
  int ret = 0, req_type = type;
  struct ipcbc_callback res;
  res.ipcbc_callback = ipcbc_callback_resources;
  res.pVoid = reskey;  

  ret = ipcbc_client_sendmsg(ipcbc_client, ZPL_IPCBC_GET, &req_type, 4);

  if(ret == OK) 
  {
    if(ack.result == OK)
      return OK;
  }        
  return ERROR;                        
}



void ipcbc_init(void *m, zpl_uint32 slot)
{
  memset(&_host_standby, 0, sizeof(struct ipcbc));
	_host_standby.slot = slot;
	_host_standby.master = m; 
  ipcbc_client = ipcbc_client_new(m);
  if(ipcbc_client)
  {
    struct ipcbc_callback msg;
    struct ipcbc_callback res;
    ipcbc_serv_init(m);

    msg.ipcbc_callback = ipcbc_callback_msg;
    msg.pVoid = NULL;
    res.ipcbc_callback = ipcbc_callback_res;
    res.pVoid = NULL;

    ipcbc_client_init(ipcbc_client, 0, 0);
    ipcbc_client_start (ipcbc_client);
  }
}

void ipcbc_exit(void)
{
  if(ipcbc_client)
  {
    ipcbc_client_stop(ipcbc_client);
    ipcbc_client_free (ipcbc_client);
    ipcbc_client = NULL;
  }  
  ipcbc_serv_exit();
}


/************************************************************************/

static void ipcbc_client_statistics_show(struct vty *vty, struct ipcbc_client *client)
{
  zpl_char cbuf[128];
  union prefix46constptr pu;

  pu.p = &client->remote;

  memset(cbuf, 0, sizeof(cbuf));
  vty_out(vty, "Server           : %s %s", prefix2str(pu, cbuf, sizeof(cbuf)), VTY_NEWLINE);
  // vty_out(vty, " Version         : %s %s", client->version, VTY_NEWLINE);
  vty_out(vty, " FD              : %d %s", ipstack_fd(client->sock), VTY_NEWLINE);
  vty_out(vty, " Connect Time    : %s %s", os_time_fmt("/", client->connect_time), VTY_NEWLINE);
  vty_out(vty, " Last Msg Rx Time: %s %s", os_time_fmt("/", client->last_read_time), VTY_NEWLINE);
  vty_out(vty, " Last Msg Tx Time: %s %s", os_time_fmt("/", client->last_write_time), VTY_NEWLINE);

  vty_out(vty, " Recv Packet     : %d %s", client->recv_cnt, VTY_NEWLINE);
  vty_out(vty, " Recv Fail Packet: %d %s", client->recv_faild_cnt, VTY_NEWLINE);
  vty_out(vty, " Recv Err Packet : %d %s", client->pkt_err_cnt, VTY_NEWLINE);
  vty_out(vty, " Send Packet     : %d %s", client->send_cnt, VTY_NEWLINE);
  vty_out(vty, " Send Fail Packet: %d %s", client->send_faild_cnt, VTY_NEWLINE);
  vty_out(vty, " Connect Count   : %d %s", client->connect_cnt, VTY_NEWLINE);

  vty_out(vty, "%s", VTY_NEWLINE);

  return;
}

static void show_ipcbc_serv_statistics_info(struct vty *vty, struct ipcbc_serv *client)
{
  zpl_char cbuf[128];
  union prefix46constptr pu;

  pu.p = &client->remote;

  memset(cbuf, 0, sizeof(cbuf));
  vty_out(vty, "Client           : %s %s", prefix2str(pu, cbuf, sizeof(cbuf)), VTY_NEWLINE);
  vty_out(vty, " Version         : %s %s", client->version, VTY_NEWLINE);
  vty_out(vty, " FD              : %d %s", ipstack_fd(client->sock), VTY_NEWLINE);
  vty_out(vty, " Connect Time    : %s %s", os_time_fmt("/", client->connect_time), VTY_NEWLINE);
  vty_out(vty, " Last Msg Rx Time: %s %s", os_time_fmt("/", client->last_read_time), VTY_NEWLINE);
  vty_out(vty, " Last Msg Tx Time: %s %s", os_time_fmt("/", client->last_write_time), VTY_NEWLINE);

  vty_out(vty, " Recv Packet     : %d %s", client->recv_cnt, VTY_NEWLINE);
  vty_out(vty, " Recv Fail Packet: %d %s", client->recv_faild_cnt, VTY_NEWLINE);
  vty_out(vty, " Recv Err Packet : %d %s", client->pkt_err_cnt, VTY_NEWLINE);
  vty_out(vty, " Send Packet     : %d %s", client->send_cnt, VTY_NEWLINE);
  vty_out(vty, " Send Fail Packet: %d %s", client->send_faild_cnt, VTY_NEWLINE);
  vty_out(vty, " Connect Count   : %d %s", client->connect_cnt, VTY_NEWLINE);

  vty_out(vty, "%s", VTY_NEWLINE);

  return;
}

/* This command is for debugging purpose. */
DEFUN(show_ipcbc_serv_statistics,
      show_ipcbc_serv_statistics_cmd,
      "show standby server statistics",
      SHOW_STR
      "Standby information\n"
      "Server information\n"
      "Statistics information\n")
{
  struct listnode *node;
  struct ipcbc_serv *client;

  for (ALL_LIST_ELEMENTS_RO(ipcbc_server.client_list, node, client))
    show_ipcbc_serv_statistics_info(vty, client);
  return CMD_SUCCESS;
}

DEFUN(ipcbc_serv_statistics_clear,
      ipcbc_serv_statistics_clear_cmd,
      "clear standby server statistics",
      CLEAR_STR
      "Standby information\n"
      "Server information\n"
      "Statistics information\n")
{
  struct listnode *node;
  struct ipcbc_serv *client;

  for (ALL_LIST_ELEMENTS_RO(ipcbc_server.client_list, node, client))
  {
    client->recv_cnt = 0;
    client->recv_faild_cnt = 0;
    client->pkt_err_cnt = 0;
    client->send_cnt = 0;
    client->send_faild_cnt = 0;
    client->connect_cnt = 0;
  }
  return CMD_SUCCESS;
}





/* This command is for debugging purpose. */
DEFUN(show_ipcbc_client_statistics,
      show_ipcbc_client_statistics_cmd,
      "show standby client statistics",
      SHOW_STR
      "Standby information\n"
      "Client information\n"
      "Statistics information\n")
{
  if (ipcbc_client)
    ipcbc_client_statistics_show(vty, ipcbc_client);
  return CMD_SUCCESS;
}

DEFUN(ipcbc_client_statistics_clear,
      ipcbc_client_statistics_clear_cmd,
      "clear standby client statistics",
      CLEAR_STR
      "Standby information\n"
      "Client information\n"
      "Statistics information\n")
{
  if (ipcbc_client)
  {
    ipcbc_client->recv_cnt = 0;
    ipcbc_client->recv_faild_cnt = 0;
    ipcbc_client->pkt_err_cnt = 0;
    ipcbc_client->send_cnt = 0;
    ipcbc_client->send_faild_cnt = 0;
    ipcbc_client->connect_cnt = 0;
  }
  return CMD_SUCCESS;
}





void ipcbc_cmd_init(void)
{
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcbc_client_statistics_cmd);
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &ipcbc_client_statistics_clear_cmd);

  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcbc_serv_statistics_cmd);
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &ipcbc_serv_statistics_clear_cmd);
}

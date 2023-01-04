/* ipcstandby's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
 *
 * This file is part of GNU ipcstandby.
 *
 * GNU ipcstandby is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU ipcstandby is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcstandby; see the file COPYING.  If not, write to the
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
#include "ipcstandby_client.h"
#include "ipcstandby_serv.h"

struct ipcstandby _host_standby;

struct module_list module_list_standby =
    {
        .module = MODULE_STANDBY,
        .name = "STANDBY\0",
        .module_init = ipcstandby_init,
        .module_exit = ipcstandby_exit,
        .module_task_init = ipcstandby_task_init,
        .module_task_exit = ipcstandby_task_exit,
        .module_cmd_init = ipcstandby_cmd_init,
        .taskid = 0,
};

void ipcstandby_create_header(struct stream *s, zpl_uint16 command)
{
    stream_reset(s);
    stream_putw(s, ZPL_IPCMSG_HEADER_SIZE);
    stream_putc(s, ZPL_IPCMSG_HEADER_MARKER);
    stream_putc(s, ZPL_IPCMSG_VERSION);
    stream_putw(s, command);
}

static int ipcstandby_switch_callback(zpl_bool standby)
{
    host_active(standby);
    return OK;
}

int ipcstandby_switch_master(zpl_bool master)
{
    return ipcstandby_switch_callback(master);
}

/* server callback */
static int ipcstandby_callback_cli(const zpl_char *data, zpl_uint32 len, void *pVoid)
{
    int ret = 0;
    struct vty *vty = pVoid;
    if (strstr(data, "switch standby")) // 切换为备用
    {
        if (host_isactive()) // 主
        {
            if (_host_standby.ipcstandby_switch_callback)
            {
                ret = (_host_standby.ipcstandby_switch_callback)(zpl_false);
            }
        }
        return ret;
    }
    if (strstr(data, "switch master")) // 切换为主用
    {
        if (host_isstandby()) // 主
        {
            if (_host_standby.ipcstandby_switch_callback)
            {
                ret = (_host_standby.ipcstandby_switch_callback)(zpl_true);
            }
        }
        return ret;
    }

    if (host_isactive()) // 主
    {
        return OK;
    }

    if (vty && host_isstandby()) // 当前是备用，执行主用发送的CLI命令
        ret = vty_command(vty, data);
    return ret;
}

static int ipcstandby_callback_msg(zpl_uint8 *data, zpl_uint32 len, void *pVoid)
{
    int ret = 0;
    if (host_isactive())
        return OK;
    return ret;
}

static int ipcstandby_callback_res(zpl_uint8 *data, zpl_uint32 len, void *pVoid)
{
    int ret = 0;
    struct ipcstanby_reskey reskey;
    if (ret == OK)
        ipcstandby_serv_result(pVoid, 0, OK, (char *)&reskey, sizeof(struct ipcstanby_reskey));
    else
        ipcstandby_serv_result(pVoid, 0, ret, (char *)zpl_strerror(ret), strlen(zpl_strerror(ret)));
    return OK;
}

/* client call vty->length */
int ipcstandby_execue_clicmd(char *cmd, int len)
{
    int ret = 0;
    struct ipcstanby_result ack;
    if (!_host_standby.ipcstandby_client)
    {
        return OK;
    }
    if (host_isstandby() && _host_standby.ipcstandby_client) // 备用
    {
        if (strstr(cmd, "switch master"))
        {
            ret = ipcstandby_client_sendmsg(_host_standby.ipcstandby_client, &ack,
                                            NULL, ZPL_IPCSTANBY_CLI, cmd, len);
            if (ret == OK)
            {
                if (ack.result == OK)
                    return OK;
            }
            return ERROR;
        }
        return OK;
    }
    if (host_isactive() && _host_standby.ipcstandby_client) //
    {
        if (strstr(cmd, "switch standby"))
        {
            ret = ipcstandby_client_sendmsg(_host_standby.ipcstandby_client, &ack,
                                            NULL, ZPL_IPCSTANBY_CLI, cmd, len);
            if (ret == OK)
            {
                if (ack.result == OK)
                    return OK;
            }
            return ERROR;
        }
    }
    if (strstr(cmd, "tftp") || strstr(cmd, "ftp") ||
        strstr(cmd, "ping") || strstr(cmd, "traceroute") ||
        strstr(cmd, "ymodem") || strstr(cmd, "xmodem") ||
        strstr(cmd, "esp update") || strstr(cmd, "scp") ||
        strstr(cmd, "copy") || strstr(cmd, "show") || strstr(cmd, "sh") ||
        strstr(cmd, "ssh") || strstr(cmd, "write") ||
        strstr(cmd, "terminal") || strstr(cmd, "log commands") ||
        strstr(cmd, "exit-platform") || strstr(cmd, "list") ||
        strstr(cmd, "help") || strstr(cmd, "echo") ||
        strstr(cmd, "login-type"))
    {
        return OK;
    }
    ret = ipcstandby_client_sendmsg(_host_standby.ipcstandby_client, &ack,
                                    NULL, ZPL_IPCSTANBY_CLI, cmd, len);

    if (ret == OK)
    {
        if (ack.result == OK)
            return OK;
    }
    return ERROR;
}

int ipcstandby_sendto_msg(char *msg, int len)
{
    struct ipcstanby_result ack;
    int ret = ipcstandby_client_sendmsg(_host_standby.ipcstandby_client, &ack,
                                        NULL, ZPL_IPCSTANBY_MSG, msg, len);

    if (ret == OK)
    {
        if (ack.result == OK)
            return OK;
    }
    return ERROR;
}

static int ipcstandby_callback_resources(zpl_uint8 *data, zpl_uint32 len, void *pVOid)
{
    struct ipcstanby_reskey *reskey = pVOid;
    struct ipcstanby_reskey *res = (struct ipcstanby_reskey *)data;
    if (len >= sizeof(struct ipcstanby_reskey))
    {
        memcpy(reskey, res, sizeof(struct ipcstanby_reskey));
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

int ipcstandby_request_res(int type, struct ipcstanby_reskey *reskey)
{
    struct ipcstanby_result ack;
    int ret = 0, req_type = type;
    struct ipcstandby_callback res;
    res.ipcstandby_callback = ipcstandby_callback_resources;
    res.pVoid = reskey;

    ret = ipcstandby_client_sendmsg(_host_standby.ipcstandby_client, &ack,
                                    &res, ZPL_IPCSTANBY_RES, (char *)&req_type, 4);

    if (ret == OK)
    {
        if (ack.result == OK)
            return OK;
    }
    return ERROR;
}

zpl_bool ipcstandby_done(int waitime)
{
    int waitime_cnt = waitime;
    if (!(_global_host.ipmctable[0].state && _global_host.ipmctable[1].state)) // 当前只有一个板卡在线
    {
        if (_host_standby.ipcstandby_client)
        {
            if (_host_standby.ipcstandby_client->ipcstanby.negotiate == 0)
                _host_standby.ipcstandby_client->ipcstanby.negotiate = 3;
        }
        return zpl_true;
    }
    while (waitime_cnt--)
    {
        if (_host_standby.start_state == 0)
        {
            if (_host_standby.state == 2)
            {
                _host_standby.start_state = 2;
            }
        }
        os_sleep(1);
    }

    if (_host_standby.ipcstandby_client)
    {
        if (_host_standby.ipcstandby_client->ipcstanby.negotiate == 0)
            _host_standby.ipcstandby_client->ipcstanby.negotiate = 3;
    }
    if (_host_standby.start_state == 2)
    {
        return zpl_true;
    }
    return zpl_false;
}

static int ipcstandby_negotiate_handle(struct ipcstanby_negotiate *client)
{
    struct ipcstanby_negotiate *local = NULL;
    if (_host_standby.ipcstandby_client)
    {
        local = &_host_standby.ipcstandby_client->ipcstanby;
        if (local->slot < client->slot)
        {
            if (client->active && client->negotiate) // 对端是主，且协商后的
            {
                if (local->active) // 本地也是主，切换为备
                {
                    local->active = zpl_false;
                    zlog_debug(MODULE_STANDBY, "client %d change to standby", local->slot);
                }
            }
            else if (client->active && client->negotiate == 0) // 对端是主，未协商的
            {
                if (local->active == 0) // 本地是备，切换为主
                {
                    local->active = (zpl_true);
                    zlog_debug(MODULE_STANDBY, "client %d change to master", local->slot);
                }
            }
            else if (client->active == 0 && client->negotiate) // 对端是备，协商的
            {
                if (local->active == 0) // 本地是备，切换为主
                {
                    local->active = (zpl_true);
                    zlog_debug(MODULE_STANDBY, "client %d change to master", local->slot);
                }
            }
            else if (client->active == 0 && client->negotiate == 0) // 对端是备，未协商的
            {
                if (local->active == 0) // 本地是备，切换为主
                {
                    local->active = (zpl_true);
                    zlog_debug(MODULE_STANDBY, "client %d change to master", local->slot);
                }
            }
        }
        else
        {
            if (client->active && client->negotiate) // 对端是主，且协商后的
            {
                if (local->active) // 本地也是主，切换为备
                {
                    local->active = zpl_false;
                    zlog_debug(MODULE_STANDBY, "client %d change to standby", local->slot);
                }
            }
            else if (client->active && client->negotiate == 0) // 对端是主，未协商的
            {
                if (local->active) // 本地也是主，切换为备
                {
                    local->active = zpl_false;
                    zlog_debug(MODULE_STANDBY, "client %d change to standby", local->slot);
                }
            }
            else if (client->active == 0 && client->negotiate) // 对端是备，协商的
            {
                if (local->active == 0) // 本地是备，切换为主
                {
                    local->active = (zpl_true);
                    zlog_debug(MODULE_STANDBY, "client %d change to master", local->slot);
                }
            }
            else if (client->active == 0 && client->negotiate == 0) // 对端是备，未协商的
            {
                if (local->active == 0) // 本地是备，切换为主
                {
                    local->active = (zpl_true);
                    zlog_debug(MODULE_STANDBY, "client %d change to master", local->slot);
                }
            }
        }
        if (local->negotiate >= 3)
        {
            if (local->active == client->active) // 协商一致
            {
                if (local->active == zpl_false)
                {
                    if (host_isactive())
                    {
                        host_standby(zpl_true);
                        zlog_debug(MODULE_STANDBY, "set local to standby");
                    }
                }
                else if (local->active == zpl_true)
                {
                    if (host_isstandby())
                    {
                        zlog_debug(MODULE_STANDBY, "set local to master");
                        host_active(zpl_true);
                    }
                }
            }
        }
        local->negotiate++;
    }
    return OK;
}

int ipcstandby_init(void)
{
    memset(&_host_standby, 0, sizeof(struct ipcstandby));
    _host_standby.slot = _global_host.slot;
    _host_standby.master = thread_master_module_create(MODULE_STANDBY);
    _host_standby.ipcstandby_client = ipcstandby_client_new(_host_standby.master);
    _host_standby.ipcstandby_server = ipcstandby_serv_init(_host_standby.master, os_netservice_port_get("standbyd_port"));
    if (_host_standby.ipcstandby_client && _host_standby.ipcstandby_server)
    {
        struct ipcstandby_callback cli;
        struct ipcstandby_callback msg;
        struct ipcstandby_callback res;
        // ipcstandby_serv_init(_host_standby.master, os_netservice_port_get("standbyd_port"));

        cli.ipcstandby_callback = ipcstandby_callback_cli;
        cli.pVoid = _host_standby.ipcstandby_server->vty;
        msg.ipcstandby_callback = ipcstandby_callback_msg;
        msg.pVoid = NULL;
        res.ipcstandby_callback = ipcstandby_callback_res;
        res.pVoid = NULL;

        ipcstandby_serv_callback(_host_standby.ipcstandby_server, cli, msg, res);
        _host_standby.ipcstandby_switch_callback = ipcstandby_switch_callback;
        ipcstandby_client_start(_host_standby.ipcstandby_client, _host_standby.slot, NULL, os_netservice_port_get("standbyc_port"));

        _host_standby.ipcstandby_server->ipcstandby_negotiate_callback = ipcstandby_negotiate_handle;
        return OK;
    }
    return ERROR;
}

int ipcstandby_exit(void)
{
    if (_host_standby.ipcstandby_client)
    {
        ipcstandby_client_stop(_host_standby.ipcstandby_client);
        ipcstandby_client_free(_host_standby.ipcstandby_client);
        _host_standby.ipcstandby_client = NULL;
    }
    ipcstandby_serv_exit(_host_standby.ipcstandby_server);
    if (_host_standby.master)
    {
        thread_master_free(_host_standby.master);
        _host_standby.master = NULL;
    }
    return OK;
}

static int standby_main_task(void *p)
{

    struct thread_master *master = (struct thread_master *)p;
    module_setup_task(master->module, os_task_id_self());

    while (thread_mainloop(master))
        ;
    return OK;
}

int ipcstandby_task_init(void)
{
    if (!_host_standby.master)
    {
        _host_standby.master = thread_master_module_create(MODULE_STANDBY);
    }
    if (_host_standby.taskid <= 0)
        _host_standby.taskid = os_task_create("standbyTask", OS_TASK_DEFAULT_PRIORITY,
                                              0, standby_main_task, _host_standby.master, OS_TASK_DEFAULT_STACK * 4);
    if (_host_standby.taskid > 0)
    {
        module_setup_task(MODULE_STANDBY, _host_standby.taskid);
        return OK;
    }
    return ERROR;
}

int ipcstandby_task_exit(void)
{
    if (_host_standby.taskid > 0)
        os_task_destroy(_host_standby.taskid);
    if (_host_standby.master)
    {
        thread_master_free(_host_standby.master);
        _host_standby.master = NULL;
    }
    return OK;
}
/************************************************************************/

static void ipcstandby_client_statistics_show(struct vty *vty, struct ipcstandby_client *client)
{
    vty_out(vty, "Server           : %s:%d %s", client->remote, client->port, VTY_NEWLINE);
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

static void show_ipcstandby_serv_statistics_info(struct vty *vty, struct ipcstandby_serv *client)
{
    vty_out(vty, "Client           : %s:%d %s", client->remote, client->port, VTY_NEWLINE);
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
DEFUN(show_ipcstandby_serv_statistics,
      show_ipcstandby_serv_statistics_cmd,
      "show standby server statistics",
      SHOW_STR
      "Standby information\n"
      "Server information\n"
      "Statistics information\n")
{
    struct listnode *node;
    struct ipcstandby_serv *client;

    for (ALL_LIST_ELEMENTS_RO(_host_standby.ipcstandby_server->client_list, node, client))
        show_ipcstandby_serv_statistics_info(vty, client);
    return CMD_SUCCESS;
}

DEFUN(ipcstandby_serv_statistics_clear,
      ipcstandby_serv_statistics_clear_cmd,
      "clear standby server statistics",
      CLEAR_STR
      "Standby information\n"
      "Server information\n"
      "Statistics information\n")
{
    struct listnode *node;
    struct ipcstandby_serv *client;

    for (ALL_LIST_ELEMENTS_RO(_host_standby.ipcstandby_server->client_list, node, client))
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
DEFUN(show_ipcstandby_client_statistics,
      show_ipcstandby_client_statistics_cmd,
      "show standby client statistics",
      SHOW_STR
      "Standby information\n"
      "Client information\n"
      "Statistics information\n")
{
    if (_host_standby.ipcstandby_client)
        ipcstandby_client_statistics_show(vty, _host_standby.ipcstandby_client);
    return CMD_SUCCESS;
}

DEFUN(ipcstandby_client_statistics_clear,
      ipcstandby_client_statistics_clear_cmd,
      "clear standby client statistics",
      CLEAR_STR
      "Standby information\n"
      "Client information\n"
      "Statistics information\n")
{
    if (_host_standby.ipcstandby_client)
    {
        _host_standby.ipcstandby_client->recv_cnt = 0;
        _host_standby.ipcstandby_client->recv_faild_cnt = 0;
        _host_standby.ipcstandby_client->pkt_err_cnt = 0;
        _host_standby.ipcstandby_client->send_cnt = 0;
        _host_standby.ipcstandby_client->send_faild_cnt = 0;
        _host_standby.ipcstandby_client->connect_cnt = 0;
    }
    return CMD_SUCCESS;
}

DEFUN(ipcstandby_switch_cli,
      ipcstandby_switch_cli_cmd,
      "switch (master|standby)",
      "Switch\n"
      "Master mode\n"
      "Standby mode\n")
{
    if (host_isstandby())
    {
        if (strstr(argv[0], "master"))
            ipcstandby_switch_master(zpl_true);
        else
        {
            vty_out(vty, "Is already on standby mode%s", VTY_NEWLINE);
        }
    }
    else
    {
        if (strstr(argv[0], "standby"))
            ipcstandby_switch_master(zpl_false);
        else
        {
            vty_out(vty, "Is already on master mode%s", VTY_NEWLINE);
        }
    }
    return CMD_SUCCESS;
}

DEFUN(show_ipcstandby_negotiate,
      show_ipcstandby_negotiate_cmd,
      "show standby negotiate",
      SHOW_STR
      "Standby information\n"
      "Negotiate information\n")
{
    struct listnode *node;
    struct ipcstandby_serv *client;
    struct ipcstanby_negotiate *local = NULL;
    if (_host_standby.ipcstandby_client)
    {
        local = &_host_standby.ipcstandby_client->ipcstanby;
        vty_out(vty, "Local slot        %d%s", local->slot, VTY_NEWLINE);
        vty_out(vty, "Local active      %d%s", local->active, VTY_NEWLINE);
        vty_out(vty, "Local negotiate   %d%s", local->negotiate, VTY_NEWLINE);

        for (ALL_LIST_ELEMENTS_RO(_host_standby.ipcstandby_server->client_list, node, client))
        {
            vty_out(vty, " remote slot      %d%s", client->ipcstanby.slot, VTY_NEWLINE);
            vty_out(vty, " remote active    %d%s", client->ipcstanby.active, VTY_NEWLINE);
            vty_out(vty, " remote negotiate %d%s", client->ipcstanby.negotiate, VTY_NEWLINE);
        }
    }
    return CMD_SUCCESS;
}
DEFUN(ipcstandby_negotiate_reset,
      ipcstandby_negotiate_reset_cmd,
      "reset standby negotiate",
      CLEAR_STR
      "Standby information\n"
      "Negotiate information\n")
{
    struct ipcstanby_negotiate *local = NULL;
    if (_host_standby.ipcstandby_client)
    {
        local = &_host_standby.ipcstandby_client->ipcstanby;
        local->active = 1;
        local->negotiate = 0;
    }
    return CMD_SUCCESS;
}

int ipcstandby_cmd_init(void)
{
    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcstandby_client_statistics_cmd);
    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &ipcstandby_client_statistics_clear_cmd);

    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcstandby_serv_statistics_cmd);
    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &ipcstandby_serv_statistics_clear_cmd);
    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipcstandby_negotiate_cmd);
    install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &ipcstandby_negotiate_reset_cmd);
    install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &ipcstandby_switch_cli_cmd);
    return OK;
}
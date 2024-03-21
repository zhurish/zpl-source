#include "auto_include.h"
#include "zpl_type.h"
#include "os_ipstack.h"
#include "os_sem.h"
#include "os_socket.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "log.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"
#include "hal_client.h"

struct module_list module_list_bsp =
    {
        .module = MODULE_BSP,
        .name = "BSP\0",
        .module_init = NULL,
        .module_exit = NULL,
        .module_task_init = NULL,
        .module_task_exit = NULL,
        .module_cmd_init = NULL,
        .taskid = 0,
        .master = NULL,
        .flags = 0,
};

static int hal_client_node_msg_send_hello(struct hal_client_node *hal_client);
static int hal_client_node_msg_send_register(struct hal_client_node *hal_client);
static void hal_client_event(enum hal_client_event_e event, struct hal_client_node *hal_client, int val);
/* hello ----> init -----> port -----> register done */

/* Allocate hal_client structure. */
static struct hal_client *hal_client_new(void)
{
    struct hal_client *hal_client;
    hal_client = XCALLOC(MTYPE_HALIPCCLIENT, sizeof(struct hal_client));

    hal_client->outmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_client->evt_outmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->outmsg);
    hal_ipcmsg_create(&hal_client->evt_outmsg);
    hal_client->ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->ipcmsg);

    hal_client->master[0].ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->master[0].ipcmsg);
    hal_client->master[1].ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->master[1].ipcmsg);

    hal_client->standby[0].ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->standby[0].ipcmsg);
    hal_client->standby[1].ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&hal_client->standby[1].ipcmsg);

    hal_client->master[0].state = HAL_CLIENT_CLOSE;
    hal_client->master[1].state = HAL_CLIENT_CLOSE;
    hal_client->standby[0].state = HAL_CLIENT_CLOSE;
    hal_client->standby[1].state = HAL_CLIENT_CLOSE;
    hal_client->master[0].hal_client = hal_client;
    hal_client->master[1].hal_client = hal_client;
    hal_client->standby[0].hal_client = hal_client;
    hal_client->standby[1].hal_client = hal_client;

    hal_client->standby[0].is_event = zpl_false;
    hal_client->standby[1].is_event = zpl_true;
    hal_client->standby[0].is_standby = zpl_true;
    hal_client->standby[1].is_standby = zpl_true;

    hal_client->master[0].is_event = zpl_false;
    hal_client->master[1].is_event = zpl_true;
    hal_client->master[0].is_standby = zpl_false;
    hal_client->master[1].is_standby = zpl_false;
    return hal_client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free hal_client structure. */
static void hal_client_free(struct hal_client *hal_client)
{
    hal_ipcmsg_destroy(&hal_client->master[0].ipcmsg);
    hal_ipcmsg_destroy(&hal_client->master[1].ipcmsg);
    hal_ipcmsg_destroy(&hal_client->standby[0].ipcmsg);
    hal_ipcmsg_destroy(&hal_client->standby[1].ipcmsg);
    hal_ipcmsg_destroy(&hal_client->ipcmsg);
    hal_ipcmsg_destroy(&hal_client->outmsg);
    hal_ipcmsg_destroy(&hal_client->evt_outmsg);
    XFREE(MTYPE_HALIPCCLIENT, hal_client);
}

/* Stop zebra client services. */
static void hal_client_stop(struct hal_client_node *client_node)
{
    THREAD_OFF(client_node->t_read);
    THREAD_OFF(client_node->t_connect);
    THREAD_OFF(client_node->t_time);

    /* Close ipstack_socket. */
    if (!ipstack_invalid(client_node->sock))
    {
        ipstack_close(client_node->sock);
    }
    client_node->state = HAL_CLIENT_CLOSE;
}

/* Initialize zebra client.  Argument redist_default is unwanted
   redistribute route type. */
struct hal_client *hal_client_create(module_t module, zpl_int8 unit, zpl_int8 slot, zpl_char *version)
{
    /* Enable zebra client connection by default. */
    struct hal_client *hal_client = hal_client_new();
    hal_client->unit = unit;
    hal_client->slot = slot;
    hal_client->module = module;
    //hal_client->portnum = portnum;
    strcpy(hal_client->version, version);
    return hal_client;
}

int hal_client_hwport_register(struct hal_client *hal_client, zpl_int8 portnum, struct hal_ipcmsg_hwport *tbl)
{
    int i = 0;
    for (i = 0; i < portnum; i++)
    {
        hal_client->hwport_table[i].unit = tbl[i].unit;
        hal_client->hwport_table[i].slot = tbl[i].slot;
        hal_client->hwport_table[i].type = tbl[i].type;
        hal_client->hwport_table[i].lport = tbl[i].lport;
        hal_client->hwport_table[i].phyid = tbl[i].phyid;
    }
    hal_client->portnum = portnum;

    if (hal_client->master[0].state == HAL_CLIENT_CONNECT)
    {
        hal_client_event(HAL_EVENT_REGISTER, &hal_client->master[0], 1);
    }
    if (hal_client->master[1].state == HAL_CLIENT_CONNECT)
    {
        hal_client_event(HAL_EVENT_REGISTER, &hal_client->master[1], 1);
    }
    if (hal_client->standby[0].state == HAL_CLIENT_CONNECT)
    {
        hal_client_event(HAL_EVENT_REGISTER, &hal_client->standby[0], 1);
    }
    if (hal_client->standby[1].state == HAL_CLIENT_CONNECT)
    {
        hal_client_event(HAL_EVENT_REGISTER, &hal_client->standby[1], 1);
    }
    return OK;
}

int hal_client_destroy(struct hal_client *hal_client)
{
    hal_client_stop(&hal_client->master[0]);
    hal_client_stop(&hal_client->master[1]);
    hal_client_stop(&hal_client->standby[0]);
    hal_client_stop(&hal_client->standby[1]);
    hal_client_free(hal_client);
    return OK;
}

static void hal_client_reset(struct hal_client_node *client)
{
    hal_client_stop(client);
    hal_client_event(HAL_EVENT_SCHEDULE, client, 0);
}

static int hal_client_node_failed(struct hal_client_node *client)
{
    hal_client_stop(client);
    hal_client_event(HAL_EVENT_CONNECT, client, 0);
    return OK;
}
static int hal_client_connect_check(struct thread *t)
{
    struct hal_client_node *client;
    client = THREAD_ARG(t);
    client->t_read = NULL;

    if (ipstack_sock_connect_check(client->sock) == OK)
    {
        /* Clear fail count. */
        client->fail = 0;

        client->state = HAL_CLIENT_CONNECT;
        THREAD_OFF(client->t_time);
        /* Create read thread. */
        hal_client_event(HAL_EVENT_READ, client, 0);
        hal_client_node_msg_send_hello(client);
        zlog_warn(MODULE_HAL, "hal client connect to %s:%d sock [%d] OK", client->remote, client->port, ipstack_fd(client->sock));
        return OK;
    }
    THREAD_OFF(client->t_time);
    zlog_warn(MODULE_HAL, "hal client connect to %s:%d sock [%d] failed", client->remote, client->port, ipstack_fd(client->sock));
    return hal_client_node_failed(client);
}

static int hal_client_connect_timeout(struct thread *t)
{
    struct hal_client_node *client;
    client = THREAD_ARG(t);
    client->t_time = NULL;
    hal_client_node_failed(client);
    return OK;
}
/* Make ipstack_socket to zebra daemon. Return zebra ipstack_socket. */
static int hal_client_socket(struct hal_client_node *client, char *remote, int port)
{
    int ret;
    /* We should think about IPv6 connection. */
    client->sock = ipstack_sock_create(IPSTACK_OS, zpl_true);
    if (ipstack_invalid(client->sock))
    {
        return ERROR;
    }
    /* Connect to ipcstandby. */
    ret = ipstack_sock_connect_nonblock(client->sock, remote ? remote : "127.0.0.1", port);
    if (ret < 0)
    {
        ipstack_close(client->sock);
        return ERROR;
    }
    client->t_read = thread_add_write(client->hal_client->thread_master, hal_client_connect_check, client, client->sock);
    client->t_time = thread_add_timer(client->hal_client->thread_master, hal_client_connect_timeout, client, 5);
    return OK;
}

/* For ipstack_sockaddr_un. */
static int hal_client_socket_un(struct hal_client_node *client, const char *path)
{
#if 1
    int ret;
    zpl_uint32 len;
    struct ipstack_sockaddr_un addr;
    struct stat s_stat;
    /* Stat socket to see if we have permission to access it. */
    ret = stat(path, &s_stat);
    if (ret < 0 && ipstack_errno != ENOENT)
    {
        _OS_ERROR("ipstack sock (%s): stat = %s\n", path,
                  strerror(ipstack_errno));
        return ERROR;
    }

    if (ret >= 0)
    {
        if (!S_ISSOCK(s_stat.st_mode))
        {
            _OS_ERROR("ipstack sock(%s): Not a socket\n", path);
            return ERROR;
        }
    }

    client->sock = ipstack_socket(IPSTACK_OS, AF_UNIX, SOCK_STREAM, 0 /*tcp ? IPPROTO_TCP:IPPROTO_UDP*/);
    if (ipstack_invalid(client->sock))
    {
        _OS_ERROR("ipstack sock(%d) (%s): socket = %s\n", ipstack_fd(client->sock), path, strerror(ipstack_errno));
        return ERROR;
    }

    memset(&addr, 0, sizeof(struct ipstack_sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
    len = addr.sun_len = SUN_LEN(&addr);
#else
    len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

    ret = ipstack_connect(client->sock, (struct ipstack_sockaddr *)&addr, len);
    if (ret < 0)
    {
        _OS_ERROR("ipstack sock(%d) (%s): connect = %s\n", ipstack_fd(client->sock), path,
                  strerror(ipstack_errno));
        ipstack_close(client->sock);
        return ERROR;
    }
    /* Clear fail count. */
    client->fail = 0;

    client->state = HAL_CLIENT_CONNECT;

    /* Create read thread. */
    hal_client_event(HAL_EVENT_READ, client, 0);
    hal_client_node_msg_send_hello(client);
    zlog_warn(MODULE_HAL, "hal client connect to %s:%d sock [%d] OK", client->remote, client->port, ipstack_fd(client->sock));
#else
    client->sock = ipstack_sock_unix_client_create(IPSTACK_OS, zpl_true, path);
    if (ipstack_invalid(client->sock))
    {
        return hal_client_node_failed(client);
    }
    /* Clear fail count. */
    client->fail = 0;

    client->state = HAL_CLIENT_CONNECT;

    /* Create read thread. */
    hal_client_event(HAL_EVENT_READ, client, 0);
    hal_client_node_msg_send_hello(client);
    zlog_warn(MODULE_HAL, "hal client connect to %s:%d sock [%d] OK", client->remote, client->port, ipstack_fd(client->sock));
#endif
    return OK;
}

/**
 * Connect to zebra daemon.
 * @param hal_client a pointer to hal_client structure
 * @return ipstack_socket fd just to make sure that connection established
 * @see hal_client_init
 * @see hal_client_new
 */
static int hal_client_socket_connect(struct hal_client_node *client)
{
    int ret = 0;
    if (client->port)
        ret = hal_client_socket(client, client->remote, client->port);
    else
        ret = hal_client_socket_un(client, client->remote);

    return ret;
}

/* Make connection to zebra daemon. */
int hal_client_start(struct hal_client *hal_client, char *remote, int port, int standby)
{
    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
        zlog_debug(MODULE_HAL, "hal_client_start is called");

    if (port > 0)
    {
        if (standby)
        {
            hal_client->standby[0].port = port;
            hal_client->standby[1].port = port;
        }
        else
        {
            hal_client->master[0].port = port;
            hal_client->master[1].port = port;
        }
    }
    if (remote)
    {
        if (standby)
        {
            memset(hal_client->standby[0].remote, 0, sizeof(hal_client->standby[0].remote));
            memcpy(hal_client->standby[0].remote, remote, min(sizeof(hal_client->standby[0].remote), strlen(remote)));
            memset(hal_client->standby[1].remote, 0, sizeof(hal_client->standby[1].remote));
            memcpy(hal_client->standby[1].remote, remote, min(sizeof(hal_client->standby[1].remote), strlen(remote)));
        }
        else
        {
            memset(hal_client->master[0].remote, 0, sizeof(hal_client->master[0].remote));
            memcpy(hal_client->master[0].remote, remote, min(sizeof(hal_client->master[0].remote), strlen(remote)));
            memset(hal_client->master[1].remote, 0, sizeof(hal_client->master[1].remote));
            memcpy(hal_client->master[1].remote, remote, min(sizeof(hal_client->master[1].remote), strlen(remote)));
        }
    }
    if (standby)
    {
        if (hal_client_socket_connect(&hal_client->standby[0]) == ERROR)
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
                zlog_warn(MODULE_HAL, "hal_client connection fail");
            hal_client_event(HAL_EVENT_CONNECT, &hal_client->standby[0], 0);
        }
        if (hal_client_socket_connect(&hal_client->standby[1]) == ERROR)
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
                zlog_warn(MODULE_HAL, "hal_client connection fail");
            hal_client_event(HAL_EVENT_CONNECT, &hal_client->standby[1], 0);
        }
    }
    else
    {
        if (hal_client_socket_connect(&hal_client->master[0]) == ERROR)
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
                zlog_warn(MODULE_HAL, "hal_client connection fail");
            hal_client_event(HAL_EVENT_CONNECT, &hal_client->master[0], 0);
        }
        if (hal_client_socket_connect(&hal_client->master[1]) == ERROR)
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
                zlog_warn(MODULE_HAL, "hal_client connection fail");
            hal_client_event(HAL_EVENT_CONNECT, &hal_client->master[1], 0);
        }
    }
    return 0;
}

static int hal_client_node_send_message(struct hal_client_node *node_client)
{
    int bytes = 0;
    zpl_size_t already = 0;
    struct hal_client *hal_client = NULL;
    if (node_client == NULL || node_client->hal_client == NULL || node_client->state != HAL_CLIENT_CONNECT)
    {
        return ERROR;
    }
    if (ipstack_invalid(node_client->sock))
        return ERROR;
    hal_client = node_client->hal_client;
    if (IS_HAL_IPCMSG_DEBUG_PACKET(hal_client->debug) &&
        IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug) &&
        IS_HAL_IPCMSG_DEBUG_HEX(hal_client->debug))
        hal_ipcmsg_hexmsg(&hal_client->outmsg, hal_ipcmsg_msglen_get(&hal_client->outmsg), "Client Send:");
    hal_ipcmsg_hdrlen_set(&hal_client->outmsg);
    while (1)
    {
        bytes = ipstack_write_timeout(node_client->sock, hal_client->outmsg.buf + already, hal_client->outmsg.setp - already, node_client->timeout);
        if (bytes == (hal_client->outmsg.setp - already))
            break;
        else if (bytes)
        {
            already += bytes;
        }
        else
            return ERROR;
    }
    return 0;
}

static int hal_client_node_msg_send_hello(struct hal_client_node *node_client)
{
    struct hal_ipcmsg *outmsg = NULL;
    struct hal_ipcmsg_hello *hello = NULL;

    outmsg = &node_client->hal_client->outmsg;

    hello = (struct hal_ipcmsg_hello *)(outmsg->buf + sizeof(struct hal_ipcmsg_header));
    hal_ipcmsg_reset(outmsg);
    hello->stype = node_client->is_event ? HAL_IPCTYPE_EVENT : HAL_IPCTYPE_CMD;
    hello->unit = node_client->hal_client->unit;
    hello->slot = node_client->hal_client->slot;
    strcpy(hello->version, node_client->hal_client->version);
    if (IS_HAL_IPCMSG_DEBUG_EVENT(node_client->hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(node_client->hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send hello msg unit %d slot %d portnum %d version %s",
                   node_client->hal_client->unit, node_client->hal_client->slot, node_client->hal_client->portnum, node_client->hal_client->version);

    hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_HELLO, 0));
    outmsg->setp += sizeof(struct hal_ipcmsg_hello);

    if (node_client->state == HAL_CLIENT_CONNECT)
    {
        hal_client_node_send_message(node_client);
        if(node_client->hal_client->portnum)
            hal_client_event(HAL_EVENT_REGISTER, node_client, 1);
    }
    return 0;
}

static int hal_client_node_msg_send_start_done(struct hal_client_node *node_client)
{
    struct hal_ipcmsg *outmsg = NULL;

    outmsg = &node_client->hal_client->outmsg;

    hal_ipcmsg_reset(outmsg);
    hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_STARTONDE, 0));

    hal_ipcmsg_putl(outmsg, 1);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(node_client->hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(node_client->hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send register msg unit %d slot %d portnum %d",
                   node_client->hal_client->unit, node_client->hal_client->slot, node_client->hal_client->portnum);

    return hal_client_node_send_message(node_client);
}

static int hal_client_node_msg_send_register(struct hal_client_node *node_client)
{
    struct hal_ipcmsg *outmsg = NULL;
    outmsg = &node_client->hal_client->outmsg;

    hal_ipcmsg_reset(outmsg);
    hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_REGISTER, 0));

    hal_ipcmsg_putc(outmsg, node_client->hal_client->unit);
    hal_ipcmsg_putc(outmsg, node_client->hal_client->slot);
    hal_ipcmsg_putc(outmsg, node_client->hal_client->portnum);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(node_client->hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(node_client->hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send init msg unit %d slot %d portnum %d version %s",
                   node_client->hal_client->unit, node_client->hal_client->slot, node_client->hal_client->portnum, node_client->hal_client->version);

    return hal_client_node_send_message(node_client);
}

int hal_client_send_report(struct hal_client *hal_client, char *data, int len)
{
    hal_ipcmsg_reset(&hal_client->evt_outmsg);
    hal_ipcmsg_create_header(&hal_client->evt_outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_REPORT, 0));
    if (data && len)
    {
        hal_ipcmsg_put(&hal_client->evt_outmsg, data, len);
    }
    if (hal_client->master[1].state == HAL_CLIENT_CONNECT)
    {
        hal_client_node_send_message(&hal_client->master[1]);
    }
    if (hal_client->standby[1].state == HAL_CLIENT_CONNECT)
    {
        hal_client_node_send_message(&hal_client->standby[1]);
    }
    return OK;
}

int hal_client_send_return(struct hal_client *hal_client, int ret, const char *fmt, ...)
{
    int len = 0;
    va_list args;
    char logbuf[1024];
    struct hal_ipcmsg_result getvalue;
    if(hal_client == NULL || hal_client->current == NULL)
    {
        return ERROR;
    }    
    // struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
    memset(logbuf, 0, sizeof(logbuf));
    memset(&getvalue, 0, sizeof(getvalue));
    hal_ipcmsg_reset(&hal_client->outmsg);
    hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
    getvalue.result = ret;
    hal_ipcmsg_result_set(&hal_client->outmsg, &getvalue);
    // hal_ipcmsg_putl(&hal_client->outmsg, ret);
    // result->result = htonl(ret);
    // hal_client->outmsg.setp += sizeof(struct hal_ipcmsg_result);
    if (ret != OK)
    {
        va_start(args, fmt);
        len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
        va_end(args);
        if (len > 0 && len < sizeof(logbuf))
            hal_ipcmsg_put(&hal_client->outmsg, logbuf, len);
    }
    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s",
                   ipstack_fd(hal_client->current->sock), hal_client->unit, hal_client->slot, ret, (ret != OK) ? logbuf : "OK");

    return hal_client_node_send_message(hal_client->current);
}

int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue)
{
    // int len = 0;
    // va_list args;
    char logbuf[1024];
    if(hal_client == NULL || hal_client->current == NULL)
    {
        return ERROR;
    }
    // struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
    memset(logbuf, 0, sizeof(logbuf));
    hal_ipcmsg_reset(&hal_client->outmsg);
    hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
    getvalue->result = ret;
    hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s",
                   ipstack_fd(hal_client->current->sock), hal_client->unit, hal_client->slot, ret, (ret != OK) ? logbuf : "OK");

    return hal_client_node_send_message(hal_client->current);
}

int hal_client_send_result_msg(struct hal_client *hal_client, int ret,
                               struct hal_ipcmsg_result *getvalue, int subcmd, char *msg, int msglen)
{
    // int len = 0;
    // va_list args;
    char logbuf[1024];
    // struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
    memset(logbuf, 0, sizeof(logbuf));
    if(hal_client == NULL || hal_client->current == NULL)
    {
        return ERROR;
    }
    hal_ipcmsg_reset(&hal_client->outmsg);
    hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, subcmd));
    getvalue->result = ret;
    hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);
    // hal_ipcmsg_putl(&hal_client->outmsg, ret);
    if (msglen && msg)
        hal_ipcmsg_put(&hal_client->outmsg, msg, msglen);
    // result->result = htonl(ret);
    // hal_client->outmsg.setp += sizeof(struct hal_ipcmsg_result);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s",
                   ipstack_fd(hal_client->current->sock), hal_client->unit, hal_client->slot, ret, (ret != OK) ? logbuf : "OK");

    return hal_client_node_send_message(hal_client->current);
}

/* This function is a wrapper function for calling hal_client_start from
   timer or event thread. */
static int hal_client_connect_thread(struct thread *t)
{
    struct hal_client_node *node_client;

    node_client = THREAD_ARG(t);
    node_client->t_connect = NULL;

    if (IS_HAL_IPCMSG_DEBUG_EVENT(node_client->hal_client->debug))
        zlog_debug(MODULE_HAL, "hal_client_connect is called");
    if (hal_client_socket_connect(node_client) == ERROR)
    {
        if (IS_HAL_IPCMSG_DEBUG_EVENT(node_client->hal_client->debug))
            zlog_warn(MODULE_HAL, "hal_client connection fail");
        hal_client_event(HAL_EVENT_CONNECT, node_client, 0);
    }
    return OK;
}

static int hal_client_timeout_thread(struct thread *thread)
{
    struct hal_client_node *node_client = THREAD_ARG(thread);
    node_client->t_time = NULL;
    node_client->ttl--;
    hal_client_event(HAL_EVENT_TIME, node_client, HAL_IPCMSG_HELLO_TIME);
    return OK;
}

static int hal_client_register_thread(struct thread *thread)
{
    struct hal_client_node *node_client = THREAD_ARG(thread);
    node_client->t_time = NULL;
    if (node_client->state == HAL_CLIENT_CONNECT)
    {
        hal_client_node_msg_send_register(node_client);
        hal_client_event(HAL_EVENT_HWPORT, node_client, 1);
    }
    return OK;
}

static int hal_client_hwport_report(struct hal_client_node *client, zpl_int8 portnum, struct hal_ipcmsg_hwport *tbl)
{
    int i = 0;
    hal_ipcmsg_reset(&client->hal_client->outmsg);
    hal_ipcmsg_create_header(&client->hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_HWPORTTBL, 0));

    hal_ipcmsg_putc(&client->hal_client->outmsg, portnum);
    for (i = 0; i < portnum; i++)
    {
        hal_ipcmsg_putc(&client->hal_client->outmsg, tbl[i].unit);
        hal_ipcmsg_putc(&client->hal_client->outmsg, tbl[i].slot);
        hal_ipcmsg_putc(&client->hal_client->outmsg, tbl[i].type);
        hal_ipcmsg_putc(&client->hal_client->outmsg, tbl[i].lport);
        hal_ipcmsg_putl(&client->hal_client->outmsg, tbl[i].phyid);
    }
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->hal_client->debug) && IS_HAL_IPCMSG_DEBUG_SEND(client->hal_client->debug))
        zlog_debug(MODULE_HAL, "Client Send Port Table msg unit %d slot %d portnum %d", tbl->unit, tbl->slot, portnum);

    if (client->state == HAL_CLIENT_CONNECT)
    {
        hal_client_node_send_message(client);
    }
    return OK;
}
static int hal_client_hwport_thread(struct thread *thread)
{
    struct hal_client_node *node_client = THREAD_ARG(thread);
    node_client->t_time = NULL;
    if (node_client->state == HAL_CLIENT_CONNECT)
    {
        if(!node_client->is_event)
            hal_client_hwport_report(node_client, node_client->hal_client->portnum, node_client->hal_client->hwport_table);
        hal_client_event(HAL_EVENT_DONE, node_client, 1);
    }
    return OK;
}

static int hal_client_start_done_thread(struct thread *thread)
{
    struct hal_client_node *node_client = THREAD_ARG(thread);
    node_client->t_time = NULL;
    hal_client_node_msg_send_start_done(node_client);
    hal_client_event(HAL_EVENT_TIME, node_client, HAL_IPCMSG_HELLO_TIME*2);
    return OK;
}

/* Zebra client message read function. */
static int hal_client_read_thread(struct thread *thread)
{
    int ret = 0;
    zpl_socket_t sock;
    struct hal_client *halclient = NULL;
    struct hal_client_node *node_client = NULL;
    struct hal_ipcmsg_header hdr;
    struct hal_ipcmsg *ipcmsg = NULL;
    /* Get thread data.  Reset reading thread because I'm running. */
    sock = THREAD_FD(thread);
    node_client = THREAD_ARG(thread);
    halclient = node_client->hal_client;

    ipcmsg = &node_client->ipcmsg;

    /* Read length and command (if we don't have it already). */
    if (ipcmsg->setp < HAL_IPCMSG_HEADER_SIZE)
    {
        ssize_t nbyte = 0;
        nbyte = ipstack_read(sock, ipcmsg->buf, HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp);
        if ((nbyte == 0) || (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(halclient->debug))
                zlog_err(MODULE_HAL, "connection closed ipstack_socket [%d]", ipstack_fd(sock));
            hal_client_reset(node_client);
            return -1;
        }
        if (nbyte != (ssize_t)(HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp))
        {
            /* Try again later. */
            hal_client_event(HAL_EVENT_READ, node_client, 0);
            return 0;
        }
        ipcmsg->setp = HAL_IPCMSG_HEADER_SIZE;
    }

    /* Fetch header values */
    hal_ipcmsg_get_header(ipcmsg, &hdr);

    if (hdr.marker != HAL_IPCMSG_HEADER_MARKER || hdr.version != HAL_IPCMSG_VERSION)
    {
        zlog_err(MODULE_HAL, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
                 __func__, ipstack_fd(sock), hdr.marker, hdr.version);
        return -1;
    }
    if (hdr.length < HAL_IPCMSG_HEADER_SIZE)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u is less than header size %d",
                  __func__, ipstack_fd(sock), hdr.length, (int)HAL_IPCMSG_HEADER_SIZE);
        return -1;
    }
    if (hdr.length > ipcmsg->length_max)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
                  __func__, ipstack_fd(sock), hdr.length, (u_long)(ipcmsg->length_max));
        hal_client_reset(node_client);
        return -1;
    }

    /* Read rest of data. */
    if (ipcmsg->setp < hdr.length)
    {
        ssize_t nbyte;
        if (((nbyte = ipstack_read(sock, ipcmsg->buf + ipcmsg->setp,
                                   hdr.length - ipcmsg->setp)) == 0) ||
            (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(halclient->debug))
                zlog_err(MODULE_HAL, "connection closed [%d] when reading zebra data", ipstack_fd(sock));
            hal_client_reset(node_client);
            return -1;
        }
        if (nbyte != (ssize_t)(hdr.length - ipcmsg->setp))
        {
            /* Try again later. */
            hal_client_event(HAL_EVENT_READ, node_client, 0);
            return 0;
        }
        ipcmsg->setp += hdr.length;
    }

    hdr.length -= HAL_IPCMSG_HEADER_SIZE;

    /* Debug packet information. */
    if (IS_HAL_IPCMSG_DEBUG_EVENT(halclient->debug))
        zlog_debug(MODULE_HAL, "Client Recv message comes from ipstack_socket [%d]", ipstack_fd(sock));

    if (IS_HAL_IPCMSG_DEBUG_PACKET(halclient->debug) && IS_HAL_IPCMSG_DEBUG_RECV(halclient->debug))
        zlog_debug(MODULE_HAL, "Client Recv message received [%s] %d ",
                   hal_module_cmd_name(hdr.command), hdr.length);

    if (IS_HAL_IPCMSG_DEBUG_PACKET(halclient->debug) &&
        IS_HAL_IPCMSG_DEBUG_RECV(halclient->debug) &&
        IS_HAL_IPCMSG_DEBUG_HEX(halclient->debug))
        hal_ipcmsg_hexmsg(ipcmsg, hal_ipcmsg_msglen_get(ipcmsg), "Client Recv:");

    ipcmsg->getp = HAL_IPCMSG_HEADER_SIZE;
    halclient->current = node_client;
    if (hal_ipcmsg_hdr_unit_get(ipcmsg) == halclient->unit)
    {
        hal_ipcmsg_msg_clone(&halclient->ipcmsg, ipcmsg);
        switch (IPCCMD_MODULE_GET(hdr.command))
        {
        case HAL_MODULE_MGT:
            if (IPCCMD_CMD_GET(hdr.command) == HAL_MODULE_CMD_HELLO)
            {
                zpl_int8 val8;
                hal_ipcmsg_getc(ipcmsg, &val8);
                hal_ipcmsg_getc(ipcmsg, &val8);
                node_client->ttl = HAL_IPCMSG_HELLO_TIME * 2;
                hal_client_send_return(halclient, OK, "OK");
            }
        default:
            if (IPCCMD_MODULE_GET(hdr.command) > HAL_MODULE_MGT &&
                IPCCMD_MODULE_GET(hdr.command) <= HAL_MODULE_MAX)
            {
                if (halclient->bsp_client_msg_handle)
                {
                    ret = (halclient->bsp_client_msg_handle)(halclient, hdr.command, halclient->bsp_driver);
#if defined(ZPL_SDK_NONE) || defined(ZPL_SDK_USER)
                    if (ret == OS_NO_CALLBACK)
                    {
                        hal_client_send_return(halclient, OS_NO_CALLBACK, ipstack_strerror(OS_NO_CALLBACK));
                    }
                    else
                    {
                        if (IPCCMD_CMD_GET(hdr.command) != HAL_MODULE_CMD_GET)
                            hal_client_send_return(halclient, ret, ipstack_strerror(ret));
                    }
#endif
                }
                else
                {
                    hal_client_send_return(halclient, OS_NO_CALLBACK, ipstack_strerror(OS_NO_CALLBACK));
                }
            }
            else
            {
                ret = OS_NO_SDKSPUUORT;
                hal_client_send_return(halclient, -1, "Client Recv unknown command %d", hdr.command);
                zlog_warn(MODULE_HAL, "Client Recv unknown command %d", hdr.command);
            }
            break;
        }
    }
    else
    {
        ret = OS_NO_SDKSPUUORT;
        hal_client_send_return(halclient, ret, ipstack_strerror(ret));
    }
    hal_ipcmsg_reset(ipcmsg);
    halclient->current = NULL;
    hal_client_event(HAL_EVENT_READ, node_client, 0);
    return 0;
}

static void hal_client_event(enum hal_client_event_e event, struct hal_client_node *client, int val)
{
    switch (event)
    {
    case HAL_EVENT_SCHEDULE:
        if (!client->t_connect)
            client->t_connect =
                thread_add_event(client->hal_client->thread_master, hal_client_connect_thread, client, 0);
        break;
    case HAL_EVENT_CONNECT:
        if (client->fail >= 10)
            return;
        if (IS_HAL_IPCMSG_DEBUG_EVENT(client->hal_client->debug))
            zlog_debug(MODULE_HAL, "hal client ipstack_connect schedule interval is %d", client->fail < 3 ? 10 : 60);
        if (!client->t_connect)
            client->t_connect =
                thread_add_timer(client->hal_client->thread_master, hal_client_connect_thread, client, client->fail < 3 ? 10 : 60);
        break;
    case HAL_EVENT_READ:
        client->t_read =
            thread_add_read(client->hal_client->thread_master, hal_client_read_thread, client, client->sock);
        break;
    case HAL_EVENT_TIME:
        client->t_time =
            thread_add_timer(client->hal_client->thread_master, hal_client_timeout_thread, client, val);
        break;
    case HAL_EVENT_REGISTER:
        client->t_time =
            thread_add_timer(client->hal_client->thread_master, hal_client_register_thread, client, val);
        break;
    case HAL_EVENT_HWPORT:
        client->t_time = thread_add_timer(client->hal_client->thread_master, hal_client_hwport_thread, client, val);
        break;
    case HAL_EVENT_DONE:
        client->t_time =
            thread_add_timer(client->hal_client->thread_master, hal_client_start_done_thread, client, val);
        break;
    default:
        break;
    }
}

int hal_client_callback(struct hal_client *hal_client, int (*bsp_handle)(struct hal_client *, zpl_uint32, void *), void *p)
{
    hal_client->bsp_client_msg_handle = bsp_handle;
    hal_client->bsp_driver = p;
    return OK;
}

/************************************************************************/

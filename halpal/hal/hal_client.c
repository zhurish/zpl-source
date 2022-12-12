#include "auto_include.h"
#include "zplos_include.h"
#include "nsm_event.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "prefix.h"
#include "log.h"
#include "hal_client.h"
#include <sys/un.h>


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


static int hal_client_msg_hello(struct hal_client *hal_client, enum hal_ipctype_e evt);
static int hal_client_msg_register(struct hal_client *hal_client, enum hal_ipctype_e evt);
/* hello ----> init -----> port -----> register done */

/* Allocate hal_client structure. */
static struct hal_client *hal_client_new(void)
{
  struct hal_client *hal_client;
  hal_client = XCALLOC(MTYPE_HALIPCCLIENT, sizeof(struct hal_client));
  hal_client->ipcmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
  hal_client->outmsg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
  hal_ipcmsg_create(&hal_client->ipcmsg);
  hal_ipcmsg_create(&hal_client->outmsg);
  hal_ipcmsg_create(&hal_client->evt_outmsg);
  hal_client->client[0].state = HAL_CLIENT_CLOSE;
  hal_client->client[0].evt_state = HAL_CLIENT_CLOSE;
  hal_client->client[1].state = HAL_CLIENT_CLOSE;
  hal_client->client[1].evt_state = HAL_CLIENT_CLOSE;
  return hal_client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free hal_client structure. */
static void hal_client_free(struct hal_client *hal_client)
{
  hal_ipcmsg_destroy(&hal_client->ipcmsg);
  hal_ipcmsg_destroy(&hal_client->outmsg);
  hal_ipcmsg_destroy(&hal_client->evt_outmsg);
  XFREE(MTYPE_HALIPCCLIENT, hal_client);
}

/* Stop zebra client services. */
static void hal_client_stop(struct hal_client *hal_client, int standby)
{
  int index = 0;
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
    zlog_debug(MODULE_HAL, "hal_client stopped");

  /* Stop threads. */
  if(standby)
  {
    index = 1;
  }
  THREAD_OFF(hal_client->client[index].t_read);
  THREAD_OFF(hal_client->client[index].t_connect);
  THREAD_OFF(hal_client->client[index].t_evt_connect);
  hal_ipcmsg_reset(&hal_client->ipcmsg);
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_reset(&hal_client->evt_outmsg);
  /* Close ipstack_socket. */
  if (!ipstack_invalid(hal_client->client[index].sock))
  {
    ipstack_close(hal_client->client[index].sock);
  }
  if (!ipstack_invalid(hal_client->client[index].evt_sock))
  {
    ipstack_close(hal_client->client[index].evt_sock);
  }  
  hal_client->client[index].state = HAL_CLIENT_CLOSE;
  hal_client->client[index].evt_state = HAL_CLIENT_CLOSE;  
}

/* Initialize zebra client.  Argument redist_default is unwanted
   redistribute route type. */
struct hal_client *hal_client_create(module_t module, zpl_int8 unit, zpl_int8 slot)
{
  /* Enable zebra client connection by default. */
  struct hal_client *hal_client = hal_client_new();
  hal_client->unit = unit;
  hal_client->slot = slot;
  hal_client->module = module;
  //hal_client->ipctype = ipctype;
  /*if(atoi(port) > 0)
   hal_client->socket_port = atoi(port);
  else
  hal_client->socket_port = 0;*/
  return hal_client;
}


int hal_client_destroy(struct hal_client *hal_client)
{
  hal_client_stop(hal_client, 0);
  hal_client_stop(hal_client, 1);
  hal_client_free(hal_client);
  return OK;
}

static void hal_client_reset(struct hal_client *hal_client, int standby)
{
  hal_client_stop(hal_client, standby);
  hal_client_event(HAL_EVENT_SCHEDULE, hal_client, 0, standby);
}
/* Make ipstack_socket to zebra daemon. Return zebra ipstack_socket. */
static zpl_socket_t hal_client_socket(char *remote, int port)
{
  zpl_socket_t sock;
  int ret;
  struct ipstack_sockaddr_in serv;

  /* We should think about IPv6 connection. */
  sock = ipstack_socket(OS_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);
  if (ipstack_invalid(sock))
    return sock;

  /* Make server ipstack_socket. */
  memset(&serv, 0, sizeof(struct ipstack_sockaddr_in));
  serv.sin_family = IPSTACK_AF_INET;
  serv.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  serv.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  if(remote)
    serv.sin_addr.s_addr = inet_addr(remote);
  else
    serv.sin_addr.s_addr = htonl(IPSTACK_INADDR_LOOPBACK);

  /* Connect to zebra. */
  ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&serv, sizeof(serv));
  if (ret < 0)
  {
    zlog_err(MODULE_HAL, "hal client socket sock=%d Connot Connect to %d, (%s)", ipstack_fd(sock), port, ipstack_strerror(errno));
    _OS_ERROR("Connot Connect to %d (%s)", port, ipstack_strerror(errno));
    ipstack_close(sock);
    return sock;
  }
  return sock;
}

/* For ipstack_sockaddr_un. */

static zpl_socket_t hal_client_socket_un(const char *path)
{
  int ret = 0;
  zpl_socket_t sock;
  int len = 0;
  struct ipstack_sockaddr_un addr;

  sock = ipstack_socket(OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0);
  if (ipstack_invalid(sock))
    return sock;

  /* Make server ipstack_socket. */
  memset(&addr, 0, sizeof(struct ipstack_sockaddr_un));
  addr.sun_family = IPSTACK_AF_UNIX;
  strncpy(addr.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
  len = addr.sun_len = SUN_LEN(&addr);
#else
  len = sizeof(addr.sun_family) + strlen(addr.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

  
  ret = ipstack_connect(sock, (struct ipstack_sockaddr *)&addr, len);
  if (ret < 0)
  {
    zlog_err(MODULE_HAL, "hal client socket sock=%d Connot Connect to %s, (%s)", ipstack_fd(sock), path, ipstack_strerror(errno));
    _OS_ERROR("Connot Connect to %s(%s)", path, ipstack_strerror(errno));
    ipstack_close(sock);
    return sock;
  }
  return sock;
}


/**
 * Connect to zebra daemon.
 * @param hal_client a pointer to hal_client structure
 * @return ipstack_socket fd just to make sure that connection established
 * @see hal_client_init
 * @see hal_client_new
 */
static int hal_client_socket_connect(struct hal_client *hal_client, enum hal_ipctype_e evt, int standby)
{
  int index = 0;
  /*
  if (hal_client->port)
    hal_client->sock = hal_client_socket(hal_client->ipctype ? HAL_IPCMSG_EVENT_PORT : HAL_IPCMSG_CMD_PORT);
  else
    hal_client->sock = hal_client_socket_un(hal_client->ipctype ? HAL_IPCMSG_EVENT_PATH : HAL_IPCMSG_CMD_PATH);
  */ 
  if(standby)
    index = 1;
  if(evt == HAL_IPCTYPE_EVENT)
  {
  if (hal_client->client[index].port)
    hal_client->client[index].evt_sock = hal_client_socket(hal_client->client[index].remote, hal_client->client[index].port);
  else
    hal_client->client[index].evt_sock = hal_client_socket_un(hal_client->client[index].remote);
    zlog_force_trap(MODULE_HAL, "hal_client_socket_connect sock fd=%d", ipstack_fd(hal_client->client[index].evt_sock));
  if (ipstack_invalid(hal_client->client[index].evt_sock))
  {
    zlog_force_trap(MODULE_HAL, "hal_client_socket_connect ipstack_invalid sock fd=%d", ipstack_fd(hal_client->client[index].evt_sock));
    return ERROR;
  }
  }
  else
  {
  if (hal_client->client[index].port)
    hal_client->client[index].sock = hal_client_socket(hal_client->client[index].remote, hal_client->client[index].port);
  else
    hal_client->client[index].sock = hal_client_socket_un(hal_client->client[index].remote);
    zlog_force_trap(MODULE_HAL, "hal_client_socket_connect sock fd=%d", ipstack_fd(hal_client->client[index].sock));
  if (ipstack_invalid(hal_client->client[index].sock))
  {
    zlog_force_trap(MODULE_HAL, "hal_client_socket_connect ipstack_invalid sock fd=%d", ipstack_fd(hal_client->client[index].sock));
    return ERROR;
  }
  }
  return OK;
}

/* Make connection to zebra daemon. */
int hal_client_start(struct hal_client *hal_client, char *remote, int port, int standby)
{
  int index = 0;
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
    zlog_debug(MODULE_HAL, "hal_client_start is called");
  if(standby)
    index = 1;
  if(remote)
  {
    memset(hal_client->client[index].remote, 0, sizeof(hal_client->client[index].remote));
    memcpy(hal_client->client[index].remote, remote, min(sizeof(hal_client->client[index].remote), strlen(remote)));
  }
  if(port > 0)
  {
    hal_client->client[index].port = port;
  }
  /* If already connected to the zebra. */
  if (!ipstack_invalid(hal_client->client[index].sock))
    return 0;

  /* Check ipstack_connect thread. */
  if (hal_client->client[index].t_connect)
    return 0;

  if (hal_client_socket_connect(hal_client, HAL_IPCTYPE_CMD, standby) == ERROR)
  {
    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
      zlog_warn(MODULE_HAL, "hal_client connection fail");
    hal_client->client[index].fail++;
    hal_client_event(HAL_EVENT_CONNECT, hal_client, 0, standby);
    return ERROR;
  }

  if (hal_client_socket_connect(hal_client, HAL_IPCTYPE_EVENT, standby) == ERROR)
  {
    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
      zlog_warn(MODULE_HAL, "hal_client connection fail");
    ipstack_close(hal_client->client[index].sock);
    hal_client_event(HAL_EVENT_CONNECT, hal_client, 0, standby);
    return ERROR;
  }

  if (ipstack_set_nonblocking(hal_client->client[index].sock) < 0)
  {
    zlog_warn(MODULE_HAL, "%s: ipstack_set_nonblocking(%d) failed", __func__, ipstack_fd(hal_client->client[index].sock));
  }
  if (ipstack_set_nonblocking(hal_client->client[index].evt_sock) < 0)
  {
    zlog_warn(MODULE_HAL, "%s: ipstack_set_nonblocking(%d) failed", __func__, ipstack_fd(hal_client->client[index].sock));
  }
  /* Clear fail count. */
  hal_client->client[index].fail = 0;

  hal_client->client[index].state = HAL_CLIENT_CONNECT;
  hal_client->client[index].evt_state = HAL_CLIENT_CONNECT;
  /* Create read thread. */
  hal_client_event(HAL_EVENT_READ, hal_client, 0, standby);

  hal_client_msg_hello(hal_client, HAL_IPCTYPE_CMD);
  hal_client_msg_hello(hal_client, HAL_IPCTYPE_EVENT);

  return 0;
}

static int hal_client_send_message(struct hal_client *hal_client, enum hal_ipctype_e evt, int index)
{
  int bytes = 0;
  zpl_size_t already = 0;

  if(evt == HAL_IPCTYPE_EVENT)
  {
    if (ipstack_invalid(hal_client->client[index].evt_sock))
      return ERROR;
    if (hal_client->client[index].evt_state != HAL_CLIENT_CONNECT)
    {
      return ERROR;
    }

    if (IS_HAL_IPCMSG_DEBUG_PACKET(hal_client->debug) && 
          IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug) && 
          IS_HAL_IPCMSG_DEBUG_HEX(hal_client->debug))
      hal_ipcmsg_hexmsg(&hal_client->evt_outmsg, hal_ipcmsg_msglen_get(&hal_client->evt_outmsg), "Client Send:");
    hal_ipcmsg_hdrlen_set(&hal_client->evt_outmsg);
    while (1)
    {
      bytes = ipstack_write_timeout(hal_client->client[index].evt_sock, hal_client->evt_outmsg.buf + already, hal_client->evt_outmsg.setp - already, hal_client->client[index].timeout);
      if (bytes == (hal_client->evt_outmsg.setp - already))
        break;
      else if (bytes)
      {
        already += bytes;
      }
      else
        return ERROR;
    }
  }
  else
  {
    if (ipstack_invalid(hal_client->client[index].sock))
      return ERROR;
    if (hal_client->client[index].state != HAL_CLIENT_CONNECT)
    {
      return ERROR;
    }

    if (IS_HAL_IPCMSG_DEBUG_PACKET(hal_client->debug) && 
          IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug) && 
          IS_HAL_IPCMSG_DEBUG_HEX(hal_client->debug))
      hal_ipcmsg_hexmsg(&hal_client->outmsg, hal_ipcmsg_msglen_get(&hal_client->outmsg), "Client Send:");
    hal_ipcmsg_hdrlen_set(&hal_client->outmsg);
    while (1)
    {
      bytes = ipstack_write_timeout(hal_client->client[index].sock, hal_client->outmsg.buf + already, hal_client->outmsg.setp - already, hal_client->client[index].timeout);
      if (bytes == (hal_client->outmsg.setp - already))
        break;
      else if (bytes)
      {
        already += bytes;
      }
      else
        return ERROR;
    }
  }
  return 0;
}

static int hal_client_msg_hello(struct hal_client *hal_client, enum hal_ipctype_e evt)
{
  int index = 0;
  struct hal_ipcmsg *outmsg = NULL;
  struct hal_ipcmsg_hello *hello = NULL;
  if(evt == HAL_IPCTYPE_EVENT)
  {
    outmsg = &hal_client->evt_outmsg;
  }
  else
  {
    outmsg = &hal_client->outmsg;
  }
  hello = (struct hal_ipcmsg_hello *)(outmsg->buf + sizeof(struct hal_ipcmsg_header));
  hal_ipcmsg_reset(outmsg);
  hello->stype = evt;
  hello->module = hal_client->module;
  hello->unit = hal_client->unit;
  hello->slot = hal_client->slot;
  hello->portnum = hal_client->portnum;
  strcpy(hello->version, hal_client->version);
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send hello msg unit %d slot %d portnum %d version %s", hal_client->unit, hal_client->slot, hal_client->portnum, hal_client->version);

  hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_HELLO, 0));
  outmsg->setp += sizeof(struct hal_ipcmsg_hello);
  index = hal_client->index;
  if (hal_client->client[0].state == HAL_CLIENT_CONNECT)
  {
    hal_client->index = 0;
    hal_client_send_message(hal_client, evt, hal_client->index);
  }
  if (hal_client->client[1].state == HAL_CLIENT_CONNECT)
  {
    hal_client->index = 1;
    hal_client_send_message(hal_client, evt, hal_client->index);
  }
  hal_client->index = index;
  return 0;   
}

static int hal_client_msg_start_done(struct hal_client *hal_client, enum hal_ipctype_e evt)
{
  struct hal_ipcmsg *outmsg = NULL;
  if(evt == HAL_IPCTYPE_EVENT)
  {
    outmsg = &hal_client->evt_outmsg;
  }
  else
  {
    outmsg = &hal_client->outmsg;
  }  
  hal_ipcmsg_reset(outmsg);
  hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_STARTONDE, 0));

  hal_ipcmsg_putl(outmsg, 1);

  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send register msg unit %d slot %d portnum %d", hal_client->unit, hal_client->slot, hal_client->portnum);

  return hal_client_send_message(hal_client, evt, hal_client->index);
}

static int hal_client_msg_register(struct hal_client *hal_client, enum hal_ipctype_e evt)
{
  struct hal_ipcmsg *outmsg = NULL;
  if(evt == HAL_IPCTYPE_EVENT)
  {
    outmsg = &hal_client->evt_outmsg;
  }
  else
  {
    outmsg = &hal_client->outmsg;
  }  
  hal_ipcmsg_reset(outmsg);
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send init msg unit %d slot %d portnum %d version %s", hal_client->unit, hal_client->slot, hal_client->portnum, hal_client->version);
  
  hal_ipcmsg_create_header(outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_REGISTER, 0));
  hal_ipcmsg_putl(outmsg, 1);
  return hal_client_send_message(hal_client, evt, hal_client->index); 
}

int hal_client_send_report(struct hal_client *hal_client, char *data, int len)
{
  hal_ipcmsg_reset(&hal_client->evt_outmsg);
  hal_ipcmsg_create_header(&hal_client->evt_outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_REPORT, 0));
  if (data && len)
  {
      hal_ipcmsg_put(&hal_client->evt_outmsg, data, len);
  }
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send Report msg [%d] unit %d slot %d len %d", 
      ipstack_fd(hal_client->client[hal_client->index].sock), hal_client->unit, hal_client->slot, len);

  return hal_client_send_message(hal_client, HAL_IPCTYPE_EVENT, hal_client->index);
}

int hal_client_send_return(struct hal_client *hal_client, int ret, const char *fmt, ...)
{
  int len = 0;
  va_list args;
  char logbuf[1024];
  struct hal_ipcmsg_result getvalue;
  //struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  memset(logbuf, 0, sizeof(logbuf));
  memset(&getvalue, 0, sizeof(getvalue));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue.result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, &getvalue);
  //hal_ipcmsg_putl(&hal_client->outmsg, ret);
  //result->result = htonl(ret);
  //hal_client->outmsg.setp += sizeof(struct hal_ipcmsg_result);
  if (ret != OK)
  {
    va_start(args, fmt);
    len = vsnprintf(logbuf, sizeof(logbuf), fmt, args);
    va_end(args);
    if (len > 0 && len < sizeof(logbuf))
      hal_ipcmsg_put(&hal_client->outmsg, logbuf, len);
  }
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s", 
      ipstack_fd(hal_client->client[hal_client->index].sock), hal_client->unit, hal_client->slot, ret, (ret != OK)?logbuf:"OK");

  return hal_client_send_message(hal_client, HAL_IPCTYPE_CMD, hal_client->index);
}

int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue)
{
  //int len = 0;
  //va_list args;
  char logbuf[1024];
  //struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  memset(logbuf, 0, sizeof(logbuf));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, 0));
  getvalue->result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);

  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s", 
      ipstack_fd(hal_client->client[hal_client->index].sock), hal_client->unit, hal_client->slot, ret, (ret != OK)?logbuf:"OK");

  return hal_client_send_message(hal_client, HAL_IPCTYPE_CMD, hal_client->index);
}

int hal_client_send_result_msg(struct hal_client *hal_client, int ret, 
    struct hal_ipcmsg_result *getvalue, int subcmd, char *msg, int msglen)
{
  //int len = 0;
  //va_list args;
  char logbuf[1024];
  //struct hal_ipcmsg_result *result = (struct hal_ipcmsg_result *)(hal_client->outmsg.buf + sizeof(struct hal_ipcmsg_header));
  memset(logbuf, 0, sizeof(logbuf));
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_ACK, subcmd));
  getvalue->result = ret;
  hal_ipcmsg_result_set(&hal_client->outmsg, getvalue);
  //hal_ipcmsg_putl(&hal_client->outmsg, ret);
  if(msglen && msg)
    hal_ipcmsg_put(&hal_client->outmsg, msg, msglen);
  //result->result = htonl(ret);
  //hal_client->outmsg.setp += sizeof(struct hal_ipcmsg_result);

  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send result msg [%d] unit %d slot %d result %d %s", 
      ipstack_fd(hal_client->client[hal_client->index].sock), hal_client->unit, hal_client->slot, ret, (ret != OK)?logbuf:"OK");

  return hal_client_send_message(hal_client, HAL_IPCTYPE_CMD, hal_client->index);
}


/* This function is a wrapper function for calling hal_client_start from
   timer or event thread. */
static int hal_client_connect_thread(struct thread *t)
{
  struct hal_client *hal_client;

  hal_client = THREAD_ARG(t);
  hal_client->client[hal_client->index].t_connect = NULL;

  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
    zlog_debug(MODULE_HAL, "hal_client_connect is called");

  return hal_client_start(hal_client, NULL, -1, hal_client->index);
}

static int
hal_client_time_thread(struct thread *thread)
{
  struct hal_client *hal_client = THREAD_ARG(thread);
  hal_client->client[hal_client->index].t_time = NULL;
  return OK;
}

static int hal_client_register_thread(struct thread *thread)
{
  struct hal_client *hal_client = THREAD_ARG(thread);
  hal_client->client[hal_client->index].t_time = NULL;
  hal_client_msg_start_done(hal_client, HAL_IPCTYPE_CMD);
  return OK;
}

/* Zebra client message read function. */
static int
hal_client_read_thread(struct thread *thread)
{
  int ret = 0;
  zpl_socket_t sock;
  struct hal_client *halclient = NULL;
  struct hal_ipcmsg_header hdr;
  struct hal_ipcmsg *ipcmsg = NULL;
  /* Get thread data.  Reset reading thread because I'm running. */
  sock = THREAD_FD(thread);
  halclient = THREAD_ARG(thread);
  if(ipstack_fd(sock) == ipstack_fd(halclient->client[0].sock))
    halclient->index = 0;
  else if(ipstack_fd(sock) == ipstack_fd(halclient->client[1].sock))
    halclient->index = 1;
  halclient->client[halclient->index].t_read = NULL;
  ipcmsg = &halclient->ipcmsg;

  /* Read length and command (if we don't have it already). */
  if (ipcmsg->setp < HAL_IPCMSG_HEADER_SIZE)
  {
    ssize_t nbyte = 0;
    nbyte = ipstack_read(sock, ipcmsg->buf, HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp);
    if ((nbyte == 0) || (nbyte == -1))
    {
      if (IS_HAL_IPCMSG_DEBUG_EVENT(halclient->debug))
        zlog_err(MODULE_HAL, "connection closed ipstack_socket [%d]", ipstack_fd(sock));
      hal_client_reset(halclient, halclient->index);
      return -1;
    }
    if (nbyte != (ssize_t)(HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp))
    {
      /* Try again later. */
      halclient->client[halclient->index].t_read = thread_add_read(halclient->master, hal_client_read_thread, halclient, sock);
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
    hal_client_reset(halclient, halclient->index);
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
      hal_client_reset(halclient, halclient->index);
      return -1;
    }
    if (nbyte != (ssize_t)(hdr.length - ipcmsg->setp))
    {
      /* Try again later. */
      halclient->client[halclient->index].t_read = thread_add_read(halclient->master, hal_client_read_thread, halclient, sock);
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
    hal_ipcmsg_hexmsg(&halclient->outmsg, hal_ipcmsg_msglen_get(&halclient->outmsg), "Client Recv:");

  ipcmsg->getp = HAL_IPCMSG_HEADER_SIZE;

  if (hal_ipcmsg_hdr_unit_get(ipcmsg) == halclient->unit)
  {
    switch (IPCCMD_MODULE_GET(hdr.command))
    {
    case HAL_MODULE_MGT:
      if (IPCCMD_CMD_GET(hdr.command) == HAL_MODULE_CMD_HELLO)
      {

      }
      /*
      if (IPCCMD_CMD_GET(hdr.command) == HAL_MODULE_CMD_REPORT)
      {
        if(halclient->bsp_client_msg_handle)
        {
          ret = (halclient->bsp_client_msg_handle)(halclient, hdr.command, halclient->bsp_driver);
        }
      }
      break;
      */
    default:
      if(IPCCMD_MODULE_GET(hdr.command) > HAL_MODULE_MGT && 
        IPCCMD_MODULE_GET(hdr.command) <= HAL_MODULE_MAX)
      {
        if(halclient->bsp_client_msg_handle)
        {
          ret = (halclient->bsp_client_msg_handle)(halclient, hdr.command, halclient->bsp_driver);
#if defined(ZPL_SDK_NONE) || defined(ZPL_SDK_USER)    
          if(ret == OS_NO_CALLBACK)
          {
            hal_client_send_return(halclient,  OS_NO_CALLBACK, ipstack_strerror(OS_NO_CALLBACK));
          }
          else
          {
            if (IPCCMD_CMD_GET(hdr.command) != HAL_MODULE_CMD_GET)
              hal_client_send_return(halclient,  ret, ipstack_strerror(ret));
          }
#endif          
        }
        else
        {
          hal_client_send_return(halclient,  OS_NO_CALLBACK, ipstack_strerror(OS_NO_CALLBACK));
        }
      }
      else
      {
        ret = OS_NO_SDKSPUUORT;
        hal_client_send_return(halclient,  -1, "Client Recv unknown command %d", hdr.command);
        zlog_warn(MODULE_HAL, "Client Recv unknown command %d", hdr.command);
      }
      break;
    }
  }
  else
  {
    ret = OS_NO_SDKSPUUORT;
    hal_client_send_return(halclient,  ret, ipstack_strerror(ret));
  }
  hal_ipcmsg_reset(ipcmsg);
  halclient->client[halclient->index].t_read = thread_add_read(halclient->master, hal_client_read_thread, halclient, sock);
  return 0;
}

void hal_client_event(enum hal_client_event_e event, struct hal_client *hal_client, int val, int standby)
{
  int index = 0;
  if(standby)
    index = 1;
  switch (event)
  {
  case HAL_EVENT_SCHEDULE:
    if (!hal_client->client[index].t_connect)
      hal_client->client[index].t_connect =
          thread_add_event(hal_client->master, hal_client_connect_thread, hal_client, 0);
    break;
  case HAL_EVENT_CONNECT:
    if (hal_client->client[index].fail >= 10)
      return;
    if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug))
      zlog_debug(MODULE_HAL, "hal client ipstack_connect schedule interval is %d", hal_client->client[index].fail < 3 ? 10 : 60);
    if (!hal_client->client[index].t_connect)
      hal_client->client[index].t_connect =
          thread_add_timer(hal_client->master, hal_client_connect_thread, hal_client, hal_client->client[index].fail < 3 ? 10 : 60);
    break;
  case HAL_EVENT_READ:
    hal_client->client[index].t_read =
        thread_add_read(hal_client->master, hal_client_read_thread, hal_client, hal_client->client[index].sock);
    break;
  case HAL_EVENT_TIME:
    hal_client->client[index].t_time =
        thread_add_timer(hal_client->master, hal_client_time_thread, hal_client, val);
    break;
  case HAL_EVENT_REGISTER:
    hal_client->client[index].t_time =
        thread_add_timer(hal_client->master, hal_client_register_thread, hal_client, val);
    break;
  case HAL_EVENT_EVTCONNECT:
    if (!hal_client->client[index].t_evt_connect)
      hal_client->client[index].t_evt_connect = thread_add_timer(hal_client->master, hal_client_connect_thread, hal_client, hal_client->client[index].fail < 3 ? 10 : 60);
    break;    
  case HAL_EVENT_EVTTIME:
    hal_client->client[index].t_evt_time =
        thread_add_timer(hal_client->master, hal_client_time_thread, hal_client, val);
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

static int hal_client_hwport_register(struct hal_client *hal_client, zpl_int8 portnum, struct hal_ipcmsg_hwport *tbl)
{
  int i = 0;
  hal_ipcmsg_reset(&hal_client->outmsg);
  hal_ipcmsg_create_header(&hal_client->outmsg, IPCCMD_SET(HAL_MODULE_MGT, HAL_MODULE_CMD_HWPORTTBL, 0));

  hal_ipcmsg_putc(&hal_client->outmsg, portnum);
  for (i = 0; i < portnum; i++)
  {
    hal_ipcmsg_putc(&hal_client->outmsg, tbl[i].unit);
    hal_ipcmsg_putc(&hal_client->outmsg, tbl[i].slot);
    hal_ipcmsg_putc(&hal_client->outmsg, tbl[i].type);
    hal_ipcmsg_putc(&hal_client->outmsg, tbl[i].lport);
    hal_ipcmsg_putl(&hal_client->outmsg, tbl[i].phyid);
  }
  if (IS_HAL_IPCMSG_DEBUG_EVENT(hal_client->debug)&&IS_HAL_IPCMSG_DEBUG_SEND(hal_client->debug))
    zlog_debug(MODULE_HAL, "Client Send Port Table msg unit %d slot %d portnum %d", tbl->unit, tbl->slot, portnum);
  
  return hal_client_send_message(hal_client, HAL_IPCTYPE_CMD, hal_client->index);
}

int hal_client_bsp_register(struct hal_client *hal_client, module_t module, 
    zpl_int8 unit, zpl_int8 slot, zpl_int8 portnum, zpl_char *version)
{
  hal_client->unit = unit;
  hal_client->module = module;
  hal_client->slot = slot;
  hal_client->portnum = portnum;
  strcpy(hal_client->version, version);
  hal_client_msg_register(hal_client, HAL_IPCTYPE_CMD);
  hal_client_msg_register(hal_client, HAL_IPCTYPE_EVENT);
  return OK;
}

int hal_client_bsp_hwport_register(struct hal_client *hal_client, zpl_int8 portnum, struct hal_ipcmsg_hwport *tbl)
{
  hal_client_hwport_register(hal_client,  portnum, tbl);
  return OK;
}

/************************************************************************/

  

#ifndef __HAL_IPC_SRV_H__
#define __HAL_IPC_SRV_H__
#ifdef __cplusplus
extern "C" {
#endif


#include "os_include.h"
#include "hal_ipcmsg.h"


struct hal_ipcsrv
{
    zpl_socket_t       sock;
 
    zpl_socket_t       unixsock;

    struct thread_master *master;
    struct list *client_list;

    struct thread *t_accept;
    struct thread *u_accept;
   
    struct hal_ipcmsg output_msg;
    struct hal_ipcmsg input_msg;
    zpl_uint32  debug;
    os_mutex_t *mutex;
};

struct hal_ipcclient
{
    zpl_socket_t   sock;

    struct thread *t_read;
    struct ipstack_sockaddr_in clientaddr;
    enum hal_client_state state;
    zpl_bool event_client;

    zpl_bool dynamic;       //动态上线
    zpl_int8 module;
    zpl_int8 unit;
    zpl_int8 slot;
    zpl_int8 portnum;
    char     version[128];

    struct hal_ipcsrv *ipcsrv;
    void *board;
};


int hal_ipcsrv_send_message(int unit, zpl_uint32 command, void *msg, int len, int timeout);

int hal_ipcsrv_init(void *m, int port, const char *path, int evport, const char *evpath);
int hal_ipcsrv_exit(void);
#ifdef ZPL_SHELL_MODULE
int hal_ipcsrv_show(void *pvoid);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_SRV_H__ */
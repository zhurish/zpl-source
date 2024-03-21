#ifndef __HAL_IPC_SRV_H__
#define __HAL_IPC_SRV_H__
#ifdef __cplusplus
extern "C" {
#endif


#include "auto_include.h"
#include "zpl_type.h"
#include "os_sem.h"
#include "linklist.h"
#include "hal_ipcmsg.h"



//#define HAL_IPCSRV_SYNC_ACK
#define HAL_IPCSRV_ACK_TIMEOUT  5000//MS


struct hal_ipcsrv
{
    zpl_socket_t       sock;
 
    zpl_socket_t       unixsock;

    struct thread_master *master;
    struct list *client_list;

    struct thread *t_accept;
    struct thread *u_accept;
   
    struct hal_ipcmsg output_msg;
    //struct hal_ipcmsg input_msg;
    zpl_uint32  debug;
    os_mutex_t *mutex;
    zpl_bool    b_init;
#ifdef HAL_IPCSRV_SYNC_ACK
    zpl_socket_t waitfd[2];
    struct timeval  wait_timeval;
    struct hal_ipcmsg ack_msg;
#endif 
};

struct hal_ipcclient
{
    zpl_socket_t   sock;

    struct thread *t_read;
    struct thread *t_hello;
    struct ipstack_sockaddr_in clientaddr;
    enum hal_client_state state;
    enum hal_ipctype_e ipctype;

    zpl_bool dynamic;       //动态上线
    zpl_int8 module;
    zpl_int8 unit;
    zpl_int8 slot;
    zpl_int8 portnum;
    char     version[128];
    struct hal_ipcmsg input_msg;
    struct hal_ipcsrv *ipcsrv;
    void *board;
};


extern int hal_ipcsrv_send_message(int unit, zpl_uint32 command, void *msg, int len);
extern int hal_ipcsrv_copy_send_ipcmsg(int unit, zpl_uint32 command, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcsrv_send_ipcmsg(int unit, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcsrv_send_and_get_message(int unit, zpl_uint32 command, void *msg, int len, struct hal_ipcmsg_result *getvalue);
extern int hal_ipcsrv_getmsg_callback(int unit, zpl_uint32 command, void *msg, int len, struct hal_ipcmsg_result *getvalue, 
    struct hal_ipcmsg_callback *callback);



extern int hal_ipcsrv_init(void *m, int port, const char *path);
extern int hal_ipcsrv_exit(void);
#ifdef ZPL_SHELL_MODULE
extern int hal_ipcsrv_show(void *pvoid);
extern void hal_ipcsrv_cli(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_SRV_H__ */
#ifndef __HAL_CLIENT_H__
#define __HAL_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "auto_include.h"
#include "zpl_type.h"
#include "module.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#define BSP_ENTER_FUNC() zlog_debug(MODULE_BSP, "Into %s line %d", __func__, __LINE__)
#define BSP_LEAVE_FUNC() zlog_debug(MODULE_BSP, "Leave %s line %d", __func__, __LINE__)

    enum hal_client_event_e
    {
        HAL_EVENT_SCHEDULE,
        HAL_EVENT_READ,
        HAL_EVENT_CONNECT,
        HAL_EVENT_TIME,
        HAL_EVENT_REGISTER,
        HAL_EVENT_HWPORT,
        HAL_EVENT_DONE,
    };

    struct hal_client;

    struct hal_client_node
    {
        zpl_socket_t sock;
        zpl_uint16 port; // sock 端口
        char remote[128];

        enum hal_client_state state;
        zpl_uint8 fail;
        zpl_uint32 timeout;
        zpl_uint32 ttl;

        struct thread *t_read;
        struct thread *t_connect;
        struct thread *t_time;
        zpl_bool is_connect;
        zpl_bool is_standby;
        zpl_bool is_event;
        struct hal_ipcmsg ipcmsg;
        struct hal_client *hal_client;
    };
    /* Structure for the hal client. */
    struct hal_client
    {
        module_t module; // 模块ID
        zpl_int8 unit;
        zpl_int8 slot;
        zpl_int8 portnum;
        char version[128];
        // int index;
        struct hal_client_node *current;
        struct hal_client_node master[2];
        struct hal_client_node standby[2];

        struct thread_master *thread_master;
        struct hal_ipcmsg ipcmsg;
        struct hal_ipcmsg outmsg;
        struct hal_ipcmsg evt_outmsg;
        zpl_uint32 debug;
        struct hal_ipcmsg_hwport hwport_table[PHY_PORT_MAX+8];
        int (*bsp_client_msg_handle)(struct hal_client *, zpl_uint32, void *);
        void *bsp_driver;
    };

    /*
     *   hal client   ------------------- > bsp driver -------------------> sdk driver
     *
     */
    /* Prototypes of zebra client service functions. */
    extern struct hal_client *hal_client_create(module_t module, zpl_int8 unit, zpl_int8 slot, zpl_char *version);
    extern int hal_client_hwport_register(struct hal_client *hal_client, zpl_int8 portnum, struct hal_ipcmsg_hwport *tbl);
    extern int hal_client_destroy(struct hal_client *hal_client);
    extern int hal_client_start(struct hal_client *hal_client, char *remote, int port, int standby);
    extern int hal_client_send_report(struct hal_client *hal_client, char *data, int len);
    extern int hal_client_send_return(struct hal_client *hal_client, int ret, const char *fmt, ...);
    extern int hal_client_send_result(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue);
    extern int hal_client_send_result_msg(struct hal_client *hal_client, int ret, struct hal_ipcmsg_result *getvalue,
                                          int subcmd, char *msg, int len);
    extern int hal_client_callback(struct hal_client *, int (*bsp_handle)(struct hal_client *, zpl_uint32, void *), void *);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_CLIENT_H__ */

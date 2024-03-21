/*
 * ipcstandby.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_IPCSTANBY_H__
#define __ZPL_IPCSTANBY_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "os_list.h"
#include "os_sem.h"
#include "stream.h"

#define ZPL_IPCSTANBY_TIMEOUT 2

    enum ipcstanby_cmd
    {
        ZPL_IPCSTANBY_SWITCH,
        ZPL_IPCSTANBY_HELLO,
        ZPL_IPCSTANBY_ACK,
        ZPL_IPCSTANBY_CLI,
        ZPL_IPCSTANBY_MSG,
        ZPL_IPCSTANBY_RES,
    };

    enum ipcstanby_state
    {
        ZPL_IPCSTANBY_STATE_DOWN,
        ZPL_IPCSTANBY_STATE_INIT,
        ZPL_IPCSTANBY_STATE_FULL,
    };
    struct ipcstanby_negotiate
    {
        int ID;           // ID编号
        int seqnum;       // 序列号
        zpl_uint8 state;  // 状态
        zpl_uint8 slot;   // 槽位号
        zpl_uint8 MS;     // 主备协商结果标志
        zpl_uint8 master; // 主备协商结果
        int need_delay;   // 是否需要延迟回切
        int cli_switch;   // 手动切换，手动切换需要通知对端
    };


    struct ipcstanby_result
    {
        int precoss;
        int result;
        int msglen;
    };

    struct ipcstandby_callback
    {
        int (*ipcstandby_callback)(zpl_uint8 *, zpl_uint32, void *);
        void *pVoid;
    };

    struct ipcstanby_reskey
    {
        int key1;
        int key2;
        int key3;
        int key4;
        int key5;
        int key6;
        zpl_uint8 keystr1[32];
        zpl_uint8 keystr2[32];
        zpl_uint8 keystr3[32];
        zpl_uint8 keystr4[32];
        zpl_uint8 keystr5[32];
        zpl_uint8 keystr6[32];
    };

    struct ipcstandby
    {
        zpl_uint32 slot;
        zpl_void *master;
        zpl_taskid_t taskid;
        zpl_void *t_neg_timeout;
        struct ipcstandby_server_t *ipcstandby_server;
        struct ipcstandby_client *ipcstandby_client;

        int (*ipcstandby_change_notify)(zpl_bool master);

        void *_lock;
    };

#define IPCSTANDBY_LOCK()    \
    if (_host_standby._lock) \
    os_mutex_lock(_host_standby._lock, OS_WAIT_FOREVER)

#define IPCSTANDBY_UNLOCK()  \
    if (_host_standby._lock) \
    os_mutex_unlock(_host_standby._lock)

    extern struct ipcstandby _host_standby;

    extern void ipcstandby_create_header(struct stream *s, zpl_uint16 command);
    extern int ipcstandby_active_change_event(int delay);
    extern int ipcstandby_negotiate_timeout_cennel(void);
    extern int ipcstandby_execue_clicmd(char *cmd, int len);
    extern int ipcstandby_sendto_msg(char *msg, int len);
    extern int ipcstandby_request_res(int type, struct ipcstanby_reskey *reskey);

    extern zpl_bool ipcstandby_done(int waitime);
    extern int ipcstandby_change_callback(int (*func)(zpl_bool));

    extern int ipcstandby_cmd_init(void);
    extern int ipcstandby_init(void);
    extern int ipcstandby_exit(void);

    extern int ipcstandby_task_init(void);
    extern int ipcstandby_task_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* __ZPL_IPCSTANBY_H__ */

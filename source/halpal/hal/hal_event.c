#include "auto_include.h"
#include "zpl_type.h"
#include "os_ipstack.h"
#include "os_sem.h"
#include "os_socket.h"
#include "os_netservice.h"
#include "os_list.h"
#include "os_task.h"
#include "module.h"
#include "zmemory.h"
#include "linklist.h"
#include "thread.h"
#include "hal_ipcsrv.h"
#include "hal_ipcmsg.h"
#include "hal_ipccmd.h"
#include "hal_event.h"

struct hal_event_serv _ipcsrv_event;

static void hal_event_client_close(struct hal_event_serv *ipcsrv, struct hal_event_client *client)
{
    if (ipcsrv->mutex)
        os_mutex_lock(ipcsrv->mutex, OS_WAIT_FOREVER);
    listnode_delete(ipcsrv->client_list, client);
    XFREE(MTYPE_HALEVENT, client);
    if (ipcsrv->mutex)
        os_mutex_unlock(ipcsrv->mutex);
}

static int hal_event_client_create(struct hal_event_serv *ipcsrv, zpl_uint8 event, hal_event_callback event_callback, void *event_argv)
{
    struct hal_event_client *client;
    client = XCALLOC(MTYPE_HALEVENT, sizeof(struct hal_event_client));
    if (client == NULL)
        return ERROR;
    /* Make client input/output buffer. */
    client->event = event;
    client->event_callback = event_callback;
    client->event_argv = event_argv;
    if (ipcsrv->mutex)
        os_mutex_lock(ipcsrv->mutex, OS_WAIT_FOREVER);
    listnode_add(ipcsrv->client_list, client);
    if (ipcsrv->mutex)
        os_mutex_unlock(ipcsrv->mutex);
    return OK;
}

int hal_event_init(void)
{
    memset(&_ipcsrv_event, 0, sizeof(struct hal_event_serv));
    _ipcsrv_event.mutex = os_mutex_name_create("haleventmutex");
    if (_ipcsrv_event.mutex == NULL)
    {
        return ERROR;
    }
    _ipcsrv_event.client_list = list_new();
    if (_ipcsrv_event.client_list)
    {
        _ipcsrv_event.client_list->del = NULL;
    }
    else
    {
        os_mutex_destroy(_ipcsrv_event.mutex);
        _ipcsrv_event.mutex = NULL;
        return ERROR;
    }
    return OK;
}

int hal_event_exit(void)
{
    list_delete(_ipcsrv_event.client_list);
    if (_ipcsrv_event.mutex)
    {
        os_mutex_destroy(_ipcsrv_event.mutex);
        _ipcsrv_event.mutex = NULL;
    }
    return OK;
}


void hal_event_close(struct hal_event_client *client)
{
    hal_event_client_close(&_ipcsrv_event, client);
}

int hal_event_install(zpl_uint8 event, hal_event_callback event_callback, void *event_argv)
{
    return hal_event_client_create(&_ipcsrv_event, event, event_callback, event_argv);
}

int hal_event_handler(struct hal_ipcmsg_event *report_event, zpl_uint8 *data, zpl_uint32 len)
{
    struct listnode *node = NULL;
    struct hal_event_client *client = NULL;

    if (_ipcsrv_event.mutex)
        os_mutex_lock(_ipcsrv_event.mutex, OS_WAIT_FOREVER);

    for (ALL_LIST_ELEMENTS_RO(_ipcsrv_event.client_list, node, client))
    {
        if (client && (client->event & report_event->event))
        {
            if(client->event_callback)
             (client->event_callback)(report_event, data, len, client->event_argv);
        }
    }
    if (_ipcsrv_event.mutex)
        os_mutex_unlock(_ipcsrv_event.mutex);
    return OK;
}

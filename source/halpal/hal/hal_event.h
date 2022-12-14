#ifndef __HAL_EVENT_H__
#define __HAL_EVENT_H__
#ifdef __cplusplus
extern "C"
{
#endif


struct hal_ipcmsg_event
{
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 module;
    zpl_uint8 event;
};

typedef int (*hal_event_callback)(struct hal_ipcmsg_event *, zpl_uint8 *, zpl_uint32, void *);

struct hal_event_client
{
    zpl_uint8 event;
    hal_event_callback event_callback;
    void *event_argv;
};

struct hal_event_serv
{
    struct list *client_list;
    os_mutex_t *mutex;
};

int hal_event_init(void);
int hal_event_exit(void);
void hal_event_close(struct hal_event_client *client);
int hal_event_install(zpl_uint8 event, hal_event_callback event_callback, void *event_argv);
int hal_event_handler(struct hal_ipcmsg_event *event, zpl_uint8 *data, zpl_uint32 len);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_EVENT_H__ */
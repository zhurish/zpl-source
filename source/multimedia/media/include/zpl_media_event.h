/*
 * zpl_media_event.h
 *
 *  Created on: Apr 23, 2017
 *      Author: zhurish
 */

#ifndef __ZPL_MEDIA_EVENT_H__
#define __ZPL_MEDIA_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    ZPL_MEDIA_EVENT_NONE = 0x00,			//
    ZPL_MEDIA_EVENT_DISTPATCH = 0x01,
    ZPL_MEDIA_EVENT_CAPTURE = 0x02,		
    ZPL_MEDIA_EVENT_IMAGE   = ZPL_MEDIA_EVENT_CAPTURE,  //抓拍事件
    ZPL_MEDIA_EVENT_RECORD = 0x04,  //录制事件
    ZPL_MEDIA_EVENT_MS = 0x08, //多播发送
    ZPL_MEDIA_EVENT_PROXY = 0x10, //多播发送
} ZPL_MEDIA_EVENT_E;

typedef enum
{
    ZPL_MEDIA_EVENT_FLAG_NONE = 0x00,			//
    ZPL_MEDIA_EVENT_FLAG_ONCE = 0x01,			//
} ZPL_MEDIA_EVENT_FLAG_E;

typedef enum
{
    ZPL_MEDIA_EVENT_STATE_NONE  = 0x00,			//
    ZPL_MEDIA_EVENT_STATE_ACTIVE = 0x01,			//
    ZPL_MEDIA_EVENT_STATE_INACTIVE = 0x02,			//
} ZPL_MEDIA_EVENT_STATE_E;

typedef struct zpl_media_event_s zpl_media_event_t;

typedef int (*eventcb_handler)(zpl_media_event_t*);

struct zpl_media_event_s
{
    NODE            node;
    zpl_uint32      module;             //模块
    zpl_uint32      ev_type;            //
    zpl_uint32      ev_flags;             //
    eventcb_handler evcb_handler;
    zpl_void        *event_data;       //buffer
    zpl_uint8       state;
    zpl_uint32      evid;
};

typedef struct 
{
    void	*sem;
    void	*mutex;
    char    *name;
    zpl_uint32	maxsize;
    LIST	list;
    LIST	ulist;
    zpl_taskid_t taskid;
}zpl_media_event_queue_t;

extern zpl_media_event_queue_t *zpl_media_event_create(const char *name, zpl_uint32 maxsize);
extern zpl_media_event_queue_t *zpl_media_event_default(void);
extern int zpl_media_event_destroy(zpl_media_event_queue_t *queue);
extern int zpl_media_event_start(zpl_media_event_queue_t *queue, int pri, int stacksize);

extern zpl_uint32 zpl_media_event_register_entry(zpl_media_event_queue_t *queue, zpl_uint32 module, zpl_uint32 event,
        eventcb_handler evcb, zpl_void *p, zpl_uint32 flag);

extern int zpl_media_event_unregister_entry(zpl_media_event_queue_t *queue, zpl_uint32 id);
        
extern int zpl_media_event_push(zpl_media_event_queue_t *queue, zpl_media_event_t *data);
extern int zpl_media_event_dispatch(zpl_media_event_queue_t *queue);
extern int zpl_media_event_scheduler(zpl_media_event_queue_t *queue);
extern int zpl_media_event_distribute(zpl_media_event_queue_t *queue);


extern int zpl_media_event_dispatch_signal(zpl_media_channel_t *mchannel, int event);

#define zpl_media_event_register(q,m,t,e,p) zpl_media_event_register_entry(q,m,t,e,p,ZPL_MEDIA_EVENT_FLAG_ONCE)
#define zpl_media_event_register_once(q,m,t,e,p) zpl_media_event_register_entry(q,m,t,e,p,ZPL_MEDIA_EVENT_FLAG_ONCE)
#define zpl_media_event_add(q,m,t,e,p) zpl_media_event_register_entry(q,m,t,e,p,ZPL_MEDIA_EVENT_FLAG_ONCE)
#define zpl_media_event_add_once(q,m,t,e,p) zpl_media_event_register_entry(q,m,t,e,p,ZPL_MEDIA_EVENT_FLAG_ONCE)

#define zpl_media_event_del(q,i) zpl_media_event_unregister_entry(q,i)


#ifdef __cplusplus
}
#endif
#endif /* __ZPL_MEDIA_EVENT_H__ */

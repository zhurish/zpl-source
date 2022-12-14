#ifndef __KHAL_IPC_MSG_H__
#define __KHAL_IPC_MSG_H__
#ifdef __cplusplus
extern "C"
{
#endif

#define HAL_IPCMSG_MAX_PACKET_SIZ 4096
#define HAL_IPCMSG_VERSION 1
#define HAL_IPCMSG_HEADER_MARKER 255


#define HAL_ENTER_FUNC()
// printk("Into %s line %d", __func__, __LINE__)
#define HAL_LEAVE_FUNC() 
//printk("Leave %s line %d", __func__, __LINE__)


enum khal_ipctype_e
{
    HAL_IPCTYPE_NONE,
    HAL_IPCTYPE_EVENT,
    HAL_IPCTYPE_CMD,
};

#pragma pack(1)
struct khal_ipcmsg_header
{
    zpl_uint16 length;
    zpl_uint8 marker;
    zpl_uint8 version;
    zpl_uint32 command;
    zpl_uint8 from_unit;
    zpl_uint8 to_unit;
};
 
#define HAL_IPCMSG_HEADER_SIZE sizeof(struct khal_ipcmsg_header)
struct khal_ipcmsg_callback
{
    int (*ipcmsg_callback)(zpl_uint8 *, zpl_uint32, void *);
    void  *pVoid;
};

//应答消息
struct khal_ipcmsg_result
{
    zpl_int32 result;
    zpl_uint32 vrfid;
    zpl_uint32 l3ifid;
    zpl_uint32 nhid;
    zpl_uint32 routeid;
    zpl_uint32 resource;
    zpl_uint32 state;
    zpl_uint32 value;
};

//hello 消息
struct khal_ipcmsg_hello
{
    zpl_uint8 stype;
    zpl_uint8 module;
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 portnum;
    zpl_int8 version[128];
};
  
//交换芯片物理接口表信息
struct khal_ipcmsg_hwport
{
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 type;
    zpl_uint8 lport;
    zpl_phyport_t phyid;
};
    
//注册信息
struct khal_ipcmsg_register
{
    zpl_int32 value;
};
  
//全局信息
typedef struct khal_global_header_s
{
    zpl_uint32 vrfid;
    zpl_uint32 value;
    zpl_uint32 value1;
    zpl_uint32 value2;
} khal_global_header_t;

//L2接口信息
typedef struct khal_port_header_s
{
    zpl_uint8     unit;
    zpl_uint8     slot;
    zpl_uint8     type;

    zpl_uint32    lport;
    zpl_phyport_t phyport; 
    zpl_uint32    l3ifindex;
    zpl_uint32    vrfid;  
} khal_port_header_t;

#define IF_TYPE_SET(n) (((n)&0x3F) << 26)
#define IF_TYPE_CLR(n) (((n)) & 0x03FFFFFF)
#define IF_USPV_SET(u, s, p, v) (((u)&0x07) << 23) | (((s)&0x1F) << 18) | (((p)&0x3F) << 12) | ((v)&0x0FFF)

#define IF_INFINDEX_GET(n)	IF_TYPE_SET((n)->type)|IF_USPV_SET((n)->unit, (n)->slot, (n)->lport, 0)


#pragma pack(0)

#define HAL_IPCMSG_DEBUG_EVENT  0x10
#define HAL_IPCMSG_DEBUG_PACKET 0x01
#define HAL_IPCMSG_DEBUG_SEND   0x02
#define HAL_IPCMSG_DEBUG_RECV   0x04
#define HAL_IPCMSG_DEBUG_DETAIL 0x08
#define HAL_IPCMSG_DEBUG_HEX    0x00100000

#define IS_HAL_IPCMSG_DEBUG_EVENT(n) ((n)&HAL_IPCMSG_DEBUG_EVENT)
#define IS_HAL_IPCMSG_DEBUG_PACKET(n) ((n)&HAL_IPCMSG_DEBUG_PACKET)
#define IS_HAL_IPCMSG_DEBUG_SEND(n) ((n)&HAL_IPCMSG_DEBUG_SEND)
#define IS_HAL_IPCMSG_DEBUG_RECV(n) ((n)&HAL_IPCMSG_DEBUG_RECV)
#define IS_HAL_IPCMSG_DEBUG_DETAIL(n) ((n)&HAL_IPCMSG_DEBUG_DETAIL)
#define IS_HAL_IPCMSG_DEBUG_HEX(n) ((n)&HAL_IPCMSG_DEBUG_HEX)

enum khal_client_state
{
    HAL_CLIENT_NONE,
    HAL_CLIENT_CLOSE,
    HAL_CLIENT_CONNECT,
    HAL_CLIENT_INIT,
    HAL_CLIENT_REGISTER,
};

struct khal_ipcmsg
{
    char *buf;
    zpl_uint16 length_max;
    zpl_uint16 getp;
    zpl_uint16 setp;
};

extern int khal_ipcmsg_put(struct khal_ipcmsg *ipcmsg, void *buf, int len);
extern int khal_ipcmsg_putc(struct khal_ipcmsg *ipcmsg, zpl_uint8);
extern int khal_ipcmsg_putw(struct khal_ipcmsg *ipcmsg, zpl_uint16);
extern int khal_ipcmsg_putl(struct khal_ipcmsg *ipcmsg, zpl_uint32);
extern int khal_ipcmsg_put_reset(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_putnull(struct khal_ipcmsg *ipcmsg, zpl_uint32);
extern int khal_ipcmsg_putc_at(struct khal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint8 c);
extern int khal_ipcmsg_putw_at(struct khal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint16 w);

extern int khal_ipcmsg_get(struct khal_ipcmsg *ipcmsg, void *buf, int len);
extern int khal_ipcmsg_getc(struct khal_ipcmsg *ipcmsg, zpl_uint8 *);
extern int khal_ipcmsg_getw(struct khal_ipcmsg *ipcmsg, zpl_uint16 *);
extern int khal_ipcmsg_getl(struct khal_ipcmsg *ipcmsg, zpl_uint32 *);
extern int khal_ipcmsg_get_reset(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_getnull(struct khal_ipcmsg *ipcmsg, zpl_uint32 len);

extern int khal_ipcmsg_create(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_destroy(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_reset(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_msg_init(struct khal_ipcmsg *ipcmsg, char *buf, int len);
extern int khal_ipcmsg_create_header(struct khal_ipcmsg *ipcmsg, zpl_uint32 command);
extern int khal_ipcmsg_get_header(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_header *header);
extern int khal_ipcmsg_hdr_unit_set(struct khal_ipcmsg *ipcmsg, zpl_uint32 unit);
extern int khal_ipcmsg_hdr_unit_get(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_msg_copy(struct khal_ipcmsg *dst_ipcmsg, struct khal_ipcmsg *src_ipcmsg);
extern int khal_ipcmsg_msg_clone(struct khal_ipcmsg *dst_ipcmsg, struct khal_ipcmsg *src_ipcmsg);
extern int khal_ipcmsg_hdrlen_set(struct khal_ipcmsg *ipcmsg);

extern int khal_ipcmsg_msglen_get(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_get_setp(struct khal_ipcmsg *ipcmsg);
extern int khal_ipcmsg_get_getp(struct khal_ipcmsg *ipcmsg);


extern int khal_ipcmsg_global_set(struct khal_ipcmsg *ipcmsg, khal_global_header_t *glo);
extern int khal_ipcmsg_global_get(struct khal_ipcmsg *ipcmsg, khal_global_header_t *glo);

extern int khal_ipcmsg_port_get(struct khal_ipcmsg *ipcmsg, khal_port_header_t *bspport);

extern int khal_ipcmsg_result_set(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_result *val);
extern int khal_ipcmsg_result_get(struct khal_ipcmsg *ipcmsg, struct khal_ipcmsg_result *val);

#ifdef __cplusplus
}
#endif

#endif /* __KHAL_IPC_MSG_H__ */
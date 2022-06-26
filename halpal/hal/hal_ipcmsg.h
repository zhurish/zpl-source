#ifndef __HAL_IPC_MSG_H__
#define __HAL_IPC_MSG_H__
#ifdef __cplusplus
extern "C"
{
#endif

#define HAL_IPCMSG_MAX_PACKET_SIZ 4096
#define HAL_IPCMSG_VERSION 1
#define HAL_IPCMSG_HEADER_MARKER 255

#define HAL_IPCMSG_CMD_PATH SYSVARDIR "/halipcsrv-cmd.sock"
#define HAL_IPCMSG_EVENT_PATH SYSVARDIR "/halipcsrv-event.sock"
#define HAL_IPCMSG_CMD_PORT 65300
#define HAL_IPCMSG_EVENT_PORT 65301

#define HAL_ENTER_FUNC() 
//zlog_debug(MODULE_HAL, "Into %s line %d", __func__, __LINE__)
#define HAL_LEAVE_FUNC() 
//zlog_debug(MODULE_HAL, "Leave %s line %d", __func__, __LINE__)


enum hal_ipcmsg_type
{
    HAL_IPCMSG_NONE,
    HAL_IPCMSG_EVENT,
    HAL_IPCMSG_CMD,
};

#pragma pack(1)
struct hal_ipcmsg_header
{
    zpl_uint16 length;
    zpl_uint8 marker;
    zpl_uint8 version;
    zpl_uint32 command;
    zpl_uint32 unit;
};
 
#define HAL_IPCMSG_HEADER_SIZE sizeof(struct hal_ipcmsg_header)
struct hal_ipcmsg_callback
{
    int (*ipcmsg_callback)(zpl_uint8 *, zpl_uint32, void *);
    void  *pVoid;
};

//应答消息
struct hal_ipcmsg_result
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
struct hal_ipcmsg_hello
{
    zpl_uint8 stype;
    zpl_uint8 module;
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 portnum;
    zpl_int8 version[128];
};
  
//交换芯片物理接口表信息
struct hal_ipcmsg_hwport
{
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 type;
    zpl_uint8 port;
    zpl_phyport_t phyid;
};
    
//注册信息
struct hal_ipcmsg_register
{
    zpl_int32 value;
};
  
//全局信息
typedef struct hal_global_header_s
{
    zpl_uint32 vrfid;
    zpl_uint32 value;
    zpl_uint32 value1;
    zpl_uint32 value2;
} hal_global_header_t;

//L2接口信息
typedef struct hal_port_header_s
{
    zpl_uint8     unit;
    zpl_uint8     slot;
    zpl_uint8     type;
    //union
    //{
      zpl_uint32    lgport;
      zpl_phyport_t phyport; 
      zpl_uint32    l3ifindex;
    //} uport;
    zpl_uint32    vrfid;  
} hal_port_header_t;

//#define port_lgport      uport.lgport
//#define port_phyport     uport.phyport
//#define port_l3ifindex   uport.l3ifindex

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

enum hal_client_state
{
    HAL_CLIENT_NONE,
    HAL_CLIENT_CLOSE,
    HAL_CLIENT_CONNECT,
    HAL_CLIENT_INIT,
    HAL_CLIENT_REGISTER,
};

struct hal_ipcmsg
{
    char *buf;
    zpl_uint16 length_max;
    zpl_uint16 getp;
    zpl_uint16 setp;
};

extern int hal_ipcmsg_put(struct hal_ipcmsg *ipcmsg, void *buf, int len);
extern int hal_ipcmsg_putc(struct hal_ipcmsg *ipcmsg, zpl_uint8);
extern int hal_ipcmsg_putw(struct hal_ipcmsg *ipcmsg, zpl_uint16);
extern int hal_ipcmsg_putl(struct hal_ipcmsg *ipcmsg, zpl_uint32);
extern int hal_ipcmsg_put_reset(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_putnull(struct hal_ipcmsg *ipcmsg, zpl_uint32);
extern int hal_ipcmsg_putc_at(struct hal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint8 c);
extern int hal_ipcmsg_putw_at(struct hal_ipcmsg *ipcmsg, zpl_uint32 s, zpl_uint16 w);

extern int hal_ipcmsg_get(struct hal_ipcmsg *ipcmsg, void *buf, int len);
extern int hal_ipcmsg_getc(struct hal_ipcmsg *ipcmsg, zpl_uint8 *);
extern int hal_ipcmsg_getw(struct hal_ipcmsg *ipcmsg, zpl_uint16 *);
extern int hal_ipcmsg_getl(struct hal_ipcmsg *ipcmsg, zpl_uint32 *);
extern int hal_ipcmsg_get_reset(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_getnull(struct hal_ipcmsg *ipcmsg, zpl_uint32 len);

extern int hal_ipcmsg_create(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_destroy(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_reset(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_msg_init(struct hal_ipcmsg *ipcmsg, char *buf, int len);
extern int hal_ipcmsg_create_header(struct hal_ipcmsg *ipcmsg, zpl_uint32 command);
extern int hal_ipcmsg_get_header(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_header *header);
extern int hal_ipcmsg_hdr_unit_set(struct hal_ipcmsg *ipcmsg, zpl_uint32 unit);
extern int hal_ipcmsg_hdr_unit_get(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_msg_copy(struct hal_ipcmsg *dst_ipcmsg, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcmsg_msg_clone(struct hal_ipcmsg *dst_ipcmsg, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcmsg_hdrlen_set(struct hal_ipcmsg *ipcmsg);

extern int hal_ipcmsg_msglen_get(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_get_setp(struct hal_ipcmsg *ipcmsg);
extern int hal_ipcmsg_get_getp(struct hal_ipcmsg *ipcmsg);

extern int hal_ipcmsg_send_message(int unit, zpl_uint32 command, void *ipcmsg, int len);
extern int hal_ipcmsg_send_cmd(int unit, zpl_uint32 command, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcmsg_send(int unit, struct hal_ipcmsg *src_ipcmsg);
extern int hal_ipcmsg_send_andget_message(int unit, zpl_uint32 command, 
  void *ipcmsg, int len,  struct hal_ipcmsg_result *getvalue);
extern int hal_ipcmsg_getmsg_callback(int unit, zpl_uint32 command, void *ipcmsg, int len, 
  struct hal_ipcmsg_result *getvalue, struct hal_ipcmsg_callback *callback);

extern int hal_ipcmsg_hexmsg(struct hal_ipcmsg *ipcmsg, zpl_uint32 len, char *hdr);

extern int hal_ipcmsg_global_set(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo);
extern int hal_ipcmsg_global_get(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo);

extern int hal_ipcmsg_port_set(struct hal_ipcmsg *ipcmsg, ifindex_t ifindex);
extern int hal_ipcmsg_port_get(struct hal_ipcmsg *ipcmsg, hal_port_header_t *bspport);

extern int hal_ipcmsg_result_set(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *val);
extern int hal_ipcmsg_result_get(struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *val);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_MSG_H__ */
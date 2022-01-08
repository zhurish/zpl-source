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

  struct hal_ipcmsg_result
  {
    zpl_int32 result;
  };

  struct hal_ipcmsg_hello
  {
    zpl_uint8 module;
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 portnum;
    zpl_uint8 version[128];
  };

  struct hal_ipcmsg_porttbl
  {
    zpl_uint8 port;
    zpl_uint32 phyid;
  };
  struct hal_ipcmsg_register
  {
    zpl_uint8 type;
    zpl_uint8 unit;
    zpl_uint8 slot;
    zpl_uint8 portnum;
    /*
     * port table data
     */
  };

  typedef struct hal_global_header_s
  {
    zpl_uint32 vrfid;
    zpl_vlan_t vlanid;
  } hal_global_header_t;
#define HAL_GLOBAL_HDR_SIZE (sizeof(hal_global_header_t))

  typedef struct hal_port_header_s
  {
    zpl_uint32 ifindex;
    zpl_uint32 vrfid;
    zpl_vlan_t vlanid;
    zpl_phyport_t phyport;
  } hal_port_header_t;

#define HAL_PORT_HDR_SIZE (sizeof(hal_port_header_t))

  typedef struct hal_table_header_s
  {
    zpl_uint32 vrfid;
    zpl_vlan_t vlanid;
    zpl_uint32 table;
  } hal_table_header_t;

#define HAL_TABLE_HDR_SIZE (sizeof(hal_table_header_t))

#pragma pack(0)

#define HAL_IPCMSG_DEBUG_EVENT 0x10
#define HAL_IPCMSG_DEBUG_PACKET 0x01
#define HAL_IPCMSG_DEBUG_SEND 0x02
#define HAL_IPCMSG_DEBUG_RECV 0x04
#define HAL_IPCMSG_DEBUG_DETAIL 0x08

#define IS_HAL_IPCMSG_DEBUG_EVENT(n) ((n)&HAL_IPCMSG_DEBUG_EVENT)
#define IS_HAL_IPCMSG_DEBUG_PACKET(n) ((n)&HAL_IPCMSG_DEBUG_PACKET)
#define IS_HAL_IPCMSG_DEBUG_SEND(n) ((n)&HAL_IPCMSG_DEBUG_SEND)
#define IS_HAL_IPCMSG_DEBUG_RECV(n) ((n)&HAL_IPCMSG_DEBUG_RECV)
#define IS_HAL_IPCMSG_DEBUG_DETAIL(n) ((n)&HAL_IPCMSG_DEBUG_DETAIL)

  enum hal_client_state
  {
    HAL_CLIENT_NONE,
    HAL_CLIENT_CONNECT,
    HAL_CLIENT_CLOSE,
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
  extern int hal_ipcmsg_hdr_unit_set(struct hal_ipcmsg *ipcmsg, zpl_uint32 unit);
  extern int hal_ipcmsg_hdr_unit_get(struct hal_ipcmsg *ipcmsg);
  extern int hal_ipcmsg_msg_add(struct hal_ipcmsg *ipcmsg, void *msg, int len);
  extern int hal_ipcmsg_msglen_set(struct hal_ipcmsg *ipcmsg);
  extern int hal_ipcmsg_msglen_get(struct hal_ipcmsg *ipcmsg);
  extern int hal_ipcmsg_send_message(int unit, zpl_uint32 command, void *ipcmsg, int len);

  extern int hal_ipcmsg_global_set(ifindex_t ifindex, hal_global_header_t *glo);
  extern int hal_ipcmsg_port_set(struct hal_ipcmsg *ipcmsg, ifindex_t ifindex);
  extern int hal_ipcmsg_table_set(ifindex_t ifindex, hal_table_header_t *table);

  extern int hal_ipcmsg_global_get(struct hal_ipcmsg *ipcmsg, hal_global_header_t *glo);
  extern int hal_ipcmsg_port_get(struct hal_ipcmsg *ipcmsg, hal_port_header_t *bspport);
  extern int hal_ipcmsg_table_get(struct hal_ipcmsg *ipcmsg, hal_table_header_t *table);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_MSG_H__ */
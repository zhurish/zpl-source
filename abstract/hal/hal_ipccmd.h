#ifndef __HAL_IPC_CMD_H__
#define __HAL_IPC_CMD_H__
#ifdef __cplusplus
extern "C" {
#endif



enum hal_module_id
{
    HAL_MODULE_NONE,
	HAL_MODULE_MGT,
	HAL_MODULE_GLOBAL,
	HAL_MODULE_SWITCH,
    HAL_MODULE_CPU,
	HAL_MODULE_PORT,
    HAL_MODULE_IFADDR,
    HAL_MODULE_ROUTE,
    HAL_MODULE_STP,
    HAL_MODULE_MSTP,
    HAL_MODULE_8021X,
    HAL_MODULE_IGMP,
    HAL_MODULE_DOS,
    HAL_MODULE_MAC,
    HAL_MODULE_MIRROR,
    HAL_MODULE_QINQ,
    HAL_MODULE_VLAN,
    HAL_MODULE_QOS,
    HAL_MODULE_ACL,
    HAL_MODULE_TRUNK,
    HAL_MODULE_ARP,
    HAL_MODULE_BRIDGE,
    HAL_MODULE_PPP,
    HAL_MODULE_SECURITY,
    HAL_MODULE_SNMP,
    HAL_MODULE_VRF,
    HAL_MODULE_MPLS,
    HAL_MODULE_STATISTICS,//Statistics
    HAL_MODULE_EVENT,
    HAL_MODULE_STATUS,
    HAL_MODULE_MAX,
};

enum hal_module_cmd 
{
    HAL_MODULE_CMD_NONE,
    HAL_MODULE_CMD_DATA,        //数据
	HAL_MODULE_CMD_REQ,         //设置
	HAL_MODULE_CMD_ACK,         //应答
 
	HAL_MODULE_CMD_REPORT,      //上报
	HAL_MODULE_CMD_HELLO,       //探测
    HAL_MODULE_CMD_KEEPALIVE,   //心跳
	HAL_MODULE_CMD_INIT,        //初始化
    HAL_MODULE_CMD_PORTTBL,
    HAL_MODULE_CMD_REGISTER,    //注册
    HAL_MODULE_CMD_MAX,
};

#define HAL_CALLBACK_ENTRY(m, f) { (m), (f), (#f) }
typedef struct 
{
    int module;
    int (*module_handle)(void*, zpl_uint32, zpl_uint32, void *);
    const char *name;
}hal_ipccmd_callback_t;

typedef struct 
{
    int subcmd;
    int (*cmd_handle)(void *, void *, void *);
    const char *name;
}hal_ipcsubcmd_callback_t;

#define IPCCMD_SET(m,s,c)               (((m)&0xff) << 24)|(((s)&0xFF)<<16)|((c)&0xFFFF)
#define IPCCMD_MODULE_GET(C)            (((C) >> 24)&0xFF)
#define IPCCMD_CMD_GET(C)             (((C) >> 16)&0xFF)
#define IPCCMD_SUBCMD_GET(C)               ((C)&0xffFF)

extern const char * hal_module_cmd_name(zpl_uint32 cmd);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_CMD_H__ */
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
	HAL_MODULE_CMD_SET,
	HAL_MODULE_CMD_GET,
	HAL_MODULE_CMD_ADD,
	HAL_MODULE_CMD_DEL,   
    HAL_MODULE_CMD_DELALL,
	HAL_MODULE_CMD_ENABLE,
	HAL_MODULE_CMD_DISABLE,        
	HAL_MODULE_CMD_REPORT,
	HAL_MODULE_CMD_HELLO,
    HAL_MODULE_CMD_REGISTER,
    HAL_MODULE_CMD_KEEPALIVE,
	HAL_MODULE_CMD_ACK,
    HAL_MODULE_CMD_MAX,
};



#define IPCCMD_SET(m,s,c)               (((m)&0xff) << 24)|(((s)&0xFF)<<16)|((c)&0xFFFF)
#define IPCCMD_MODULE_GET(C)            (((C) >> 24)&0xFF)
#define IPCCMD_CMD_GET(C)             (((C) >> 16)&0xFF)
#define IPCCMD_SUBCMD_GET(C)               ((C)&0xffFF)

extern const char * hal_module_cmd_name(zpl_uint32 cmd);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_IPC_CMD_H__ */
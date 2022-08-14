#include "kbsp_types.h"
#include "khal_ipcmsg.h"
#include "khal_ipccmd.h"




#define DESC_ENTRY(T) [(T)] = {(T), (#T)}
static const struct kmessage khal_module_types[] = {
    DESC_ENTRY(HAL_MODULE_NONE),
    DESC_ENTRY(HAL_MODULE_MGT),
    DESC_ENTRY(HAL_MODULE_GLOBAL),
    DESC_ENTRY(HAL_MODULE_SWITCH),
    DESC_ENTRY(HAL_MODULE_PORT),
    DESC_ENTRY(HAL_MODULE_L3IF),
    DESC_ENTRY(HAL_MODULE_ROUTE),
    DESC_ENTRY(HAL_MODULE_STP),
    DESC_ENTRY(HAL_MODULE_8021X),
    DESC_ENTRY(HAL_MODULE_IGMP),
    DESC_ENTRY(HAL_MODULE_DOS),
    DESC_ENTRY(HAL_MODULE_MAC),
    DESC_ENTRY(HAL_MODULE_MIRROR),
    DESC_ENTRY(HAL_MODULE_QINQ),
    DESC_ENTRY(HAL_MODULE_VLAN),
    DESC_ENTRY(HAL_MODULE_QOS),
    DESC_ENTRY(HAL_MODULE_ACL),
    DESC_ENTRY(HAL_MODULE_TRUNK),
    DESC_ENTRY(HAL_MODULE_ARP),
    DESC_ENTRY(HAL_MODULE_BRIDGE),
    DESC_ENTRY(HAL_MODULE_PPP),
    DESC_ENTRY(HAL_MODULE_SECURITY),
    DESC_ENTRY(HAL_MODULE_SNMP),
    DESC_ENTRY(HAL_MODULE_VRF),
    DESC_ENTRY(HAL_MODULE_MPLS),
    DESC_ENTRY(HAL_MODULE_STATISTICS), // Statistics
    DESC_ENTRY(HAL_MODULE_EVENT),
    DESC_ENTRY(HAL_MODULE_STATUS),
    DESC_ENTRY(HAL_MODULE_MAX),
};

#undef DESC_ENTRY


static const char * _khal_cmd_namestr(int cmd)
{
    switch(cmd)
    {
    case HAL_MODULE_CMD_NONE:
    return "NONE";
    case HAL_MODULE_CMD_DATA:
    return "DATA";    
	case HAL_MODULE_CMD_REQ:
    return "SET";
	case HAL_MODULE_CMD_ACK:
    return "ACK";
	case HAL_MODULE_CMD_GET:
    return "ACK";    
	case HAL_MODULE_CMD_REPORT:
    return "REPORT";
	case HAL_MODULE_CMD_HELLO:
    return "HELLO";
	case HAL_MODULE_CMD_REGISTER:
    return "REGISTER";
	case HAL_MODULE_CMD_KEEPALIVE:
    return "KEEPALIVE";
	case HAL_MODULE_CMD_STARTONDE:
    return "START DONE";
 	case HAL_MODULE_CMD_HWPORTTBL:
    return "PORTTBL";   
	case HAL_MODULE_CMD_MAX:
    return "MAX";
    default:
    return NULL;//itoa();
    }
    return NULL;
}


static const char * _khal_module_namestr(int cmd)
{
    if (cmd >= ZPL_ARRAY_SIZE(khal_module_types)) {
		return NULL;
	}
	return khal_module_types[cmd].str;
}



const char * khal_module_cmd_name(zpl_uint32 cmd)
{
    int n = 0;
    static char bustmp[256];
    memset(bustmp, 0, sizeof(bustmp));//unknown
    if(_khal_module_namestr(IPCCMD_MODULE_GET(cmd)))
        n = snprintf(bustmp, sizeof(bustmp), "%s ", _khal_module_namestr(IPCCMD_MODULE_GET(cmd)));
    else  
        n = snprintf(bustmp, sizeof(bustmp), "unknown:0x%x ", (IPCCMD_MODULE_GET(cmd)));  
    
    if(_khal_cmd_namestr(IPCCMD_CMD_GET(cmd)))
        n = snprintf(bustmp + n, sizeof(bustmp) - n, "%s", _khal_cmd_namestr(IPCCMD_CMD_GET(cmd)));
    else  
        n = snprintf(bustmp + n, sizeof(bustmp) - n, "unknown:0x%x", (IPCCMD_CMD_GET(cmd)));  
    return bustmp;
}


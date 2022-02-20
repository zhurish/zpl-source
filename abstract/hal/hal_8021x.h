/*
 * hal_8021x.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __HAL_8021X_H__
#define __HAL_8021X_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Definition for 802.1X callout functions. */
typedef void (*hal_8021x_auth_cb_t)(
    void *cookie, 
    int unit, 
    int port, 
    int reason);

/* Features that can be controlled for EAP packets. */
typedef enum hal_auth_mac_control_e {
    halEapControlL2UserAddr,    /* enable L2 User Address frame bypass EAP Port
                                   State Filter and SA Filter. */
    halEapControlDHCP,          /* enable DHCP frame bypass EAP Port State
                                   Filter and SA Filter. */
    halEapControlARP,           /* enable ARP frame bypass EAP Port State Filter
                                   and SA Filter. */
    halEapControlMAC2X,         /* enable(DA=01-80-c2-00-00-22,23,....,2f) frame
                                   bypass EAP Port State Filter and SA Filter. */
    halEapControlGVRP,          /* enable(DA=01-80-c2-00-00-21) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    halEapControlGMRP,          /* enable(DA=01-80-c2-00-00-20) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    halEapControlMAC1X,         /* enable(DA=01-80-c2-00-00-11,12,....,1f) frame
                                   bypass EAP Port State FIlter and SA Filter. */
    halEapControlAllBridges,    /* enable(DA=01-80-c2-00-00-10) frame bypass EAP
                                   Port State FIlter and SA Filter. */
    halEapControlMAC0X,         /* enable(DA=01-80-c2-00-00-02)or
                                   (DA=01-80-c2-00-00-04,05,....,0f) frame
                                   bypass EAP Port State FIlter and SA Filter. */
    halEapControlMACBPDU        /* enable BPDU frame bypass EAP Port State
                                   FIlter and SA Filter. */
} hal_auth_mac_control_t;


enum hal_8021x_cmd 
{
    HAL_8021X_NONE,
	HAL_8021X,
	HAL_8021X_PORT_MODE,
	HAL_8021X_PORT_MAC,
	HAL_8021X_PORT_STATE,
	HAL_8021X_PORT_BYPASS,
    HAL_8021X_PORT,
};

typedef struct hal_8021x_param_s
{
	union hal_8021x
	{
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
	}u;

}hal_8021x_param_t;

/*全局使能*/
extern int hal_8021x_enable(zpl_bool enable);
/*端口使能*/
extern int hal_8021x_interface_enable(ifindex_t ifindex, zpl_bool enable);
/*端口认证模式*/
extern int hal_8021x_interface_mode(ifindex_t ifindex, zpl_uint32 value);
/*端口认证状态*/
extern int hal_8021x_interface_state(ifindex_t ifindex, zpl_uint32 value);

/*端口认证模式下通过报文*/
extern int hal_8021x_auth_bypass(ifindex_t ifindex, zpl_uint32 value);

extern int hal_8021x_interface_addmac(ifindex_t ifindex, mac_t *mac);
extern int hal_8021x_interface_delmac(ifindex_t ifindex, mac_t *mac);
extern int hal_8021x_interface_delallmac(ifindex_t ifindex);




#ifdef __cplusplus
}
#endif
#endif /* __HAL_8021X_H__ */

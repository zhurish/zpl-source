/*
 * bsp_mac.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MAC_H__
#define __BSP_MAC_H__
#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER
enum hal_mac_cmd 
{
    HAL_MAC_CMD_NONE,
	HAL_MAC_CMD_AGE,
	HAL_MAC_CMD_ADD,
	HAL_MAC_CMD_DEL,
	HAL_MAC_CMD_CLEAR,
	HAL_MAC_CMD_READ,
    HAL_MAC_CMD_MAX,
};

#pragma pack(1)
typedef struct hal_mac_tbl_s
{
	zpl_phyport_t	phyport;
	vlan_t			vlan;
	mac_t 			mac[NSM_MAC_MAX];
	vrf_id_t		vrfid;
	zpl_uint8 		is_valid:1;
	zpl_uint8 		is_age:1;
	zpl_uint8 		is_static:1;
}hal_mac_tbl_t;
#pragma pack(0)

typedef struct mac_table_info_s
{
	hal_mac_tbl_t *mactbl;
	zpl_uint32 macnum;
}mac_table_info_t;

typedef struct hal_mac_param_s
{
	vlan_t vlan;
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
	mac_table_info_t table;
}hal_mac_param_t;
#endif
typedef struct sdk_mac_s
{
	int (*sdk_mac_age_cb) (void *, zpl_uint32);
	int (*sdk_mac_add_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 , mac_t *, zpl_uint32 );
	int (*sdk_mac_del_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 , mac_t *, zpl_uint32 );
	int (*sdk_mac_clr_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32 );
	int (*sdk_mac_read_cb) (void *, zpl_phyport_t , zpl_vlan_t , zpl_uint32, mac_table_info_t *);
}sdk_mac_t;

extern sdk_mac_t sdk_maccb;
extern int bsp_mac_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MAC_H__ */

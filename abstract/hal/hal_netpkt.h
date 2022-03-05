/*
 * hal_netpkt.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_NETPKT_H__
#define __HAL_NETPKT_H__

#ifdef __cplusplus
extern "C" {
#endif


int hal_netpkt_send(ifindex_t ifindex, zpl_vlan_t vlanid, 
	zpl_uint8 pri, zpl_uchar *data, zpl_uint32 len);
    
#ifdef __cplusplus
}
#endif

#endif /* __HAL_NETPKT_H__ */

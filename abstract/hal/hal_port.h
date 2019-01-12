/*
 * hal_port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_PORT_H_
#define ABSTRACT_HAL_HAL_PORT_H_


typedef struct sdk_port_s
{
  int (*sdk_port_up_cb) (ifindex_t);
  int (*sdk_port_down_cb) (ifindex_t);

  int (*sdk_port_set_address_cb) (ifindex_t, struct prefix *cp, int secondry);
  int (*sdk_port_unset_address_cb) (ifindex_t, struct prefix *cp, int secondry);

  int (*sdk_port_mac_cb) (ifindex_t, unsigned char *, int);

  int (*sdk_port_mtu_cb) (ifindex_t, int);
  int (*sdk_port_metric_cb) (ifindex_t, int);
  int (*sdk_port_vrf_cb) (ifindex_t, int);
  int (*sdk_port_multicast_cb) (ifindex_t, int);
  int (*sdk_port_bandwidth_cb) (ifindex_t, int);
  int (*sdk_port_speed_cb) (ifindex_t, int);

  int (*sdk_port_mode_cb) (ifindex_t, int);

  int (*sdk_port_linkdetect_cb) (ifindex_t, int);

  int (*sdk_port_stp_cb) (ifindex_t, int);
  int (*sdk_port_loop_cb) (ifindex_t, int);
  int (*sdk_port_8021x_cb) (ifindex_t, int);
  int (*sdk_port_duplex_cb) (ifindex_t, int);
}sdk_port_t;


extern int hal_port_up(ifindex_t ifindex);
extern int hal_port_down(ifindex_t ifindex);
extern int hal_port_address_set(ifindex_t ifindex, struct prefix *cp, int secondry);
extern int hal_port_address_unset(ifindex_t ifindex, struct prefix *cp, int secondry);
extern int hal_port_mac_set(ifindex_t ifindex, unsigned char *cp, int secondry);
extern int hal_port_mtu_set(ifindex_t ifindex, int value);
extern int hal_port_metric_set(ifindex_t ifindex, int value);
extern int hal_port_vrf_set(ifindex_t ifindex, int value);
extern int hal_port_multicast_set(ifindex_t ifindex, int value);
extern int hal_port_bandwidth_set(ifindex_t ifindex, int value);
extern int hal_port_speed_set(ifindex_t ifindex, int value);
extern int hal_port_duplex_set(ifindex_t ifindex, int value);
extern int hal_port_mode_set(ifindex_t ifindex, int value);
extern int hal_port_linkdetect_set(ifindex_t ifindex, int value);
extern int hal_port_stp_set(ifindex_t ifindex, int value);
extern int hal_port_loop_set(ifindex_t ifindex, int value);
extern int hal_port_8021x_set(ifindex_t ifindex, int value);

#endif /* ABSTRACT_HAL_HAL_PORT_H_ */

/*
 * bsp_port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_PORT_H__
#define __BSP_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER
struct if_stats
{
   zpl_ulong rx_packets;   /* total packets received       */
   zpl_ulong tx_packets;   /* total packets transmitted    */
   zpl_ulong rx_bytes;     /* total bytes received         */
   zpl_ulong tx_bytes;     /* total bytes transmitted      */
   zpl_ulong rx_errors;    /* bad packets received         */
   zpl_ulong tx_errors;    /* packet transmit problems     */
   zpl_ulong rx_dropped;   /* no space in linux buffers    */
   zpl_ulong tx_dropped;   /* no space available in linux  */
   zpl_ulong rx_multicast; /* multicast packets received   */
   zpl_ulong collisions;

   /* detailed rx_errors: */
   zpl_ulong rx_length_errors;
   zpl_ulong rx_over_errors;   /* receiver ring buff overflow  */
   zpl_ulong rx_crc_errors;    /* recved pkt with crc error    */
   zpl_ulong rx_frame_errors;  /* recv'd frame alignment error */
   zpl_ulong rx_fifo_errors;   /* recv'r fifo overrun          */
   zpl_ulong rx_missed_errors; /* receiver missed packet     */
   /* detailed tx_errors */
   zpl_ulong tx_aborted_errors;
   zpl_ulong tx_carrier_errors;
   zpl_ulong tx_fifo_errors;
   zpl_ulong tx_heartbeat_errors;
   zpl_ulong tx_window_errors;
   /* for cslip etc */
   zpl_ulong rx_compressed;
   zpl_ulong tx_compressed;
};

enum hal_port_cmd 
{
    HAL_PORT_NONE,
	HAL_PORT,
	HAL_PORT_LINK,
	HAL_PORT_SPEED,
	HAL_PORT_DUPLEX,
	HAL_PORT_FLOW,
    HAL_PORT_PAUSE,
    HAL_PORT_JUMBO,
	HAL_PORT_LOOP,
	HAL_PORT_LEARNING,
	HAL_PORT_SWLEARNING,
	HAL_PORT_PROTECTED,
    HAL_PORT_VRF,
    HAL_PORT_MODE,
    HAL_PORT_MTU,
    HAL_PORT_MULTICAST,
    HAL_PORT_BANDWIDTH,
	//STORM
	HAL_PORT_STORM_RATELIMIT,  
    HAL_PORT_STATS,    
};



typedef struct hal_port_param_s
{
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
    zpl_uint32 value1;
    zpl_uint32 value2;
}hal_port_param_t;
#endif
typedef struct sdk_port_s
{
	int (*sdk_port_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_link_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_speed_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_duplex_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_flow_cb)(void *, zpl_phyport_t, zpl_uint32, zpl_uint32);
	int (*sdk_port_jumbo_cb)(void *, zpl_phyport_t, zpl_bool);

	zpl_bool (*sdk_port_state_get_cb)(void *, zpl_phyport_t);
	zpl_uint32 (*sdk_port_speed_get_cb)(void *, zpl_phyport_t);
	zpl_uint32 (*sdk_port_duplex_get_cb)(void *, zpl_phyport_t);


	int (*sdk_port_loop_cb)(void *, zpl_phyport_t, zpl_uint32);

	int (*sdk_port_learning_enable_cb)(void *, zpl_phyport_t, zpl_bool);
	int (*sdk_port_swlearning_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_protected_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_vrf_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_mode_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_pause_cb)(void *, zpl_phyport_t, zpl_bool, zpl_bool);
	int (*sdk_port_discard_cb)(void *, zpl_phyport_t, zpl_bool);

	//风暴
	int (*sdk_port_storm_rate_cb) (void *, zpl_phyport_t, zpl_uint32 ,
			zpl_uint32, zpl_uint32);

	int (*sdk_port_stat_cb)(void *, zpl_phyport_t, struct if_stats *);

}sdk_port_t;


extern sdk_port_t sdk_port;
extern int bsp_port_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif


#endif /* __BSP_PORT_H__ */

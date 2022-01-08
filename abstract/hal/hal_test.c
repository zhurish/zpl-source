/*
 * hal_test.c
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */

#include "zpl_include.h"
#include "nsm_include.h"
#include "hal_ipccmd.h"
#include "hal_ipcmsg.h"

#include "hal_driver.h"



/*
 * Global
 */
DEFUN (hal_test_manage_mode,
		hal_test_manage_mode_cmd,
		"sdk manage",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_switch_mode(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (no_hal_test_manage_mode,
		no_hal_test_manage_mode_cmd,
		"no sdk manage",
		NO_STR
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_switch_mode(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_forward_mode,
		hal_test_forward_mode_cmd,
		"sdk forward",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_switch_forward(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (no_hal_test_forward_mode,
		no_hal_test_forward_mode_cmd,
		"no sdk forward",
		NO_STR
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_switch_forward(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_multicast_learning,
		hal_test_multicast_learning_cmd,
		"sdk multicast learning",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_multicast_learning(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (no_hal_test_multicast_learning,
		no_hal_test_multicast_learning_cmd,
		"no sdk multicast learning",
		NO_STR
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_multicast_learning(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


//全局使能接收BPDU报文
DEFUN (hal_test_bpdu,
		hal_test_bpdu_cmd,
		"sdk bpdu enable",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_global_bpdu_enable(zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
DEFUN (no_hal_test_bpdu,
		no_hal_test_bpdu_cmd,
		"no sdk bpdu enable",
		NO_STR
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_global_bpdu_enable(zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * dos
 */
DEFUN (hal_test_icmp_size,
		hal_test_icmp_size_cmd,
		"sdk icmp-size <64-65530>",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_dos_icmp_size(zpl_false, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_tcp_hdr_size,
		hal_test_tcp_hdr_size_cmd,
		"sdk tcp-hdr size <64-65530>",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_dos_tcp_hdr_size(atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_dos_enable,
		hal_test_dos_enable_cmd,
		"sdk dos-enable (iplan|tcp-blat|udp-blat|tcp-nullscan|tcp-xmasscan|tcp-synfinscan|"
		"tcp-synerror|tcp-zpl_int16hdr|tcp-fragerror|icmpv4-fragment|icmpv6-fragment|icmpv4-long|icmpv4-long)",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	dos_type_en type;
	if(strstr(argv[0], "iplan"))
		type = DOS_IP_LAN_DRIP;
	else if(strstr(argv[0], "tcp-blat"))
		type = TCP_BLAT_DROP;
	else if(strstr(argv[0], "udp-blat"))
		type = UDP_BLAT_DROP;
	else if(strstr(argv[0], "nullscan"))
		type = TCP_NULLSCAN_DROP;
	else if(strstr(argv[0], "xmasscan"))
		type = TCP_XMASSCAN_DROP;
	else if(strstr(argv[0], "synfinscan"))
		type = TCP_SYNFINSCAN_DROP;
	else if(strstr(argv[0], "synerror"))
		type = TCP_SYNERROR_DROP;
	else if(strstr(argv[0], "zpl_int16hdr"))
		type = TCP_SHORTHDR_DROP;
	else if(strstr(argv[0], "fragerror"))
		type = TCP_FRAGERROR_DROP;
	else if(strstr(argv[0], "icmpv4-fragment"))
		type = ICMPv4_FRAGMENT_DROP;
	else if(strstr(argv[0], "icmpv6-fragment"))
		type = ICMPv6_FRAGMENT_DROP;
	else if(strstr(argv[0], "icmpv4-long"))
		type = ICMPv4_LONGPING_DROP;
	else if(strstr(argv[0], "icmpv6-long"))
		type = ICMPv6_LONGPING_DROP;
	ret = hal_dos_enable(zpl_true, type);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_dos_enable,
		no_hal_test_dos_enable_cmd,
		"no sdk dos-enable (iplan|tcp-blat|udp-blat|tcp-nullscan|tcp-xmasscan|tcp-synfinscan|"
		"tcp-synerror|tcp-zpl_int16hdr|tcp-fragerror|icmpv4-fragment|icmpv6-fragment|icmpv4-long|icmpv4-long)",
		NO_STR
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	dos_type_en type;
	if(strstr(argv[0], "iplan"))
		type = DOS_IP_LAN_DRIP;
	else if(strstr(argv[0], "tcp-blat"))
		type = TCP_BLAT_DROP;
	else if(strstr(argv[0], "udp-blat"))
		type = UDP_BLAT_DROP;
	else if(strstr(argv[0], "nullscan"))
		type = TCP_NULLSCAN_DROP;
	else if(strstr(argv[0], "xmasscan"))
		type = TCP_XMASSCAN_DROP;
	else if(strstr(argv[0], "synfinscan"))
		type = TCP_SYNFINSCAN_DROP;
	else if(strstr(argv[0], "synerror"))
		type = TCP_SYNERROR_DROP;
	else if(strstr(argv[0], "zpl_int16hdr"))
		type = TCP_SHORTHDR_DROP;
	else if(strstr(argv[0], "fragerror"))
		type = TCP_FRAGERROR_DROP;
	else if(strstr(argv[0], "icmpv4-fragment"))
		type = ICMPv4_FRAGMENT_DROP;
	else if(strstr(argv[0], "icmpv6-fragment"))
		type = ICMPv6_FRAGMENT_DROP;
	else if(strstr(argv[0], "icmpv4-long"))
		type = ICMPv4_LONGPING_DROP;
	else if(strstr(argv[0], "icmpv6-long"))
		type = ICMPv6_LONGPING_DROP;
	ret = hal_dos_enable(zpl_false, type);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * MAC
 */
/*
int hal_mac_age(int age);
int hal_mac_add(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri);
int hal_mac_del(ifindex_t ifindex, vlan_t vlan, mac_t *mac, int pri);
int hal_mac_clr(ifindex_t ifindex, vlan_t vlan);
int hal_mac_read(ifindex_t ifindex, vlan_t vlan);
*/

/*
 * mirror
 */

DEFUN (hal_test_mirror_enable,
		hal_test_mirror_enable_cmd,
		"sdk mirror interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_mirror_enable(if_ifindex_make(argv[0], argv[1]), zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_mirror_enable,
		no_hal_test_mirror_enable_cmd,
		"no sdk mirror interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_mirror_enable(if_ifindex_make(argv[0], argv[1]), zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_mirror_src_enable,
		hal_test_mirror_src_enable_cmd,
		"sdk mirror src-interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_mirror_source_enable(if_ifindex_make(argv[0], argv[1]), zpl_true, MIRROR_SOURCE_PORT, MIRROR_BOTH);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_mirror_src_enable,
		no_hal_test_mirror_src_enable_cmd,
		"no sdk mirror src-interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_mirror_source_enable(if_ifindex_make(argv[0], argv[1]), zpl_false, MIRROR_SOURCE_PORT, MIRROR_BOTH);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/*
 * PORT
 */
DEFUN (hal_test_interface_up,
		hal_test_interface_up_cmd,
		"sdk up interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_up(if_ifindex_make(argv[0], argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_interface_down,
		hal_test_interface_down_cmd,
		"sdk down interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_down(if_ifindex_make(argv[0], argv[1]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_interface_speed,
		hal_test_interface_speed_cmd,
		"sdk speed interface " CMD_IF_USPV_STR " "CMD_USP_STR " (10|100|1000)",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_speed_set(if_ifindex_make(argv[0], argv[1]), atoi(argv[2]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_interface_duplex,
		hal_test_interface_duplex_cmd,
		"sdk duplex interface " CMD_IF_USPV_STR " "CMD_USP_STR " (half|full)",
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_duplex_set(if_ifindex_make(argv[0], argv[1]), strstr(argv[2],"full")?1:0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (hal_test_jumbo_enable,
		hal_test_jumbo_enable_cmd,
		"sdk jumbo interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_jumbo_set(if_ifindex_make(argv[0], argv[1]), zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_jumbo_enable,
		no_hal_test_jumbo_enable_cmd,
		"no sdk jumbo interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_jumbo_set(if_ifindex_make(argv[0], argv[1]), zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_enable_enable,
		hal_test_enable_enable_cmd,
		"sdk enable interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_enable_set(if_ifindex_make(argv[0], argv[1]), zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_enable_enable,
		no_hal_test_enable_enable_cmd,
		"no sdk enable interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_enable_set(if_ifindex_make(argv[0], argv[1]), zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (hal_test_learning_enable,
		hal_test_learning_enable_cmd,
		"sdk learning interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_learning_set(if_ifindex_make(argv[0], argv[1]), zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_learning_enable,
		no_hal_test_learning_enable_cmd,
		"no sdk learning interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_learning_set(if_ifindex_make(argv[0], argv[1]), zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (hal_test_swlearning_enable,
		hal_test_swlearning_enable_cmd,
		"sdk swlearning interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_software_learning_set(if_ifindex_make(argv[0], argv[1]), zpl_true);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_hal_test_swlearning_enable,
		no_hal_test_swlearning_enable_cmd,
		"no sdk swlearning interface " CMD_IF_USPV_STR " "CMD_USP_STR,
		"sdk\n"
		"Manage\n")
{
	int ret = 0;
	ret = hal_port_software_learning_set(if_ifindex_make(argv[0], argv[1]), zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#if 0
/*
 * QOS
 */
int hal_qos_enable(zpl_bool enable);
int hal_qos_ipg_enable(zpl_bool enable);
int hal_qos_base_mode(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_8021q_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_diffserv_enable(ifindex_t ifindex, nsm_qos_trust_e enable);
int hal_qos_port_map_queue(ifindex_t ifindex, nsm_qos_priority_e pri, nsm_qos_queue_e queue);
int hal_qos_diffserv_map_queue(ifindex_t ifindex, int diffserv, nsm_qos_queue_e queue);

//队列到class的映射
int hal_qos_queue_class(ifindex_t ifindex, nsm_qos_queue_e queue, nsm_qos_class_e class);
int hal_qos_queue_scheduling(nsm_qos_class_e class, nsm_class_sched_t type);
int hal_qos_queue_weight(nsm_qos_class_e class, int weight);

//风暴
int hal_qos_storm_mode(ifindex_t ifindex, zpl_bool enable, int mode);
int hal_qos_storm_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size);

//端口限速
int hal_qos_egress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size);
int hal_qos_ingress_rate_limit(ifindex_t ifindex, zpl_uint32 limit, zpl_uint32 burst_size);
//CPU
int hal_qos_cpu_rate_limit(zpl_uint32 limit, zpl_uint32 burst_size);

/*
 * TRUNK
 */
int hal_trunk_enable(zpl_bool enable);
int hal_trunk_mode(int mode);
int hal_trunk_interface_enable(ifindex_t ifindex, int trunkid);
int hal_trunk_interface_disable(ifindex_t ifindex, int trunkid);

int hal_trunkid(int trunkid);


/*
 *VLAN
 */
extern int hal_vlan_enable(zpl_bool enable);
extern int hal_vlan_create(vlan_t vlan);
extern int hal_vlan_destroy(vlan_t vlan);

extern int hal_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);
extern int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);

extern int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan);
#endif

int hal_test_init(void)
{
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_manage_mode_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_manage_mode_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_forward_mode_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_forward_mode_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_multicast_learning_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_multicast_learning_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_bpdu_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_bpdu_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_icmp_size_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_tcp_hdr_size_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_dos_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_dos_enable_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_mirror_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_mirror_enable_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_mirror_src_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_mirror_src_enable_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_interface_up_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_interface_down_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_interface_speed_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_interface_duplex_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_jumbo_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_jumbo_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_enable_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_enable_enable_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_learning_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_learning_enable_cmd);

	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &hal_test_swlearning_enable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &no_hal_test_swlearning_enable_cmd);
	return 0;
}

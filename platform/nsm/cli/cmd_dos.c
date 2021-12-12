/*
 * cmd_dos.c
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */


#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

DEFUN (dos_icmp_long_ping_enable,
		dos_icmp_long_ping_enable_cmd,
		"ip (icmpv4|icmpv6) long-ping",
		IP_STR
		"ICMPv4\n"
		"ICMPv6\n"
		"Long ping\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_enable_api(ICMPv4_LONGPING_DROP);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_enable_api(ICMPv6_LONGPING_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_icmp_long_ping_enable,
		no_dos_icmp_long_ping_enable_cmd,
		"no ip (icmpv4|icmpv6) long-ping",
		NO_STR
		IP_STR
		"ICMPv4\n"
		"ICMPv6\n"
		"Long ping\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_disable_api(ICMPv4_LONGPING_DROP);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_disable_api(ICMPv6_LONGPING_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_frag_ment_enable,
		dos_frag_ment_enable_cmd,
		"ip (icmpv4|icmpv6) fragment",
		IP_STR
		"ICMPv4\n"
		"ICMPv6\n"
		"fragment\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_enable_api(ICMPv4_FRAGMENT_DROP);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_enable_api(ICMPv6_FRAGMENT_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_frag_ment_enable,
		no_dos_frag_ment_enable_cmd,
		"no ip (icmpv4|icmpv6) fragment",
		NO_STR
		IP_STR
		"ICMPv4\n"
		"ICMPv6\n"
		"fragment\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_disable_api(ICMPv4_FRAGMENT_DROP);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_disable_api(ICMPv6_FRAGMENT_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_frag_error_enable,
		dos_frag_error_enable_cmd,
		"ip tcp frag-error",
		IP_STR
		"TCP \n"
		"fragment error\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_enable_api(TCP_FRAGERROR_DROP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_frag_error_enable,
		no_dos_frag_error_enable_cmd,
		"no ip tcp frag-error",
		NO_STR
		IP_STR
		"TCP \n"
		"fragment error\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_disable_api(TCP_FRAGERROR_DROP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dozpl_zpl_int16_hdr_enable,
		dozpl_zpl_int16_hdr_enable_cmd,
		"ip tcp short-hdr",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_enable_api(TCP_SHORTHDR_DROP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dozpl_zpl_int16_hdr_enable,
		no_dozpl_zpl_int16_hdr_enable_cmd,
		"no ip tcp short-hdr",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_disable_api(TCP_SHORTHDR_DROP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_tcp_enable,
		dos_tcp_enable_cmd,
		"ip tcp (null-scan|xmas-scan|synfin-scan|syn-error)",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "null-scan", 6) == 0)
		ret = nsm_dos_enable_api(TCP_NULLSCAN_DROP);
	else if(os_memcmp(argv[0], "xmas-scan", 6) == 0)
		ret = nsm_dos_enable_api(TCP_XMASSCAN_DROP);
	else if(os_memcmp(argv[0], "synfin-scan", 6) == 0)
		ret = nsm_dos_enable_api(TCP_SYNFINSCAN_DROP);
	else if(os_memcmp(argv[0], "syn-error", 6) == 0)
		ret = nsm_dos_enable_api(TCP_SYNERROR_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_tcp_enable,
		no_dos_tcp_enable_cmd,
		"no ip tcp (null-scan|xmas-scan|synfin-scan|syn-error)",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "null-scan", 6) == 0)
		ret = nsm_dos_disable_api(TCP_NULLSCAN_DROP);
	else if(os_memcmp(argv[0], "xmas-scan", 6) == 0)
		ret = nsm_dos_disable_api(TCP_XMASSCAN_DROP);
	else if(os_memcmp(argv[0], "synfin-scan", 6) == 0)
		ret = nsm_dos_disable_api(TCP_SYNFINSCAN_DROP);
	else if(os_memcmp(argv[0], "syn-error", 6) == 0)
		ret = nsm_dos_disable_api(TCP_SYNERROR_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_blat_enable,
		dos_blat_enable_cmd,
		"ip (tcp|udp) blat",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "tcp", 2) == 0)
		ret = nsm_dos_enable_api(TCP_BLAT_DROP);
	else if(os_memcmp(argv[0], "udp", 6) == 2)
		ret = nsm_dos_enable_api(UDP_BLAT_DROP);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_blat_enable,
		no_dos_blat_enable_cmd,
		"no ip (tcp|udp) blat",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	if(os_memcmp(argv[0], "tcp", 2) == 0)
		ret = nsm_dos_disable_api(TCP_BLAT_DROP);
	else if(os_memcmp(argv[0], "udp", 2) == 0)
		ret = nsm_dos_disable_api(UDP_BLAT_DROP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_lan_drip_enable,
		dos_lan_drip_enable_cmd,
		"ip lan-drip",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_enable_api(DOS_IP_LAN_DRIP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_lan_drip_enable,
		no_dos_lan_drip_enable_cmd,
		"no ip lan-drip",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();

	ret = nsm_dos_disable_api(DOS_IP_LAN_DRIP);

	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_icmp_size_enable,
		dos_icmp_size_enable_cmd,
		"ip (icmpv4|icmpv6) max-size <64-65530>",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	int value = 0;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	value = atoi(argv[1]);
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_icmpv4_max(value);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_icmpv6_max(value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_icmp_size_enable,
		no_dos_icmp_size_enable_cmd,
		"no ip (icmpv4|icmpv6) max-size",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	int value = 0;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	value = 0;//atoi(argv[1]);
	if(os_memcmp(argv[0], "icmpv4", 6) == 0)
		ret = nsm_dos_icmpv4_max(value);
	else if(os_memcmp(argv[0], "icmpv6", 6) == 0)
		ret = nsm_dos_icmpv6_max(value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (dos_tcp_hdr_size_enable,
		dos_tcp_hdr_size_enable_cmd,
		"ip tcp-hdr min-size <64-250>",
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	int value = 0;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	value = atoi(argv[1]);

	ret = nsm_dos_tcp_hdr_min(value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_dos_tcp_hdr_size_enable,
		no_dos_tcp_hdr_size_enable_cmd,
		"no ip tcp-hdr min-size",
		NO_STR
		IP_STR
		"TCP \n"
		"short-hdr\n")
{
	int ret = ERROR;
	int value = 0;
	if (argc < 1)
		return CMD_WARNING;
	if(!nsm_dos_is_enable())
		nsm_dos_enable();
	value = 0;//atoi(argv[1]);

	ret = nsm_dos_tcp_hdr_min(value);
	return (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



void cmd_dos_init (void)
{
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_icmp_long_ping_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_icmp_long_ping_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_frag_ment_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_frag_ment_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_frag_error_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_frag_error_enable_cmd);

	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dozpl_zpl_int16_hdr_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dozpl_zpl_int16_hdr_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_tcp_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_tcp_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_blat_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_blat_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_lan_drip_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_lan_drip_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_icmp_size_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_icmp_size_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &dos_tcp_hdr_size_enable_cmd);
	  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_dos_tcp_hdr_size_enable_cmd);
}

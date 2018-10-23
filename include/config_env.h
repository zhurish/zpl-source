/*
 * config_env.h
 *
 *  Created on: Oct 12, 2018
 *      Author: zhurish
 */

#ifndef INCLUDE_CONFIG_ENV_H_
#define INCLUDE_CONFIG_ENV_H_



/* bgpd vty socket */
#define BGP_VTYSH_PATH DAEMON_VTY_DIR "/bgpd.vty"

/* frpd vty socket */
#define FRP_VTYSH_PATH DAEMON_VTY_DIR "/frpd.vty"

/* IMI Mdoule socket */
#define IMISH_UNIX_PATH DAEMON_VTY_DIR "/imi-cli.vty"
/* isisd vty socket */
#define ISIS_VTYSH_PATH DAEMON_VTY_DIR "/isisd.vty"
/* ldpd vty socket */
#define LDP_VTYSH_PATH DAEMON_VTY_DIR "/ldpd.vty"
/* lldpd vty socket */
#define LLDP_VTYSH_PATH DAEMON_VTY_DIR "/lldpd.vty"
/* olsrd vty socket */
#define OLSR_VTYSH_PATH DAEMON_VTY_DIR "/olsrd.vty"
/* nhrpd vty socket */
#define NHRP_VTYSH_PATH DAEMON_VTY_DIR "/nhrpd.vty"
/* ospf6d vty socket */
#define OSPF6_VTYSH_PATH DAEMON_VTY_DIR "/ospf6d.vty"
/* ospfd vty socket */
#define OSPF_VTYSH_PATH DAEMON_VTY_DIR "/ospfd.vty"
/* pimd vty socket */
#define PIM_VTYSH_PATH DAEMON_VTY_DIR "/pimd.vty"
/* ripng vty socket */
#define RIPNG_VTYSH_PATH DAEMON_VTY_DIR "/ripngd.vty"
/* rip vty socket */
#define RIP_VTYSH_PATH DAEMON_VTY_DIR "/ripd.vty"
/* utils vty socket */
#define UTILS_VTYSH_PATH DAEMON_VTY_DIR "/utils.vty"
/* vrrpd vty socket */
#define VRRP_VTYSH_PATH DAEMON_VTY_DIR "/vrrpd.vty"
/* zebra vty socket */
#define ZEBRA_VTYSH_PATH DAEMON_VTY_DIR "/zebra.vty"





/* bfdd PID */
/* #undef PATH_BFDD_PID */

/* bgpd PID */
#define PATH_BGPD_PID DAEMON_VTY_DIR "/bgpd.pid"

/* frpd PID */
#define PATH_FRPD_PID DAEMON_VTY_DIR "/frpd.pid"

/* IMI Mdoule PID */
#define PATH_IMISH_PID DAEMON_VTY_DIR "/imi-cli.pid"

/* isisd PID */
#define PATH_ISISD_PID DAEMON_VTY_DIR "/isisd.pid"

/* ldpd PID */
#define PATH_LDPD_PID DAEMON_VTY_DIR "/ldpd.pid"

/* lldpd PID */
#define PATH_LLDPD_PID DAEMON_VTY_DIR "/lldpd.pid"

/* nhrpd PID */
#define PATH_NHRPD_PID DAEMON_VTY_DIR "/nhrpd.pid"

/* olsrd PID */
#define PATH_OLSRD_PID DAEMON_VTY_DIR "/olsrd.pid"

/* ospf6d PID */
#define PATH_OSPF6D_PID DAEMON_VTY_DIR "/ospf6d.pid"

/* ospfd PID */
#define PATH_OSPFD_PID DAEMON_VTY_DIR "/ospfd.pid"

/* pimd PID */
#define PATH_PIMD_PID DAEMON_VTY_DIR "/pimd.pid"

/* ripd PID */
#define PATH_RIPD_PID DAEMON_VTY_DIR "/ripd.pid"

/* ripngd PID */
#define PATH_RIPNGD_PID DAEMON_VTY_DIR "/ripngd.pid"

/* utils PID */
#define PATH_UTILS_PID DAEMON_VTY_DIR "/utils.pid"

/* vrrpd PID */
#define PATH_VRRPD_PID DAEMON_VTY_DIR "/vrrpd.pid"

/* watchquagga PID */
#define PATH_WATCHQUAGGA_PID DAEMON_VTY_DIR "/watchquagga.pid"

/* zebra PID */
#define PATH_ZEBRA_PID DAEMON_VTY_DIR "/zebra.pid"






/* zebra api socket */
#define ZEBRA_SERV_PATH DAEMON_VTY_DIR "/zserv.api"

/* default oem file */
#define HOST_DEFAULT_OEM_FILE SYSCONFDIR "/oem.bin"


/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
//#define LT_OBJDIR ".libs/"


#endif /* INCLUDE_CONFIG_ENV_H_ */

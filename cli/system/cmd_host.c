/*
 * cmd_host.c
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#include "zebra.h"

#include "log.h"
#include "memory.h"
#include "thread.h"
#include "vector.h"
#include "version.h"
#include "workqueue.h"
#include "command.h"
#include "vty.h"
#include "vty_user.h"
#include "host.h"
#include "template.h"

#define CMD_HOST_DEBUG

extern vector cmdvec;
const char *default_motd;

/* Standard command node structures. */
static struct cmd_node user_node =
{
	USER_NODE,
	"username: ",
};
static struct cmd_node auth_node =
{
	AUTH_NODE,
	"Password: ",
};

static struct cmd_node view_node =
{
	VIEW_NODE,
	"%s> ",
};


static struct cmd_node service_node =
{
	SERVICE_NODE,
	"%s(config)# ",
	1
};

static struct cmd_node all_service_node =
{
	ALL_SERVICE_NODE,
	"%s(config-%s)# ",
	1
};

static struct cmd_node auth_enable_node =
{
	AUTH_ENABLE_NODE,
	"Password: ",
};

static struct cmd_node enable_node =
{
	ENABLE_NODE,
	"%s# ",
};

static struct cmd_node config_node =
{
	CONFIG_NODE,
	"%s(config)# ",
	1
};

static struct cmd_node template_node =
{
	TEMPLATE_NODE,
	"%s(config-%s)# ",
	1
};


struct cmd_node interface_node =
{
		INTERFACE_NODE,
		"%s(config-if)# ",
		1
};
struct cmd_node interface_l3_node =
{
		INTERFACE_L3_NODE,
		"%s(config-if)# ",
		1
};
struct cmd_node interface_wireless_node =
{
		WIRELESS_INTERFACE_NODE,
		"%s(config-if)# ",
		1
};
struct cmd_node tunnel_interface_node =
{
		TUNNEL_INTERFACE_NODE,
		"%s(config-tunnel)# ",
		1
};
struct cmd_node loopback_interface_node =
{
		LOOPBACK_INTERFACE_NODE,
		"%s(config-loopback)# ",
		1
};
struct cmd_node lag_interface_node =
{
		LAG_INTERFACE_NODE,
		"%s(config-port-channel)# ",
		1
};
struct cmd_node lag_interface_l3_node =
{
		LAG_INTERFACE_L3_NODE,
		"%s(config-port-channel)# ",
		1
};

struct cmd_node brigde_interface_node =
{
		BRIGDE_INTERFACE_NODE,
		"%s(config-brigde)# ",
		1
};

#ifdef CUSTOM_INTERFACE
struct cmd_node wifi_interface_node =
{
		WIFI_INTERFACE_NODE,
		"%s(config-wifi)# ",
		1
};
struct cmd_node modem_interface_node =
{
		MODEM_INTERFACE_NODE,
		"%s(config-modem)# ",
		1
};
#endif

struct cmd_node trunk_group_node =
{
		TRUNK_NODE,
		"%s(config-trunk-group)# ",
		1
};
struct cmd_node modem_profile_node =
{
		MODEM_PROFILE_NODE,
		"%s(config-modem-profile)# ",
		1
};
struct cmd_node modem_channel_node =
{
		MODEM_CHANNEL_NODE,
		"%s(config-modem-channel)# ",
		1
};

struct cmd_node serial_interface_node =
{
		SERIAL_INTERFACE_NODE,
		"%s(config-serial)# ",
		1
};

/* Configration from terminal */
DEFUN (config_terminal,
		config_terminal_cmd,
		"configure terminal",
		"Configuration from vty interface\n"
		"Configuration terminal\n")
{
	if (vty_user_getting_privilege(vty, vty->username) <= ENABLE_LEVEL) {
		vty_out(vty, "%%Users not authorized%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (vty_config_lock(vty))
		vty->node = CONFIG_NODE;
	else {
		vty_out(vty, "VTY configuration is locked by other VTY%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/* Enable command */
DEFUN (enable,
		config_enable_cmd,
		"enable",
		"Turn on privileged mode command\n")
{
	/* If enable password is NULL, change to ENABLE_NODE */
	if (vty_user_getting_privilege(vty, vty->username) < ENABLE_LEVEL) {
		vty_out(vty, "%%Users not authorized%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
#if 0//def CMD_HOST_DEBUG
	if (vty->user)
		vty_out(vty, "%s:%s %s %s", __func__, vty->user->enable,
				vty->user->enable_encrypt, VTY_NEWLINE);
#endif
	if(!vty_user_enable_password(vty, vty->username)
			|| vty->type == VTY_SHELL_SERV)
/*	if ((vty->user && vty->user->enable == NULL
			&& vty->user->enable_encrypt[0] == 0)
			|| vty->type == VTY_SHELL_SERV)*/
		vty->node = ENABLE_NODE;
	else
		vty->node = AUTH_ENABLE_NODE;
	return CMD_SUCCESS;
}

/* Disable command */
DEFUN (disable,
		config_disable_cmd,
		"disable",
		"Turn off privileged mode command\n")
{
	if (vty->node == ENABLE_NODE)
		vty->node = VIEW_NODE;
	return CMD_SUCCESS;
}

/* Down vty node level. */
DEFUN (config_exit,
		config_exit_cmd,
		"exit",
		"Exit current mode and down to previous mode\n")
{
	switch (vty->node) {
	case VIEW_NODE:
		//case ENABLE_NODE:
		/*case RESTRICTED_NODE:*/
		if (vty_shell(vty))
			exit(0);
		else
			vty->status = VTY_CLOSE;
		break;
	case ENABLE_NODE:
		vty->node = VIEW_NODE;
		break;
	case CONFIG_NODE:
		vty->node = ENABLE_NODE;
		vty_config_unlock(vty);
		break;

	case DHCPS_NODE:
	case TEMPLATE_NODE:
	case ALL_SERVICE_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE:		/* Interface mode node. */
	case WIRELESS_INTERFACE_NODE:

	case TUNNEL_INTERFACE_NODE:	/* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE:	/* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE:		/* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE:	/* Lag L3 Interface mode node. */
	case SERIAL_INTERFACE_NODE:
	case BRIGDE_INTERFACE_NODE:
#ifdef CUSTOM_INTERFACE
	case WIFI_INTERFACE_NODE:
	case MODEM_INTERFACE_NODE:
#endif
	case TRUNK_NODE:
	case ZEBRA_NODE:
	case BGP_NODE:
	case RIP_NODE:
	case RIPNG_NODE:
	case BABEL_NODE:
	case OSPF_NODE:
	case OSPF6_NODE:
	case ISIS_NODE:
	case KEYCHAIN_NODE:
	case MASC_NODE:
	case RMAP_NODE:
	case PIM_NODE:
	case VTY_NODE:

	case HSLS_NODE: /* HSLS protocol node. */
	case OLSR_NODE: /* OLSR protocol node. */
	case VRRP_NODE:
	case FRP_NODE: /* FRP protocol node */
	case LLDP_NODE:

	case BFD_NODE:
	case LDP_NODE:
    case VRF_NODE:
    case VLAN_DATABASE_NODE:
    case VLAN_NODE:
		vty->node = CONFIG_NODE;
		break;
	case BGP_IPV4_NODE:
	case BGP_IPV4M_NODE:
	case BGP_VPNV4_NODE:
	case BGP_VPNV6_NODE:
	case BGP_ENCAP_NODE:
	case BGP_ENCAPV6_NODE:
	case BGP_IPV6_NODE:
	case BGP_IPV6M_NODE:
		vty->node = BGP_NODE;
		break;
	case KEYCHAIN_KEY_NODE:
		vty->node = KEYCHAIN_NODE;
		break;
	case LINK_PARAMS_NODE:
		vty->node = INTERFACE_NODE;
		break;
	default:
		break;
	}
	return CMD_SUCCESS;
}

/* quit is alias of exit. */
ALIAS(config_exit,
		config_quit_cmd,
		"quit",
		"Exit current mode and down to previous mode\n")

/* End of configuration. */
DEFUN (config_end,
		config_end_cmd,
		"end",
		"End current mode and change to enable mode.")
{
	switch (vty->node) {
	case VIEW_NODE:
	case ENABLE_NODE:
		/*    case RESTRICTED_NODE:*/
		/* Nothing to do. */
		break;

	case CONFIG_NODE:
	case DHCPS_NODE:
	case TEMPLATE_NODE:
	case ALL_SERVICE_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE:		/* Interface mode node. */
	case WIRELESS_INTERFACE_NODE:
	case TUNNEL_INTERFACE_NODE:	/* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE:	/* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE:		/* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE:	/* Lag L3 Interface mode node. */
	case SERIAL_INTERFACE_NODE:
	case BRIGDE_INTERFACE_NODE:
#ifdef CUSTOM_INTERFACE
	case WIFI_INTERFACE_NODE:
	case MODEM_INTERFACE_NODE:
#endif
	case TRUNK_NODE:
	case ZEBRA_NODE:
	case RIP_NODE:
	case RIPNG_NODE:
	case BABEL_NODE:
	case BGP_NODE:
	case BGP_ENCAP_NODE:
	case BGP_ENCAPV6_NODE:
	case BGP_VPNV4_NODE:
	case BGP_VPNV6_NODE:
	case BGP_IPV4_NODE:
	case BGP_IPV4M_NODE:
	case BGP_IPV6_NODE:
	case BGP_IPV6M_NODE:
	case RMAP_NODE:
	case OSPF_NODE:
	case OSPF6_NODE:
	case ISIS_NODE:
	case KEYCHAIN_NODE:
	case KEYCHAIN_KEY_NODE:
	case MASC_NODE:
	case PIM_NODE:
	case VTY_NODE:
	case LINK_PARAMS_NODE:

	case HSLS_NODE: /* HSLS protocol node. */
	case OLSR_NODE: /* OLSR protocol node. */
	case VRRP_NODE:
	case FRP_NODE: /* FRP protocol node */
	case LLDP_NODE:

	case BFD_NODE:
	case LDP_NODE:
    case VRF_NODE:
    case VLAN_DATABASE_NODE:
    case VLAN_NODE:
		vty_config_unlock(vty);
		vty->node = ENABLE_NODE;
		break;
	default:
		break;
	}
	return CMD_SUCCESS;
}

/* Show version. */
DEFUN (show_version,
		show_version_cmd,
		"show version",
		SHOW_STR
		"Displays version\n")
{
	vty_out(vty, "%s", VTY_NEWLINE);
	vty_out(vty, " DeviceName    : %s%s", OEM_DEVICE_NAME, VTY_NEWLINE);
	vty_out(vty, " HW Version    : %s%s", OEM_HW_VERSION, VTY_NEWLINE);

	vty_out(vty, " Software      : %s%s", OEM_PACKAGE_NAME, VTY_NEWLINE);
	vty_out(vty, " Version       : %s%s", OEM_VERSION, VTY_NEWLINE);
	vty_out(vty, " Copyright     : %s%s", OEM_PACKAGE_COPYRIGHT, VTY_NEWLINE);
	//vty_out(vty, " Base          : %s(V%x.%x.%x)%s", OEM_PACKAGE_BASE,
	//			(OEM_PACKAGE_VERSION >> 16) & 0xFF,
	//			(OEM_PACKAGE_VERSION >> 8) & 0xFF,
	//			(OEM_PACKAGE_VERSION) & 0xFF,
	//			VTY_NEWLINE);

	vty_out(vty, " BugReport     : %s%s", OEM_PACKAGE_BUGREPORT, VTY_NEWLINE);
	//vty_out(vty, " Git Release   : %s%s", OEM_GIT_RELEASE, VTY_NEWLINE);
	//vty_out(vty, " Git Commit    : %s%s", OEM_GIT_COMMIT, VTY_NEWLINE);
	//vty_out(vty, " Version       : %s%s", OEM_VERSION, VTY_NEWLINE);
#ifdef PL_BUILD_TIME
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(PL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " Build Time    : %s %s .%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* PL_BUILD_TIME */

#if 0
	vty_out(vty, " Hello this is \"%s\" (version:%s).%s", OEM_PACKAGE_NAME,
				OEM_VERSION, VTY_NEWLINE);
	vty_out(vty, " %s%s", OEM_PACKAGE_COPYRIGHT, VTY_NEWLINE);
	vty_out(vty, " Design it Base on (%s).%s", OEM_PACKAGE_BASE,
				VTY_NEWLINE);
	if ((GIT_SUFFIX) && (strlen(GIT_SUFFIX) > 2))
		vty_out(vty, " Git suffex : %s%s Git info:%s%s", GIT_SUFFIX,
					VTY_NEWLINE, GIT_INFO, VTY_NEWLINE);
#ifdef PL_BUILD_TIME
	//20181024160232
	vty_out(vty, " It't make: %s.%s", os_build_time2date(PL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " It't make: %s.%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " It't make: %s %s.%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* PL_BUILD_TIME */
#endif
	vty_out(vty, "%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN_HIDDEN (show_hidden_version,
		show_hidden_version_cmd,
		"show version-hidden",
		SHOW_STR
		"Displays detail version\n")
{
	vty_out(vty, "%s", VTY_NEWLINE);
	vty_out(vty, " DeviceName    : %s%s", OEM_DEVICE_NAME, VTY_NEWLINE);
	vty_out(vty, " HW Version    : %s%s", OEM_HW_VERSION, VTY_NEWLINE);

	vty_out(vty, " Software      : %s%s", OEM_PACKAGE_NAME, VTY_NEWLINE);
	vty_out(vty, " Version       : %s%s", OEM_VERSION, VTY_NEWLINE);
	vty_out(vty, " Copyright     : %s%s", OEM_PACKAGE_COPYRIGHT, VTY_NEWLINE);
	vty_out(vty, " Base          : %s(V%x.%x.%x)%s", OEM_PACKAGE_BASE,
				(OEM_PACKAGE_VERSION >> 16) & 0xFF,
				(OEM_PACKAGE_VERSION >> 8) & 0xFF,
				(OEM_PACKAGE_VERSION) & 0xFF,
				VTY_NEWLINE);

	vty_out(vty, " BugReport     : %s%s", OEM_PACKAGE_BUGREPORT, VTY_NEWLINE);
	vty_out(vty, " Git Release   : %s%s", OEM_GIT_RELEASE, VTY_NEWLINE);
	vty_out(vty, " Git Commit    : %s%s", OEM_GIT_COMMIT, VTY_NEWLINE);
	vty_out(vty, " Git Version   : %s%s", GIT_VERSION, VTY_NEWLINE);
#ifdef PL_BUILD_TIME
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(PL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " Build Time    : %s %s .%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* PL_BUILD_TIME */
	vty_out(vty, "%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN (show_system,
		show_system_cmd,
		"show system",
		SHOW_STR
		"Displays system information\n")
{
	struct host_system host_system;
	vty_out(vty, "%s", VTY_NEWLINE);
	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);
	show_host_system_information(&host_system, vty);
	vty_out(vty, "%s", VTY_NEWLINE);
	return CMD_SUCCESS;
}

/*
ALIAS(show_system,
		show_system_detail_cmd,
		"show system (detail|)",
		SHOW_STR
		"Displays System information\n"
		"Displays Detail information\n");
*/


/* Help display function for all node. */
DEFUN (config_help,
		config_help_cmd,
		"help",
		"Description of the interactive help system\n")
{
	vty_out(vty,
			"Quagga VTY provides advanced help feature.  When you need help,%s\
anytime at the command line please press '?'.%s\
%s\
If nothing matches, the help list will be empty and you must backup%s\
 until entering a '?' shows the available options.%s\
Two styles of help are provided:%s\
1. Full help is available when you are ready to enter a%s\
command argument (e.g. 'show ?') and describes each possible%s\
argument.%s\
2. Partial help is provided when an abbreviated argument is entered%s\
   and you want to know what arguments match the input%s\
   (e.g. 'show me?'.)%s%s",
			VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE,
			VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE,
			VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE);
	return CMD_SUCCESS;
}

/* Help display function for all node. */
DEFUN (config_list,
		config_list_cmd,
		"list",
		"Print command list\n")
{
	ospl_uint32  i;
	struct cmd_node *cnode = vector_slot(cmdvec, vty->node);
	struct cmd_element *cmd;

	for (i = 0; i < vector_active(cnode->cmd_vector); i++)
		if ((cmd = vector_slot(cnode->cmd_vector, i)) != NULL
				&& !(cmd->attr == CMD_ATTR_DEPRECATED
						|| cmd->attr == CMD_ATTR_HIDDEN))
			vty_out(vty, "  %s%s", cmd->string,
			VTY_NEWLINE);
	return CMD_SUCCESS;
}

/* Write current configuration into file. */
DEFUN (config_write_file,
		config_write_file_cmd,
		"write file",
		"Write running configuration to memory, network, or terminal\n"
		"Write to configuration file\n")
{
	ospl_uint32  i;
	int fd;
	struct cmd_node *node;
	char *config_file;
	char *config_file_sav = NULL;
	struct vty *file_vty;
	/* Check and see if we are operating under vtysh configuration */
	if (host.config == NULL)
	{
		vty_out(vty, "Can't save to configuration file, using vtysh.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	/* Get filename. */
	config_file = host.config;
	config_file_sav = malloc(strlen(config_file) + strlen(CONF_BACKUP_EXT) + 1);
	strcpy(config_file_sav, config_file);
	strcat(config_file_sav, CONF_BACKUP_EXT);
	/* Move current configuration file to backup config file. */
	unlink(config_file_sav);
	rename(config_file, config_file_sav);
	free(config_file_sav);
	sync();
	fd = open(config_file, O_WRONLY | O_CREAT, CONFIGFILE_MASK);
	if (fd < 0)
	{
		vty_out(vty, "Can't open configuration file %s%s", config_file,
				VTY_NEWLINE);
		return CMD_WARNING;
	}
	/* Make vty for configuration file. */
	file_vty = vty_new();
	file_vty->fd = fd;
	file_vty->wfd = fd;
	file_vty->type = VTY_FILE;
	file_vty->fd_type = OS_STACK;
	/* Config file header print. */
	vty_out(file_vty, "!\n! Zebra configuration saved from vty\n");
	vty_out(file_vty, "!\n");
	vty_out(file_vty, "!");
	vty_time_print(file_vty, 1);
	vty_out(file_vty, "!\n");

	for (i = 0; i < vector_active(cmdvec); i++)
		if ((node = vector_slot(cmdvec, i)) && node->func)
		{
			if ((*node->func)(file_vty))
				vty_out(file_vty, "!\n");
		}
	vty_out(file_vty, "end%s", VTY_NEWLINE);
	vty_close(file_vty);
	sync();
	if (chmod(config_file, CONFIGFILE_MASK) != 0)
	{
		vty_out(vty, "%% Can't chmod configuration file %s: %s (%d)%s",
				config_file, safe_strerror(errno), errno, VTY_NEWLINE);
		return CMD_WARNING;
	}
	vty_out(vty, "Configuration saved to %s%s", config_file, VTY_NEWLINE);
	vty_out(vty, "[OK]%s", VTY_NEWLINE);
	host_sysconfig_sync();
	return CMD_SUCCESS;
}

ALIAS(config_write_file,
		config_write_cmd,
		"write",
		"Write running configuration to memory, network, or terminal\n")

ALIAS(config_write_file,
		config_write_memory_cmd,
		"write memory",
		"Write running configuration to memory, network, or terminal\n"
		"Write configuration to the file (same as write file)\n")

ALIAS(config_write_file,
		copy_runningconfig_startupconfig_cmd,
		"copy running-config startup-config",
		"Copy configuration\n"
		"Copy running config to... \n"
		"Copy running config to startup config (same as write file)\n")

/* Write current configuration into the terminal. */
DEFUN (config_write_terminal,
		config_write_terminal_cmd,
		"write terminal",
		"Write running configuration to memory, network, or terminal\n"
		"Write to terminal\n")
{
	ospl_uint32  i;
	struct cmd_node *node;

	if (vty->type == VTY_SHELL_SERV)
	{
		for (i = 0; i < vector_active(cmdvec); i++)
			if ((node = vector_slot(cmdvec, i)) && node->func && node->vtysh)
			{
				if ((*node->func)(vty))
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
	}
	else
	{
		vty_out(vty, "%sCurrent configuration:%s", VTY_NEWLINE,
		VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);

		for (i = 0; i < vector_active(cmdvec); i++)
			if ((node = vector_slot(cmdvec, i)) && node->func)
			{
				if ((*node->func)(vty))
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
		vty_out(vty, "end%s", VTY_NEWLINE);
	}
	return CMD_SUCCESS;
}

/* Write current configuration into the terminal. */
ALIAS(config_write_terminal,
		show_running_config_cmd,
		"show running-config",
		SHOW_STR "running configuration\n")

/* Write startup configuration into the terminal. */
DEFUN (show_startup_config,
		show_startup_config_cmd,
		"show startup-config",
		SHOW_STR
		"Contentes of startup configuration\n")
{
	char buf[BUFSIZ];
	FILE *confp;

	confp = fopen(host.config, "r");
	if (confp == NULL)
	{
		vty_out(vty, "Can't open configuration file [%s]%s", host.config,
				VTY_NEWLINE);
		return CMD_WARNING;
	}
	fseek(confp, 0, SEEK_END);
	if(ftell(confp) < 16)
	{
		fclose(confp);
		vty_out(vty, "ERROR: configuration file size %s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	fseek(confp, 0, SEEK_SET);

	while (fgets(buf, BUFSIZ, confp))
	{
		char *cp = buf;

		while (*cp != '\r' && *cp != '\n' && *cp != '\0')
			cp++;
		*cp = '\0';

		vty_out(vty, "%s%s", buf, VTY_NEWLINE);
	}

	fclose(confp);

	return CMD_SUCCESS;
}

/* Hostname configuration */
DEFUN (config_hostname,
		hostname_cmd,
		"hostname WORD",
		"Set system's network name\n"
		"This system's network name\n")
{
	if (!isalpha((int) *argv[0])) {
		vty_out(vty, "Please specify string starting with alphabet%s",
				VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (host_config_set_api(API_SET_HOSTNAME_CMD, argv[0]) != OK) {
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN (config_no_hostname,
		no_hostname_cmd,
		"no hostname [HOSTNAME]",
		NO_STR
		"Reset system's network name\n"
		"Host name of this router\n")
{
	if (host_config_set_api(API_SET_HOSTNAME_CMD, NULL) != OK)
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/* VTY interface password set. */
DEFUN (config_password,
		password_cmd,
		"password (8|) WORD",
		"Assign the terminal connection password\n"
		"Specifies a HIDDEN password will follow\n"
		"dummy string \n"
		"The HIDDEN line password string\n")
{
	/* Argument check. */
	if (argc == 0) {
		vty_out(vty, "Please specify password.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	if (argc == 2) {
		if (*argv[0] == '8') {
			vty_user_create(vty, vty->username, argv[1], ospl_false, ospl_false);
			return CMD_SUCCESS;
		} else {
			vty_out(vty, "Unknown encryption type.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}

	if (!isalnum((int) *argv[0])) {
		vty_out(vty, "Please specify string starting with alphanumeric%s",
				VTY_NEWLINE);
		return CMD_WARNING;
	}
	vty_user_create(vty, vty->username, argv[1], ospl_false, host.encrypt);
	return CMD_SUCCESS;
}

ALIAS(config_password,
		password_text_cmd,
		"password LINE",
		"Assign the terminal connection password\n"
		"The UNENCRYPTED (cleartext) line password\n")

/* VTY enable password set. */
DEFUN (config_enable_password,
		enable_password_cmd,
		"enable password (8|) WORD",
		"Modify enable password parameters\n"
		"Assign the privileged level password\n"
		"Specifies a HIDDEN password will follow\n"
		"dummy string \n"
		"The HIDDEN 'enable' password string\n")
{
	/* Argument check. */
	if (argc == 0) {
		vty_out(vty, "Please specify password.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}

	/* Crypt type is specified. */
	if (argc == 2) {
		if (*argv[0] == '8') {
			vty_user_create(vty, vty->username, argv[1], ospl_true, ospl_false);
			return CMD_SUCCESS;
		} else {
			vty_out(vty, "Unknown encryption type.%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
	}

	if (!isalnum((int) *argv[0])) {
		vty_out(vty, "Please specify string starting with alphanumeric%s",
				VTY_NEWLINE);
		return CMD_WARNING;
	}
	vty_user_create(vty, vty->username, argv[1], ospl_true, host.encrypt);
	return CMD_SUCCESS;
}

ALIAS(config_enable_password,
		enable_password_text_cmd,
		"enable password LINE",
		"Modify enable password parameters\n"
		"Assign the privileged level password\n"
		"The UNENCRYPTED (cleartext) 'enable' password\n")

/* VTY enable password delete. */
DEFUN (no_config_enable_password,
		no_enable_password_cmd,
		"no enable password",
		NO_STR
		"Modify enable password parameters\n"
		"Assign the privileged level password\n")
{
	vty_user_delete(vty, vty->username, ospl_true, ospl_false);
	return CMD_SUCCESS;
}

DEFUN (service_password_encrypt,
		service_password_encrypt_cmd,
		"service password-encryption",
		"Set up miscellaneous service\n"
		"Enable encrypted passwords\n")
{
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if (encrypt)
	{
		vty_user_encrypt_enable(ospl_true);
		return CMD_SUCCESS;
	}
	encrypt = 1;
	if (host_config_set_api(API_SET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	vty_user_encrypt_enable(ospl_true);
	return CMD_SUCCESS;
}

DEFUN (no_service_password_encrypt,
		no_service_password_encrypt_cmd,
		"no service password-encryption",
		NO_STR
		"Set up miscellaneous service\n"
		"Enable encrypted passwords\n")
{
	int encrypt = 0;
	if (host_config_get_api(API_GET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	if (!encrypt)
	{
		vty_user_encrypt_enable(ospl_false);
		return CMD_SUCCESS;
	}
	encrypt = 0;
	if (host_config_set_api(API_SET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	vty_user_encrypt_enable(ospl_false);
	return CMD_SUCCESS;
}

DEFUN (config_terminal_length,
		config_terminal_length_cmd,
		"terminal length <0-512>",
		"Set terminal line parameters\n"
		"Set number of lines on a screen\n"
		"Number of lines on screen (0 for no pausing)\n")
{
	int lines;
	char *endptr = NULL;

	lines = strtol(argv[0], &endptr, 10);
	if (lines < 0 || lines > 512 || *endptr != '\0') {
		vty_out(vty, "length is malformed%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	vty->lines = lines;
	return CMD_SUCCESS;
}

DEFUN (config_terminal_no_length,
		config_terminal_no_length_cmd,
		"terminal no length",
		"Set terminal line parameters\n"
		NO_STR
		"Set number of lines on a screen\n")
{
	vty->lines = -1;
	return CMD_SUCCESS;
}

DEFUN (service_terminal_length,
		service_terminal_length_cmd,
		"service terminal-length <0-512>",
		"Set up miscellaneous service\n"
		"System wide terminal length configuration\n"
		"Number of lines of VTY (0 means no line control)\n")
{
	int lines;
	char *endptr = NULL;

	lines = strtol(argv[0], &endptr, 10);
	if (lines < 0 || lines > 512 || *endptr != '\0') {
		vty_out(vty, "length is malformed%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if (host_config_set_api(API_SET_LINES_CMD, &lines) != OK) {
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (no_service_terminal_length,
		no_service_terminal_length_cmd,
		"no service terminal-length [<0-512>]",
		NO_STR
		"Set up miscellaneous service\n"
		"System wide terminal length configuration\n"
		"Number of lines of VTY (0 means no line control)\n")
{
	int lines = -1;
	if (host_config_set_api(API_SET_LINES_CMD, &lines) != OK) {
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN_HIDDEN (do_echo,
		echo_cmd,
		"echo .MESSAGE",
		"Echo a message back to the vty\n"
		"The message to echo\n")
{
	char *message;

	vty_out(vty, "%s%s",
			((message = argv_concat(argv, argc, 0)) ? message : ""),
			VTY_NEWLINE);
	if (message)
		XFREE(MTYPE_TMP, message);
	return CMD_SUCCESS;
}

DEFUN (banner_motd_file,
		banner_motd_file_cmd,
		"banner motd file [FILE]",
		"Set banner\n"
		"Banner for motd\n"
		"Banner from a file\n"
		"Filename\n")
{
	if (host_config_set_api(API_SET_MOTDFILE_CMD, argv[0]) != OK)
	{
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN (banner_motd_default,
		banner_motd_default_cmd,
		"banner motd default",
		"Set banner string\n"
		"Strings for motd\n"
		"Default string\n")
{
	if (host_config_set_api(API_SET_MOTDFILE_CMD, default_motd) != OK)
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (no_banner_motd,
		no_banner_motd_cmd,
		"no banner motd",
		NO_STR
		"Set banner string\n"
		"Strings for motd\n")
{
	if (host_config_set_api(API_SET_MOTDFILE_CMD, NULL) != OK)
	{
		return CMD_WARNING;
	}
	if (host_config_set_api(API_SET_MOTD_CMD, NULL) != OK)
	{
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN (system_description,
		system_description_cmd,
		"description .LINE",
		"Set system's description\n"
		"This system's description\n")
{
	if (argc)
	{
		if (host_config_set_api(API_SET_DESC_CMD, argv_concat(argv, argc, 0))!= OK)
		{
			return CMD_WARNING;
		}
		/*		if (host.description)
		 XFREE (MTYPE_TMP, host.description);
		 host.description = argv_concat(argv, argc, 0);*/
		return CMD_SUCCESS;
	} else {
		vty_out(vty, "Please specify string starting with alphabet%s",
				VTY_NEWLINE);
		return CMD_WARNING;
	}
}
DEFUN (no_system_description,
		no_system_description_cmd,
		"no description",
		NO_STR
		"system description\n")
{
	if (host_config_set_api(API_SET_DESC_CMD, NULL) != OK)
	{
		return CMD_WARNING;
	}
	/*    if (host.description)
	 XFREE (MTYPE_TMP, host.description);
	 host.description = NULL;*/
	return CMD_SUCCESS;
}

DEFUN (show_commandtree,
		show_commandtree_cmd,
		"show commandtree",
		SHOW_STR
		"Show command tree\n")
{
	/* TBD */
	vector cmd_vector;
	ospl_uint32  i;

	vty_out(vty, "Current node id: %d%s", vty->node, VTY_NEWLINE);

	/* vector of all commands installed at this node */
	cmd_vector = vector_copy(cmd_node_vector(cmdvec, vty->node));

	/* loop over all commands at this node */
	for (i = 0; i < vector_active(cmd_vector); ++i) {
		struct cmd_element *cmd_element;

		/* A cmd_element (seems to be) is an individual command */
		if ((cmd_element = vector_slot(cmd_vector, i)) == NULL)
			continue;

		vty_out(vty, "    %s%s", cmd_element->string, VTY_NEWLINE);
	}

	vector_free(cmd_vector);
	return CMD_SUCCESS;
}

DEFUN (show_time_queues,
		show_time_queues_cmd,
		"show time-queues",
		SHOW_STR
		"Show Time Queues\n")
{
	os_time_show(vty_out, vty);
	return CMD_SUCCESS;
}


/* This function write configuration of this host. */
static int
config_write_host (struct vty *vty)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if (host.name)
		vty_out(vty, "hostname %s%s", host.name, VTY_NEWLINE);

	if (host.description) {
		if (vty->type == VTY_FILE)
			vty_out(vty, "description %s%s", host.description, VTY_NEWLINE);
		else
			vty_out(vty, "! description:%s%s", host.description, VTY_NEWLINE);
	}
	vty_out(vty, "!%s", VTY_NEWLINE);
	if (host.mutx)
		os_mutex_unlock(host.mutx);

	vty_user_config_write(vty);

	vty_out(vty, "!%s", VTY_NEWLINE);
	if (zlog_default->mutex)
		os_mutex_lock(zlog_default->mutex, OS_WAIT_FOREVER);


	if (zlog_default->trap_lvl)
	{
		vty_out(vty, "! N.B. The 'log trapping' command is deprecated.%s",VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);
		//vty_out(vty, "log trapping %s%s", zlog_priority_name(zlog_default->trap_lvl), VTY_NEWLINE);
	}
	if (vty->trapping)
		vty_out(vty, "log trapping %s", VTY_NEWLINE);

	if (host.logfile
			&& (zlog_default->maxlvl[ZLOG_DEST_FILE] != ZLOG_DISABLED)) {
		vty_out(vty, "log file %s", host.logfile);
		if (zlog_default->maxlvl[ZLOG_DEST_FILE] != zlog_default->default_lvl[ZLOG_DEST_FILE])
			vty_out(vty, " %s",
					zlog_priority_name(zlog_default->maxlvl[ZLOG_DEST_FILE]));
		vty_out(vty, "%s", VTY_NEWLINE);
		if(zlog_default->filesize != ZLOG_FILE_SIZE)
			vty_out(vty, "log file size %d%s", zlog_default->filesize, VTY_NEWLINE);
	}

	if (zlog_default->maxlvl[ZLOG_DEST_STDOUT] != ZLOG_DISABLED) {
		vty_out(vty, "log stdout");
		if (zlog_default->maxlvl[ZLOG_DEST_STDOUT] != zlog_default->default_lvl[ZLOG_DEST_STDOUT])
			vty_out(vty, " %s",
					zlog_priority_name(zlog_default->maxlvl[ZLOG_DEST_STDOUT]));
		vty_out(vty, "%s", VTY_NEWLINE);
	}

	if (zlog_default->maxlvl[ZLOG_DEST_MONITOR] != ZLOG_DISABLED && vty->type != VTY_FILE)
	{
		vty_out(vty, "log monitor");
		if (zlog_default->maxlvl[ZLOG_DEST_MONITOR] != zlog_default->default_lvl[ZLOG_DEST_MONITOR])
			vty_out(vty, "%s",
					zlog_priority_name(zlog_default->maxlvl[ZLOG_DEST_MONITOR]));
		vty_out(vty, "%s", VTY_NEWLINE);
	}

	if (zlog_default->maxlvl[ZLOG_DEST_BUFFER] != ZLOG_DISABLED)
	{
		vty_out(vty, "log buffer");
		if (zlog_default->maxlvl[ZLOG_DEST_BUFFER] != zlog_default->default_lvl[ZLOG_DEST_BUFFER])
			vty_out(vty, "%s",
					zlog_priority_name(zlog_default->maxlvl[ZLOG_DEST_BUFFER]));
		vty_out(vty, "%s", VTY_NEWLINE);

		if(zlog_default->log_buffer.max_size != ZLOG_BUFF_SIZE)
			vty_out(vty, "log buffer size %d%s", zlog_default->log_buffer.max_size, VTY_NEWLINE);
	}
	if (zlog_default->maxlvl[ZLOG_DEST_SYSLOG] != ZLOG_DISABLED)
	{
		vty_out(vty, "log syslog");
		if (zlog_default->maxlvl[ZLOG_DEST_SYSLOG] != zlog_default->default_lvl[ZLOG_DEST_SYSLOG])
			vty_out(vty, " %s",
					zlog_priority_name(zlog_default->maxlvl[ZLOG_DEST_SYSLOG]));
		vty_out(vty, "%s", VTY_NEWLINE);
	}
#ifdef PL_SERVICE_SYSLOG
		if (syslogc_is_enable()) {
			int port = 0;
			char log_host[32];
			memset(log_host, 0, sizeof(log_host));
			if (syslogc_host_config_get(log_host, &port, NULL) == 0) {
				if(syslogc_is_dynamics())
				{
					memset(log_host, 0, sizeof(log_host));
					sprintf(log_host, "dynamics");
				}
				if (port == SYSLOGC_DEFAULT_PORT)
					vty_out(vty, "syslog host %s%s", log_host, VTY_NEWLINE);
				else
					vty_out(vty, "syslog host %s port %d%s", log_host, port, VTY_NEWLINE);

				if (syslogc_mode_get(&port) == 0) {
					if (port == SYSLOG_TCP_MODE)
						vty_out(vty, "syslog mode tcp%s", VTY_NEWLINE);
				}
			}
		}
#endif

	if (zlog_default->facility != LOG_DAEMON)
		vty_out(vty, "log facility %s%s", zlog_facility_name(zlog_default->facility),
				VTY_NEWLINE);

	if (zlog_default->record_priority == 1)
		vty_out(vty, "log record-priority%s", VTY_NEWLINE);

	if(zlog_default->timestamp == ZLOG_TIMESTAMP_NONE)
		vty_out(vty, "log timestamp none%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_DATE)
		vty_out(vty, "log timestamp date%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_SHORT)
		vty_out(vty, "log timestamp ospl_int16%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_BSD)
		vty_out(vty, "log timestamp bsd%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_ISO)
		vty_out(vty, "log timestamp iso%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_RFC3164)
		vty_out(vty, "log timestamp rfc3164%s",VTY_NEWLINE);
	else if(zlog_default->timestamp == ZLOG_TIMESTAMP_RFC3339)
		vty_out(vty, "log timestamp rfc3339%s",VTY_NEWLINE);

/*
	if (zlog_default->timestamp_precision > 0)
		vty_out(vty, "log timestamp precision %d%s",
				zlog_default->timestamp_precision, VTY_NEWLINE);
*/

	if (zlog_default->mutex)
		os_mutex_unlock(zlog_default->mutex);

	vty_out(vty, "!%s", VTY_NEWLINE);
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	if (host.encrypt)
		vty_out(vty, "service password-encryption%s", VTY_NEWLINE);

	if (host.lines >= 0)
		vty_out(vty, "service terminal-length %d%s", host.lines, VTY_NEWLINE);

	if (host.motdfile)
		vty_out(vty, "banner motd file %s%s", host.motdfile, VTY_NEWLINE);
	else if (!host.motd)
		vty_out(vty, "no banner motd%s", VTY_NEWLINE);
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	vty_out(vty, "!%s", VTY_NEWLINE);
	vty_out(vty, "!%s", VTY_NEWLINE);
#ifdef PL_NSM_MODULE
	nsm_client_write_config (0, vty);
#endif
	return 1;
}


static int
config_write_service (struct vty *vty)
{
	#ifdef PL_NSM_MODULE
	nsm_ip_arp_config(vty);
	vty_out(vty, "!%s", VTY_NEWLINE);
	nsm_ip_dns_host_config(vty);
	vty_out(vty, "!%s", VTY_NEWLINE);
	nsm_client_service_write_config (0, vty);
	#endif
	return 1;
}


static int _cmd_host_base_init(ospl_bool terminal)
{
	/*  install_node (&restricted_node, NULL);*/
	install_node(&user_node, NULL);
	install_node(&view_node, NULL);
	install_node(&enable_node, NULL);
	install_node(&auth_node, NULL);
	install_node(&auth_enable_node, NULL);
	install_node(&service_node, config_write_service);
	install_node(&config_node, config_write_host);
	install_node(&interface_node, NULL);
	install_node(&interface_l3_node, NULL);
	install_node(&tunnel_interface_node, NULL);
	install_node(&loopback_interface_node, NULL);
	install_node(&lag_interface_node, NULL);
	install_node(&lag_interface_l3_node, NULL);
	install_node(&serial_interface_node, NULL);
	install_node(&trunk_group_node, NULL);
	install_node(&brigde_interface_node, NULL);
	install_node(&interface_wireless_node, NULL);

	install_node(&all_service_node, nsm_template_service_write_config);
	install_node(&template_node, nsm_template_write_config);

#ifdef CUSTOM_INTERFACE
	install_node(&wifi_interface_node, NULL);
	install_node(&modem_interface_node, NULL);
#endif
	install_node(&modem_profile_node, NULL);
	install_node(&modem_channel_node, NULL);

	install_default(VIEW_NODE);
	install_default(CONFIG_NODE);
	install_default_basic(VIEW_NODE);
	install_default_basic(CONFIG_NODE);

	install_default(TEMPLATE_NODE);
	install_default_basic(TEMPLATE_NODE);

	/* Each node's basic commands. */
	install_element(VIEW_NODE, &show_version_cmd);
	install_element(VIEW_NODE, &show_hidden_version_cmd);
	install_element(VIEW_NODE, &show_system_cmd);
	//install_element(VIEW_NODE, &show_system_detail_cmd);


	install_element(VIEW_NODE, &config_enable_cmd);
	install_element(VIEW_NODE, &config_terminal_length_cmd);
	install_element(VIEW_NODE, &config_terminal_no_length_cmd);

	install_element(VIEW_NODE, &show_commandtree_cmd);
	install_element(VIEW_NODE, &echo_cmd);

	install_default(ENABLE_NODE);
	install_element(ENABLE_NODE, &config_disable_cmd);
	install_element(ENABLE_NODE, &config_terminal_cmd);
	install_element(ENABLE_NODE, &copy_runningconfig_startupconfig_cmd);

	install_element(ENABLE_NODE, &show_startup_config_cmd);


	install_element(CONFIG_NODE, &hostname_cmd);
	install_element(CONFIG_NODE, &no_hostname_cmd);


	install_element(CONFIG_NODE, &system_description_cmd);
	install_element(CONFIG_NODE, &no_system_description_cmd);


	install_element(CONFIG_NODE, &password_cmd);
	install_element(CONFIG_NODE, &password_text_cmd);
	install_element(CONFIG_NODE, &enable_password_cmd);
	install_element(CONFIG_NODE, &enable_password_text_cmd);
	install_element(CONFIG_NODE, &no_enable_password_cmd);


	install_element(CONFIG_NODE, &service_password_encrypt_cmd);
	install_element(CONFIG_NODE, &no_service_password_encrypt_cmd);

	install_element(CONFIG_NODE, &banner_motd_default_cmd);
	install_element(CONFIG_NODE, &banner_motd_file_cmd);
	install_element(CONFIG_NODE, &no_banner_motd_cmd);
	install_element(CONFIG_NODE, &service_terminal_length_cmd);
	install_element(CONFIG_NODE, &no_service_terminal_length_cmd);

	install_element(VIEW_NODE, &show_work_queues_cmd);
	install_element(VIEW_NODE, &show_time_queues_cmd);

	install_element(CONFIG_NODE, &show_commandtree_cmd);
	return OK;
}

int cmd_host_init(ospl_bool terminal)
{
	return _cmd_host_base_init(terminal);
}

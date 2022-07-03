/*
 * cmd__global_host.c
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "cli_node.h"
#include "zmemory.h"
#include "vector.h"
#include "command.h"
#include "vty.h"
#include "vty_user.h"
#include "host.h"
#include "workqueue.h"

#define CMD_HOST_DEBUG

extern vector cmdvec;

/* Standard command node structures. */
DEFUN_NODE(host_node, HOST_NODE, "%s(config)# ", 1);
DEFUN_NODE(hostsrv_node, HOSTSRV_NODE, "%s(config)# ", 1);
DEFUN_NODE(login_node, LOGIN_NODE, "username: ", 0);
DEFUN_NODE(auth_node, AUTH_NODE, "Password: ", 0);
DEFUN_NODE(view_node, VIEW_NODE, "%s> ", 1);
DEFUN_NODE(auth_enable_node, AUTH_ENABLE_NODE, "Password: ", 0);
DEFUN_NODE(enable_node, ENABLE_NODE, "%s# ", 1);
DEFUN_NODE(config_node, CONFIG_NODE, "%s(config)# ", 1);
#if defined(ZPL_SDK_MODULE)
DEFUN_NODE(config_sdk_node, SDK_NODE, "%s(config-sdk)# ", 1);
#endif
/*
static struct cmd_node host_node =
{
	HOST_NODE,
	"%s(config)# ",
	1
};

static struct cmd_node hostsrv_node =
{
	HOSTSRV_NODE,
	"%s(config)# ",
	1
};

static struct cmd_node login_node =
{
	LOGIN_NODE,
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
*/



/* Configration from terminal */
DEFUN (config_terminal,
		config_terminal_cmd,
		"configure terminal",
		"Configuration from vty interface\n"
		"Configuration terminal\n")
{
	if (vty_user_getting_privilege(vty, vty->username) <= CMD_ENABLE_LEVEL) {
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
	if (vty_user_getting_privilege(vty, vty->username) < CMD_ENABLE_LEVEL) {
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
		if (vty_shell(vty))
			exit(0);
		else
			vty->status = VTY_CLOSE;
		break;
	case ENABLE_NODE:
		vty->node = VIEW_NODE;
		break;
#if defined(ZPL_SDK_MODULE)
	case SDK_NODE:
#endif
	case CONFIG_NODE:
		vty->node = ENABLE_NODE;
		vty_config_unlock(vty);
		break;
	default:
		vty->node = cmd_exit_node(vty);
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
	if(vty->node >= CONFIG_NODE && vty->node < CMD_NODE_MAX)
	{
		vty_config_unlock(vty);
	}
	vty->node = cmd_end_node(vty);

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
#ifdef ZPL_BUILD_TIME
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(ZPL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " Build Time    : %s %s .%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* ZPL_BUILD_TIME */

#if 0
	vty_out(vty, " Hello this is \"%s\" (version:%s).%s", OEM_PACKAGE_NAME,
				OEM_VERSION, VTY_NEWLINE);
	vty_out(vty, " %s%s", OEM_PACKAGE_COPYRIGHT, VTY_NEWLINE);
	vty_out(vty, " Design it Base on (%s).%s", OEM_PACKAGE_BASE,
				VTY_NEWLINE);
	if ((GIT_SUFFIX) && (strlen(GIT_SUFFIX) > 2))
		vty_out(vty, " Git suffex : %s%s Git info:%s%s", GIT_SUFFIX,
					VTY_NEWLINE, GIT_INFO, VTY_NEWLINE);
#ifdef ZPL_BUILD_TIME
	//20181024160232
	vty_out(vty, " It't make: %s.%s", os_build_time2date(ZPL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " It't make: %s.%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " It't make: %s %s.%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* ZPL_BUILD_TIME */
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
#ifdef ZPL_BUILD_TIME
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(ZPL_BUILD_TIME), VTY_NEWLINE);
#else
#ifdef OEM_MAKE_DATE
	//20181024160232
	vty_out(vty, " Build Time    : %s .%s", os_build_time2date(OEM_MAKE_DATE), VTY_NEWLINE);
#else
	vty_out(vty, " Build Time    : %s %s .%s", __DATE__, __TIME__, VTY_NEWLINE);
#endif /* OEM_MAKE_DATE */
#endif /* ZPL_BUILD_TIME */
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
	zpl_uint32  i;
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
	zpl_uint32  i;
	int fd;
	struct cmd_node *node;
	char *config_file;
	char *config_file_sav = NULL;
	struct vty *file_vty;
#ifdef ZPL_ACTIVE_STANDBY
	if(host_isstandby())
		return CMD_SUCCESS;
#endif		
	/* Check and see if we are operating under vtysh configuration */
	if (_global_host.config == NULL)
	{
		vty_out(vty, "Can't save to configuration file, configuration file is not exist.%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	/* Get filename. */
	config_file = _global_host.config;
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
	ipstack_init(OS_STACK, file_vty->fd);
	ipstack_init(OS_STACK, file_vty->wfd);
	file_vty->fd._fd = fd;
	file_vty->wfd._fd = fd;
	file_vty->type = VTY_FILE;
	
	vty_out(file_vty, "!\n! Zebra configuration saved from vty\n");
	vty_out(file_vty, "!\n");
	vty_out(file_vty, "!");
	vty_time_print(file_vty, 1);
	vty_out(file_vty, "!\n");

	for (i = 0; i < vector_active(cmdvec); i++)
		if ((node = vector_slot(cmdvec, i)) && node->func)
		{
#ifdef ZPL_BUILD_DEBUG
  			zpl_backtrace_symb_set(node->funcname, NULL, 1);
#endif
			if ((*node->func)(file_vty))
				vty_out(file_vty, "!\n");
		}
	vty_out(file_vty, "end%s", VTY_NEWLINE);
	vty_close(file_vty);
	sync();
	if (chmod(config_file, CONFIGFILE_MASK) != 0)
	{
		vty_out(vty, "%% Can't chmod configuration file %s: %s (%d)%s",
				config_file, ipstack_strerror(ipstack_errno), ipstack_errno, VTY_NEWLINE);
		return CMD_WARNING;
	}
	vty_out(vty, "Configuration saved to %s%s", config_file, VTY_NEWLINE);
	vty_out(vty, "[OK]%s", VTY_NEWLINE);
	host_sysconfig_sync();
#ifdef ZPL_ACTIVE_STANDBY
	if(!host_isstandby())
	{
		return CMD_SUCCESS;
	}
#endif
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
	zpl_uint32  i;
	struct cmd_node *node;

	if (vty->type == VTY_SHELL_SERV)
	{
		for (i = 0; i < vector_active(cmdvec); i++)
			if ((node = vector_slot(cmdvec, i)) && node->func && node->vtysh)
			{
#ifdef ZPL_BUILD_DEBUG
  				zpl_backtrace_symb_set(node->funcname, NULL, 1);
#endif
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
#ifdef ZPL_BUILD_DEBUG
  				zpl_backtrace_symb_set(node->funcname, NULL, 1);
#endif
				if ((*node->func)(vty))
					vty_out(vty, "!%s", VTY_NEWLINE);
			}
		vty_out(vty, "end%s", VTY_NEWLINE);
		vty_out(vty, "!%s", VTY_NEWLINE);
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

	confp = fopen(_global_host.config, "r");
	if (confp == NULL)
	{
		vty_out(vty, "Can't open configuration file [%s]%s", _global_host.config,
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
			vty_user_create(vty, vty->username, argv[1], zpl_false, zpl_false);
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
	vty_user_create(vty, vty->username, argv[1], zpl_false, _global_host.encrypt);
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
			vty_user_create(vty, vty->username, argv[1], zpl_true, zpl_false);
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
	vty_user_create(vty, vty->username, argv[1], zpl_true, _global_host.encrypt);
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
	vty_user_delete(vty, vty->username, zpl_true, zpl_false);
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
		vty_user_encrypt_enable(zpl_true);
		return CMD_SUCCESS;
	}
	encrypt = 1;
	if (host_config_set_api(API_SET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	vty_user_encrypt_enable(zpl_true);
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
		vty_user_encrypt_enable(zpl_false);
		return CMD_SUCCESS;
	}
	encrypt = 0;
	if (host_config_set_api(API_SET_ENCRYPT_CMD, &encrypt) != OK) {
		return CMD_WARNING;
	}
	vty_user_encrypt_enable(zpl_false);
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
	int lines = 0;
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
		/*		if (_global_host.description)
		 XFREE (MTYPE_TMP, _global_host.description);
		 _global_host.description = argv_concat(argv, argc, 0);*/
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
	/*    if (_global_host.description)
	 XFREE (MTYPE_TMP, _global_host.description);
	 _global_host.description = NULL;*/
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
	zpl_uint32  i;

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

#ifdef ZPL_ACTIVE_STANDBY
DEFUN(show_ipcstandby_state,
      show_ipcstandby_state_cmd,
      "show standby",
      SHOW_STR
      "Standby information\n")
{
  	if (_global_host.active_standby)
    	vty_out(vty, " Active Standby Mode : Standby%s", VTY_NEWLINE);
	else 
		vty_out(vty, " Active Standby Mode : Master%s", VTY_NEWLINE);
  	return CMD_SUCCESS;
}
#endif

/* This function write configuration of this _global_host. */
static int
config_write_host (struct vty *vty)
{
	if (_global_host.mutex)
		os_mutex_lock(_global_host.mutex, OS_WAIT_FOREVER);
	if (_global_host.name)
		vty_out(vty, "hostname %s%s", _global_host.name, VTY_NEWLINE);

	if (_global_host.description) {
		if (vty->type == VTY_FILE)
			vty_out(vty, "description %s%s", _global_host.description, VTY_NEWLINE);
		else
			vty_out(vty, "! description:%s%s", _global_host.description, VTY_NEWLINE);
	}
	vty_out(vty, "!%s", VTY_NEWLINE);

	if(vty->type != VTY_FILE && vty->username)
	{
		vty_out(vty,"! current login user:%s %s", vty->username, VTY_NEWLINE);
		vty_out(vty,"!%s", VTY_NEWLINE);
	}
	if (_global_host.mutex)
		os_mutex_unlock(_global_host.mutex);
	return 1;
}

static int
config_write_hostsrv (struct vty *vty)
{
	if (_global_host.mutex)
		os_mutex_lock(_global_host.mutex, OS_WAIT_FOREVER);

	if (_global_host.encrypt)
		vty_out(vty, "service password-encryption%s", VTY_NEWLINE);

	if (_global_host.lines > 0)
		vty_out(vty, "service terminal-length %d%s", _global_host.lines, VTY_NEWLINE);

	if (_global_host.motdfile)
		vty_out(vty, "banner motd file %s%s", _global_host.motdfile, VTY_NEWLINE);
	else if (!_global_host.motd)
		vty_out(vty, "no banner motd%s", VTY_NEWLINE);
		
	if (_global_host.mutex)
		os_mutex_unlock(_global_host.mutex);
	return 1;
}

#if defined(ZPL_SDK_MODULE)
DEFUN (into_sdk_node,
		into_sdk_node_cmd,
		"sdk view",
		"SDK View Node\n"
		"View Node\n")
{
	vty->node = SDK_NODE;
	return CMD_SUCCESS;
}
#endif

static int _cmd_host_base_init(zpl_bool terminal)
{
	install_node(&host_node, config_write_host);
	install_node(&hostsrv_node, config_write_hostsrv);
	install_node(&login_node, NULL);
	install_node(&view_node, NULL);
	install_node(&enable_node, NULL);
	install_node(&auth_node, NULL);
	install_node(&auth_enable_node, NULL);
	install_node(&config_node, NULL);

#if defined(ZPL_SDK_MODULE)
	install_node(&config_sdk_node, NULL);
	install_default(SDK_NODE);
	install_default_basic(SDK_NODE);
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &into_sdk_node_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &into_sdk_node_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &into_sdk_node_cmd);
#endif

	install_default(VIEW_NODE);
	install_default(CONFIG_NODE);
	install_default_basic(VIEW_NODE);
	install_default_basic(CONFIG_NODE);


	/* Each node's basic commands. */
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_version_cmd);
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_hidden_version_cmd);
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_system_cmd);
	//install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_system_detail_cmd);


	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &config_enable_cmd);
	install_element(VIEW_NODE, CMD_CONFIG_LEVEL, &config_terminal_length_cmd);
	install_element(VIEW_NODE, CMD_CONFIG_LEVEL, &config_terminal_no_length_cmd);

	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_commandtree_cmd);
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &echo_cmd);
	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &echo_cmd);
	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &echo_cmd);

	install_default(ENABLE_NODE);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &config_disable_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &config_terminal_cmd);
	install_element(ENABLE_NODE, CMD_CONFIG_LEVEL, &copy_runningconfig_startupconfig_cmd);

	install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_startup_config_cmd);
	//install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_running_config_cmd);
	//install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &config_write_terminal_cmd);

	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &hostname_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_hostname_cmd);


	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &system_description_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_system_description_cmd);


	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &password_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &password_text_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &enable_password_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &enable_password_text_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_enable_password_cmd);


	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &service_password_encrypt_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_service_password_encrypt_cmd);

	install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &banner_motd_default_cmd);
	install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &banner_motd_file_cmd);
	install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_banner_motd_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &service_terminal_length_cmd);
	install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_service_terminal_length_cmd);
#ifdef ZPL_WORKQUEUE
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_work_queues_cmd);
#endif	
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_time_queues_cmd);

	install_element(CONFIG_NODE, CMD_VIEW_LEVEL, &show_commandtree_cmd);

#ifdef ZPL_ACTIVE_STANDBY
	install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_ipcstandby_state_cmd);
#endif
	return OK;
}

int cmd_host_init(zpl_bool terminal)
{
	return _cmd_host_base_init(terminal);
}

/*
 * cmd_ssh.c
 *
 *  Created on: Nov 6, 2018
 *      Author: zhurish
 */



#include "ssh_def.h"
#include "prefix.h"
#include "vty_include.h"

#ifdef ZPL_LIBSSH_MODULE
#include "ssh_api.h"

DEFUN (ssh_service_enable,
		ssh_service_enable_cmd,
	    "ssh service enable",
		"SSH configure\n"
		"Service configure\n"
		"Enable\n")
{
	if(ssh_enable_api(zpl_true) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (no_ssh_service_enable,
		no_ssh_service_enable_cmd,
	    "no ssh service enable",
		NO_STR
		"SSH configure\n"
		"Service configure\n"
		"Enable\n")
{
	if(ssh_enable_api(zpl_false) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (sftp_service_enable,
		sftp_service_enable_cmd,
	    "sftp service enable",
		"SFTP configure\n"
		"Service configure\n"
		"Enable\n")
{
	if(ssh_enable_api(zpl_true) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (no_sftp_service_enable,
		no_sftp_service_enable_cmd,
	    "no sftp service enable",
		NO_STR
		"SFTP configure\n"
		"Service configure\n"
		"Enable\n")
{
	if(ssh_enable_api(zpl_false) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ssh_bind_address,
		ssh_bind_address_cmd,
	    "ip ssh bind-address "CMD_KEY_IPV4,
		IP_STR
		"SSH configure\n"
		"bind address configure\n"
		CMD_KEY_IPV4_HELP)
{
	struct prefix cp;
	if(ssh_is_running_api())
	{
		ssh_enable_api(zpl_false);
	}
	if(argc == 1)
	{
		if(strstr(argv[0], "."))
		{
			if (str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&cp) <= 0)
			{
				vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
				return CMD_WARNING;
			}
			if(ssh_bind_address_api(&cp) == OK)
			{
				ssh_enable_api(zpl_true);
				return CMD_SUCCESS;
			}
		}
		else
		{
			if(ssh_bind_port_api(atoi(argv[0])) == OK)
			{
				ssh_enable_api(zpl_true);
				return CMD_SUCCESS;
			}
		}
	}
	else if(argc == 2)
	{
		if (str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&cp) <= 0)
		{
			vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(ssh_bind_address_api(&cp) != OK)
		{
			//ssh_enable_api(zpl_true);
			return CMD_WARNING;
		}
		if(ssh_bind_port_api(atoi(argv[1])) == OK)
		{
			ssh_enable_api(zpl_true);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

ALIAS(ssh_bind_address,
		ssh_bind_address_port_cmd,
	    "ip ssh bind-address "CMD_KEY_IPV4 " port <1-65535>",
		IP_STR
		"SSH configure\n"
		"bind address configure\n"
		CMD_KEY_IPV4_HELP
		"bind port configure\n"
		"port value(default 22)\n");

ALIAS(ssh_bind_address,
		ssh_bind_port_cmd,
	    "ip ssh bind-port <1-65535>",
		IP_STR
		"SSH configure\n"
		"bind port configure\n"
		"port value(default 22)\n");

/*DEFUN (ssh_bind_port,
		ssh_bind_port_cmd,
	    "ssh bind-port <1-65535>",
		"SSH configure\n"
		"bind port configure\n"
		"port value(default 22)\n")
{
	if(ssh_is_running_api())
	{
		ssh_enable_api(zpl_false);
	}
	if(ssh_bind_port_api(atoi(argv[0])) == OK)
	{
		ssh_enable_api(zpl_true);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}*/

DEFUN (ssh_login_set,
		ssh_login_set_cmd,
	    "ip ssh login enable",
		IP_STR
		"SSH configure\n"
		"Login configure\n"
		"Enbale login\n")
{
	if(ssh_login_api(zpl_true) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_login_set,
		no_ssh_login_set_cmd,
	    "no ip ssh login enable",
		NO_STR
		IP_STR
		"SSH configure\n"
		"Login configure\n"
		"Enbale login\n")
{
	if(ssh_login_api(zpl_false) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (ssh_version_set,
		ssh_version_set_cmd,
	    "ip ssh version (1|2|all)",
		IP_STR
		"SSH configure\n"
		"version configure\n"
		"SSH1 version\n"
		"SSH2 version\n"
		"SSH1 and SSH2 version\n")
{
	int ver = 0;
	if(ssh_is_running_api())
	{
		ssh_enable_api(zpl_false);
	}
	if(strstr(argv[0], "all"))
		ver = 3;
	else
		ver = atoi(argv[0]);
	if(ssh_version_api(ver) == OK)
	{
		ssh_enable_api(zpl_true);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_version_set,
		no_ssh_version_set_cmd,
	    "no ip ssh version",
		NO_STR
		IP_STR
		"SSH configure\n"
		"version configure\n")
{
	int ver = 0;
	if(ssh_is_running_api())
	{
		ssh_enable_api(zpl_false);
	}
	ver = 3;
	if(ssh_version_api(ver) == OK)
	{
		ssh_enable_api(zpl_true);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (ssh_authentication_type_set,
		ssh_authentication_type_set_cmd,
	    "ip ssh authentication-type (auto|none|password|public|hostbase)",
		IP_STR
		"SSH configure\n"
		"Authentication Type configure\n"
		"Auto Authentication\n"
		"None Authentication\n"
		"Password Authentication\n"
		"Public Authentication\n"
		"Hostbase Authentication\n")
{
	int ver = 0;
	if(strstr(argv[0], "auto"))
		ver = SSH_AUTH_AUTO;
	else if(strstr(argv[0], "none"))
		ver = SSH_AUTH_NONE;
	else if(strstr(argv[0], "password"))
		ver = SSH_AUTH_PASSWORD;
	else if(strstr(argv[0], "public"))
		ver = SSH_AUTH_PUBLIC_KEY;
	else if(strstr(argv[0], "hostbase"))
		ver = SSH_AUTH_HOSTBASE;

	if(ssh_authentication_type_api(ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_authentication_type_set,
		no_ssh_authentication_type_set_cmd,
	    "no ip ssh authentication-type",
		NO_STR
		IP_STR
		"SSH configure\n"
		"Authentication Type configure\n")
{
	int ver = 0;

	ver = SSH_AUTH_AUTO;

	if(ssh_authentication_type_api(ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}


DEFUN (ssh_authentication_retries_set,
		ssh_authentication_retries_set_cmd,
	    "ip ssh authentication-retries <1-16>",
		IP_STR
		"SSH configure\n"
		"Authentication Retries configure\n"
		"Auto Authentication\n"
)
{
	int ver = atoi(argv[0]);

	if(ssh_authentication_retries_api(ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_authentication_retries_set,
		no_ssh_authentication_retries_set_cmd,
	    "no ip ssh authentication-retries",
		NO_STR
		IP_STR
		"SSH configure\n"
		"Authentication Retries configure\n")
{
	int ver = 0;

	ver = 3;

	if(ssh_authentication_retries_api(ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

//crypto key generate rsa
DEFUN (ssh_key_generate,
		ssh_key_generate_cmd,
	    "ip ssh key-generate NAME type (rsa|dsa|ecdsa)",
		IP_STR
		"SSH configure\n"
		"key-generate configure\n"
		"Key Name\n"
		"Key type\n"
		"Rsa Key\n"
		"Dsa Key\n"
		"Ecdsa Key\n")
{
	int timeval = 0;
	zpl_uint32 type = SSH_KEYTYPE_UNKNOWN;
	if(memcmp(argv[0], "rsa", 2) == 0)
		type = SSH_KEYTYPE_RSA;
	else if(memcmp(argv[0], "dsa", 2) == 0)
		type = SSH_KEYTYPE_DSS;
	else if(memcmp(argv[0], "ecdsa", 2) == 0)
		type = SSH_KEYTYPE_ECDSA;
	if(type == SSH_KEYTYPE_UNKNOWN)
		return CMD_WARNING;
	timeval = os_time(NULL);
	if(ssh_generate_key_api(vty, type, argv[0]) == OK)
	{
		vty_sync_out(vty, "[OK] (elapsed time was %d seconds)%s", os_time(NULL) - timeval, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_key_generate,
		no_ssh_key_generate_cmd,
	    "no ip ssh key-generate NAME",
		NO_STR
		IP_STR
		"SSH configure\n"
		"key-generate configure\n"
		"Key Name\n")
{
	if(ssh_key_delete_api( argv[0]) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ssh_key_import,
		ssh_key_import_cmd,
	    "ip ssh key import NAME (public|private) FILENAME",
		IP_STR
		"SSH configure\n"
		"key configure\n"
		"Import Key\n"
		"Key Name\n"
		"Public Type\n"
		"Private Type\n"
		"Key File Name\n")
{
	zpl_uint32 type = 0;
	if(memcmp(argv[1], "public", 2) == 0)
		type = 1;
	else if(memcmp(argv[1], "private", 2) == 0)
		type = 2;
	if(ssh_keymgt_import_api( argv[0], type, argv[2], NULL) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

DEFUN (ssh_key_export,
		ssh_key_export_cmd,
	    "ip ssh key export NAME (public|private) FILENAME",
		IP_STR
		"SSH configure\n"
		"key configure\n"
		"Import Key\n"
		"Key Name\n"
		"Public Type\n"
		"Private Type\n"
		"Key File Name\n")
{
	zpl_uint32 type = 0;
	if(memcmp(argv[1], "public", 2) == 0)
		type = 1;
	else if(memcmp(argv[1], "private", 2) == 0)
		type = 2;
	if(ssh_keymgt_export_api( argv[0], type, argv[2], NULL) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ssh_key_set,
		ssh_key_set_cmd,
	    "ip ssh (hostkey|rsakey|dsakey|ecdsakey) FILENAME",
		IP_STR
		"SSH configure\n"
		"Host key\n"
		"Rsa Key\n"
		"Dsa Key\n"
		"Ecdsa Key\n"
		"Key File Name\n")
{
	zpl_uint32 type = 0;
	if(memcmp(argv[0], "hostkey", 2) == 0)
		type = SSH_BIND_OPTIONS_HOSTKEY;
	else if(memcmp(argv[0], "rsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_RSAKEY;
	else if(memcmp(argv[0], "dsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_DSAKEY;
	else if(memcmp(argv[0], "ecdsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_ECDSAKEY;

	if(ssh_keyfile_api(type, argv[1]) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

DEFUN (no_ssh_key_set,
		no_ssh_key_set_cmd,
	    "no ip ssh (hostkey|rsakey|dsakey|ecdsakey)",
		NO_STR
		IP_STR
		"SSH configure\n"
		"Host key\n"
		"Rsa Key\n"
		"Dsa Key\n"
		"Ecdsa Key\n")
{
	zpl_uint32 type = 0;
	if(memcmp(argv[0], "hostkey", 2) == 0)
		type = SSH_BIND_OPTIONS_HOSTKEY;
	else if(memcmp(argv[0], "rsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_RSAKEY;
	else if(memcmp(argv[0], "dsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_DSAKEY;
	else if(memcmp(argv[0], "ecdsakey", 2) == 0)
		type = SSH_BIND_OPTIONS_ECDSAKEY;

	if(ssh_keyfile_api(type, NULL) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ssh_debug_set,
		ssh_debug_set_cmd,
	    "debug ssh (all|event|packet|protocol|error)",
		DEBUG_STR
		"SSH configure\n"
		"All debug\n"
		"Event debug\n"
		"Packet debug\n"
		"Protocol debug\n"
		"Error debug\n")
{
	int ver = 0;
	if(strstr(argv[0], "all"))
		ver = SSH_DEBUG_EVENT|SSH_DEBUG_PACKET|SSH_DEBUG_PROTO|SSH_DEBUG_ERROR;
	else if(strstr(argv[0], "event"))
		ver = SSH_DEBUG_EVENT;
	else if(strstr(argv[0], "packet"))
		ver = SSH_DEBUG_PACKET;
	else if(strstr(argv[0], "protocol"))
		ver = SSH_DEBUG_PROTO;
	else if(strstr(argv[0], "error"))
		ver = SSH_DEBUG_ERROR;

	if(ssh_debug_api(1, ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (no_ssh_debug_set,
		no_ssh_debug_set_cmd,
	    "no debug ssh (all|event|packet|protocol|error)",
		NO_STR
		DEBUG_STR
		"SSH configure\n"
		"All debug\n"
		"Event debug\n"
		"Packet debug\n"
		"Protocol debug\n"
		"Error debug\n")
{
	int ver = 0;
	if(strstr(argv[0], "all"))
		ver = SSH_DEBUG_EVENT|SSH_DEBUG_PACKET|SSH_DEBUG_PROTO|SSH_DEBUG_ERROR;
	else if(strstr(argv[0], "event"))
		ver = SSH_DEBUG_EVENT;
	else if(strstr(argv[0], "packet"))
		ver = SSH_DEBUG_PACKET;
	else if(strstr(argv[0], "protocol"))
		ver = SSH_DEBUG_PROTO;
	else if(strstr(argv[0], "error"))
		ver = SSH_DEBUG_ERROR;

	if(ssh_debug_api(0, ver) == OK)
	{
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

DEFUN (show_ssh_keys,
		show_ssh_keys_cmd,
	    "show ip ssh keys",
		SHOW_STR
		IP_STR
		"SSH Information\n"
		"Keys Information\n")
{
	show_ssh_keymgt(vty);
	return CMD_SUCCESS;
}



int ssh_cmd_init(void)
{
/*	reinstall_node(CONFIG_NODE, ssh_write_config);
	install_default(CONFIG_NODE);
	install_default_basic(CONFIG_NODE);*/
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_service_enable_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_service_enable_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &sftp_service_enable_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_sftp_service_enable_cmd);
	//install_element (CONFIG_NODE, &ssh_bind_port_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_bind_address_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_bind_address_port_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_bind_port_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_login_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_login_set_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_version_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_version_set_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_authentication_type_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_authentication_type_set_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_authentication_retries_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_authentication_retries_set_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_key_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_key_set_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_key_generate_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_key_generate_cmd);

	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_key_import_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_key_export_cmd);


	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &ssh_debug_set_cmd);
	install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &no_ssh_debug_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &ssh_debug_set_cmd);
	install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ssh_debug_set_cmd);

	install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_ssh_keys_cmd);
	install_element (CONFIG_NODE, CMD_VIEW_LEVEL, &show_ssh_keys_cmd);

	return OK;
}

#endif

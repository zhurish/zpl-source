/*
 * cmd_systools.c
 *
 *  Created on: Oct 25, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"

#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vty.h"
#include "os_util.h"
#include "xyz_modem.h"
#include "systools.h"
#include "ftpdLib.h"
#include "ftpLib.h"
#include "tftpdLib.h"
#include "tftpLib.h"
#include "pingLib.h"
#include "telnetLib.h"
#include "tracerouteLib.h"
#include "tty_com.h"
#ifdef PL_SSH_MODULE
#include "ssh_api.h"
#endif


DEFUN (tftp_copy_download,
		tftp_copy_download_cmd,
	    "copy url-string URL FILE",
		"Copy configure\n"
		"URL format txt\n"
		"Remove URL Argument ([tftp|ftp|scp|sftp]://[user[:password@]] ip [:port][:][/file])\n"
		"Local file name\n")
{
	int ret = CMD_SUCCESS;
	os_url_t spliurl;
	char *url = NULL;
	char *localfileName = argv[1];
	if(strstr(argv[0], ":"))
	{
		url = argv[0];
		localfileName = argv[1];
	}
	else
	{
		url = argv[1];
		localfileName = argv[0];
	}
	memset(&spliurl, 0, sizeof(os_url_t));

	if(os_url_split(url, &spliurl) == OK)
	{
		vty_ansync_enable(vty, TRUE);
		vty_out(vty, "%s", VTY_NEWLINE);
		if(strstr(spliurl.proto,"tftp"))
			ret = tftp_download(vty, spliurl.host, spliurl.port, spliurl.filename,
					NULL, NULL, localfileName);
		else if(strstr(spliurl.proto,"ftp"))
			ret = ftp_download(vty, spliurl.host, spliurl.port, spliurl.path, spliurl.filename,
					spliurl.user, spliurl.pass, localfileName);
#ifdef PL_SSH_MODULE
		else if(strstr(spliurl.proto,"scp"))
			ret = ssh_scp_download(vty, TRUE, url, localfileName);
			//ret = do_ssh_copy(vty, TRUE, url, localfileName);
		else if(strstr(spliurl.proto,"sftp"))
			ret = sftp_action(vty, TRUE, url, localfileName);
#endif
		else
		{
			vty_out(vty, " URL protocol '%s' is not support%s", spliurl.proto, VTY_NEWLINE);
			vty_ansync_enable(vty, FALSE);
			os_url_free(&spliurl);
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "URL format is warning.%s", VTY_NEWLINE);
		vty_ansync_enable(vty, FALSE);
		os_url_free(&spliurl);
		return CMD_WARNING;
	}
	vty_ansync_enable(vty, FALSE);
	os_url_free(&spliurl);
	if(ret != CMD_SUCCESS)
		return CMD_WARNING;
	return CMD_SUCCESS;
}

DEFUN (tftp_copy_upload,
		tftp_copy_upload_cmd,
	    "copy FILE url-string URL",
		"Copy configure\n"
		"Local file name\n"
		"URL format txt\n"
		"Remove URL Argument ([tftp|ftp|scp|sftp]://[user[:password@]] ip [:port][:][/file])\n")
{
	int ret = CMD_SUCCESS;
	os_url_t spliurl;
	char *url = NULL;
	char *localfileName = argv[1];
	if(strstr(argv[0], ":"))
	{
		url = argv[0];
		localfileName = argv[1];
	}
	else
	{
		url = argv[1];
		localfileName = argv[0];
	}
	memset(&spliurl, 0, sizeof(os_url_t));

	if(os_url_split(url, &spliurl) == OK)
	{
		vty_ansync_enable(vty, TRUE);
		vty_out(vty, "%s", VTY_NEWLINE);
		if(strstr(spliurl.proto,"tftp"))
			ret = tftp_upload(vty, spliurl.host, spliurl.port, spliurl.filename,
					NULL, NULL, localfileName);
		else if(strstr(spliurl.proto,"ftp"))
			ret = ftp_upload(vty, spliurl.host, spliurl.port, spliurl.path, spliurl.filename,
					spliurl.user, spliurl.pass, localfileName);
#ifdef PL_SSH_MODULE
		else if(strstr(spliurl.proto,"scp"))
			ret = ssh_scp_upload(vty, FALSE, url, localfileName);
			//ret = do_ssh_copy(vty, FALSE, url, localfileName);
		else if(strstr(spliurl.proto,"sftp"))
			ret = sftp_action(vty, FALSE, url, localfileName);
#endif
		else
		{
			vty_out(vty, " URL protocol '%s' is not support%s", spliurl.proto, VTY_NEWLINE);
			vty_ansync_enable(vty, FALSE);
			os_url_free(&spliurl);
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "URL format is warning.%s", VTY_NEWLINE);
		os_url_free(&spliurl);
		return CMD_WARNING;
	}
	vty_ansync_enable(vty, FALSE);
	os_url_free(&spliurl);
	if(ret != CMD_SUCCESS)
		return CMD_WARNING;
	return CMD_SUCCESS;
}


DEFUN (tftp_server_enable,
		tftp_server_enable_cmd,
	    "tftp server enable",
		"TFTP configure\n"
		"Server configure\n"
		"enable configure\n")
{
	if(argc == 1 && argv[0])
	{
		if(tftpdEnable(TRUE, argv[0], 0) == OK)
			return CMD_SUCCESS;
	}
	else
	{
		if(tftpdEnable(TRUE, NULL, 0) == OK)
			return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

ALIAS (tftp_server_enable,
		tftp_server_address_cmd,
	    "tftp server bind "CMD_KEY_IPV4,
		"TFTP configure\n"
		"Server configure\n"
		"bind configure\n"
		CMD_KEY_IPV4_HELP)


DEFUN (no_tftp_server_enable,
		no_tftp_server_enable_cmd,
	    "no tftp server enable",
		NO_STR
		"TFTP configure\n"
		"Server configure\n"
		"enable configure\n")
{
	if(tftpdEnable(FALSE, NULL, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ftp_server_enable,
		ftp_server_enable_cmd,
	    "ftp server enable",
		"FTP configure\n"
		"Server configure\n"
		"enable configure\n")
{
	if(argc == 1 && argv[0])
	{
		if(strstr(argv[0], "."))
		{
			if(ftpdEnable(argv[0], 0) == OK)
				return CMD_SUCCESS;
		}
		else
		{
			if(ftpdEnable(NULL, atoi(argv[0])) == OK)
				return CMD_SUCCESS;
		}
	}
	else
	{
		if(ftpdEnable(NULL, 0) == OK)
			return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

ALIAS (ftp_server_enable,
		ftp_server_address_cmd,
	    "ftp server bind "CMD_KEY_IPV4,
		"FTP configure\n"
		"Server configure\n"
		"bind configure\n"
		CMD_KEY_IPV4_HELP)

ALIAS (ftp_server_enable,
		ftp_server_port_cmd,
	    "ftp server port <1-65536>",
		"FTP configure\n"
		"Server configure\n"
		"port configure\n"
		"port value(default 21)\n")


DEFUN (no_ftp_server_enable,
		no_ftp_server_enable_cmd,
	    "no ftp server enable",
		NO_STR
		"FTP configure\n"
		"Server configure\n"
		"enable configure\n")
{
	if(ftpdDisable() == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}



DEFUN (telnet_client,
		telnet_client_cmd,
	    "telnet "CMD_KEY_IPV4,
		"telnet client\n"
		CMD_KEY_IPV4_HELP)
{
	if(argc == 1)
	{
		if(telnet(vty, argv[0], 0) == OK)
			return CMD_SUCCESS;
	}
	else if(argc == 2)
	{
		if(telnet(vty, argv[0], atoi(argv[1])) == OK)
			return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

ALIAS (telnet_client,
		telnet_client_port_cmd,
	    "telnet "CMD_KEY_IPV4 " <1-65535>",
		"telnet client\n"
		CMD_KEY_IPV4_HELP
		"telnet server port\n");

#ifdef PL_SSH_MODULE
DEFUN (ssh_client_opt,
		ssh_client_opt_cmd,
	    "ssh URL",
		"SSH configure\n"
		"Remove URL Argument (ssh://user[:password@] ip [:port])\n")
{
	int ret = CMD_SUCCESS;
	os_url_t spliurl;
	char *url = NULL;
	url = argv[0];

	memset(&spliurl, 0, sizeof(os_url_t));

	if(os_url_split(url, &spliurl) == OK)
	{
		if(strstr(spliurl.proto,"ssh"))
			ret = ssh_client(vty, spliurl.host, spliurl.port, spliurl.user, spliurl.pass);
		else
		{
			vty_out(vty, " URL protocol '%s' is not support%s", spliurl.proto, VTY_NEWLINE);
			os_url_free(&spliurl);
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty, "URL format is warning.%s", VTY_NEWLINE);
		os_url_free(&spliurl);
		return CMD_WARNING;
	}
	os_url_free(&spliurl);
	return CMD_SUCCESS;
}
#endif



DEFUN (ping_start,
		ping_start_cmd,
	    "ping ("CMD_KEY_IPV4"|HOSTNAME)",
		"PING configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n")
{
	int count = 3;
	if(argc == 2)
		count = atoi(argv[1]);
	if(ping(vty, argv[0], count, 0, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

ALIAS (ping_start,
		ping_start_cnt_cmd,
	    "ping ("CMD_KEY_IPV4"|HOSTNAME) count <1-65536>",
		"PING configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"count packet configure\n"
		"count value\n");

DEFUN (ping_len_start,
		ping_start_len_cmd,
	    "ping ("CMD_KEY_IPV4"|HOSTNAME) size <1-65536>",
		"PING configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"size packet configure\n"
		"size value\n")
{
	int count = 3, size = 0;
	if(argc == 2)
		size = atoi(argv[1]);
	if(argc == 3)
		count = atoi(argv[2]);
	if(ping(vty, argv[0], count, size, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

ALIAS (ping_len_start,
		ping_start_len_cnt_cmd,
	    "ping ("CMD_KEY_IPV4"|HOSTNAME) size <1-65536> count <1-65536>",
		"PING configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"size packet configure\n"
		"size value\n"
		"count packet configure\n"
		"count value\n")


DEFUN (ping_len_cnt_start,
		ping_start_cnt_len_cmd,
	    "ping ("CMD_KEY_IPV4"|HOSTNAME) count <1-65536> size <1-65536>",
		"PING configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"size packet configure\n"
		"size value\n")
{
	int count = 3, size = 0;
	if(argc == 3)
		size = atoi(argv[1]);
	if(argc == 2)
		count = atoi(argv[2]);
	if(ping(vty, argv[0], count, size, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}



DEFUN (traceroute_start,
		traceroute_start_cmd,
	    "traceroute ("CMD_KEY_IPV4"|HOSTNAME)",
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n")
{
	int ttl = 0;
	if(argc == 2)
		ttl = atoi(argv[1]);
	if(traceroute(vty, argv[0], ttl, 0, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

ALIAS (traceroute_start,
		traceroute_start_ttl_cmd,
	    "traceroute ("CMD_KEY_IPV4"|HOSTNAME) ttl <1-250>",
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"TTL configure\n"
		"TTL value\n")


DEFUN (traceroute_len_start,
		traceroute_start_len_cmd,
	    "traceroute ("CMD_KEY_IPV4"|HOSTNAME) size <1-65536>",
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"size packet configure\n"
		"size value\n")
{
	int size = 0, ttl = 0;
	if(argc == 2)
		size = atoi(argv[1]);
	if(argc == 3)
		ttl = atoi(argv[2]);
	if(traceroute(vty, argv[0], ttl, size, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

ALIAS (traceroute_len_start,
		traceroute_start_len_ttl_cmd,
	    "traceroute ("CMD_KEY_IPV4"|HOSTNAME) size <1-65536> ttl <1-250>",
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"size packet configure\n"
		"size value\n"
		"TTL configure\n"
		"TTL value\n");

DEFUN (traceroute_ttl_len_start,
		traceroute_start_ttl_len_cmd,
	    "traceroute ("CMD_KEY_IPV4"|HOSTNAME) ttl <1-250> size <1-65536>",
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP
		"Host name\n"
		"TTL configure\n"
		"TTL value\n"
		"size packet\n"
		"size value\n")
{
	int size = 0, ttl = 0;
	if(argc == 3)
		size = atoi(argv[3]);
	if(argc == 2)
		ttl = atoi(argv[1]);
	if(traceroute(vty, argv[0], ttl, size, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

DEFUN_HIDDEN (load_image_xyz_modem,
		load_image_xyz_modem_cmd,
	    "load image (xmodem|ymodem)",
		"Load configure\n"
		"Image configure\n"
		"Xmodem mode\n"
		"Ymodem mode\n")
{
	int ret = 0, mode = 2;
	xyz_modem_t xyz;
	memset(&xyz, 0, sizeof(xyz));
	if(strstr(argv[0], "xmodem"))
		mode = 1;
	else if(strstr(argv[0], "ymodem"))
		mode = 2;

	if(!vty_is_console(vty) && argc == 1)
	{
		vty_out(vty, "##This command can only be used on a serial port%s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	xyz.sequm = 0;
	xyz.vty = vty;
	if(argc == 1)
		xyz.fd = vty->fd;
	else
	{
		xyz.show_debug = vty_sync_out;
	}
	ret = xyz_modem_load(&xyz, mode, (argc == 2) ? argv[1]:NULL);
	if(ret > 0)
	{
		vty_out(vty, "## Total Size %d Bytes%s", ret, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

ALIAS_HIDDEN (load_image_xyz_modem,
	load_image_xyz_modem_dev_cmd,
	"load image (xmodem|ymodem) DEVNAME",
	"Load configure\n"
	"Image configure\n"
	"Xmodem mode\n"
	"Ymodem mode\n"
	"tty Device name\n");




int systools_cmd_init()
{
	install_element (ENABLE_NODE, &tftp_copy_download_cmd);
	install_element (ENABLE_NODE, &tftp_copy_upload_cmd);

	install_element (CONFIG_NODE, &tftp_server_enable_cmd);
	install_element (CONFIG_NODE, &tftp_server_address_cmd);
	install_element (CONFIG_NODE, &no_tftp_server_enable_cmd);

	install_element (CONFIG_NODE, &ftp_server_enable_cmd);
	install_element (CONFIG_NODE, &ftp_server_address_cmd);
	install_element (CONFIG_NODE, &ftp_server_port_cmd);
	install_element (CONFIG_NODE, &no_ftp_server_enable_cmd);


	install_element (ENABLE_NODE, &telnet_client_cmd);
	install_element (ENABLE_NODE, &telnet_client_port_cmd);
#ifdef PL_SSH_MODULE
	install_element (ENABLE_NODE, &ssh_client_opt_cmd);
#endif

	install_element (ENABLE_NODE, &ping_start_cmd);
	install_element (ENABLE_NODE, &ping_start_cnt_cmd);
	install_element (ENABLE_NODE, &ping_start_len_cmd);
	install_element (ENABLE_NODE, &ping_start_len_cnt_cmd);
	install_element (ENABLE_NODE, &ping_start_cnt_len_cmd);


	install_element (ENABLE_NODE, &traceroute_start_cmd);
	install_element (ENABLE_NODE, &traceroute_start_ttl_cmd);
	install_element (ENABLE_NODE, &traceroute_start_len_cmd);
	install_element (ENABLE_NODE, &traceroute_start_len_ttl_cmd);
	install_element (ENABLE_NODE, &traceroute_start_ttl_len_cmd);

	install_element (ENABLE_NODE, &load_image_xyz_modem_cmd);
	install_element (ENABLE_NODE, &load_image_xyz_modem_dev_cmd);
	return 0;
}

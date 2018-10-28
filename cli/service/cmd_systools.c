/*
 * cmd_tftp.c
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

#include "systools.h"
#include "ftpdLib.h"
#include "ftpLib.h"
#include "tftpdLib.h"
#include "tftpLib.h"
#include "pingLib.h"
#include "telnetLib.h"
#include "tracerouteLib.h"


DEFUN (tftp_copy_download,
		tftp_copy_download_cmd,
	    "copy url-string URL FILE",
		"Copy configure\n"
		"URL format txt\n"
		"Remove URL Argument (tftp://1.1.1.1:80/file)\n"
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
			ret = ftp_download(vty, spliurl.host, spliurl.port, NULL, spliurl.filename,
					spliurl.user, spliurl.pass, localfileName);
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

DEFUN (tftp_copy_upload,
		tftp_copy_upload_cmd,
	    "copy FILE url-string URL",
		"Copy configure\n"
		"Local file name\n"
		"URL format txt\n"
		"Remove URL Argument (tftp://1.1.1.1:80/file)\n")
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
			ret = ftp_upload(vty, spliurl.host, spliurl.port, NULL, spliurl.filename,
					spliurl.user, spliurl.pass, localfileName);
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
	if(tftpdEnable(TRUE, NULL, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}

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

DEFUN (telnet_client,
		telnet_client_cmd,
	    "telnet "CMD_KEY_IPV4,
		"telnet client\n"
		CMD_KEY_IPV4_HELP)
{
	if(telnet(vty, argv[0], 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (ping_tools,
		ping_tools_cmd,
	    "ping "CMD_KEY_IPV4,
		"PING configure\n"
		CMD_KEY_IPV4_HELP)
{
	if(ping(vty, argv[0], 3, 0, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}


DEFUN (traceroute_start,
		traceroute_start_cmd,
	    "traceroute "CMD_KEY_IPV4,
		"traceroute configure\n"
		CMD_KEY_IPV4_HELP)
{
	if(traceroute(vty, argv[0], 3, 0, 0) == OK)
		return CMD_SUCCESS;
	return CMD_WARNING;
}



int systools_cmd_init()
{
	install_element (ENABLE_NODE, &tftp_copy_download_cmd);
	install_element (ENABLE_NODE, &tftp_copy_upload_cmd);
	install_element (ENABLE_NODE, &tftp_server_enable_cmd);
	install_element (ENABLE_NODE, &no_tftp_server_enable_cmd);


	install_element (ENABLE_NODE, &telnet_client_cmd);

	install_element (ENABLE_NODE, &ping_tools_cmd);
	install_element (ENABLE_NODE, &traceroute_start_cmd);
	return 0;
}

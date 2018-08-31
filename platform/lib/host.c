/*
 * host.c
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
#include "host.h"
#include "vty_user.h"

struct host host;

int host_config_init(char *motd)
{
	os_memset(&host, 0, sizeof(host));
	/* Default host value settings. */
	host.name = NULL;
	host.logfile = NULL;
	//XFREE (MTYPE_HOST
	host.config = XSTRDUP(MTYPE_HOST, STARTUP_CONFIG_FILE);
	host.default_config = XSTRDUP(MTYPE_HOST, DEFAULT_CONFIG_FILE);
	host.factory_config = XSTRDUP(MTYPE_HOST, FACTORY_CONFIG_FILE);
	host.lines = -1;
	host.motd = motd; //default_motd;
	host.motdfile = NULL;
	host.vty_timeout_val = VTY_TIMEOUT_DEFAULT;
	/* Vty access-class command */
	host.vty_accesslist_name = NULL;
	/* Vty access-calss for IPv6. */
	host.vty_ipv6_accesslist_name = NULL;
	/* VTY server thread. */
	//host.Vvty_serv_thread;
	/* Current directory. */
	host.vty_cwd = NULL;
	/* Configure lock. */
	host.vty_config = 0;
	/* Login password check. */
	host.no_password_check = 0;
	host.mutx = os_mutex_init();
	host.cli_mutx = os_mutex_init();
	return OK;
}

int host_config_exit(void)
{
	if (host.name)
		XFREE(MTYPE_HOST, host.name);

	if (host.vty_accesslist_name)
		XFREE(MTYPE_VTY, host.vty_accesslist_name);
	if (host.vty_ipv6_accesslist_name)
		XFREE(MTYPE_VTY, host.vty_ipv6_accesslist_name);

	if (host.logfile)
		XFREE(MTYPE_HOST, host.logfile);
	if (host.motdfile)
		XFREE(MTYPE_HOST, host.motdfile);
	if (host.config)
		XFREE(MTYPE_HOST, host.config);
	if (host.default_config)
		XFREE(MTYPE_HOST, host.default_config);
	if (host.factory_config)
		XFREE(MTYPE_HOST, host.factory_config);
	if (host.cli_mutx)
		os_mutex_exit(host.cli_mutx);
	if (host.mutx)
		os_mutex_exit(host.mutx);
	return OK;
}



/* Set config filename.  Called from vty.c */
void
host_config_set (char *filename)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if (host.config)
		XFREE(MTYPE_HOST, host.config);
	host.config = XSTRDUP(MTYPE_HOST, filename);
	if (host.mutx)
		os_mutex_unlock(host.mutx);
}

const char *
host_config_get (void)
{
	return host.config;
}



int
host_config_set_api (int cmd, void *pVoid)
{
	int ret = ERROR;
	char *strValue = (char *)pVoid;
	int *intValue = (int *)pVoid;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_SET_HOSTNAME_CMD:
		if (host.name)
			XFREE(MTYPE_HOST, host.name);
		if(strValue)
			host.name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.name = NULL;
		ret = OK;
		break;
	case API_SET_DESC_CMD:
		if (host.description)
			XFREE(MTYPE_HOST, host.description);
		if(strValue)
			host.description = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.description = NULL;
		ret = OK;
		break;
	case API_SET_LINES_CMD:
		host.lines = *intValue;
		ret = OK;
		break;
	case API_SET_LOGFILE_CMD:
		if (host.logfile)
			XFREE(MTYPE_HOST, host.logfile);
		if(strValue)
			host.logfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.logfile = NULL;
		ret = OK;
		break;
	case API_SET_CONFIGFILE_CMD:
		if (host.config)
			XFREE(MTYPE_HOST, host.config);
		if(strValue)
			host.config = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.config = NULL;
		ret = OK;
		break;
	case API_SET_ENCRYPT_CMD:
		host.encrypt = *intValue;
		ret = OK;
		break;
	case API_SET_MOTD_CMD:
		if (host.motd)
			XFREE(MTYPE_HOST, host.motd);
		if(strValue)
			host.motd = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.motd = NULL;
		ret = OK;
		break;
	case API_SET_MOTDFILE_CMD:
		if (host.motdfile)
			XFREE(MTYPE_HOST, host.motdfile);
		if(strValue)
			host.motdfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.motdfile = NULL;
		ret = OK;
		break;
	case API_SET_VTY_TIMEOUT_CMD:
		host.vty_timeout_val = *intValue;
		ret = OK;
		break;
	case API_SET_ACCESS_CMD:
		if (host.vty_accesslist_name)
			XFREE(MTYPE_HOST, host.vty_accesslist_name);
		if(strValue)
			host.vty_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.vty_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_IPV6ACCESS_CMD:
		if (host.vty_ipv6_accesslist_name)
			XFREE(MTYPE_HOST, host.vty_ipv6_accesslist_name);
		if(strValue)
			host.vty_ipv6_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.vty_ipv6_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_NOPASSCHK_CMD:
		host.no_password_check = *intValue;
		ret = OK;
		break;
	default:
		break;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return ret;
}

int
host_config_get_api (int cmd, void *pVoid)
{
	int ret = ERROR;
	char *strValue = (char *)pVoid;
	int *intValue = (int *)pVoid;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_GET_HOSTNAME_CMD:
		if (host.name)
		{
			if(strValue)
				os_strcpy(strValue, host.name);
		}
		ret = OK;
		break;
	case API_GET_DESC_CMD:
		if (host.description)
		{
			if(strValue)
				os_strcpy(strValue, host.description);
		}
		ret = OK;
		break;
	case API_GET_LINES_CMD:
		if(intValue)
			*intValue = host.lines;
		ret = OK;
		break;
	case API_GET_LOGFILE_CMD:
		if (host.logfile)
		{
			if(strValue)
				os_strcpy(strValue, host.logfile);
		}
		ret = OK;
		break;
	case API_GET_CONFIGFILE_CMD:
		if (host.config)
		{
			if(strValue)
				os_strcpy(strValue, host.config);
		}
		ret = OK;
		break;
	case API_GET_ENCRYPT_CMD:
		if(intValue)
			*intValue = host.encrypt;
		ret = OK;
		break;
	case API_GET_MOTD_CMD:
		if (host.motd)
		{
			if(strValue)
				os_strcpy(strValue, host.motd);
		}
		ret = OK;
		break;
	case API_GET_MOTDFILE_CMD:
		if (host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_VTY_TIMEOUT_CMD:
		if(intValue)
			*intValue = host.vty_timeout_val;
		ret = OK;
		break;
	case API_GET_ACCESS_CMD:
		if (host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_IPV6ACCESS_CMD:
		if (host.vty_ipv6_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_ipv6_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_NOPASSCHK_CMD:
		if(intValue)
			*intValue = host.no_password_check;
		ret = OK;
		break;
	default:
		break;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return ret;
}

/*
 * ssh_connect.c
 *
 *  Created on: Oct 31, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "getopt.h"
#include <log.h>
#include "command.h"
#include "memory.h"
#include "prefix.h"
#include "network.h"
#include "vty.h"
#include "buffer.h"
#include "host.h"
#include "eloop.h"

#include "ssh_api.h"
#include "ssh_util.h"




ssh_session ssh_connect_api(struct vty *vty, const char *host, int port,
		const char *user, char *password)
{
	ssh_session session;
	int auth = 0;
	int verbosity=7;
	session = ssh_new();
	if (session == NULL)
	{
		return NULL;
	}
	if (user != NULL)
	{
		if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}
	if (port != 0)
	{
		int cport = port;
		if (ssh_options_set(session, SSH_OPTIONS_PORT, &cport) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}

	if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0)
	{
		ssh_free(session);
		return NULL;
	}

	ssh_set_session_private(session, vty);

	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	if (ssh_connect(session))
	{
		//fprintf(ssh_stderr, "Connection failed : %s\n", ssh_get_error(session));
		ssh_printf(session, "Connection failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	if (ssh_verify_knownhost(vty->fd, session) < 0)
	{
		ssh_printf(session, "verify_knownhost failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	auth = ssh_authenticate_api(session, password);
	if (auth == SSH_AUTH_SUCCESS)
	{
		  char *banner = ssh_get_issue_banner(session);
		  if(banner){
			  ssh_printf(session, "%s\n",banner);
		      free(banner);
		  }
		return session;
	}
	else if (auth == SSH_AUTH_DENIED)
	{
		ssh_printf(session, "Authentication failed\n");
	}
	else
	{
		ssh_printf(session, "Error while authenticating : %s\n",
				ssh_get_error(session));
	}
	ssh_disconnect(session);
	ssh_free(session);
	return NULL;
}

ssh_session ssh_client_connect_api(ssh_session session, struct vty *vty, const char *host, int port,
		const char *user, char *password)
{
	//ssh_session session;
	int auth = 0;
	int verbosity = 3;
	//session = ssh_new();
	if (session == NULL)
	{
		return NULL;
	}
	if (user != NULL)
	{
		if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}
	if (port != 0)
	{
		int cport = port;
		if (ssh_options_set(session, SSH_OPTIONS_PORT, &cport) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}

	if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0)
	{
		ssh_free(session);
		return NULL;
	}

	ssh_set_session_private(session, vty);

	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	if (ssh_connect(session))
	{
		//fprintf(ssh_stderr, "Connection failed : %s\n", ssh_get_error(session));
		ssh_printf(session, "Connection failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	if (ssh_verify_knownhost(vty->fd, session) < 0)
	{
		ssh_printf(session, "verify_knownhost failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	auth = ssh_authenticate_api(session, password);
	if (auth == SSH_AUTH_SUCCESS)
	{
		  char *banner = ssh_get_issue_banner(session);
		  if(banner){
			  ssh_printf(session, "%s\n",banner);
		      free(banner);
		  }
		return session;
	}
	else if (auth == SSH_AUTH_DENIED)
	{
		ssh_printf(session, "Authentication failed\n");
	}
	else
	{
		ssh_printf(session, "Error while authenticating : %s\n",
				ssh_get_error(session));
	}
	ssh_disconnect(session);
	ssh_free(session);
	return NULL;
}

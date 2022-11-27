/*
 * ssh_connect.c
 *
 *  Created on: Oct 31, 2018
 *      Author: zhurish
 */


#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#include "ssh_api.h"
#include "ssh_util.h"




ssh_session ssh_connect_api(struct vty *vty, const char *remotehost, zpl_uint16 port,
		const char *user, char *password)
{
	ssh_session session;
	zpl_uint32 auth = 0;
	zpl_uint32 verbosity=7;
	session = ssh_new();
	if (session == NULL)
	{
		return NULL;
	}
	session->ssh_cli = vty;
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
		zpl_uint16 cport = port;
		if (ssh_options_set(session, SSH_OPTIONS_PORT, &cport) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}

	if (ssh_options_set(session, SSH_OPTIONS_HOST, remotehost) < 0)
	{
		ssh_free(session);
		return NULL;
	}

	ssh_set_session_private(session, session);

	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	if (ssh_connect(session))
	{
		//fprintf(ssh_stderr, "Connection failed : %s\n", ssh_get_error(session));
		ssh_printf(session, "Connection failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	if (ssh_verify_knownhost(ipstack_fd(vty->fd), session) < 0)
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

ssh_session ssh_client_connect_api(ssh_session session, struct vty *vty, const char *remotehost, zpl_uint16 port,
		const char *user, char *password)
{
	zpl_uint32 auth = 0;
	zpl_uint32 verbosity = 3;
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
		zpl_uint16 cport = port;
		if (ssh_options_set(session, SSH_OPTIONS_PORT, &cport) < 0)
		{
			ssh_free(session);
			return NULL;
		}
	}

	if (ssh_options_set(session, SSH_OPTIONS_HOST, remotehost) < 0)
	{
		ssh_free(session);
		return NULL;
	}

	ssh_set_session_private(session, session);

	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	if (ssh_connect(session))
	{
		//fprintf(ssh_stderr, "Connection failed : %s\n", ssh_get_error(session));
		ssh_printf(session, "Connection failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	if (ssh_verify_knownhost(ipstack_fd(vty->fd), session) < 0)
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

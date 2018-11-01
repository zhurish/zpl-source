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

int verify_knownhost(ssh_session session)
{
	char *hexa;
	int state;
	char buf[10];
	unsigned char *hash = NULL;
	size_t hlen;
	ssh_key srv_pubkey;
	int rc;

	state = ssh_is_server_known(session);

	rc = ssh_get_publickey(session, &srv_pubkey);
	if (rc < 0)
	{
		return -1;
	}

	rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash,
			&hlen);
	ssh_key_free(srv_pubkey);
	if (rc < 0)
	{
		return -1;
	}

	switch (state)
	{
	case SSH_SERVER_KNOWN_OK:
		break; /* ok */
	case SSH_SERVER_KNOWN_CHANGED:
		ssh_printf(session,
				"Host key for server changed : server's one is now :\n");
		//ssh_print_hexa("Public key hash", hash, hlen);

		ssh_clean_pubkey_hash(&hash);
		ssh_printf(session, "For security reason, connection will be stopped\n");
		return -1;
	case SSH_SERVER_FOUND_OTHER:
		ssh_printf(session,
				"The host key for this server was not found but an other type of key exists.\n");
		ssh_printf(session,
				"An attacker might change the default server key to confuse your client"
						"into thinking the key does not exist\n"
						"We advise you to rerun the client with -d or -r for more safety.\n");
		return -1;
	case SSH_SERVER_FILE_NOT_FOUND:
		ssh_printf(session,
				"Could not find known host file. If you accept the host key here,\n");
		ssh_printf(session, "the file will be automatically created.\n");
		/* fallback to SSH_SERVER_NOT_KNOWN behavior */
	case SSH_SERVER_NOT_KNOWN:
		hexa = ssh_get_hexa(hash, hlen);
		ssh_printf(session, "The server is unknown. Do you trust the host key ?\n");
		ssh_printf(session, "Public key hash: %s\n", hexa);
		ssh_string_free_char(hexa);
		ssh_printf(session, "waiting input [yes/no] ?\n");
		if (fgets(buf, sizeof(buf), ssh_stdin_get(session)) == NULL)
		{
			ssh_clean_pubkey_hash(&hash);
			return -1;
		}
		if (strncasecmp(buf, "yes", 3) != 0)
		{
			ssh_clean_pubkey_hash(&hash);
			return -1;
		}
		ssh_printf(session,
				"This new key will be written on disk for further usage. do you agree ?\n");
		ssh_printf(session, "waiting input [yes/no] ?\n");
		if (fgets(buf, sizeof(buf), ssh_stdin_get(session)) == NULL)
		{
			ssh_clean_pubkey_hash(&hash);
			return -1;
		}
		if (strncasecmp(buf, "yes", 3) == 0)
		{
			if (ssh_write_knownhost(session) < 0)
			{
				ssh_clean_pubkey_hash(&hash);
				ssh_printf(session, "error %s\n", strerror(errno));
				return -1;
			}
		}

		break;
	case SSH_SERVER_ERROR:
		ssh_clean_pubkey_hash(&hash);
		ssh_printf(session, "%s", ssh_get_error(session));
		return -1;
	}
	ssh_clean_pubkey_hash(&hash);
	return 0;
}



int ssh_authenticate_api (ssh_session session, char *password)
{
	int rc;
	int method;
	char *banner;

	// Try to authenticate
	rc = ssh_userauth_none(session, NULL);
	if (rc == SSH_AUTH_ERROR)
	{
		ssh_printf(session, "authenticate failed : %s\n", ssh_get_error(session));
		//error(session);
		return rc;
	}

	method = ssh_userauth_list(session, NULL);
	while (rc != SSH_AUTH_SUCCESS)
	{
		if (method & SSH_AUTH_METHOD_GSSAPI_MIC)
		{
			rc = ssh_userauth_gssapi(session);
			if (rc == SSH_AUTH_ERROR)
			{
				//error(session);
				ssh_printf(session, "authenticate failed : %s\n", ssh_get_error(session));
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}
		}
		// Try to authenticate with public key first
		if (method & SSH_AUTH_METHOD_PUBLICKEY)
		{
			rc = ssh_userauth_publickey_auto(session, NULL, NULL);
			if (rc == SSH_AUTH_ERROR)
			{
				//error(session);
				ssh_printf(session, "authenticate failed : %s\n", ssh_get_error(session));
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}
		}

		// Try to authenticate with keyboard interactive";
/*		if (method & SSH_AUTH_METHOD_INTERACTIVE)
		{
			rc = authenticate_kbdint(session, NULL);
			if (rc == SSH_AUTH_ERROR)
			{
				error(session);
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}
		}

		if (ssh_getpass("Password: ", password, sizeof(password), 0, 0) < 0)
		{
			return SSH_AUTH_ERROR;
		}*/
		// Try to authenticate with password
		if (method & SSH_AUTH_METHOD_PASSWORD)
		{
			rc = ssh_userauth_password(session, NULL, password);
			if (rc == SSH_AUTH_ERROR)
			{
				ssh_printf(session, "authenticate failed : %s\n", ssh_get_error(session));
				//ssh_get_error(session);
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}
		}
		memset(password, 0, sizeof(password));
	}

	banner = ssh_get_issue_banner(session);
	if (banner)
	{
		ssh_printf(session, "%s\n", banner);
		ssh_string_free_char(banner);
	}

	return rc;
}

ssh_session ssh_connect_api(struct vty *vty, const char *host, int port, const char *user, char *password)
{
	ssh_session session;
	int auth = 0;

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

	//ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	if (ssh_connect(session))
	{
		//fprintf(ssh_stderr, "Connection failed : %s\n", ssh_get_error(session));
		ssh_printf(session, "Connection failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	if (verify_knownhost(session) < 0)
	{
		ssh_printf(session, "verify_knownhost failed : %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		return NULL;
	}
	auth = ssh_authenticate_api(session, password);
	if (auth == SSH_AUTH_SUCCESS)
	{
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

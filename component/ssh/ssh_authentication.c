/*
 * ssh_authentication.c
 *
 *  Created on: Nov 3, 2018
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



int ssh_authenticate_kbdint(int fd, ssh_session session, const char *password) {
    int err;
    err = ssh_userauth_kbdint(session, NULL, NULL);
    while (err == SSH_AUTH_INFO) {
        const char *instruction;
        const char *name;
        char buffer[128];
        ospl_uint32 i, n;

        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        n = ssh_userauth_kbdint_getnprompts(session);

        if (name && strlen(name) > 0) {
        	ssh_printf(session,"%s\n", name);
        }

        if (instruction && strlen(instruction) > 0) {
        	ssh_printf(session,"%s\n", instruction);
        }

        for (i = 0; i < n; i++) {
            const char *answer;
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(session, i, &echo);
            if (prompt == NULL) {
                break;
            }

            if (echo) {
                char *p;

                ssh_printf(session,"%s", prompt);

                if (!ssh_get_input(fd, buffer, sizeof(buffer))) {
                    return SSH_AUTH_ERROR;
                }

                buffer[sizeof(buffer) - 1] = '\0';
                if ((p = strchr(buffer, '\n'))) {
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                    return SSH_AUTH_ERROR;
                }

                memset(buffer, 0, strlen(buffer));
            } else {
                if (password && strstr(prompt, "Password:")) {
                    answer = password;
                } else {
                    buffer[0] = '\0';

                    if (ssh_getpass(fd, prompt, buffer, (size_t)sizeof(buffer), 0, 0) < 0) {
                        return SSH_AUTH_ERROR;
                    }
                    answer = buffer;
                }
                err = ssh_userauth_kbdint_setanswer(session, i, answer);
                memset(buffer, 0, sizeof(buffer));
                if (err < 0) {
                    return SSH_AUTH_ERROR;
                }
            }
        }
        err=ssh_userauth_kbdint(session,NULL,NULL);
    }

    return err;
}

int ssh_verify_knownhost(int fd, ssh_session session)
{
	char *hexa;
	ospl_uint32 state;
	char buf[10];
	ospl_uint8 *hash = NULL;
	size_t hlen;
	ssh_key srv_pubkey;
	int rc;

	state = ssh_is_server_known(session);

	rc = ssh_get_publickey(session, &srv_pubkey);
	if (rc < 0)
	{
		return -1;
	}
	_ssh_debug_log(7,  __func__, "ssh_get_publickey -> ssh_get_publickey_hash\n");
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
		if (!ssh_get_input(fd, buf, sizeof(buf)))
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
		if (!ssh_get_input(fd, buf, sizeof(buf)))
		//if (fgets(buf, sizeof(buf), ssh_stdin_get(session)) == NULL)
		{
			ssh_clean_pubkey_hash(&hash);
			return -1;
		}
		if (strncasecmp(buf, "yes", 3) == 0)
		{
			_ssh_debug_log(7,  __func__, "ssh_write_knownhost\n");
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



int ssh_authenticate_api (ssh_session session, char *pass)
{
	int rc = SSH_AUTH_DENIED;
	ospl_uint32 method = 0;
	char *banner = NULL;
	char password[128];
	int fd = ssh_stdin_get(session);
	if(fd < 0)
	{
		ssh_printf(session, "ssh_stdin_get : can not find stdin FD\n");
		//error(session);
		return -1;
	}
	// Try to authenticate
	rc = ssh_userauth_none(session, NULL);
	if (rc == SSH_AUTH_ERROR)
	{
		ssh_printf(session, "authenticate failed : %s\n", ssh_get_error(session));
		//error(session);
		return rc;
	}
	memset(password, 0, sizeof(password));
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
		if (method & SSH_AUTH_METHOD_INTERACTIVE)
		{
			rc = ssh_authenticate_kbdint(fd, session, NULL);
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

		// Try to authenticate with host base";
		if (method & SSH_AUTH_METHOD_HOSTBASED)
		{
			rc = SSH_AUTH_SUCCESS;
			break;
/*			rc = ssh_authenticate_kbdint(fd, session, NULL);
			if (rc == SSH_AUTH_ERROR)
			{
				error(session);
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}*/
		}

		// Try to authenticate with none";
		if (method & SSH_AUTH_METHOD_NONE)
		{
			rc = SSH_AUTH_SUCCESS;
			break;
/*			rc = ssh_authenticate_kbdint(fd, session, NULL);
			if (rc == SSH_AUTH_ERROR)
			{
				error(session);
				return rc;
			}
			else if (rc == SSH_AUTH_SUCCESS)
			{
				break;
			}*/
		}

		// Try to authenticate with password
		if (method & SSH_AUTH_METHOD_PASSWORD)
		{
			memset(password, 0, sizeof(password));
			if (ssh_getpass(fd, "Password: ", password, sizeof(password), 0, 0) < 0)
			{
				return SSH_AUTH_ERROR;
			}
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
	}

	banner = ssh_get_issue_banner(session);
	if (banner)
	{
		ssh_printf(session, "%s\n", banner);
		ssh_string_free_char(banner);
	}

	return rc;
}

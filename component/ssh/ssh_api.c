/*
 * ssh_api.c
 *
 *  Created on: Nov 7, 2018
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
#include "os_task.h"

#include "libssh_config.h"

#include "libssh/priv.h"
#include "libssh/libssh.h"
#include "libssh/poll.h"
#include "libssh/socket.h"
#include "libssh/session.h"
#include "libssh/misc.h"
#include "libssh/pki.h"
#ifdef WITH_SERVER
#include "libssh/server.h"
#endif


#include "ssh_api.h"
#include "ssh_util.h"
#include "sshd_main.h"


static ssh_config_t ssh_config;


static int ssh_ctl_thread(socket_t fd, int revents, void *userdata);


int ssh_module_init()
{
	int socket[2] = { 0, 0 };
	memset(&ssh_config, 0, sizeof(ssh_config));
    ssh_init();
    ssh_set_log_level(7);
    ssh_set_log_callback(ssh_log_callback_func);
    if(socketpair (AF_UNIX, SOCK_STREAM, 0, socket) == 0)
    {
    	ssh_config.sock = socket[1];
    	ssh_config.ctlfd = socket[0];
    }
    else
    {
		return SSH_ERROR;
    }
    ssh_config.sshbind = ssh_bind_new();
    ssh_config.event = ssh_event_new();
    ssh_config.initialized = TRUE;
    ssh_config.auth_type = SSH_AUTH_AUTO;
    ssh_config.auth_retries = 3;
    ssh_config.auth_waitting = 5;
    ssh_config.ssh_version = 3;
    //ssh_config.shell_enable = TRUE;
    //sshd_set_default_keys(ssh_config.sshbind, 0, 0, 0);
    sshd_set_keys(&ssh_config, ssh_config.sshbind);


    ssh_socket_set_nonblocking(socket[0]);
    ssh_socket_set_nonblocking(socket[1]);
    ssh_event_add_fd(ssh_config.event, ssh_config.sock, POLLIN, ssh_ctl_thread, &ssh_config);
    return OK;
}

int ssh_module_exit()
{
    ssh_event_free(ssh_config.event);
    ssh_bind_free(ssh_config.sshbind);
    ssh_finalize();
    ssh_config.initialized = FALSE;
    return OK;
}


int ssh_module_task_init ()
{
	if(ssh_config.sshd_taskid == 0)
		ssh_config.sshd_taskid = os_task_create("sshdTask", OS_TASK_DEFAULT_PRIORITY,
	               0, sshd_task, &ssh_config, OS_TASK_DEFAULT_STACK);
	if(ssh_config.sshd_taskid)
		return OK;
	return ERROR;
}

int ssh_module_task_exit ()
{
	ssh_config.quit = TRUE;
/*	if(ssh_config.sshd_taskid)
		os_task_destroy(ssh_config.sshd_taskid);*/
	ssh_config.sshd_taskid = 0;
	return OK;
}


/*

int sshd_enable(char *address, int port)
{
	int local_port = port;
    if(!ssh_config.initialized)
    	sshd_module_init();

    if(port)
    	ssh_bind_options_set(ssh_config.sshbind, SSH_BIND_OPTIONS_BINDPORT, &local_port);

    if(address)
    	ssh_bind_options_set(ssh_config.sshbind, SSH_BIND_OPTIONS_BINDADDR, address);

    sshd_set_default_keys(ssh_config.sshbind, 0, 1, 1);

    if(ssh_bind_listen(ssh_config.sshbind) < 0) {
    	ssh_printf(NULL, "%s\n", ssh_get_error(ssh_config.sshbind));
        return 1;
    }
    ssh_socket_set_nonblocking(ssh_bind_get_fd(ssh_config.sshbind));
    ssh_event_add_fd(ssh_config.event,
		  ssh_bind_get_fd(ssh_config.sshbind), POLLIN, sshd_accept, &ssh_config);

    sshd_module_task_init ();

    return OK;
}
*/



static int sshd_stop(ssh_config_t *sshconfig)
{
	//ssh_config.running = FALSE;
	//close all channel
	//close all session
	zlog_debug(ZLOG_UTILS, "%ss", __func__);
	ssh_event_remove_fd(sshconfig->event,
			  ssh_bind_get_fd(sshconfig->sshbind));

	ssh_session_clean(sshconfig->event);

    //ssh_event_free(sshconfig->event);

    ssh_bind_close(sshconfig->sshbind);

    //sshconfig->event = NULL;
    //sshconfig->sshbind = NULL;

	//sshconfig->event = ssh_event_new();
    //ssh_event_add_fd(sshconfig->event, sshconfig->sock, POLLIN, ssh_ctl_thread, sshconfig);
/*
	close(ssh_bind_get_fd(sshconfig->sshbind));
	sshconfig->sshbind->bindfd = SSH_INVALID_SOCKET;
*/
    sshconfig->running = FALSE;
	return OK;
}

static int sshd_start(ssh_config_t *sshconfig)
{
	if(sshconfig->sshbind == NULL)
		sshconfig->sshbind = ssh_bind_new();
	if(sshconfig->event == NULL)
		sshconfig->event = ssh_event_new();

	zlog_debug(ZLOG_UTILS, "%ss", __func__);

    if(sshconfig->bindport)
    	ssh_bind_options_set(sshconfig->sshbind, SSH_BIND_OPTIONS_BINDPORT, &sshconfig->bindport);

    if(sshconfig->bind_prefix.u.prefix4.s_addr)
    {
    	ssh_bind_options_set(sshconfig->sshbind, SSH_BIND_OPTIONS_BINDADDR, inet_ntoa(sshconfig->bind_prefix.u.prefix4));
    }

    //ssh_bind_set_fd
    //sshd_set_default_keys(sshconfig->sshbind, 0, 1, 1);
    sshconfig->shell_enable = TRUE;

    if(ssh_bind_listen(sshconfig->sshbind) < 0) {
    	ssh_printf(NULL, "%s\n", ssh_get_error(sshconfig->sshbind));
        return ERROR;
    }
    ssh_socket_set_nonblocking(ssh_bind_get_fd(sshconfig->sshbind));

    ssh_event_add_fd(sshconfig->event,
			  ssh_bind_get_fd(sshconfig->sshbind), POLLIN, sshd_accept, &ssh_config);

    sshconfig->running = TRUE;
    return OK;
}



static int ssh_ctl_thread(socket_t fd, int revents, void *userdata)
{
    char buf[64];
    int n = -1;//, ret = 0;
    int	*cmd = (int*)buf;
    os_bzero(buf, sizeof(buf));
    ssh_config_t *ssh = (ssh_config_t *)userdata;
    if ((revents & POLLIN) != 0)
    {
        n = read(fd, buf, 4);
        if (n > 0)
        {
        	switch(*cmd)
        	{
        	case SSH_NONE_CMD:
        		break;
        	case SSH_STOP_CMD:
        		sshd_stop(ssh);
        		break;
        	case SSH_START_CMD:
        		sshd_start(ssh);
        		break;
        	default:
        		break;
        	}
        }
    }
    return n;
}

static int ssh_ctl_cmd(int cmd)
{
    char buf[64];
    int	*ctlcmd = (int*)buf;
    os_bzero(buf, sizeof(buf));
    *ctlcmd = cmd;
    write(ssh_config.ctlfd, buf, 4);
    return OK;
}


int ssh_bind_address_api(struct prefix *address)
{
	if(memcmp(&ssh_config.bind_prefix, address, sizeof(struct prefix)) == 0)
		return OK;
	memcpy(&ssh_config.bind_prefix, address, sizeof(struct prefix));
	return OK;
}


int ssh_bind_port_api(int port)
{
	if(ssh_config.bindport == port)
		return OK;
	ssh_config.bindport = port;
	if(ssh_config.bindport == 0)
		ssh_config.bindport = SSH_PORT_DEFAULT;
	return OK;
}

int ssh_version_api(int version)
{
	if(ssh_config.ssh_version == version)
		return OK;
	ssh_config.ssh_version = version;
	return OK;
}

int ssh_login_api(BOOL enable)
{
	if(ssh_config.shell_enable == enable)
		return OK;
	ssh_config.shell_enable = enable;
	return OK;
}


int ssh_authentication_retries_api(int value)
{
	if(ssh_config.auth_retries == value)
		return OK;
	ssh_config.auth_retries = value;
	return OK;
}

int ssh_authentication_waitting_api(int value)
{
	if(ssh_config.auth_waitting == value)
		return OK;
	ssh_config.auth_waitting = value;
	return OK;
}

int ssh_authentication_type_api(int value)
{
	if(ssh_config.auth_type == value)
		return OK;
	ssh_config.auth_type = value;
	return OK;
}

int ssh_keyfile_api(int type, char * value)
{
	switch(type)
	{
	case SSH_BIND_OPTIONS_HOSTKEY:
		os_bzero(ssh_config.hostkey, sizeof(ssh_config.hostkey));
		if(value)
			os_strcpy(ssh_config.hostkey, value);
		break;
	case SSH_BIND_OPTIONS_RSAKEY:
		os_bzero(ssh_config.rsakey, sizeof(ssh_config.rsakey));
		if(value)
			os_strcpy(ssh_config.rsakey, value);
		break;
	case SSH_BIND_OPTIONS_DSAKEY:
		os_bzero(ssh_config.dsakey, sizeof(ssh_config.dsakey));
		if(value)
			os_strcpy(ssh_config.dsakey, value);
		break;
	case SSH_BIND_OPTIONS_ECDSAKEY:
		os_bzero(ssh_config.ecdsakey, sizeof(ssh_config.ecdsakey));
		if(value)
			os_strcpy(ssh_config.ecdsakey, value);
		break;
	default:
		break;
	}
	return OK;
}


int ssh_generate_key_api(struct vty *vty, int type, char * keyname)
{
	if(ssh_keymgt_lookup(&ssh_config, keyname))
	{
		vty_sync_out(vty, " ssh key %s is already exist.%s", keyname, VTY_NEWLINE);
		return ERROR;
	}
	return ssh_keymgt_add(&ssh_config, vty, type, keyname);
}

int ssh_key_delete_api(char * keyname)
{
	return ssh_keymgt_delete(&ssh_config, keyname);
}

int ssh_keymgt_export_api(char *keyname, int type, char *filename, char *password)
{
	return ssh_keymgt_export_set(&ssh_config, keyname,  type, filename, password);
}

int ssh_keymgt_import_api(char *keyname, int type, char *filename, char *password)
{
	return ssh_keymgt_import_set(&ssh_config, keyname,  type, filename, password);
}

int ssh_debug_api(int type, int debug)
{
	if(type)
	{
		ssh_config.ssh_debug |= debug;
		ssh_set_log_level(ssh_config.ssh_debug & 0XFF);
		ssh_set_log_module(ssh_config.ssh_debug & 0XFF00);
	}
	else
	{
		ssh_config.ssh_debug &= ~debug;
		ssh_set_log_level(ssh_config.ssh_debug & 0XFF);
		ssh_set_log_module(ssh_config.ssh_debug & 0XFF00);
	}
	return OK;
}


int ssh_enable_api(BOOL enable)
{
	if(ssh_config.enable == enable)
		return OK;
	ssh_config.enable = enable;
	if(enable)
	{
		ssh_ctl_cmd(SSH_START_CMD);
		os_msleep(10);
		//ssh_config.running = TRUE;
	}
	else
	{
		if(ssh_config.running)
		{
			ssh_ctl_cmd(SSH_STOP_CMD);
			os_msleep(10);
			//ssh_config.running = FALSE;
		}
	}
	return OK;
}

BOOL ssh_is_enable_api(void)
{
	return ssh_config.enable;
}

BOOL ssh_is_running_api(void)
{
	return ssh_config.running;
}


static int _ssh_keymgt_show(ssh_config_t *ssh, struct vty *vty)
{
	int i = 0, head = 0;
/*    char key_buf[4096];
    char *b64_key;*/
	ssh_key 	key = NULL;
	for (i = 0; i < SSH_KEY_MAX; i++)
	{
		if (ssh->ssh_key[i].used)
		{
/*		    rc = ssh_pki_export_pubkey_base64(ssh->ssh_key[i].key, &b64_key);
		    if (rc < 0) {
		        continue;
		    }*/
			if(head == 0)
			{
				vty_out(vty, " %-12s %-12s %s%s", "------------", "------------", "------------", VTY_NEWLINE);
				vty_out(vty, " %-12s %-12s %s%s", "    Name    ", "    Type    ", "   Modulus  ", VTY_NEWLINE);
				vty_out(vty, " %-12s %-12s %s%s", "------------", "------------", "------------", VTY_NEWLINE);
				head = 1;
			}
			key = ssh->ssh_key[i].key;
			switch (ssh->ssh_key[i].type)
			{
			case SSH_KEYTYPE_DSS:
				vty_out(vty, " %-12s %-12s %d%s",
						ssh->ssh_key[i].keyname, key->type_c ? key->type_c:"UNKNOWN",
						key->enca_size, VTY_NEWLINE);
				break;
			case SSH_KEYTYPE_RSA:
			case SSH_KEYTYPE_RSA1:
				vty_out(vty, " %-12s %-12s %d%s",
						ssh->ssh_key[i].keyname, key->type_c ? key->type_c:"UNKNOWN",
						key->enca_size, VTY_NEWLINE);
				break;
			case SSH_KEYTYPE_ECDSA:
				vty_out(vty, " %-12s %-12s %d%s",
						ssh->ssh_key[i].keyname, key->type_c ? key->type_c:"UNKNOWN",
						key->enca_size, VTY_NEWLINE);
				break;
			case SSH_KEYTYPE_ED25519:
				vty_out(vty, " %-12s %-12s %d%s",
						ssh->ssh_key[i].keyname, key->type_c ? key->type_c:"UNKNOWN",
						key->enca_size, VTY_NEWLINE);
				break;
			case SSH_KEYTYPE_UNKNOWN:
				break;
			}
		}
	}
	return OK;
}


static int _ssh_config_show(ssh_config_t *ssh, struct vty *vty)
{
	if(ssh->enable)
	{
		vty_out(vty, "ssh service enable%s", VTY_NEWLINE);

		if(ssh->ssh_version != 3)
			vty_out(vty, "ip ssh version %d%s", ssh->ssh_version, VTY_NEWLINE);

		if(ssh->bindport && ssh->bind_prefix.prefixlen)
			vty_out(vty, "ip ssh bind-address %s port %d%s",
							inet_ntoa(ssh->bind_prefix.u.prefix4), ssh->bindport, VTY_NEWLINE);
		else if(ssh->bindport && !ssh->bind_prefix.prefixlen)
			vty_out(vty, "ip ssh bind-port %d%s", ssh->bindport, VTY_NEWLINE);
		else if(!ssh->bindport && ssh->bind_prefix.prefixlen)
			vty_out(vty, "ip ssh bind-address %s%s",
							inet_ntoa(ssh->bind_prefix.u.prefix4), VTY_NEWLINE);

		if(ssh->shell_enable)
			vty_out(vty, "ip ssh login enable%s", VTY_NEWLINE);

		switch(ssh->auth_type)
		{
		case SSH_AUTH_NONE:
			vty_out(vty, "ip ssh authentication-type none%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_AUTO:
			break;
		case SSH_AUTH_PASSWORD:
			vty_out(vty, "ip ssh authentication-type password%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_PUBLIC_KEY:
			vty_out(vty, "ip ssh authentication-type public%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_RSA:
			vty_out(vty, "ip ssh authentication-type rsa%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_HOSTBASE:
			vty_out(vty, "ip ssh authentication-type hostbase%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_KB:
			vty_out(vty, "ip ssh authentication-type kb%s", VTY_NEWLINE);
			break;
		case SSH_AUTH_GSSAPI:
			vty_out(vty, "ip ssh authentication-type gssapi%s", VTY_NEWLINE);
			break;
		default:
			break;
		}
		if(strlen(ssh->hostkey))
		{
			vty_out(vty, "ip ssh hostkey %s%s", ssh->hostkey, VTY_NEWLINE);
		}
		if(strlen(ssh->rsakey))
		{
			vty_out(vty, "ip ssh rsakey %s%s", ssh->rsakey, VTY_NEWLINE);
		}
		if(strlen(ssh->dsakey))
		{
			vty_out(vty, "ip ssh dsakey %s%s", ssh->dsakey, VTY_NEWLINE);
		}
		if(strlen(ssh->ecdsakey))
		{
			vty_out(vty, "ip ssh ecdsakey %s%s", ssh->ecdsakey, VTY_NEWLINE);
		}
		if(ssh->auth_retries != 3)
			vty_out(vty, "ip ssh authentication-retries %d%s", ssh->bindport, VTY_NEWLINE);

		return 1;
	}
	return OK;
}


int show_ssh_keymgt(struct vty *vty)
{
	return _ssh_keymgt_show(&ssh_config, vty);
}

int ssh_write_config(struct vty *vty)
{
	return _ssh_config_show(&ssh_config, vty);
}


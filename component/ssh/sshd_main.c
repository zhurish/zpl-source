
#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

#include "ssh_api.h"
#include "sshd_main.h"
#include "ssh_util.h"


static int sshd_shell_output(socket_t fd, zpl_uint32 revents, void *userdata)
{
    char buf[4096];
    int n = -1, ret = 0;
    os_bzero(buf, sizeof(buf));
    sshd_client_t * client = (ssh_channel) userdata;
    //ssh_channel channel = (ssh_channel) userdata;
    if ((revents & POLLIN) != 0) {
        n = read(fd, buf, sizeof(buf));
        if (n > 0) {
        	if(client != NULL && client->channel)
        		ret = ssh_channel_write(client->channel, buf, n);
        	if(ret < 0)
        		return SSH_ERROR;
        	else if (ret == 0)
        		return SSH_ERROR;
        	return ret;
        }
        if(n <= 0)
        {
        	//ssh_event_remove_fd(fd);
        	if(client->config->event)
        		ssh_event_remove_fd(client->config->event, fd);
        	close(fd);
        	client->sock = 0;
        	return -1;
        }
    }
    return n;
}

static int sshd_shell_input(sshd_client_t *client, char *buf, zpl_uint32 len)
{
	int ret = 0;
	if(client && client->sock)
	{
		ret = write(client->sock, buf, len);
		if(ret < 0)
		{
			if (!ERRNO_IO_RETRY(errno))
			{
				return SSH_ERROR;
			}
		}
		return ret ? ret:SSH_ERROR;
	}
	return SSH_ERROR;
}

/*
 * close VTY will be call
 */
static int sshd_close(struct vty *vty)
{
	sshd_client_t *sshclient = NULL;
	sshclient = vty->ssh;
	if(sshclient)
	{
	    zlog_debug(MODULE_UTILS, "%s :", __func__);

	    vty->ssh = NULL;
        vty->ssh_enable = zpl_false;
        if(sshclient->sock)
        	close(sshclient->sock);
    	sshclient->sock = 0;
    	if(vty->wfd)
    		close(vty->wfd);
        vty->wfd = 0;

	    ssh_event_remove_session(sshclient->config->event, sshclient->session);

		ssh_channel_close(sshclient->channel);

        ssh_disconnect(sshclient->session);
        ssh_free(sshclient->session);

        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
	}
	return 0;
}

/*
 * ssh close on exp will call
 */
static void sshd_shell_close(ssh_session session,
        ssh_channel channel,
        void *userdata)
{
    sshd_client_t *sshclient = (sshd_client_t *)userdata;
    zlog_debug(MODULE_UTILS, "%s :", __func__);

    //ssh_event_remove_session(client->config->event, client->session);
	if(sshclient)
	{
		if(sshclient->sock)
		{
			ssh_event_remove_fd(sshclient->config->event, sshclient->sock);
        	close(sshclient->sock);
        	sshclient->sock = 0;
		}
	    if(sshclient && sshclient->type == SSH_C_SHELL && sshclient->vty)
	    {
	    	sshclient->vty->ssh_close = NULL;
	    	vty_close(sshclient->vty);
	    	sshclient->vty->ssh = NULL;
	    	if(sshclient->vty->wfd)
	    		close(sshclient->vty->wfd);
	    	sshclient->vty->wfd = 0;
	    }
#ifdef SSH_SCPD_ENABLE
	    if(sshclient && sshclient->type == SSH_C_SCP)
	    {
	    	ssh_scpd_exit(sshclient);
	    	if(sshclient->scp_data.filename)
	    		free(sshclient->scp_data.filename);
	    	sshclient->scp_data.filename = NULL;

	    	if(sshclient->scp_data.scp)
	    		free(sshclient->scp_data.scp);
	    	sshclient->scp_data.scp = NULL;

	    	if(sshclient->scp_data.input)
	    		close(sshclient->scp_data.input);
	    	sshclient->scp_data.input = 0;
	    	sshclient->scp_data.output = 0;
	    }
#endif
        XFREE(MTYPE_SSH_CLIENT, sshclient);
	}
}

/*
 * ssh server close will call
 */
static void sshd_session_userdata_close(ssh_session session, sshd_client_t *sshclient)
{
	//sshd_client_t *sshclient = NULL;
	//sshclient = ssh_get_session_private(session);
	if(sshclient)
	{
		if(sshclient->sock)
		{
			ssh_event_remove_fd(sshclient->config->event, sshclient->sock);
        	close(sshclient->sock);
        	sshclient->sock = 0;
		}
	    if(sshclient && sshclient->type == SSH_C_SHELL && sshclient->vty)
	    {
	    	sshclient->vty->ssh_close = NULL;
	    	vty_close(sshclient->vty);
	    	sshclient->vty->ssh = NULL;
	    	if(sshclient->vty->wfd)
	    		close(sshclient->vty->wfd);
	    	sshclient->vty->wfd = 0;
	    }
#ifdef SSH_SCPD_ENABLE
	    if(sshclient && sshclient->type == SSH_C_SCP)
	    {
	    	ssh_scpd_exit(sshclient);
	    	if(sshclient->scp_data.filename)
	    		free(sshclient->scp_data.filename);
	    	sshclient->scp_data.filename = NULL;

	    	if(sshclient->scp_data.scp)
	    		free(sshclient->scp_data.scp);
	    	sshclient->scp_data.scp = NULL;

	    	if(sshclient->scp_data.input)
	    		close(sshclient->scp_data.input);
	    	sshclient->scp_data.input = 0;
	    	sshclient->scp_data.output = 0;
	    }
#endif
        XFREE(MTYPE_SSH_CLIENT, sshclient);
	}
}

static struct vty * sshd_shell_new(int vty_sock)
{
	struct vty *vty = NULL;
	vty = vty_new_init(vty_sock);
	vty->fd = vty_sock;
	vty->wfd = vty_sock;
	vty->fd_type = OS_STACK;

	vty->type = VTY_TERM;
	vty->node = ENABLE_NODE;
	vty->login_type = VTY_LOGIN_SSH;
	vty->ssh_enable = zpl_true;
	vty->ssh_close = sshd_close;

	host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vty->v_timeout);

	host_config_get_api(API_GET_LINES_CMD, &vty->lines);

	vty_sshd_init(vty->fd, vty);
	return vty;
}


static int sshd_shell_create(sshd_client_t *sshclient, ssh_session sseion)
{
	//int n = 0;
	struct sockaddr_in * client_address;
	int socket[2] = { 0, 0 };

	if(!sshclient)
		return SSH_ERROR;

    if(socketpair (AF_UNIX, SOCK_STREAM, 0, socket) == 0)
    {
    	sshclient->sock = socket[1];
    }
    else
    {
		return SSH_ERROR;
    }
	ssh_socket_set_nonblocking(socket[0]);
	ssh_socket_set_nonblocking(socket[1]);

	sshclient->vty = sshd_shell_new(socket[0]);
	zlog_debug(MODULE_UTILS, "%s :", __func__);
	if(!sshclient->vty)
	{
		sshclient->sock = 0;
    	close(socket[0]);
    	close(socket[1]);
		return SSH_ERROR;
	}

	client_address = ssh_get_session_client_address(sshclient->session);
	if(client_address)
	{
		if(client_address->sin_addr.s_addr)
		{
			union sockunion *su = client_address;
			char buf[SU_ADDRSTRLEN];
			sockunion2str(su, buf, SU_ADDRSTRLEN);
			strcpy(sshclient->vty->address, buf);
		}
	}
	sshclient->vty->ssh = sshclient;

	ssh_event_add_fd(sshclient->config->event, sshclient->sock, POLLIN, sshd_shell_output,
			sshclient);
	sshclient->type = SSH_C_SHELL;
	vty_write_hello(sshclient->vty);
	return SSH_OK;
}

static int sshd_shell_window_open(ssh_session session, ssh_channel channel,
                       const char *term, zpl_uint32 cols, zpl_uint32 rows, zpl_uint32 py, zpl_uint32 px,
                       void *userdata) {
    sshd_client_t *client = (sshd_client_t *)userdata;

    (void) session;
    (void) channel;
    (void) term;
    client->winsize.ws_row = rows;
    client->winsize.ws_col = cols;
    client->winsize.ws_xpixel = px;
    client->winsize.ws_ypixel = py;
    zlog_debug(MODULE_UTILS, "%s :", __func__);

    return SSH_OK;
}

static int sshd_shell_window_change(ssh_session session, ssh_channel channel, zpl_uint32 cols,
                      zpl_uint32 rows, zpl_uint32 py, zpl_uint32 px, void *userdata) {
	sshd_client_t *client = (sshd_client_t *)userdata;

    (void) session;
    (void) channel;

    client->winsize.ws_row = rows;
    client->winsize.ws_col = cols;
    client->winsize.ws_xpixel = px;
    client->winsize.ws_ypixel = py;
    zlog_debug(MODULE_UTILS, "%s :", __func__);
    return SSH_OK;
}


static int sshd_shell_request(ssh_session session, ssh_channel channel,
                         void *userdata) {
	sshd_client_t *client = (sshd_client_t *)userdata;
    (void) session;
    (void) channel;

    if(!client->config->shell_enable)
    	return SSH_ERROR;
    sshd_shell_create(client, session);

	if(sshd_shell_input(client, "\n", 1) == SSH_ERROR)
	{
		ssh_channel_send_eof(channel);
		ssh_channel_close(channel);
        if(client->sock)
        	close(client->sock);
        client->sock = 0;
		return SSH_ERROR;
	}
    zlog_debug(MODULE_UTILS, "%s :", __func__);
    return SSH_OK;
}



/*
static int sshd_scp_thread(sshd_client_t *client)
{
	char *buf = NULL;
	int len = 0;
	while(1)
	{
		extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, int timeout_ms);
	}
}
*/

#ifdef SSH_SCPD_ENABLE
static int sshd_exec_request(ssh_session session, ssh_channel channel,
                        const char *command, void *userdata)
{
	sshd_client_t *client = (sshd_client_t *)userdata;

    (void) session;
    (void) channel;
    //scp -f /tmp/resolv.conf
    zlog_debug(MODULE_UTILS, "%s : %s", __func__, command);
    if(strstr(command, "scp"))
    {
    	client->type = SSH_C_SCP;
    	ssh_scpd_init(client, command);
    }
    return SSH_OK;
}

static int sshd_subsystem_request(ssh_session session, ssh_channel channel,
                             const char *subsystem, void *userdata) {
    /* subsystem requests behave simillarly to exec requests. */

	zlog_debug(MODULE_UTILS, "sshd_subsystem_request:%s", subsystem);
#ifdef SSH_OPENSSH_TOOL
    if (strcmp(subsystem, "sftp") == 0) {
        return sshd_exec_request(session, channel, SFTP_SERVER_PATH, userdata);
    }
#else
    return SSH_OK;
#endif
    return SSH_ERROR;
}
#endif

static int sshd_auth_password(ssh_session session, const char *user,
                         const char *pass, void *userdata) {
	sshd_client_t *client = (sshd_client_t *)userdata;

    (void) session;
    zlog_debug(MODULE_UTILS, "auth_password : %s -> %s", user, pass);
    //return SSH_AUTH_SUCCESS;

    //if (strcmp(user, USER) == 0 && strcmp(pass, PASS) == 0)
    if(user_authentication(user, pass) == OK)
    {
    	client->authenticated = 1;
    	zlog_debug(MODULE_UTILS, "auth_password : SSH_AUTH_SUCCESS");
        return SSH_AUTH_SUCCESS;
    }

    client->auth_attempts++;
    return SSH_AUTH_DENIED;
}

/*static int auth_gssapi_mic(ssh_session session, const char *user, const char *principal, void *userdata){
    ssh_gssapi_creds creds = ssh_gssapi_get_creds(session);
    (void)userdata;
    printf("Authenticating user %s with gssapi principal %s\n",user, principal);
    if (creds != NULL)
        printf("Received some gssapi credentials\n");
    else
        printf("Not received any forwardable creds\n");
    printf("authenticated\n");
    authenticated = 1;
    return SSH_AUTH_SUCCESS;
}*/

static ssh_channel sshd_channel_open(ssh_session session, void *userdata) {
	sshd_client_t *client = (sshd_client_t *)userdata;

	client->channel = ssh_channel_new(session);
    return client->channel;
}



static int sshd_data_function(ssh_session session, ssh_channel channel, void *data,
                         zpl_uint32  len, int is_stderr, void *userdata) {
	sshd_client_t *client = (sshd_client_t *)userdata;
    (void) session;
    (void) channel;
    (void) is_stderr;
    char *buf = (char *)data;
    if (len == 0 || !client/* || !client->vty*/) {
        return 0;
    }
    if(ssh_channel_is_open(channel))
    {
    	int ret = 0;
    	zlog_debug(MODULE_UTILS, "data_function :%s(%x->%d)", buf, buf[0],len);
    	ret = sshd_shell_input(client, (char *) data, len);
    	if(ret == SSH_ERROR)
    	{
    		ssh_channel_send_eof(channel);
    		ssh_channel_close(channel);
            if(client->sock)
            	close(client->sock);
            client->sock = 0;
    		return SSH_ERROR;
    	}
    	return ret;
    }
    if(client->sock)
    	close(client->sock);
    client->sock = 0;
    return SSH_ERROR;
}


static int sshd_userdata_set(sshd_client_t *client, ssh_config_t *ssh, ssh_session sseion)
{
	zpl_uint32 auth_methods = 0;
	client->config = ssh;
	client->session = sseion;

	client->winsize.ws_row = 0;
    client->winsize.ws_col = 0;
	client->winsize.ws_xpixel = 0;
	client->winsize.ws_ypixel = 0;

	client->channel = NULL;
	client->auth_attempts = 0;
	client->authenticated = 0;

    client->ssh_channel_cb.userdata = client;
    client->ssh_channel_cb.channel_data_function = sshd_data_function;
    client->ssh_channel_cb.channel_eof_function = NULL;
    client->ssh_channel_cb.channel_close_function = sshd_shell_close;

    client->ssh_channel_cb.channel_signal_function = NULL;
    client->ssh_channel_cb.channel_exit_status_function = NULL;
    client->ssh_channel_cb.channel_exit_signal_function = NULL;
    client->ssh_channel_cb.channel_pty_request_function = sshd_shell_window_open;
    client->ssh_channel_cb.channel_shell_request_function = sshd_shell_request;
    client->ssh_channel_cb.channel_auth_agent_req_function = NULL;
    client->ssh_channel_cb.channel_x11_req_function = NULL;
    client->ssh_channel_cb.channel_pty_window_change_function = sshd_shell_window_change;
#ifdef SSH_SCPD_ENABLE
    client->ssh_channel_cb.channel_exec_request_function = sshd_exec_request;
    client->ssh_channel_cb.channel_env_request_function = NULL;
    client->ssh_channel_cb.channel_subsystem_request_function = sshd_subsystem_request;
#endif
    //client->ssh_channel_cb.channel_close_exit_function = sshd_session_userdata_close;

    client->ssh_server_cb.userdata = client;
    client->ssh_server_cb.auth_password_function = sshd_auth_password;
    client->ssh_server_cb.auth_none_function = NULL;
    client->ssh_server_cb.auth_gssapi_mic_function = NULL;//auth_gssapi_mic;
    client->ssh_server_cb.auth_pubkey_function = NULL;
    client->ssh_server_cb.service_request_function = NULL;
    client->ssh_server_cb.channel_open_request_session_function = sshd_channel_open;
    client->ssh_server_cb.gssapi_select_oid_function = NULL;
    client->ssh_server_cb.gssapi_accept_sec_ctx_function = NULL;
    client->ssh_server_cb.gssapi_verify_mic_function = NULL;

    sseion->session_callbacks.session_close_function = sshd_session_userdata_close;
    sseion->session_callbacks.userdata = client;

    ssh_callbacks_init(&client->ssh_server_cb);
    ssh_callbacks_init(&client->ssh_channel_cb);

    ssh_set_server_callbacks(client->session, &client->ssh_server_cb);

    if (ssh_handle_key_exchange(client->session) != SSH_OK)
    {
        return SSH_ERROR;
    }
    switch(ssh->auth_type)
    {
    case SSH_AUTH_NONE:
       	auth_methods = SSH_AUTH_METHOD_NONE;
    	break;
    case SSH_AUTH_AUTO:
        auth_methods = SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY |
        		SSH_AUTH_METHOD_HOSTBASED | SSH_AUTH_METHOD_INTERACTIVE;
    	break;
    case SSH_AUTH_PASSWORD:
    	auth_methods = SSH_AUTH_METHOD_PASSWORD;
    	break;
    case SSH_AUTH_PUBLIC_KEY:
    	auth_methods = SSH_AUTH_METHOD_PUBLICKEY;
    	break;
    case SSH_AUTH_RSA:
     	auth_methods = SSH_AUTH_METHOD_PUBLICKEY;
    	break;
    case SSH_AUTH_HOSTBASE:
     	auth_methods = SSH_AUTH_METHOD_HOSTBASED;
    	break;	//host
    case SSH_AUTH_KB:
     	auth_methods = SSH_AUTH_METHOD_INTERACTIVE;
    	break;		//keyboard
    case SSH_AUTH_GSSAPI:
     	auth_methods = SSH_AUTH_METHOD_GSSAPI_MIC;
    	break;
    default:
        auth_methods = SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_PUBLICKEY |
        		SSH_AUTH_METHOD_HOSTBASED | SSH_AUTH_METHOD_INTERACTIVE;
    	break;
    }
    ssh_set_auth_methods(client->session, auth_methods);

    return SSH_OK;
}


static int sshd_key_authenticate(sshd_client_t *client, ssh_event event)
{
	//int n = 0;
    //n = 0;
    ssh_event_add_session(event, client->session);
    while (client->authenticated == 0 || client->channel == NULL) {
        /* If the user has used up all attempts, or if he hasn't been able to
         * authenticate in 10 seconds (n * 100ms), disconnect. */
        if (client->auth_attempts >= client->config->auth_retries/* ||
        		n >= 3*/) {
        	ssh_event_remove_session(event, client->session);
        	zlog_debug(MODULE_UTILS, "%s : too mush time", __func__);
            return SSH_ERROR;
        }

        if (ssh_event_dopoll(event, client->config->auth_waitting * 1000) == SSH_ERROR) {
        	ssh_event_remove_session(event, client->session);
            zlog_debug(MODULE_UTILS, "%s : %s", __func__, ssh_get_error(client->session));
            return SSH_ERROR;
        }
        //n++;
    }
    ssh_set_channel_callbacks(client->channel, &client->ssh_channel_cb);
    return SSH_OK;
}


static int sshd_client_create(ssh_config_t *ssh, ssh_session sseion)
{
	int n = 0;
	sshd_client_t *sshclient = NULL;
	sshclient = XMALLOC(MTYPE_SSH_CLIENT, sizeof(sshd_client_t));
	if(!sshclient)
		return SSH_ERROR;

	memset(sshclient, 0, sizeof(sshd_client_t));

	ssh_set_session_private(sseion, sshclient);
	if(ssh->ssh_version)
	{
		int value = 1;
		if(ssh->ssh_version == 1)
			ssh_options_set(sseion, SSH_OPTIONS_SSH1, &value);
		if(ssh->ssh_version == 2)
			ssh_options_set(sseion, SSH_OPTIONS_SSH2, &value);
		if(ssh->ssh_version == 3)
		{
			ssh_options_set(sseion, SSH_OPTIONS_SSH1, &value);
			ssh_options_set(sseion, SSH_OPTIONS_SSH2, &value);
		}
	}
	n = sshd_userdata_set(sshclient, ssh,  sseion);
	if(n != SSH_OK)
	{
		ssh_set_session_private(sseion, NULL);
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
		return SSH_ERROR;
	}
	n = sshd_key_authenticate(sshclient, ssh->event);
	if(n != SSH_OK)
	{
		//ssh_event_remove_session(ssh->event, sseion);
		ssh_set_session_private(sseion, NULL);
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
		return SSH_ERROR;
	}
	ssh_socket_set_nonblocking(ssh_get_fd(sshclient->session));

	//sshd_shell_create(sshclient, sshclient->session);
	return SSH_OK;
}

int sshd_accept(socket_t fd, zpl_uint32 revents, void *userdata)
{
	ssh_session session = NULL;
	ssh_config_t *ssh_config = userdata;
    session = ssh_new();
    if (session == NULL) {
    	ssh_printf(NULL, "Failed to allocate session\n");
        return ERROR;
    }
    /* Blocks until there is a new incoming connection. */
    if(ssh_bind_accept(ssh_config->sshbind, session) != SSH_ERROR)
    {
    	//TODO check ACL
    	if(!sshd_acl_action(ssh_config, session))
    	{
        	ssh_disconnect(session);
            ssh_free(session);
            return SSH_ERROR;
    	}
        if(sshd_client_create(ssh_config, session) != SSH_OK)
        {
        	ssh_disconnect(session);
            ssh_free(session);
            return SSH_ERROR;
        }
        return SSH_OK;
    }
    else
    {
        ssh_disconnect(session);
        ssh_free(session);
    }
    return SSH_ERROR;
}




int sshd_task(void *argv)
{
	int ret = 0;
	zpl_uint32 waittime = 2;

	ssh_config_t *sshd = argv;
	host_config_load_waitting();
	while(1)
	{
		if(sshd->quit)
			break;
		//if(sshd->running)
		{
			ret = ssh_event_dopoll(sshd->event, waittime);
			if (ret == SSH_ERROR)
			{
				break;
			}
			if (ret == SSH_AGAIN)//timeout
			{
			}
		}
/*		else
		{
			os_sleep(1);
		}*/
	}
	sshd->quit = zpl_false;
	if(!sshd->event)
		return OK;
	if(ssh_event_session_count(sshd->event))
	{

	}
	ssh_module_exit();
	return 0;
}




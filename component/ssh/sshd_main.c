
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
#include "sshd_main.h"
#include "ssh_util.h"

static ssh_config_t sshd_config;



#ifndef SSH_SHELL_PROXY_ENABLE
static int sshd_shell_flush(struct vty *vty)
{
	int ret = 0;
	ret = buffer_flush_available(vty->obuf, vty->wfd, vty->fd_type);
	while(ret == BUFFER_PENDING)
		ret = buffer_flush_available(vty->obuf, vty->wfd, vty->fd_type);
	if(ret == BUFFER_ERROR)
	{
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(ZLOG_DEFAULT, "%s: write error to fd %d, closing", __func__, vty->wfd);
		buffer_reset(vty->obuf);
		vty_close(vty);
		return ERROR;
	}
	zlog_debug(ZLOG_UTILS, "into:%s", "sshd_vty_flush");
	return 0;
}

static int sshd_shell_execute (struct vty *vty, unsigned char buf[], int nbytes)
{
	unsigned char *p;

	if (vty->length + nbytes >= vty->max)
	{
		/* Clear command line buffer. */
		vty->cp = vty->length = 0;

		memset(vty->buf, 0, vty->max);
		vty_out (vty, "%% Command is too long.%s", VTY_NEWLINE);
		return 0;
	}

	for (p = buf; p < buf+nbytes; p++)
	{
		vty->buf[vty->length++] = *p;
		if (*p == '\0' || *p == '\r' || *p == '\n')
		{
			vty_execute (vty);
			sshd_shell_flush(vty);
			return 0;
		}
	}
	return 0;
}
#endif

static int sshd_shell_output(socket_t fd, int revents, void *userdata)
{
    char buf[4096];
    int n = -1;
    os_bzero(buf, sizeof(buf));
    ssh_channel channel = (ssh_channel) userdata;
	//zlog_debug(ZLOG_UTILS, "start:%s", buf);
    if ((revents & POLLIN) != 0) {
        n = read(fd, buf, sizeof(buf));
        if (n > 0) {
        	if(channel != NULL)
        		ssh_channel_write(channel, buf, n);
        }
    }
	zlog_debug(ZLOG_UTILS, "end:%s", buf);
    return n;
}


static int sshd_shell_close(struct vty *vty)
{
	ssh_config_client_t *sshclient = NULL;
	sshclient = vty->ssh;
	if(sshclient)
	{
		int n = 0;
		//ssh_channel_send_eof(sshclient->sdata.channel);
		ssh_channel_close(sshclient->sdata.channel);

		/* Wait up to 5 seconds for the client to terminate the session. */
/*		for (n = 0; n < 50 && (ssh_get_status(sshclient->session) & SESSION_END) == 0; n++)
		{
			ssh_event_dopoll(sshclient->config->event, 100);
		}*/
        ssh_disconnect(sshclient->session);
        ssh_free(sshclient->session);
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
        vty->ssh = NULL;
        vty->ssh_enable = FALSE;
#ifndef SSH_SHELL_PROXY_ENABLE
        os_pipe_close(vty->wfd);
        vty->wfd = 0;
#endif
	}
	return 0;
}

#ifndef SSH_SHELL_PROXY_ENABLE
static int sshd_shell_timeout(ssh_session session, void *userdata, int timev)
{
	struct vty *vty = NULL;
	ssh_config_client_t *sshclient = userdata;
	if(!sshclient)
		return 0;
	vty = sshclient->vty;
	if(vty && sshclient->timeval && sshclient->timeval <= timev)
	{
		vty->t_timeout = NULL;
		vty->v_timeout = 0;
		buffer_reset(vty->obuf);
		vty_out(vty, "%s%sVty connection is timed out.%s%s",
				VTY_NEWLINE, VTY_NEWLINE,
				VTY_NEWLINE, VTY_NEWLINE);
		vty->status = VTY_CLOSE;
		vty_close(vty);
	}
	return 0;
}
#endif


static void set_default_keys(ssh_bind sshbind,
                             int rsa_already_set,
                             int dsa_already_set,
                             int ecdsa_already_set) {
    if (!rsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
                             KEYS_FOLDER "ssh_host_rsa_key");
        //ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY,
        //                     KEYS_FOLDER "id_rsa");
    }
    if (!dsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY,
                             KEYS_FOLDER "ssh_host_dsa_key");
    }
    if (!ecdsa_already_set) {
        ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_ECDSAKEY,
                             KEYS_FOLDER "ssh_host_ecdsa_key");
    }
}



static int data_function(ssh_session session, ssh_channel channel, void *data,
                         uint32_t len, int is_stderr, void *userdata) {
    struct channel_data_struct *cdata = (struct channel_data_struct *) userdata;
    ssh_config_client_t * client = ssh_get_session_private(session);
    (void) session;
    (void) channel;
    (void) is_stderr;
    char *buf = (char *)data;
    if (len == 0 || !cdata->vty) {
        return 0;
    }
    if(ssh_channel_is_open(channel))
    {
    	zlog_debug(ZLOG_UTILS, "data_function :%s(%x->%d)", buf, buf[0],len);
#ifndef SSH_SHELL_PROXY_ENABLE
    	sshd_shell_execute(cdata->vty, buf, len);

    	client->timeval = cdata->vty->v_timeout + os_time(NULL);

    	if (cdata->vty->status == VTY_CLOSE)
    	{
    		vty_close(cdata->vty);
    		return SSH_ERROR;
    	}
    	return len;//write(cdata->child_stdin, (char *) data, len);
#else
    	int ret = 0;
    	if(client && client->sock)
    	ret = write(client->sock, (char *) data, len);
    	if(ret < 0)
    	{
//#define ERRNO_IO_RETRY(EN) \
//	(((EN) == EAGAIN) || ((EN) == EWOULDBLOCK) || ((EN) == EINTR))
    		if (!ERRNO_IO_RETRY(errno))
    		{
    		    ssh_channel_send_eof(channel);
    		    ssh_channel_close(channel);
				return SSH_ERROR;
    		}
    	}
    	return ret;
#endif
    }
    return SSH_ERROR;
}


static int pty_request(ssh_session session, ssh_channel channel,
                       const char *term, int cols, int rows, int py, int px,
                       void *userdata) {
    struct channel_data_struct *cdata = (struct channel_data_struct *)userdata;

    (void) session;
    (void) channel;
    (void) term;

    cdata->winsize->ws_row = rows;
    cdata->winsize->ws_col = cols;
    cdata->winsize->ws_xpixel = px;
    cdata->winsize->ws_ypixel = py;
    zlog_debug(ZLOG_UTILS, "pty_request :");
    return SSH_OK;
}

static int pty_resize(ssh_session session, ssh_channel channel, int cols,
                      int rows, int py, int px, void *userdata) {
    struct channel_data_struct *cdata = (struct channel_data_struct *)userdata;

    (void) session;
    (void) channel;

    cdata->winsize->ws_row = rows;
    cdata->winsize->ws_col = cols;
    cdata->winsize->ws_xpixel = px;
    cdata->winsize->ws_ypixel = py;
    zlog_debug(ZLOG_UTILS, "pty_resize :");
    return SSH_OK;
}


static int shell_request(ssh_session session, ssh_channel channel,
                         void *userdata) {
    struct channel_data_struct *cdata = (struct channel_data_struct *) userdata;

    (void) session;
    (void) channel;

    return SSH_OK;
}

static int auth_password(ssh_session session, const char *user,
                         const char *pass, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    (void) session;
    zlog_debug(ZLOG_UTILS, "auth_password : %s -> %s", user, pass);
    if (strcmp(user, USER) == 0 && strcmp(pass, PASS) == 0) {
        sdata->authenticated = 1;
        return SSH_AUTH_SUCCESS;
    }

    sdata->auth_attempts++;
    return SSH_AUTH_DENIED;
}

static ssh_channel channel_open(ssh_session session, void *userdata) {
    struct session_data_struct *sdata = (struct session_data_struct *) userdata;

    sdata->channel = ssh_channel_new(session);
    return sdata->channel;
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
	vty->ssh_enable = TRUE;
	vty->ssh_close = sshd_shell_close;

	host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vty->v_timeout);

	host_config_get_api(API_GET_LINES_CMD, &vty->lines);
#ifndef SSH_SHELL_PROXY_ENABLE
	if(vty->wfd <= 0)
	{
		vty_close(vty);
		return NULL;
	}
#else
	vty_sshd_init(vty->fd, vty);
#endif
	return vty;
}


static int sshd_userdata_set(ssh_config_client_t *client, ssh_bind 	sshbind, ssh_session sseion)
{
	//client->sshbind = sshbind;
	client->session = sseion;

	client->wsize.ws_row = 0;
    client->wsize.ws_col = 0;
	client->wsize.ws_xpixel = 0;
	client->wsize.ws_ypixel = 0;

	//client->cdata.event = NULL;
	client->cdata.winsize = &client->wsize;

	client->sdata.channel = NULL;
	client->sdata.auth_attempts = 0;
	client->sdata.authenticated = 0;

    client->channel_cb.userdata = &client->cdata;
	client->channel_cb.channel_pty_request_function = pty_request;
	client->channel_cb.channel_pty_window_change_function = pty_resize;
	client->channel_cb.channel_shell_request_function = shell_request;
	client->channel_cb.channel_exec_request_function = NULL;
	client->channel_cb.channel_data_function = data_function;
	client->channel_cb.channel_subsystem_request_function = NULL;

    client->server_cb.userdata = &client->sdata;
	client->server_cb.auth_password_function = auth_password;
	client->server_cb.channel_open_request_session_function = channel_open;

    ssh_callbacks_init(&client->server_cb);
    ssh_callbacks_init(&client->channel_cb);

    ssh_set_server_callbacks(client->session, &client->server_cb);

    if (ssh_handle_key_exchange(client->session) != SSH_OK)
    {
        return SSH_ERROR;
    }

    ssh_set_auth_methods(client->session, SSH_AUTH_METHOD_PASSWORD);
#ifndef SSH_SHELL_PROXY_ENABLE
    ssh_set_session_timeout_cb(client->session, sshd_shell_timeout);
#endif
    return SSH_OK;
}


static int sshd_key_authenticate(ssh_config_client_t *client, ssh_event event)
{
	int n = 0;
    n = 0;
    ssh_event_add_session(event, client->session);
    while (client->sdata.authenticated == 0 || client->sdata.channel == NULL) {
        /* If the user has used up all attempts, or if he hasn't been able to
         * authenticate in 10 seconds (n * 100ms), disconnect. */
        if (client->sdata.auth_attempts >= 3 || n >= 100) {
        	zlog_debug(ZLOG_UTILS, "%s : too mush time", __func__);
            return SSH_ERROR;
        }

        if (ssh_event_dopoll(event, 100) == SSH_ERROR) {
            zlog_debug(ZLOG_UTILS, "%s : %s", __func__, ssh_get_error(client->session));
            return SSH_ERROR;
        }
        n++;
    }
    ssh_set_channel_callbacks(client->sdata.channel, &client->channel_cb);
    return SSH_OK;
}


static int sshd_shell_create(ssh_config_t *ssh, ssh_session sseion)
{
	int n = 0;
	struct sockaddr_in * client_address;
#ifdef SSH_SHELL_PROXY_ENABLE
	int socket[2] = { 0, 0 };
#endif
	ssh_config_client_t *sshclient = NULL;
	sshclient = XMALLOC(MTYPE_SSH_CLIENT, sizeof(ssh_config_client_t));
	if(!sshclient)
		return SSH_ERROR;

	memset(sshclient, 0, sizeof(ssh_config_client_t));

	ssh_set_session_private(sseion, sshclient);

	n = sshd_userdata_set(sshclient, ssh->sshbind,  sseion);
	if(n != SSH_OK)
	{
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
		return SSH_ERROR;
	}
	n = sshd_key_authenticate(sshclient, ssh->event);
	if(n != SSH_OK)
	{
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
		return SSH_ERROR;
	}
#ifdef SSH_SHELL_PROXY_ENABLE
    if(socketpair (AF_UNIX, SOCK_STREAM, 0, socket) == 0)
    {
    	sshclient->sock = socket[1];
    }
    else
    {
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
		return SSH_ERROR;
    }
	ssh_socket_set_nonblocking(socket[0]);
	ssh_socket_set_nonblocking(socket[1]);
#endif

#ifdef SSH_SHELL_PROXY_ENABLE
	sshclient->vty = sshd_shell_new(socket[0]);
#else
	sshclient->vty = sshd_shell_new(ssh_get_fd(sshclient->session));
#endif
	if(!sshclient->vty)
	{
    	close(socket[0]);
    	close(socket[1]);
        XFREE(MTYPE_SSH_CLIENT, sshclient);
        sshclient = NULL;
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
	ssh_socket_set_nonblocking(ssh_get_fd(sshclient->session));

	sshclient->vty->ssh = sshclient;
	sshclient->cdata.vty = sshclient->vty;
#ifndef SSH_SHELL_PROXY_ENABLE
	sshclient->timeval = sshclient->vty->v_timeout + os_time(NULL);
	ssh_event_add_fd(ssh->event, sshclient->vty->wfd, POLLIN, sshd_shell_output,
			sshclient->sdata.channel);
#else
	ssh_event_add_fd(ssh->event, sshclient->sock, POLLIN, sshd_shell_output,
			sshclient->sdata.channel);
#endif

#ifndef SSH_SHELL_PROXY_ENABLE
	vty_write_hello(sshclient->vty);
	sshd_shell_flush(sshclient->vty);
#else
	vty_write_hello(sshclient->vty);
#endif
	return SSH_OK;
}



static int sshd_accept(socket_t fd, int revents, void *userdata)
{
	ssh_session session = NULL;
	ssh_config_t *ssh_config = userdata;
    session = ssh_new();
    if (session == NULL) {
        fprintf(ssh_stderr, "Failed to allocate session\n");
        return ERROR;
    }
    /* Blocks until there is a new incoming connection. */
    if(ssh_bind_accept(ssh_config->sshbind, session) != SSH_ERROR)
    {
        if(sshd_shell_create(ssh_config, session) != SSH_OK)
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


int sshd_enable(char *address, int port)
{
	int local_port = port;
    if(!sshd_config.init)
    	sshd_module_init();

    if(port)
    	ssh_bind_options_set(sshd_config.sshbind, SSH_BIND_OPTIONS_BINDPORT, &local_port);

    if(address)
    	ssh_bind_options_set(sshd_config.sshbind, SSH_BIND_OPTIONS_BINDADDR, address);

    set_default_keys(sshd_config.sshbind, 0, 1, 1);

    if(ssh_bind_listen(sshd_config.sshbind) < 0) {
        fprintf(ssh_stderr, "%s\n", ssh_get_error(sshd_config.sshbind));
        return 1;
    }
    ssh_socket_set_nonblocking(ssh_bind_get_fd(sshd_config.sshbind));
    ssh_event_add_fd(sshd_config.event,
		  ssh_bind_get_fd(sshd_config.sshbind), POLLIN, sshd_accept, &sshd_config);

    sshd_module_task_init ();

    return OK;
}

int sshd_disable()
{
	sshd_config.quit = TRUE;
	return OK;
}

int sshd_module_init()
{
	memset(&sshd_config, 0, sizeof(sshd_config));
    ssh_init();
    ssh_set_log_level(7);
    ssh_set_log_callback(ssh_log_callback_func);
    sshd_config.sshbind = ssh_bind_new();
    sshd_config.event = ssh_event_new();
    sshd_config.init = TRUE;
    return OK;
}

int sshd_module_exit()
{
    ssh_event_free(sshd_config.event);
    ssh_bind_free(sshd_config.sshbind);
    ssh_finalize();
    sshd_config.init = FALSE;
    return OK;
}



static int sshd_module_task(void *argv)
{
	int ret = 0;
	int waittime = 2;
#ifndef SSH_SHELL_PROXY_ENABLE
	int timev = 0;
#endif
	ssh_config_t *ssh_config = argv;
	while(!os_load_config_done())
	{
		os_sleep(1);
	}
	while(1)
	{
		if(ssh_config->quit)
			break;
		ret = ssh_event_dopoll(ssh_config->event, waittime);
		if (ret == SSH_ERROR)
		{
			break;
		}
		if (ret == SSH_AGAIN)//timeout
		{
#ifndef SSH_SHELL_PROXY_ENABLE
			timev = os_time(NULL);
			ssh_handle_session_timeout(ssh_config->event, timev);
#endif
		}
	}
	ssh_config->quit = FALSE;
	if(!ssh_config->event)
		return OK;
	if(ssh_event_session_count(ssh_config->event))
	{

	}
	sshd_module_exit();
	return 0;
}

int sshd_module_task_init ()
{
	if(sshd_config.sshd_taskid == 0)
		sshd_config.sshd_taskid = os_task_create("sshdTask", OS_TASK_DEFAULT_PRIORITY,
	               0, sshd_module_task, &sshd_config, OS_TASK_DEFAULT_STACK);
	if(sshd_config.sshd_taskid)
		return OK;
	return ERROR;
}

int sshd_module_task_exit ()
{
	sshd_config.quit = TRUE;
/*	if(sshd_config.sshd_taskid)
		os_task_destroy(sshd_config.sshd_taskid);*/
	sshd_config.sshd_taskid = 0;
	return OK;
}





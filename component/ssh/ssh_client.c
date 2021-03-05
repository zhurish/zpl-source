/*
 * ssh_keymgt.c
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
#include "os_task.h"
#include "ssh_api.h"
#include "ssh_util.h"



static int ssh_client_auth_callback(const char *prompt, char *buf, ospl_size_t len,
    ospl_uint32 echo, ospl_uint32 verify, void *userdata) {
    (void) verify;
    struct vty *vty = (struct vty *)userdata;
    if(vty)
    {
    	ospl_size_t slen = ssh_getpass(vty->fd, prompt, buf, len, echo, verify);
    	vty_out(vty, "get password:%s%s", buf, VTY_NEWLINE);
    	return slen;
    	//return ssh_getpass(vty->fd, prompt, buf, len, echo, verify);
    }
    return -1;
}

struct ssh_callbacks_struct ssh_client_auth_cb = {
		.auth_function=ssh_client_auth_callback,
		.userdata=NULL
};

static void ssh_client_pty_size_changed(ssh_channel channel, struct vty *vty)
{
	struct winsize win = { 0, 0, 0, 0 };
	win.ws_col = vty->width;
	win.ws_row = vty->height;
	ssh_channel_change_pty_size(channel, win.ws_col, win.ws_row);
}

static int ssh_client_exit(ssh_session session);

static void ssh_client_select_loop(ssh_session session,ssh_channel channel)
{
	fd_set fds;
	struct timeval timeout;
	char buffer[4096];
	/* channels will be set to the channels to poll.
	 * outchannels will contain the result of the poll
	 */
	ssh_channel channels[2], outchannels[2];
	ospl_uint32 lus = 0;
	ospl_uint32 eof = 0;
	int maxfd = 0;
	ospl_uint32 r = 0;
	int ret = 0;
	struct vty *vty = ssh_get_session_private(session);
	while (channel)
	{
		do
		{
			int fd;

			FD_ZERO(&fds);
			if (!eof)
				FD_SET(vty->fd, &fds);
			timeout.tv_sec = 30;
			timeout.tv_usec = 0;

			fd = ssh_get_fd(session);
			if (fd < 0)
			{
				fprintf(stderr, "Error getting fd\n");
				goto taskout;
			}
			FD_SET(fd, &fds);
			maxfd = MAX(fd, vty->fd) + 1;

			channels[0] = channel; // set the first channel we want to read from
			channels[1] = NULL;
			//ssh_printf(NULL, "Error ========= ssh_select\n");
			ret = ssh_select(channels, outchannels, maxfd, &fds, &timeout);
			if (ret == EINTR)
				continue;
			if (FD_ISSET(vty->fd, &fds))
			{
				lus = read(vty->fd, buffer, sizeof(buffer));
				if (lus)
					ssh_channel_write(channel, buffer, lus);
				else
				{
					eof = 1;
					ssh_printf(NULL, "Error ========= ssh_channel_send_eof\n");
					ssh_channel_send_eof(channel);
				}
			}
			if (channel && ssh_channel_is_closed(channel))
			{
				ssh_printf(NULL, "Error ========= ssh_channel_free 0\n");
				ssh_channel_free(channel);
				channel = NULL;
				channels[0] = NULL;
				goto taskout;
			}
			if (outchannels[0])
			{
				while (channel && ssh_channel_is_open(channel) && (r =
						ssh_channel_poll(channel, 0)) != 0)
				{
					lus = ssh_channel_read(channel, buffer,
							sizeof(buffer) > r ? r : sizeof(buffer), 0);
					if (lus == -1)
					{
						ssh_printf(NULL, "Error reading channel: %s\n",
								ssh_get_error(session));
						goto taskout;
					}
					if (lus == 0)
					{
						ssh_printf(NULL, "Error ========= ssh_channel_free 1\n");
						ssh_channel_free(channel);
						channel = channels[0] = NULL;
						goto taskout;
					}
					else if (write(vty->wfd, buffer, lus) < 0)
					{
						ssh_printf(NULL, "Error writing to buffer\n");
						goto taskout;
					}
				}
				while (channel && ssh_channel_is_open(channel) && (r =
						ssh_channel_poll(channel, 1)) != 0)
				{ /* stderr */
					lus = ssh_channel_read(channel, buffer,
							sizeof(buffer) > r ? r : sizeof(buffer), 1);
					if (lus == -1)
					{
						ssh_printf(NULL, "Error reading channel: %s\n",
								ssh_get_error(session));
						goto taskout;
					}
					if (lus == 0)
					{
						ssh_printf(NULL, "Error ========= ssh_channel_free 2\n");
						ssh_channel_free(channel);
						channel = channels[0] = NULL;
						goto taskout;
					}
					else if (write(vty->wfd, buffer, lus) < 0)
					{
						ssh_printf(NULL, "Error writing to buffer\n");
						goto taskout;
					}
				}
			}
			if (channel && ssh_channel_is_closed(channel))
			{
				ssh_printf(NULL, "Error ========= ssh_channel_free 3\n");
				ssh_channel_free(channel);
				channel = NULL;
				goto taskout;
			}
		} while (ret == EINTR || ret == SSH_EINTR);

	}
taskout:
	ssh_printf(NULL, "taskout =========================\n");
	if (channel && !ssh_channel_is_closed(channel))
		ssh_channel_close(channel);
	if(channel)
		ssh_channel_free(channel);
	ssh_printf(NULL, "taskout ============------=========\n");
	ssh_client_exit( session);
}


static int ssh_client_exit(ssh_session session)
{
	struct vty *vty = ssh_get_session_private(session);
	if(vty)
	{
		ssh_printf(NULL, "taskout ============vty_ansync_enable\n");
		vty_ansync_enable(vty, ospl_false);
		vty_resume(vty);
	}
	ssh_printf(NULL, "taskout ============ssh_stdout_set\n");
	ssh_stdout_set(NULL);
	ssh_printf(NULL, "taskout ============ssh_free\n");
	ssh_free(session);
	return 0;
}

static void ssh_client_shell(ssh_session session)
{
	ssh_channel channel;
	struct vty *vty = ssh_get_session_private(session);
	channel = ssh_channel_new(session);
	if (ssh_channel_open_session(channel))
	{
		ssh_printf(NULL, "error opening channel : %s\n",
				ssh_get_error(session));
		ssh_client_exit(session);
		return;
	}
	ssh_channel_request_pty(channel);
	if(vty)
		ssh_client_pty_size_changed(channel, vty);

	if (ssh_channel_request_shell(channel))
	{
		ssh_printf(NULL, "Requesting shell : %s\n", ssh_get_error(session));
		ssh_client_exit(session);
		return;
	}
	ssh_client_select_loop(session, channel);
}


int ssh_client(struct vty *vty, char *remotehost, ospl_uint16 port, char *user, char *pasword)
{
    ssh_session session;
	vty_ansync_enable(vty, ospl_true);
	vty_cancel(vty);
	vty_out(vty, "%sTrying %s...%s", VTY_NEWLINE, remotehost, VTY_NEWLINE);
	session = ssh_new();
	if(!session)
	{
		vty_ansync_enable(vty, ospl_false);
		vty_resume(vty);
		return -1;
	}
	ssh_stdout_set(vty);
	ssh_client_auth_cb.userdata = vty;
    ssh_callbacks_init(&ssh_client_auth_cb);
    ssh_set_callbacks(session, &ssh_client_auth_cb);
    if(ssh_client_connect_api(session, vty, remotehost,  port, user, pasword))
    {
		if(os_task_create("ssh-client", OS_TASK_DEFAULT_PRIORITY,
				   0, ssh_client_shell, session, OS_TASK_DEFAULT_STACK) > 0)
		{
			//vty_ansync_enable(vty, ospl_false);
			return OK;
		}
    }
    else
    {
		vty_ansync_enable(vty, ospl_false);
		vty_resume(vty);
    }
	//vty_ansync_enable(vty, ospl_false);
    return 0;
}

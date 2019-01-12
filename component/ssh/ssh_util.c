/*
 * ssh_util.c
 *
 *  Created on: Nov 1, 2018
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

/*
int ssh_set_log_level(int level)
int ssh_set_log_callback(ssh_logging_callback cb)
int ssh_set_log_userdata(void *data)
*/
static struct vty *ssh_vty = NULL;

void ssh_log_callback_func(int priority,
        const char *function,
        const char *buffer,
        void *userdata)
{
	switch(priority)
	{
	case LOG_ERR:
		zlog_err(ZLOG_UTILS, "SSH %s", buffer);
		break;
	case LOG_WARNING:
		zlog_warn(ZLOG_UTILS, "SSH %s", buffer);
		break;
	case LOG_INFO:
		zlog_info(ZLOG_UTILS, "SSH %s", buffer);
		break;
	case LOG_NOTICE:
		zlog_notice(ZLOG_UTILS, "SSH %s", buffer);
		break;
	case LOG_DEBUG:
		zlog_debug(ZLOG_UTILS, "SSH %s", buffer);
		break;
	case LOG_TRAP:
		zlog_trap(ZLOG_UTILS, "SSH %s", buffer);
		break;
	default:
		break;
	}
}

/*
#ifdef SSH_STD_REDIST
FILE * _ssh_stdout()
{
	return (FILE *)stdout;
}
FILE * _ssh_stderr()
{
	return (FILE *)stderr;
}
FILE * _ssh_stdin()
{
	return (FILE *)stdin;
}
#endif
*/

/*
FILE * ssh_stdin_get(ssh_session session)
{
	if(!session)
	{
		return _ssh_stdin();
	}
    struct vty * vty = ssh_get_session_private(session);
    if(vty)
    {
    	return fdopen(vty->fd, "a+");
    }
    else
    	return _ssh_stdin();
}
*/

int ssh_stdin_get(ssh_session session)
{
    struct vty * vty = NULL;
	if(!session)
	{
		//printf();
		vty = ssh_vty;
	}
	else
	{
		vty = ssh_get_session_private(session);
	}
    if(vty)
    {
    	return vty->fd;
    }
    return -1;
}

int ssh_get_input(int fd, char *buf, int len)
{
	int c = 0, i = 0;
	while(1)
	{
		c = vty_getc_input(ssh_vty);
		if(c == '\r' || c == '\n')
		{
			buf[i++] = '\n';
			break;
		}
		else
		{
			if(i < len)
				buf[i++] = c;
			else
				return 0;
		}
	}
	return i;
}

int ssh_stdout_set(void *v)
{
	ssh_vty = v;
	return OK;
}

int ssh_printf(ssh_session session, const char *fmt,...)
{
    struct vty * vty = NULL;
	if(!session)
	{
		//printf();
		vty = ssh_vty;
	}
	else
	{
		vty = ssh_get_session_private(session);
		ssh_vty = vty;
	}
    if(vty)
    {
    	int len = 0;
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        va_list va;
        va_start(va, fmt);
        len = vsnprintf(buffer, sizeof(buffer), fmt, va);
        va_end(va);
        if(buffer[len-1] == '\n')
        	buffer[len-1] = '\0';
        if(len)
        	vty_out(vty, "%s%s", buffer, VTY_NEWLINE);
        return OK;
    }
    else
    {
    	int len = 0;
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        va_list va;
        va_start(va, fmt);
        len = vsnprintf(buffer, sizeof(buffer), fmt, va);
        va_end(va);
    	zlog_debug(ZLOG_UTILS, "SSH %s", buffer);
    }
    return OK;
}

BOOL sshd_acl_action(ssh_config_t *ssh, ssh_session session)
{
	return TRUE;
}

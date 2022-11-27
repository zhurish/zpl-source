/*
 * ssh_util.c
 *
 *  Created on: Nov 1, 2018
 *      Author: zhurish
 */




#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#include "ssh_api.h"
#include "ssh_util.h"

/*
int ssh_set_log_level(int level)
int ssh_set_log_callback(ssh_logging_callback cb)
int ssh_set_log_userdata(void *data)
*/

void ssh_log_callback_func(zpl_uint32 priority,
        const char *function,
        const char *buffer,
        void *userdata)
{
	switch(priority)
	{
	case ZLOG_LEVEL_ERR:
		zlog_err(MODULE_SSH, "SSH %s", buffer);
		break;
	case ZLOG_LEVEL_WARNING:
		zlog_warn(MODULE_SSH, "SSH %s", buffer);
		break;
	case ZLOG_LEVEL_INFO:
		zlog_info(MODULE_SSH, "SSH %s", buffer);
		break;
	case ZLOG_LEVEL_NOTICE:
		zlog_notice(MODULE_SSH, "SSH %s", buffer);
		break;
	case ZLOG_LEVEL_DEBUG:
		zlog_debug(MODULE_SSH, "SSH %s", buffer);
		break;
	case ZLOG_LEVEL_TRAP:
		zlog_trap(MODULE_SSH, "SSH %s", buffer);
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
	struct vty *vty = session->ssh_cli;
    if(vty)
    {
    	return ipstack_fd(vty->fd);
    }
    return -1;
}

int ssh_stdout_get(ssh_session session) 
{
	struct vty *vty = session->ssh_cli;
	if (vty)
	{
		return ipstack_fd(vty->wfd);
	}
	return -1;
}
int ssh_get_input(ssh_session session, int fd, char *buf, zpl_uint32 len)
{
	int c = 0;
	zpl_uint32 i = 0;
	while (session && session->ssh_cli)
	{

		c = vty_getc_input(session->ssh_cli);
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



int ssh_printf(ssh_session session, const char *fmt,...)
{
    struct vty * vty = NULL;
	if(session)
	{
		vty = session->ssh_cli;
	}
    if(vty)
    {
    	zpl_uint32 len = 0;
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
    	zpl_uint32 len = 0;
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        va_list va;
        va_start(va, fmt);
        len = vsnprintf(buffer, sizeof(buffer), fmt, va);
        va_end(va);
    	zlog_debug(MODULE_SSH, "SSH %s", buffer);
    }
    return OK;
}

zpl_bool sshd_acl_action(ssh_config_t *ssh, ssh_session session)
{
	return zpl_true;
}

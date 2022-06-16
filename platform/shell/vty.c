/*
 * Virtual terminal [aka TeletYpe] interface routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "linklist.h"
#include "zmemory.h"


#ifdef ZPL_SHELL_MODULE
#include "cli_node.h"
#include "buffer.h"
#include "vector.h"
#include "command.h"
#endif
#include "log.h"
#include "str.h"
#include "prefix.h"
#include "host.h"
#include "network.h"
#include "sockunion.h"
#include "sockopt.h"

#include "eloop.h"
#include "thread.h"
#ifdef ZPL_IP_FILTER
#include "filter.h"	
#endif
#include "vty.h"
#include "vty_user.h"

#include <arpa/telnet.h>
#include <termios.h>
#include "sys/wait.h"
#ifdef ZPL_ACTIVE_STANDBY
#include "ipcstandby.h"
#endif

#define CONTROL(X) ((X) - '@')
#define VTY_NORMAL 0
#define VTY_PRE_ESCAPE 1 /* Esc seen */
#define VTY_ESCAPE 2	 /* ANSI terminal escape (Esc-[) seen */
#define VTY_LITERAL 3	 /* Next zpl_char taken as literal */
#ifndef CONTROL
#define CONTROL(X) ((X) - '@')
#endif

static void vty_event(enum vtyevent, zpl_socket_t, struct vty *);
static int vty_flush_handle(struct vty *vty, zpl_socket_t vty_sock);
static void vty_kill_line_from_beginning(struct vty *);
static void vty_redraw_line(struct vty *);
#ifdef ZPL_IPCOM_MODULE
static int cli_telnet_task_init(void);
static int cli_telnet_task_exit(void);
#endif

static int cli_console_task_init(void);
static int cli_console_task_exit(void);

/* Extern host structure from command.c */
static struct tty_com cli_tty_com =
	{
		.devname = "/dev/ttyS0",
		.speed = 115200,
		.databit = TTY_DATA_8BIT,
		.stopbit = TTY_STOP_1BIT,
		.parity = TTY_PARITY_NONE,
		.flow_control = TTY_FLOW_CTL_NONE,
};

struct module_list module_list_console =
{
		.module = MODULE_CONSOLE,
		.name = "CONSOLE\0",
		.module_init = vty_init,
		.module_exit = NULL,
		.module_task_init = cli_console_task_init,
		.module_task_exit = cli_console_task_exit,
		.module_cmd_init = NULL,
		.taskid = 0,
		.flags = 0,
};

struct module_list module_list_telnet =
	{
		.module = MODULE_TELNET,
		.name = "TELNET\0",
		.module_init = vty_init,
		.module_exit = NULL,
#ifdef ZPL_IPCOM_MODULE
		.module_task_init = cli_telnet_task_init,
		.module_task_exit = cli_telnet_task_exit,
#else
		.module_task_init = NULL,
		.module_task_exit = NULL,
#endif	
		.module_cmd_init = NULL,
		.taskid = 0,
		.flags = 0,
};
/*******************************************************************************/
cli_shell_t cli_shell = 
{
	.init = 0,
	.mutex = NULL,
};	

static const char telnet_backward_char = 0x08;
static const char telnet_space_char = ' ';
/*******************************************************************************/
/*******************************************************************************/
static void vty_buf_assert(struct vty *vty)
{
	assert(vty->cp <= vty->length);
	assert(vty->length < vty->max);
	assert(vty->buf[vty->length] == '\0');
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
#ifdef CMD_MODIFIER_STR
static int vty_output_filter_key(struct out_filter *out_filter, const char *fmt, int len)
{
	int ret = 1;
	if(out_filter->filter_type == VTY_FILTER_NONE)
		ret = 1;
	else if(out_filter->filter_type == VTY_FILTER_BEGIN)
	{
		if(out_filter->key_flags == VTY_FILTER_BEGIN)
		{
			ret = 1;
		}
		else
		{
			if(out_filter->filter_key == NULL)
				ret = 1;
			else
			{
				if(fmt && strstr(fmt, out_filter->filter_key))
				{
					out_filter->key_flags = VTY_FILTER_BEGIN;
					ret = 1;
				}
				else
					ret = 0;
			}
		}	
	}
	else if(out_filter->filter_type == VTY_FILTER_INCLUDE)
	{
		if(out_filter->filter_key == NULL)
			ret = 1;
		else 
		{
			if(fmt && strstr(fmt, out_filter->filter_key))
				ret = 1;
			else
				ret = 0;
		}	
	}
	else if(out_filter->filter_type == VTY_FILTER_EXCLUDE)
	{
		if(out_filter->filter_key == NULL)
			ret = 1;
		else 
		{
			if(fmt && strstr(fmt, out_filter->filter_key))
				ret = 0;
			else
				ret = 1;
		}
	}
	else if(out_filter->filter_type == VTY_FILTER_REDIRECT)
	{
		if(out_filter->redirect_fd)
		{
			write(out_filter->redirect_fd, fmt, len);
		}
		ret = 0;
	}
	return ret;	
}

static int vty_output_filter(struct vty *vty, const char *fmt, int len)
{
	if(vty->type == VTY_FILE)  
		return 1;
	if(vty->out_filter.filter_type == VTY_FILTER_NONE)
		return 1;
	return vty_output_filter_key(&vty->out_filter, fmt,  len);
}

int out_filter_set(struct vty *vty, const char *key, enum out_filter_type filter_type)
{
	if(vty->out_filter.redirect_fd)
	{
		close(vty->out_filter.redirect_fd);
		sync();
		vty->out_filter.redirect_fd = 0;
	}
	if(vty->out_filter.filter_key)
	{
		free(vty->out_filter.filter_key);
		vty->out_filter.filter_key = NULL;
	}
	if(key)
		vty->out_filter.filter_key = strdup(key);
	vty->out_filter.filter_type = filter_type;
	vty->out_filter.key_flags = 0;
	vty->out_filter.redirect_fd = 0;
	if(VTY_FILTER_REDIRECT == filter_type)
	{
		if(os_file_access(vty->out_filter.filter_key) == OK)
		{
			remove(vty->out_filter.filter_key);
			sync();
		}
		vty->out_filter.redirect_fd = open(vty->out_filter.filter_key, O_CREAT|O_RDWR);
	}
	return 0;
}

static enum out_filter_type out_filter_slpit(char *buf, char *key)
{
	enum out_filter_type filter_type = VTY_FILTER_NONE;
	if(strstr(buf, "show"))
	{
		int i = 0;
		char *cp = NULL;
		char filter_key[256];
		memset(filter_key, '\0', sizeof(filter_key));
		cp = strstr(buf, "|");
		if(cp)
		{
			cp++;
			if(strstr(buf, "begin"))
			{
				filter_type = VTY_FILTER_BEGIN;
				cp = strstr(buf, "begin");
				cp += 5;
			}
			else if(strstr(buf, "include"))
			{
				filter_type = VTY_FILTER_INCLUDE;
				cp = strstr(buf, "include");
				cp += 7;
			}
			else if(strstr(buf, "exclude"))
			{
				filter_type = VTY_FILTER_EXCLUDE;
				cp = strstr(buf, "exclude");
				cp += 7;
			}
			while (isspace((int)*cp) && *cp != '\0')
				cp++;
			while (*cp != '\0')
			{
				key[i++] = *cp;
				cp++;
			}
		}
		else 
		{
			cp = strstr(buf, ">");
			if(cp)
			{
				cp++;
				filter_type = VTY_FILTER_REDIRECT;
				while (isspace((int)*cp) && *cp != '\0')
					cp++;
				while (*cp != '\0')
				{
					key[i++] = *cp;
					cp++;
				}
			}	
		}
		cp = NULL;
		cp = strstr(buf, "|");
		if(cp)
		{
			while (*cp != '\0')
			{
				*cp = '\0';
				cp++;
			}
		}
		else
		{
			cp = strstr(buf, ">");
			if(cp)
			{
				while (*cp != '\0')
				{
					*cp = '\0';
					cp++;
				}
			}
		}
	}
	return filter_type;
}
#else
static int vty_output_filter(struct vty *vty, const char *fmt, int len)
{
	return 1;
}
#endif

int vty_result_out(struct vty *vty, const char *format, ...)
{
	va_list args;
	if(vty->type == VTY_STABDVY)
		return 0;
	os_bzero(vty->result_msg, sizeof(vty->result_msg));
	va_start(args, format);
	vty->result_len = vsnprintf(vty->result_msg, sizeof(vty->result_msg), format, args);
	va_end(args);
	return vty->result_len;
}
/*******************************************************************************/
/* Sanity/safety wrappers around access to vty->buf */
static void vty_buf_put(struct vty *vty, zpl_char c)
{
	vty_buf_assert(vty);
	if(vty->type == VTY_STABDVY)
		return;	
	vty->buf[vty->cp] = c;
	vty->buf[vty->max - 1] = '\0';
}

/* VTY standard output function. */
int vty_out(struct vty *vty, const char *format, ...)
{
	va_list args;
	zpl_uint32 len = 0;
	zpl_size_t size = 4096;
	zpl_char buf[4096];
	zpl_char *p = NULL;
	os_bzero(buf, sizeof(buf));
	if(vty->type == VTY_STABDVY)
		return 0;
	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	/* Initial buffer is not enough.  */
	if (len < 0 || len >= size)
	{
		while (1)
		{
			if (len > -1)
				size = len + 1;
			else
				size = size * 2;

			p = XREALLOC(MTYPE_VTY_OUT_BUF, p, size);
			if (!p)
				return -1;

			va_start(args, format);
			len = vsnprintf(p, size, format, args);
			va_end(args);

			if (len > -1 && len < size)
				break;
		}
	}
	if (!p)
		p = buf;

	if (vty_shell(vty))
	{
		if(vty_output_filter(vty, p, len))
		{
			fwrite(p, len, 1, stdout);
			fflush(stdout);
		}
	}
	else
	{
		if (vty->vty_output)
		{
			if(vty_output_filter(vty, p, len))
				len = vty->vty_output(vty->p_output, p, len);
		}
		else
		{
			if(vty_output_filter(vty, p, len))
			{
				if (vty->ansync)
				{
					ipstack_write(vty->wfd, p, len);
				}
				else
				{
					/* Pointer p must point out buffer. */
					if (vty->obuf)
						buffer_put(vty->obuf, (zpl_uchar *)p, len);
				}
			}
		}
	}
	/* If p is not different with buf, it is allocated buffer.  */
	if (p != buf)
		XFREE(MTYPE_VTY_OUT_BUF, p);

	return len;
}

int vty_sync_out(struct vty *vty, const char *format, ...)
{
	va_list args;
	zpl_uint32 len = 0;
	zpl_size_t size = 4096;
	zpl_char buf[4096];
	zpl_char *p = NULL;
	os_bzero(buf, sizeof(buf));
	if(vty->type == VTY_STABDVY)
		return 0;	
	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	/* Initial buffer is not enough.  */
	if (len < 0 || len >= size)
	{
		while (1)
		{
			if (len > -1)
				size = len + 1;
			else
				size = size * 2;
			p = XREALLOC(MTYPE_VTY_OUT_BUF, p, size);
			if (!p)
				return -1;
			va_start(args, format);
			len = vsnprintf(p, size, format, args);
			va_end(args);
			if (len > -1 && len < size)
				break;
		}
	}
	/* When initial buffer is enough to store all output.  */
	if (!p)
		p = buf;
	if (vty_shell(vty))
	{
		if(vty_output_filter(vty, p, len))
		{
			fwrite(p, len, 1, stdout);
			fflush(stdout);
		}
	}
	else
	{
		/* Try to write to initial buffer.  */

		if (vty->vty_output)
		{
			if(vty_output_filter(vty, p, len))
				len = vty->vty_output(vty->p_output, p, len);
		}
		else
		{
			if(vty_output_filter(vty, p, len))
			{
				if (vty_login_type(vty) >= VTY_LOGIN_TELNET)
					ipstack_write(vty->wfd, p, len);
				else
				{
					if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
					{
						 tcflush(vty->wfd._fd, TCIOFLUSH);
					}
					ipstack_write(vty->wfd, p, len);
					if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
					{
						 tcdrain(vty->wfd._fd);
					}
				}
			}
		}
	}
	if (p != buf)
		XFREE(MTYPE_VTY_OUT_BUF, p);
	return len;
}


static int vty_log_out(struct vty *vty, const char *level,
					   const char *proto_str, const char *format, zlog_timestamp_t ctl,
					   va_list va, const char *file, const char *func, const zpl_uint32 line)
{
	int ret;
	zpl_uint32 len = 0;
	zpl_char buf[1024];
	if(vty->type == VTY_STABDVY)
		return 0;
	os_memset(buf, 0, sizeof(buf));
	len += quagga_timestamp(ctl, buf, sizeof(buf));

	if ((uint)(len + 1) >= sizeof(buf))
		return -1;

	if (len)
	{
		buf[len++] = ' ';
		buf[len] = '\0';
	}
	if (level)
		ret = snprintf(buf + len, sizeof(buf) - len, "%s: %s: ", level,
					   proto_str);
	else
		ret = snprintf(buf + len, sizeof(buf) - len, "%s: ", proto_str);

	if (file)
	{
		zpl_char *bk = strrchr(file, '/');
		len += ret;
		ret = 0;
		if (bk)
			ret = snprintf(buf + len, sizeof(buf) - len, "[LWP=%d](%s ", os_task_gettid(), bk);
		else
			ret = snprintf(buf + len, sizeof(buf) - len, "[LWP=%d](%s ", os_task_gettid(), file);

		len += ret;

		if (func)
			ret = snprintf(buf + len, sizeof(buf) - len, "->%s ", func);
		len += ret;
		ret = snprintf(buf + len, sizeof(buf) - len, "line %d:)", line);
	}

	if ((ret < 0) || ((zpl_size_t)(len + ret) >= sizeof(buf)))
		return -1;

	len += ret;

	ret = vsnprintf(buf + len, sizeof(buf) - len, format, va);

	if ((ret < 0) || ((zpl_size_t)(len + ret + 2) > sizeof(buf)))
		return -1;

	len += ret;
	if (vty->type == VTY_TERM)
		buf[len++] = '\r';
	buf[len++] = '\n';

	if (vty_shell(vty))
	{
		len = fprintf(stdout, "%s", buf);
		fflush(stdout);
		return 0;
	}
	if (vty->vty_output)
	{
		len = vty->vty_output(vty->p_output, buf, len);
		return 0;
	}
	if (vty_login_type(vty) >= VTY_LOGIN_TELNET)
		ret = ipstack_write(vty->wfd, buf, len);
	else
	{
		if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
		{
			 tcflush(vty->wfd._fd, TCIOFLUSH);
		}
		ret = ipstack_write(vty->wfd, buf, len);
		if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
		{
			 tcdrain(vty->wfd._fd);
		}
	}
	if (ret < 0)
	{
		if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			/* Kernel buffer is full, probably too much debugging output, so just
			 drop the data and ignore. */
			return -1;
		/* Fatal I/O error. */
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(MODULE_DEFAULT,
				  "%s: write failed to vty client fd %d, closing: %s", __func__,
				  vty->fd, ipstack_strerror(ipstack_errno));
		if (vty->obuf)
			buffer_reset(vty->obuf);
		/* cannot call vty_close, because a parent routine may still try to access the vty struct */
		vty->status = VTY_CLOSE;
		vty_close(vty);
		return -1;
	}
	return 0;
}

/* Output current time to the vty. */
void vty_time_print(struct vty *vty, zpl_bool cr)
{
	zpl_char buf[QUAGGA_TIMESTAMP_LEN];
	if(vty->type == VTY_STABDVY)
		return ;
	if (quagga_timestamp(ZLOG_TIMESTAMP_DATE, buf, sizeof(buf)) == 0)
	{
		zlog(MODULE_DEFAULT, ZLOG_LEVEL_INFO, "quagga_timestamp error");
		return;
	}
	if (cr)
		vty_out(vty, "%s\n", buf);
	else
		vty_out(vty, "%s ", buf);

	return;
}

/* Say hello to vty interface. */
void vty_hello(struct vty *vty)
{
	if(vty->type == VTY_STABDVY)
		return ;	
	if (_global_host.motdfile)
	{
		FILE *f;
		zpl_char buf[4096];

		f = fopen(_global_host.motdfile, "r");
		if (f)
		{
			while (fgets(buf, sizeof(buf), f))
			{
				zpl_char *s;
				/* work backwards to ignore trailling isspace() */
				for (s = buf + strlen(buf);
					 (s > buf) && isspace((int)*(s - 1)); s--)
					;
				*s = '\0';
				vty_out(vty, "%s%s", buf, VTY_NEWLINE);
			}
			fclose(f);
		}
		else
			vty_out(vty, "MOTD file not found%s", VTY_NEWLINE);
	}
	else if (_global_host.motd)
		vty_out(vty, "%s", _global_host.motd);
}

/* Put out prompt and wait input from user. */
const char *vty_prompt(struct vty *vty)
{
	struct utsname names;
	const char *hostname;
	static char buf[100];

	hostname = _global_host.name;
	if (!hostname)
	{
		uname(&names);
		hostname = names.nodename;
	}

	memset(buf, 0, sizeof(buf));

	if (os_strlen(vty->prompt))
		snprintf(buf, sizeof buf, cmd_prompt(vty->node), hostname, vty->prompt);
	else
		snprintf(buf, sizeof buf, cmd_prompt(vty->node), hostname);
	if (vty_login_type(vty) != VTY_LOGIN_VTYSH_STDIO)
		vty_out(vty, "%s", buf);

	/*
	if (os_strlen(vty->prompt))
		vty_out(vty, cmd_prompt(vty->node), hostname, vty->prompt);
	else
		vty_out(vty, cmd_prompt(vty->node), hostname);
	*/
	return (const char *)buf;
}

/* Send WILL TELOPT_ECHO to remote server. */
static void vty_will_echo(struct vty *vty)
{
	zpl_uchar cmd[] =
		{IAC, WILL, TELOPT_ECHO, '\0'};
	vty_out(vty, "%s", cmd);
}

/* Make suppress Go-Ahead telnet option. */
static void vty_will_suppress_go_ahead(struct vty *vty)
{
	zpl_uchar cmd[] =
		{IAC, WILL, TELOPT_SGA, '\0'};
	vty_out(vty, "%s", cmd);
}

/* Make don't use linemode over telnet. */
static void vty_dont_linemode(struct vty *vty)
{
	zpl_uchar cmd[] =
		{IAC, DONT, TELOPT_LINEMODE, '\0'};
	vty_out(vty, "%s", cmd);
}

/* Use window size. */
static void vty_do_window_size(struct vty *vty)
{
	zpl_uchar cmd[] =
		{IAC, DO, TELOPT_NAWS, '\0'};
	vty_out(vty, "%s", cmd);
}

/* Allocate new vty struct. */
struct vty *
vty_new()
{
	struct vty *new = XCALLOC(MTYPE_VTY, sizeof(struct vty));

	new->obuf = buffer_new(0); /* Use default buffer size. */
	new->buf = XCALLOC(MTYPE_VTY, VTY_BUFSIZ);
	new->max = VTY_BUFSIZ;
	new->pid = 0;
	new->pthd = 0;
	new->trapping = 1;
	new->monitor = 1;
	new->cancel = 0;
	return new;
}

int vty_free(struct vty *vty)
{
	if (vty)
	{
		if (vty->obuf)
			buffer_free(vty->obuf);
		if (vty->buf)
			XFREE(MTYPE_VTY, vty->buf);
		XFREE(MTYPE_VTY, vty);
	}
	return OK;
}
/* Authentication of vty */
static void vty_auth(struct vty *vty, zpl_char *buf)
{
	enum node_type next_node = 0;
	switch (vty->node)
	{
	case LOGIN_NODE:
		vty_user_setting(vty, buf);
		vty->node = AUTH_NODE;
		return;
	case AUTH_NODE:
		if (vty_user_authentication(vty, buf) == CMD_WARNING)
		{
			vty->node = LOGIN_NODE;
			vty->fail++;
			if (vty->fail >= 3)
			{
				// if (vty->node == AUTH_NODE || vty->node == LOGIN_NODE)
				{
					vty_out(vty, "%% Bad passwords, too many failures!%s",
							VTY_NEWLINE);
					vty->status = VTY_CLOSE;
					vty->reload = zpl_true;
				}
			}
			break;
		}

		next_node = VIEW_NODE;
		vty->fail = 0;
		vty->node = next_node; /* Success ! */
		break;
	case AUTH_ENABLE_NODE:
		if (vty_user_authentication(vty, buf) == CMD_WARNING)
		{
			vty->fail++;
			if (vty->fail >= 3)
			{
				// if (vty->node == AUTH_NODE || vty->node == LOGIN_NODE)
				{
					vty->fail = 0;
					vty_out(vty,
							"%% Bad enable passwords, too many failures!%s",
							VTY_NEWLINE);
					vty->node = VIEW_NODE;
				}
			}
			break;
		}
		next_node = ENABLE_NODE;
		vty->fail = 0;
		vty->node = next_node; /* Success ! */
		break;
	}
}

/* Command execution over the vty interface. */
int vty_command(struct vty *vty, zpl_char *buf)
{
	int ret;
	vector vline;
	const char *protocolname;
	zpl_char *cp = NULL;
#ifdef CMD_MODIFIER_STR	
    enum out_filter_type filter_type = VTY_FILTER_NONE; 	
	zpl_char filter_key[256];
#endif
	if (cli_shell.mutex)
		os_mutex_lock(cli_shell.mutex, OS_WAIT_FOREVER);
#ifdef CMD_MODIFIER_STR
	if(strstr(buf, "show"))
	{
		memset(filter_key, '\0', sizeof(filter_key));
		filter_type = out_filter_slpit(buf, filter_key);		
	}
#endif	
	/*
	 * Log non empty command lines
	 */
	if (cli_shell.do_log_commands)
		cp = buf;
	if (cp != NULL)
	{
		/* Skip white spaces. */
		while (isspace((int)*cp) && *cp != '\0')
			cp++;
	}
	if (cp != NULL && *cp != '\0')
	{
		unsigned i;
		zpl_char vty_str[VTY_BUFSIZ];
		zpl_char prompt_str[VTY_BUFSIZ];

		/* format the base vty info */
		snprintf(vty_str, sizeof(vty_str), "vty[??]@%s", vty->address);
		if (vty)
		{
			for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
			{
				if (vty == vector_slot(cli_shell.vtyvec, i) && (!vty->cancel))
				{
					snprintf(vty_str, sizeof(vty_str), "vty[%d]@%s", i,
							 vty->address);
					break;
				}
			}
		}
		/* format the prompt */
		snprintf(prompt_str, sizeof(prompt_str), cmd_prompt(vty->node),
				 vty_str);

		/* now log the command */
		zlog(MODULE_DEFAULT, ZLOG_LEVEL_ERR, "%s%s", prompt_str, buf);
	}

	if (vty_user_authorization(vty, buf) != CMD_SUCCESS)
	{
		vty_out(vty, "%% %s not authorization for %s %s", buf, vty->username,
				VTY_NEWLINE);
		if (cli_shell.mutex)
			os_mutex_unlock(cli_shell.mutex);
		return CMD_WARNING;
	}

	/* Split readline string up into the vector */
	vline = cmd_make_strvec(buf);

	if (vline == NULL)
	{
		if (cli_shell.mutex)
			os_mutex_unlock(cli_shell.mutex);
		return CMD_SUCCESS;
	}
#ifdef CONSUMED_TIME_CHECK
	{
		struct timeval before;
		struct timeval after;
		zpl_ulong realtime, cputime;

		os_get_monotonic(&before);
#endif /* CONSUMED_TIME_CHECK */
		cli_shell.cli_shell_vty = vty;
#ifdef CMD_MODIFIER_STR
		if(strstr(buf, "show"))
			out_filter_set(vty, filter_key, filter_type);
#endif			
		ret = cmd_execute_command(vline, vty, NULL, 0);
#ifdef CMD_MODIFIER_STR		
		if(strstr(buf, "show"))
			out_filter_set(vty, NULL, VTY_FILTER_NONE);
#endif			
		cli_shell.cli_shell_vty = NULL;
		/* Get the name of the protocol if any */
		if (zlog_default)
			protocolname = zlog_proto_names(zlog_default->protocol);
		else
			protocolname = zlog_proto_names(MODULE_DEFAULT);

#ifdef CONSUMED_TIME_CHECK
		os_get_monotonic(&after);
		if ((realtime = thread_consumed_time(&after, &before, &cputime)) >
			CONSUMED_TIME_CHECK)
		{
			/* Warn about CPU hog that must be fixed. */
			if (!(strstr(buf, "tftp") || strstr(buf, "ftp") ||
				  strstr(buf, "ping") || strstr(buf, "traceroute") ||
				  strstr(buf, "ymodem") || strstr(buf, "xmodem") || strstr(buf, "esp update") ||
				  strstr(buf, "scp")))
				zlog_warn(MODULE_DEFAULT,
						  "SLOW COMMAND: command took %lums (cpu time %lums): %s",
						  realtime / 1000, cputime / 1000, buf);
		}
	}
#endif /* CONSUMED_TIME_CHECK */
#ifdef ZPL_ACTIVE_STANDBY
	if(ret == CMD_SUCCESS && vty->node > AUTH_ENABLE_NODE)
	{
		ipcstandby_execue_clicmd(buf, vty->length);
	}
#endif
	if (ret != CMD_SUCCESS)
		switch (ret)
		{
		case CMD_WARNING:
			if(vty->result_len)
				vty_out(vty, "%% Warning %s.%s", vty->result_msg, VTY_NEWLINE);
			else
				vty_out(vty, "%% Warning...%s", VTY_NEWLINE);
			break;
		case CMD_ERR_AMBIGUOUS:
			vty_out(vty, "%% Ambiguous command.%s", VTY_NEWLINE);
			break;
		case CMD_ERR_NO_MATCH:
			vty_out(vty, "%% [%s] Unknown command: %s%s", protocolname, buf,
					VTY_NEWLINE);
			break;
		case CMD_ERR_INCOMPLETE:
			vty_out(vty, "%% Command incomplete.%s", VTY_NEWLINE);
			break;
		}
	cmd_free_strvec(vline);
	if (cli_shell.mutex)
		os_mutex_unlock(cli_shell.mutex);
	return ret;
}

/* Basic function to write buffer to vty. */
static void vty_write(struct vty *vty, const char *buf, zpl_size_t nbytes)
{
	if ((vty->node == AUTH_NODE) || (vty->node == AUTH_ENABLE_NODE))
		return;

	if (vty->ansync && !vty_shell(vty))
	{
		ipstack_write(vty->wfd, buf, nbytes);
	}
	else
	{
		/* Should we do buffering here ?  And make vty_flush (vty) ? */
		buffer_put(vty->obuf, buf, nbytes);
	}
}

/* Basic function to insert character into vty. */
void vty_self_insert(struct vty *vty, zpl_char c)
{
	zpl_uint32 i;
	zpl_size_t length;

	vty_buf_assert(vty);

	/* length is sans nul, max is with */
	if (vty->length + 1 >= vty->max)
		return;

	length = vty->length - vty->cp;
	memmove(&vty->buf[vty->cp + 1], &vty->buf[vty->cp], length);
	vty->length++;
	vty->buf[vty->length] = '\0';

	vty_buf_put(vty, c);

	vty_write(vty, &vty->buf[vty->cp], length + 1);
	for (i = 0; i < length; i++)
		vty_write(vty, &telnet_backward_char, 1);

	vty->cp++;

	vty_buf_assert(vty);
}

/* Self insert character 'c' in overwrite mode. */
static void vty_self_insert_overwrite(struct vty *vty, zpl_char c)
{
	vty_buf_assert(vty);

	if (vty->cp == vty->length)
	{
		vty_self_insert(vty, c);
		return;
	}

	vty_buf_put(vty, c);
	vty->cp++;

	vty_buf_assert(vty);

	vty_write(vty, &c, 1);
}

/**
 * Insert a string into vty->buf at the current cursor position.
 *
 * If the resultant string would be larger than VTY_BUFSIZ it is
 * truncated to fit.
 */
static void vty_insert_word_overwrite(struct vty *vty, zpl_char *str)
{
	vty_buf_assert(vty);

	zpl_size_t nwrite = MIN((int)strlen(str), vty->max - vty->cp - 1);
	memcpy(&vty->buf[vty->cp], str, nwrite);
	vty->cp += nwrite;
	vty->length = vty->cp;
	vty->buf[vty->length] = '\0';
	vty_buf_assert(vty);

	vty_write(vty, str, nwrite);
}

/* Forward character. */
static void vty_forward_char(struct vty *vty)
{
	vty_buf_assert(vty);

	if (vty->cp < vty->length)
	{
		vty_write(vty, &vty->buf[vty->cp], 1);
		vty->cp++;
	}

	vty_buf_assert(vty);
}

/* Backward character. */
static void vty_backward_char(struct vty *vty)
{
	vty_buf_assert(vty);

	if (vty->cp > 0)
	{
		vty->cp--;
		vty_write(vty, &telnet_backward_char, 1);
	}

	vty_buf_assert(vty);
}

/* Move to the beginning of the line. */
static void vty_beginning_of_line(struct vty *vty)
{
	while (vty->cp)
		vty_backward_char(vty);
}

/* Move to the end of the line. */
static void vty_end_of_line(struct vty *vty)
{
	while (vty->cp < vty->length)
		vty_forward_char(vty);
}

/* Print command line history.  This function is called from
 vty_next_line and vty_previous_line. */
static void vty_history_print(struct vty *vty)
{
	zpl_size_t length;

	vty_kill_line_from_beginning(vty);

	/* Get previous line from history buffer */
	length = strlen(vty->hist[vty->hp]);
	memcpy(vty->buf, vty->hist[vty->hp], length);
	vty->cp = vty->length = length;
	vty->buf[vty->length] = '\0';
	vty_buf_assert(vty);

	/* Redraw current line */
	vty_redraw_line(vty);
}

/* Show next command line history. */
static void vty_next_line(struct vty *vty)
{
	zpl_uint32 try_index;

	if (vty->hp == vty->hindex)
		return;

	/* Try is there history exist or not. */
	try_index = vty->hp;
	if (try_index == (VTY_MAXHIST - 1))
		try_index = 0;
	else
		try_index++;

	/* If there is not history return. */
	if (vty->hist[try_index] == NULL)
		return;
	else
		vty->hp = try_index;

	vty_history_print(vty);
}

/* Show previous command line history. */
static void vty_previous_line(struct vty *vty)
{
	zpl_uint32 try_index;

	try_index = vty->hp;
	if (try_index == 0)
		try_index = VTY_MAXHIST - 1;
	else
		try_index--;

	if (vty->hist[try_index] == NULL)
		return;
	else
		vty->hp = try_index;

	vty_history_print(vty);
}

/* This function redraw all of the command line character. */
static void vty_redraw_line(struct vty *vty)
{
	vty_write(vty, vty->buf, vty->length);
	vty->cp = vty->length;

	vty_buf_assert(vty);
}

/* Forward word. */
static void vty_forward_word(struct vty *vty)
{
	while (vty->cp != vty->length && vty->buf[vty->cp] != ' ')
		vty_forward_char(vty);

	while (vty->cp != vty->length && vty->buf[vty->cp] == ' ')
		vty_forward_char(vty);
}

/* Backward word without skipping training space. */
static void vty_backward_pure_word(struct vty *vty)
{
	while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
		vty_backward_char(vty);
}

/* Backward word. */
static void vty_backward_word(struct vty *vty)
{
	while (vty->cp > 0 && vty->buf[vty->cp - 1] == ' ')
		vty_backward_char(vty);

	while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
		vty_backward_char(vty);
}

/* When '^D' is typed at the beginning of the line we move to the down
 level. */
static void vty_down_level(struct vty *vty)
{
	vty_out(vty, "%s", VTY_NEWLINE);
	(*config_exit_cmd.func)(NULL, vty, 0, NULL);
	vty_prompt(vty);
	vty->cp = 0;
}

/* When '^Z' is received from vty, move down to the enable mode. */
static void vty_end_config(struct vty *vty)
{
	vty_out(vty, "%s", VTY_NEWLINE);

	if (vty->node >= CONFIG_NODE && vty->node < CMD_NODE_MAX)
	{
		vty_config_unlock(vty);
	}
	vty->node = cmd_end_node(vty);
	vty_prompt(vty);
	vty->cp = 0;
}

/* Delete a charcter at the current point. */
static void vty_delete_char(struct vty *vty)
{
	zpl_uint32 i;
	zpl_size_t size;

	if (vty->length == 0)
	{
		vty_down_level(vty);
		return;
	}

	if (vty->cp == vty->length)
		return; /* completion need here? */

	vty_buf_assert(vty);

	size = vty->length - vty->cp;

	vty->length--;
	memmove(&vty->buf[vty->cp], &vty->buf[vty->cp + 1], size - 1);
	vty->buf[vty->length] = '\0';

	if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
		return;

	vty_write(vty, &vty->buf[vty->cp], size - 1);
	vty_write(vty, &telnet_space_char, 1);

	for (i = 0; i < size; i++)
		vty_write(vty, &telnet_backward_char, 1);
}

/* Delete a character before the point. */
static void vty_delete_backward_char(struct vty *vty)
{
	if (vty->cp == 0)
		return;

	vty_backward_char(vty);
	vty_delete_char(vty);
}

/* Kill rest of line from current point. */
static void vty_kill_line(struct vty *vty)
{
	zpl_uint32 i;
	zpl_size_t size;

	size = vty->length - vty->cp;

	if (size == 0)
		return;

	for (i = 0; i < size; i++)
		vty_write(vty, &telnet_space_char, 1);
	for (i = 0; i < size; i++)
		vty_write(vty, &telnet_backward_char, 1);

	memset(&vty->buf[vty->cp], 0, size);
	vty->length = vty->cp;
	vty_buf_assert(vty);
}

/* Kill line from the beginning. */
static void vty_kill_line_from_beginning(struct vty *vty)
{
	vty_beginning_of_line(vty);
	vty_kill_line(vty);
}

/* Delete a word before the point. */
static void vty_forward_kill_word(struct vty *vty)
{
	while (vty->cp != vty->length && vty->buf[vty->cp] == ' ')
		vty_delete_char(vty);
	while (vty->cp != vty->length && vty->buf[vty->cp] != ' ')
		vty_delete_char(vty);
}

/* Delete a word before the point. */
static void vty_backward_kill_word(struct vty *vty)
{
	while (vty->cp > 0 && vty->buf[vty->cp - 1] == ' ')
		vty_delete_backward_char(vty);
	while (vty->cp > 0 && vty->buf[vty->cp - 1] != ' ')
		vty_delete_backward_char(vty);
}

/* Transpose chars before or at the point. */
static void vty_transpose_chars(struct vty *vty)
{
	zpl_char c1, c2;

	/* If length is zpl_int16 or point is near by the beginning of line then
	 return. */
	if (vty->length < 2 || vty->cp < 1)
		return;

	/* In case of point is located at the end of the line. */
	if (vty->cp == vty->length)
	{
		c1 = vty->buf[vty->cp - 1];
		c2 = vty->buf[vty->cp - 2];

		vty_backward_char(vty);
		vty_backward_char(vty);
		vty_self_insert_overwrite(vty, c1);
		vty_self_insert_overwrite(vty, c2);
	}
	else
	{
		c1 = vty->buf[vty->cp];
		c2 = vty->buf[vty->cp - 1];

		vty_backward_char(vty);
		vty_self_insert_overwrite(vty, c1);
		vty_self_insert_overwrite(vty, c2);
	}
}

/* Do completion at vty interface. */
static void vty_complete_command(struct vty *vty)
{
	zpl_uint32 i;
	int ret;
	zpl_char **matched = NULL;
	vector vline;

	if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
		return;

	if (vty->node == LOGIN_NODE)
		return;

	vline = cmd_make_strvec(vty->buf);
	if (vline == NULL)
		return;

	/* In case of 'help \t'. */
	if (isspace((int)vty->buf[vty->length - 1]))
		vector_set(vline, NULL);

	matched = cmd_complete_command_lib(vline, vty, &ret, 1);

	cmd_free_strvec(vline);

	vty_out(vty, "%s", VTY_NEWLINE);
	switch (ret)
	{
	case CMD_ERR_AMBIGUOUS:
		vty_out(vty, "%% Ambiguous command.%s", VTY_NEWLINE);
		vty_prompt(vty);
		vty_redraw_line(vty);
		break;
	case CMD_ERR_NO_MATCH:
		/* vty_out (vty, "%% There is no matched command.%s", VTY_NEWLINE); */
		vty_prompt(vty);
		vty_redraw_line(vty);
		break;
	case CMD_COMPLETE_FULL_MATCH:
		vty_prompt(vty);
		vty_redraw_line(vty);
		vty_backward_pure_word(vty);
		vty_insert_word_overwrite(vty, matched[0]);
		vty_self_insert(vty, ' ');
		XFREE(MTYPE_TMP, matched[0]);
		break;
	case CMD_COMPLETE_MATCH:
		vty_prompt(vty);
		vty_redraw_line(vty);
		vty_backward_pure_word(vty);
		vty_insert_word_overwrite(vty, matched[0]);
		XFREE(MTYPE_TMP, matched[0]);
		vector_only_index_free(matched);
		return;
		break;
	case CMD_COMPLETE_LIST_MATCH:
		for (i = 0; matched[i] != NULL; i++)
		{
			if (i != 0 && ((i % 6) == 0))
				vty_out(vty, "%s", VTY_NEWLINE);
			vty_out(vty, "%-10s ", matched[i]);
			XFREE(MTYPE_TMP, matched[i]);
		}
		vty_out(vty, "%s", VTY_NEWLINE);

		vty_prompt(vty);
		vty_redraw_line(vty);
		break;
	case CMD_ERR_NOTHING_TODO:
		vty_prompt(vty);
		vty_redraw_line(vty);
		break;
	default:
		break;
	}
	if (matched)
		vector_only_index_free(matched);
}

static void vty_describe_fold(struct vty *vty, zpl_uint32 cmd_width,
							  zpl_uint32 desc_width, struct cmd_token *token)
{
	zpl_char *buf = NULL;
	const char *cmd = NULL, *p = NULL;
	zpl_uint32 pos;

	cmd = token->cmd[0] == '.' ? token->cmd + 1 : token->cmd;

	if (desc_width <= 0)
	{
		vty_out(vty, "  %-*s  %s%s", cmd_width, cmd, token->desc, VTY_NEWLINE);
		return;
	}

	buf = XCALLOC(MTYPE_TMP, strlen(token->desc) + 1);

	for (p = token->desc; strlen(p) > desc_width; p += pos + 1)
	{
		for (pos = desc_width; pos > 0; pos--)
		{
			if (*(p + pos) == ' ')
				break;
		}
		if (pos == 0)
			break;

		strncpy(buf, p, pos);
		buf[pos] = '\0';
		vty_out(vty, "  %-*s  %s%s", cmd_width, cmd, buf, VTY_NEWLINE);

		cmd = "";
	}

	vty_out(vty, "  %-*s  %s%s", cmd_width, cmd, p, VTY_NEWLINE);

	XFREE(MTYPE_TMP, buf);
}

/* Describe matched command function. */
static void vty_describe_command(struct vty *vty)
{
	int ret;
	vector vline;
	vector describe;
	zpl_uint32 i, width, desc_width;
	struct cmd_token *token, *token_cr = NULL;

	vline = cmd_make_strvec(vty->buf);

	/* In case of '> ?'. */
	if (vline == NULL)
	{
		vline = vector_init(1);
		vector_set(vline, NULL);
	}
	else if (isspace((int)vty->buf[vty->length - 1]))
		vector_set(vline, NULL);

	describe = cmd_describe_command(vline, vty, &ret);

	vty_out(vty, "%s", VTY_NEWLINE);

	/* Ambiguous error. */
	switch (ret)
	{
	case CMD_ERR_AMBIGUOUS:
		vty_out(vty, "%% Ambiguous command.%s", VTY_NEWLINE);
		goto out;
		break;
	case CMD_ERR_NO_MATCH:
		vty_out(vty, "%% There is no matched command.%s", VTY_NEWLINE);
		goto out;
		break;
	}

	/* Get width of command string. */
	width = 0;
	for (i = 0; i < vector_active(describe); i++)
	{
		if ((token = vector_slot(describe, i)) != NULL)
		{
			zpl_uint32 len;

			if (token->cmd[0] == '\0')
				continue;

			len = strlen(token->cmd);
			if (token->cmd[0] == '.')
				len--;

			if (width < len)
				width = len;
		}
	}

	/* Get width of description string. */
	desc_width = vty->width - (width + 6);

	/* Print out description. */
	for (i = 0; i < vector_active(describe); i++)
	{
		if ((token = vector_slot(describe, i)) != NULL)
		{
			if (token->cmd[0] == '\0')
				continue;

			if (strcmp(token->cmd, command_cr) == 0)
			{
				token_cr = token;
				continue;
			}

			if (!token->desc)
				vty_out(vty, "  %-s%s",
						token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
						VTY_NEWLINE);
			else if (desc_width >= strlen(token->desc))
				vty_out(vty, "  %-*s  %s%s", width,
						token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
						token->desc, VTY_NEWLINE);
			else
				vty_describe_fold(vty, width, desc_width, token);

#if 0
			vty_out (vty, "  %-*s %s%s", width
					desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
					desc->str ? desc->str : "", VTY_NEWLINE);
#endif /* 0 */
		}
	}
	if ((token = token_cr))
	{
		if (!token->desc)
			vty_out(vty, "  %-s%s",
					token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
					VTY_NEWLINE);
		else if (desc_width >= strlen(token->desc))
			vty_out(vty, "  %-*s  %s%s", width,
					token->cmd[0] == '.' ? token->cmd + 1 : token->cmd,
					token->desc, VTY_NEWLINE);
		else
			vty_describe_fold(vty, width, desc_width, token);
	}

out:
	cmd_free_strvec(vline);
	if (describe)
		vector_free(describe);

	vty_prompt(vty);
	vty_redraw_line(vty);
}

static void vty_clear_buf(struct vty *vty)
{
	memset(vty->buf, 0, vty->max);
}

/* ^C stop current input and do not add command line to the history. */
static void vty_stop_input(struct vty *vty)
{
	vty->cp = vty->length = 0;
	vty_clear_buf(vty);
	vty_out(vty, "%s", VTY_NEWLINE);

	if (vty->node >= CONFIG_NODE && vty->node < CMD_NODE_MAX)
	{
		vty_config_unlock(vty);
	}
	vty->node = cmd_stop_node(vty);

	vty_prompt(vty);

	/* Set history pointer to the latest one. */
	vty->hp = vty->hindex;
}

/* Add current command line to the history buffer. */
static void vty_hist_add(struct vty *vty)
{
	zpl_uint32 index;

	if (vty->length == 0)
		return;

	index = vty->hindex ? vty->hindex - 1 : VTY_MAXHIST - 1;

	/* Ignore the same string as previous one. */
	if (vty->hist[index])
	{
		if (strcmp(vty->buf, vty->hist[index]) == 0)
		{
			vty->hp = vty->hindex;
			return;
		}
	}
	/* Insert history entry. */
	if (vty->hist[vty->hindex])
		XFREE(MTYPE_VTY_HIST, vty->hist[vty->hindex]);
	vty->hist[vty->hindex] = XSTRDUP(MTYPE_VTY_HIST, vty->buf);

	/* History index rotation. */
	vty->hindex++;
	if (vty->hindex == VTY_MAXHIST)
		vty->hindex = 0;

	vty->hp = vty->hindex;
}

/* #define TELNET_OPTION_DEBUG */

/* Get telnet window size. */
static int vty_telnet_option(struct vty *vty, zpl_uchar *buf, zpl_uint32 nbytes)
{
#ifdef TELNET_OPTION_DEBUG
	zpl_uint32 i;

	for (i = 0; i < nbytes; i++)
	{
		switch (buf[i])
		{
		case IAC:
			vty_out(vty, "IAC ");
			break;
		case WILL:
			vty_out(vty, "WILL ");
			break;
		case WONT:
			vty_out(vty, "WONT ");
			break;
		case DO:
			vty_out(vty, "DO ");
			break;
		case DONT:
			vty_out(vty, "DONT ");
			break;
		case SB:
			vty_out(vty, "SB ");
			break;
		case SE:
			vty_out(vty, "SE ");
			break;
		case TELOPT_ECHO:
			vty_out(vty, "TELOPT_ECHO %s", VTY_NEWLINE);
			break;
		case TELOPT_SGA:
			vty_out(vty, "TELOPT_SGA %s", VTY_NEWLINE);
			break;
		case TELOPT_NAWS:
			vty_out(vty, "TELOPT_NAWS %s", VTY_NEWLINE);
			break;
		default:
			vty_out(vty, "%x ", buf[i]);
			break;
		}
	}
	vty_out(vty, "%s", VTY_NEWLINE);

#endif /* TELNET_OPTION_DEBUG */

	switch (buf[0])
	{
	case SB:
		vty->sb_len = 0;
		vty->iac_sb_in_progress = 1;
		return 0;
		break;
	case SE:
	{
		if (!vty->iac_sb_in_progress)
			return 0;

		if ((vty->sb_len == 0) || (vty->sb_buf[0] == '\0'))
		{
			vty->iac_sb_in_progress = 0;
			return 0;
		}
		switch (vty->sb_buf[0])
		{
		case TELOPT_NAWS:
			if (vty->sb_len != TELNET_NAWS_SB_LEN)
				zlog_warn(MODULE_DEFAULT,
						  "RFC 1073 violation detected: telnet NAWS option "
						  "should ipstack_send %d characters, but we received %lu",
						  TELNET_NAWS_SB_LEN, (u_long)vty->sb_len);
			else if (sizeof(vty->sb_buf) < TELNET_NAWS_SB_LEN)
				zlog_err(MODULE_DEFAULT,
						 "Bug detected: sizeof(vty->sb_buf) %lu < %d, "
						 "too small to handle the telnet NAWS option",
						 (u_long)sizeof(vty->sb_buf), TELNET_NAWS_SB_LEN);
			else
			{
				vty->width = ((vty->sb_buf[1] << 8) | vty->sb_buf[2]);
				vty->height = ((vty->sb_buf[3] << 8) | vty->sb_buf[4]);
#ifdef TELNET_OPTION_DEBUG
				vty_out(vty, "TELNET NAWS window size negotiation completed: "
							 "width %d, height %d%s",
						vty->width, vty->height, VTY_NEWLINE);
#endif
			}
			break;
		}
		vty->iac_sb_in_progress = 0;
		return 0;
		break;
	}
	default:
		break;
	}
	return 1;
}

/* Execute current command line. */
int vty_execute(struct vty *vty)
{
	int ret;

	ret = CMD_SUCCESS;

	switch (vty->node)
	{
	case LOGIN_NODE:
	case AUTH_NODE:
	case AUTH_ENABLE_NODE:
		vty_auth(vty, vty->buf);
		break;
	default:
		ret = vty_command(vty, vty->buf);
		if (vty->type == VTY_TERM)
			vty_hist_add(vty);
		break;
	}

	/* Clear command line buffer. */
	vty->cp = vty->length = 0;
	vty_clear_buf(vty);

	if (vty->status != VTY_CLOSE)
		vty_prompt(vty);

	return ret;
}


/* Escape character command map. */
static void vty_escape_map(zpl_uchar c, struct vty *vty)
{
	switch (c)
	{
	case ('A'):
		vty_previous_line(vty);
		break;
	case ('B'):
		vty_next_line(vty);
		break;
	case ('C'):
		vty_forward_char(vty);
		break;
	case ('D'):
		vty_backward_char(vty);
		break;
	default:
		break;
	}

	/* Go back to normal mode. */
	vty->escape = VTY_NORMAL;
}

/* Quit print out to the buffer. */
static void vty_buffer_reset(struct vty *vty)
{
	buffer_reset(vty->obuf);
	vty_prompt(vty);
	vty_redraw_line(vty);
}
static void vty_ctrl_default(zpl_uint32 ctrl, struct vty *vty)
{
	//鎵ч敓鏂ゆ嫹ctrl + c閿熸枻鎷锋皭閿熸枻鎷烽敓鏂ゆ嫹閿燂拷
	// pid_t pid;//閿熸枻鎷峰墠閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鍙殑鏂ゆ嫹閿熸枻鎷�
	// pthread_t pthd;//閿熸枻鎷峰墠閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鍙鎷烽敓绔鎷�
	//	if( (vty->pid == 0)&&(vty->pthd == 0) )
	//		return 0;
	switch (ctrl)
	{
	case CONTROL('A'):
	case CONTROL('B'):
		break;
	case CONTROL('C'):
		if (vty->node <= AUTH_NODE)
		{
			kill(getpid(), SIGTERM);
			// vty_terminate ();
			// exit(1);
		}
		if (vty->pthd)
		{
			if (pthread_cancel(vty->pthd) == 0)
			{
				if (pthread_join(vty->pthd, 0) == 0)
					vty->pthd = 0;
			}
		}
		if (vty->pid)
		{
			if (kill(vty->pid, SIGKILL) == 0)
			{
				int status = 0;
				if (waitpid(vty->pid, &status, WNOHANG))
					vty->pthd = 0;
			}
		}
		// vty_out (vty, "ctrl + c%s", VTY_NEWLINE);
		break;
	case CONTROL('D'):
		// vty_out (vty, "ctrl + d%s", VTY_NEWLINE);
		break;
	case CONTROL('Z'):
		if (vty->node <= AUTH_NODE)
		{
			kill(getpid(), SIGTERM);
			// pthread_kill(pstTcb->taskId, SIGSTOP);
			// vty_terminate ();
			// exit(1);
		}
		if (vty->pthd)
		{
			if (pthread_cancel(vty->pthd) == 0)
			{
				if (pthread_join(vty->pthd, 0) == 0)
					vty->pthd = 0;
			}
		}
		if (vty->pid)
		{
			if (kill(vty->pid, SIGKILL) == 0)
			{
				int status = 0;
				if (waitpid(vty->pid, &status, WNOHANG))
					vty->pthd = 0;
			}
		}
		// vty_out (vty, "ctrl + z%s", VTY_NEWLINE);
		break;
	case CONTROL('E'):
	case CONTROL('F'):
	case CONTROL('H'):
	case 0x7f:
	case CONTROL('K'):
	case CONTROL('N'):
	case CONTROL('P'):
	case CONTROL('T'):
	case CONTROL('U'):
	case CONTROL('W'):
		break;
	default:
		break;
	}
}

int vty_getc_input(struct vty *vty)
{
	if (is_os_stack(vty->fd) || os_memcmp(vty->address, "stdin", os_strlen("stdin")))
	{
		return getchar();
	}
	/*	if (vty->fd_type == OS_STACK
				|| os_memcmp(vty->address, "console", os_strlen("console")))
		{
			return getchar();
		}*/
	else
	{
		//		int ret;
		zpl_uint32 nbytes;
		zpl_uchar buf[VTY_READ_BUFSIZ];
		while (1)
		{
			if (is_os_stack(vty->fd) || os_memcmp(vty->address, "console", os_strlen("console")))
				nbytes = read(vty->fd._fd, buf, VTY_READ_BUFSIZ);
			else
				nbytes = ipstack_read(vty->fd, buf, VTY_READ_BUFSIZ);
			if (nbytes <= 0)
			// if ((nbytes = ipstack_read(vty->fd, buf, VTY_READ_BUFSIZ)) <= 0)
			{
				if (nbytes < 0)
				{
					if (IPSTACK_ERRNO_RETRY(ipstack_errno))
					{
						// vty_event(VTYSH_READ, vty->fd, vty);
						continue;
					}
					vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
					zlog_warn(MODULE_DEFAULT,
							  "%s: read failed on vtysh client fd %d, closing: %s",
							  __func__, vty->fd, ipstack_strerror(ipstack_errno));
					return -1;
				}
			}
			else
			{
				break;
			}
		}
		return (int)buf[0];
	}
}

int vty_read_handle(struct vty *vty, zpl_uchar *buf, zpl_uint32 len)
{
	zpl_uint32 i;
	zpl_uint32 nbytes = len;
	if (nbytes == 2)
	{
		if (buf[0] == '\n' && buf[1] == '\r')
			nbytes = 1;
		nbytes = 1;
	}

	for (i = 0; i < nbytes; i++)
	{
		if (buf[i] == IAC)
		{
			if (!vty->iac)
			{
				vty->iac = 1;
				continue;
			}
			else
			{
				vty->iac = 0;
			}
		}

		if (vty->iac_sb_in_progress && !vty->iac)
		{
			if (vty->sb_len < sizeof(vty->sb_buf))
				vty->sb_buf[vty->sb_len] = buf[i];
			vty->sb_len++;
			continue;
		}

		if (vty->iac)
		{
			/* In case of telnet command */
			int ret = 0;
			ret = vty_telnet_option(vty, buf + i, nbytes - i);
			vty->iac = 0;
			i += ret;
			continue;
		}

		if (vty->status == VTY_MORE)
		{
			switch (buf[i])
			{
			case CONTROL('C'):
			case 'q':
			case 'Q':
				vty_buffer_reset(vty);
				break;
#if 0 /* More line does not work for "show ip bgp".  */
				case '\n':
				case '\r':
				vty->status = VTY_MORELINE;
				break;
#endif
			default:
				break;
			}
			continue;
		}

		/* Escape character. */
		if (vty->escape == VTY_ESCAPE)
		{
			vty_escape_map(buf[i], vty);
			continue;
		}

		if (vty->escape == VTY_LITERAL)
		{
			vty_self_insert(vty, buf[i]);
			vty->escape = VTY_NORMAL;
			continue;
		}

		/* Pre-escape status. */
		if (vty->escape == VTY_PRE_ESCAPE)
		{
			switch (buf[i])
			{
			case '[':
				vty->escape = VTY_ESCAPE;
				break;
			case 'b':
				vty_backward_word(vty);
				vty->escape = VTY_NORMAL;
				break;
			case 'f':
				vty_forward_word(vty);
				vty->escape = VTY_NORMAL;
				break;
			case 'd':
				vty_forward_kill_word(vty);
				vty->escape = VTY_NORMAL;
				break;
			case CONTROL('H'):
			case 0x7f:
				vty_backward_kill_word(vty);
				vty->escape = VTY_NORMAL;
				break;
			default:
				vty->escape = VTY_NORMAL;
				break;
			}
			continue;
		}
		if (vty->shell_ctrl_cmd)
		{
			if (vty->shell_ctrl_cmd != NULL)
				(vty->shell_ctrl_cmd)(vty, buf[i], vty->ctrl);
		}
		else
		{
			if (cli_shell.vty_ctrl_cmd != NULL)
				(cli_shell.vty_ctrl_cmd)(buf[i], vty);
		}
		switch (buf[i])
		{
		case CONTROL('A'):
			vty_beginning_of_line(vty);
			break;
		case CONTROL('B'):
			vty_backward_char(vty);
			break;
		case CONTROL('C'):
			vty_stop_input(vty);
			break;
		case CONTROL('D'):
			vty_delete_char(vty);
			break;
		case CONTROL('E'):
			vty_end_of_line(vty);
			break;
		case CONTROL('F'):
			vty_forward_char(vty);
			break;
		case CONTROL('H'):
		case 0x7f:
			vty_delete_backward_char(vty);
			break;
		case CONTROL('K'):
			vty_kill_line(vty);
			break;
		case CONTROL('N'):
			vty_next_line(vty);
			break;
		case CONTROL('P'):
			vty_previous_line(vty);
			break;
		case CONTROL('T'):
			vty_transpose_chars(vty);
			break;
		case CONTROL('U'):
			vty_kill_line_from_beginning(vty);
			break;
		case CONTROL('V'):
			vty->escape = VTY_LITERAL;
			break;
		case CONTROL('W'):
			vty_backward_kill_word(vty);
			break;
		case CONTROL('Z'):
			vty_end_config(vty);
			break;
		case '\n':
		case '\r':
			vty_out(vty, "%s", VTY_NEWLINE);
			vty_execute(vty);
			break;
		case '\t':
			vty_complete_command(vty);
			break;
		case '?': // 63
			if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
				vty_self_insert(vty, buf[i]);
			else
				vty_describe_command(vty);
			break;
		case '\033': // ESC
			if (i + 1 < nbytes && buf[i + 1] == '[')
			{
				vty->escape = VTY_ESCAPE;
				i++;
			}
			else
				vty->escape = VTY_PRE_ESCAPE;
			break;
		default:
			if (buf[i] > 31 && buf[i] < 127)
				vty_self_insert(vty, buf[i]);
			break;
		}
	}

	return 0;
}


static int vty_read(struct eloop *thread)
{
	zpl_uint32 nbytes;
	zpl_uchar buf[VTY_READ_BUFSIZ];
	struct vty *vty = ELOOP_ARG(thread);
	vty->t_read = NULL;
	os_bzero(buf, sizeof(buf));
	nbytes = ipstack_read(vty->fd, buf, VTY_READ_BUFSIZ);
	/* Read raw data from ipstack_socket */
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTY_READ, vty->fd /*vty_sock*/, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(MODULE_DEFAULT,
					  "%s: read error on vty client fd %d, closing: %s", __func__,
					  vty->fd._fd, ipstack_strerror(ipstack_errno));
			buffer_reset(vty->obuf);
		}
		vty->status = VTY_CLOSE;
	}
	vty_read_handle(vty, buf, nbytes);

	/* Check status. */
	if (vty->status == VTY_CLOSE)
		vty_close(vty);
	else
	{
		vty_event(VTY_WRITE, vty->wfd, vty);
		vty_event(VTY_READ, vty->fd, vty);
	}
	return 0;
}


/* Flush buffer to the vty. */
static int vty_flush_handle(struct vty *vty, zpl_socket_t vty_sock)
{
	int erase;

	buffer_status_t flushrc;

	vty->t_write = NULL;

	/* Tempolary disable read thread. */
	if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
	{
		if ((vty->lines == 0) && vty->t_read)
		{
			thread_cancel(vty->t_read);
			vty->t_read = NULL;
		}
	}
	else
	{
		if ((vty->lines == 0) && vty->t_read)
		{
			eloop_cancel(vty->t_read);
			vty->t_read = NULL;
		}
	}
	/* Function execution continue. */
	erase = ((vty->status == VTY_MORE || vty->status == VTY_MORELINE));
	/* N.B. if width is 0, that means we don't know the window size. */
	if ((vty->lines == 0) || (vty->width == 0) || (vty->height == 0))
		flushrc = buffer_flush_available(vty->obuf, vty_sock);
	else if (vty->status == VTY_MORELINE)
		flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width, 1, erase,
									  0);
	else
		flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width,
									  vty->lines >= 0 ? vty->lines : vty->height, erase, 0);

	switch (flushrc)
	{
	case BUFFER_ERROR:
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(MODULE_DEFAULT,
				  "buffer_flush failed on vty client fd %d, closing", vty->wfd);
		buffer_reset(vty->obuf);
		vty_close(vty);
		return 0;
	case BUFFER_EMPTY:
		if (vty->status == VTY_CLOSE)
			vty_close(vty);
		else
		{
			vty->status = VTY_NORMAL;
			if (vty->lines == 0)
			{
				vty_event(VTY_READ, vty_sock, vty);
			}
		}
		break;
	case BUFFER_PENDING:
		/* There is more data waiting to be written. */
		vty->status = VTY_MORE;
		if (vty->lines == 0)
		{
			vty_event(VTY_WRITE, vty->wfd /*vty_sock*/, vty);
		}
		break;
	}

	return 0;
}

static int vty_flush(struct eloop *thread)
{
	struct vty *vty = ELOOP_ARG(thread);
	vty->t_write = NULL;
	vty_flush_handle(vty, vty->wfd);
	return 0;
}

/* allocate and initialise vty */
struct vty *
vty_new_init(zpl_socket_t vty_sock)
{
	struct vty *vty;

	vty = vty_new();
	vty->fd = vty_sock;
	vty->wfd = vty_sock;
	vty->type = VTY_TERM;
	vty->node = AUTH_NODE;
	vty->node = LOGIN_NODE;
	vty->privilege = CMD_VIEW_LEVEL;
	vty->fail = 0;
	vty->cp = 0;
	vty_clear_buf(vty);
	vty->length = 0;
	memset(vty->hist, 0, sizeof(vty->hist));
	vty->hp = 0;
	vty->hindex = 0;
	vector_set_index(cli_shell.vtyvec, vty_sock._fd, vty);
	vty->status = VTY_NORMAL;
	vty->lines = -1;
	vty->iac = 0;
	vty->iac_sb_in_progress = 0;
	vty->sb_len = 0;
	// ipstack_init(OS_STACK, vty->fd);
	// ipstack_init(OS_STACK, vty->wfd);
	vty->trapping = zpl_true;
	if (vty_sock._fd == STDIN_FILENO)
		vty->ansync = zpl_true;
	return vty;
}

/* Create new vty structure. */
static struct vty *
vty_create(zpl_socket_t vty_sock, union sockunion *su)
{
	zpl_char buf[SU_ADDRSTRLEN];
	struct vty *vty;

	sockunion2str(su, buf, SU_ADDRSTRLEN);

	/* Allocate new vty structure and set up default values. */
	vty = vty_new_init(vty_sock);

	/* configurable parameters not part of basic init */

	host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vty->v_timeout);
	strcpy(vty->address, buf);

	vty->node = LOGIN_NODE;

	if (_global_host.lines >= 0)
		host_config_get_api(API_GET_LINES_CMD, &vty->lines);

	/* Say hello to the world. */
	vty_hello(vty);
	if (!_global_host.no_password_check)
		vty_out(vty, "%sUser Access Verification%s%s", VTY_NEWLINE, VTY_NEWLINE,
				VTY_NEWLINE);

	/* Setting up terminal. */
	vty_will_echo(vty);
	vty_will_suppress_go_ahead(vty);

	vty_dont_linemode(vty);
	vty_do_window_size(vty);


	vty_prompt(vty);
	vty->login_type = VTY_LOGIN_TELNET;
	/* Add read/write thread. */
	vty_event(VTY_WRITE, vty_sock, vty);
	vty_event(VTY_READ, vty_sock, vty);

	return vty;
}

int vty_write_hello(struct vty *vty)
{
	vty_hello(vty);
	vty_prompt(vty);
	return OK;
}
/*****************************************************************************/
/**********************************console************************************/
#if 0
/* create vty for stdio */
static struct termios stdio_orig_termios;
static struct vty *stdio_vty = NULL;
static void (*stdio_vty_atclose)(void);

static void
vty_stdio_reset (void)
{
  if (stdio_vty)
    {
      tcsetattr (0, TCSANOW, &stdio_orig_termios);
      stdio_vty = NULL;

      if (stdio_vty_atclose)
        stdio_vty_atclose ();
      stdio_vty_atclose = NULL;
    }
}

struct vty *
vty_stdio (void (*atclose)())
{
  struct vty *vty;
  struct termios termios;

  /* refuse creating two vtys on stdio */
  if (stdio_vty)
    return NULL;

  vty = stdio_vty = vty_new_init (0);
  stdio_vty_atclose = atclose;
  vty->wfd = 1;

  /* always have stdio vty in a known _unchangeable_ state, don't want config
   * to have any effect here to make sure scripting this works as intended */
  vty->node = ENABLE_NODE;
  vty->v_timeout = 0;
  strcpy (vty->address, "console");

  if (!tcgetattr (0, &stdio_orig_termios))
    {
      termios = stdio_orig_termios;
      termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
      termios.c_oflag &= ~OPOST;
      termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
      termios.c_cflag &= ~(CSIZE | PARENB);
      termios.c_cflag |= CS8;
      tcsetattr (0, TCSANOW, &termios);
    }

  vty_prompt (vty);

  /* Add read/write thread. */
  vty_event (VTY_WRITE, 1, vty);
  vty_event (VTY_READ, 0, vty);

  return vty;
}
#endif
/*****************************************************************************/
static int vty_console_flush(struct thread *thread)
{
	struct vty *vty = THREAD_ARG(thread);
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcflush(vty->wfd._fd, TCIOFLUSH);
	}
	vty->t_write = NULL;
	vty_flush_handle(vty, vty->wfd);
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcdrain(vty->wfd._fd);
	}
	return 0;
}

static int vty_console_read(struct thread *thread)
{
	zpl_uint32 nbytes;
	zpl_uchar buf[VTY_READ_BUFSIZ];
	struct vty *vty = THREAD_ARG(thread);
	vty->t_read = NULL;
	nbytes = read(vty->fd._fd, buf, VTY_READ_BUFSIZ);
	/* Read raw data from ipstack_socket */
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTY_READ, vty->fd, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(MODULE_DEFAULT,
					  "%s: read error on vty client fd %d, closing: %s", __func__,
					  vty->fd._fd, ipstack_strerror(ipstack_errno));
			buffer_reset(vty->obuf);
		}
		vty->status = VTY_CLOSE;
	}

	vty_read_handle(vty, buf, nbytes);

	/* Check status. */
	if (vty->status == VTY_CLOSE)
	{
		vty_close(vty);
	}
	else
	{
		vty_event(VTY_WRITE, vty->wfd, vty);
		vty_event(VTY_READ, vty->fd, vty);
	}
	return 0;
}

static struct vty *vty_console_new(const char *tty, zpl_socket_t vty_sock)
{
	struct vty *vty;
	vty = vty_new_init(vty_sock);
	if (vty == NULL)
		return vty;
	vty->fd.stack = OS_STACK;
	vty->wfd.stack = OS_STACK;
	vty->login_type = tty ? VTY_LOGIN_CONSOLE : VTY_LOGIN_STDIO;
	if (vty->fd._fd == STDIN_FILENO)
	{
		vty->wfd._fd = STDOUT_FILENO;
	}
	return vty;
}

static int vty_console_wait(struct thread *thread)
{
	struct vty *vty = THREAD_ARG(thread);
	zassert(vty != NULL);
	vty->t_wait = NULL;
	if (_global_host.load != LOAD_DONE)
	{
		if (cli_shell.m_thread_master)
			vty->t_wait = thread_add_timer(cli_shell.m_thread_master,
										   vty_console_wait, vty, 1);
		return OK;
	}
	if (vty)
	{
		vty_sync_out(vty, "\r\n\r\nPlease Enter To Start Console CLI\r\n");
		if (vty_login_type(vty) == VTY_LOGIN_CONSOLE
			|| vty_login_type(vty) == VTY_LOGIN_STDIO
		)
		{
			vty->monitor = 1;
			if (cli_shell.vty->node == LOGIN_NODE)
				vty_event(VTY_STDIO_ACCEPT, vty->fd, vty);
			else
			{
				vty_event(VTY_READ, vty->fd, vty);
			}
		}
	}
	return 0;
}

static int vty_console_accept(struct thread *thread)
{
	int c;
	struct vty *vty = THREAD_ARG(thread);

	if (!vty)
		return ERROR;
	zassert(vty != NULL);
	vty->t_read = NULL;

	c = vty_getc_input(vty);

	vty_ctrl_default(c, vty);
	vty_hello(vty);
	vty_prompt(vty);

	/* Add read/write thread. */

	vty_event(VTY_WRITE, vty->wfd, vty);
	vty_event(VTY_READ, vty->fd, vty);

	vty->monitor = 1;

	return 0;
}

static void vty_console_close_cache(struct vty *vty)
{
	zassert(vty != NULL);
	if (vty)
	{
		zpl_uint32 i;
		/* Cancel threads.*/
		if (vty->t_read)
			thread_cancel(vty->t_read);
		if (vty->t_write)
			thread_cancel(vty->t_write);
		if (vty->t_timeout)
			thread_cancel(vty->t_timeout);

		vty->t_read = NULL;
		vty->t_write = NULL;
		vty->t_timeout = NULL;

		if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
		{
			 tcflush(vty->wfd._fd, TCIOFLUSH);
		}
		/* Flush buffer. */
		if (vty->obuf)
			buffer_flush_all(vty->obuf, vty->wfd);

		if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
		{
			 tcdrain(vty->wfd._fd);
		}
		/* Free command history. */
		for (i = 0; i < VTY_MAXHIST; i++)
		{
			if (vty->hist[i])
				XFREE(MTYPE_VTY_HIST, vty->hist[i]);
		}
	}
}

static void vty_console_close(struct vty *vty)
{
	zassert(vty != NULL);
	/* Check configure. */
	vty_config_unlock(vty);
	vty->ansync = zpl_true;

	vty_console_close_cache(vty);
	/* Free input buffer. */
	if (vty->obuf)
		buffer_free(vty->obuf);
	/* Unset vector. */
	vector_unset(cli_shell.vtyvec, vty->fd._fd);

	if (vty->buf)
		XFREE(MTYPE_VTY, vty->buf);

	/* Close ipstack_socket. */
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		tty_com_close(&cli_shell.ttycom);
			fprintf(stdout, "%s vty_login_type(vty)=%d  VTY_LOGIN_CONSOLE\r\n",__func__, vty_login_type(vty));
	fflush(stdout);
	}
	else
	{
			fprintf(stdout, "%s vty_login_type(vty)=%d  old_termios\r\n",__func__, vty_login_type(vty));
	fflush(stdout);
		//tcflush(vty->wfd._fd, TCIOFLUSH);
		tcsetattr(vty->fd._fd, TCSANOW, &cli_shell.ttycom.old_termios);
		cli_shell.ttycom.fd = -1;
	}
	/* OK free vty. */
	XFREE(MTYPE_VTY, vty);
}

static void vty_console_reset(struct vty *vty)
{
	vty_console_close_cache(vty);
	vty->node = LOGIN_NODE;
	host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vty->v_timeout);
	vty_event(VTY_STDIO_WAIT, vty->fd, vty);
	return OK;
}

static int vty_console_timeout(struct thread *thread)
{
	struct vty *vty = NULL;

	vty = THREAD_ARG(thread);
	vty->t_timeout = NULL;
	vty->v_timeout = 0;

	/* Clear buffer*/
	buffer_reset(vty->obuf);

	vty_sync_out(vty, "%s%sVty connection is timed out.%s%s",
				 VTY_NEWLINE, VTY_NEWLINE,
				 VTY_NEWLINE, VTY_NEWLINE);
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
		vty_console_reset(vty);
	else if (vty_login_type(vty) == VTY_LOGIN_STDIO)
	{
		vty_console_reset(vty);
	}
	else
	{
		vty->status = VTY_CLOSE;
		vty_close(vty);
	}
	return 0;
}

static int vty_stdio_attribute(void)
{
	if (cli_shell.vty == NULL)
		return -1;
	if (FD_IS_STDOUT(cli_shell.vty->fd._fd))
	{
		if (!tcgetattr(cli_shell.vty->fd._fd, &cli_shell.ttycom.old_termios))
		{
			memcpy(&cli_shell.ttycom.termios, &cli_shell.ttycom.old_termios, sizeof(struct termios));
			// cfmakeraw(&cli_shell.ttycom.termios);
			// cli_shell.ttycom.termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHOCTL |
			//									 ECHONL | ECHOPRT | ECHOKE | ICRNL);
			// cli_shell.ttycom.termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

			// cli_shell.ttycom.termios.c_oflag &= ~ONLRET;
			// cli_shell.ttycom.termios.c_cc[VMIN] = 1; /* define the minimum bytes data to be readed*/
			/*
			cli_shell.ttycom.termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
						   | INLCR | IGNCR | ICRNL | IXON);
			cli_shell.ttycom.termios.c_oflag &= ~OPOST;
			cli_shell.ttycom.termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			cli_shell.ttycom.termios.c_cflag &= ~(CSIZE | PARENB);
			cli_shell.ttycom.termios.c_cflag |= CS8;
			*/
			cfmakeraw(&cli_shell.ttycom.termios);
			//cli_shell.ttycom.termios.c_lflag |= (ISIG);
			cli_shell.ttycom.termios.c_lflag &= ~(ISIG);//忽略终端输入的CTRL+C等信号

			// cli_shell.ttycom.termios.c_lflag &= ~(ECHO | ECHONL | ICANON );
			// cli_shell.ttycom.termios.c_iflag &= ~(ICRNL);
			// cli_shell.ttycom.termios.c_lflag &= ~(ECHO | ECHONL | ICANON |ICRNL);
			/*
			cfmakeraw sets the terminal attributes as follows:
			termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
							|INLCR|IGNCR|ICRNL|IXON);
			termios_p->c_oflag &= ~OPOST;
			termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
			termios_p->c_cflag &= ~(CSIZE|PARENB);
			termios_p->c_cflag |= CS8;
			*/
			tcsetattr(cli_shell.vty->fd._fd, TCSANOW, &cli_shell.ttycom.termios);
		}
	}
	return 0;
}


static int vty_console_init(const char *tty)
{
	if (cli_shell.init == 1)
	{
		vty_tty_init(tty);
	}
	if (cli_shell.vty)
	{
		/* always have console vty in a known _unchangeable_ state, don't want config
		 * to have any effect here to make sure scripting this works as intended */
		host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &cli_shell.vty->v_timeout);

		if (tty)
			strcpy(cli_shell.vty->address, "console");
		else
			strcpy(cli_shell.vty->address, "stdin");
		if (vty_login_type(cli_shell.vty) == VTY_LOGIN_CONSOLE)
			vty_event(VTY_STDIO_WAIT, cli_shell.vty->fd, cli_shell.vty);
		else
		{
			vty_event(VTY_STDIO_WAIT, cli_shell.vty->fd, cli_shell.vty);
		}
	}
	return OK;
}

void vty_tty_init(zpl_char *tty)
{
	zpl_socket_t ttyfd;
	ipstack_init(OS_STACK, ttyfd);
#ifdef ZPL_SHRL_MODULE
	if(tty && strstr(tty, "readline"))
	{
		vtyrl_stdio_init();
		return;
	}
#endif /*ZPL_SHRL_MODULE*/
	if (tty && _global_host.console_enable)
	{
		zlog_notice(MODULE_LIB,"VTY Shell open '%s' for CLI!\r\n", tty);
		if (cli_shell.ttycom.fd <= 0)
		{
			os_memset(&cli_shell.ttycom, 0, sizeof(struct tty_com));
			os_strcpy(cli_shell.ttycom.devname, tty);

			cli_shell.ttycom.speed = cli_tty_com.speed;				  // speed bit
			cli_shell.ttycom.databit = cli_tty_com.databit;			  // data bit
			cli_shell.ttycom.stopbit = cli_tty_com.stopbit;			  // stop bit
			cli_shell.ttycom.parity = cli_tty_com.parity;			  // parity
			cli_shell.ttycom.flow_control = cli_tty_com.flow_control; // flow control

			if (tty_com_open(&cli_shell.ttycom) == OK)
				ipstack_init(OS_STACK, ttyfd);
			else
			{
				zlog_err(MODULE_LIB,"vty can not open %s(%s)\r\n", tty, strerror(ipstack_errno));
				return ERROR;
			}
		}
		ttyfd._fd = cli_shell.ttycom.fd;
	}
	else
	{
		ipstack_init(OS_STACK, ttyfd);
		ttyfd._fd = STDIN_FILENO;
	}
	if (cli_shell.init == 1)
	{
		if (cli_shell.vty == NULL)
			cli_shell.vty = vty_console_new(tty, ttyfd);

		if (cli_shell.vty)
		{
			if (VTY_LOGIN_CONSOLE == vty_login_type(cli_shell.vty))
			{
				cli_shell.vty->type = VTY_TERM;
				dup2(ttyfd._fd, STDERR_FILENO);
				dup2(ttyfd._fd, STDOUT_FILENO);
			}
			else if (VTY_LOGIN_STDIO == vty_login_type(cli_shell.vty))
			{
				vty_stdio_attribute();

				if (cli_shell.vty->node >= ENABLE_NODE)
				{
					cli_shell.vty->privilege = CMD_CONFIG_LEVEL;
				}
			}
			cli_shell.vty->priv = &cli_shell;
			cli_shell.init = 2;
		}
	}
}
/* Accept connection from the network. */
static int vty_accept(struct eloop *thread)
{
	zpl_socket_t vty_sock;
	union sockunion su;
	int ret;
	zpl_uint32 on;
	zpl_socket_t accept_sock;
	struct prefix p;
	#ifdef ZPL_IP_FILTER
	struct access_list *acl = NULL;
	#endif
	zpl_char buf[SU_ADDRSTRLEN];
	accept_sock = ELOOP_FD(thread);
	/* We continue hearing vty ipstack_socket. */
	vty_event(VTY_SERV, accept_sock, NULL);

	memset(&su, 0, sizeof(union sockunion));

	/* We can handle IPv4 or IPv6 ipstack_socket. */
	vty_sock = sockunion_accept(accept_sock, &su);
	if (vty_sock._fd < 0)
	{
		zlog_warn(MODULE_DEFAULT, "can't ipstack_accept vty ipstack_socket : %s",
				  ipstack_strerror(ipstack_errno));
		return -1;
	}
	ipstack_set_nonblocking(vty_sock);

	sockunion2hostprefix(&su, &p);
	if (cli_shell.mutex)
		os_mutex_lock(cli_shell.mutex, OS_WAIT_FOREVER);
	#ifdef ZPL_IP_FILTER	
	/* VTY's accesslist apply. */
	if (p.family == IPSTACK_AF_INET && _global_host.vty_accesslist_name)
	{
		if ((acl = access_list_lookup(AFI_IP, _global_host.vty_accesslist_name)) && (access_list_apply(acl, &p) == FILTER_DENY))
		{
			zlog(MODULE_DEFAULT, ZLOG_LEVEL_INFO, "Vty connection refused from %s",
				 sockunion2str(&su, buf, SU_ADDRSTRLEN));
			ipstack_close(vty_sock);
			if (cli_shell.mutex)
				os_mutex_unlock(cli_shell.mutex);
			/* continue accepting connections */
			vty_event(VTY_SERV, accept_sock, NULL);
			return 0;
		}
	}

#ifdef ZPL_BUILD_IPV6
	/* VTY's ipv6 accesslist apply. */
	if (p.family == IPSTACK_AF_INET6 && _global_host.vty_ipv6_accesslist_name)
	{
		if ((acl = access_list_lookup(AFI_IP6, _global_host.vty_ipv6_accesslist_name)) &&
			(access_list_apply(acl, &p) == FILTER_DENY))
		{
			zlog(MODULE_DEFAULT, ZLOG_LEVEL_INFO, "Vty connection refused from %s",
				 sockunion2str(&su, buf, SU_ADDRSTRLEN));
			ipstack_close(vty_sock);
			if (cli_shell.mutex)
				os_mutex_unlock(cli_shell.mutex);
			/* continue accepting connections */
			vty_event(VTY_SERV, accept_sock, NULL);

			return 0;
		}
	}
#endif /* ZPL_BUILD_IPV6 */
	#endif
	if (cli_shell.mutex)
		os_mutex_unlock(cli_shell.mutex);
	on = 1;
	ret = ipstack_setsockopt(vty_sock, IPSTACK_IPPROTO_TCP, IPSTACK_TCP_NODELAY, (zpl_char *)&on,
							 sizeof(on));
	if (ret < 0)
		zlog(MODULE_DEFAULT, ZLOG_LEVEL_INFO, "can't set sockopt to vty_sock : %s",
			 ipstack_strerror(ipstack_errno));

	zlog(MODULE_DEFAULT, ZLOG_LEVEL_INFO, "Vty connection from %s",
		 sockunion2str(&su, buf, SU_ADDRSTRLEN));

	vty_create(vty_sock, &su);

	return 0;
}
#ifndef ZPL_BUILD_IPV6
/* Make vty server ipstack_socket. */
static void vty_serv_sock_family(const char *addr, zpl_ushort port,
								 int family)
{
	int ret;
	union sockunion su;
	zpl_socket_t accept_sock;
	void *naddr = NULL;

	memset(&su, 0, sizeof(union sockunion));
	su.sa.sa_family = family;
	if (addr)
	{
		switch (family)
		{
		case IPSTACK_AF_INET:
			naddr = &su.sin.sin_addr;
			break;
#ifdef ZPL_BUILD_IPV6
		case IPSTACK_AF_INET6:
			naddr = &su.sin6.sin6_addr;
			break;
#endif
		}
	}
	if (naddr)
	{
		switch (ipstack_inet_pton(family, addr, naddr))
		{
		case -1:
			zlog_err(MODULE_DEFAULT, "bad address %s", addr);
			naddr = NULL;
			break;
		case 0:
			zlog_err(MODULE_DEFAULT, "error translating address %s: %s", addr,
					 ipstack_strerror(ipstack_errno));
			naddr = NULL;
		}
	}
	/* Make new ipstack_socket. */
	accept_sock = sockunion_stream_socket(&su);
	if (accept_sock._fd < 0)
		return;

	/* This is server, so reuse address. */
	sockopt_reuseaddr(accept_sock);
	sockopt_reuseport(accept_sock);

	/* Bind ipstack_socket to universal address and given port. */
	ret = sockunion_bind(accept_sock, &su, port, naddr);
	if (ret < 0)
	{
		zlog_warn(MODULE_DEFAULT, "can't ipstack_bind ipstack_socket");
		ipstack_close(accept_sock); /* Avoid sd leak. */
		return;
	}

	/* Listen ipstack_socket under queue 3. */
	ret = ipstack_listen(accept_sock, 3);
	if (ret < 0)
	{
		zlog(MODULE_DEFAULT, ZLOG_LEVEL_WARNING, "can't ipstack_listen ipstack_socket(%s)",
			 ipstack_strerror(ipstack_errno));
		ipstack_close(accept_sock); /* Avoid sd leak. */
		return;
	}

	/* Add vty server event. */
	vty_event(VTY_SERV, accept_sock, NULL);
}
#endif

#ifdef VTYSH
/* For ipstack_sockaddr_un. */
#include <sys/un.h>
/* VTY shell UNIX domain ipstack_socket. */
static void
vty_serv_un(const char *path)
{
	int ret;
	int len;
	zpl_socket_t sock;
	struct ipstack_sockaddr_un serv;
	mode_t old_mask;

	/* First of all, unlink existing ipstack_socket */
	unlink(path);

	/* Set umask */
	old_mask = umask(0007);

	/* Make UNIX domain ipstack_socket. */
	sock = ipstack_socket(OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0);
	if (sock._fd < 0)
	{
		zlog_err(MODULE_DEFAULT, "Cannot create unix stream ipstack_socket: %s", ipstack_strerror(ipstack_errno));
		return;
	}

	/* Make server ipstack_socket. */
	memset(&serv, 0, sizeof(struct ipstack_sockaddr_un));
	serv.sun_family = IPSTACK_AF_UNIX;
	strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = serv.sun_len = SUN_LEN(&serv);
#else
	len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = ipstack_bind(sock, (struct ipstack_sockaddr *)&serv, len);
	if (ret < 0)
	{
		zlog_err(MODULE_DEFAULT, "Cannot ipstack_bind path %s: %s", path, ipstack_strerror(ipstack_errno));
		ipstack_close(sock); /* Avoid sd leak. */
		return;
	}

	ret = ipstack_listen(sock, 5);
	if (ret < 0)
	{
		zlog_err(MODULE_DEFAULT, "ipstack_listen(fd %d) failed: %s", sock._fd, ipstack_strerror(ipstack_errno));
		ipstack_close(sock); /* Avoid sd leak. */
		return;
	}

	umask(old_mask);

	vty_event(VTYSH_SERV, sock, NULL);
}

#define VTYSH_DEBUG 1

static int
vtysh_accept(struct thread *thread)
{
	zpl_socket_t accept_sock;
	zpl_socket_t sock;
	int client_len;
	struct ipstack_sockaddr_un client;
	struct vty *vty;

	accept_sock = THREAD_FD(thread);

	vty_event(VTYSH_SERV, accept_sock, NULL);

	memset(&client, 0, sizeof(struct ipstack_sockaddr_un));
	client_len = sizeof(struct ipstack_sockaddr_un);

	sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client,
						  (socklen_t *)&client_len);

	if (sock._fd < 0)
	{
		zlog_warn(MODULE_DEFAULT, "can't ipstack_accept vty ipstack_socket : %s", ipstack_strerror(ipstack_errno));
		return -1;
	}

	if (ipstack_set_nonblocking(sock) < 0)
	{
		zlog_warn(MODULE_DEFAULT, "vtysh_accept: could not set vty ipstack_socket %d to non-blocking,"
								  " %s, closing",
				  sock._fd, ipstack_strerror(ipstack_errno));
		ipstack_close(sock);
		return -1;
	}

	vty = vty_new();
	vty->vtysh_msg = zpl_osmsg_new(VTYSH_BUFSIZ);
	vty->fd = sock;
	vty->wfd = sock;
	vty->type = VTY_SHELL_SERV;
	vty->node = ENABLE_NODE;
	vty->login_type = VTY_LOGIN_VTYSH;
	//vty->trapping = vty->monitor = 1;
	vty_event(VTYSH_READ, sock, vty);

	return 0;
}

static int
vtysh_flush(struct vty *vty)
{
	switch (buffer_flush_available(vty->obuf, vty->wfd))
	{
	case BUFFER_PENDING:
		vty_event(VTYSH_WRITE, vty->wfd, vty);
		break;
	case BUFFER_ERROR:
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(MODULE_DEFAULT, "%s: write error to fd %d, closing", __func__, vty->wfd._fd);
		buffer_reset(vty->obuf);
		vty_close(vty);
		return -1;
		break;
	case BUFFER_EMPTY:
		break;
	}
	return 0;
}

static int vtysh_write_msg(struct vty *vty, zpl_socket_t fd, char *data, int len)
{
	int datalen = 0, ret = 0;
	while(1)
	{
		ret = ipstack_write(fd, data+datalen, len-datalen);
		if(ret <= 0)
		{
			if (!IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				zlog_warn(MODULE_DEFAULT,
						"%s: read error on vtysh client fd %d, closing: %s", __func__,
						fd._fd, ipstack_strerror(ipstack_errno));
				vty->trapping = vty->monitor = 0;
				buffer_reset(vty->obuf);
				zpl_osmsg_reset(vty->vtysh_msg);
				vty_close(vty);
				return -1;
			}
		}
		datalen += ret;
		if(datalen == len)
			break;
	}
	return datalen;
}
static int vtysh_read(struct thread *thread)
{
	int ret = 0;
	zpl_socket_t sock;
	zpl_uint32 nbytes = 0, already = 0, totallen = 0;
	struct vty *vty = NULL;
	zpl_uchar *p = NULL, *buf = NULL;
	vtysh_result_t *header = NULL;
	vtysh_msghdr_t msg;
	sock = THREAD_FD(thread);
	vty = THREAD_ARG(thread);
	vty->t_read = NULL;
#if 1
	zlog_warn(MODULE_DEFAULT," vtysh_read: getp=%d endp=%d size=%d\r\n", vty->vtysh_msg->getp, vty->vtysh_msg->endp, vty->vtysh_msg->size);

	/* Read length and command (if we don't have it already). */
	already = zpl_osmsg_get_endp(vty->vtysh_msg);
	if (already < sizeof(vtysh_msghdr_t))
	{
		nbytes = zpl_osmsg_readfd(vty->vtysh_msg, sock, sizeof(vtysh_msghdr_t) - already);
		if (nbytes == -1)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTYSH_READ, sock, vty);
				return 0;
			}
			zlog_warn(MODULE_DEFAULT,
					  "%s: read error on vtysh client fd %d, closing: %s", __func__,
					  sock._fd, ipstack_strerror(ipstack_errno));
			vty->trapping = vty->monitor = 0;
			buffer_reset(vty->obuf);
			zpl_osmsg_reset(vty->vtysh_msg);
			vty_close(vty);
			return -1;
		}
		if (nbytes != (ssize_t)(sizeof(vtysh_msghdr_t) - already))
		{
			/* Try again later. */
			vty_event(VTYSH_READ, sock, vty);
			return 0;
		}
		already = sizeof(vtysh_msghdr_t);
	}
	

	/* Reset to read from the beginning of the incoming packet. */
	zpl_osmsg_set_getp(vty->vtysh_msg, 0);

	/* Fetch header values */
	msg.type = zpl_osmsg_getl(vty->vtysh_msg);
	msg.msglen = zpl_osmsg_getl(vty->vtysh_msg);

	if (msg.msglen > zpl_osmsg_get_size(vty->vtysh_msg))
	{
		zlog_warn(MODULE_DEFAULT, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
				  __func__, sock._fd, msg.msglen, zpl_osmsg_get_size(vty->vtysh_msg));
		vty->trapping = vty->monitor = 0;
		buffer_reset(vty->obuf);
		zpl_osmsg_reset(vty->vtysh_msg);
		vty_close(vty);
		return -1;
	}
	totallen = msg.msglen + sizeof(vtysh_msghdr_t);
	/* Read rest of data. */
	if (already < totallen)
	{
		if (((nbytes = zpl_osmsg_readfd(vty->vtysh_msg, sock,
										 totallen - already)) == 0) ||
			(nbytes == -1))
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTYSH_READ, sock, vty);
				return 0;
			}
			zlog_warn(MODULE_DEFAULT,
					  "%s: read error on vtysh client fd %d, closing: %s", __func__,
					  sock._fd, ipstack_strerror(ipstack_errno));	
			vty->trapping = vty->monitor = 0;		  		
			buffer_reset(vty->obuf);
			zpl_osmsg_reset(vty->vtysh_msg);
			vty_close(vty);
			return -1;
		}
		if (nbytes != (ssize_t)(totallen - already))
		{
			/* Try again later. */
			vty_event(VTYSH_READ, sock, vty);
			return 0;
		}
	}
	buf = zpl_osmsg_pnt(vty->vtysh_msg);
	//printf("==========vtysh cmd: type=%d total=%d paload=%d (%s)\r\n", msg.type, zpl_osmsg_get_endp(vty->vtysh_msg), msg.msglen, buf);
#else
	if ((nbytes = ipstack_read(sock, vty->buf, VTY_READ_BUFSIZ)) <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTYSH_READ, sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(MODULE_DEFAULT, "%s: read failed on vtysh client fd %d, closing: %s",
					  __func__, sock, ipstack_strerror(ipstack_errno));
		}
		buffer_reset(vty->obuf);
		vty_close(vty);
#ifdef VTYSH_DEBUG
		printf("close vtysh\n");
#endif /* VTYSH_DEBUG */
		return 0;
	}

#ifdef VTYSH_DEBUG
	printf("line: %.*s\n", nbytes, buf);
#endif /* VTYSH_DEBUG */
#endif
	if (msg.type == VTYSH_MSG_DATA)
	{
		zpl_osmsg_reset(vty->vtysh_msg);
		header = (vtysh_result_t *)buffer_dataptr(vty->obuf);
		buffer_putemptyeize(vty->obuf, sizeof(vtysh_result_t));
		buffer_putstr(vty->obuf, "No support\n");
		header->type = htonl(msg.type);
		header->retcode = htonl(-1);
		header->retlen = htonl(buffer_size(vty->obuf)-sizeof(vtysh_result_t));

		if (!vty->t_write && (vtysh_flush(vty) < 0))
			return 0;
	}
	else if (msg.type == VTYSH_MSG_ECHO)
	{
		zpl_osmsg_reset(vty->vtysh_msg);
		header = (vtysh_result_t *)vty->buf;
		header->type = htonl(msg.type);
		header->retcode = htonl(0);
		header->retlen = htonl(0);
		vtysh_write_msg(vty, sock, (char *)vty->buf, sizeof(vtysh_result_t));
		vty_event(VTYSH_READ, sock, vty);
		return 0;
	}	
	else if (msg.type == VTYSH_MSG_CMD)
	{
		if (vty->length + msg.msglen >= vty->max)
		{
			/* Clear command line buffer. */
			vty->cp = vty->length = 0;
			vty_clear_buf(vty);
			zpl_osmsg_reset(vty->vtysh_msg);
			header = (vtysh_result_t *)buffer_dataptr(vty->obuf);
			buffer_putemptyeize(vty->obuf, sizeof(vtysh_result_t));
			buffer_putstr(vty->obuf, "%% Command is too long.\n");
			header->type = htonl(msg.type);
			header->retcode = htonl(-1);
			header->retlen = htonl(buffer_size(vty->obuf)-sizeof(vtysh_result_t));			
			if (!vty->t_write && (vtysh_flush(vty) < 0))
				return 0;
		}
		for (p = buf; p < buf + msg.msglen; p++)
		{
			vty->buf[vty->length++] = *p;
			if (*p == '\0' || *p == '\r' || *p == '\n')
			{
#ifdef VTYSH_DEBUG
				zlog_warn(MODULE_DEFAULT,"vtysh cmd: (%s)\r\n", vty->buf);
#endif /* VTYSH_DEBUG */

				header = (vtysh_result_t *)buffer_dataptr(vty->obuf);
				buffer_putemptyeize(vty->obuf, sizeof(vtysh_result_t));

				/* Pass this line to parser. */
				ret = vty_execute(vty);
				/* Note that vty_execute clears the command buffer and resets vty->length to 0. */
				/* Return result. */
#ifdef VTYSH_DEBUG
				zlog_warn(MODULE_DEFAULT,"result: %d\r\n", ret);
#endif /* VTYSH_DEBUG */

				header->type = htonl(msg.type);
				header->retcode = htonl(ret);
				header->retlen = htonl(buffer_size(vty->obuf)-sizeof(vtysh_result_t));

				if (!vty->t_write && (vtysh_flush(vty) < 0))
					/* Try to flush results; exit if a write error occurs. */
					return 0;
			}
		}
	}
	else
	{
		zpl_osmsg_reset(vty->vtysh_msg);
		header = (vtysh_result_t *)buffer_dataptr(vty->obuf);
		buffer_putemptyeize(vty->obuf, sizeof(vtysh_result_t));
		buffer_putstr(vty->obuf, "No support\n");
		header->type = htonl(msg.type);
		header->retcode = htonl(-1);
		header->retlen = htonl(buffer_size(vty->obuf)-sizeof(vtysh_result_t));

		if (!vty->t_write && (vtysh_flush(vty) < 0))
			return 0;
	}
	zpl_osmsg_reset(vty->vtysh_msg);
	vty_event(VTYSH_READ, sock, vty);
	return 0;
}

static int
vtysh_write(struct thread *thread)
{
	struct vty *vty = THREAD_ARG(thread);
	vty->t_write = NULL;
	vtysh_flush(vty);
	return 0;
}

static int vty_sshd_read(struct thread *thread)
{
	zpl_socket_t sock;
	zpl_uint32 nbytes;
	struct vty *vty;
	zpl_uchar buf[VTY_READ_BUFSIZ];
	sock = THREAD_FD(thread);
	vty = THREAD_ARG(thread);
	vty->t_read = NULL;
	os_bzero(buf, sizeof(buf));
	if ((nbytes = ipstack_read(sock, buf, VTY_READ_BUFSIZ)) <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
				vty_event(VTYSH_READ, sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(MODULE_DEFAULT, "%s: read failed on vtysh client fd %d, closing: %s",
					  __func__, sock._fd, ipstack_strerror(ipstack_errno));
		}
		buffer_reset(vty->obuf);
		vty_close(vty);
		return 0;
	}

	if (vty->length + nbytes >= vty->max)
	{
		/* Clear command line buffer. */
		vty->cp = vty->length = 0;
		vty_clear_buf(vty);
		vty_out(vty, "%% Command is too long.%s", VTY_NEWLINE);
		goto out;
	}

	vty_read_handle(vty, buf, nbytes);

	/* Check status. */
	if (vty->status == VTY_CLOSE)
	{
		vty_close(vty);
		return 0;
	}
	else
	{
		vtysh_flush(vty);
	}
out:
	vty->t_read = thread_add_read(cli_shell.m_thread_master, vty_sshd_read, vty, sock);
	return 0;
}

int vty_sshd_init(zpl_socket_t sock, struct vty *vty)
{
	vty->t_read = thread_add_read(cli_shell.m_thread_master, vty_sshd_read, vty, sock);
	return OK;
}

#endif /* VTYSH */
#ifdef ZPL_BUILD_IPV6
static void
vty_serv_sock_addrinfo (const char *hostname, unsigned short port)
{
	int ret;
  	struct ipstack_addrinfo req;
	struct ipstack_addrinfo *ainfo;
	struct ipstack_addrinfo *ainfo_save;
	zpl_socket_t sock;
	char port_str[BUFSIZ];

  	memset (&req, 0, sizeof (struct ipstack_addrinfo));
  	req.ai_flags = AI_PASSIVE;
  	req.ai_family = AF_UNSPEC;
  	req.ai_socktype = SOCK_STREAM;
  	sprintf (port_str, "%d", port);
  	port_str[sizeof (port_str) - 1] = '\0';

  	ret = ipstack_getaddrinfo (hostname, port_str, &req, &ainfo);

  	if (ret != 0)
	{
      fprintf (stderr, "getaddrinfo failed: %s\n", gai_strerror (ret));
      exit (1);
    }

  	ainfo_save = ainfo;

  	do
    {
      	if (ainfo->ai_family != IPSTACK_AF_INET	&& ainfo->ai_family != IPSTACK_AF_INET6)
			continue;

      	sock = ipstack_socket (IPCOM_STACK, ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
      	if (sock._fd < 0)
			continue;

      	sockopt_v6only (ainfo->ai_family, sock);
      	sockopt_reuseaddr (sock);
      	sockopt_reuseport (sock);

      	ret = ipstack_bind (sock, ainfo->ai_addr, ainfo->ai_addrlen);
      	if (ret < 0)
		{
	  		ipstack_close (sock);	/* Avoid sd leak. */
			continue;
		}
      	ret = ipstack_listen (sock, 3);
      	if (ret < 0) 
		{
	  		ipstack_close (sock);	/* Avoid sd leak. */
			continue;
		}

      	vty_event (VTY_SERV, sock, NULL);
	}
	while ((ainfo = ainfo->ai_next) != NULL);
	ipstack_freeaddrinfo (ainfo_save);
}
#endif

/* Determine address family to ipstack_bind. */
void vty_serv_init(const char *addr, zpl_ushort port, const char *path, const char *tty)
{
	/* If port is set to 0, do not ipstack_listen on TCP/IP at all! */
	if (port)
	{
#ifdef ZPL_BUILD_IPV6
		vty_serv_sock_addrinfo (addr, port);
#else  /* ! ZPL_BUILD_IPV6 */
		vty_serv_sock_family(addr, port, IPSTACK_AF_INET);
#endif /* ZPL_BUILD_IPV6 */
	}

#ifdef VTYSH
	vty_serv_un(path);
#endif /* VTYSH */
	vty_console_init(tty);
#ifdef ZPL_SHRL_MODULE
	if(tty && strstr(tty, "readline"))
		vtyrl_stdio_start(1);
#endif /*ZPL_SHRL_MODULE*/	
}

/* Close vty interface.  Warning: call this only from functions that
 will be careful not to access the vty afterwards (since it has
 now been freed).  This is safest from top-level functions (called
 directly by the thread dispatcher). */
void vty_close(struct vty *vty)
{
	zpl_uint32 i;

	/* Check configure. */
	vty_config_unlock(vty);
#ifdef VTYSH
	if (vty->vtysh_msg)
	{
		zpl_osmsg_free(vty->vtysh_msg);
		vty->vtysh_msg = NULL;
	}
#endif /* VTYSH */

	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE || vty_login_type(vty) == VTY_LOGIN_STDIO)
	{
			fprintf(stdout, "%s vty_login_type(vty)=%d\r\n",__func__, vty_login_type(vty));
	fflush(stdout);
		vty_console_close(vty);
		return;
	}
	if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
	{
		/* Cancel threads.*/
		if (vty->t_read)
			thread_cancel(vty->t_read);
		if (vty->t_write)
			thread_cancel(vty->t_write);
		if (vty->t_timeout)
			thread_cancel(vty->t_timeout);
		vty->t_read = NULL;
		vty->t_write = NULL;
		vty->t_timeout = NULL;
	}
	else
	{
		/* Cancel threads.*/
		if (vty->t_read)
			eloop_cancel(vty->t_read);
		if (vty->t_write)
			eloop_cancel(vty->t_write);
		if (vty->t_timeout)
			eloop_cancel(vty->t_timeout);
		vty->t_read = NULL;
		vty->t_write = NULL;
		vty->t_timeout = NULL;
	}

	/* Flush buffer. */
	if (vty->obuf)
	{
		buffer_flush_all(vty->obuf, vty->wfd);
		buffer_free(vty->obuf);
	}
	/* Free command history. */
	for (i = 0; i < VTY_MAXHIST; i++)
		if (vty->hist[i])
			XFREE(MTYPE_VTY_HIST, vty->hist[i]);
	/* Unset vector. */
	vector_unset(cli_shell.vtyvec, vty->fd._fd);

	if (vty->buf)
		XFREE(MTYPE_VTY, vty->buf);

	if (vty_login_type(vty) == VTY_LOGIN_SSH)
	{
		if (vty->ssh_close)
			(vty->ssh_close)(vty);
	}
	else
	{
		ipstack_close(vty->fd);
	}
	/* OK free vty. */
	XFREE(MTYPE_VTY, vty);
	return;
}

/* When time out occur output message then close connection. */
static int vty_timeout(struct eloop *thread)
{
	struct vty *vty;

	vty = ELOOP_ARG(thread);
	vty->t_timeout = NULL;
	vty->v_timeout = 0;

	/* Clear buffer*/
	buffer_reset(vty->obuf);

	vty_sync_out(vty, "%s%sVty connection is timed out.%s%s",
				 VTY_NEWLINE, VTY_NEWLINE,
				 VTY_NEWLINE, VTY_NEWLINE);

	/* Close connection. */
	vty->status = VTY_CLOSE;
	vty_close(vty);

	return 0;
}

/* Read up configuration file from file_name. */
static void vty_read_file(FILE *confp)
{
	int ret;
	struct vty *vty;
	zpl_uint32 line_num = 0;
	vty = vty_new();
	ipstack_init(OS_STACK, vty->fd);
	ipstack_init(OS_STACK, vty->wfd);
	vty->wfd._fd = STDOUT_FILENO;
	vty->fd._fd = STDIN_FILENO;
	vty->type = VTY_FILE;
	vty->node = CONFIG_NODE;

	/* Execute configuration file */
	ret = config_from_file(vty, confp, &line_num);

	/* Flush any previous errors before printing messages below */
	if (vty->obuf)
		buffer_flush_all(vty->obuf, vty->wfd);

	if (!((ret == CMD_SUCCESS) || (ret == CMD_ERR_NOTHING_TODO)))
	{
		switch (ret)
		{
		case CMD_ERR_AMBIGUOUS:
			zlog_err(MODULE_LIB,"*** Error reading config: Ambiguous command.\n");
			//fprintf(stderr, "*** Error reading config: Ambiguous command.\n");
			break;
		case CMD_ERR_NO_MATCH:
			zlog_err(MODULE_LIB,"*** Error reading config: There is no such command.\n");
			//fprintf(stderr,
			//		"*** Error reading config: There is no such command.\n");
			break;
		}
		zlog_err(MODULE_LIB,"*** Error occured processing line %u, below:\n%s\n",
				line_num, vty->buf);
		//fprintf(stderr, "*** Error occured processing line %u, below:\n%s\n",
		//		line_num, vty->buf);
		vty_close(vty);
		exit(1);
	}
	vty_close(vty);
}

static int host_config_default(zpl_char *password, zpl_char *defult_config)
{
	if (cli_shell.mutex)
		os_mutex_lock(cli_shell.mutex, OS_WAIT_FOREVER);
	if (defult_config == NULL)
	{
		zlog_err(MODULE_LIB,"failed to setting default configuration file :%s\n", ipstack_strerror(ipstack_errno));
	}
	if (_global_host.name == NULL) //
		_global_host.name = XSTRDUP(MTYPE_HOST, OEM_PROGNAME);
	if (cli_shell.mutex)
		os_mutex_unlock(cli_shell.mutex);

	if (defult_config)
		host_config_set(defult_config);

	return 0;
}

/* Read up configuration file from file_name. */
static zpl_char *vty_default_config_getting(void)
{
	if (_global_host.config && access(_global_host.config, 0x04) == 0)
		return _global_host.config;
	if (_global_host.default_config && access(_global_host.default_config, 0x04) == 0)
		return _global_host.default_config;
	if (_global_host.factory_config && access(_global_host.factory_config, 0x04) == 0)
		return _global_host.factory_config;
	return NULL;
}
void vty_load_config(zpl_char *config_file)
{
	FILE *confp = NULL;

	host_config_default(NULL, config_file);

	if (config_file)
		confp = fopen(config_file, "r");
	else
	{
		if (vty_default_config_getting())
		{
			confp = fopen(vty_default_config_getting(), "r");
		}
		else
		{
			zlog_err(MODULE_LIB, "configuration file exits.");
			host_loadconfig_state(LOAD_DONE);
			return;
		}
	}
	if (confp == NULL)
	{
		zlog_err(MODULE_LIB, "failed to open configuration file %s: %s", config_file ? config_file : "null",
				ipstack_strerror(ipstack_errno));		
		host_loadconfig_state(LOAD_DONE);
		return;
	}
	if (confp)
	{
		fseek(confp, 0, SEEK_END);
		if (ftell(confp) < 16)
		{
			fclose(confp);
			confp = NULL;
			zlog_err(MODULE_LIB, "configuration file is BACK");
		}
		else
		{
			fseek(confp, 0, SEEK_SET);
			vty_read_file(confp);
			fclose(confp);
		}
	}
	host_loadconfig_state(LOAD_DONE);
}

/* Small utility function which output log to the VTY. */
void vty_log(const char *level, const char *proto_str, const char *format,
			 zlog_timestamp_t ctl, va_list va)
{
	zpl_uint32 i;
	struct vty *vty = NULL;

	if (!cli_shell.vtyvec)
		return;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if ((vty = vector_slot(cli_shell.vtyvec, i)) != NULL)
		{
			if (vty->monitor && !(vty->cancel))
			{
				va_list ac;
				va_copy(ac, va);
				vty_log_out(vty, level, proto_str, format, ctl, ac, NULL, NULL, 0);
				va_end(ac);
			}
		}
	}
}

void vty_log_debug(const char *level, const char *proto_str, const char *format,
				   zlog_timestamp_t ctl, va_list va, const char *file, const char *func, const zpl_uint32 line)
{
	zpl_uint32 i;
	struct vty *vty = NULL;

	if (!cli_shell.vtyvec)
		return;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if ((vty = vector_slot(cli_shell.vtyvec, i)) != NULL)
		{
			if (vty->monitor && !(vty->cancel))
			{
				va_list ac;
				va_copy(ac, va);
				vty_log_out(vty, level, proto_str, format, ctl, ac, file, func, line);
				va_end(ac);
			}
		}
	}
}

int vty_trap_log(const char *level, const char *proto_str, const char *format,
				 zlog_timestamp_t ctl, va_list va)
{
	zpl_uint32 i = 0, flag = 0;
	struct vty *vty = NULL;

	if (!cli_shell.vtyvec)
		return ERROR;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if ((vty = vector_slot(cli_shell.vtyvec, i)) != NULL)
		{
			if (vty->trapping && (!vty->cancel))
			{
				va_list ac;
				va_copy(ac, va);
				vty_log_out(vty, level, proto_str, format, ctl, ac, NULL, NULL, 0);
				va_end(ac);
				flag++;
			}
		}
	}
	if (flag)
		return OK;
	return ERROR;
}

/* Async-signal-safe version of vty_log for fixed strings. */
static void ip_vty_log_fixed(struct vty *vty, zpl_char *buf, zpl_size_t len)
{
	struct ipstack_iovec iov[2];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	iov[1].iov_base = (void *)"\r\n";
	iov[1].iov_len = 2;
	if(vty->type == VTY_STABDVY)
		return 0;	
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcflush(vty->wfd._fd, TCIOFLUSH);//串口清空缓存
	}
	ipstack_writev(vty->wfd, iov, 2);
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcdrain(vty->wfd._fd);//等待所有输出完毕
	}
}

//#undef ipstack_iovec
static void os_vty_log_fixed(struct vty *vty, zpl_char *buf, zpl_size_t len)
{
	struct ipstack_iovec iov[2];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	iov[1].iov_base = (void *)"\r\n";
	iov[1].iov_len = 2;
	if(vty->type == VTY_STABDVY)
		return 0;	
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcflush(vty->wfd._fd, TCIOFLUSH);
	}
	writev(vty->wfd._fd, iov, 2);
	if (vty_login_type(vty) == VTY_LOGIN_CONSOLE)
	{
		 tcdrain(vty->wfd._fd);
	}
}

void vty_log_fixed(zpl_char *buf, zpl_size_t len)
{
	zpl_uint32 i;
	struct vty *vty = NULL;
	/* vty may not have been initialised */
	if (!cli_shell.vtyvec)
		return;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if (((vty = vector_slot(cli_shell.vtyvec, i)) != NULL) &&
			(vty->trapping || vty->monitor) &&
			!(vty->cancel))
		{
			/* N.B. We don't care about the return code, since process is
			 most likely just about to die anyway. */
			if (is_os_stack(vty->wfd))
				os_vty_log_fixed(vty, buf, len);
			else
				ip_vty_log_fixed(vty, buf, len);
		}
	}
}

int vty_config_lock(struct vty *vty)
{
	if (_global_host.vty_config == 0)
	{
		vty->config = 1;
		_global_host.vty_config = 1;
	}
	return vty->config;
}

int vty_config_unlock(struct vty *vty)
{
	if (_global_host.vty_config == 1 && vty->config == 1)
	{
		vty->config = 0;
		_global_host.vty_config = 0;
	}
	return vty->config;
}

static void vty_event(enum vtyevent event, zpl_socket_t sock, struct vty *vty)
{
	struct eloop *vty_serv_thread;
	if (vty && vty->cancel)
	{
		if (event == VTY_SERV || event == VTYSH_SERV ||
			event == VTY_STDIO_WAIT || event == VTY_STDIO_ACCEPT)
			;
		else
			return;
	}
	switch (event)
	{
	case VTY_SERV:
		vty_serv_thread = eloop_add_read(cli_shell.m_eloop_master, vty_accept, vty, sock);
		vector_set_index(cli_shell.serv_thread, sock._fd, vty_serv_thread);
		break;
#ifdef VTYSH
	case VTYSH_SERV:
		vty_serv_thread = thread_add_read(cli_shell.m_thread_master, vtysh_accept, vty, sock);
		vector_set_index(cli_shell.serv_thread, sock._fd, vty_serv_thread);
		break;
	case VTYSH_READ:
		vty->t_read = thread_add_read(cli_shell.m_thread_master, vtysh_read, vty, sock);
		break;
	case VTYSH_WRITE:
		vty->t_write = thread_add_write(cli_shell.m_thread_master, vtysh_write, vty, sock);
		break;
#endif /* VTYSH */
	case VTY_READ:
		if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
		{
			vty->t_read = thread_add_read(cli_shell.m_thread_master, vty_console_read, vty, sock);

			if (vty->t_timeout)
				thread_cancel(vty->t_timeout);
			vty->t_timeout = NULL;
			vty->t_timeout = thread_add_timer(cli_shell.m_thread_master,
											  vty_console_timeout, vty, vty->v_timeout ? vty->v_timeout : _global_host.vty_timeout_val);
		}
		else
		{
			vty->t_read = eloop_add_read(cli_shell.m_eloop_master, vty_read, vty, sock);

			if (vty->t_timeout)
				eloop_cancel(vty->t_timeout);
			vty->t_timeout = NULL;
			vty->t_timeout = eloop_add_timer(cli_shell.m_eloop_master, vty_timeout, vty,
											 vty->v_timeout ? vty->v_timeout : _global_host.vty_timeout_val);
		}
		break;
	case VTY_WRITE:
		if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
		{
			if (!vty->t_write)
				vty->t_write = thread_add_write(cli_shell.m_thread_master,
												vty_console_flush, vty, sock);
		}
		else
		{
			if (!vty->t_write)
				vty->t_write = eloop_add_write(cli_shell.m_eloop_master, vty_flush, vty,
											   sock);
		}
		break;
	case VTY_TIMEOUT_RESET:
		if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
		{
			if (vty->t_timeout)
			{
				thread_cancel(vty->t_timeout);
				vty->t_timeout = NULL;
			}
			vty->t_timeout = thread_add_timer(cli_shell.m_thread_master,
											  vty_console_timeout,
											  vty, vty->v_timeout ? vty->v_timeout : _global_host.vty_timeout_val);
		}
		else
		{
			if (vty->t_timeout)
			{
				eloop_cancel(vty->t_timeout);
				vty->t_timeout = NULL;
			}
			vty->t_timeout = eloop_add_timer(cli_shell.m_eloop_master, vty_timeout, vty,
											 vty->v_timeout ? vty->v_timeout : _global_host.vty_timeout_val);
		}
		break;

	case VTY_STDIO_WAIT:
		if (vty->t_wait)
		{
			thread_cancel(vty->t_wait);
			vty->t_wait = NULL;
		}
		vty->t_wait = thread_add_timer(cli_shell.m_thread_master, vty_console_wait,
									   vty, 2);
		break;
	case VTY_STDIO_ACCEPT:
		if (vty->t_read)
		{
			thread_cancel(vty->t_read);
			vty->t_read = NULL;
		}
		vty->t_read = thread_add_read(cli_shell.m_thread_master, vty_console_accept, vty,
									  sock);
		break;
	}
}

/* Set time out value. */
int vty_exec_timeout(struct vty *vty, const char *min_str, const char *sec_str)
{
	zpl_socket_t tmp;
	zpl_ulong timeout = 0;
	if (cli_shell.mutex)
		os_mutex_lock(cli_shell.mutex, OS_WAIT_FOREVER);
	/* min_str and sec_str are already checked by parser.  So it must be
	 all digit string. */
	if (min_str)
	{
		timeout = strtol(min_str, NULL, 10);
		timeout *= 60;
	}
	if (sec_str)
		timeout += strtol(sec_str, NULL, 10);

	vty->v_timeout = timeout;
	vty_event(VTY_TIMEOUT_RESET, tmp, vty);
	if (cli_shell.mutex)
		os_mutex_unlock(cli_shell.mutex);

	return CMD_SUCCESS;
}
/* Reset all VTY status. */
void vty_reset()
{
	zpl_uint32 i;
	struct vty *vty;
	struct eloop *vty_serv_thread;

	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if ((vty = vector_slot(cli_shell.vtyvec, i)) != NULL)
		{
			if (vty->obuf)
				buffer_reset(vty->obuf);
			vty->status = VTY_CLOSE;
			vty_close(vty);
		}
	}

	for (i = 0; i < vector_active(cli_shell.serv_thread); i++)
	{
		if ((vty_serv_thread = vector_slot(cli_shell.serv_thread, i)) != NULL)
		{
			eloop_cancel(vty_serv_thread);
			vty_serv_thread = NULL;
			vector_slot(cli_shell.serv_thread, i) = NULL;
			close(i);
		}
	}
	if (cli_shell.mutex)
		os_mutex_lock(cli_shell.mutex, OS_WAIT_FOREVER);
	_global_host.vty_timeout_val = VTY_TIMEOUT_DEFAULT;

	if (_global_host.vty_accesslist_name)
	{
		XFREE(MTYPE_VTY, _global_host.vty_accesslist_name);
		_global_host.vty_accesslist_name = NULL;
	}

	if (_global_host.vty_ipv6_accesslist_name)
	{
		XFREE(MTYPE_VTY, _global_host.vty_ipv6_accesslist_name);
		_global_host.vty_ipv6_accesslist_name = NULL;
	}
	if (cli_shell.mutex)
		os_mutex_unlock(cli_shell.mutex);
}

int vty_cancel(struct vty *vty)
{
	if (vty)
	{
		if (is_os_stack(vty->fd) || vty->type == VTY_FILE)
		{
			/* Cancel threads.*/
			if (vty->t_read)
				thread_cancel(vty->t_read);
			if (vty->t_write)
				thread_cancel(vty->t_write);
			if (vty->t_timeout)
				thread_cancel(vty->t_timeout);
			if (vty->t_wait)
				thread_cancel(vty->t_wait);
			vty->t_read = NULL;
			vty->t_write = NULL;
			vty->t_timeout = NULL;
			vty->t_wait = NULL;
		}
		else
		{
			/* Cancel threads.*/
			if (vty->t_read)
				eloop_cancel(vty->t_read);
			if (vty->t_write)
				eloop_cancel(vty->t_write);
			if (vty->t_timeout)
				eloop_cancel(vty->t_timeout);
			if (vty->t_wait)
				eloop_cancel(vty->t_wait);
			vty->t_read = NULL;
			vty->t_write = NULL;
			vty->t_timeout = NULL;
			vty->t_wait = NULL;
		}
		vty->cancel = zpl_true;
	}
	return OK;
}

int vty_resume(struct vty *vty)
{
	if (vty)
	{
		if (vty->cancel)
		{
			vty->cancel = zpl_false;
			vty_event(VTY_READ, vty->fd, vty);
		}
	}
	return OK;
}

struct vty *vty_lookup(zpl_socket_t sock)
{
	zpl_uint32 i;
	struct vty *vty = NULL;
	for (i = 0; i < vector_active(cli_shell.vtyvec); i++)
	{
		if ((vty = vector_slot(cli_shell.vtyvec, i)) != NULL)
		{
			if (vty->fd._fd == sock._fd)
				return vty;
		}
	}
	return NULL;
}

static void vty_save_cwd(void)
{
	zpl_char cwd[MAXPATHLEN];
	zpl_char *c;

	c = getcwd(cwd, MAXPATHLEN);

	if (!c)
	{
		chdir(SYSCONFDIR);
		getcwd(cwd, MAXPATHLEN);
	}

	_global_host.vty_cwd = XMALLOC(MTYPE_TMP, strlen(cwd) + 1);
	strcpy(_global_host.vty_cwd, cwd);
}

zpl_char *
vty_get_cwd()
{
	return _global_host.vty_cwd;
}

int vty_shell(struct vty *vty)
{
	return vty->type == VTY_SHELL ? 1 : 0;
}

int vty_shell_serv(struct vty *vty)
{
	return vty->type == VTY_SHELL_SERV ? 1 : 0;
}

int vty_ansync_enable(struct vty *vty, zpl_bool enable)
{
	vty->ansync = enable;
	return OK;
}

enum vtylogin_type vty_login_type(struct vty *vty)
{
	return vty->login_type;
}

void vty_init_vtysh(void)
{
	cli_shell.vtyvec = vector_init(VECTOR_MIN_SIZE);
}

void *vty_thread_master(void)
{
	return cli_shell.m_thread_master;
}

/* Install vty's own commands like `who' command. */
void vty_init(void)
{
	if(cli_shell.init == 0)
	{
		memset(&cli_shell, 0, sizeof(cli_shell_t));
		cli_shell.init = 1;
		cli_shell.mutex = os_mutex_init();
#ifdef ZPL_IPCOM_MODULE
		if (cli_shell.m_eloop_master == NULL)
			cli_shell.m_eloop_master = eloop_master_module_create(MODULE_TELNET);

		if (cli_shell.m_thread_master == NULL)
			cli_shell.m_thread_master = thread_master_module_create(MODULE_CONSOLE);
#else
		if (cli_shell.m_thread_master == NULL)
			cli_shell.m_eloop_master = cli_shell.m_thread_master = thread_master_module_create(MODULE_CONSOLE);
#endif
		/* For further configuration read, preserve current directory. */
		vty_save_cwd();

		cli_shell.vtyvec = vector_init(VECTOR_MIN_SIZE);

		cli_shell.vty_ctrl_cmd = vty_ctrl_default;

		/* Initilize server thread vector. */
		cli_shell.serv_thread = vector_init(VECTOR_MIN_SIZE);
	}
}

void vty_terminate(void)
{
	if (cli_shell.mutex)
	{
		os_mutex_exit(cli_shell.mutex);
		cli_shell.mutex = NULL;
	}
#ifdef ZPL_SHRL_MODULE
	vtyrl_stdio_exit();
#endif /*ZPL_SHRL_MODULE*/
	if (_global_host.vty_cwd)
		XFREE(MTYPE_TMP, _global_host.vty_cwd);

	if (cli_shell.vtyvec && cli_shell.serv_thread)
	{
		vty_reset();
		fprintf(stdout, "=====================%s\r\n", __func__);
		fflush(stdout);
		vector_free(cli_shell.vtyvec);
		vector_free(cli_shell.serv_thread);
	}
}

int vty_execute_shell(void *cli, const char *cmd)
{
	int ret = 0, vty_cflags = 0;
	struct vty *vty = (struct vty *)cli;
	if (vty == NULL)
	{
		vty = vty_new();
		ipstack_init(OS_STACK, vty->fd);
		ipstack_init(OS_STACK, vty->fd);
		vty->wfd._fd = STDOUT_FILENO;
		vty->fd._fd = STDIN_FILENO;
		vty->type = VTY_TERM;
		vty->node = ENABLE_NODE;
		vty_cflags = 1;
	}
	memset(vty->buf, 0, (VTY_BUFSIZ));
	strcpy(vty->buf, cmd);
	ret = vty_execute(vty);
	if (vty_cflags)
		vty_close(vty);
	return ret;
}

int cli_shell_result (const char *format, ...)
{
	if(cli_shell.cli_shell_vty)
	{
		va_list args;
		zpl_char buf[VTY_BUFSIZ];
		os_bzero(buf, sizeof(buf));
		va_start(args, format);
		snprintf(buf,sizeof(buf), format, args);
		va_end(args);
		vty_out(cli_shell.cli_shell_vty, "%s\r\n", buf);
	}
	return OK;	
}

#ifdef ZPL_IPCOM_MODULE
static int cli_telnet_task(void *argv)
{
	module_setup_task(MODULE_TELNET, os_task_id_self());
	host_waitting_loadconfig();
	eloop_mainloop(argv);
	return 0;
}

static int cli_telnet_task_init(void)
{
	if (cli_shell.m_eloop_master == NULL)
		cli_shell.m_eloop_master = eloop_master_module_create(MODULE_TELNET);

	if (cli_shell.telnet_taskid == 0)
		cli_shell.telnet_taskid = os_task_create("telnetdTask", OS_TASK_DEFAULT_PRIORITY,
												 0, cli_telnet_task, cli_shell.m_eloop_master, OS_TASK_DEFAULT_STACK);
	if (cli_shell.telnet_taskid)
	{
		module_setup_task(MODULE_TELNET, cli_shell.telnet_taskid);
		return OK;
	}
	return ERROR;
}

static int cli_telnet_task_exit(void)
{
	if (cli_shell.telnet_taskid)
		os_task_destroy(cli_shell.telnet_taskid);
	cli_shell.telnet_taskid = 0;
	if (cli_shell.m_eloop_master)
		eloop_master_free(cli_shell.m_eloop_master);
	cli_shell.m_eloop_master = NULL;
	return OK;
}
#endif

static int cli_console_task(void *argv)
{
	module_setup_task(MODULE_CONSOLE, os_task_id_self());
	host_waitting_loadconfig();
	thread_mainloop(argv);
	return 0;
}

static int cli_console_task_init(void)
{
	if (cli_shell.m_thread_master == NULL)
		cli_shell.m_thread_master = thread_master_module_create(MODULE_CONSOLE);
#ifdef ZPL_IPCOM_MODULE
	if (cli_shell.console_taskid == 0)
		cli_shell.console_taskid = os_task_create("consoleTask", OS_TASK_DEFAULT_PRIORITY,
												  0, cli_console_task, cli_shell.m_thread_master, OS_TASK_DEFAULT_STACK);
#else
	if (cli_shell.console_taskid == 0)
		cli_shell.console_taskid = os_task_create("shellTask", OS_TASK_DEFAULT_PRIORITY,
												  0, cli_console_task, cli_shell.m_thread_master, OS_TASK_DEFAULT_STACK);
#endif
	if (cli_shell.console_taskid)
	{
		module_setup_task(MODULE_CONSOLE, cli_shell.console_taskid);
		return OK;
	}
	return ERROR;
}

static int cli_console_task_exit(void)
{
	if (cli_shell.console_taskid)
		os_task_destroy(cli_shell.console_taskid);
	cli_shell.console_taskid = 0;
	if (cli_shell.m_thread_master)
		thread_master_free(cli_shell.m_thread_master);
	cli_shell.m_thread_master = NULL;
	return OK;
}

void vty_task_init(void)
{
#ifdef ZPL_SHRL_MODULE
	vtyrl_stdio_task_init();
#endif /*ZPL_SHRL_MODULE*/	
#ifdef ZPL_IPCOM_MODULE
	cli_console_task_init();
	cli_telnet_task_init();
#else
	cli_console_task_init();
#endif
	return OK;
}

void vty_task_exit(void)
{
#ifdef ZPL_IPCOM_MODULE
	cli_console_task_exit();
	cli_telnet_task_exit();
#else
	cli_console_task_exit();
#endif
#ifdef ZPL_SHRL_MODULE
	vtyrl_stdio_task_exit();
#endif /*ZPL_SHRL_MODULE*/
	return OK;
}




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


#include "zebra.h"
#include <arpa/telnet.h>
#include <termios.h>

#include "buffer.h"
#include "command.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "network.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "thread.h"
#include "eloop.h"
#include "version.h"
#include "host.h"
#include "vty.h"

#include "filter.h"
#include "vty_user.h"



static void vty_event(enum vtyevent, int, struct vty *);

/* Extern host structure from command.c */
extern struct host host;

/* Vector which store each vty structure. */
vector vtyvec;
/* Master of the threads. */
static struct thread_master *thread_master;
static struct eloop_master *eloop_master;

struct tty_console
{
#define VTYSTDIO	0
#define VTYCONSOLE	1
	struct termios termios_console;
	struct vty *console;
	void (*console_atclose)(void);
	struct thread *t_wait;
	char *tty;
};
struct tty_console vty_tty_console;
//struct tty_console vty_tty_console[VTYCONSOLE+1];
/*

 static struct termios console_orig_termios;
 static struct vty *console = NULL;
 static void (*console_atclose)(void);
 static struct thread *t_wait = NULL;
 */

/* Vty timeout value. */
/*static unsigned long vty_timeout_val = VTY_TIMEOUT_DEFAULT;*/

/* Vty access-class command */
/*static char *vty_accesslist_name = NULL;*/

/* Vty access-calss for IPv6. */
/*static char *vty_ipv6_accesslist_name = NULL;*/

/* VTY server thread. */
/*static vector Vvty_serv_thread;*/

/* Current directory. */
/*char *vty_cwd = NULL;*/

/* Configure lock. */
/*static int vty_config;*/

/* Login password check. */
/*static int no_password_check = 0;*/

/* Restrict unauthenticated logins? */
/*static const u_char restricted_mode_default = 0;
 static u_char restricted_mode = 0;*/
int vty_out_to_all_terminal( const char *format, ...)
{
    int i = 0,len=0;
    struct vty*vty;
    va_list args;
    char cszBuffer[1024] = {0};

    if (!vtyvec)
        return -1;

    /* Try to write to initial buffer.  */
    va_start (args, format);
    len = vsprintf (cszBuffer, format, args);
    va_end (args);

    for (i = 0; i < vector_active (vtyvec); i++)
    {
        if ((vty = vector_slot (vtyvec, i)) != NULL)
        {
            vty_out(vty,"%s%s", cszBuffer, VTY_NEWLINE);
        }
    }

    return len;
}
/* Integrated configuration file path */
//char integrate_default[] = SYSCONFDIR INTEGRATE_DEFAULT_CONFIG;
int do_log_commands = 0;
void (*vty_ctrl_cmd)(int ctrl, struct vty *vty);

static void vty_buf_assert(struct vty *vty)
{
	assert(vty->cp <= vty->length);
	assert(vty->length < vty->max);
	assert(vty->buf[vty->length] == '\0');
}

/* Sanity/safety wrappers around access to vty->buf */
static void vty_buf_put(struct vty *vty, char c)
{
	vty_buf_assert(vty);
	vty->buf[vty->cp] = c;
	vty->buf[vty->max - 1] = '\0';
}

/* VTY standard output function. */
int vty_out(struct vty *vty, const char *format, ...)
{
	va_list args;
	int len = 0;
	int size = 1024;
	char buf[1024];
	char *p = NULL;

	if (vty_shell(vty))
	{
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
	else
	{
		/* Try to write to initial buffer.  */
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

		/* Pointer p must point out buffer. */
		buffer_put(vty->obuf, (u_char *) p, len);

		/* If p is not different with buf, it is allocated buffer.  */
		if (p != buf)
			XFREE(MTYPE_VTY_OUT_BUF, p);
	}

	return len;
}

static int vty_log_out(struct vty *vty, const char *level,
		const char *proto_str, const char *format, zlog_timestamp_t ctl,
		va_list va)
{
	int ret;
	int len = 0;
	char buf[1024];

	os_memset(buf, 0, sizeof(buf));
	len += quagga_timestamp(ctl, buf, sizeof(buf));

	if (len + 1 >= sizeof(buf))
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

	if ((ret < 0) || ((size_t)(len + ret) >= sizeof(buf)))
		return -1;

	len += ret;

	ret = vsnprintf(buf + len, sizeof(buf)-len, format, va);

	if ((ret < 0) || ((size_t)(len + ret + 2) > sizeof(buf)))
		return -1;

	len += ret;
	if (vty->type == VTY_TERM)
		buf[len++] = '\r';
	buf[len++] = '\n';
	if (vty->fd_type == IPCOM_STACK)
		ret = ip_write(vty->wfd, buf, len);
	else
		ret = write(vty->wfd, buf, len);
	if (ret < 0)
	{
		if (ERRNO_IO_RETRY(errno))
			/* Kernel buffer is full, probably too much debugging output, so just
			 drop the data and ignore. */
			return -1;
		/* Fatal I/O error. */
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(ZLOG_DEFAULT,
				"%s: write failed to vty client fd %d, closing: %s", __func__,
				vty->fd, safe_strerror(errno));
		buffer_reset(vty->obuf);
		/* cannot call vty_close, because a parent routine may still try
		 to access the vty struct */
		vty->status = VTY_CLOSE;
		if (vty->fd_type == OS_STACK)
			shutdown(vty->fd, SHUT_RDWR);
		else
			ip_shutdown(vty->fd, SHUT_RDWR);
		return -1;
	}
	return 0;
}

/* Output current time to the vty. */
void vty_time_print(struct vty *vty, int cr)
{
	char buf[QUAGGA_TIMESTAMP_LEN];

	if (quagga_timestamp(0, buf, sizeof(buf)) == 0)
	{
		zlog(ZLOG_DEFAULT, LOG_INFO, "quagga_timestamp error");
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
	if (host.motdfile)
	{
		FILE *f;
		char buf[4096];

		f = fopen(host.motdfile, "r");
		if (f)
		{
			while (fgets(buf, sizeof(buf), f))
			{
				char *s;
				/* work backwards to ignore trailling isspace() */
				for (s = buf + strlen(buf);
						(s > buf) && isspace((int) *(s - 1)); s--)
					;
				*s = '\0';
				vty_out(vty, "%s%s", buf, VTY_NEWLINE);
			}
			fclose(f);
		}
		else
			vty_out(vty, "MOTD file not found%s", VTY_NEWLINE);
	}
	else if (host.motd)
		vty_out(vty, "%s", host.motd);
}

/* Put out prompt and wait input from user. */
static void vty_prompt(struct vty *vty)
{
	struct utsname names;
	const char*hostname;

	if (vty->type == VTY_TERM)
	{
		hostname = host.name;
		if (!hostname)
		{
			uname(&names);
			hostname = names.nodename;
		}
		if(os_strlen(vty->prompt))
			vty_out(vty, cmd_prompt(vty->node), hostname, vty->prompt);
		else
			vty_out(vty, cmd_prompt(vty->node), hostname);
	}
}

/* Send WILL TELOPT_ECHO to remote server. */
static void vty_will_echo(struct vty *vty)
{
	unsigned char cmd[] =
	{ IAC, WILL, TELOPT_ECHO, '\0' };
	vty_out(vty, "%s", cmd);
}

/* Make suppress Go-Ahead telnet option. */
static void vty_will_suppress_go_ahead(struct vty *vty)
{
	unsigned char cmd[] =
	{ IAC, WILL, TELOPT_SGA, '\0' };
	vty_out(vty, "%s", cmd);
}

/* Make don't use linemode over telnet. */
static void vty_dont_linemode(struct vty *vty)
{
	unsigned char cmd[] =
	{ IAC, DONT, TELOPT_LINEMODE, '\0' };
	vty_out(vty, "%s", cmd);
}

/* Use window size. */
static void vty_do_window_size(struct vty *vty)
{
	unsigned char cmd[] =
	{ IAC, DO, TELOPT_NAWS, '\0' };
	vty_out(vty, "%s", cmd);
}

#if 0 /* Currently not used. */
/* Make don't use lflow vty interface. */
static void
vty_dont_lflow_ahead (struct vty *vty)
{
	unsigned char cmd[] =
	{	IAC, DONT, TELOPT_LFLOW, '\0'};
	vty_out (vty, "%s", cmd);
}
#endif /* 0 */

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
	return new;
}

/* Authentication of vty */
static void vty_auth(struct vty *vty, char *buf)
{
	enum node_type next_node = 0;
	switch (vty->node)
	{
	case USER_NODE:
		vty_user_setting(vty, buf);
		vty->node = AUTH_NODE;
		return;
	case AUTH_NODE:
		if (vty_user_authentication(vty, buf) == CMD_WARNING)
		{
			vty->node = USER_NODE;
			vty->fail++;
			if (vty->fail >= 3)
			{
				//if (vty->node == AUTH_NODE || vty->node == USER_NODE)
				{
					vty_out(vty, "%% Bad passwords, too many failures!%s",
							VTY_NEWLINE);
					vty->status = VTY_CLOSE;
				}
			}
			break;
		}
		//vty_user_update (vty);
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
				//if (vty->node == AUTH_NODE || vty->node == USER_NODE)
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
int vty_command(struct vty *vty, char *buf)
{
	int ret;
	vector vline;
	const char *protocolname;
	char *cp = NULL;
	if (host.cli_mutx)
		os_mutex_lock(host.cli_mutx, -1);
	/*
	 * Log non empty command lines
	 */
	if (do_log_commands)
		cp = buf;
	if (cp != NULL)
	{
		/* Skip white spaces. */
		while (isspace((int) *cp) && *cp != '\0')
			cp++;
	}
	if (cp != NULL && *cp != '\0')
	{
		unsigned i;
		char vty_str[VTY_BUFSIZ];
		char prompt_str[VTY_BUFSIZ];

		/* format the base vty info */
		snprintf(vty_str, sizeof(vty_str), "vty[??]@%s", vty->address);
		if (vty)
			for (i = 0; i < vector_active(vtyvec); i++)
				if (vty == vector_slot(vtyvec, i))
				{
					snprintf(vty_str, sizeof(vty_str), "vty[%d]@%s", i,
							vty->address);
					break;
				}

		/* format the prompt */
		snprintf(prompt_str, sizeof(prompt_str), cmd_prompt(vty->node),
				vty_str);

		/* now log the command */
		zlog(ZLOG_DEFAULT, LOG_ERR, "%s%s", prompt_str, buf);
	}

	if (vty_user_authorization(vty, buf) != CMD_SUCCESS)
	{
		vty_out(vty, "%% %s not authorization for %s %s", buf, vty->username,
				VTY_NEWLINE);
		if (host.cli_mutx)
			os_mutex_unlock(host.cli_mutx);
		return CMD_WARNING;
	}

	/* Split readline string up into the vector */
	vline = cmd_make_strvec(buf);

	if (vline == NULL)
	{
		if (host.cli_mutx)
			os_mutex_unlock(host.cli_mutx);
		return CMD_SUCCESS;
	}
#ifdef CONSUMED_TIME_CHECK
	{
		struct timeval before;
		struct timeval after;
		unsigned long realtime, cputime;

		os_get_relative(&before);
#endif /* CONSUMED_TIME_CHECK */

		ret = cmd_execute_command(vline, vty, NULL, 0);

		/* Get the name of the protocol if any */
		if (zlog_default)
			protocolname = zlog_proto_names(zlog_default->protocol);
		else
			protocolname = zlog_proto_names(ZLOG_NONE);

#ifdef CONSUMED_TIME_CHECK
		os_get_relative(&after);
		if ((realtime = thread_consumed_time(&after, &before, &cputime)) >
		CONSUMED_TIME_CHECK)
			/* Warn about CPU hog that must be fixed. */
			zlog_warn(ZLOG_DEFAULT,
					"SLOW COMMAND: command took %lums (cpu time %lums): %s",
					realtime / 1000, cputime / 1000, buf);
	}
#endif /* CONSUMED_TIME_CHECK */

	if (ret != CMD_SUCCESS)
		switch (ret)
		{
		case CMD_WARNING:
			//if (vty->type == VTY_FILE)
			vty_out(vty, "Warning...%s", VTY_NEWLINE);
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
	if (host.cli_mutx)
		os_mutex_unlock(host.cli_mutx);
	return ret;
}

static const char telnet_backward_char = 0x08;
static const char telnet_space_char = ' ';

/* Basic function to write buffer to vty. */
static void vty_write(struct vty *vty, const char *buf, size_t nbytes)
{
	if ((vty->node == AUTH_NODE) || (vty->node == AUTH_ENABLE_NODE))
		return;

	/* Should we do buffering here ?  And make vty_flush (vty) ? */
	buffer_put(vty->obuf, buf, nbytes);
}

/* Basic function to insert character into vty. */
void vty_self_insert(struct vty *vty, char c)
{
	int i;
	int length;

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
static void vty_self_insert_overwrite(struct vty *vty, char c)
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
static void vty_insert_word_overwrite(struct vty *vty, char *str)
{
	vty_buf_assert(vty);

	size_t nwrite = MIN((int ) strlen(str), vty->max - vty->cp - 1);
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

static void vty_kill_line_from_beginning(struct vty *);
static void vty_redraw_line(struct vty *);

/* Print command line history.  This function is called from
 vty_next_line and vty_previous_line. */
static void vty_history_print(struct vty *vty)
{
	int length;

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
	int try_index;

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
	int try_index;

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

	switch (vty->node)
	{
	case VIEW_NODE:
	case ENABLE_NODE:
		/*case RESTRICTED_NODE:*/
		/* Nothing to do. */
		break;
	case CONFIG_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE: /* Interface mode node. */
	case TUNNEL_INTERFACE_NODE: /* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE: /* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE: /* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE: /* Lag L3 Interface mode node. */
	case BRIGDE_INTERFACE_NODE:
#ifdef CUSTOM_INTERFACE
	case WIFI_INTERFACE_NODE:
	case MODEM_INTERFACE_NODE:
#endif
	case SERIAL_INTERFACE_NODE:
	case TRUNK_NODE:
	case ZEBRA_NODE:
	case RIP_NODE:
	case RIPNG_NODE:
	case BABEL_NODE:
	case BGP_NODE:
	case BGP_VPNV4_NODE:
	case BGP_VPNV6_NODE:
	case BGP_ENCAP_NODE:
	case BGP_ENCAPV6_NODE:
	case BGP_IPV4_NODE:
	case BGP_IPV4M_NODE:
	case BGP_IPV6_NODE:
	case BGP_IPV6M_NODE:
	case RMAP_NODE:
	case OSPF_NODE:
	case OSPF6_NODE:
	case ISIS_NODE:
	case KEYCHAIN_NODE:
	case KEYCHAIN_KEY_NODE:
	case MASC_NODE:
	case PIM_NODE:
	case VTY_NODE:
	case HSLS_NODE: /* HSLS protocol node. */
	case OLSR_NODE: /* OLSR protocol node. */
	case VRRP_NODE: /* ICRP protocol node. */
	case FRP_NODE: /* FRP protocol node */
	case LLDP_NODE:

	case BFD_NODE:
	case LDP_NODE:
	case VLAN_DATABASE_NODE:
	case VLAN_NODE:
		vty_config_unlock(vty);
		vty->node = ENABLE_NODE;
		break;
	default:
		/* Unknown node, we have to ignore it. */
		break;
	}

	vty_prompt(vty);
	vty->cp = 0;
}

/* Delete a charcter at the current point. */
static void vty_delete_char(struct vty *vty)
{
	int i;
	int size;

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
	int i;
	int size;

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
	char c1, c2;

	/* If length is short or point is near by the beginning of line then
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
	int i;
	int ret;
	char **matched = NULL;
	vector vline;

	if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
		return;

	if (vty->node == USER_NODE)
		return;

	vline = cmd_make_strvec(vty->buf);
	if (vline == NULL)
		return;

	/* In case of 'help \t'. */
	if (isspace((int) vty->buf[vty->length - 1]))
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

static void vty_describe_fold(struct vty *vty, int cmd_width,
		unsigned int desc_width, struct cmd_token *token)
{
	char *buf;
	const char *cmd, *p;
	int pos;

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
			if (*(p + pos) == ' ')
				break;

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
	unsigned int i, width, desc_width;
	struct cmd_token *token, *token_cr = NULL;

	vline = cmd_make_strvec(vty->buf);

	/* In case of '> ?'. */
	if (vline == NULL)
	{
		vline = vector_init(1);
		vector_set(vline, NULL);
	}
	else if (isspace((int) vty->buf[vty->length - 1]))
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
		if ((token = vector_slot(describe, i)) != NULL)
		{
			unsigned int len;

			if (token->cmd[0] == '\0')
				continue;

			len = strlen(token->cmd);
			if (token->cmd[0] == '.')
				len--;

			if (width < len)
				width = len;
		}

	/* Get width of description string. */
	desc_width = vty->width - (width + 6);

	/* Print out description. */
	for (i = 0; i < vector_active(describe); i++)
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

	out: cmd_free_strvec(vline);
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

	switch (vty->node)
	{
	case VIEW_NODE:
	case ENABLE_NODE:
		/*case RESTRICTED_NODE:*/
		/* Nothing to do. */
		break;
	case CONFIG_NODE:
	case MODEM_PROFILE_NODE:
	case MODEM_CHANNEL_NODE:
	case INTERFACE_NODE:
	case INTERFACE_L3_NODE: /* Interface mode node. */
	case TUNNEL_INTERFACE_NODE: /* Tunnel Interface mode node. */
	case LOOPBACK_INTERFACE_NODE: /* Loopback Interface mode node. */
	case LAG_INTERFACE_NODE: /* Lag Interface mode node. */
	case LAG_INTERFACE_L3_NODE: /* Lag L3 Interface mode node. */
	case BRIGDE_INTERFACE_NODE:
#ifdef CUSTOM_INTERFACE
	case WIFI_INTERFACE_NODE:
	case MODEM_INTERFACE_NODE:
#endif
	case SERIAL_INTERFACE_NODE:
	case TRUNK_NODE:
	case ZEBRA_NODE:
	case RIP_NODE:
	case RIPNG_NODE:
	case BABEL_NODE:
	case BGP_NODE:
	case RMAP_NODE:
	case OSPF_NODE:
	case OSPF6_NODE:
	case ISIS_NODE:
	case KEYCHAIN_NODE:
	case KEYCHAIN_KEY_NODE:
	case MASC_NODE:
	case PIM_NODE:
	case VTY_NODE:

	case HSLS_NODE: /* HSLS protocol node. */
	case OLSR_NODE: /* OLSR protocol node. */
	case VRRP_NODE: /* ICRP protocol node. */
	case FRP_NODE: /* FRP protocol node */
	case LLDP_NODE:

	case BFD_NODE:
	case LDP_NODE:
	case VRF_NODE:
	case VLAN_DATABASE_NODE:
	case VLAN_NODE:
		vty_config_unlock(vty);
		vty->node = ENABLE_NODE;
		break;
	default:
		/* Unknown node, we have to ignore it. */
		break;
	}
	vty_prompt(vty);

	/* Set history pointer to the latest one. */
	vty->hp = vty->hindex;
}

/* Add current command line to the history buffer. */
static void vty_hist_add(struct vty *vty)
{
	int index;

	if (vty->length == 0)
		return;

	index = vty->hindex ? vty->hindex - 1 : VTY_MAXHIST - 1;

	/* Ignore the same string as previous one. */
	if (vty->hist[index])
		if (strcmp(vty->buf, vty->hist[index]) == 0)
		{
			vty->hp = vty->hindex;
			return;
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
static int vty_telnet_option(struct vty *vty, unsigned char *buf, int nbytes)
{
#ifdef TELNET_OPTION_DEBUG
	int i;

	for (i = 0; i < nbytes; i++)
	{
		switch (buf[i])
		{
			case IAC:
			vty_out (vty, "IAC ");
			break;
			case WILL:
			vty_out (vty, "WILL ");
			break;
			case WONT:
			vty_out (vty, "WONT ");
			break;
			case DO:
			vty_out (vty, "DO ");
			break;
			case DONT:
			vty_out (vty, "DONT ");
			break;
			case SB:
			vty_out (vty, "SB ");
			break;
			case SE:
			vty_out (vty, "SE ");
			break;
			case TELOPT_ECHO:
			vty_out (vty, "TELOPT_ECHO %s", VTY_NEWLINE);
			break;
			case TELOPT_SGA:
			vty_out (vty, "TELOPT_SGA %s", VTY_NEWLINE);
			break;
			case TELOPT_NAWS:
			vty_out (vty, "TELOPT_NAWS %s", VTY_NEWLINE);
			break;
			default:
			vty_out (vty, "%x ", buf[i]);
			break;
		}
	}
	vty_out (vty, "%s", VTY_NEWLINE);

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
				zlog_warn(ZLOG_DEFAULT,
						"RFC 1073 violation detected: telnet NAWS option "
								"should send %d characters, but we received %lu",
						TELNET_NAWS_SB_LEN, (u_long) vty->sb_len);
			else if (sizeof(vty->sb_buf) < TELNET_NAWS_SB_LEN)
				zlog_err(ZLOG_DEFAULT,
						"Bug detected: sizeof(vty->sb_buf) %lu < %d, "
								"too small to handle the telnet NAWS option",
						(u_long) sizeof(vty->sb_buf), TELNET_NAWS_SB_LEN);
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
	case USER_NODE:
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

#define CONTROL(X)  ((X) - '@')
#define VTY_NORMAL     0
#define VTY_PRE_ESCAPE 1  /* Esc seen */
#define VTY_ESCAPE     2  /* ANSI terminal escape (Esc-[) seen */
#define VTY_LITERAL    3  /* Next char taken as literal */

/* Escape character command map. */
static void vty_escape_map(unsigned char c, struct vty *vty)
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
static void vty_ctrl_default(int ctrl, struct vty *vty)
{
#ifndef CONTROL
#define CONTROL(X)  ((X) - '@')
#endif

	//执锟斤拷ctrl + c锟斤拷氐锟斤拷锟斤拷锟�
	//pid_t pid;//锟斤拷前锟斤拷锟斤拷锟斤拷锟叫的斤拷锟斤拷
	//pthread_t pthd;//锟斤拷前锟斤拷锟斤拷锟斤拷锟叫碉拷锟竭筹拷
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
			//vty_terminate ();
			//exit(1);
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
		//vty_out (vty, "ctrl + c%s", VTY_NEWLINE);
		break;
	case CONTROL('D'):
		//vty_out (vty, "ctrl + d%s", VTY_NEWLINE);
		break;
	case CONTROL('Z'):
		if (vty->node <= AUTH_NODE)
		{
			kill(getpid(), SIGTERM);
			//pthread_kill(pstTcb->taskId, SIGSTOP);
			//vty_terminate ();
			//exit(1);
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
		//vty_out (vty, "ctrl + z%s", VTY_NEWLINE);
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
	if (vty->fd_type == OS_STACK
			|| os_memcmp(vty->address, "console", os_strlen("console")))
	{
		return getchar();
	}
	else
	{
//		int ret;
		int nbytes;
		unsigned char buf[VTY_READ_BUFSIZ];
		while (1)
		{
			if ((nbytes = ip_read(vty->fd, buf, VTY_READ_BUFSIZ)) <= 0)
			{
				if (nbytes < 0)
				{
					if (ERRNO_IO_RETRY(errno))
					{
						//vty_event(VTYSH_READ, vty->fd, vty);
						continue;
					}
					vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
					zlog_warn(ZLOG_DEFAULT,
							"%s: read failed on vtysh client fd %d, closing: %s",
							__func__, vty->fd, safe_strerror(errno));
					return -1;
				}
			}
			else
			{
				break;
			}
		}
		return (int) buf[0];
	}
}

static int vty_read_handle(struct vty *vty, unsigned char *buf, int len)
{
	int i;
	int nbytes = len;
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

		if (vty_ctrl_cmd != NULL)
			(vty_ctrl_cmd)(buf[i], vty);

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
		case '?':
			if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
				vty_self_insert(vty, buf[i]);
			else
				vty_describe_command(vty);
			break;
		case '\033':
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

	/* Check status. */
	/*	if (vty->status == VTY_CLOSE)
	 vty_close(vty);
	 else {
	 vty_event(VTY_WRITE, vty->wfd, vty);
	 vty_event(VTY_READ, vty->fd, vty);
	 }*/
	return 0;
}

#if 1
static int vty_read(struct thread *thread)
{
	int i;
	int nbytes;
	unsigned char buf[VTY_READ_BUFSIZ];
	int vty_sock = THREAD_FD(thread);
	struct vty *vty = THREAD_ARG(thread);
	vty->t_read = NULL;
	nbytes = ip_read(vty->fd, buf, VTY_READ_BUFSIZ);
	/* Read raw data from socket */
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				vty_event(VTY_READ, vty_sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(ZLOG_DEFAULT,
					"%s: read error on vty client fd %d, closing: %s", __func__,
					vty->fd, safe_strerror(errno));
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

static int vty_console_read(struct thread *thread)
{
	int i;
	int nbytes;
	unsigned char buf[VTY_READ_BUFSIZ];
	int vty_sock = THREAD_FD(thread);
	struct vty *vty = THREAD_ARG(thread);
	vty->t_read = NULL;
	nbytes = read(vty->fd, buf, VTY_READ_BUFSIZ);
	/* Read raw data from socket */
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				vty_event(VTY_READ, vty_sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(ZLOG_DEFAULT,
					"%s: read error on vty client fd %d, closing: %s", __func__,
					vty->fd, safe_strerror(errno));
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

#else
/* Read data via vty socket. */
static int
vty_read (struct thread *thread)
{
	int i;
	int nbytes;
	unsigned char buf[VTY_READ_BUFSIZ];

	int vty_sock = THREAD_FD(thread);
	struct vty *vty = THREAD_ARG(thread);
	vty->t_read = NULL;
	if (vty->fd_type == OS_STACK)
	nbytes = read(vty->fd, buf, VTY_READ_BUFSIZ);
	else
	nbytes = ip_read(vty->fd, buf, VTY_READ_BUFSIZ);
	/* Read raw data from socket */
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				vty_event(VTY_READ, vty_sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(ZLOG_DEFAULT,
					"%s: read error on vty client fd %d, closing: %s", __func__,
					vty->fd, safe_strerror(errno));
			buffer_reset(vty->obuf);
		}
		vty->status = VTY_CLOSE;
	}

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

		if (vty_ctrl_cmd != NULL)
		(vty_ctrl_cmd)(buf[i], vty);

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
			case '?':
			if (vty->node == AUTH_NODE || vty->node == AUTH_ENABLE_NODE)
			vty_self_insert(vty, buf[i]);
			else
			vty_describe_command(vty);
			break;
			case '\033':
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

	/* Check status. */
	if (vty->status == VTY_CLOSE)
	vty_close(vty);
	else
	{
		vty_event(VTY_WRITE, vty->wfd, vty);
		vty_event(VTY_READ, vty_sock, vty);
	}
	return 0;
}
#endif

/* Flush buffer to the vty. */
static int vty_flush_handle(struct vty *vty, int vty_sock)
{
	int erase;
	int type = 0;
	buffer_status_t flushrc;
	//int vty_sock = THREAD_FD (thread);
	//struct vty *vty = THREAD_ARG (thread);

	vty->t_write = NULL;

	/* Tempolary disable read thread. */
	if (vty->fd_type == OS_STACK || vty->type == VTY_FILE)
	{
		if ((vty->lines == 0) && vty->t_read)
		{
			thread_cancel(vty->t_read);
			vty->t_read = NULL;
		}
		type = OS_STACK;
	}
	else
	{
		if ((vty->lines == 0) && vty->t_read)
		{
			eloop_cancel(vty->t_read);
			vty->t_read = NULL;
		}
		type = IPCOM_STACK;
	}
	/* Function execution continue. */
	erase = ((vty->status == VTY_MORE || vty->status == VTY_MORELINE));
	/* N.B. if width is 0, that means we don't know the window size. */
	if ((vty->lines == 0) || (vty->width == 0) || (vty->height == 0))
		flushrc = buffer_flush_available(vty->obuf, vty_sock, type);
	else if (vty->status == VTY_MORELINE)
		flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width, 1, erase,
				0, type);
	else
		flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width,
				vty->lines >= 0 ? vty->lines : vty->height, erase, 0, type);
	switch (flushrc)
	{
	case BUFFER_ERROR:
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(ZLOG_DEFAULT,
				"buffer_flush failed on vty client fd %d, closing", vty->fd);
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
				vty_event(VTY_READ, vty_sock, vty);
		}
		break;
	case BUFFER_PENDING:
		/* There is more data waiting to be written. */
		vty->status = VTY_MORE;
		if (vty->lines == 0)
			vty_event(VTY_WRITE, vty_sock, vty);
		break;
	}

	return 0;
}

static int vty_flush(struct thread *thread)
{
	//int erase;
	//int type = 0;
	//buffer_status_t flushrc;
	int vty_sock = THREAD_FD(thread);
	struct vty *vty = THREAD_ARG(thread);

	vty->t_write = NULL;
	vty_flush_handle(vty, vty_sock);
#if 0
	/* Tempolary disable read thread. */
	if ((vty->lines == 0) && vty->t_read)
	{
		thread_cancel (vty->t_read);
		vty->t_read = NULL;
	}
	if(vty->fd_type == OS_STACK || vty->type == VTY_FILE)
	type = OS_STACK;
	else
	type = IPCOM_STACK;
	/* Function execution continue. */
	erase = ((vty->status == VTY_MORE || vty->status == VTY_MORELINE));

	/* N.B. if width is 0, that means we don't know the window size. */
	if ((vty->lines == 0) || (vty->width == 0) || (vty->height == 0))
	flushrc = buffer_flush_available(vty->obuf, vty_sock, type);
	else if (vty->status == VTY_MORELINE)
	flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width,
			1, erase, 0, type);
	else
	flushrc = buffer_flush_window(vty->obuf, vty_sock, vty->width,
			vty->lines >= 0 ? vty->lines :
			vty->height,
			erase, 0, type);
	switch (flushrc)
	{
		case BUFFER_ERROR:
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(ZLOG_DEFAULT, "buffer_flush failed on vty client fd %d, closing",
				vty->fd);
		buffer_reset(vty->obuf);
		vty_close(vty);
		return 0;
		case BUFFER_EMPTY:
		if (vty->status == VTY_CLOSE)
		vty_close (vty);
		else
		{
			vty->status = VTY_NORMAL;
			if (vty->lines == 0)
			vty_event (VTY_READ, vty_sock, vty);
		}
		break;
		case BUFFER_PENDING:
		/* There is more data waiting to be written. */
		vty->status = VTY_MORE;
		if (vty->lines == 0)
		vty_event (VTY_WRITE, vty_sock, vty);
		break;
	}
#endif
	return 0;
}
static int vty_console_flush(struct thread *thread)
{
	//int erase;
	//int type = 0;
	//buffer_status_t flushrc;
	int vty_sock = THREAD_FD(thread);
	struct vty *vty = THREAD_ARG(thread);

	vty->t_write = NULL;
	vty_flush_handle(vty, vty_sock);
	return 0;
}
/* allocate and initialise vty */
static struct vty *
vty_new_init(int vty_sock)
{
	struct vty *vty;

	vty = vty_new();
	vty->fd = vty_sock;
	vty->wfd = vty_sock;
	vty->type = VTY_TERM;
	vty->node = AUTH_NODE;
	vty->fail = 0;
	vty->cp = 0;
	vty_clear_buf(vty);
	vty->length = 0;
	memset(vty->hist, 0, sizeof(vty->hist));
	vty->hp = 0;
	vty->hindex = 0;
	vector_set_index(vtyvec, vty_sock, vty);
	vty->status = VTY_NORMAL;
	vty->lines = -1;
	vty->iac = 0;
	vty->iac_sb_in_progress = 0;
	vty->sb_len = 0;
	//if(vty_sock <= 3)
	vty->fd_type = OS_STACK;
	return vty;
}

/* Create new vty structure. */
static struct vty *
vty_create(int vty_sock, union sockunion *su)
{
	char buf[SU_ADDRSTRLEN];
	struct vty *vty;

	sockunion2str(su, buf, SU_ADDRSTRLEN);

	/* Allocate new vty structure and set up default values. */
	vty = vty_new_init(vty_sock);

	/* configurable parameters not part of basic init */
	//vty->v_timeout = host.vty_timeout_val;
	host_config_get_api(API_GET_VTY_TIMEOUT_CMD, &vty->v_timeout);
	strcpy(vty->address, buf);

	vty->node = USER_NODE;
	vty->fd_type = IPCOM_STACK;
	if (host.lines >= 0)
		host_config_get_api(API_GET_LINES_CMD, &vty->lines);
	//vty->lines = host.lines;

	/* Say hello to the world. */
	vty_hello(vty);
	if (!host.no_password_check)
		vty_out(vty, "%sUser Access Verification%s%s", VTY_NEWLINE, VTY_NEWLINE,
				VTY_NEWLINE);

	/* Setting up terminal. */
	vty_will_echo(vty);
	vty_will_suppress_go_ahead(vty);

	vty_dont_linemode(vty);
	vty_do_window_size(vty);
	/* vty_dont_lflow_ahead (vty); */

	vty_prompt(vty);

	/* Add read/write thread. */
	vty_event(VTY_WRITE, vty_sock, vty);
	vty_event(VTY_READ, vty_sock, vty);

	return vty;
}

/* create vty for console */

static int vty_console_create(void)
{
	struct termios termios;
	if (vty_tty_console.console == NULL)
		return -1;
	if (!tcgetattr(vty_tty_console.console->fd, &vty_tty_console.termios_console))
	{
		termios = vty_tty_console.termios_console;
		if(!FD_IS_STDOUT(vty_tty_console.console->fd))
		{
			termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR
					| ICRNL | IXON);
			termios.c_oflag &= ~OPOST;
			termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			termios.c_cflag &= ~(CSIZE | PARENB);
			termios.c_cflag |= CS8;
		}
		else
			termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK |
					ECHONL | ECHOPRT | ECHOKE | ICRNL);
			//termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

		tcsetattr(vty_tty_console.console->fd, TCSANOW, &termios);
	}
	return 0;
}

static int vty_console_wait(struct thread *thread)
{
	struct vty *vty = THREAD_ARG(thread);
	if (host.load != LOAD_DONE)
	{
		if (vty_tty_console.t_wait)
			thread_cancel(vty_tty_console.t_wait);
		if (thread_master)
			vty_tty_console.t_wait = thread_add_timer(thread_master,
					vty_console_wait, vty, 1);
		return OK;
	}
	if (vty && vty_console_create() == 0)
	{
		/*
		 vty_out (vty, " Please Enter To Start Console CLI%s",  VTY_NEWLINE);
		 vty_event (VTY_WRITE, STDOUT_FILENO, vty);
		 */
		fprintf(stdout, "\r\n\r\nPlease Enter To Start Console CLI\r\n");
		fflush(stdout);
		vty_event(VTY_STDIO_ACCEPT, vty->fd, vty);
	}
//	fprintf(stdout,"%s:\r\n",__func__);
	return 0;
}

static int vty_console_accept(struct thread *thread)
{
	int c;
	struct vty *vty = THREAD_ARG(thread);
	if (!vty)
		return ERROR;
	c = getchar();
	//if(vty->node <= AUTH_NODE)
	{
		vty_ctrl_default(c, vty);
		//exit(1);
	}
	vty_hello(vty);
	vty_prompt(vty);
	/* Add read/write thread. */

	vty_event(VTY_WRITE, vty->fd, vty);
	vty_event(VTY_READ, vty->fd, vty);
	//vty_event(VTY_WRITE, STDOUT_FILENO, vty);
	//vty_event(VTY_READ, STDIN_FILENO, vty);
	return 0;
}

static void vty_console_close(int close)
{
	if (vty_tty_console.console)
	{
		if (close)
		{
			int i;
			/* Cancel threads.*/
			if (vty_tty_console.console->t_read)
				thread_cancel(vty_tty_console.console->t_read);
			if (vty_tty_console.console->t_write)
				thread_cancel(vty_tty_console.console->t_write);
			if (vty_tty_console.console->t_timeout)
				thread_cancel(vty_tty_console.console->t_timeout);

			/* Flush buffer. */
			buffer_flush_all(vty_tty_console.console->obuf,
					vty_tty_console.console->wfd, OS_STACK);

			/* Free input buffer. */
			buffer_free(vty_tty_console.console->obuf);

			/* Free command history. */
			for (i = 0; i < VTY_MAXHIST; i++)
				if (vty_tty_console.console->hist[i])
					XFREE(MTYPE_VTY_HIST, vty_tty_console.console->hist[i]);

			/* Unset vector. */
			vector_unset(vtyvec, vty_tty_console.console->fd);
		}
		/* Close socket. */
		//if(!FD_IS_STDOUT(vty_tty_console.console->fd))
		tcsetattr(vty_tty_console.console->fd, TCSANOW, &vty_tty_console.termios_console);

		//console = NULL;
		if (vty_tty_console.console_atclose)
			vty_tty_console.console_atclose();
		//console_atclose = NULL;
		if (close)
		{
			if (vty_tty_console.console->buf)
				XFREE(MTYPE_VTY, vty_tty_console.console->buf);

			/* Check configure. */
			vty_config_unlock(vty_tty_console.console);

			/* OK free vty. */
			XFREE(MTYPE_VTY, vty_tty_console.console);
		}
		vty_tty_console.console = NULL;
	}
}

static void vty_console_reset(void)
{
	if (vty_tty_console.console)
	{
		vty_console_close(1);
		vty_console(vty_tty_console.tty, vty_tty_console.console_atclose);
		vty_console_create();
		struct vty *vty = vty_tty_console.console;
		if (vty_tty_console.t_wait)
		{
			thread_cancel(vty_tty_console.t_wait);
			vty_tty_console.t_wait = NULL;
		}
		/*
		 vty_out (vty, " Please Enter To Start Console CLI%s",  VTY_NEWLINE);
		 vty_event (VTY_WRITE, STDOUT_FILENO, vty);
		 */
		fprintf(stdout, "\r\nPlease Enter To Start Console CLI\r\n");
		fflush(stdout);
		vty_event(VTY_STDIO_ACCEPT, vty->fd, vty);
	}
}

int vty_console(const char *tty, void (*atclose)())
{
	int ttyfd = STDIN_FILENO;
	if(tty)
		ttyfd = open(tty, 0, O_RDWR|O_NOCTTY|O_NDELAY);
	/* refuse creating two vtys on console */
	if (vty_tty_console.console == NULL)
	{
		vty_tty_console.console = vty_new_init(ttyfd);
		if (vty_tty_console.console == NULL)
			return ERROR;
	}
	if(tty)
	{
		if(vty_tty_console.tty)
			free(vty_tty_console.tty);
		vty_tty_console.tty = strdup(tty);
	}
	vty_tty_console.console->fd_type = OS_STACK;
	vty_tty_console.console_atclose = atclose;
	vty_tty_console.console->wfd = tty ? ttyfd : STDOUT_FILENO;
	/* always have console vty in a known _unchangeable_ state, don't want config
	 * to have any effect here to make sure scripting this works as intended */
	vty_tty_console.console->node = USER_NODE;  //ENABLE_NODE;
	vty_tty_console.console->v_timeout = 120;
	//vty->v_timeout = host.vty_timeout_val;
	strcpy(vty_tty_console.console->address, "console");

	vty_event(VTY_STDIO_WAIT, ttyfd, vty_tty_console.console);

	return OK;
}
/* Accept connection from the network. */
static int vty_accept(struct eloop *thread)
{
	int vty_sock;
	union sockunion su;
	int ret;
	unsigned int on;
	int accept_sock;
	struct prefix p;
	struct access_list *acl = NULL;
	char buf[SU_ADDRSTRLEN];

	accept_sock = ELOOP_FD(thread);

	/* We continue hearing vty socket. */
	vty_event(VTY_SERV, accept_sock, NULL);

	memset(&su, 0, sizeof(union sockunion));

	/* We can handle IPv4 or IPv6 socket. */
	vty_sock = sockunion_accept(accept_sock, &su);
	if (vty_sock < 0)
	{
		zlog_warn(ZLOG_DEFAULT, "can't accept vty socket : %s",
				safe_strerror(errno));
		return -1;
	}
	set_nonblocking(vty_sock);

	sockunion2hostprefix(&su, &p);
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	/* VTY's accesslist apply. */
	if (p.family == AF_INET && host.vty_accesslist_name)
	{
		if ((acl = access_list_lookup(AFI_IP, host.vty_accesslist_name))
				&& (access_list_apply(acl, &p) == FILTER_DENY))
		{
			zlog(ZLOG_DEFAULT, LOG_INFO, "Vty connection refused from %s",
					sockunion2str(&su, buf, SU_ADDRSTRLEN));
			close(vty_sock);
			if (host.mutx)
				os_mutex_unlock(host.mutx);
			/* continue accepting connections */
			vty_event(VTY_SERV, accept_sock, NULL);
			return 0;
		}
	}

#ifdef HAVE_IPV6
	/* VTY's ipv6 accesslist apply. */
	if (p.family == AF_INET6 && host.vty_ipv6_accesslist_name)
	{
		if ((acl = access_list_lookup (AFI_IP6, host.vty_ipv6_accesslist_name)) &&
				(access_list_apply (acl, &p) == FILTER_DENY))
		{
			zlog (ZLOG_DEFAULT, LOG_INFO, "Vty connection refused from %s",
					sockunion2str (&su, buf, SU_ADDRSTRLEN));
			close (vty_sock);
			if (host.mutx)
			os_mutex_unlock(host.mutx);
			/* continue accepting connections */
			vty_event (VTY_SERV, accept_sock, NULL);

			return 0;
		}
	}
#endif /* HAVE_IPV6 */

	if (host.mutx)
		os_mutex_unlock(host.mutx);
	on = 1;
	ret = ip_setsockopt(vty_sock, IPPROTO_TCP, TCP_NODELAY, (char *) &on,
			sizeof(on));
	if (ret < 0)
		zlog(ZLOG_DEFAULT, LOG_INFO, "can't set sockopt to vty_sock : %s",
				safe_strerror(errno));

	zlog(ZLOG_DEFAULT, LOG_INFO, "Vty connection from %s",
			sockunion2str(&su, buf, SU_ADDRSTRLEN));

	vty_create(vty_sock, &su);

	return 0;
}

/* Make vty server socket. */
static void vty_serv_sock_family(const char* addr, unsigned short port,
		int family)
{
	int ret;
	union sockunion su;
	int accept_sock;
	void* naddr = NULL;

	memset(&su, 0, sizeof(union sockunion));
	su.sa.sa_family = family;
	if (addr)
		switch (family)
		{
		case AF_INET:
			naddr = &su.sin.sin_addr;
			break;
#ifdef HAVE_IPV6
			case AF_INET6:
			naddr=&su.sin6.sin6_addr;
			break;
#endif	
		}

	if (naddr)
		switch (inet_pton(family, addr, naddr))
		{
		case -1:
			zlog_err(ZLOG_DEFAULT, "bad address %s", addr);
			naddr = NULL;
			break;
		case 0:
			zlog_err(ZLOG_DEFAULT, "error translating address %s: %s", addr,
					safe_strerror(errno));
			naddr = NULL;
		}

	/* Make new socket. */
	accept_sock = sockunion_stream_socket(&su);
	if (accept_sock < 0)
		return;

	/* This is server, so reuse address. */
	sockopt_reuseaddr(accept_sock);
	sockopt_reuseport(accept_sock);

	/* Bind socket to universal address and given port. */
	ret = sockunion_bind(accept_sock, &su, port, naddr);
	if (ret < 0)
	{
		zlog_warn(ZLOG_DEFAULT, "can't bind socket");
		ip_close(accept_sock); /* Avoid sd leak. */
		return;
	}

	/* Listen socket under queue 3. */
	ret = ip_listen(accept_sock, 3);
	if (ret < 0)
	{
		zlog(ZLOG_DEFAULT, LOG_WARNING, "can't listen socket(%s)",
				safe_strerror(errno));
		ip_close(accept_sock); /* Avoid sd leak. */
		return;
	}

	/* Add vty server event. */
	vty_event(VTY_SERV, accept_sock, NULL);
}

#ifdef VTYSH
/* For sockaddr_un. */
#include <sys/un.h>
/* VTY shell UNIX domain socket. */
static void
vty_serv_un (const char *path)
{
	int ret;
	int sock, len;
	struct sockaddr_un serv;
	mode_t old_mask;

	/* First of all, unlink existing socket */
	unlink (path);

	/* Set umask */
	old_mask = umask (0007);

	/* Make UNIX domain socket. */
	sock = ip_socket (AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		zlog_err(ZLOG_DEFAULT, "Cannot create unix stream socket: %s", safe_strerror(errno));
		return;
	}

	/* Make server socket. */
	memset (&serv, 0, sizeof (struct sockaddr_un));
	serv.sun_family = AF_UNIX;
	strncpy (serv.sun_path, path, strlen (path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
	len = serv.sun_len = SUN_LEN(&serv);
#else
	len = sizeof (serv.sun_family) + strlen (serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

	ret = ip_bind (sock, (struct sockaddr *) &serv, len);
	if (ret < 0)
	{
		zlog_err(ZLOG_DEFAULT, "Cannot bind path %s: %s", path, safe_strerror(errno));
		ip_close (sock); /* Avoid sd leak. */
		return;
	}

	ret = ip_listen (sock, 5);
	if (ret < 0)
	{
		zlog_err(ZLOG_DEFAULT, "listen(fd %d) failed: %s", sock, safe_strerror(errno));
		ip_close (sock); /* Avoid sd leak. */
		return;
	}

	umask (old_mask);

	vty_event (VTYSH_SERV, sock, NULL);
}

/* #define VTYSH_DEBUG 1 */

static int
vtysh_accept (struct thread *thread)
{
	int accept_sock;
	int sock;
	int client_len;
	struct sockaddr_un client;
	struct vty *vty;

	accept_sock = THREAD_FD (thread);

	vty_event (VTYSH_SERV, accept_sock, NULL);

	memset (&client, 0, sizeof (struct sockaddr_un));
	client_len = sizeof (struct sockaddr_un);

	sock = ip_accept (accept_sock, (struct sockaddr *) &client,
			(socklen_t *) &client_len);

	if (sock < 0)
	{
		zlog_warn (ZLOG_DEFAULT, "can't accept vty socket : %s", safe_strerror (errno));
		return -1;
	}

	if (set_nonblocking(sock) < 0)
	{
		zlog_warn (ZLOG_DEFAULT, "vtysh_accept: could not set vty socket %d to non-blocking,"
				" %s, closing", sock, safe_strerror (errno));
		ip_close (sock);
		return -1;
	}

#ifdef VTYSH_DEBUG
	printf ("VTY shell accept\n");
#endif /* VTYSH_DEBUG */

	vty = vty_new ();
	vty->fd = sock;
	vty->wfd = sock;
	vty->type = VTY_SHELL_SERV;
	vty->node = VIEW_NODE;

	vty_event (VTYSH_READ, sock, vty);

	return 0;
}

static int
vtysh_flush(struct vty *vty)
{
	switch (buffer_flush_available(vty->obuf, vty->wfd, 0))
	{
		case BUFFER_PENDING:
		vty_event(VTYSH_WRITE, vty->wfd, vty);
		break;
		case BUFFER_ERROR:
		vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
		zlog_warn(ZLOG_DEFAULT, "%s: write error to fd %d, closing", __func__, vty->fd);
		buffer_reset(vty->obuf);
		vty_close(vty);
		return -1;
		break;
		case BUFFER_EMPTY:
		break;
	}
	return 0;
}

static int
vtysh_read (struct thread *thread)
{
	int ret;
	int sock;
	int nbytes;
	struct vty *vty;
	unsigned char buf[VTY_READ_BUFSIZ];
	unsigned char *p;
	u_char header[4] =
	{	0, 0, 0, 0};

	sock = THREAD_FD (thread);
	vty = THREAD_ARG (thread);
	vty->t_read = NULL;

	if ((nbytes = ip_read (sock, buf, VTY_READ_BUFSIZ)) <= 0)
	{
		if (nbytes < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				vty_event (VTYSH_READ, sock, vty);
				return 0;
			}
			vty->trapping = vty->monitor = 0; /* disable monitoring to avoid infinite recursion */
			zlog_warn(ZLOG_DEFAULT, "%s: read failed on vtysh client fd %d, closing: %s",
					__func__, sock, safe_strerror(errno));
		}
		buffer_reset(vty->obuf);
		vty_close (vty);
#ifdef VTYSH_DEBUG
		printf ("close vtysh\n");
#endif /* VTYSH_DEBUG */
		return 0;
	}

#ifdef VTYSH_DEBUG
	printf ("line: %.*s\n", nbytes, buf);
#endif /* VTYSH_DEBUG */

	if (vty->length + nbytes >= vty->max)
	{
		/* Clear command line buffer. */
		vty->cp = vty->length = 0;
		vty_clear_buf (vty);
		vty_out (vty, "%% Command is too long.%s", VTY_NEWLINE);
		goto out;
	}

	for (p = buf; p < buf+nbytes; p++)
	{
		vty->buf[vty->length++] = *p;
		if (*p == '\0')
		{

			/* Pass this line to parser. */
			ret = vty_execute (vty);
			/* Note that vty_execute clears the command buffer and resets
			 vty->length to 0. */

			/* Return result. */
#ifdef VTYSH_DEBUG
			printf ("result: %d\n", ret);
			printf ("vtysh node: %d\n", vty->node);
#endif /* VTYSH_DEBUG */

			header[3] = ret;
			buffer_put(vty->obuf, header, 4);

			if (!vty->t_write && (vtysh_flush(vty) < 0))
			/* Try to flush results; exit if a write error occurs. */
			return 0;
		}
	}

	out:
	vty_event (VTYSH_READ, sock, vty);

	return 0;
}

static int
vtysh_write (struct thread *thread)
{
	struct vty *vty = THREAD_ARG (thread);

	vty->t_write = NULL;
	vtysh_flush(vty);
	return 0;
}

#endif /* VTYSH */

/* Determine address family to bind. */
void vty_serv_init(const char *addr, unsigned short port, const char *path, const char *tty)
{
	/* If port is set to 0, do not listen on TCP/IP at all! */
	if (port)
	{

#if 0//def HAVE_IPV6
		vty_serv_sock_addrinfo (addr, port);
#else /* ! HAVE_IPV6 */
		vty_serv_sock_family(addr, port, AF_INET);
#endif /* HAVE_IPV6 */
	}

#ifdef VTYSH
	vty_serv_un (path);
#endif /* VTYSH */
	vty_console(tty, NULL);
}

/* Close vty interface.  Warning: call this only from functions that
 will be careful not to access the vty afterwards (since it has
 now been freed).  This is safest from top-level functions (called
 directly by the thread dispatcher). */
void vty_close(struct vty *vty)
{
	int i;
	int type = 0;
	if (vty->fd_type == OS_STACK || vty->type == VTY_FILE)
	{
		/* Cancel threads.*/
		if (vty->t_read)
			thread_cancel(vty->t_read);
		if (vty->t_write)
			thread_cancel(vty->t_write);
		if (vty->t_timeout)
			thread_cancel(vty->t_timeout);

		type = OS_STACK;
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

		type = IPCOM_STACK;
	}
	/* Flush buffer. */
	if(vty->type != VTY_FILE)
		buffer_flush_all(vty->obuf, vty->wfd, type);

	/* Free input buffer. */
	buffer_free(vty->obuf);

	/* Free command history. */
	if(vty->type != VTY_FILE)
	{
		for (i = 0; i < VTY_MAXHIST; i++)
			if (vty->hist[i])
				XFREE(MTYPE_VTY_HIST, vty->hist[i]);
	}
	/* Unset vector. */
	if(vty->type != VTY_FILE)
		vector_unset(vtyvec, vty->fd);
	/* Close socket. */
	if (vty_tty_console.console == vty)
		vty_console_close(0);
	else
	{
		if (vty->fd_type == IPCOM_STACK)
			ip_close(vty->fd);
		else
		{
			if(!FD_IS_STDOUT(vty->fd))
				close(vty->fd);
		}
	}
	if (vty->buf)
		XFREE(MTYPE_VTY, vty->buf);

	/* Check configure. */
	vty_config_unlock(vty);

	/* OK free vty. */
	XFREE(MTYPE_VTY, vty);
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
	//if(vty_tty_console.console != vty)
	vty_out(vty, "%s%sVty connection is timed out.%s%s",
	VTY_NEWLINE, VTY_NEWLINE,
	VTY_NEWLINE, VTY_NEWLINE);

	/* Close connection. */
	vty->status = VTY_CLOSE;
	if (vty_tty_console.console == vty)
		vty_console_reset();
	else
		vty_close(vty);

	return 0;
}

static int vty_console_timeout(struct thread *thread)
{
	struct vty *vty;

	vty = THREAD_ARG(thread);
	vty->t_timeout = NULL;
	vty->v_timeout = 0;

	/* Clear buffer*/
	buffer_reset(vty->obuf);
	//if(vty_tty_console.console != vty)
	vty_out(vty, "%s%sVty connection is timed out.%s%s",
	VTY_NEWLINE, VTY_NEWLINE,
	VTY_NEWLINE, VTY_NEWLINE);

	/* Close connection. */
	vty->status = VTY_CLOSE;
	if (vty_tty_console.console == vty)
		vty_console_reset();
	else
		vty_close(vty);

	return 0;
}
/* Read up configuration file from file_name. */
static void vty_read_file(FILE *confp)
{
	int ret;
	struct vty *vty;
	unsigned int line_num = 0;
	//vty_console(NULL);
	//vty = vty_tty_console.console;
	vty = vty_new();
	vty->wfd = STDOUT_FILENO;
	vty->fd = STDIN_FILENO;
	vty->type = VTY_FILE;
	vty->node = CONFIG_NODE;
	vty->fd_type = OS_STACK;
	/* Execute configuration file */
	ret = config_from_file(vty, confp, &line_num);

	/* Flush any previous errors before printing messages below */
	buffer_flush_all(vty->obuf, vty->wfd, OS_STACK);

	if (!((ret == CMD_SUCCESS) || (ret == CMD_ERR_NOTHING_TODO)))
	{
		switch (ret)
		{
		case CMD_ERR_AMBIGUOUS:
			fprintf(stderr, "*** Error reading config: Ambiguous command.\n");
			break;
		case CMD_ERR_NO_MATCH:
			fprintf(stderr,
					"*** Error reading config: There is no such command.\n");
			break;
		}
		fprintf(stderr, "*** Error occured processing line %u, below:\n%s\n",
				line_num, vty->buf);
#if 1
		vty_close(vty);
#else
		/* Cancel threads.*/
		if (vty->t_read)
			eloop_cancel(vty->t_read);
		if (vty->t_write)
			eloop_cancel(vty->t_write);
		if (vty->t_timeout)
			eloop_cancel(vty->t_timeout);
		/* Free input buffer. */
		buffer_free(vty->obuf);
		//close(vty->fd);
		if (vty->buf)
			XFREE(MTYPE_VTY, vty->buf);
		/* Check configure. */
		vty_config_unlock(vty);
		/* OK free vty. */
		XFREE(MTYPE_VTY, vty);
#endif
		exit(1);
	}
#if 1
	vty_close(vty);
#else
	/* Cancel threads.*/
	if (vty->t_read)
		eloop_cancel(vty->t_read);
	if (vty->t_write)
		eloop_cancel(vty->t_write);
	if (vty->t_timeout)
		eloop_cancel(vty->t_timeout);
	/* Free input buffer. */
	buffer_free(vty->obuf);
	//close(vty->fd);
	if (vty->buf)
		XFREE(MTYPE_VTY, vty->buf);
	/* Check configure. */
	vty_config_unlock(vty);
	/* OK free vty. */
	XFREE(MTYPE_VTY, vty);
#endif
}

static int host_config_default(char *password, char *defult_config)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if (defult_config == NULL)
	{
		fprintf(stderr,
				"%s: failed to setting default configuration file :%s\n",
				__func__, safe_strerror(errno));
		//exit(0);
	}
	if (host.name == NULL)  //
		host.name = XSTRDUP(MTYPE_HOST, QUAGGA_PROGNAME);
	if (host.mutx)
		os_mutex_unlock(host.mutx);

	if (defult_config)
		host_config_set(defult_config);
	/*	if (zlog_default)
	 zlog_set_level(ZLOG_DEST_STDOUT, zlog_default->default_lvl);*/
	return 0;
}

/* Read up configuration file from file_name. */
static char * vty_default_config_getting(void)
{
	extern struct host host;
	if (host.config && access(host.config, 0x04) == 0)
		return host.config;
	if (host.default_config && access(host.default_config, 0x04) == 0)
		return host.default_config;
	if (host.factory_config && access(host.factory_config, 0x04) == 0)
		return host.factory_config;
	return NULL;
	//host_config_default(NULL, host.factory_config);
}
void vty_load_config(char *config_file)
{
	extern struct host host;
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
			fprintf(stderr, "configuration file exits.\n");
			return;
		}
	}
	if (confp == NULL)
	{
		fprintf(stderr, "%s: failed to open configuration file %s: %s\n",
				__func__, config_file ? config_file : "null",
				safe_strerror(errno));
		//host_config_default(NULL, NULL);
		return;
	}
	if (confp)
	{
		fseek(confp, 0, SEEK_END);
		if (ftell(confp) < 16)
		{
			fclose(confp);
			confp = NULL;
			fprintf(stderr, "configuration file is BACK\n");
		}
		else
		{
			fseek(confp, 0, SEEK_SET);
			vty_read_file(confp);
			fclose(confp);
		}
	}
}

/* Small utility function which output log to the VTY. */
void vty_log(const char *level, const char *proto_str, const char *format,
		zlog_timestamp_t ctl, va_list va)
{
	unsigned int i;
	struct vty *vty;

	if (!vtyvec)
		return;

	for (i = 0; i < vector_active(vtyvec); i++)
		if ((vty = vector_slot(vtyvec, i)) != NULL)
			if (vty->monitor)
			{
				va_list ac;
				va_copy(ac, va);
				vty_log_out(vty, level, proto_str, format, ctl, ac);
				va_end(ac);
			}
}

void vty_trap_log(const char *level, const char *proto_str, const char *format,
		zlog_timestamp_t ctl, va_list va)
{
	unsigned int i;
	struct vty *vty;

	if (!vtyvec)
		return;

	for (i = 0; i < vector_active(vtyvec); i++)
		if ((vty = vector_slot(vtyvec, i)) != NULL)
			if (vty->trapping)
			{
				va_list ac;
				va_copy(ac, va);
				vty_log_out(vty, level, proto_str, format, ctl, ac);
				va_end(ac);
			}
}

/* Async-signal-safe version of vty_log for fixed strings. */
static void ip_vty_log_fixed(struct vty *vty, char *buf, size_t len)
{
	struct iovec iov[2];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	iov[1].iov_base = (void *) "\r\n";
	iov[1].iov_len = 2;
	ip_writev(vty->fd, iov, 2);
}

#undef iovec
static void os_vty_log_fixed(struct vty *vty, char *buf, size_t len)
{
	struct iovec iov[2];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	iov[1].iov_base = (void *) "\r\n";
	iov[1].iov_len = 2;
	writev(vty->fd, iov, 2);
}


void vty_log_fixed(char *buf, size_t len)
{
	unsigned int i;
	struct vty *vty;
	/* vty may not have been initialised */
	if (!vtyvec)
		return;

	for (i = 0; i < vector_active(vtyvec); i++)
	{
		if (((vty = vector_slot(vtyvec, i)) != NULL) && (vty->trapping || vty->monitor))
		{
			/* N.B. We don't care about the return code, since process is
			 most likely just about to die anyway. */
			if (vty->fd_type == OS_STACK)
				os_vty_log_fixed(vty, buf, len);
			else
				ip_vty_log_fixed(vty, buf, len);
		}
	}
}

int vty_config_lock(struct vty *vty)
{
	if (host.vty_config == 0)
	{
		vty->config = 1;
		host.vty_config = 1;
	}
	return vty->config;
}

int vty_config_unlock(struct vty *vty)
{
	if (host.vty_config == 1 && vty->config == 1)
	{
		vty->config = 0;
		host.vty_config = 0;
	}
	return vty->config;
}

static void vty_event(enum vtyevent event, int sock, struct vty *vty)
{
	struct eloop *vty_serv_thread;

	switch (event)
	{
	case VTY_SERV:
		vty_serv_thread = eloop_add_read(eloop_master, vty_accept, vty, sock);
		vector_set_index(host.Vvty_serv_thread, sock, vty_serv_thread);
		break;
#ifdef VTYSH
		case VTYSH_SERV:
		vty_serv_thread = thread_add_read(thread_master, vtysh_accept, vty, sock);
		vector_set_index(host.Vvty_serv_thread, sock, vty_serv_thread);
		break;
		case VTYSH_READ:
		vty->t_read = thread_add_read(thread_master, vtysh_read, vty, sock);
		break;
		case VTYSH_WRITE:
		vty->t_write = thread_add_write(thread_master, vtysh_write, vty, sock);
		break;
#endif /* VTYSH */
	case VTY_READ:
		if (vty->fd_type == OS_STACK || vty->type == VTY_FILE)
		{
			vty->t_read = thread_add_read(thread_master, vty_console_read, vty,
					sock);

			/* Time out treatment. */
			if (vty->v_timeout)
			{
				if (vty->t_timeout)
					thread_cancel(vty->t_timeout);
				vty->t_timeout = thread_add_timer(thread_master,
						vty_console_timeout, vty, vty->v_timeout);
			}
		}
		else
		{
			vty->t_read = eloop_add_read(eloop_master, vty_read, vty, sock);

			/* Time out treatment. */
			if (vty->v_timeout)
			{
				if (vty->t_timeout)
					eloop_cancel(vty->t_timeout);
				vty->t_timeout = eloop_add_timer(eloop_master, vty_timeout, vty,
						vty->v_timeout);
			}
		}
		break;
	case VTY_WRITE:
		if (vty->fd_type == OS_STACK || vty->type == VTY_FILE)
		{
			if (!vty->t_write)
				vty->t_write = thread_add_write(thread_master,
						vty_console_flush, vty, sock);
		}
		else
		{
			if (!vty->t_write)
				vty->t_write = eloop_add_write(eloop_master, vty_flush, vty,
						sock);
		}
		break;
	case VTY_TIMEOUT_RESET:
		if (vty->fd_type == OS_STACK || vty->type == VTY_FILE)
		{
			if (vty->t_timeout)
			{
				thread_cancel(vty->t_timeout);
				vty->t_timeout = NULL;
			}
			if (vty->v_timeout)
			{
				vty->t_timeout = thread_add_timer(thread_master,
						vty_console_timeout, vty, vty->v_timeout);
			}
		}
		else
		{
			if (vty->t_timeout)
			{
				eloop_cancel(vty->t_timeout);
				vty->t_timeout = NULL;
			}
			if (vty->v_timeout)
			{
				vty->t_timeout = eloop_add_timer(eloop_master, vty_timeout, vty,
						vty->v_timeout);
			}
		}
		break;
	case VTY_STDIO_WAIT:
		vty_tty_console.t_wait = thread_add_timer(thread_master, vty_console_wait,
				vty, 5);
		break;
	case VTY_STDIO_ACCEPT:
		vty->t_read = thread_add_read(thread_master, vty_console_accept, vty,
				sock);
		break;

	}
}

/* Set time out value. */
int exec_timeout(struct vty *vty, const char *min_str, const char *sec_str)
{
	unsigned long timeout = 0;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	/* min_str and sec_str are already checked by parser.  So it must be
	 all digit string. */
	if (min_str)
	{
		timeout = strtol(min_str, NULL, 10);
		timeout *= 60;
	}
	if (sec_str)
		timeout += strtol(sec_str, NULL, 10);

	host.vty_timeout_val = timeout;
	vty->v_timeout = timeout;
	vty_event(VTY_TIMEOUT_RESET, 0, vty);
	if (host.mutx)
		os_mutex_unlock(host.mutx);

	return CMD_SUCCESS;
}
/* Reset all VTY status. */
void vty_reset()
{
	unsigned int i;
	struct vty *vty;
	struct eloop *vty_serv_thread;

	for (i = 0; i < vector_active(vtyvec); i++)
		if ((vty = vector_slot(vtyvec, i)) != NULL)
		{
			buffer_reset(vty->obuf);
			vty->status = VTY_CLOSE;
			if (vty != vty_tty_console.console)
				vty_close(vty);
			else
				vty_console_close(1);
		}

	for (i = 0; i < vector_active(host.Vvty_serv_thread); i++)
		if ((vty_serv_thread = vector_slot(host.Vvty_serv_thread, i)) != NULL)
		{
			eloop_cancel(vty_serv_thread);
			vector_slot (host.Vvty_serv_thread, i) = NULL;
			close(i);
		}
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	host.vty_timeout_val = VTY_TIMEOUT_DEFAULT;

	if (host.vty_accesslist_name)
	{
		XFREE(MTYPE_VTY, host.vty_accesslist_name);
		host.vty_accesslist_name = NULL;
	}

	if (host.vty_ipv6_accesslist_name)
	{
		XFREE(MTYPE_VTY, host.vty_ipv6_accesslist_name);
		host.vty_ipv6_accesslist_name = NULL;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
}

static void vty_save_cwd(void)
{
	char cwd[MAXPATHLEN];
	char *c;

	c = getcwd(cwd, MAXPATHLEN);

	if (!c)
	{
		chdir(SYSCONFDIR);
		getcwd(cwd, MAXPATHLEN);
	}

	host.vty_cwd = XMALLOC(MTYPE_TMP, strlen(cwd) + 1);
	strcpy(host.vty_cwd, cwd);
}

char *
vty_get_cwd()
{
	return host.vty_cwd;
}

int vty_shell(struct vty *vty)
{
	return vty->type == VTY_SHELL ? 1 : 0;
}

int vty_shell_serv(struct vty *vty)
{
	return vty->type == VTY_SHELL_SERV ? 1 : 0;
}

void vty_init_vtysh()
{
	vtyvec = vector_init(VECTOR_MIN_SIZE);
}

void * vty_thread_master()
{
	return thread_master;
}
/* Install vty's own commands like `who' command. */
void vty_init(void *m1, void *m2)
{
	/* For further configuration read, preserve current directory. */
	vty_save_cwd();

	vtyvec = vector_init(VECTOR_MIN_SIZE);

	thread_master = m1;
	eloop_master = m2;
	vty_ctrl_cmd = vty_ctrl_default;

	atexit(vty_console_reset);

	/* Initilize server thread vector. */
	host.Vvty_serv_thread = vector_init(VECTOR_MIN_SIZE);

}

void vty_terminate(void)
{
	if (host.vty_cwd)
		XFREE(MTYPE_TMP, host.vty_cwd);

	if (vtyvec && host.Vvty_serv_thread)
	{
		vty_reset();
		vector_free(vtyvec);
		vector_free(host.Vvty_serv_thread);
	}
}

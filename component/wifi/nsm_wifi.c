/*
 * nsm_wifi.c
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */



#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "vrf.h"
#include "dirent.h"
#include "os_util.h"

#include "nsm_wifi.h"

#ifndef CONFIG_IW_TOOLS
#define DAEMON_VTY_DIR	"/var/tmp/"
#define NET_BASE_DIR	"/sys/class/net/"
#endif

wifi_interface_t wifi_interface =
{
#ifndef CONFIG_IW_TOOLS
	.filename = WIFI_FILE_NAME,
#endif
};

#ifdef CONFIG_IW_TOOLS

static LIST	iw_cmd;

extern int wifi_main(int argc, char **argv);

static int wifi_vty_obuf_swap(struct vty *vty, char *buf, int len)
{
	int i = 0, j = 0;
	char *p = XREALLOC(MTYPE_VTY_OUT_BUF, p, len + 256);
	if (!p)
		return -1;
	while(1)
	{
		if(buf[i] == '\n')
		{
			p[j++] = '\r';
			p[j++] = '\n';
		}
		else
			p[j++] = buf[i];
		if(i == len)
			break;
		i++;
	}
	//printf("\r\n===========================\r\n");
	buffer_put(vty->obuf, (u_char *) p, j);
	XFREE(MTYPE_VTY_OUT_BUF, p);
	return 0;
}

int iw_printf(const char *format, ...)
{
	if (wifi_interface.vty)
	{
		va_list args;
		int len = 0;
		int size = 1024;
		char buf[1024];
		char *p = NULL;
		struct vty *vty = wifi_interface.vty;
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
			if(vty->type == VTY_TERM)
				wifi_vty_obuf_swap(vty, (u_char *) p, len);
			else
				buffer_put(vty->obuf, (u_char *) p, len);
			/* If p is not different with buf, it is allocated buffer.  */
			if (p != buf)
				XFREE(MTYPE_VTY_OUT_BUF, p);
			return len;
		}
	}
	return 0;
}

int iw_fprintf(void *fp, const char *format, ...)
{
	if(fp)
	{
		va_list args;
		int len = 0;
		char buf[1024];
		os_memset(buf, 0, sizeof(buf));
		/* Try to write to initial buffer.  */
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		zlog_err(ZLOG_PAL, buf);
	}
	else
	{
		va_list args;
		int len = 0;
		char buf[1024];
		os_memset(buf, 0, sizeof(buf));
		/* Try to write to initial buffer.  */
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);
		iw_printf(buf);
	}
	return 0;
}





int iw_cmd_init(void)
{
	lstInit(&iw_cmd);
	return 0;
}

int iw_cmd_exit(void)
{
	if(lstCount(&iw_cmd))
		lstFree(&iw_cmd);
	return 0;
}


static int iw_cmd_add_node(void *data)
{
	wifi_cmd_t *node = malloc(sizeof(wifi_cmd_t));
	if(node)
	{
		memset(node, 0, sizeof(wifi_cmd_t));
		node->cmd = data;
		lstAdd(&iw_cmd, (NODE *)node);
		return 0;
	}
	return -1;
}

int iw_cmd_add(void *data)
{
	iw_cmd_add_node(data);
}













int wifi_show(struct vty *vty)
{
	const char *tmpcmd[3] = {"iw", "list", NULL};
	wifi_interface.vty = vty;
	wifi_main(2, tmpcmd);
	//wifi_show_from_file(vty);
	wifi_interface.vty = NULL;
	return OK;
}

#else

const char *wifi_file_path()
{
	static char filepath[256];
	os_memset(filepath, 0, sizeof(filepath));
	os_sprintf(filepath, "%s%s", DAEMON_VTY_DIR, WIFI_FILE_NAME);
	return filepath;
}


static int wifi_show_from_file(struct vty *vty)
{
	FILE *fp = NULL;
	char buf[512];
	char *filepath = wifi_file_path();
	fp = fopen(filepath, "r");
	if(fp)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
				char *s;
				for (s = buf + strlen(buf);
						(s > buf) && isspace((int) *(s - 1)); s--)
					;
				*s = '\0';
				vty_out(vty, "%s%s", buf, VTY_NEWLINE);
		}
		fclose(fp);
	}
	//vty_out(vty, "%s%s", buf, VTY_NEWLINE);
	return OK;
}

static int phy_id_lookup(char *name)
{
	char buf[200];
	int fd, pos;
	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	return atoi(buf);
}

//
static BOOL interface_is_wireless(char *name)
{
	char buf[200];
	DIR * dir = NULL;
	os_memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s%s/phy80211", NET_BASE_DIR, name);
	dir = opendir(buf);
	if(dir)
	{
		closedir(dir);
		os_memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%s%s/wireless", NET_BASE_DIR, name);
		dir = opendir(buf);
		if(dir)
		{
			closedir(dir);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

#endif




int show_wifi_capabilities(struct interface *ifp, struct vty *vty)
{
	char tmpcmd[128];
	os_memset(tmpcmd, 0, sizeof(tmpcmd));
	//os_sprintf(tmpcmd, "rm %s", wifi_file_path());
	super_system(tmpcmd);
	os_memset(tmpcmd, 0, sizeof(tmpcmd));
	//os_sprintf(tmpcmd, "iw list >> %s 2>&1", wifi_file_path());
	super_system(tmpcmd);
	//wifi_show_from_file(vty);
	return OK;
}

//export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu/:$LD_LIBRARY_PATH"
int show_wifi_ap(struct vty *vty)
{
	char tmpcmd[128];
	os_memset(tmpcmd, 0, sizeof(tmpcmd));
	//os_sprintf(tmpcmd, "rm %s", wifi_file_path());
	super_system(tmpcmd);
	os_memset(tmpcmd, 0, sizeof(tmpcmd));
	//os_sprintf(tmpcmd, "iw list >> %s 2>&1", wifi_file_path());
	super_system(tmpcmd);
	//wifi_show_from_file(vty);
	return OK;
}


/*
 * ./iw argc = 1 ./iw
 */

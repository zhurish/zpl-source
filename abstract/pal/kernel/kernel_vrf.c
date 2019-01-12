/*
 * VRF functions.
 * Copyright (C) 2014 6WIND S.A.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <zebra.h>

#ifdef HAVE_NETNS
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <sched.h>
#endif

#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "table.h"
#include "log.h"
#include "memory.h"
#include "command.h"
#include "vty.h"
#include "pal_interface.h"
#ifndef CLONE_NEWNET
#define CLONE_NEWNET 0x40000000 /* New network namespace (lo, device, names sockets, etc) */
#endif


#ifndef HAVE_SETNS
static inline int setns(int fd, int nstype)
{
#ifdef __NR_setns
	return syscall(__NR_setns, fd, nstype);
#else
	errno = ENOSYS;
	return -1;
#endif
}
#endif /* HAVE_SETNS */

#define VRF_RUN_DIR         "/var/run/netns"

#ifdef HAVE_NETNS

#define VRF_DEFAULT_NAME    "/proc/self/ns/net"
static int have_netns_enabled = -1;

#else /* !HAVE_NETNS */

#define VRF_DEFAULT_NAME    "Default-IP-Routing-Table"

#endif /* HAVE_NETNS */

static int have_netns(void)
{
#ifdef HAVE_NETNS
	if (have_netns_enabled < 0)
	{
		int fd = open(VRF_DEFAULT_NAME, O_RDONLY);

		if (fd < 0)
			have_netns_enabled = 0;
		else
		{
			have_netns_enabled = 1;
			close(fd);
		}
	}
	return have_netns_enabled;
#else
	return 0;
#endif
}

/*
 * VRF realization with NETNS
 */

static char *
vrf_netns_pathname(const char *name)
{
	static char pathname[PATH_MAX];
	char *result;

	if (name[0] == '/') /* absolute pathname */
		result = realpath(name, pathname);
	else /* relevant pathname */
	{
		char tmp_name[PATH_MAX];
		snprintf(tmp_name, PATH_MAX, "%s/%s", VRF_RUN_DIR, name);
		result = realpath(tmp_name, pathname);
	}

	if (!result)
	{
		zlog_err(ZLOG_PAL, "Invalid pathname: %s", safe_strerror(errno));
		return NULL;
	}
	return pathname;
}

/*
 * Check whether the VRF is enabled - that is, whether the VRF
 * is ready to allocate resources. Currently there's only one
 * type of resource: socket.
 */
static int vrf_is_enabled(struct vrf *vrf)
{
	if (have_netns())
		return vrf && vrf->fd >= 0;
	else
		return vrf && vrf->fd == -2 && vrf->vrf_id == VRF_DEFAULT;
}

/*
 * Enable a VRF - that is, let the VRF be ready to use.
 * The VRF_ENABLE_HOOK callback will be called to inform
 * that they can allocate resources in this VRF.
 *
 * RETURN: 1 - enabled successfully; otherwise, 0.
 */
static int vrf_enable(vrf_id_t vrf_id)
{
	struct vrf *vrf = vrf_lookup(vrf_id);
	char *pathname = vrf_netns_pathname (vrf->name);
	if (!vrf_is_enabled(vrf))
	{
		if (have_netns())
		{
			vrf->fd = open(pathname, O_RDONLY);
		}
		else
		{
			vrf->fd = -2; /* Remember that vrf_enable_hook has been called */
			errno = -ENOTSUP;
			return -1;
		}

		if (!vrf_is_enabled(vrf))
		{
			zlog_err(ZLOG_PAL, "Can not enable VRF %u: %s!", vrf->vrf_id,
					safe_strerror(errno));
			return -1;
		}

		if (have_netns())
			zlog_info(ZLOG_PAL, "VRF %u is associated with NETNS %s.",
					vrf->vrf_id, vrf->name);

		zlog_info(ZLOG_PAL, "VRF %u is enabled.", vrf->vrf_id);
	}
	return 0;
}

/*
 * Disable a VRF - that is, let the VRF be unusable.
 * The VRF_DELETE_HOOK callback will be called to inform
 * that they must release the resources in the VRF.
 */
static int vrf_disable(vrf_id_t vrf_id)
{
	struct vrf *vrf = vrf_lookup(vrf_id);
	if (vrf_is_enabled(vrf))
	{
		zlog_info(ZLOG_PAL, "VRF %u is to be disabled.", vrf->vrf_id);
		if (have_netns())
			close(vrf->fd);

		vrf->fd = -1;
		return 0;
	}
	return -1;
}

int os_vrf_stack_init()
{
	//route
	pal_stack.ip_stack_vrf_create = vrf_enable;
	pal_stack.ip_stack_vrf_delete = vrf_disable;
	return OK;
}

/*
 DEFUN (vrf_netns,
 vrf_netns_cmd,
 "vrf <1-65535> netns NAME",
 "Enable a VRF\n"
 "Specify the VRF identifier\n"
 "Associate with a NETNS\n"
 "The file name in " VRF_RUN_DIR ", or a full pathname\n")
 {
 vrf_id_t vrf_id = VRF_DEFAULT;
 struct vrf *vrf = NULL;
 char *pathname = vrf_netns_pathname (vty, argv[1]);

 if (!pathname)
 return CMD_WARNING;

 VTY_GET_INTEGER ("VRF ID", vrf_id, argv[0]);
 vrf = vrf_get (vrf_id);

 if (vrf->name && strcmp (vrf->name, pathname) != 0)
 {
 vty_out (vty, "VRF %u is already configured with NETNS %s%s",
 vrf->vrf_id, vrf->name, VTY_NEWLINE);
 return CMD_WARNING;
 }

 if (!vrf->name)
 vrf->name = XSTRDUP (MTYPE_VRF_NAME, pathname);

 if (!vrf_enable (vrf))
 {
 vty_out (vty, "Can not associate VRF %u with NETNS %s%s",
 vrf->vrf_id, vrf->name, VTY_NEWLINE);
 return CMD_WARNING;
 }

 return CMD_SUCCESS;
 }

 DEFUN (no_vrf_netns,
 no_vrf_netns_cmd,
 "no vrf <1-65535> netns NAME",
 NO_STR
 "Enable a VRF\n"
 "Specify the VRF identifier\n"
 "Associate with a NETNS\n"
 "The file name in " VRF_RUN_DIR ", or a full pathname\n")
 {
 vrf_id_t vrf_id = VRF_DEFAULT;
 struct vrf *vrf = NULL;
 char *pathname = vrf_netns_pathname (vty, argv[1]);

 if (!pathname)
 return CMD_WARNING;

 VTY_GET_INTEGER ("VRF ID", vrf_id, argv[0]);
 vrf = vrf_lookup (vrf_id);

 if (!vrf)
 {
 vty_out (vty, "VRF %u is not found%s", vrf_id, VTY_NEWLINE);
 return CMD_SUCCESS;
 }

 if (vrf->name && strcmp (vrf->name, pathname) != 0)
 {
 vty_out (vty, "Incorrect NETNS file name%s", VTY_NEWLINE);
 return CMD_WARNING;
 }

 vrf_disable (vrf);

 if (vrf->name)
 {
 XFREE (MTYPE_VRF_NAME, vrf->name);
 vrf->name = NULL;
 }

 return CMD_SUCCESS;
 }

 VRF node.
 static struct cmd_node vrf_node =
 {
 VRF_NODE,
 "",        VRF node has no interface.
 1
 };

 VRF configuration write function.
 static int
 vrf_config_write (struct vty *vty)
 {
 struct route_node *rn;
 struct vrf *vrf;
 int write = 0;

 for (rn = route_top (vrf_table); rn; rn = route_next (rn))
 if ((vrf = rn->info) != NULL &&
 vrf->vrf_id != VRF_DEFAULT && vrf->name)
 {
 vty_out (vty, "vrf %u netns %s%s", vrf->vrf_id, vrf->name, VTY_NEWLINE);
 write++;
 }

 return write;
 }

 Initialize VRF module.
 void
 vrf_init (void)
 {
 struct vrf *default_vrf;

 Allocate VRF table.
 vrf_table = route_table_init ();

 The default VRF always exists.
 default_vrf = vrf_get (VRF_DEFAULT);
 if (!default_vrf)
 {
 zlog_err ("vrf_init: failed to create the default VRF!");
 exit (1);
 }

 Set the default VRF name.
 default_vrf->name = XSTRDUP (MTYPE_VRF_NAME, VRF_DEFAULT_NAME);

 Enable the default VRF.
 if (!vrf_enable (default_vrf))
 {
 zlog_err ("vrf_init: failed to enable the default VRF!");
 exit (1);
 }

 if (have_netns())
 {
 Install VRF commands.
 install_node (&vrf_node, vrf_config_write);
 install_element (CONFIG_NODE, &vrf_netns_cmd);
 install_element (CONFIG_NODE, &no_vrf_netns_cmd);
 }
 }

 Terminate VRF module.
 void
 vrf_terminate (void)
 {
 struct route_node *rn;
 struct vrf *vrf;

 for (rn = route_top (vrf_table); rn; rn = route_next (rn))
 if ((vrf = rn->info) != NULL)
 vrf_delete (vrf);

 route_table_finish (vrf_table);
 vrf_table = NULL;
 }*/

/* Create a socket for the VRF. */
int vrf_socket(int domain, int type, int protocol, vrf_id_t vrf_id)
{
	struct vrf *vrf = vrf_lookup(vrf_id);
	int ret = -1;

/*
	if (!vrf_is_enabled(vrf))
	{
		errno = ENOSYS;
		return -1;
	}
*/

	if (have_netns())
	{
		ret = (vrf_id != VRF_DEFAULT) ? setns(vrf->fd, CLONE_NEWNET) : 0;
		if (ret >= 0)
		{
			ret = socket(domain, type, protocol);
			if (vrf_id != VRF_DEFAULT)
				setns(vrf_lookup(VRF_DEFAULT)->fd, CLONE_NEWNET);
		}
	}
	else
		ret = socket(domain, type, protocol);

	return ret;
}

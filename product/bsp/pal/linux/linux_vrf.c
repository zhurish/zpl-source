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


#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "vrf.h"
#include "command.h"
#include "prefix.h"


#ifdef HAVE_NETNS
#undef  _GNU_SOURCE
#define _GNU_SOURCE

#include <sched.h>
#endif


#include "linux_driver.h"
#include "pal_include.h"
#ifndef CLONE_NEWNET
#define CLONE_NEWNET 0x40000000 /* New network namespace (lo, device, names sockets, etc) */
#endif


#ifndef HAVE_SETNS
static inline int setns(int fd, int nstype)
{
#ifdef __NR_setns
	return syscall(__NR_setns, fd, nstype);
#else
	ipstack_errno = ENOSYS;
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
		zpl_fd_t fd = ipstack_open(IPCOM_STACK, VRF_DEFAULT_NAME, O_RDONLY);

		if (ipstack_invalid(fd))
			have_netns_enabled = 0;
		else
		{
			have_netns_enabled = 1;
			ipstack_close(fd);
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

static char * vrf_netns_pathname(const char *name)
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
		zlog_err(MODULE_PAL, "Invalid pathname: %s", ipstack_strerror(ipstack_errno));
		return NULL;
	}
	return pathname;
}

/*
 * Check whether the VRF is enabled - that is, whether the VRF
 * is ready to allocate resources. Currently there's only one
 * type of resource: ipstack_socket.
 */
static int vrf_is_enabled(struct ip_vrf *vrf)
{
	if (have_netns())
		return vrf && vrf->fd._fd >= 0;
	else
		return vrf && vrf->fd._fd == -2 && vrf->vrf_id == VRF_DEFAULT;
}

/*
 * Enable a VRF - that is, let the VRF be ready to use.
 * The VRF_ENABLE_HOOK callback will be called to inform
 * that they can allocate resources in this VRF.
 *
 * RETURN: 1 - enabled successfully; otherwise, 0.
 */
int _ipkernel_vrf_enable(struct ip_vrf *vrf)
{
	if(!vrf)
		return -1;

	char *pathname = vrf_netns_pathname (vrf->name);
	if (!vrf_is_enabled(vrf))
	{
		if (have_netns())
		{
			vrf->fd = ipstack_open(IPCOM_STACK, pathname, O_RDONLY);
		}
		else
		{
			//vrf->fd = -2; /* Remember that vrf_enable_hook has been called */
			ipstack_errno = -ENOTSUP;
			return -1;
		}

		if (!vrf_is_enabled(vrf))
		{
			zlog_err(MODULE_PAL, "Can not enable VRF %u: %s!", vrf->vrf_id,
					ipstack_strerror(ipstack_errno));
			return -1;
		}

		if (have_netns())
			zlog_info(MODULE_PAL, "VRF %u is associated with NETNS %s.",
					vrf->vrf_id, vrf->name);

		zlog_info(MODULE_PAL, "VRF %u is enabled.", vrf->vrf_id);
	}
	return 0;
}

/*
 * Disable a VRF - that is, let the VRF be unusable.
 * The VRF_DELETE_HOOK callback will be called to inform
 * that they must release the resources in the VRF.
 */
int _ipkernel_vrf_disable(struct ip_vrf *vrf)
{
	if(!vrf)
		return -1;
	if (vrf_is_enabled(vrf))
	{
		zlog_info(MODULE_PAL, "VRF %u is to be disabled.", vrf->vrf_id);
		if (have_netns())
			ipstack_close(vrf->fd);

		vrf->fd._fd = -1;
		return 0;
	}
	return -1;
}



/* Create a socket for the VRF. */
zpl_socket_t _kernel_vrf_socket (int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id)
{
	struct ip_vrf *vrf = ip_vrf_lookup (vrf_id);
	int ret = -1;
	zpl_socket_t tmp;
	ipstack_init(OS_STACK, tmp);
	if(!vrf)
		return tmp;
	if (!vrf_is_enabled (vrf))
    {
      errno = ENOSYS;
      return tmp;
    }

	if (have_netns())
    {
      ret = (vrf_id != VRF_DEFAULT) ? setns (vrf->fd._fd, CLONE_NEWNET) : 0;
      if (ret >= 0)
        {
          tmp._fd = socket (domain, type, protocol);
          if (vrf_id != VRF_DEFAULT)
            setns (ip_vrf_lookup (VRF_DEFAULT)->fd._fd, CLONE_NEWNET);
        }
    }
	else
		tmp._fd = socket (domain, type, protocol);
	return tmp;
}


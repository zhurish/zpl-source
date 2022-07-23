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
#include "if.h"
#include "vrf.h"
#include "prefix.h"
#include "table.h"
#include "log.h"
#include "zmemory.h"
#include "command.h"
#include "vty.h"
#include "nsm_rib.h"
#include "nsm_ipvrf.h"
#include "nsm_halpal.h"

/* Initialize VRF module. */
struct ip_vrf *ipvrf_create(const char *name)
{
  int ret = 0;
  struct ip_vrf *ip_vrf = ip_vrf_create(name);
  if (ip_vrf)
  {
    ip_vrf->info = nsm_vrf_create(ip_vrf->vrf_id);
    if (ip_vrf->info)
    {
      ret = nsm_halpal_create_vrf(ip_vrf);
    }
    if (ret != OK)
    {
      nsm_vrf_destroy(ip_vrf->info);
      ip_vrf_delete(name);
      ip_vrf = NULL;
    }
  }
  return ip_vrf;
}

int ipvrf_delete(const char *name)
{
  int ret = 0;
  struct ip_vrf *ip_vrf = NULL;
  ip_vrf = ip_vrf_lookup_by_name(name);
  if (!ip_vrf)
  {
    zlog_err(MODULE_NSM, "vrf_init: failed to create the default VRF!");
    return ERROR;
  }
  ret = nsm_halpal_delete_vrf(ip_vrf);
  if (ret == OK)
  {
    nsm_vrf_destroy(ip_vrf->info);
    ip_vrf_delete(name);
    ip_vrf = NULL;
  }
  zlog_err(MODULE_NSM, "vrf_init: failed to create the default VRF!");
  return ERROR;
}

/* Zebra VRF initialization. */
void ipvrf_init(void)
{
  ip_vrf_init();
  ipvrf_create("Default-IP-Routing-Table");
}

/* Terminate VRF module. */
void ipvrf_terminate(void)
{
  struct ip_vrf *ip_vrf = NULL;
  NODE index;
  if (lstCount(_ip_vrf_master.ip_vrf_list) == 0)
    return;
  for (ip_vrf = (struct ip_vrf *)lstFirst(_ip_vrf_master.ip_vrf_list);
       ip_vrf != NULL; ip_vrf = (struct ip_vrf *)lstNext((NODE *)&index))
  {
    index = ip_vrf->node;
    if (ip_vrf)
    {
      nsm_halpal_delete_vrf(ip_vrf);
      nsm_vrf_destroy(ip_vrf->info);
      lstDelete(_ip_vrf_master.ip_vrf_list, (NODE *)ip_vrf);
      ip_vrf = NULL;
    }
  }
  return;
}

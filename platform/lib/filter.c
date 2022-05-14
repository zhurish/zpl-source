/* Route filtering function.
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
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
#include "prefix.h"
#include "command.h"
#include "filter.h"
#include "zmemory.h"
#include "sockunion.h"
#include "buffer.h"
#include "log.h"
#include "if.h"
/* Static structure for IPv4 access_list's master. */
static struct access_master access_master_ipv4 =
    {
        {NULL, NULL},
        {NULL, NULL},
        // NULL,
        // NULL,
};

#ifdef ZPL_BUILD_IPV6
/* Static structure for IPv6 access_list's master. */
static struct access_master access_master_ipv6 =
    {
        {NULL, NULL},
        {NULL, NULL},
        // NULL,
        // NULL,
};
#endif /* ZPL_BUILD_IPV6 */
#ifdef ZPL_FILTER_MAC
static struct access_master access_master_l2mac =
    {
        {NULL, NULL},
        {NULL, NULL},
        // NULL,
        // NULL,
};
#endif
static struct access_master *
access_master_get(afi_t afi)
{
  if (afi == AFI_IP)
    return &access_master_ipv4;
#ifdef ZPL_BUILD_IPV6
  else if (afi == AFI_IP6)
    return &access_master_ipv6;
#endif /* ZPL_BUILD_IPV6 */
#ifdef ZPL_FILTER_MAC
  else if (afi == AFI_ETHER)
    return &access_master_l2mac;
#endif
  return NULL;
}


/* Allocate new filter structure. */
static struct filter_list *
filter_new(void)
{
  return (struct filter_list *)XCALLOC(MTYPE_ACCESS_FILTER,
                                       sizeof(struct filter_list));
}

static void
filter_free(struct filter_list *filter)
{
  XFREE(MTYPE_ACCESS_FILTER, filter);
}

/* Return string of filter_type. */
const char *
filter_type_str(enum filter_type type)
{
  switch (type)
  {
  case FILTER_PERMIT:
    return "permit";
    break;
  case FILTER_DENY:
    return "deny";
    break;
  case FILTER_DYNAMIC:
    return "dynamic";
    break;
  default:
    return "";
    break;
  }
}

/* If filter match to the prefix then return 1. */
int
filter_match_cisco(struct filter_cisco *filter, struct prefix *p)
{
  //struct filter_cisco *filter;
  struct ipstack_in_addr mask;
  zpl_uint32 check_addr;
  zpl_uint32 check_mask;

  //filter = &mfilter->u.cfilter;
  check_addr = p->u.prefix4.s_addr & ~filter->addr_mask.s_addr;

  if (filter->extended)
  {
    masklen2ip(p->prefixlen, &mask);
    check_mask = mask.s_addr & ~filter->mask_mask.s_addr;

    if (memcmp(&check_addr, &filter->addr.s_addr, 4) == 0 && memcmp(&check_mask, &filter->mask.s_addr, 4) == 0)
      return 1;
  }
  else if (memcmp(&check_addr, &filter->addr.s_addr, 4) == 0)
    return 1;

  return 0;
}

/* If filter match to the prefix then return 1. */
int
filter_match_zebra(struct filter_zebra *filter, struct prefix *p)
{
  //struct filter_zebra *filter;

  //filter = &mfilter->u.zfilter;

  if (filter->prefix.family == p->family)
  {
    if (filter->exact)
    {
      if (filter->prefix.prefixlen == p->prefixlen)
        return prefix_match(&filter->prefix, p);
      else
        return 0;
    }
    else
      return prefix_match(&filter->prefix, p);
  }
  else
    return 0;
}
#ifdef ZPL_FILTER_ZEBRA_EXT
int
filter_match_zebos_ext(struct filter_zebos_ext *filter, struct filter_zebos_ext *p)
{
  //struct filter_zebos_ext *filter;

  //filter = &mfilter->u.zextfilter;

  if ((filter->protocol == p->protocol) &&
      prefix_match(&filter->sprefix, &p->sprefix) &&
      (filter->sport_op = p->sport_op) &&
      (filter->sport == p->sport) &&
      prefix_match(&filter->dprefix, &p->dprefix) &&
      (filter->dport_op == p->dport_op) &&
      (filter->dport == p->dport))
    return 1;

  return 0;
}
#endif
#ifdef ZPL_FILTER_MAC
int
filter_match_mac(struct filter_l2 *filter, struct filter_l2 *p)
{
  //struct filter_l2 *filter;

  //filter = &mfilter->u.mac_filter;

  if (memcmp(filter, p, sizeof(struct filter_l2)) == 0)
    return 1;
  return 0;
}
#endif
/* Allocate new access list structure. */
static struct access_list *
access_list_new(void)
{
  return (struct access_list *)XCALLOC(MTYPE_ACCESS_LIST,
                                       sizeof(struct access_list));
}

/* Free allocated access_list. */
static void
access_list_free(struct access_list *access)
{
  struct filter_list *filter = NULL;
  struct filter_list *next = NULL;
  for (filter = access->head; filter; filter = next)
  {
    next = filter->next;
    filter_free(filter);
  }
  if (access->name)
    XFREE(MTYPE_ACCESS_LIST_STR, access->name);

  if (access->remark)
    XFREE(MTYPE_TMP, access->remark);
  XFREE(MTYPE_ACCESS_LIST, access);
}

static struct access_list *
access_list_lock(struct access_list *access)
{
  access->ref_cnt++;
  return access;
}

static void
access_list_unlock(struct access_list *access)
{
  access->ref_cnt--;
  if (access->ref_cnt == 0)
    access_list_free(access);
}

/* Delete access_list from access_master and free it. */
static void
access_list_delete(struct access_list *access)
{
  struct filter_list *filter;
  struct filter_list *next;
  struct access_list_list *list;
  struct access_master *master;

  master = access->master;

  if (access->type == ACCESS_TYPE_NUMBER)
    list = &master->num;
  else
    list = &master->str;

  if (access->next)
    access->next->prev = access->prev;
  else
    list->tail = access->prev;

  if (access->prev)
    access->prev->next = access->next;
  else
    list->head = access->next;

  access_list_unlock(access);
}

/* Insert new access list to list of access_list.  Each acceess_list
   is sorted by the name. */
static struct access_list *
access_list_insert(afi_t afi, const char *name)
{
  zpl_uint32 i;
  long number;
  struct access_list *access;
  struct access_list *point;
  struct access_list_list *alist;
  struct access_master *master;

  master = access_master_get(afi);
  if (master == NULL)
    return NULL;

  /* Allocate new access_list and copy given name. */
  access = access_list_new();
  access->name = XSTRDUP(MTYPE_ACCESS_LIST_STR, name);
  access->master = master;

  /* If name is made by all digit character.  We treat it as
     number. */
  for (number = 0, i = 0; i < strlen(name); i++)
  {
    if (isdigit((int)name[i]))
      number = (number * 10) + (name[i] - '0');
    else
      break;
  }

  /* In case of name is all digit character */
  if (i == strlen(name))
  {
    access->type = ACCESS_TYPE_NUMBER;

    /* Set access_list to number list. */
    alist = &master->num;

    for (point = alist->head; point; point = point->next)
      if (atol(point->name) >= number)
        break;
  }
  else
  {
    access->type = ACCESS_TYPE_STRING;

    /* Set access_list to string list. */
    alist = &master->str;

    /* Set point to insertion point. */
    for (point = alist->head; point; point = point->next)
      if (strcmp(point->name, name) >= 0)
        break;
  }

  /* In case of this is the first element of master. */
  if (alist->head == NULL)
  {
    alist->head = alist->tail = access;
    return access;
  }

  /* In case of insertion is made at the tail of access_list. */
  if (point == NULL)
  {
    access->prev = alist->tail;
    alist->tail->next = access;
    alist->tail = access;
    return access;
  }

  /* In case of insertion is made at the head of access_list. */
  if (point == alist->head)
  {
    access->next = alist->head;
    alist->head->prev = access;
    alist->head = access;
    return access;
  }

  /* Insertion is made at middle of the access_list. */
  access->next = point;
  access->prev = point->prev;

  if (point->prev)
    point->prev->next = access;
  point->prev = access;

  return access;
}

/* Lookup access_list from list of access_list by name. */
struct access_list *
access_list_lookup(afi_t afi, const char *name)
{
  struct access_list *access;
  struct access_master *master;

  if (name == NULL)
    return NULL;

  master = access_master_get(afi);
  if (master == NULL)
    return NULL;

  for (access = master->num.head; access; access = access->next)
    if (strcmp(access->name, name) == 0)
      return access;

  for (access = master->str.head; access; access = access->next)
    if (strcmp(access->name, name) == 0)
      return access;

  return NULL;
}

int access_list_reference (afi_t afi, const char *name, zpl_bool enable)
{
  struct access_list *node = access_list_lookup( afi, name);
    if(node)
    {
        if(enable)
            node->ref_cnt += 1;
        else if(node->ref_cnt)
        {
            node->ref_cnt -= 1;
        }    
        return OK;
    }
    return ERROR;
}
/* Get access list from list of access_list.  If there isn't matched
   access_list create new one and return it. */
static struct access_list *
access_list_get(afi_t afi, const char *name)
{
  struct access_list *access;

  access = access_list_lookup(afi, name);
  if (access == NULL)
    access = access_list_insert(afi, name);
  return access;
}

/* Apply access list to object (which should be struct prefix *). */
enum filter_type
access_list_apply(struct access_list *access, void *object)
{
  struct filter_list *filter;
  struct prefix *p;

  p = (struct prefix *)object;
#ifdef ZPL_FILTER_ZEBRA_EXT
  struct filter_zebos_ext *exp = (struct filter_zebos_ext *)object;
#endif
#ifdef ZPL_FILTER_MAC
  struct filter_l2 *l2mac = (struct filter_l2 *)object;
#endif
  if (access == NULL)
    return FILTER_DENY;

  for (filter = access->head; filter; filter = filter->next)
  {
    if (filter->nodetype == FILTER_COMMON)
    {
      if (filter_match_cisco(&filter->u.cfilter, p))
        return filter->type;
    }
    else if (filter->nodetype == FILTER_ZEBOS)
    {
      if (filter_match_zebra(&filter->u.zfilter, p))
        return filter->type;
    }
#ifdef ZPL_FILTER_ZEBRA_EXT
    else if (filter->nodetype == FILTER_ZEBOS_EXT)
    {
      if (filter_match_zebos_ext(&filter->u.zextfilter, exp))
        return filter->type;
    }
#endif
#ifdef ZPL_FILTER_MAC
    else if (filter->nodetype == FILTER_MAC)
    {
      if (filter_match_mac(&filter->u.mac_filter, l2mac))
        return filter->type;
    }
#endif
  }
  return FILTER_DENY;
}


/* Add hook function. */
void access_list_add_hook(zpl_uint32 module, void (*func)(struct access_list *access))
{
  if (INT_MAX_MIN_SPACE(module, MODULE_NONE, MODULE_MAX))
  {
    access_master_ipv4.add_hook[module] = func;
#ifdef ZPL_BUILD_IPV6
    access_master_ipv6.add_hook[module] = func;
#endif /* ZPL_BUILD_IPV6 */
  }
}

/* Delete hook function. */
void access_list_delete_hook(zpl_uint32 module, void (*func)(struct access_list *access))
{
  if (INT_MAX_MIN_SPACE(module, MODULE_NONE, MODULE_MAX))
  {
    access_master_ipv4.delete_hook[module] = func;
#ifdef ZPL_BUILD_IPV6
    access_master_ipv6.delete_hook[module] = func;
#endif /* ZPL_BUILD_IPV6 */
  }
}
static int access_list_hook_callback(zpl_uint32 type, struct access_list *access)
{
  zpl_uint32 i = 0;
  for (i = 0; i < MODULE_MAX; i++)
  {
    if (type == 1)
    {
      if (access->master->add_hook[i])
        (access->master->add_hook[i])(access);
    }
    else if (type == 0)
    {
      if (access->master->delete_hook[i])
        (access->master->delete_hook[i])(access);
    }
  }
  return 0;
}
/* Add new filter to the end of specified access_list. */
static void
access_list_filter_add(struct access_list *access, struct filter_list *filter)
{
  filter->next = NULL;
  filter->prev = access->tail;

  if (access->tail)
    access->tail->next = filter;
  else
    access->head = filter;
  access->tail = filter;

  /* Run hook function. */
  access_list_hook_callback(1, access);
  /*
    if (access->master->add_hook)
      (*access->master->add_hook) (access);*/
}

/* If access_list has no filter then return 1. */
static int
access_list_empty(struct access_list *access)
{
  if (access->head == NULL && access->tail == NULL)
    return 1;
  else
    return 0;
}

/* Delete filter from specified access_list.  If there is hook
   function execute it. */
static void
access_list_filter_delete(struct access_list *access, struct filter_list *filter)
{
  //struct access_master *master;

  //master = access->master;

  if (filter->next)
    filter->next->prev = filter->prev;
  else
    access->tail = filter->prev;

  if (filter->prev)
    filter->prev->next = filter->next;
  else
    access->head = filter->next;
  access_list_lock(access);

  /* If access_list becomes empty delete it from access_master. */
  if (access_list_empty(access))
    access_list_delete(access);

  /* Run hook function. */
  access_list_hook_callback(0, access);

  access_list_unlock(access);
  filter_free(filter);

  /*  if (master->delete_hook)
      (*master->delete_hook) (access);*/
}

/*
  deny    Specify packets to reject
  permit  Specify packets to forward
  dynamic ?
*/

/*
  Hostname or A.B.C.D  Address to match
  any                  Any source host
  host                 A single host address
*/
int filter_compare_cisco(struct filter_cisco *filter, struct filter_cisco *new)
{
    if (filter->extended)
    {
      if (filter->addr.s_addr == new->addr.s_addr &&
          filter->addr_mask.s_addr == new->addr_mask.s_addr &&
          filter->mask.s_addr == new->mask.s_addr &&
          filter->mask_mask.s_addr == new->mask_mask.s_addr)
        return 1;
    }
    else
    {
      if (filter->addr.s_addr == new->addr.s_addr &&
          filter->addr_mask.s_addr == new->addr_mask.s_addr)
        return 1;
    }
  return 0;
}

static struct filter_list *
filter_lookup_cisco_local(struct access_list *access, struct filter_list *mnew)
{
  struct filter_list *mfilter;
  struct filter_cisco *filter;
  struct filter_cisco *new;

  new = &mnew->u.cfilter;

  for (mfilter = access->head; mfilter; mfilter = mfilter->next)
  {
    filter = &mfilter->u.cfilter;

    if (filter->extended)
    {
      if (mfilter->type == mnew->type &&
           filter_compare_cisco(filter, new) == 1)
        return mfilter;
    }
    else
    {
      if (mfilter->type == mnew->type &&
          filter_compare_cisco(filter, new) == 1)
        return mfilter;
    }
  }

  return NULL;
}

int filter_compare_zebra(struct filter_zebra *filter, struct filter_zebra *new)
{
    if (filter->exact == new->exact && prefix_same(&filter->prefix, &new->prefix))
      return 1;
  return 0;
}

static struct filter_list *
filter_lookup_zebra_local(struct access_list *access, struct filter_list *mnew)
{
  struct filter_list *mfilter;
  struct filter_zebra *filter;
  struct filter_zebra *new;

  new = &mnew->u.zfilter;

  for (mfilter = access->head; mfilter; mfilter = mfilter->next)
  {
    filter = &mfilter->u.zfilter;

    if (mfilter->type == mnew->type && filter_compare_zebra(filter, new))
      return mfilter;
  }
  return NULL;
}
#ifdef ZPL_FILTER_ZEBRA_EXT
int filter_compare_zebos_extended(struct filter_zebos_ext *filter, struct filter_zebos_ext *new)
{
    if (filter->protocol == new->protocol &&
        prefix_same(&filter->sprefix, &new->sprefix) &&
        filter->sport_op == new->sport_op &&
        filter->sport == new->sport &&
                             prefix_same(&filter->dprefix, &new->dprefix) &&
        filter->dport_op == new->dport_op &&
        filter->dport == new->dport)
      return 1;
  return 0;
}
static struct filter_list *
filter_lookup_zebos_extended_local(struct access_list *access,
                             struct filter_list *mnew)
{
  struct filter_list *mfilter;
  struct filter_zebos_ext *filter;
  struct filter_zebos_ext *new;

  new = &mnew->u.zextfilter;

  for (mfilter = access->head; mfilter; mfilter = mfilter->next)
  {
    filter = &mfilter->u.zextfilter;

    if (mfilter->type == mnew->type && filter_compare_zebos_extended(filter, new))
      return mfilter;
  }
  return NULL;
}
#endif

#ifdef ZPL_FILTER_MAC
int filter_compare_l2mac(struct filter_l2 *filter, struct filter_l2 *new)
{
  if (memcmp(filter, new, sizeof(struct filter_l2)) == 0)
    return 1;
  return 0;
}

static struct filter_list *
filter_lookup_mac_local(struct access_list *access,
                  struct filter_list *mnew)
{
  struct filter_list *mfilter;
  struct filter_l2 *filter;
  struct filter_l2 *new;

  new = &mnew->u.zextfilter;

  for (mfilter = access->head; mfilter; mfilter = mfilter->next)
  {
    filter = &mfilter->u.zextfilter;
    if (mfilter->type == mnew->type && filter_compare_l2mac(filter, new))
      return mfilter;
  }
  return NULL;
}
#endif

#ifdef ZPL_SHELL_MODULE
static int
vty_access_list_remark_unset(struct vty *vty, afi_t afi, const char *name)
{
  struct access_list *access;

  access = access_list_lookup(afi, name);
  if (!access)
  {
    vty_out(vty, "%% access-list %s doesn't exist%s", name,
            VTY_NEWLINE);
    return CMD_WARNING;
  }

  if (access->remark)
  {
    XFREE(MTYPE_TMP, access->remark);
    access->remark = NULL;
  }

  if (access->head == NULL && access->tail == NULL && access->remark == NULL)
    access_list_delete(access);

  return CMD_SUCCESS;
}

int filter_cisco_format(struct vty *vty, const char *addr_str, const char *addr_mask_str,
                 const char *mask_str, const char *mask_mask_str,
                 int extended, struct filter_cisco *filter)
{
  struct ipstack_in_addr addr;
  struct ipstack_in_addr addr_mask;
  struct ipstack_in_addr mask;
  struct ipstack_in_addr mask_mask;
  int ret = 0;

  ret = ipstack_inet_aton(addr_str, &addr);
  if (ret <= 0)
  {
    vty_out(vty, "%%Inconsistent address and mask%s",
            VTY_NEWLINE);
    return CMD_WARNING;
  }

  ret = ipstack_inet_aton(addr_mask_str, &addr_mask);
  if (ret <= 0)
  {
    vty_out(vty, "%%Inconsistent address and mask%s",
            VTY_NEWLINE);
    return CMD_WARNING;
  }
		if(IPV4_NET127(addr.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(addr.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
  if (extended)
  {
    ret = ipstack_inet_aton(mask_str, &mask);
    if (ret <= 0)
    {
      vty_out(vty, "%%Inconsistent address and mask%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }

    ret = ipstack_inet_aton(mask_mask_str, &mask_mask);
    if (ret <= 0)
    {
      vty_out(vty, "%%Inconsistent address and mask%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
  filter->extended = extended;
  filter->addr.s_addr = addr.s_addr & ~addr_mask.s_addr;
  filter->addr_mask.s_addr = addr_mask.s_addr;

  if (extended)
  {
    filter->mask.s_addr = mask.s_addr & ~mask_mask.s_addr;
    filter->mask_mask.s_addr = mask_mask.s_addr;
  }
  return CMD_SUCCESS;
}

static int
filter_set_cisco(struct vty *vty, const char *name_str, const char *type_str,
                 const char *addr_str, const char *addr_mask_str,
                 const char *mask_str, const char *mask_mask_str,
                 int extended, int set)
{
  int ret;
  enum filter_type type;
  struct filter_list *mfilter;
  struct filter_cisco *filter;
  struct access_list *access;
  struct ipstack_in_addr addr;
  struct ipstack_in_addr addr_mask;
  struct ipstack_in_addr mask;
  struct ipstack_in_addr mask_mask;
  int access_num = 0;
  /* Access-list name check. */
  access_num = atoi(name_str);
  if (access_num == UINT32_MAX)
  {
    vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  if (extended)
  {
    /* Extended access-list name range check. */
    if (access_num < 100 || access_num > 2699)
    {
      vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
    if (access_num > 199 && access_num < 2000)
    {
      vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
  else
  {
    /* Standard access-list name range check. */
    if (access_num < 1 || access_num > 1999)
    {
      vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
    if (access_num > 99 && access_num < 1300)
    {
      vty_out(vty, "%% filter name rang invalid%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
  /* Check of filter type. */
  if (strncmp(type_str, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp(type_str, "d", 1) == 0)
    type = FILTER_DENY;
  else
  {
    vty_out(vty, "%% filter type must be permit or deny%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  ret = ipstack_inet_aton(addr_str, &addr);
  if (ret <= 0)
  {
    vty_out(vty, "%%Inconsistent address and mask%s",
            VTY_NEWLINE);
    return CMD_WARNING;
  }
		if(IPV4_NET127(addr.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(addr.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
  ret = ipstack_inet_aton(addr_mask_str, &addr_mask);
  if (ret <= 0)
  {
    vty_out(vty, "%%Inconsistent address and mask%s",
            VTY_NEWLINE);
    return CMD_WARNING;
  }

  if (extended)
  {
    ret = ipstack_inet_aton(mask_str, &mask);
    if (ret <= 0)
    {
      vty_out(vty, "%%Inconsistent address and mask%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }

    ret = ipstack_inet_aton(mask_mask_str, &mask_mask);
    if (ret <= 0)
    {
      vty_out(vty, "%%Inconsistent address and mask%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
  }

  mfilter = filter_new();
  mfilter->type = type;
  mfilter->nodetype = FILTER_COMMON;
  filter = &mfilter->u.cfilter;
  filter->extended = extended;
  filter->addr.s_addr = addr.s_addr & ~addr_mask.s_addr;
  filter->addr_mask.s_addr = addr_mask.s_addr;

  if (extended)
  {
    filter->mask.s_addr = mask.s_addr & ~mask_mask.s_addr;
    filter->mask_mask.s_addr = mask_mask.s_addr;
  }

  /* Install new filter to the access_list. */
  access = access_list_get(AFI_IP, name_str);

  if (set)
  {
    if (filter_lookup_cisco_local(access, mfilter))
      filter_free(mfilter);
    else
      access_list_filter_add(access, mfilter);
  }
  else
  {
    struct filter_list *delete_filter;

    delete_filter = filter_lookup_cisco_local(access, mfilter);
    if (delete_filter)
      access_list_filter_delete(access, delete_filter);

    filter_free(mfilter);
  }

  return CMD_SUCCESS;
}

/* Standard access-list */
DEFUN(access_list_standard,
      access_list_standard_cmd,
      "ip access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Address to match\n"
      "Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], argv[3],
                          NULL, NULL, 0, 1);
}

DEFUN(access_list_standard_nomask,
      access_list_standard_nomask_cmd,
      "ip access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Address to match\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], "0.0.0.0",
                          NULL, NULL, 0, 1);
}

DEFUN(access_list_standard_host,
      access_list_standard_host_cmd,
      "ip access-list (<1-99>|<1300-1999>) (deny|permit) host A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "A single host address\n"
      "Address to match\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], "0.0.0.0",
                          NULL, NULL, 0, 1);
}

DEFUN(access_list_standard_any,
      access_list_standard_any_cmd,
      "ip access-list (<1-99>|<1300-1999>) (deny|permit) any",
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any source host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", NULL, NULL, 0, 1);
}

DEFUN(no_access_list_standard,
      no_access_list_standard_cmd,
      "no ip access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Address to match\n"
      "Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], argv[3],
                          NULL, NULL, 0, 0);
}

DEFUN(no_access_list_standard_nomask,
      no_access_list_standard_nomask_cmd,
      "no ip access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Address to match\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], "0.0.0.0",
                          NULL, NULL, 0, 0);
}

DEFUN(no_access_list_standard_host,
      no_access_list_standard_host_cmd,
      "no ip access-list (<1-99>|<1300-1999>) (deny|permit) host A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "A single host address\n"
      "Address to match\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2], "0.0.0.0",
                          NULL, NULL, 0, 0);
}

DEFUN(no_access_list_standard_any,
      no_access_list_standard_any_cmd,
      "no ip access-list (<1-99>|<1300-1999>) (deny|permit) any",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP standard access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any source host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", NULL, NULL, 0, 0);
}

/* Extended access-list */
DEFUN(access_list_extended,
      access_list_extended_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], argv[4], argv[5], 1, 1);
}

DEFUN(access_list_extended_mask_any,
      access_list_extended_mask_any_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D any",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], "0.0.0.0",
                          "255.255.255.255", 1, 1);
}

DEFUN(access_list_extended_any_mask,
      access_list_extended_any_mask_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any A.B.C.D A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", argv[2],
                          argv[3], 1, 1);
}

DEFUN(access_list_extended_any_any,
      access_list_extended_any_any_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any any",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", "0.0.0.0",
                          "255.255.255.255", 1, 1);
}

DEFUN(access_list_extended_mask_host,
      access_list_extended_mask_host_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D host A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], argv[4],
                          "0.0.0.0", 1, 1);
}

DEFUN(access_list_extended_host_mask,
      access_list_extended_host_mask_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D A.B.C.D A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", argv[3],
                          argv[4], 1, 1);
}

DEFUN(access_list_extended_host_host,
      access_list_extended_host_host_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D host A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", argv[3],
                          "0.0.0.0", 1, 1);
}

DEFUN(access_list_extended_any_host,
      access_list_extended_any_host_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any host A.B.C.D",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", argv[2],
                          "0.0.0.0", 1, 1);
}

DEFUN(access_list_extended_host_any,
      access_list_extended_host_any_cmd,
      "ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D any",
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", "0.0.0.0",
                          "255.255.255.255", 1, 1);
}

DEFUN(no_access_list_extended,
      no_access_list_extended_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], argv[4], argv[5], 1, 0);
}

DEFUN(no_access_list_extended_mask_any,
      no_access_list_extended_mask_any_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D any",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], "0.0.0.0",
                          "255.255.255.255", 1, 0);
}

DEFUN(no_access_list_extended_any_mask,
      no_access_list_extended_any_mask_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any A.B.C.D A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", argv[2],
                          argv[3], 1, 0);
}

DEFUN(no_access_list_extended_any_any,
      no_access_list_extended_any_any_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any any",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", "0.0.0.0",
                          "255.255.255.255", 1, 0);
}

DEFUN(no_access_list_extended_mask_host,
      no_access_list_extended_mask_host_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D host A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Source address\n"
      "Source wildcard bits\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          argv[3], argv[4],
                          "0.0.0.0", 1, 0);
}

DEFUN(no_access_list_extended_host_mask,
      no_access_list_extended_host_mask_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D A.B.C.D A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "Destination address\n"
      "Destination Wildcard bits\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", argv[3],
                          argv[4], 1, 0);
}

DEFUN(no_access_list_extended_host_host,
      no_access_list_extended_host_host_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D host A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", argv[3],
                          "0.0.0.0", 1, 0);
}

DEFUN(no_access_list_extended_any_host,
      no_access_list_extended_any_host_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip any host A.B.C.D",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "Any source host\n"
      "A single destination host\n"
      "Destination address\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], "0.0.0.0",
                          "255.255.255.255", argv[2],
                          "0.0.0.0", 1, 0);
}

DEFUN(no_access_list_extended_host_any,
      no_access_list_extended_host_any_cmd,
      "no ip access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D any",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access list\n"
      "IP extended access list (expanded range)\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any Internet Protocol\n"
      "A single source host\n"
      "Source address\n"
      "Any destination host\n")
{
  return filter_set_cisco(vty, argv[0], argv[1], argv[2],
                          "0.0.0.0", "0.0.0.0",
                          "255.255.255.255", 1, 0);
}

int filter_zebra_format(struct vty *vty, afi_t afi, const char *prefix_str, int exact, struct filter_zebra *filter)
{
  int ret;
  struct prefix p;

  /* Check string format of prefix and prefixlen. */
  if (afi == AFI_IP)
  {
    ret = str2prefix_ipv4(prefix_str, (struct prefix_ipv4 *)&p);
    if (ret <= 0)
    {
      vty_out(vty, "IP address prefix/prefixlen is malformed%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#ifdef ZPL_BUILD_IPV6
  else if (afi == AFI_IP6)
  {
    ret = str2prefix_ipv6(prefix_str, (struct prefix_ipv6 *)&p);
    if (ret <= 0)
    {
      vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#endif /* ZPL_BUILD_IPV6 */
  else
    return CMD_WARNING;

  prefix_copy(&filter->prefix, &p);

  /* "exact-match" */
  if (exact)
    filter->exact = 1;
  return CMD_SUCCESS;
}

static int
filter_set_zebra(struct vty *vty, const char *name_str, const char *type_str,
                 afi_t afi, const char *prefix_str, int exact, int set)
{
  int ret;
  enum filter_type type;
  struct filter_list *mfilter;
  struct filter_zebra *filter;
  struct access_list *access;
  struct prefix p;

  /* Check of filter type. */
  if (strncmp(type_str, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp(type_str, "d", 1) == 0)
    type = FILTER_DENY;
  else
  {
    vty_out(vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Check string format of prefix and prefixlen. */
  if (afi == AFI_IP)
  {
    ret = str2prefix_ipv4(prefix_str, (struct prefix_ipv4 *)&p);
    if (ret <= 0)
    {
      vty_out(vty, "IP address prefix/prefixlen is malformed%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
		if(IPV4_NET127(p.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(p.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
  }
#ifdef ZPL_BUILD_IPV6
  else if (afi == AFI_IP6)
  {
    ret = str2prefix_ipv6(prefix_str, (struct prefix_ipv6 *)&p);
    if (ret <= 0)
    {
      vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s",
              VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#endif /* ZPL_BUILD_IPV6 */
  else
    return CMD_WARNING;

  mfilter = filter_new();
  mfilter->type = type;
  mfilter->nodetype = FILTER_ZEBOS;
  filter = &mfilter->u.zfilter;
  prefix_copy(&filter->prefix, &p);

  /* "exact-match" */
  if (exact)
    filter->exact = 1;

  /* Install new filter to the access_list. */
  access = access_list_get(afi, name_str);

  if (set)
  {
    if (filter_lookup_zebra_local(access, mfilter))
      filter_free(mfilter);
    else
      access_list_filter_add(access, mfilter);
  }
  else
  {
    struct filter_list *delete_filter;

    delete_filter = filter_lookup_zebra_local(access, mfilter);
    if (delete_filter)
      access_list_filter_delete(access, delete_filter);

    filter_free(mfilter);
  }

  return CMD_SUCCESS;
}

/* Zebra access-list */
DEFUN(access_list,
      access_list_cmd,
      "ip access-list WORD (deny|permit) A.B.C.D/M",
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, argv[2], 0, 1);
}

DEFUN(access_list_exact,
      access_list_exact_cmd,
      "ip access-list WORD (deny|permit) A.B.C.D/M exact-match",
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n"
      "Exact match of the prefixes\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, argv[2], 1, 1);
}

DEFUN(access_list_any,
      access_list_any_cmd,
      "ip access-list WORD (deny|permit) any",
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, "0.0.0.0/0", 0, 1);
}

DEFUN(no_access_list,
      no_access_list_cmd,
      "no ip access-list WORD (deny|permit) A.B.C.D/M",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, argv[2], 0, 0);
}

DEFUN(no_access_list_exact,
      no_access_list_exact_cmd,
      "no ip access-list WORD (deny|permit) A.B.C.D/M exact-match",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n"
      "Exact match of the prefixes\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, argv[2], 1, 0);
}

DEFUN(no_access_list_any,
      no_access_list_any_cmd,
      "no ip access-list WORD (deny|permit) any",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP zebra access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP, "0.0.0.0/0", 0, 0);
}

DEFUN(no_access_list_all,
      no_access_list_all_cmd,
      "no ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD)",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP extended access list\n"
      "IP standard access list (expanded range)\n"
      "IP extended access list (expanded range)\n"
      "IP zebra access-list name\n")
{
  struct access_list *access;
  //struct access_master *master;

  /* Looking up access_list. */
  access = access_list_lookup(AFI_IP, argv[0]);
  if (access == NULL)
  {
    vty_out(vty, "%% access-list %s doesn't exist%s", argv[0],
            VTY_NEWLINE);
    return CMD_WARNING;
  }

  //master = access->master;

  /* Run hook function. */
  access_list_hook_callback(0, access);
  /*  if (master->delete_hook)
      (*master->delete_hook) (access);*/

  /* Delete all filter from access-list. */
  access_list_delete(access);

  return CMD_SUCCESS;
}

DEFUN(access_list_remark,
      access_list_remark_cmd,
      "ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark .LINE",
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP extended access list\n"
      "IP standard access list (expanded range)\n"
      "IP extended access list (expanded range)\n"
      "IP zebra access-list\n"
      "Access list entry comment\n"
      "Comment up to 100 characters\n")
{
  struct access_list *access;

  access = access_list_get(AFI_IP, argv[0]);

  if (access->remark)
  {
    XFREE(MTYPE_TMP, access->remark);
    access->remark = NULL;
  }
  access->remark = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}

DEFUN(no_access_list_remark,
      no_access_list_remark_cmd,
      "no ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP extended access list\n"
      "IP standard access list (expanded range)\n"
      "IP extended access list (expanded range)\n"
      "IP zebra access-list\n"
      "Access list entry comment\n")
{
  return vty_access_list_remark_unset(vty, AFI_IP, argv[0]);
}

ALIAS(no_access_list_remark,
      no_access_list_remark_arg_cmd,
      "no ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark .LINE",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP standard access list\n"
      "IP extended access list\n"
      "IP standard access list (expanded range)\n"
      "IP extended access list (expanded range)\n"
      "IP zebra access-list\n"
      "Access list entry comment\n"
      "Comment up to 100 characters\n")

#ifdef ZPL_BUILD_IPV6
DEFUN(ipv6_access_list,
      ipv6_access_list_cmd,
      "ipv6 access-list WORD (deny|permit) X:X::X:X/M",
      IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 3ffe:506::/32\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, argv[2], 0, 1);
}

DEFUN(ipv6_access_list_exact,
      ipv6_access_list_exact_cmd,
      "ipv6 access-list WORD (deny|permit) X:X::X:X/M exact-match",
      IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 3ffe:506::/32\n"
      "Exact match of the prefixes\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, argv[2], 1, 1);
}

DEFUN(ipv6_access_list_any,
      ipv6_access_list_any_cmd,
      "ipv6 access-list WORD (deny|permit) any",
      IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any prefixi to match\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, "::/0", 0, 1);
}

DEFUN(no_ipv6_access_list,
      no_ipv6_access_list_cmd,
      "no ipv6 access-list WORD (deny|permit) X:X::X:X/M",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 3ffe:506::/32\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, argv[2], 0, 0);
}

DEFUN(no_ipv6_access_list_exact,
      no_ipv6_access_list_exact_cmd,
      "no ipv6 access-list WORD (deny|permit) X:X::X:X/M exact-match",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Prefix to match. e.g. 3ffe:506::/32\n"
      "Exact match of the prefixes\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, argv[2], 1, 0);
}

DEFUN(no_ipv6_access_list_any,
      no_ipv6_access_list_any_cmd,
      "no ipv6 access-list WORD (deny|permit) any",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Any prefixi to match\n")
{
  return filter_set_zebra(vty, argv[0], argv[1], AFI_IP6, "::/0", 0, 0);
}

DEFUN(no_ipv6_access_list_all,
      no_ipv6_access_list_all_cmd,
      "no ipv6 access-list WORD",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n")
{
  struct access_list *access;
  //struct access_master *master;

  /* Looking up access_list. */
  access = access_list_lookup(AFI_IP6, argv[0]);
  if (access == NULL)
  {
    vty_out(vty, "%% access-list %s doesn't exist%s", argv[0],
            VTY_NEWLINE);
    return CMD_WARNING;
  }

  //master = access->master;

  /* Run hook function. */
  access_list_hook_callback(0, access);
  /*  if (master->delete_hook)
      (*master->delete_hook) (access);*/

  /* Delete all filter from access-list. */
  access_list_delete(access);

  return CMD_SUCCESS;
}

DEFUN(ipv6_access_list_remark,
      ipv6_access_list_remark_cmd,
      "ipv6 access-list WORD remark .LINE",
      IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Access list entry comment\n"
      "Comment up to 100 characters\n")
{
  struct access_list *access;

  access = access_list_get(AFI_IP6, argv[0]);

  if (access->remark)
  {
    XFREE(MTYPE_TMP, access->remark);
    access->remark = NULL;
  }
  access->remark = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}

DEFUN(no_ipv6_access_list_remark,
      no_ipv6_access_list_remark_cmd,
      "no ipv6 access-list WORD remark",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Access list entry comment\n")
{
  return vty_access_list_remark_unset(vty, AFI_IP6, argv[0]);
}

ALIAS(no_ipv6_access_list_remark,
      no_ipv6_access_list_remark_arg_cmd,
      "no ipv6 access-list WORD remark .LINE",
      NO_STR
          IPV6_STR
      "Add an access list entry\n"
      "IPv6 zebra access-list\n"
      "Access list entry comment\n"
      "Comment up to 100 characters\n")
#endif /* ZPL_BUILD_IPV6 */

#ifdef ZPL_FILTER_ZEBRA_EXT
int filter_zebos_ext_format(struct vty *vty, afi_t afi,
                                     const char *prot_str, const char *sprefix,
                                     const char *sport_op, const char *sport, const char *seport,
                                     const char *dprefix, const char *dport_op,
                                     const char *dport, const char *deport, struct filter_zebos_ext *filter)
{
  int ret;  
  struct prefix sp, dp;

  /* Check string format of prefix and prefixlen. */
  if (afi == AFI_IP)
  {
    if (!strncmp(sprefix, "a", 1))
    {
      sp.family = IPSTACK_AF_INET;
      sp.prefixlen = 0;
      sp.u.prefix4.s_addr = 0;
    }
    else
    {
      ret = str2prefix_ipv4(sprefix, (struct prefix_ipv4 *)&sp);
      if (ret <= 0)
      {
        vty_out(vty, "IP address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
		if(IPV4_NET127(sp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(sp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
    }

    if (!strncmp(dprefix, "a", 1))
    {
      dp.family = IPSTACK_AF_INET;
      dp.prefixlen = 0;
      dp.u.prefix4.s_addr = 0;
    }
    else
    {
      ret = str2prefix_ipv4(dprefix, (struct prefix_ipv4 *)&dp);
      if (ret <= 0)
      {
        vty_out(vty, "IP address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
		if(IPV4_NET127(dp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(dp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
    }
  }
#ifdef ZPL_BUILD_IPV6
  else if (afi == AFI_IP6)
  {
    if (!strncmp(sprefix, "a", 1))
    {
      int i;

      sp.family = IPSTACK_AF_INET6;
      sp.prefixlen = 0;

      for (i = 0; i < 16; i++)
        sp.u.prefix6.s6_addr[i] = 0;
    }
    else
    {
      ret = str2prefix_ipv6(sprefix, (struct prefix_ipv6 *)&sp);
      if (ret <= 0)
      {
        vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
    }

    if (!strncmp(dprefix, "a", 1))
    {
      int i;

      dp.family = IPSTACK_AF_INET6;
      dp.prefixlen = 0;

      for (i = 0; i < 16; i++)
        sp.u.prefix6.s6_addr[i] = 0;
    }
    else
    {
      ret = str2prefix_ipv6(dprefix, (struct prefix_ipv6 *)&dp);
      if (ret <= 0)
      {
        vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
    }
  }
#endif /* ZPL_BUILD_IPV6 */
  else
    return CMD_WARNING;

  prefix_copy(&filter->sprefix, &sp);
  prefix_copy(&filter->dprefix, &dp);
  if (prot_str)
    filter->protocol = ip_protocol_type(prot_str);
  else  
    filter->protocol = IP_IPPROTO_MAX;
  if (sport_op)
  {
    filter->sport_op = port_operation_type(sport_op);
    if (sport)
      sscanf(sport, "%d", &filter->sport);
  }
  if (dport_op)
  {
    filter->dport_op = port_operation_type(dport_op);
    if (dport)
      sscanf(dport, "%d", &filter->dport);
  }
  return CMD_SUCCESS;
}      

static int filter_set_zebos_extended(struct vty *vty, const char *name_str,
                                     const char *type_str, afi_t afi,
                                     const char *prot_str, const char *sprefix,
                                     const char *sport_op, const char *sport, const char *seport,
                                     const char *dprefix, const char *dport_op,
                                     const char *dport, const char *deport, u_char set)
{
  int ret;
  enum filter_type type;
  struct filter_list *mfilter;
  struct filter_zebos_ext *filter;
  struct access_list *access;
  struct prefix sp, dp;

  /* Check of filter type. */
  if (strncmp(type_str, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp(type_str, "d", 1) == 0)
    type = FILTER_DENY;
  else
  {
    vty_out(vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Check string format of prefix and prefixlen. */
  if (afi == AFI_IP)
  {
    if (!strncmp(sprefix, "a", 1))
    {
      sp.family = IPSTACK_AF_INET;
      sp.prefixlen = 0;
      sp.u.prefix4.s_addr = 0;
    }
    else
    {
      ret = str2prefix_ipv4(sprefix, (struct prefix_ipv4 *)&sp);
      if (ret <= 0)
      {
        vty_out(vty, "IP address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
		if(IPV4_NET127(sp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(sp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
    }

    if (!strncmp(dprefix, "a", 1))
    {
      dp.family = IPSTACK_AF_INET;
      dp.prefixlen = 0;
      dp.u.prefix4.s_addr = 0;
    }
    else
    {
      ret = str2prefix_ipv4(dprefix, (struct prefix_ipv4 *)&dp);
      if (ret <= 0)
      {
        vty_out(vty, "IP address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
		if(IPV4_NET127(dp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Lookback address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
		if(IPV4_MULTICAST(dp.u.prefix4.s_addr))
		{
			vty_out (vty, "%% Multicast address%s", VTY_NEWLINE);
			return CMD_WARNING;
		}
    }
  }
#ifdef ZPL_BUILD_IPV6
  else if (afi == AFI_IP6)
  {
    if (!strncmp(sprefix, "a", 1))
    {
      int i;

      sp.family = IPSTACK_AF_INET6;
      sp.prefixlen = 0;

      for (i = 0; i < 16; i++)
        sp.u.prefix6.s6_addr[i] = 0;
    }
    else
    {
      ret = str2prefix_ipv6(sprefix, (struct prefix_ipv6 *)&sp);
      if (ret <= 0)
      {
        vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
    }

    if (!strncmp(dprefix, "a", 1))
    {
      int i;

      dp.family = IPSTACK_AF_INET6;
      dp.prefixlen = 0;

      for (i = 0; i < 16; i++)
        sp.u.prefix6.s6_addr[i] = 0;
    }
    else
    {
      ret = str2prefix_ipv6(dprefix, (struct prefix_ipv6 *)&dp);
      if (ret <= 0)
      {
        vty_out(vty, "IPv6 address prefix/prefixlen is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
    }
  }
#endif /* ZPL_BUILD_IPV6 */
  else
    return CMD_WARNING;

  mfilter = filter_new();
  mfilter->type = type;
  mfilter->nodetype = FILTER_ZEBOS_EXT;
  filter = &mfilter->u.zextfilter;
  prefix_copy(&filter->sprefix, &sp);
  prefix_copy(&filter->dprefix, &dp);
  if (prot_str)
    filter->protocol = ip_protocol_type(prot_str);
  else
  filter->protocol = IP_IPPROTO_MAX;
  if (sport_op)
  {
    filter->sport_op = port_operation_type(sport_op);
    if (sport)
      sscanf(sport, "%d", &filter->sport);
  }
  if (dport_op)
  {
    filter->dport_op = port_operation_type(dport_op);
    if (dport)
      sscanf(dport, "%d", &filter->dport);
  }

  /* Install/Get new filter to the access_list. */
  if (set)
    access = access_list_get(afi, name_str);
  else
  {
    access = access_list_lookup(afi, name_str);
    if (!access)
    {
      vty_out(vty, "Can't delete access-list. Access-list '%s' doesn't exist.%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  }

  if (set)
  {
    if (filter_lookup_zebos_extended_local(access, mfilter))
      filter_free(mfilter);
    else
      access_list_filter_add(access, mfilter);
  }
  else
  {
    struct filter_list *delete_filter;

    delete_filter = filter_lookup_zebos_extended_local(access, mfilter);
    if (delete_filter && delete_filter->nodetype != FILTER_ZEBOS_EXT)
    {
      filter_free(mfilter);
      return CMD_WARNING;
    }

    if (delete_filter && !delete_filter->next && !delete_filter->prev && access->ref_cnt > 1)
    {
      vty_out(vty, "Can't delete access-list. Access-list '%s' being used.%s", access->name, VTY_NEWLINE);
      return CMD_WARNING;
    }

    if (delete_filter)
      access_list_filter_delete(access, delete_filter);

    filter_free(mfilter);
  }
  return CMD_SUCCESS;
}

/*
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536>

access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port any
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-gport <1-65536> <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port any dest-port (ne|gt|lt|ge|le) <1-65536>

access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port any
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-gport <1-65536> <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port any
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-gport <1-65536> <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port any
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-gport <1-65536> <1-65536>
access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (tcp|udp) src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>

access-list  (WORD|<1-2147483646>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) (ip|ipip|egp|rsvp|gre|rsvp|esp|pim|sctp|mpls|raw|ospf|icmp|igmp)

access-list  (WORD|<1-2147483646>) (deny|permit) A.B.C.D/M  any
access-list  (WORD|<1-2147483646>) (deny|permit) any A.B.C.D/M any
access-list  (WORD|<1-2147483646>) (deny|permit) A.B.C.D/M A.B.C.D/M any
*/
DEFUN(access_list_ip_protocol,
      access_list_ip_protocol_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD IP_IPPROTO_CMD")",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      IP_IPPROTO_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_access_list_ip_protocol,
      no_access_list_ip_protocol_cmd,
      "no ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD IP_IPPROTO_CMD")",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      IP_IPPROTO_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_access_list_ip_tcpudp,
      no_access_list_ip_tcpudp_cmd,
      "no ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD")",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    sport_op = "eq";
    sport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      sport_op = argv[5];
      sport = argv[6];
    }
    else
    {  
      sport_op = "eq";
      sport = argv[5];
      seport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_access_list_ip_tcpudp,
      no_access_list_ip_tcpudp_name_cmd,
      "no ip access-list (WORD|<4000-4999>)",
      NO_STR
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n")

DEFUN(access_list_ip_tcpudp,
      access_list_ip_tcpudp_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD")",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    sport_op = "eq";
    sport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      sport_op = argv[5];
      sport = argv[6];
    }
    else
    {  
      sport_op = "rn";
      sport = argv[5];
      seport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(access_list_ip_tcpudp,
      access_list_ip_tcpudp_srcport_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")
 
ALIAS(access_list_ip_tcpudp,
      access_list_ip_tcpudp_srcport_range_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")    

ALIAS(access_list_ip_tcpudp,
      access_list_ip_tcpudp_srcport_ng_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")  


DEFUN(access_list_ip_tcpudp_dstport,
      access_list_ip_tcpudp_dstport_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port any dest-port eq <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = "any";
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    dport_op = "eq";
    dport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      dport_op = argv[5];
      dport = argv[6];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[5];
      deport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ip_tcpudp_dstport,
      access_list_ip_tcpudp_dstport_range_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port any dest-port range <1-65536> <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ip_tcpudp_dstport,
      access_list_ip_tcpudp_dstport_ng_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ip_tcpudp_srcport_dstport,
      access_list_ip_tcpudp_srcport_dstport_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port eq <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = argv[5];
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 7)
  {
    dport_op = "eq";
    dport = argv[6];
  }
  else if(argc == 8)
  {
    if(port_operation_type(argv[6]) == OPT_NONE)
    {
      dport_op = argv[6];
      dport = argv[7];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[6];
      deport = argv[7];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ip_tcpudp_srcport_dstport,
      access_list_ip_tcpudp_srcport_dstport_range_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ip_tcpudp_srcport_dstport,
      access_list_ip_tcpudp_srcport_dstport_ng_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ip_tcpudp_srcport_range_dstport,
      access_list_ip_tcpudp_srcport_range_dstport_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = "rn"; 
  char *sport = argv[5];
  char *seport = argv[6];
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 8)
  {
    dport_op = "eq";
    dport = argv[7];
  }
  else if(argc == 9)
  {
    if(port_operation_type(argv[7]) == OPT_NONE)
    {
      dport_op = argv[7];
      dport = argv[8];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[7];
      deport = argv[8];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ip_tcpudp_srcport_range_dstport,
      access_list_ip_tcpudp_srcport_range_dstport_range_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ip_tcpudp_srcport_range_dstport,
      access_list_ip_tcpudp_srcport_range_dstport_ng_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ip_tcpudp_srcport_ng_dstport,
      access_list_ip_tcpudp_srcport_ng_dstport_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = argv[5]; 
  char *sport = argv[6];
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 8)
  {
    dport_op = "eq";
    dport = argv[7];
  }
  else if(argc == 9)
  {
    if(port_operation_type(argv[7]) == OPT_NONE)
    {
      dport_op = argv[7];
      dport = argv[8];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[7];
      deport = argv[8];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ip_tcpudp_srcport_ng_dstport,
      access_list_ip_tcpudp_srcport_ng_dstport_range_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ip_tcpudp_srcport_ng_dstport,
      access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd,
      "ip access-list (WORD|<4000-4999>) (deny|permit) (A.B.C.D/M|any) (A.B.C.D/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IP_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 10.0.0.0/8\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 

#ifdef ZPL_BUILD_IPV6
/////////////
DEFUN(access_list_ipv6_protocol,
      access_list_ipv6_protocol_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD IP_IPPROTO_CMD")",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      IP_IPPROTO_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

DEFUN(no_access_list_ipv6_protocol,
      no_access_list_ipv6_protocol_cmd,
      "no ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD IP_IPPROTO_CMD")",
      NO_STR
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      IP_IPPROTO_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 0);
}

DEFUN(no_access_list_ipv6_tcpudp,
      no_access_list_ipv6_tcpudp_cmd,
      "no ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD")",
      NO_STR
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    sport_op = "eq";
    sport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      sport_op = argv[5];
      sport = argv[6];
    }
    else
    {  
      sport_op = "eq";
      sport = argv[5];
      seport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 0);
}

ALIAS(no_access_list_ipv6_tcpudp,
      no_access_list_ipv6_tcpudp_name_cmd,
      "no ipv6 access-list (WORD|<4000-4999>)",
      NO_STR
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n")

DEFUN(access_list_ipv6_tcpudp,
      access_list_ipv6_tcpudp_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD")",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP)
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = NULL;
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    sport_op = "eq";
    sport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      sport_op = argv[5];
      sport = argv[6];
    }
    else
    {  
      sport_op = "rn";
      sport = argv[5];
      seport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

ALIAS(access_list_ipv6_tcpudp,
      access_list_ipv6_tcpudp_srcport_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n")
 
ALIAS(access_list_ipv6_tcpudp,
      access_list_ipv6_tcpudp_srcport_range_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n")    

ALIAS(access_list_ipv6_tcpudp,
      access_list_ipv6_tcpudp_srcport_ng_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n")  


DEFUN(access_list_ipv6_tcpudp_dstport,
      access_list_ipv6_tcpudp_dstport_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port any dest-port eq <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = "any";
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 6)
  {
    dport_op = "eq";
    dport = argv[5];
  }
  else if(argc == 7)
  {
    if(port_operation_type(argv[5]) == OPT_NONE)
    {
      dport_op = argv[5];
      dport = argv[6];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[5];
      deport = argv[6];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ipv6_tcpudp_dstport,
      access_list_ipv6_tcpudp_dstport_range_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port any dest-port range <1-65536> <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ipv6_tcpudp_dstport,
      access_list_ipv6_tcpudp_dstport_ng_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port any dest-port (ne|gt|lt|ge|le) <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Any TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ipv6_tcpudp_srcport_dstport,
      access_list_ipv6_tcpudp_srcport_dstport_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port eq <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = NULL; 
  char *sport = argv[5];
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 7)
  {
    dport_op = "eq";
    dport = argv[6];
  }
  else if(argc == 8)
  {
    if(port_operation_type(argv[6]) == OPT_NONE)
    {
      dport_op = argv[6];
      dport = argv[7];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[6];
      deport = argv[7];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ipv6_tcpudp_srcport_dstport,
      access_list_ipv6_tcpudp_srcport_dstport_range_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port range <1-65536> <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ipv6_tcpudp_srcport_dstport,
      access_list_ipv6_tcpudp_srcport_dstport_ng_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port eq <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ipv6_tcpudp_srcport_range_dstport,
      access_list_ipv6_tcpudp_srcport_range_dstport_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port eq <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = "rn"; 
  char *sport = argv[5];
  char *seport = argv[6];
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 8)
  {
    dport_op = "eq";
    dport = argv[7];
  }
  else if(argc == 9)
  {
    if(port_operation_type(argv[7]) == OPT_NONE)
    {
      dport_op = argv[7];
      dport = argv[8];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[7];
      deport = argv[8];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ipv6_tcpudp_srcport_range_dstport,
      access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port range <1-65536> <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ipv6_tcpudp_srcport_range_dstport,
      access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port range <1-65536> <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source Range TCP/UDP Port\n"
      "Specify Source TCP/UDP Start Port Vlaue\n"
      "Specify Source TCP/UDP End Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 


DEFUN(access_list_ipv6_tcpudp_srcport_ng_dstport,
      access_list_ipv6_tcpudp_srcport_ng_dstport_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port eq <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "Specify Destination TCP/UDP Port Vlaue\n"
      "Any TCP/UDP Port Vlaue\n")
{
  char *prot_str = argv[4];
  char *sprefix = argv[2];
  char *sport_op = argv[5]; 
  char *sport = argv[6];
  char *seport = NULL;
  char *dprefix = argv[3];
  char *dport_op = NULL;
  char *dport = NULL;
  char *deport = NULL;
  if(argc == 8)
  {
    dport_op = "eq";
    dport = argv[7];
  }
  else if(argc == 9)
  {
    if(port_operation_type(argv[7]) == OPT_NONE)
    {
      dport_op = argv[7];
      dport = argv[8];
    }
    else
    {  
      dport_op = "rn";
      dport = argv[7];
      deport = argv[8];
    }
  }
  return filter_set_zebos_extended(vty, argv[0], argv[1], AFI_IP6, prot_str,
                                   sprefix, sport_op, sport, seport,
                                   dprefix, dport_op, dport, deport, 1);
}

 
ALIAS(access_list_ipv6_tcpudp_srcport_ng_dstport,
      access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port range <1-65536> <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination Range TCP/UDP Port\n"
      "Specify Destination TCP/UDP Start Port Vlaue\n"
      "Specify Destination TCP/UDP End Port Vlaue\n") 

ALIAS(access_list_ipv6_tcpudp_srcport_ng_dstport,
      access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd,
      "ipv6 access-list (WORD|<4000-4999>) (deny|permit) (X:X::X:X/M|any) (X:X::X:X/M|any) ("IP_TCPUDP_CMD") src-port (ne|gt|lt|ge|le) <1-65536> dest-port (ne|gt|lt|ge|le) <1-65536>",
      IPV6_STR
      "Add an access list entry\n"
      "IP extended access-list name\n"
      "IP extended access list\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Source Any\n"
      "Specify Destination Prefix to match. e.g. 3ffe:506::/32\n"
      "Specify Destination Any\n"
      IP_TCPUDP_HELP
      "Specify Source TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Source TCP/UDP Port Vlaue\n"
      "Specify Destination TCP/UDP Port\n"
      "ne\n"
      "gt\n"
      "lt\n"
      "ge\n"
      "le\n"
      "Specify Destination TCP/UDP Port Vlaue\n") 
#endif
#endif

#ifdef ZPL_FILTER_MAC
int filter_l2mac_format(struct vty *vty, 
                            char *src, char *srcmask, 
                            char *dst, char *dstmask,
                            int proto,
                            int vlan, int cos, int innervlan, int innercos,
                            int label, int exp, int innerlabel, int innerexp,
                            struct filter_l2 *l2new)
{
  memset(l2new, 0, sizeof(struct filter_l2));

  if(src && strcmp(src, "any") == 0)
  {
    memset(l2new->srcmac_mask, 0, NSM_MAC_MAX);
    memset(l2new->srcmac, 0, NSM_MAC_MAX);
    l2new->flag |= FILTER_L2MAC_SRC;
  }
  else if(src && srcmask && strcmp(srcmask, "host") == 0)
  {
    vty_mac_get(src, l2new->srcmac);
    if (NSM_MAC_IS_BROADCAST(l2new->srcmac))
    {
      vty_out(vty, "Error: This host mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->srcmac))
    {
      vty_out(vty, "Error: This host mac Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    memset(l2new->srcmac_mask, 0xff, NSM_MAC_MAX);
    l2new->flag |= FILTER_L2MAC_SRC;
  }
  else if(src && srcmask)
  {
    vty_mac_get(src, l2new->srcmac);
    vty_mac_get(srcmask, l2new->srcmac_mask);
    if (NSM_MAC_IS_BROADCAST(l2new->srcmac))
    {
      vty_out(vty, "Error: This src mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->srcmac))
    {
      vty_out(vty, "Error: This src mac is Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_BROADCAST(l2new->srcmac_mask))
    {
      vty_out(vty, "Error: This src mask mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->srcmac_mask))
    {
      vty_out(vty, "Error: This src mask mac is Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    l2new->flag |= FILTER_L2MAC_SRC|FILTER_L2MAC_SRCMASK;
  }

  if(dst &&  strcmp(dst, "any") == 0)
  {
    memset(l2new->dstmac_mask, 0, NSM_MAC_MAX);
    memset(l2new->dstmac, 0, NSM_MAC_MAX);
    l2new->flag |= FILTER_L2MAC_DST;
  }
  else if(dst && dstmask && strcmp(dstmask, "host") == 0)
  {
    memset(l2new->srcmac_mask, 0xff, NSM_MAC_MAX);
    vty_mac_get(dst, l2new->dstmac);
    if (NSM_MAC_IS_BROADCAST(l2new->dstmac))
    {
      vty_out(vty, "Error: This dst mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->dstmac))
    {
      vty_out(vty, "Error: This dst mac is Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    l2new->flag |= FILTER_L2MAC_DST;
  }
  else if(dst && dstmask)
  {
    vty_mac_get(dst, l2new->dstmac);
    vty_mac_get(dstmask, l2new->dstmac_mask);
    if (NSM_MAC_IS_BROADCAST(l2new->dstmac))
    {
      vty_out(vty, "Error: This dst mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->dstmac))
    {
      vty_out(vty, "Error: This dst mac is Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_BROADCAST(l2new->dstmac_mask))
    {
      vty_out(vty, "Error: This dst mask mac is Broadcast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    if (NSM_MAC_IS_MULTICAST(l2new->dstmac_mask))
    {
      vty_out(vty, "Error: This dst mask mac is Multicast mac address.%s", VTY_NEWLINE);
      return ERROR;
    }
    l2new->flag |= FILTER_L2MAC_DST|FILTER_L2MAC_DSTMASK;
  }

  if(proto >= 0)
  {
    l2new->flag |= FILTER_L2MAC_PROTO;
    l2new->l2protocol = proto;
  }
  if(vlan >= 0)
  {
    l2new->flag |= FILTER_L2MAC_VLAN;
    l2new->vlanid = vlan;
  }
  if(cos >= 0)
  {
    l2new->flag |= FILTER_L2MAC_COS;
    l2new->cos = cos + 1;
  }
  if(innervlan >= 0)
  {
    l2new->flag |= FILTER_L2MAC_INNERVID;
    l2new->inner_vlanid = innervlan;
  }
  if(innercos >= 0)
  {
    l2new->flag |= FILTER_L2MAC_INNERCOS;
    l2new->inner_cos = innercos + 1;
  }

  if(label >= 0)
  {
    l2new->flag |= FILTER_L2MAC_LABEL;
    l2new->label = label;
  }
  if(exp >= 0)
  {
    l2new->flag |= FILTER_L2MAC_EXP;
    l2new->exp = exp + 1;
  }
  if(innerlabel >= 0)
  {
    l2new->flag |= FILTER_L2MAC_INNERLABEL;
    l2new->inner_label = innerlabel;
  }
  if(innerexp >= 0)
  {
    l2new->flag |= FILTER_L2MAC_INNEREXP;
    l2new->inner_exp = innerexp + 1;
  }
  return OK;
}            

static int filter_set_l2mac(struct vty *vty, const char *name_str,
                            const char *type_str, u_char mpls, struct filter_l2 *l2new,
                            u_char set)
{
  int ret;
  enum filter_type type;
  struct filter_list *mfilter;
  struct filter_l2 *filter;
  struct access_list *access;

  /* Check of filter type. */
  if (strncmp(type_str, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp(type_str, "d", 1) == 0)
    type = FILTER_DENY;
  else
  {
    vty_out(vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  mfilter = filter_new();
  mfilter->type = type;
  mfilter->nodetype = FILTER_MAC;
  filter = &mfilter->u.mac_filter;
  memcpy(filter, l2new, sizeof(struct filter_l2));

  /* Install/Get new filter to the access_list. */
  if (set)
    access = access_list_get(AFI_ETHER, name_str);
  else
  {
    access = access_list_lookup(AFI_ETHER, name_str);
    if (!access)
    {
      vty_out(vty, "Can't delete access-list. Access-list '%s' doesn't exist.%s", name_str, VTY_NEWLINE);
      return CMD_WARNING;
    }
  }

  if (set)
  {
    if (filter_lookup_mac_local(access, mfilter))
      filter_free(mfilter);
    else
      access_list_filter_add(access, mfilter);
  }
  else
  {
    struct filter_list *delete_filter;

    delete_filter = filter_lookup_mac_local(access, mfilter);
    if (delete_filter && delete_filter->nodetype != FILTER_ZEBOS_EXT)
    {
      filter_free(mfilter);
      return CMD_WARNING;
    }

    if (delete_filter && !delete_filter->next && !delete_filter->prev && access->ref_cnt > 1)
    {
      vty_out(vty, "Can't delete access-list. Access-list '%s' being used.%s", access->name, VTY_NEWLINE);
      return CMD_WARNING;
    }

    if (delete_filter)
      access_list_filter_delete(access, delete_filter);

    filter_free(mfilter);
  }

  return CMD_SUCCESS;
}

/*

mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac MAC MASK
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host MAC

mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac MAC MASK
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac host MAC

mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan xxx

mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan xxx cos xx

mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx
mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan xxx cos xx inner-vlan xxx inner-cos xx protocol xxxx
*/
DEFUN(mac_access_list_src_mac_any,
      mac_access_list_src_mac_any_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = "any";
  char *srcmask = NULL; 
  char *dst = NULL;
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);
  if(ret != OK)
    return CMD_WARNING;                        
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}
/*
DEFUN(mac_access_list_src_mac_mask,
      mac_access_list_src_mac_mask_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

DEFUN(mac_access_list_src_mac_host,
      mac_access_list_src_mac_host_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}
*/
////
DEFUN(mac_access_list_src_mac_any_dstany,
      mac_access_list_src_mac_any_dstany_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = "any";
  char *srcmask = NULL; 
  char *dst = "any";
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 3)
  {
    if(strstr(argv[2], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[2]);
  }
  else if(argc == 4)
  {
    if(strstr(argv[2], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[2]);

    if(strstr(argv[3], "any"))
      cos = 0;
    else  
      cos = atoi(argv[3]);
  }
  else if(argc == 5)
  {
    if(strstr(argv[2], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[2]);

    if(strstr(argv[3], "any"))
      cos = 0;
    else  
      cos = atoi(argv[3]);

    proto = eth_protocol_type(argv[4]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);    
  if(ret != OK)
    return CMD_WARNING;                     
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_any_dstany,
      mac_access_list_src_mac_any_dstany_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_any_dstany,
      mac_access_list_src_mac_any_dstany_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_any_dstany,
      mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac any vlan (<1-4094>|any) "
      " cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_any_dstmask,
      mac_access_list_src_mac_any_dstmask_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = "any";
  char *srcmask = NULL; 
  char *dst = argv[2];
  char *dstmask = argv[3];
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 5)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
    proto = eth_protocol_type(argv[6]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);  
  if(ret != OK)
    return CMD_WARNING;      
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_any_dstmask,
      mac_access_list_src_mac_any_dstmask_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_any_dstmask,
      mac_access_list_src_mac_any_dstmask_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_any_dstmask,
      mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_any_dsthost,
      mac_access_list_src_mac_any_dsthost_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac host " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = "any";
  char *srcmask = NULL; 
  char *dst = argv[2];
  char *dstmask = "host";
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 4)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);
  }
  else if(argc == 5)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
    proto = eth_protocol_type(argv[5]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);  
  if(ret != OK)
    return CMD_WARNING;                
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_any_dsthost,
      mac_access_list_src_mac_any_dsthost_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_any_dsthost,
      mac_access_list_src_mac_any_dsthost_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac host " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_any_dsthost,
      mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac any dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Any Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_mask,
      mac_access_list_src_mac_mask_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = argv[3]; 
  char *dst = NULL;
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 4)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);
  }
  else if(argc == 5)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
    proto = eth_protocol_type(argv[5]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);  
  if(ret != OK)
    return CMD_WARNING;                 
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

////
DEFUN(mac_access_list_src_mac_mask_dstany,
      mac_access_list_src_mac_mask_dstany_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = argv[3]; 
  char *dst = "any";
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 5)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
    proto = eth_protocol_type(argv[6]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);   
  if(ret != OK)
    return CMD_WARNING; 
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_mask_dstany,
      mac_access_list_src_mac_mask_dstany_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_mask_dstany,
      mac_access_list_src_mac_mask_dstany_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_mask_dstany,
      mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac any vlan (<1-4094>|any) cos (<0-7>|any)"
      " protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_mask_dstmask,
      mac_access_list_src_mac_mask_dstmask_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = argv[3]; 
  char *dst = argv[4];
  char *dstmask = argv[5];
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 7)
  {
    if(strstr(argv[6], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[6]);
  }
  else if(argc == 8)
  {
    if(strstr(argv[6], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[6]);

    if(strstr(argv[7], "any"))
      cos = 0;
    else  
      cos = atoi(argv[7]);
  }
  else if(argc == 9)
  {
    if(strstr(argv[6], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[6]);

    if(strstr(argv[7], "any"))
      cos = 0;
    else  
      cos = atoi(argv[7]);
    proto = eth_protocol_type(argv[8]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new);  
  if(ret != OK)
    return CMD_WARNING;          
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_mask_dstmask,
      mac_access_list_src_mac_mask_dstmask_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_mask_dstmask,
      mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_mask_dstmask,
      mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Any Vlan ID\n"
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any  Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_mask_dsthost,
      mac_access_list_src_mac_mask_dsthost_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = argv[3]; 
  char *dst = argv[4];
  char *dstmask = "host";
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 6)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
  }
  else if(argc == 8)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
    proto = eth_protocol_type(argv[7]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new); 
  if(ret != OK)
    return CMD_WARNING;          
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_mask_dsthost,
      mac_access_list_src_mac_mask_dsthost_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_mask_dsthost,
      mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_mask_dsthost,
      mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac " CMD_MAC_STR " mask " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_host,
      mac_access_list_src_mac_host_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = "host"; 
  char *dst = NULL;
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  /*if(argc == 6)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
  }
  else if(argc == 8)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
    proto = eth_protocol_type(argv[7]);
  }*/
  int ret = 0;
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new); 
  if(ret != OK)
    return CMD_WARNING;            
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

DEFUN(mac_access_list_src_mac_host_dstany,
      mac_access_list_src_mac_host_dstany_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = "host"; 
  char *dst = "any";
  char *dstmask = NULL;
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 4)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);
  }
  else if(argc == 5)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[3], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[3]);

    if(strstr(argv[4], "any"))
      cos = 0;
    else  
      cos = atoi(argv[4]);
    proto = eth_protocol_type(argv[5]);
  }

  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new); 
  if(ret != OK)
    return CMD_WARNING;     
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_host_dstany,
      mac_access_list_src_mac_host_dstany_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_host_dstany,
      mac_access_list_src_mac_host_dstany_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_host_dstany,
      mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac any"
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Any Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_host_dstmask,
      mac_access_list_src_mac_host_dstmask_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = "host"; 
  char *dst = argv[3];
  char *dstmask = argv[4];
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 6)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
  }
  else if(argc == 8)
  {
    if(strstr(argv[5], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[5]);

    if(strstr(argv[6], "any"))
      cos = 0;
    else  
      cos = atoi(argv[6]);
    proto = eth_protocol_type(argv[7]);
  }
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new); 
  if(ret != OK)
    return CMD_WARNING; 
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_host_dstmask,
      mac_access_list_src_mac_host_dstmask_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_host_dstmask,
      mac_access_list_src_mac_host_dstmask_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_host_dstmask,
      mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac " CMD_MAC_STR " mask " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Mac Address\n"
      "Specify Mac Address Mask\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

DEFUN(mac_access_list_src_mac_host_dsthost,
      mac_access_list_src_mac_host_dsthost_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR "",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n")
{
  struct filter_l2 l2new;
  char *src = argv[2];
  char *srcmask = "host"; 
  char *dst = argv[3];
  char *dstmask = "host";
  int proto = -1;
  int vlan = -1;
  int cos = -1;
  int innervlan = -1; 
  int innercos = -1;
  int label = -1;
  int exp = -1;
  int innerlabel = -1;
  int innerexp = -1;
  int ret = 0;
  if(argc == 5)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);
  }
  else if(argc == 6)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
  }
  else if(argc == 7)
  {
    if(strstr(argv[4], "any"))
      vlan = 0;
    else  
      vlan = atoi(argv[4]);

    if(strstr(argv[5], "any"))
      cos = 0;
    else  
      cos = atoi(argv[5]);
    proto = eth_protocol_type(argv[6]);
  }
    
  ret = filter_l2mac_format(vty, src, srcmask, 
                          dst, dstmask,
                          proto,
                          vlan, cos, innervlan, innercos,
                          label, exp, innerlabel, innerexp,
                          &l2new); 
  if(ret != OK)
    return CMD_WARNING;   
  return filter_set_l2mac(vty, argv[0], argv[1], 0, &l2new, 1);
}

ALIAS(mac_access_list_src_mac_host_dsthost,
      mac_access_list_src_mac_host_dsthost_vlan_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR " vlan <1-4094>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP)

ALIAS(mac_access_list_src_mac_host_dsthost,
      mac_access_list_src_mac_host_dsthost_vlan_cos_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos <0-7>",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n")

ALIAS(mac_access_list_src_mac_host_dsthost,
      mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd,
      "mac-access-list  (WORD|<1-2147483646>) (deny|permit) src-mac host " CMD_MAC_STR " dest-mac host " CMD_MAC_STR
      " vlan (<1-4094>|any) cos (<0-7>|any) protocol (ip|arp|rarp|snmp|ppp|gsmp|mpls|lldp|eap|NUMBER)",
      "Add an Mac access list entry\n"
      "MAC access-list\n"
      "MAC access-list name\n"
      "Specify packets to reject\n"
      "Specify packets to forward\n"
      "Specify Source Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n"
      "Specify Destination Mac Address\n"
      "Specify Host Mac Address\n"
      "Specify Mac Address\n" CMD_VLAN_STR_HELP
      "Specify Cos Priority Information\n"
      "Specify Cos Priority Value\n"
      "Any Cos Priority Value\n"
      "Specify L2 Protocol Information\n"
      "IP Protocol\n"
      "ARP Protocol\n"
      "RARP Protocol\n"
      "SNMP Protocol\n"
      "PPP Protocol\n"
      "GSMP Protocol\n"
      "MPLS Protocol\n"
      "LLDP Protocol\n"
      "EAP Protocol\n"
      "Protocol Hex Value\n")

#endif /* ZPL_FILTER_MAC */

static void config_write_access_zebra(struct vty *, struct filter_list *);
static void config_write_access_cisco(struct vty *, struct filter_list *);

#ifdef ZPL_FILTER_ZEBRA_EXT
static void config_write_access_zebos_ext(struct vty *, struct filter_list *);
#endif
#ifdef ZPL_FILTER_MAC
static void config_write_access_maclist(struct vty *, struct filter_list *);
#endif
static const char *get_filter_type_str(struct filter_list *mfilter)
{
  switch (mfilter->nodetype)
  {
  case FILTER_COMMON:
  {
    struct filter_cisco *filter;

    filter = &mfilter->u.cfilter;
    if (filter->extended)
      return "Extended";
    else
      return "Standard";
  }
  break;
  case FILTER_ZEBOS:
    return "ZebOS";
#ifdef ZPL_FILTER_ZEBRA_EXT
  case FILTER_ZEBOS_EXT:
    return "ZebOS extended";
#endif
#ifdef ZPL_FILTER_MAC
  case FILTER_MAC:
    return "ZebOS mac";
#endif
  }
  return "";
}

/* show access-list command. */
static int
filter_show(struct vty *vty, const char *name, afi_t afi)
{
  struct access_list *access;
  struct access_master *master;
  struct filter_list *mfilter;
  struct filter_cisco *filter;
  int write = 0;

  master = access_master_get(afi);
  if (master == NULL)
    return 0;

  /* Print the name of the protocol */
  if (zlog_default)
    vty_out(vty, "%s:%s",
            zlog_proto_names(zlog_default->protocol), VTY_NEWLINE);

  for (access = master->num.head; access; access = access->next)
  {
    if (name && strcmp(access->name, name) != 0)
      continue;

    write = 1;

    for (mfilter = access->head; mfilter; mfilter = mfilter->next)
    {
      filter = &mfilter->u.cfilter;

      if (write)
      {
        vty_out(vty, "%s IP%s access list %s%s",
                get_filter_type_str(mfilter),
                afi == AFI_IP6 ? "v6" : "",
                access->name, VTY_NEWLINE);
        write = 0;
      }

      vty_out(vty, "    %s%s", filter_type_str(mfilter->type),
              mfilter->type == FILTER_DENY ? "  " : "");
      if (mfilter->nodetype == FILTER_ZEBOS)
        config_write_access_zebra(vty, mfilter);
      else if (filter->extended)
        config_write_access_cisco(vty, mfilter);
#ifdef ZPL_FILTER_ZEBRA_EXT
      if (mfilter->nodetype == FILTER_ZEBOS_EXT)
        config_write_access_zebos_ext(vty, mfilter);
#endif
#ifdef ZPL_FILTER_MAC
      else if (mfilter->nodetype == FILTER_MAC)
        config_write_access_maclist(vty, mfilter);
#endif
      else
      {
        if (filter->addr_mask.s_addr == 0xffffffff)
          vty_out(vty, " any%s", VTY_NEWLINE);
        else
        {
          vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr));
          if (filter->addr_mask.s_addr != 0)
            vty_out(vty, ", wildcard bits %s", ipstack_inet_ntoa(filter->addr_mask));
          vty_out(vty, "%s", VTY_NEWLINE);
        }
      }
    }
  }

  for (access = master->str.head; access; access = access->next)
  {
    if (name && strcmp(access->name, name) != 0)
      continue;

    write = 1;

    for (mfilter = access->head; mfilter; mfilter = mfilter->next)
    {
      filter = &mfilter->u.cfilter;

      if (write)
      {
        vty_out(vty, "%s IP%s access list %s%s",
                get_filter_type_str(mfilter),
                afi == AFI_IP6 ? "v6" : "",
                access->name, VTY_NEWLINE);
        write = 0;
      }

      vty_out(vty, "    %s%s", filter_type_str(mfilter->type),
              mfilter->type == FILTER_DENY ? "  " : "");
      if (mfilter->nodetype == FILTER_ZEBOS)
        config_write_access_zebra(vty, mfilter);
      else if (filter->extended)
        config_write_access_cisco(vty, mfilter);
#ifdef ZPL_FILTER_ZEBRA_EXT
      else if (mfilter->nodetype == FILTER_ZEBOS_EXT)
        config_write_access_zebos_ext(vty, mfilter);
#endif
#ifdef ZPL_FILTER_MAC
      else if (mfilter->nodetype == FILTER_MAC)
        config_write_access_maclist(vty, mfilter);
#endif
      else
      {
        if (filter->addr_mask.s_addr == 0xffffffff)
          vty_out(vty, " any%s", VTY_NEWLINE);
        else
        {
          vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr));
          if (filter->addr_mask.s_addr != 0)
            vty_out(vty, ", wildcard bits %s", ipstack_inet_ntoa(filter->addr_mask));
          vty_out(vty, "%s", VTY_NEWLINE);
        }
      }
    }
  }
  return CMD_SUCCESS;
}

DEFUN(show_ip_access_list,
      show_ip_access_list_cmd,
      "show ip access-list",
      SHOW_STR
          IP_STR
      "List IP access lists\n")
{
  return filter_show(vty, NULL, AFI_IP);
}

DEFUN(show_ip_access_list_name,
      show_ip_access_list_name_cmd,
      "show ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD)",
      SHOW_STR
          IP_STR
      "List IP access lists\n"
      "IP standard access list\n"
      "IP extended access list\n"
      "IP standard access list (expanded range)\n"
      "IP extended access list (expanded range)\n"
      "IP zebra access-list\n")
{
  return filter_show(vty, argv[0], AFI_IP);
}

#ifdef ZPL_BUILD_IPV6
DEFUN(show_ipv6_access_list,
      show_ipv6_access_list_cmd,
      "show ipv6 access-list",
      SHOW_STR
          IPV6_STR
      "List IPv6 access lists\n")
{
  return filter_show(vty, NULL, AFI_IP6);
}

DEFUN(show_ipv6_access_list_name,
      show_ipv6_access_list_name_cmd,
      "show ipv6 access-list WORD",
      SHOW_STR
          IPV6_STR
      "List IPv6 access lists\n"
      "IPv6 zebra access-list\n")
{
  return filter_show(vty, argv[0], AFI_IP6);
}
#endif /* ZPL_BUILD_IPV6 */

#ifdef ZPL_FILTER_MAC
DEFUN(show_mac_access_list,
      show_mac_access_list_cmd,
      "show mac-access-list",
      SHOW_STR
      "List Mac access lists\n")
{
  return filter_show(vty, NULL, AFI_ETHER);
}

DEFUN(show_mac_access_list_name,
      show_mac_access_list_name_cmd,
      "show mac-access-list WORD",
      SHOW_STR
      "List Mac access lists\n"
      "Mac access-list\n")
{
  return filter_show(vty, argv[0], AFI_ETHER);
}
#endif /* ZPL_FILTER_MAC */

void access_list_write_config_cisco(struct vty *vty, struct filter_cisco *filter)
{
  if (filter->extended)
  {
    vty_out(vty, " ip");
    if (filter->addr_mask.s_addr == 0xffffffff)
      vty_out(vty, " any");
    else if (filter->addr_mask.s_addr == 0)
      vty_out(vty, " host %s", ipstack_inet_ntoa(filter->addr));
    else
    {
      vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr));
      vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr_mask));
    }

    if (filter->mask_mask.s_addr == 0xffffffff)
      vty_out(vty, " any");
    else if (filter->mask_mask.s_addr == 0)
      vty_out(vty, " host %s", ipstack_inet_ntoa(filter->mask));
    else
    {
      vty_out(vty, " %s", ipstack_inet_ntoa(filter->mask));
      vty_out(vty, " %s", ipstack_inet_ntoa(filter->mask_mask));
    }
    vty_out(vty, "%s", VTY_NEWLINE);
  }
  else
  {
    if (filter->addr_mask.s_addr == 0xffffffff)
      vty_out(vty, " any%s", VTY_NEWLINE);
    else
    {
      vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr));
      if (filter->addr_mask.s_addr != 0)
        vty_out(vty, " %s", ipstack_inet_ntoa(filter->addr_mask));
      vty_out(vty, "%s", VTY_NEWLINE);
    }
  }
}

static void config_write_access_cisco(struct vty *vty, struct filter_list *mfilter)
{
  struct filter_cisco *filter;
  filter = &mfilter->u.cfilter;
  access_list_write_config_cisco(vty, filter);
}


void access_list_write_config_zebra(struct vty *vty, struct filter_zebra *filter)
{
  struct prefix *p;
  zpl_char buf[BUFSIZ];
  p = &filter->prefix;

  if (p->prefixlen == 0 && !filter->exact)
    vty_out(vty, " any");
  else
    vty_out(vty, " %s/%d%s",
            ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
            p->prefixlen,
            filter->exact ? " exact-match" : "");

  vty_out(vty, "%s", VTY_NEWLINE);
}

static void config_write_access_zebra(struct vty *vty, struct filter_list *mfilter)
{
  struct filter_zebra *filter;
  struct prefix *p;
  zpl_char buf[BUFSIZ];

  filter = &mfilter->u.zfilter;
  p = &filter->prefix;

  if (p->prefixlen == 0 && !filter->exact)
    vty_out(vty, " any");
  else
    vty_out(vty, " %s/%d%s",
            ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
            p->prefixlen,
            filter->exact ? " exact-match" : "");

  vty_out(vty, "%s", VTY_NEWLINE);
}

#ifdef ZPL_FILTER_ZEBRA_EXT
void access_list_write_config_zebos_ext(struct vty *vty, struct filter_zebos_ext *filter)
{
  struct prefix *p;
  zpl_char buf[BUFSIZ];
  p = &filter->sprefix;

  if (p->prefixlen == 0)
    vty_out(vty, " any");
  else
    vty_out(vty, " %s/%d", ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
            p->prefixlen);

  p = &filter->dprefix;
  if (p->prefixlen == 0)
    vty_out(vty, " any");
  else
    vty_out(vty, " %s/%d", ipstack_inet_ntop(p->family, &p->u.prefix, buf, BUFSIZ),
            p->prefixlen);
  if(filter->protocol != IP_IPPROTO_MAX)          
  vty_out(vty, " %s", ip_protocol_type_string(filter->protocol));

  if (filter->sport && filter->sport_op != OPT_NONE && filter->sport_op != OPT_RN)
    vty_out(vty, " src-port %s %d", port_operation_type_string(filter->sport_op), filter->sport);
  if (filter->dport_op == OPT_RN && filter->sport)
    vty_out(vty, " src-port %s %d %d", port_operation_type_string(filter->sport_op), filter->sport >> 16, filter->sport & 0xffff);


  if (filter->dport && filter->dport_op != OPT_NONE && filter->dport_op != OPT_RN)
    vty_out(vty, " dest-port %s %d", port_operation_type_string(filter->dport_op), filter->dport);
  if (filter->dport && filter->dport_op == OPT_RN)
    vty_out(vty, " dest-port %s %d %d", port_operation_type_string(filter->dport_op), filter->dport >> 16, filter->dport & 0xffff);

  vty_out(vty, "%s", VTY_NEWLINE);
}

static void config_write_access_zebos_ext(struct vty *vty, struct filter_list *mfilter)
{
  struct filter_zebos_ext *filter;
  filter = &mfilter->u.zextfilter;
  access_list_write_config_zebos_ext(vty, filter);
}
#endif

#ifdef ZPL_FILTER_MAC
void access_list_write_config_mac(struct vty *vty, struct filter_l2 *filter)
{
  mac_t mac0byte[NSM_MAC_MAX] = {0, 0, 0, 0, 0, 0};
  mac_t macfbyte[NSM_MAC_MAX] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  if (cli_ethernet_cmp(filter->srcmac, mac0byte) != 0 && (filter->flag&FILTER_L2MAC_SRC) && cli_ethernet_cmp(filter->srcmac_mask, macfbyte) == 0)
    vty_out(vty, " src-mac host %s", cli_inet_ethernet(filter->srcmac));
  else if (cli_ethernet_cmp(filter->srcmac, mac0byte) == 0 && (filter->flag&FILTER_L2MAC_SRC))
    vty_out(vty, " src-mac any");
  else if((filter->flag&FILTER_L2MAC_SRC) && (filter->flag&FILTER_L2MAC_SRCMASK))
    vty_out(vty, " src-mac %s mask %s", cli_inet_ethernet(filter->srcmac), cli_inet_ethernet(filter->srcmac_mask));

  if (cli_ethernet_cmp(filter->dstmac, mac0byte) != 0 && (filter->flag&FILTER_L2MAC_DST) && cli_ethernet_cmp(filter->dstmac_mask, macfbyte) == 0)
    vty_out(vty, " dest-mac host %s", cli_inet_ethernet(filter->dstmac));
  else if (cli_ethernet_cmp(filter->dstmac, mac0byte) == 0 && (filter->flag&FILTER_L2MAC_DST))
    vty_out(vty, " dest-mac any");
  else if((filter->flag&FILTER_L2MAC_DST) && (filter->flag&FILTER_L2MAC_DSTMASK))
    vty_out(vty, " dest-mac %s mask %s", cli_inet_ethernet(filter->dstmac), cli_inet_ethernet(filter->dstmac_mask));

  if (filter->vlanid > 0)
    vty_out(vty, " vlan %d", filter->vlanid);
 if (filter->vlanid == 0 && (filter->flag&FILTER_L2MAC_VLAN))
    vty_out(vty, " vlan any");
  if (filter->cos > 0)
    vty_out(vty, " cos %d", filter->cos-1);
  if (filter->cos == 0 && (filter->flag&FILTER_L2MAC_COS))
    vty_out(vty, " cos any");
  if (filter->inner_vlanid > 0)
    vty_out(vty, " inner-vlan %d", filter->inner_vlanid);
  if (filter->inner_vlanid == 0 && (filter->flag&FILTER_L2MAC_INNERVID))
    vty_out(vty, " inner-vlan any");
  if (filter->inner_cos > 0)
    vty_out(vty, " inner-cos %d", filter->inner_cos-1);
  if (filter->inner_cos == 0 && (filter->flag&FILTER_L2MAC_INNERCOS))
    vty_out(vty, " inner-cos any");
  if (filter->l2protocol > 0)
    vty_out(vty, " protocol %s", eth_protocol_type_string(filter->l2protocol));
  if (filter->l2protocol == 0 && (filter->flag&FILTER_L2MAC_PROTO))
    vty_out(vty, " protocol any");

  if (filter->label > 0)
    vty_out(vty, " label %d", filter->label);
  if (filter->label == 0 && (filter->flag&FILTER_L2MAC_LABEL))
    vty_out(vty, " label any");
  if (filter->exp > 0)
    vty_out(vty, " exp %d", filter->exp-1);
  if (filter->exp == 0 && (filter->flag&FILTER_L2MAC_EXP))
    vty_out(vty, " exp any");
  if (filter->inner_label > 0)
    vty_out(vty, " inner-label %d", filter->inner_label);
  if (filter->inner_label == 0 && (filter->flag&FILTER_L2MAC_INNERLABEL))
    vty_out(vty, " inner-label any");
  if (filter->inner_cos > 0)
    vty_out(vty, " inner-exp %d", filter->inner_exp-1);
  if (filter->inner_cos == 0 && (filter->flag&FILTER_L2MAC_INNEREXP))
    vty_out(vty, " inner-exp any");
  vty_out(vty, "%s", VTY_NEWLINE);
}
static void config_write_access_maclist(struct vty *vty, struct filter_list *mfilter)
{
  struct filter_l2 *filter;
  filter = &mfilter->u.mac_filter;
  access_list_write_config_mac(vty, filter);
}

#endif
static int
config_write_access(struct vty *vty, afi_t afi)
{
  struct access_list *access;
  struct access_master *master;
  struct filter_list *mfilter;
  int write = 0;

  master = access_master_get(afi);
  if (master == NULL)
    return 0;

  for (access = master->num.head; access; access = access->next)
  {
    if (access->remark)
    {
      vty_out(vty, "%saccess-list %s remark %s%s",
              afi == AFI_IP ? "ip " : "ipv6 ",
              access->name, access->remark,
              VTY_NEWLINE);
      write++;
    }

    for (mfilter = access->head; mfilter; mfilter = mfilter->next)
    {
      if (mfilter->nodetype == FILTER_COMMON || mfilter->nodetype == FILTER_ZEBOS)
        vty_out(vty, "%saccess-list %s %s",
                afi == AFI_IP ? "ip " : "ipv6 ",
                access->name,
                filter_type_str(mfilter->type));

      if (mfilter->nodetype == FILTER_COMMON)
        config_write_access_cisco(vty, mfilter);
      else if (mfilter->nodetype == FILTER_ZEBOS)
        config_write_access_zebra(vty, mfilter);
#ifdef ZPL_FILTER_ZEBRA_EXT
      else if (mfilter->nodetype == FILTER_ZEBOS_EXT)
      {
        vty_out(vty, "%saccess-list %s %s",
                afi == AFI_IP ? "ip " : "ipv6 ",
                access->name,
                filter_type_str(mfilter->type));
        config_write_access_zebos_ext(vty, mfilter);
      }
#endif
#ifdef ZPL_FILTER_MAC
      else if (mfilter->nodetype == FILTER_MAC)
      {
        vty_out(vty, "mac-access-list  %s %s",
                access->name,
                filter_type_str(mfilter->type));
        config_write_access_maclist(vty, mfilter);
      }
#endif
      write++;
    }
  }

  for (access = master->str.head; access; access = access->next)
  {
    if (access->remark)
    {
      vty_out(vty, "%saccess-list %s remark %s%s",
              afi == AFI_IP ? "ip " : "ipv6 ",
              access->name, access->remark,
              VTY_NEWLINE);
      write++;
    }

    for (mfilter = access->head; mfilter; mfilter = mfilter->next)
    {
      if ((mfilter->nodetype == FILTER_COMMON) || (mfilter->nodetype == FILTER_ZEBOS))
        vty_out(vty, "%saccess-list %s %s",
                afi == AFI_IP ? "ip " : "ipv6 ",
                access->name,
                filter_type_str(mfilter->type));

      if (mfilter->nodetype == FILTER_COMMON)
        config_write_access_cisco(vty, mfilter);
      else if (mfilter->nodetype == FILTER_ZEBOS)
        config_write_access_zebra(vty, mfilter);
#ifdef ZPL_FILTER_ZEBRA_EXT
      else if (mfilter->nodetype == FILTER_ZEBOS_EXT)
      {
        vty_out(vty, "%saccess-list %s %s",
                afi == AFI_IP ? "ip " : "ipv6 ",
                access->name,
                filter_type_str(mfilter->type));
        config_write_access_zebos_ext(vty, mfilter);
      }
#endif
#ifdef ZPL_FILTER_MAC
      else if (mfilter->nodetype == FILTER_MAC)
      {
        vty_out(vty, "mac-access-list  %s %s",
                access->name,
                filter_type_str(mfilter->type));
        config_write_access_maclist(vty, mfilter);
      }
#endif
      write++;
    }
  }
  return write;
}

/* Access-list node. */
static struct cmd_node access_node =
    {
        ACCESS_NODE,
        "", /* Access list has no interface. */
        1};

static int
config_write_access_ipv4(struct vty *vty)
{
  return config_write_access(vty, AFI_IP);
}

static void
access_list_reset_ipv4(void)
{
  struct access_list *access;
  struct access_list *next;
  struct access_master *master;

  master = access_master_get(AFI_IP);
  if (master == NULL)
    return;

  for (access = master->num.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }
  for (access = master->str.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }

  assert(master->num.head == NULL);
  assert(master->num.tail == NULL);

  assert(master->str.head == NULL);
  assert(master->str.tail == NULL);
}

/* Install vty related command. */
static void
access_list_init_ipv4(void)
{
  install_node(&access_node, config_write_access_ipv4);

  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_access_list_cmd);
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ip_access_list_name_cmd);

  /* Zebra access-list */
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_exact_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_exact_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_any_cmd);

  /* Standard access-list */
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_standard_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_standard_nomask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_standard_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_standard_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_standard_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_standard_nomask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_standard_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_standard_any_cmd);

  /* Extended access-list */
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_any_mask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_mask_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_any_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_host_mask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_mask_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_host_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_any_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_extended_host_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_any_mask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_mask_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_any_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_host_mask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_mask_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_host_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_any_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_extended_host_any_cmd);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_remark_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_all_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_remark_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_remark_arg_cmd);

#ifdef ZPL_FILTER_ZEBRA_EXT
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ip_tcpudp_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ip_tcpudp_name_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_protocol_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ip_protocol_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_range_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_range_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_range_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_ng_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_ng_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ip_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}

#ifdef ZPL_BUILD_IPV6
static struct cmd_node access_ipv6_node =
    {
        ACCESS_IPV6_NODE,
        "",
        1};

static int
config_write_access_ipv6(struct vty *vty)
{
  return config_write_access(vty, AFI_IP6);
}

static void
access_list_reset_ipv6(void)
{
  struct access_list *access;
  struct access_list *next;
  struct access_master *master;

  master = access_master_get(AFI_IP6);
  if (master == NULL)
    return;

  for (access = master->num.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }
  for (access = master->str.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }

  assert(master->num.head == NULL);
  assert(master->num.tail == NULL);

  assert(master->str.head == NULL);
  assert(master->str.tail == NULL);
}

static void
access_list_init_ipv6(void)
{
  install_node(&access_ipv6_node, config_write_access_ipv6);

  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipv6_access_list_cmd);
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_ipv6_access_list_name_cmd);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ipv6_access_list_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ipv6_access_list_exact_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ipv6_access_list_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_exact_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_any_cmd);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_all_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &ipv6_access_list_remark_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_remark_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_ipv6_access_list_remark_arg_cmd);

#ifdef ZPL_FILTER_ZEBRA_EXT
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ipv6_tcpudp_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ipv6_tcpudp_name_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_protocol_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_access_list_ipv6_protocol_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_range_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_range_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_range_dstport_ng_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_ng_dstport_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_ng_dstport_range_cmd);
      install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &access_list_ipv6_tcpudp_srcport_ng_dstport_ng_cmd);
#endif
}
#endif /* ZPL_BUILD_IPV6 */
#endif

#ifdef ZPL_FILTER_MAC
/* Access-list node. */
static struct cmd_node access_mac_node =
    {
        ACCESS_MAC_NODE,
        "", /* Access list has no interface. */
        1};

static int
config_write_access_mac(struct vty *vty)
{
  return config_write_access(vty, AFI_ETHER);
}

static void
access_list_reset_mac(void)
{
  struct access_list *access;
  struct access_list *next;
  struct access_master *master;

  master = access_master_get(AFI_ETHER);
  if (master == NULL)
    return;

  for (access = master->num.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }
  for (access = master->str.head; access; access = next)
  {
    next = access->next;
    access_list_delete(access);
  }

  assert(master->num.head == NULL);
  assert(master->num.tail == NULL);

  assert(master->str.head == NULL);
  assert(master->str.tail == NULL);
}

/* Install vty related command. */
static void
access_list_init_mac(void)
{
  install_node(&access_mac_node, config_write_access_mac);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstany_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstany_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstany_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstany_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstmask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstmask_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstmask_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dstmask_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dsthost_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dsthost_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dsthost_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_any_dsthost_vlan_cos_type_cmd);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstany_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstany_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstany_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstany_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstmask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstmask_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstmask_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dstmask_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dsthost_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dsthost_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dsthost_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_mask_dsthost_vlan_cos_type_cmd);

  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstany_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstany_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstany_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstany_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstmask_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstmask_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstmask_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dstmask_vlan_cos_type_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dsthost_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dsthost_vlan_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dsthost_vlan_cos_cmd);
  install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &mac_access_list_src_mac_host_dsthost_vlan_cos_type_cmd);

  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_mac_access_list_cmd);
  install_element(ENABLE_NODE, CMD_VIEW_LEVEL, &show_mac_access_list_name_cmd);
}
#endif

void access_list_init()
{
#ifdef ZPL_SHELL_MODULE
  access_list_init_ipv4();
#ifdef ZPL_BUILD_IPV6
  access_list_init_ipv6();
#endif /* ZPL_BUILD_IPV6 */
#ifdef ZPL_FILTER_MAC
  access_list_init_mac();
#endif

#endif
}

void access_list_reset()
{
#ifdef ZPL_SHELL_MODULE
  access_list_reset_ipv4();
#ifdef ZPL_BUILD_IPV6
  access_list_reset_ipv6();
#endif /* ZPL_BUILD_IPV6 */
#ifdef ZPL_FILTER_MAC
  access_list_reset_mac();
#endif
#endif
}

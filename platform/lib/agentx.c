/* SNMP support
 * Copyright (C) 2012 Vincent Bernat <bernat@luffy.cx>
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
#include "prefix.h"
#include "command.h"
#include "filter.h"
#include "zmemory.h"
#include "sockunion.h"
#include "buffer.h"
#include "log.h"
#include "if.h"

#if defined ZPL_NSM_SNMP && defined SNMP_AGENTX
#include "agentx.h"

static int agentx_enabled = 0;

static struct thread_master *agentx_tm;
static struct thread *timeout_thr = NULL;
static struct list *events = NULL;

static void agentx_events_update(void);

static int
agentx_timeout(struct thread *t)
{
  timeout_thr = NULL;

  snmp_timeout ();
  run_alarms ();
  netsnmp_check_outstanding_agent_requests ();
  agentx_events_update ();
  return 0;
}

static int
agentx_read(struct thread *t)
{
  fd_set fds;
  struct listnode *ln = THREAD_ARG (t);
  list_delete_node (events, ln);

  FD_ZERO (&fds);
  FD_SET (ipstack_fd(THREAD_FD (t)), &fds);
  snmp_read (&fds);

  netsnmp_check_outstanding_agent_requests ();
  agentx_events_update ();
  return 0;
}

static void
agentx_events_update(void)
{
  int maxfd = 0;
  int block = 1;
  struct timeval timeout = { .tv_sec = 0, .tv_usec = 0 };
  fd_set fds;
  struct listnode *ln;
  struct thread *thr;
  int fd, thr_fd;
  zpl_socket_t rfd;
  rfd = ipstack_init(OS_STACK, -1);
  THREAD_OFF (timeout_thr);

  FD_ZERO (&fds);
  snmp_select_info (&maxfd, &fds, &timeout, &block);

  if (!block)
    timeout_thr = thread_add_timer_tv (agentx_tm, agentx_timeout, NULL, &timeout);

  ln = listhead (events);
  thr = ln ? listgetdata (ln) : NULL;
  thr_fd = thr ? ipstack_fd(THREAD_FD (thr)) : -1;

  /* "two-pointer" / two-list simultaneous iteration
   * ln/thr/thr_fd point to the next existing event listener to hit while
   * fd counts to catch up */
  for (fd = 0; fd < maxfd; fd++)
    {
      /* caught up */
      if (thr_fd == fd)
        {
          struct listnode *nextln = listnextnode (ln);
          if (!FD_ISSET (fd, &fds))
            {
              thread_cancel (thr);
              list_delete_node (events, ln);
            }
          ln = nextln;
          thr = ln ? listgetdata (ln) : NULL;
          thr_fd = thr ? ipstack_fd(THREAD_FD (thr)) : -1;
        }
      /* need listener, but haven't hit one where it would be */
      else if (FD_ISSET (fd, &fds))
        {
          struct listnode *newln;
          ipstack_fd(rfd) = fd;
          thr = thread_add_read (agentx_tm, agentx_read, NULL, rfd);
          newln = listnode_add_before (events, ln, thr);
          thr->arg = newln;
        }
    }

  /* leftover event listeners at this point have fd > maxfd, delete them */
  while (ln)
    {
      struct listnode *nextln = listnextnode (ln);
      thread_cancel (listgetdata (ln));
      list_delete_node (events, ln);
      ln = nextln;
    }
}

/* AgentX node. */
static struct cmd_node agentx_node =
{
  SMUX_NODE,
  ""                            /* AgentX has no interface. */
};

/* Logging NetSNMP messages */
static int
agentx_log_callback(int major, int minor,
		    void *serverarg, void *clientarg)
{
  char *msg = NULL;
  struct snmp_log_message *slm = (struct snmp_log_message *)serverarg;
  if(!agentx_enabled)
    return SNMP_ERR_NOERROR;
  if(slm && slm->msg)
    msg = strdup (slm->msg);
  if (msg) 
    msg[strlen(msg)-1] = '\0';
  switch (slm->priority)
    {
    case LOG_EMERG:   zlog_err   (MODULE_LIB, "snmp[emerg]: %s",   msg?msg:slm->msg); break;
    case LOG_ALERT:   zlog_err   (MODULE_LIB, "snmp[alert]: %s",   msg?msg:slm->msg); break;
    case LOG_CRIT:    zlog_err   (MODULE_LIB, "snmp[crit]: %s",    msg?msg:slm->msg); break;
    case LOG_ERR:     zlog_err   (MODULE_LIB, "snmp[err]: %s",     msg?msg:slm->msg); break;
    case LOG_WARNING: zlog_warn  (MODULE_LIB, "snmp[warning]: %s", msg?msg:slm->msg); break;
    case LOG_NOTICE:  zlog_notice(MODULE_LIB, "snmp[notice]: %s",  msg?msg:slm->msg); break;
    case LOG_INFO:    zlog_info  (MODULE_LIB, "snmp[info]: %s",    msg?msg:slm->msg); break;
    case LOG_DEBUG:   zlog_debug (MODULE_LIB, "snmp[debug]: %s",   msg?msg:slm->msg); break;
    }
  free(msg);
  return SNMP_ERR_NOERROR;
}

static int
config_write_agentx (struct vty *vty)
{
  if (agentx_enabled)
      vty_out (vty, "agentx%s", VTY_NEWLINE);
  return 0;
}

DEFUN (agentx_enable,
       agentx_enable_cmd,
       "agentx",
       "SNMP AgentX protocol settings\n"
       "SNMP AgentX settings\n")
{
  if (!agentx_enabled)
    {
      init_snmp("quagga");
      events = list_new();
      agentx_events_update ();
      agentx_enabled = 1;
      return CMD_SUCCESS;
    }
  vty_out (vty, "SNMP AgentX already enabled%s", VTY_NEWLINE);
  return CMD_WARNING;
}

DEFUN (no_agentx,
       no_agentx_cmd,
       "no agentx",
       NO_STR
       "SNMP AgentX protocol settings\n"
       "SNMP AgentX settings\n")
{
  if (!agentx_enabled) return CMD_SUCCESS;
  vty_out (vty, "SNMP AgentX support cannot be disabled once enabled%s", VTY_NEWLINE);
  return CMD_WARNING;
}

void
snmp_agentx_init (struct thread_master *tm)
{
  agentx_tm = tm;

  netsnmp_enable_subagent ();
  snmp_disable_log ();
  snmp_enable_calllog ();
  snmp_register_callback (SNMP_CALLBACK_LIBRARY,
			  SNMP_CALLBACK_LOGGING,
			  agentx_log_callback,
			  NULL);
  init_agent ("quagga");

  install_node (&agentx_node, config_write_agentx);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &agentx_enable_cmd);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_agentx_cmd);
}

void
snmp_register_mib (const char *descr, struct variable *var, 
		   size_t width, int num,
		   oid name[], size_t namelen)
{
  register_mib (descr, var, width, num, name, namelen);
}

int
snmp_trap (struct variable *vp, size_t vp_len,
	   const oid *ename, size_t enamelen,
	   const oid *name, size_t namelen,
	   const oid *iname, size_t inamelen,
	   const struct trap_object *trapobj, size_t trapobjlen,
	   u_char sptrap)
{
  oid objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
  size_t objid_snmptrap_len = sizeof objid_snmptrap / sizeof (oid);
  oid notification_oid[MAX_OID_LEN];
  size_t notification_oid_len;
  unsigned int i;

  netsnmp_variable_list *notification_vars = NULL;
  if (!agentx_enabled) return 0;

  /* snmpTrapOID */
  oid_copy (notification_oid, ename, enamelen);
  notification_oid[enamelen] = sptrap;
  notification_oid_len = enamelen + 1;
  snmp_varlist_add_variable (&notification_vars,
			     objid_snmptrap, objid_snmptrap_len,
			     ASN_OBJECT_ID,
			     (u_char *) notification_oid,
			     notification_oid_len * sizeof(oid));

  /* Provided bindings */
  for (i = 0; i < trapobjlen; i++)
    {
      unsigned int j;
      oid oid[MAX_OID_LEN];
      size_t oid_len, onamelen;
      u_char *val;
      size_t val_len;
      WriteMethod *wm = NULL;
      struct variable cvp;

      /* Make OID. */
      if (trapobj[i].namelen > 0)
        {
	  /* Columnar object */
	  onamelen = trapobj[i].namelen;
	  oid_copy (oid, name, namelen);
	  oid_copy (oid + namelen, trapobj[i].name, onamelen);
	  oid_copy (oid + namelen + onamelen, iname, inamelen);
	  oid_len = namelen + onamelen + inamelen;
        }
      else
        {
	  /* Scalar object */
	  onamelen = trapobj[i].namelen * (-1);
	  oid_copy (oid, name, namelen);
	  oid_copy (oid + namelen, trapobj[i].name, onamelen);
	  oid[onamelen + namelen] = 0;
	  oid_len = namelen + onamelen + 1;
        }

      /* Locate the appropriate function and type in the MIB registry. */
      for (j = 0; j < vp_len; j++)
	{
	  if (oid_compare (trapobj[i].name, onamelen, vp[j].name, vp[j].namelen) != 0)
	    continue;
	  /* We found the appropriate variable in the MIB registry. */
	  oid_copy(cvp.name, name, namelen);
	  oid_copy(cvp.name + namelen, vp[j].name, vp[j].namelen);
	  cvp.namelen = namelen + vp[j].namelen;
	  cvp.type = vp[j].type;
	  cvp.magic = vp[j].magic;
	  cvp.acl = vp[j].acl;
	  cvp.findVar = vp[j].findVar;
	  /* Grab the result. */
	  val = cvp.findVar (&cvp, oid, &oid_len, 1, &val_len, &wm);
	  if (!val) break;
	  snmp_varlist_add_variable (&notification_vars,
				     oid, oid_len,
				     vp[j].type,
				     val,
				     val_len);
	  break;
	}
    }


  send_v2trap (notification_vars);
  snmp_free_varbind (notification_vars);
  agentx_events_update ();
  return 1;
}






static int snmp_agentx_iterator_table_index(netsnmp_table_registration_info *table_info, int *index_type, int index_max)
{
  switch(index_max)
  {
    case 1:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], 0);    
    break;
    case 2:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], 0);    
    break;
    case 3:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], 0);    
    break;
    case 4:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 0);    
    break;
    case 5:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], index_type[4], 0);    
    break;
    case 6:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], 0);    
    break;
    case 7:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], 0);    
    break;
    case 8:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], index_type[7], 0);    
    break;
    case 9:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], index_type[7], index_type[8], 0);    
    break;
    case 10:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], index_type[7], index_type[8], index_type[9], 0);    
    break;
    case 11:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], index_type[7], index_type[8], index_type[9], index_type[10], 0);    
    break;
    case 12:
    netsnmp_table_helper_add_indexes(table_info, index_type[0], index_type[1], index_type[2], index_type[3], 
      index_type[4], index_type[5], index_type[6], index_type[7], index_type[8], index_type[9], index_type[10], index_type[11], 0);    
    break;
    default:
    break;
  }
  return OK;
}

static int snmp_agentx_iterator_register_table(const char *name, const oid *oidtbl, int oidlen, struct snmp_agentx_callback *agentxcb)
{
    netsnmp_handler_registration    *reg = NULL;
    netsnmp_iterator_info           *iinfo = NULL;
    netsnmp_table_registration_info *table_info = NULL;

    reg = netsnmp_create_handler_registration(name, agentxcb->snmp_handler,
              oidtbl, oidlen, agentxcb->wrflag);
    if(reg)
    {
      table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
      if(table_info)
      {
        snmp_agentx_iterator_table_index(table_info, agentxcb->index_type, agentxcb->index_max);

        table_info->min_column = agentxcb->min_column;
        table_info->max_column = agentxcb->max_column;
        
        iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
        if(iinfo)
        {
          iinfo->get_first_data_point = agentxcb->get_first_handler;
          iinfo->get_next_data_point  = agentxcb->get_next_handler;
          iinfo->table_reginfo        = table_info;
          return netsnmp_register_table_iterator( reg, iinfo );
        }
        else
        {
          SNMP_FREE(table_info);
          netsnmp_handler_registration_free(reg);
        }
      }
    }
    return OK;
}

static int snmp_agentx_scalar_register_table(const char *name, const oid *oidtbl, int oidlen, struct snmp_agentx_callback *agentxcb)
{
  netsnmp_handler_registration    *reg = NULL;
  reg = netsnmp_create_handler_registration(name, agentxcb->snmp_handler,
              oidtbl, oidlen, agentxcb->wrflag);
  if(reg)            
    return netsnmp_register_scalar(reg);
  return ERROR;  
}


int snmp_agentx_register_mib(const char *name, const oid *oidtbl, int oidlen, struct snmp_agentx_callback *agentxcb)
{
  if(agentxcb->get_first_handler)
  {
    return snmp_agentx_iterator_register_table(name, oidtbl,  oidlen, agentxcb);
  }
  else
  {
    return snmp_agentx_scalar_register_table(name, oidtbl, oidlen, agentxcb);
  }
}

#endif /* ZPL_NSM_SNMP */

/*
mib2c -c mib2c.iterate.conf ipForward
mib2c -c mib2c.scalar.conf ipForward
net-snmp-config --compile-subagent ipForward ipForward.c
*/
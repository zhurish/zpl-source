/* Zebra's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "os_ipstack.h"
#include "module.h"
#include "zmemory.h"
#include "linklist.h"
#include "thread.h"
#include "log.h"
#include "if.h"
#include "lib_event.h"

struct lib_event_list
{
  zpl_uint32 module;
  lib_event_cb  callback;
};

struct lib_event
{
    struct list *lib_event_tbl;
    zpl_socket_t r_evfd;
    zpl_socket_t w_evfd;
    void *t_master;
    void *t_read;
};

static struct lib_event _lib_event;
static int lib_event_base_create(void);

static struct lib_event_list * lib_event_node_new (void)
{
  struct lib_event_list *client = NULL;
  client = XCALLOC (MTYPE_EVENT_INFO, sizeof (struct lib_event_list));
  return client;
}


static void lib_event_node_free (struct lib_event_list *client)
{
	if(client)
	{
		listnode_delete (_lib_event.lib_event_tbl, client);
		XFREE (MTYPE_EVENT_INFO, client);
	}
}

static void lib_event_install (struct lib_event_list *client, zpl_uint32 module)
{
	client->module = module;
	listnode_add_sort(_lib_event.lib_event_tbl, client);
}

static struct lib_event_list * lib_event_lookup (zpl_uint32 module)
{
	struct listnode *node = NULL;
	struct lib_event_list *client = NULL;
	for (ALL_LIST_ELEMENTS_RO(_lib_event.lib_event_tbl, node, client))
	{
		if(client->module == module)
			return client;
	}
	return NULL;
}

static int lib_event_foreach_handle (lib_event_e event, lib_event_data_t *data)
{
	struct listnode *node = NULL;
	struct lib_event_list *client = NULL;
	for (ALL_LIST_ELEMENTS_RO(_lib_event.lib_event_tbl, node, client))
	{
		if(client->callback && data->module != client->module)
			(client->callback)(event, data);
	}
	return OK;
}



static int lib_event_read(struct thread *arg)
{
    int nbytes = 0;
	lib_event_data_t data;
	zpl_socket_t fd = THREAD_FD(arg);
    _lib_event.t_read = NULL;
	os_bzero(&data, sizeof(lib_event_data_t));
	nbytes = ipstack_read(fd, &data, sizeof(lib_event_data_t));
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
                _lib_event.t_read = thread_add_read(_lib_event.t_master, lib_event_read, NULL, fd);
				return 0;
			}
			zlog_warn(MODULE_LIB,
					"%s: read error on event fd %d, closing: %s", __func__,
					ipstack_fd(fd), ipstack_strerror(ipstack_errno));
            ipstack_close(_lib_event.w_evfd); 
            ipstack_close(_lib_event.r_evfd);        
            lib_event_base_create();  
            return OK;      
		}
	}
    lib_event_foreach_handle (data.event, &data);
	_lib_event.t_read = thread_add_read(_lib_event.t_master, lib_event_read, NULL, fd);
	return 0;
}

static int lib_event_base_create(void)
{
    if(ipstack_unix_sockpair_create(IPSTACK_OS, zpl_false, &_lib_event.r_evfd, &_lib_event.w_evfd) == OK)
    {
        _lib_event.t_read = thread_add_read(_lib_event.t_master, lib_event_read, NULL, _lib_event.r_evfd);
        return OK;
    }
    return ERROR;
}


int lib_event_register_api (zpl_uint32 module, lib_event_cb cb)
{
    struct lib_event_list *client = NULL;
    if(lib_event_lookup ( module))
    {
        return ERROR;
    }
    client = lib_event_node_new ();
    if(client)
    {
        client->module = module;
        client->callback = cb;
        lib_event_install (client, module);
        return OK;
    }
    return ERROR;
}

int lib_event_init (void)
{
	_lib_event.lib_event_tbl = list_new();
    if(_lib_event.lib_event_tbl)
    {
        _lib_event.lib_event_tbl->cmp =  NULL;
        _lib_event.lib_event_tbl->del = lib_event_node_free;
        return OK;
    }
    return ERROR;
}

int lib_event_exit (void)
{
    lib_event_loop_stop ();
    if(_lib_event.lib_event_tbl)
	    list_delete(_lib_event.lib_event_tbl);
    return OK;
}

/*
void lib_event_loop (void *m)
{
    _lib_event.t_master = m;
    lib_event_base_create(); 
}
*/

int lib_event_loop_start (void *m)
{
    _lib_event.t_master = m;
    lib_event_base_create(); 
    return OK;
}

int lib_event_loop_stop (void)
{
	if (_lib_event.t_read)
    {
		thread_cancel(_lib_event.t_read);
        _lib_event.t_read = NULL;
    }
    if (!ipstack_invalid(_lib_event.w_evfd))
    {
        ipstack_close(_lib_event.w_evfd); 
    }
    if (!ipstack_invalid(_lib_event.r_evfd))
    {
        ipstack_close(_lib_event.r_evfd); 
    }
    return OK;
}


int lib_event_post_data_api (zpl_uint32 module, lib_event_e event, lib_event_data_t *data)
{
    data->module = module;
    data->event = event;
    if(!ipstack_invalid(_lib_event.w_evfd))
    {
        if(ipstack_write(_lib_event.w_evfd, data, sizeof(lib_event_data_t)) > 0)
            return OK;
    }
    return ERROR;
}

int lib_event_post_api (lib_event_data_t *data)
{
    if(!ipstack_invalid(_lib_event.w_evfd))
    {
        if(ipstack_write(_lib_event.w_evfd, data, sizeof(lib_event_data_t)) > 0)
            return OK;
    }
    return ERROR;
}





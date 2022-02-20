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

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

struct nsm_event_list
{
  zpl_uint32 module;
  nsm_event_cb  callback;
};

struct nsm_event
{
    struct list *nsm_event_tbl;
    zpl_socket_t r_evfd;
    zpl_socket_t w_evfd;
    void *t_master;
    void *t_read;
};

static struct nsm_event _nsm_event;
static int nsm_event_base_create(void);

static struct nsm_event_list * nsm_event_node_new (void)
{
  struct nsm_event_list *client = NULL;
  client = XCALLOC (MTYPE_EVENT_INFO, sizeof (struct nsm_event_list));
  return client;
}


static void nsm_event_node_free (struct nsm_event_list *client)
{
	if(client)
	{
		listnode_delete (_nsm_event.nsm_event_tbl, client);
		XFREE (MTYPE_EVENT_INFO, client);
	}
}

static void nsm_event_install (struct nsm_event_list *client, zpl_uint32 module)
{
	client->module = module;
	listnode_add_sort(_nsm_event.nsm_event_tbl, client);
}

static struct nsm_event_list * nsm_event_lookup (zpl_uint32 module)
{
	struct listnode *node = NULL;
	struct nsm_event_list *client = NULL;
	for (ALL_LIST_ELEMENTS_RO(_nsm_event.nsm_event_tbl, node, client))
	{
		if(client->module == module)
			return client;
	}
	return NULL;
}

static int nsm_event_foreach_handle (nsm_event_e event, nsm_event_data_t *data)
{
	struct listnode *node = NULL;
	struct nsm_event_list *client = NULL;
	for (ALL_LIST_ELEMENTS_RO(_nsm_event.nsm_event_tbl, node, client))
	{
		if(client->callback && data->module != client->module)
			(client->callback)(event, data);
	}
	return OK;
}



static int nsm_event_read(struct thread *arg)
{
    int nbytes = 0;
	nsm_event_data_t data;
	zpl_socket_t fd = THREAD_FD(arg);
    _nsm_event.t_read = NULL;
	os_bzero(&data, sizeof(nsm_event_data_t));
	nbytes = ipstack_read(fd, &data, sizeof(nsm_event_data_t));
	if (nbytes <= 0)
	{
		if (nbytes < 0)
		{
			if (IPSTACK_ERRNO_RETRY(ipstack_errno))
			{
                _nsm_event.t_read = thread_add_read(_nsm_event.t_master, nsm_event_read, NULL, fd);
				return 0;
			}
			zlog_warn(MODULE_NSM,
					"%s: read error on event fd %d, closing: %s", __func__,
					fd._fd, ipstack_strerror(ipstack_errno));
            ipstack_close(_nsm_event.w_evfd); 
            ipstack_close(_nsm_event.r_evfd);        
            nsm_event_base_create();  
            return OK;      
		}
	}
    nsm_event_foreach_handle (data.event, &data);
	_nsm_event.t_read = thread_add_read(_nsm_event.t_master, nsm_event_read, NULL, fd);
	return 0;
}

static int nsm_event_base_create(void)
{
    if(ipstack_unix_sockpair_create(OS_STACK, zpl_false, &_nsm_event.r_evfd, &_nsm_event.w_evfd) == OK)
    {
        _nsm_event.t_read = thread_add_read(_nsm_event.t_master, nsm_event_read, NULL, _nsm_event.r_evfd);
        return OK;
    }
    return ERROR;
}


int nsm_event_register_api (zpl_uint32 module, nsm_event_cb cb)
{
    struct nsm_event_list *client = NULL;
    if(nsm_event_lookup ( module))
    {
        return ERROR;
    }
    client = nsm_event_node_new ();
    if(client)
    {
        client->module = module;
        client->callback = cb;
        nsm_event_install (client, module);
        return OK;
    }
    return ERROR;
}

int nsm_event_init (void)
{
	_nsm_event.nsm_event_tbl = list_new();
    if(_nsm_event.nsm_event_tbl)
    {
        _nsm_event.nsm_event_tbl->cmp =  NULL;
        _nsm_event.nsm_event_tbl->del = nsm_event_node_free;
        return OK;
    }
    return ERROR;
}

int nsm_event_exit (void)
{
    nsm_event_loop_stop ();
    if(_nsm_event.nsm_event_tbl)
	    list_delete(_nsm_event.nsm_event_tbl);
    return OK;
}

/*
void nsm_event_loop (void *m)
{
    _nsm_event.t_master = m;
    nsm_event_base_create(); 
}
*/

int nsm_event_loop_start (void *m)
{
    _nsm_event.t_master = m;
    nsm_event_base_create(); 
    return OK;
}

int nsm_event_loop_stop (void)
{
	if (_nsm_event.t_read)
    {
		thread_cancel(_nsm_event.t_read);
        _nsm_event.t_read = NULL;
    }
    if (!ipstack_invalid(_nsm_event.w_evfd))
    {
        ipstack_close(_nsm_event.w_evfd); 
    }
    if (!ipstack_invalid(_nsm_event.r_evfd))
    {
        ipstack_close(_nsm_event.r_evfd); 
    }
    return OK;
}


int nsm_event_post_data_api (zpl_uint32 module, nsm_event_e event, nsm_event_data_t *data)
{
    data->module = module;
    data->event = event;
    if(!ipstack_invalid(_nsm_event.w_evfd))
    {
        if(ipstack_write(_nsm_event.w_evfd, data, sizeof(nsm_event_data_t)) > 0)
            return OK;
    }
    return ERROR;
}

int nsm_event_post_api (nsm_event_data_t *data)
{
    if(!ipstack_invalid(_nsm_event.w_evfd))
    {
        if(ipstack_write(_nsm_event.w_evfd, data, sizeof(nsm_event_data_t)) > 0)
            return OK;
    }
    return ERROR;
}





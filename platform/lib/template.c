/*
 * template.c
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#include "zebra.h"
#include "os_list.h"

#include "if.h"
#include "vty.h"
#include "sockunion.h"
#include "prefix.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "vrf.h"
#include "command.h"
#include "interface.h"
#include "zclient.h"
#include "module.h"
#include "nsm_client.h"
#include "template.h"

static struct list *template_list = NULL;



/* Allocate template structure. */
template_t * nsm_template_new (void)
{
  template_t *template;
  template = XCALLOC (MTYPE_ZCLIENT, sizeof (template_t));
  return template;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free template structure. */
void nsm_template_free (template_t *template)
{
	if(template)
	{
		listnode_delete (template_list, template);
		XFREE (MTYPE_ZCLIENT, template);
	}
}

/* Initialize zebra template.  Argument redist_default is unwanted
   redistribute route type. */
void nsm_template_install (template_t *template, int module)
{
	template->module = module;
	//listnode_add_sort(template_list, template);
	listnode_add (template_list, template);
}

template_t* nsm_template_lookup (int module)
{
	struct listnode *node = NULL;
	template_t *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(template_list, node, template))
	{
		if(template->module == module)
			return template;
	}
	return NULL;
}


template_t* nsm_template_lookup_name (char * name)
{
	struct listnode *node = NULL;
	template_t *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(template_list, node, template))
	{
		if(strcmp(template->name, name) == 0)
			return template;
	}
	return NULL;
}


void nsm_template_init (void)
{
	if(template_list == NULL)
		template_list = list_new();
	template_list->cmp =  NULL;//nsm_template_cmp;
}

void nsm_template_exit (void)
{
	if(template_list)
		list_delete(template_list);
}

int nsm_template_write_config (struct vty *vty)
{
	int ret = 0;
	struct listnode *node;
	template_t  *template;
	for (ALL_LIST_ELEMENTS_RO(template_list, node, template))
	{
		if(template->write_template)
		{
			ret = 0;
			ret = (template->write_template)(vty, template->pVoid);
			if(ret)
				vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	return ret;
}

int nsm_template_show_config (struct vty *vty, BOOL detail)
{
	int ret = 0;
	struct listnode *node;
	template_t  *template;
	for (ALL_LIST_ELEMENTS_RO(template_list, node, template))
	{
		if(template->show_template)
		{
			ret = 0;
			ret = (template->show_template)(vty, template->pVoid, detail);
			if(ret)
				vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	return ret;
}

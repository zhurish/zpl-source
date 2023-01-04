/*
 * template.c
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "linklist.h"
#include "zmemory.h"
#include "log.h"
#include "command.h"
#include "vty.h"
#include "template.h"

static struct list *template_list = NULL;
static struct list *service_list = NULL;
static struct list *all_config_list = NULL;

/* Allocate template structure. */
template_t * lib_template_new (zpl_bool service)
{
  template_t *template;
  template = XCALLOC (MTYPE_ZCLIENT, sizeof (template_t));
  if(template)
	  template->service = service;
  return template;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free template structure. */
void lib_template_free (template_t *template)
{
	if(template)
	{
		listnode_delete (template->service?service_list:template_list, template);
		XFREE (MTYPE_ZCLIENT, template);
	}
}

/* Initialize zebra template.  Argument redist_default is unwanted
   redistribute route type. */
void lib_template_install (template_t *template, zpl_uint32 module)
{
	template->module = module;
	//listnode_add_sort(template_list, template);
	listnode_add (template->service?service_list:template_list, template);
}

void lib_template_config_list_install (template_t *template, zpl_uint32 module)
{
	template->module = module;
	listnode_add (all_config_list, template);
}

template_t* lib_template_lookup (zpl_bool service, zpl_uint32 module)
{
	struct listnode *node = NULL;
	template_t *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service?service_list:template_list, node, template))
	{
		if(template->module == module)
			return template;
	}
	return NULL;
}


template_t* lib_template_lookup_name (zpl_bool service, zpl_char * name)
{
	struct listnode *node = NULL;
	template_t *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service?service_list:template_list, node, template))
	{
		if(strcmp(template->name, name) == 0)
			return template;
	}
	return NULL;
}


void lib_template_init (void)
{
	if(template_list == NULL)
		template_list = list_new();
	template_list->cmp =  NULL;//lib_template_cmp;

	if(service_list == NULL)
		service_list = list_new();
	service_list->cmp =  NULL;//lib_template_cmp;

	if(all_config_list == NULL)
		all_config_list = list_new();
	all_config_list->cmp =  NULL;//lib_template_cmp;

	cmd_lib_template_init();
}

void lib_template_exit (void)
{
	if(template_list)
		list_delete(template_list);
	if(service_list)
		list_delete(service_list);
	if(all_config_list)
		list_delete(all_config_list);
}

int lib_template_write_config (struct vty *vty)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
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

int lib_template_show_config (struct vty *vty, zpl_bool detail)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
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

int lib_template_service_write_config (struct vty *vty)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service_list, node, template))
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

int lib_template_service_show_config (struct vty *vty, zpl_bool detail)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service_list, node, template))
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


int lib_template_debug_show_config (struct vty *vty, zpl_bool detail)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service_list, node, template))
	{
		if(template->show_debug)
		{
			ret = 0;
			ret = (template->show_debug)(vty, template->pVoid, detail);
			if(ret)
				vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	return ret;
}

int lib_template_debug_write_config (struct vty *vty)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(service_list, node, template))
	{
		if(template->show_debug)
		{
			ret = 0;
			ret = (template->show_debug)(vty, template->pVoid, zpl_false);
			if(ret)
				vty_out(vty, "!%s", VTY_NEWLINE);
		}
	}
	return ret;
}

static int lib_template_all_config_write_config (struct vty *vty)
{
	int ret = 0;
	struct listnode *node = NULL;
	template_t  *template = NULL;
	for (ALL_LIST_ELEMENTS_RO(all_config_list, node, template))
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

static struct cmd_node template_node =
{
	TEMPLATE_NODE,
	"%s(config-%s)# ",
	1
};
static struct cmd_node all_service_node =
{
	ALL_SERVICE_NODE,
	"%s(config-%s)# ",
	1
};
static struct cmd_node all_config_node =
{
	ALL_CONFIG_NODE,
	"%s(config)# ",
	1
};


int cmd_lib_template_init(void)
{
	install_node(&all_service_node, lib_template_service_write_config);
	install_node(&template_node, lib_template_write_config);
	install_node(&all_config_node, lib_template_all_config_write_config);
	install_default(TEMPLATE_NODE);
	install_default_basic(TEMPLATE_NODE);
	install_default(ALL_SERVICE_NODE);
	install_default_basic(ALL_SERVICE_NODE);
	install_default(ALL_CONFIG_NODE);
	install_default_basic(ALL_CONFIG_NODE);
	return OK;
}	
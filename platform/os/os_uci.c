/*
 * os_uci.c
 *
 *  Created on: Jan 29, 2019
 *      Author: zhurish
 */

/*
#include <string.h>
#include <malloc.h>
*/
#include "zebra.h"

#include "os_uci.h"

#ifdef PL_OPENWRT_UCI
#include "uci.h"

static os_uci_t os_uci_ctx;

static int os_uci_do_set_cmd(char *argv)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, argv, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	ret = uci_set(os_uci_ctx.context, &ptr);
	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(os_uci_ctx.context, ptr.p);
	return (ret == UCI_OK) ? OK:ERROR;
}

int os_uci_set_string(char *name, char *value)
{
	int ret = 0;
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),"%s=%s", name, value);
	os_uci_ctx.context = uci_alloc_context();
	ret = os_uci_do_set_cmd(cmd);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

int os_uci_set_integer(char *name, int value)
{
	int ret = 0;
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),"%s=%d", name, value);
	os_uci_ctx.context = uci_alloc_context();
	ret = os_uci_do_set_cmd(cmd);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

static int os_uci_do_add_section(char *name, char * section)
{
	struct uci_package *p = NULL;
	struct uci_section *s = NULL;
	int ret;

	ret = uci_load(os_uci_ctx.context, name, &p);
	if (ret != UCI_OK)
		goto done;

	ret = uci_add_section(os_uci_ctx.context, p, section, &s);
	if (ret != UCI_OK)
		goto done;

	ret = uci_save(os_uci_ctx.context, p);

done:
	if (ret != UCI_OK)
		;//cli_perror();
	if (s)
		fprintf(stdout, "%s\n", s->e.name);

	return ret;
}

int os_uci_section_add(char *name, char * section)
{
	int ret = 0;
	//char cmd[128];
	//memset(cmd, 0, sizeof(cmd));
	//snprintf(cmd, sizeof(cmd),"%s=%d", name, value);
	os_uci_ctx.context = uci_alloc_context();
	ret = os_uci_do_add_section(name, section);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

int os_uci_del(char *name, char * section, char * option)
{
	int ret = UCI_OK;
	struct uci_element *e;
	struct uci_ptr ptr;
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	if(section && option)
		snprintf(cmd, sizeof(cmd),"%s.%s.%s", name, section, option);
	else if(section)
		snprintf(cmd, sizeof(cmd),"%s.%s", name, section);
	else //if(section)
		snprintf(cmd, sizeof(cmd),"%s", name);

	os_uci_ctx.context = uci_alloc_context();
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, cmd, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	ret = uci_delete(os_uci_ctx.context, &ptr);
	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(os_uci_ctx.context, ptr.p);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

int os_uci_list_add(char *name, char * value)
{
	int ret = UCI_OK;
	struct uci_element *e;
	struct uci_ptr ptr;
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),"%s=%d", name, value);
	os_uci_ctx.context = uci_alloc_context();
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, cmd, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	ret = uci_add_list(os_uci_ctx.context, &ptr);
	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(os_uci_ctx.context, ptr.p);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

int os_uci_list_del(char *name, char * value)
{
	int ret = UCI_OK;
	struct uci_element *e;
	struct uci_ptr ptr;
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),"%s=%d", name, value);
	os_uci_ctx.context = uci_alloc_context();
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, cmd, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	ret = uci_del_list(os_uci_ctx.context, &ptr);
	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(os_uci_ctx.context, ptr.p);
	uci_free_context(os_uci_ctx.context);
	return ret;
}


int os_uci_commit(char *name)
{
	int ret = UCI_OK;
	struct uci_element *e;
	struct uci_ptr ptr;
	os_uci_ctx.context = uci_alloc_context();
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, name, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	ret = uci_commit(os_uci_ctx.context, &ptr.p, false);
	if (ptr.p)
		uci_unload(os_uci_ctx.context, ptr.p);
	uci_free_context(os_uci_ctx.context);
	return ret;
}


static int os_uci_do_get_cmd(char *argv, char *value)
{
	struct uci_element *e;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, argv, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		//os_uci_ctx.context->err = UCI_ERR_NOTFOUND;
		//cli_perror();
		return 1;
	}
	switch(e->type) {
	case UCI_TYPE_SECTION:
		strcpy(value, ptr.s->type);
		printf("%s:%s\r\n",__func__, ptr.s->type);
		break;
	case UCI_TYPE_OPTION:
		switch(ptr.o->type)
		{
		case UCI_TYPE_STRING:
			strcpy(value, ptr.o->v.string);
			printf("%s:%s\r\n",__func__, ptr.o->v.string);
			//printf("\n");
			break;
/*		case UCI_TYPE_LIST:
			uci_foreach_element(&o->v.list, e)
			{
				//printf("%s", (sep ? delimiter : ""));
				space = strpbrk(e->name, " \t\r\n");
				if (!space)
					printf("%s", e->name);
				else
					uci_print_value(stdout, e->name);
				sep = true;
			}
			printf("\n");
			break;*/
		default:
			printf("<unknown>\n");
			break;
		}
		//uci_show_value(ptr.o, false);
		break;
	default:
		break;
	}
	return (ret == UCI_OK) ? OK:ERROR;
}

int os_uci_get_string(char *name, char *value)
{
	int ret = 0;
	//char cmd[128];
	//memset(cmd, 0, sizeof(cmd));
	//snprintf(cmd, sizeof(cmd),"%s=%s", name, value);
	os_uci_ctx.context = uci_alloc_context();
	ret = os_uci_do_get_cmd(name, value);
	//ret = os_uci_do_set_cmd(cmd);
	uci_free_context(os_uci_ctx.context);
	return ret;
}

static int os_uci_do_get_list_cmd(char *argv, char **value, int *cnt)
{
	int i = 0;
	struct uci_element *e;
	struct uci_ptr ptr;
	bool sep = false;
	char *space;
	int ret = UCI_OK;
	if (uci_lookup_ptr(os_uci_ctx.context, &ptr, argv, true) != UCI_OK) {
		return ERROR;
	}
	e = ptr.last;
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		//ctx->err = UCI_ERR_NOTFOUND;
		//cli_perror();
		return 1;
	}
	switch(e->type) {
	case UCI_TYPE_OPTION:
		switch(ptr.o->type)
		{
		case UCI_TYPE_LIST:
			uci_foreach_element(&ptr.o->v.list, e)
			{
				//printf("%s", (sep ? delimiter : ""));
				space = strpbrk(e->name, " \t\r\n");
				if (!space)
					value[i++] = strdup(e->name);
					//printf("%s", e->name);
				else
					value[i++] = strdup(e->name);
					//uci_print_value(stdout, e->name);
				sep = true;

				printf("%s:%s\r\n",__func__, e->name);
			}
			printf("\n");
			break;
		default:
			printf("<unknown>\n");
			break;
		}
		//uci_show_value(ptr.o, false);
		break;
	default:
		break;
	}
	if(cnt)
		*cnt = i;
	return (ret == UCI_OK) ? OK:ERROR;
}

int os_uci_get_list(char *name, char **value, int *cnt)
{
	int ret = 0;
	//char cmd[128];
	//memset(cmd, 0, sizeof(cmd));
	//snprintf(cmd, sizeof(cmd),"%s=%s", name, value);
	os_uci_ctx.context = uci_alloc_context();
	ret = os_uci_do_get_list_cmd(name, value, cnt);
	//ret = os_uci_do_set_cmd(cmd);
	uci_free_context(os_uci_ctx.context);
	return ret;
}
#endif

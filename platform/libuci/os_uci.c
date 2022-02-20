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
#include "os_include.h"
#include "zpl_include.h"

#include "os_uci.h"

#ifdef ZPL_OPENWRT_UCI

#ifdef ZPL_OPENWRT_UCI_LIB

#include "uci.h"

#define UCI_MAX_ARGS	4 /* max command line arguments for batch mode */
#define UCI_RES_MAX_SIZE	64
static const char *uci_delimiter = " ";
static const char *uci_appname = NULL;
static enum {
	CLI_FLAG_MERGE =    (1 << 0),
	CLI_FLAG_QUIET =    (1 << 1),
	CLI_FLAG_NOCOMMIT = (1 << 2),
	CLI_FLAG_BATCH =    (1 << 3),
	CLI_FLAG_SHOW_EXT = (1 << 4),
} uci_flags;

static FILE *uci_input = NULL;
static int _uci_errno = 0;
static struct uci_context *uci_ctx = NULL;
enum {
	/* section cmds */
	CMD_GET,
	CMD_SET,
	CMD_ADD_LIST,
	CMD_DEL_LIST,
	CMD_DEL,
	CMD_RENAME,
	CMD_REVERT,
	CMD_REORDER,
	/* package cmds */
	CMD_SHOW,
	CMD_CHANGES,
	CMD_EXPORT,
	CMD_COMMIT,
	/* other cmds */
	CMD_ADD,
	CMD_IMPORT,
	CMD_HELP,
};

struct uci_type_list {
	unsigned int idx;
	const char *name;
	struct uci_type_list *next;
};

static struct uci_type_list *type_list = NULL;
static char *typestr = NULL;
static const char *cur_section_ref = NULL;

static int uci_cmd(int uci_argc, char **uci_argv);

static char *uci_result = NULL;
#define LIST_MAX_CNT	8
static char *uci_list_result[LIST_MAX_CNT];
static unsigned int uci_list_rescnt = 0;

static int uci_result_add(zpl_uint32 type, char *input)
{
#if 0
	if(uci_result)
	{
		//char *p = uci_result;
		int len = strlen(uci_result);
		if(input)
		{
			uci_result = realloc(uci_result, MIN(UCI_RES_MAX_SIZE, len + ) + 1);
			strcat(uci_result, ";");
			strcat(uci_result + 1, input);
		}
	}
	else
	{
		if(input)
		{
			//uci_result = malloc(MIN(UCI_RES_MAX_SIZE, strlen(input)));
			uci_result = malloc(strlen(input)+1);
			if(uci_result)
			{
				memset(uci_result, '\0', strlen(input)+1);
				//strncpy(uci_result, input, MIN(UCI_RES_MAX_SIZE, strlen(input)));
				strcpy(uci_result, input);
			}
		}
	}
#else
	if(input)
	{
		if(type == 1 && (uci_list_rescnt < LIST_MAX_CNT))
		{
			uci_list_result[uci_list_rescnt] = malloc(strlen(input)+1);
			if(uci_list_result[uci_list_rescnt])
			{
				memset(uci_list_result[uci_list_rescnt], '\0', strlen(input)+1);
				strcpy(uci_list_result[uci_list_rescnt], input);
				uci_list_rescnt++;
			}
		}
		else
		{
			if(uci_result)
			{
				free(uci_result);
				uci_result = NULL;
			}
			uci_result = malloc(strlen(input)+1);
			if(uci_result)
			{
				memset(uci_result, '\0', strlen(input)+1);
				//strncpy(uci_result, input, MIN(UCI_RES_MAX_SIZE, strlen(input)));
				strcpy(uci_result, input);
			}
		}
	}
#endif
	return OK;
}

static int uci_result_clr(void)
{
	int i = 0;
	if(uci_result)
	{
		free(uci_result);
		uci_result = NULL;
	}
	for(i = 0; i < LIST_MAX_CNT; i++)
	{
		if(uci_list_result[i])
		{
			free(uci_list_result[i]);
			uci_list_result[i] = NULL;
		}
	}
	return OK;
}

static char * uci_result_get(void)
{
	return uci_result;
}

static void
uci_reset_typelist(void)
{
	struct uci_type_list *ltype = NULL;
	while (type_list != NULL) {
		ltype = type_list;
		type_list = type_list->next;
		if(ltype)
			free(ltype);
	}
	if (typestr) {
		free(typestr);
		typestr = NULL;
	}
	cur_section_ref = NULL;
}

static char *
uci_lookup_section_ref(struct uci_section *s)
{
	struct uci_type_list *ti = type_list;
	int maxlen = 0;
	if(!s)
		return NULL;
	if (!s->anonymous || !(uci_flags & CLI_FLAG_SHOW_EXT))
		return s->e.name;

	/* look up in section type list */
	while (ti) {
		if (strcmp(ti->name, s->type) == 0)
			break;
		ti = ti->next;
	}
	if (!ti) {
		ti = malloc(sizeof(struct uci_type_list));
		if (!ti)
			return NULL;
		memset(ti, 0, sizeof(struct uci_type_list));
		ti->next = type_list;
		type_list = ti;
		ti->name = s->type;
	}

	maxlen = strlen(s->type) + 1 + 2 + 10;
	if (!typestr) {
		typestr = malloc(maxlen);
	} else {
		typestr = realloc(typestr, maxlen);
	}

	if (typestr)
		sprintf(typestr, "@%s[%d]", ti->name, ti->idx);

	ti->idx++;

	return typestr;
}


static void cli_perror(void)
{
	if (uci_flags & CLI_FLAG_QUIET)
		return;

	//uci_perror(uci_ctx, uci_appname);
}

static void cli_error(const char *fmt, ...)
{
	va_list ap;
	if (uci_flags & CLI_FLAG_QUIET)
		return;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void uci_print_value(zpl_uint32 type, FILE *f, const char *v)
{
#if 1
	char *p = v;
	int i = 0;
	char buf[64];
	memset(buf, 0, sizeof(buf));
	while (*p) {
		if (*p != '\'')
		{
			buf[i++] = *p;
		}
		else
		{
			if(i > 0)
			{
				_UCI_DEBUG("uci_print_value---->%s\r\n", buf);
				uci_result_add(type, buf);
				memset(buf, 0, sizeof(buf));
			}
			i = 0;
		}
		p++;
	}
#else
	fprintf(f, "'");
	while (*v) {
		if (*v != '\'')
		{
			fputc(*v, f);
			uci_result_add(0, *v);
		}
		else
			fprintf(f, "'\\''");
		v++;
	}
	fprintf(f, "'");
#endif
}

static void uci_show_value(struct uci_option *o, zpl_bool quote)
{
	struct uci_element *e = NULL;
	zpl_bool sep = zpl_false;
	char *space = NULL;
	if(!o)
		return;
	switch(o->type) {
	case UCI_TYPE_STRING:
		if (quote)
			uci_print_value(0, stdout, o->v.string);
		else
			uci_result_add(0, o->v.string);
			//printf("uci_show_section---->%s", o->v.string);
		_UCI_DEBUG("uci_show_value---->UCI_TYPE_STRING\r\n");
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			//uci_result_add(0, (sep ? uci_delimiter : " "));
			_UCI_DEBUG("uci_show_value---->UCI_TYPE_LIST(%s)\r\n", (sep ? uci_delimiter : " "));
			space = strpbrk(e->name, " \t\r\n");
			if (!space && !quote)
			{
				uci_result_add(1, e->name);
				_UCI_DEBUG("uci_show_value---->UCI_TYPE_LIST(e->name:%s)\r\n", e->name);
			}
			else
			{
				uci_print_value(1, stdout, e->name);
			}
			sep = zpl_true;
		}
		_UCI_DEBUG("uci_show_section---->\r\n");
		break;
	default:
		_UCI_DEBUG("uci_show_section----><unknown>\r\n");
		break;
	}
}

static void uci_show_option(struct uci_option *o, zpl_bool quote)
{
/*	printf("uci_show_option---->%s.%s.%s=",
		o->section->package->e.name,
		(cur_section_ref ? cur_section_ref : o->section->e.name),
		o->e.name);*/
	if(!o)
		return;
	uci_show_value(o, quote);
}

static void uci_show_section(struct uci_section *s)
{
	struct uci_element *e = NULL;
	const char *cname = NULL;
	const char *sname = NULL;
	if(!s || !s->package)
		return;
	cname = s->package->e.name;
	sname = (cur_section_ref ? cur_section_ref : s->e.name);
	if(s->type)
		uci_result_add(0, s->type);
	//printf("uci_show_section---->%s.%s=%s\r\n", cname, sname, s->type);
	uci_foreach_element(&s->options, e) {
		uci_show_option(uci_to_option(e), zpl_true);
	}
}

static void uci_show_package(struct uci_package *p)
{
	struct uci_element *e = NULL;
	if(!p)
		return;
	uci_reset_typelist();
	uci_foreach_element( &p->sections, e) {
		struct uci_section *s = uci_to_section(e);
		cur_section_ref = uci_lookup_section_ref(s);
		uci_show_section(s);
	}
	uci_reset_typelist();
}

static void uci_show_changes(struct uci_package *p)
{
	struct uci_element *e = NULL;
	if(!p)
		return;
	uci_foreach_element(&p->saved_delta, e) {
		struct uci_delta *h = uci_to_delta(e);
		char *prefix = "";
		char *op = "=";

		switch(h->cmd) {
		case UCI_CMD_REMOVE:
			prefix = "-";
			break;
		case UCI_CMD_LIST_ADD:
			op = "+=";
			break;
		case UCI_CMD_LIST_DEL:
			op = "-=";
			break;
		default:
			break;
		}
		//printf("uci_show_changes---->%s%s.%s", prefix, p->e.name, h->section);
/*		if (e->name)
			printf("uci_show_changes---->.%s", e->name);*/
		if (h->cmd != UCI_CMD_REMOVE && h->value) {
			//printf("uci_show_changes---->%s", op);
			uci_print_value(0, stdout, h->value);
		}
		_UCI_DEBUG("uci_show_changes---->\r\n");
	}
}

static int package_cmd(zpl_uint32 cmd, char *tuple)
{
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	int ret = 1;
	if(!tuple)
		return 1;
	memset(&ptr, 0, sizeof(struct uci_ptr));
	if (uci_lookup_ptr(uci_ctx, &ptr, tuple, zpl_true) != UCI_OK) {
		//printf("==========%s==========uci_lookup_ptr\r\n", __func__);
		cli_perror();
		return 1;
	}

	e = ptr.last;
	switch(cmd) {
	case CMD_CHANGES:
		uci_show_changes(ptr.p);
		break;
	case CMD_COMMIT:
		if (uci_flags & CLI_FLAG_NOCOMMIT) {
			ret = 0;
			goto out;
		}
		if (uci_commit(uci_ctx, &ptr.p, zpl_false) != UCI_OK) {
			//printf("==========%s==========uci_commit\r\n", __func__);
			cli_perror();
			goto out;
		}
		break;
	case CMD_EXPORT:
		if (uci_export(uci_ctx, stdout, ptr.p, zpl_true) != UCI_OK) {
			goto out;
		}
		break;
	case CMD_SHOW:
		if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
			uci_ctx->err = UCI_ERR_NOTFOUND;
			//printf("==========%s==========UCI_ERR_NOTFOUND\r\n", __func__);
			cli_perror();
			goto out;
		}
		switch(e->type) {
			case UCI_TYPE_PACKAGE:
				uci_show_package(ptr.p);
				break;
			case UCI_TYPE_SECTION:
				uci_show_section(ptr.s);
				break;
			case UCI_TYPE_OPTION:
				uci_show_option(ptr.o, zpl_true);
				break;
			default:
				/* should not happen */
				goto out;
		}
		break;
	}

	ret = 0;

out:
	if (ptr.p)
		uci_unload(uci_ctx, ptr.p);
	return ret;
}

static int uci_do_import(int uci_argc, char **uci_argv)
{
	struct uci_package *package = NULL;
	char *name = NULL;
	int ret = UCI_OK;
	zpl_bool merge = zpl_false;

	if (uci_argc > 2)
		return 255;

	if (uci_argc == 2)
		name = uci_argv[1];
	else if (uci_flags & CLI_FLAG_MERGE)
		/* need a package to merge */
		return 255;

	if (uci_flags & CLI_FLAG_MERGE) {
		if (uci_load(uci_ctx, name, &package) != UCI_OK)
			package = NULL;
		else
			merge = zpl_true;
	}
	//printf("dddddddddddddddddddd--uci_import--dddddddddddddddddddddddd\r\n");
	ret = uci_import(uci_ctx, uci_input, name, &package, (name != NULL));
	if (ret == UCI_OK) {
		if (merge) {
			ret = uci_save(uci_ctx, package);
		} else {
			struct uci_element *e = NULL;
			/* loop through all config sections and overwrite existing data */
			uci_foreach_element(&uci_ctx->root, e) {
				struct uci_package *p = uci_to_package(e);
				if(p)
					ret = uci_commit(uci_ctx, &p, zpl_true);
			}
		}
	}

	if (ret != UCI_OK) {
		//printf("==========%s==========uci_import\r\n", __func__);
		cli_perror();
		return 1;
	}

	return 0;
}

static int uci_do_package_cmd(zpl_uint32 cmd, int uci_argc, char **uci_argv)
{
	char **configs = NULL;
	char **p = NULL;
	int ret = 1;

	if (uci_argc > 2)
		return 255;

	if (uci_argc == 2)
		return package_cmd(cmd, uci_argv[1]);

	if ((uci_list_configs(uci_ctx, &configs) != UCI_OK) || !configs) {
		//printf("==========%s==========uci_list_configs\r\n", __func__);
		cli_perror();
		goto out;
	}

	for (p = configs; *p; p++) {
		package_cmd(cmd, *p);
	}

	ret = 0;
out:
	free(configs);
	return ret;
}

static int uci_do_add(int uci_argc, char **uci_argv)
{
	struct uci_package *p = NULL;
	struct uci_section *s = NULL;
	int ret;

	if (uci_argc != 3)
		return 255;

	ret = uci_load(uci_ctx, uci_argv[1], &p);
	if (ret != UCI_OK)
		goto done;

	ret = uci_add_section(uci_ctx, p, uci_argv[2], &s);
	if (ret != UCI_OK)
		goto done;

	ret = uci_save(uci_ctx, p);

done:
	if (ret != UCI_OK)
	{
		//printf("==========%s==========uci_save\r\n", __func__);
		cli_perror();
	}
	else if (s)
	{
		;//fprintf(stdout, "uci_do_add---->%s\r\n", s->e.name);
	}
	return ret;
}

static int uci_do_section_cmd(zpl_uint32 cmd, int uci_argc, char **uci_argv)
{
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	int ret = UCI_OK;
	int dummy;

	if (uci_argc != 2)
	{
		_UCI_DEBUG("uci_do_section_cmd uci_argc =%d\r\n", uci_argc);
		return 255;
	}
	memset(&ptr, 0, sizeof(struct uci_ptr));
	if (uci_lookup_ptr(uci_ctx, &ptr, uci_argv[1], zpl_true) != UCI_OK) {
		//printf("==========%s==========uci_lookup_ptr\r\n", __func__);
		cli_perror();
		_UCI_DEBUG("uci_do_section_cmd uci_lookup_ptr\r\n");
		return 1;
	}

	if (ptr.value && (cmd != CMD_SET) && (cmd != CMD_DEL) &&
	    (cmd != CMD_ADD_LIST) && (cmd != CMD_DEL_LIST) &&
	    (cmd != CMD_RENAME) && (cmd != CMD_REORDER))
	{
		_UCI_DEBUG("uci_do_section_cmd uci_lookup_ptr next\r\n");
		return 1;
	}
	e = ptr.last;
	if(!e)
		return 1;
	switch(cmd) {
	case CMD_GET:
		_UCI_DEBUG("uci_do_section_cmd CMD_GET\r\n");
		if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
			uci_ctx->err = UCI_ERR_NOTFOUND;
			//printf("==========%s==========UCI_ERR_NOTFOUND\r\n", __func__);
			cli_perror();
			_UCI_DEBUG("uci_do_section_cmd uci_lookup_ptr UCI_ERR_NOTFOUND\r\n");
			return 1;
		}
		switch(e->type) {
		case UCI_TYPE_SECTION:
			if(ptr.s->type)
			{
				printf("uci_do_section_cmd--->%s\r\n", ptr.s->type);
				uci_result_add(0, ptr.s->type);
			}
			//uci_result = strdup(ptr.s->type);
			//printf("uci_do_section_cmd--->%s\r\n", ptr.s->type);
			break;
		case UCI_TYPE_OPTION:
			uci_show_value(ptr.o, zpl_false);
			break;
		default:
			_UCI_DEBUG("uci_do_section_cmd--->e->type=%d\r\n", e->type);
			break;
		}
		/* throw the value to stdout */
		break;
	case CMD_RENAME:
		ret = uci_rename(uci_ctx, &ptr);
		break;
	case CMD_REVERT:
		ret = uci_revert(uci_ctx, &ptr);
		break;
	case CMD_SET:
		_UCI_DEBUG("uci_do_section_cmd CMD_SET\r\n");
		ret = uci_set(uci_ctx, &ptr);
		break;
	case CMD_ADD_LIST:
		ret = uci_add_list(uci_ctx, &ptr);
		break;
	case CMD_DEL_LIST:
		ret = uci_del_list(uci_ctx, &ptr);
		break;
	case CMD_REORDER:
		if (!ptr.s || !ptr.value) {
			uci_ctx->err = UCI_ERR_NOTFOUND;
			//printf("==========%s==========CMD_REORDER UCI_ERR_NOTFOUND\r\n", __func__);
			cli_perror();
			return 1;
		}
		ret = uci_reorder_section(uci_ctx, ptr.s, strtoul(ptr.value, NULL, 10));
		break;
	case CMD_DEL:
		_UCI_DEBUG("uci_do_section_cmd CMD_DEL\r\n");
		if (ptr.value && !sscanf(ptr.value, "%d", &dummy))
			return 1;
		ret = uci_delete(uci_ctx, &ptr);
		break;
	}

	/* no save necessary for get */
	if ((cmd == CMD_GET) || (cmd == CMD_REVERT))
	{
		_UCI_DEBUG("uci_do_section_cmd no save necessary for get\r\n");
		return 0;
	}
	/* save changes, but don't commit them yet */
	if (ret == UCI_OK)
		ret = uci_save(uci_ctx, ptr.p);

	if (ret != UCI_OK) {
		//printf("==========%s==========uci_save\r\n", __func__);
		cli_perror();
		_UCI_DEBUG("uci_do_section_cmd uci_save\r\n");
		return 1;
	}

	return 0;
}

static int uci_batch_cmd(void)
{
	char *uci_argv[UCI_MAX_ARGS + 2];
	char *str = NULL;
	int ret = 0;
	int i, j;

	for(i = 0; i <= UCI_MAX_ARGS; i++) {
		if (i == UCI_MAX_ARGS) {
			cli_error("Too many arguments\r\n");
			return 1;
		}
		uci_argv[i] = NULL;
		//printf("dddddddddddddddddddd--uci_parse_argument--dddddddddddddddddddddddd\r\n");
		if ((ret = uci_parse_argument(uci_ctx, uci_input, &str, &uci_argv[i])) != UCI_OK) {
			//printf("==========%s==========uci_parse_argument\r\n", __func__);
			cli_perror();
			i = 0;
			break;
		}
		if (!uci_argv[i][0])
			break;
		uci_argv[i] = strdup(uci_argv[i]);
		if (!uci_argv[i]) {
			//printf("==========%s==========strdup\r\n", __func__);
			cli_error("uci: %s", strerror(errno));
			return 1;
		}
	}
	uci_argv[i] = NULL;

	if (i > 0) {
		if (!strcasecmp(uci_argv[0], "exit"))
			return 254;
		ret = uci_cmd(i, uci_argv);
	} else
		return 0;

	for (j = 0; j < i; j++) {
		free(uci_argv[j]);
	}

	return ret;
}

static int uci_batch(void)
{
	int ret = 0;
	//printf("dddddddddddddddddddd--uci_batch--dddddddddddddddddddddddd\r\n");
	uci_flags |= CLI_FLAG_BATCH;
	while (!feof(uci_input)) {
		struct uci_element *e = NULL, *tmp = NULL;

		ret = uci_batch_cmd();
		if (ret == 254)
			return 0;
		else if (ret == 255)
			cli_error("Unknown command\r\n");

		/* clean up */
		uci_foreach_element_safe(&uci_ctx->root, tmp, e) {
			uci_unload(uci_ctx, uci_to_package(e));
		}
	}
	uci_flags &= ~CLI_FLAG_BATCH;

	return 0;
}

static int uci_cmd(int uci_argc, char **uci_argv)
{
	zpl_uint32 cmd = 0;
	_UCI_DEBUG("uci_cmd: %s \r\n", uci_argv[0]);
	if (!strcasecmp(uci_argv[0], "batch") && !(uci_flags & CLI_FLAG_BATCH))
		return uci_batch();
	else if (!strcasecmp(uci_argv[0], "show"))
		cmd = CMD_SHOW;
	else if (!strcasecmp(uci_argv[0], "changes"))
		cmd = CMD_CHANGES;
	else if (!strcasecmp(uci_argv[0], "export"))
		cmd = CMD_EXPORT;
	else if (!strcasecmp(uci_argv[0], "commit"))
		cmd = CMD_COMMIT;
	else if (!strcasecmp(uci_argv[0], "get"))
		cmd = CMD_GET;
	else if (!strcasecmp(uci_argv[0], "set"))
		cmd = CMD_SET;
	else if (!strcasecmp(uci_argv[0], "ren") ||
	         !strcasecmp(uci_argv[0], "rename"))
		cmd = CMD_RENAME;
	else if (!strcasecmp(uci_argv[0], "revert"))
		cmd = CMD_REVERT;
	else if (!strcasecmp(uci_argv[0], "reorder"))
		cmd = CMD_REORDER;
	else if (!strcasecmp(uci_argv[0], "del") ||
	         !strcasecmp(uci_argv[0], "delete"))
		cmd = CMD_DEL;
	else if (!strcasecmp(uci_argv[0], "import"))
		cmd = CMD_IMPORT;
	else if (!strcasecmp(uci_argv[0], "help"))
		cmd = CMD_HELP;
	else if (!strcasecmp(uci_argv[0], "add"))
		cmd = CMD_ADD;
	else if (!strcasecmp(uci_argv[0], "add_list"))
		cmd = CMD_ADD_LIST;
	else if (!strcasecmp(uci_argv[0], "del_list"))
		cmd = CMD_DEL_LIST;
	else
		cmd = -1;

	switch(cmd) {
		case CMD_ADD_LIST:
		case CMD_DEL_LIST:
		case CMD_GET:
		case CMD_SET:
		case CMD_DEL:
		case CMD_RENAME:
		case CMD_REVERT:
		case CMD_REORDER:
			_UCI_DEBUG("into uci_do_section_cmd\r\n");
			return uci_do_section_cmd(cmd, uci_argc, uci_argv);
		case CMD_SHOW:
		case CMD_EXPORT:
		case CMD_COMMIT:
		case CMD_CHANGES:
			_UCI_DEBUG("into uci_do_package_cmd\r\n");
			return uci_do_package_cmd(cmd, uci_argc, uci_argv);
		case CMD_IMPORT:
			_UCI_DEBUG("into uci_do_import\r\n");
			return uci_do_import(uci_argc, uci_argv);
		case CMD_ADD:
			_UCI_DEBUG("into uci_do_add\r\n");
			return uci_do_add(uci_argc, uci_argv);
		case CMD_HELP:
			//uci_usage();
			return 0;
		default:
			_UCI_DEBUG("uci_cmd default\r\n");
			return 255;
	}
}

static int uci_main(int uci_argc, char **uci_argv)
{
	int ret;
	//int c;

	uci_flags = CLI_FLAG_SHOW_EXT;
	uci_appname = uci_argv[0];
	//uci_input = stdin;
	uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		cli_error("Out of memory\n");
		_UCI_DEBUG("Out of memory\r\n");
		return 1;
	}
/*
	printf("input:\r\n");
	for(c = 0; c < uci_argc; c++)
	{
		printf("%s ", uci_argv[c]);
	}
	printf("\r\n");
*/

	uci_result_clr();
	uci_set_confdir(uci_ctx, "/etc/config");
#if 0
	while((c = getopt(uci_argc, uci_argv, "c:d:f:LmnNp:P:sSqX")) != -1) {
		switch(c) {
			case 'c':
				uci_set_confdir(uci_ctx, optarg);
				break;
			case 'd':
				uci_delimiter = optarg;
				break;
			case 'f':
				if (uci_input != stdin) {
					fclose(uci_input);
					cli_error("Too many uci_input files.\n");
					_UCI_DEBUG("Too many uci_input files.\r\n");
					return 1;
				}

				uci_input = fopen(optarg, "r");
				if (!uci_input) {
					cli_error("uci: %s", strerror(errno));
					return 1;
				}
				break;
			case 'm':
				uci_flags |= CLI_FLAG_MERGE;
				break;
			case 's':
				uci_ctx->flags |= UCI_FLAG_STRICT;
				break;
			case 'S':
				uci_ctx->flags &= ~UCI_FLAG_STRICT;
				uci_ctx->flags |= UCI_FLAG_PERROR;
				break;
			case 'n':
				uci_ctx->flags |= UCI_FLAG_EXPORT_NAME;
				break;
			case 'N':
				uci_ctx->flags &= ~UCI_FLAG_EXPORT_NAME;
				break;
			case 'p':
				uci_add_delta_path(uci_ctx, optarg);
				break;
			case 'P':
				uci_add_delta_path(uci_ctx, uci_ctx->savedir);
				uci_set_savedir(uci_ctx, optarg);
				uci_flags |= CLI_FLAG_NOCOMMIT;
				break;
			case 'q':
				uci_flags |= CLI_FLAG_QUIET;
				break;
			case 'X':
				uci_flags &= ~CLI_FLAG_SHOW_EXT;
				break;
			default:
				uci_usage();
				_UCI_DEBUG("uci_usage\r\n");
				return 0;
		}
	}
	if (optind > 1)
		uci_argv[optind - 1] = uci_argv[0];
	uci_argv += optind - 1;
	uci_argc -= optind - 1;
#endif

	if (uci_argc < 2) {
		//uci_usage();
		return 1;
	}
	_uci_errno = uci_ctx->err = UCI_OK;
	ret = uci_cmd(uci_argc - 1, uci_argv + 1);
/*	if (uci_input != stdin)
		fclose(uci_input);*/
/*
	if (ret == 255)
		uci_usage();
*/
	_uci_errno = uci_ctx->err;
	uci_free_context(uci_ctx);
	_UCI_DEBUG("uci_free_context\r\n");
	return ret;
}

int os_uci_get_errno(void)
{
	return _uci_errno;
}
/***********************************************************************/
int os_uci_set_string(char *name, char *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "set", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s=%s", name, value);
	cmd_argc = 3;

	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_set_integer(char *name, int value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "set", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s=%d", name, value);
	cmd_argc = 3;

	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_set_float(char *name, char *fmt, zpl_float value)
{
	char cmd_value[512];
	char tmp[32];
	char *cmd_argv[5] = {"uci", "set", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name)
		return ERROR;
	memset(tmp, 0, sizeof(tmp));
	memset(cmd_value, 0, sizeof(cmd_value));

	sprintf(tmp, fmt, value);
	//printf("---------%s----------%s\r\n", __func__, tmp);

	sprintf(cmd_value, "%s=%s", name, tmp);
	//printf("---------%s----------%s\r\n", __func__, cmd_value);
	cmd_argc = 3;

	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_set_double(char *name, char *fmt, double value)
{
	char cmd_value[512];
	char tmp[32];
	char *cmd_argv[5] = {"uci", "set", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name)
		return ERROR;
	memset(tmp, 0, sizeof(tmp));
	memset(cmd_value, 0, sizeof(cmd_value));

	sprintf(tmp, fmt, value);
	//printf("---------%s----------%s->%f\r\n", __func__, tmp, value);

	sprintf(cmd_value, "%s=%s", name, tmp);
	//printf("---------%s----------%s\r\n", __func__, cmd_value);
	cmd_argc = 3;

	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}
int os_uci_get_string(char *name, char *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	cmd_argv[2] = cmd_value;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		if(uci_result_get())
		{
			if(value)
				strcpy(value, uci_result_get());
				//strncpy(value, uci_result_get(), MIN(strlen(uci_result_get()), UCI_RES_MAX_SIZE));
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

int os_uci_get_address(char *name, char *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		if(uci_result_get())
		{
			if(value)
				strncpy(value, uci_result_get(), MIN(strlen(uci_result_get()), UCI_RES_MAX_SIZE));
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

int os_uci_get_integer(char *name, int *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		if(uci_result_get())
		{
			if(value)
				*value = atoi(uci_result_get());
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

int os_uci_get_float(char *name, char *fmt, zpl_float *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		if(uci_result_get())
		{
			if(value)
				*value = (zpl_float)atof(uci_result_get());
			//if(value)
			//	sscanf(uci_result_get(), fmt, value);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

int os_uci_get_double(char *name, char *fmt, double *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		if(uci_result_get())
		{
			if(value)
				*value = (double)atof(uci_result_get());
			//if(value)
			//	sscanf(uci_result_get(), fmt, value);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}
int os_uci_get_list(char *name, char **value, int *cnt)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "get", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	//char **argvtmp[5] = { "uci", "get", name, NULL};
	//os_uci_value_reset();
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		int i = 0, num = 0;
		for(i = 0; i < LIST_MAX_CNT; i++)
		{
			if(uci_list_result[i])
			{
				*value = strdup(uci_list_result[i]);
				num++;
				_UCI_DEBUG("------%s--------:%d(%s)\r\n", __func__, i, uci_list_result[i]);
			}
		}
		if(cnt)
			*cnt = num;
/*		if(uci_result_get())
		{
			printf("------%s--------:(%s)\r\n", __func__, uci_result_get());
		}*/
		return OK;
	}
	return ERROR;
}

int os_uci_list_add(char *name, char * value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "add_list", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	//sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	sprintf(cmd_value, "%s=%s", name, value);
	//char **argvtmp[5] = { "uci", "get", name, NULL};
	//os_uci_value_reset();
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
/*		if(os_uci_value.cnt == 1)
			if(value)
				strcpy(value, os_uci_value.value[0]);
		os_uci_value_reset();*/
		return OK;
	}
	return ERROR;
}

int os_uci_list_del(char *name, char * value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "del_list", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name || !value)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	//sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	sprintf(cmd_value, "%s=%s", name, value);
	//char **argvtmp[5] = { "uci", "get", name, NULL};
	//os_uci_value_reset();
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
/*		if(os_uci_value.cnt == 1)
			if(value)
				strcpy(value, os_uci_value.value[0]);
		os_uci_value_reset();*/
		return OK;
	}
	return ERROR;
}

int os_uci_del(char *name, char * section, char * option, char *value)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "delete", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name)
		return ERROR;
	cmd_argc = 3;

	if(section && option && value)
		snprintf(cmd_value, sizeof(cmd_value),"%s.%s.%s=%s", name, section, option, value);
	else if(section && option)
		snprintf(cmd_value, sizeof(cmd_value),"%s.%s.%s", name, section, option);
	else if(section)
		snprintf(cmd_value, sizeof(cmd_value),"%s.%s", name, section);
	else //if(section)
		snprintf(cmd_value, sizeof(cmd_value),"%s", name);

	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_section_add(char *name, char * section)
{
	return ERROR;
}


int os_uci_commit(char *name)
{
	char cmd_value[512];
	char *cmd_argv[5] = {"uci", "commit", cmd_value, NULL, NULL};
	int cmd_argc;
	if(!name)
		return ERROR;
	memset(cmd_value, 0, sizeof(cmd_value));
	sprintf(cmd_value, "%s", name);
	cmd_argc = 3;
	if(uci_main(cmd_argc, cmd_argv) == 0)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_save_config(char *name)
{
	return os_uci_commit(name);
}


int os_uci_init()
{
	uci_delimiter = " ";
	uci_appname = NULL;
	uci_input = NULL;
	uci_ctx = NULL;
	type_list = NULL;
	typestr = NULL;
	cur_section_ref = NULL;
	uci_result_clr();
	return OK;
}

#else /* ZPL_OPENWRT_UCI_LIB */

static int os_uci_input_split(char *input, char *file, char *type, char *option)
{
	int flag = 0;
	char *p = input, *brk = NULL;
	brk = strstr(p, ".");
	if(brk)
	{
		if(file)
			strncpy(file, p, brk - p);
		flag++;
	}
	p = brk + 1;
	brk = strstr(p, ".");
	if(brk)
	{
		if(type)
			strncpy(file, p, brk - p);
		flag++;
	}
	p = brk + 1;
	brk = strstr(p, ".");
	if(brk)
	{
		if(option)
			strncpy(file, p, brk - p);
		flag++;
	}
	if(flag == 3)
		return OK;
	return ERROR;
}

static int os_uci_input_type_get(FILE *fp, FILE *ip, char *type)
{
	if (fp)
	{
		char buf[512];
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			if(strstr(buf, "config"))
			{
				if(strstr(buf, type))
				{
					if(ip)
						fputs(buf, ip);
					return OK;
				}
			}
			else
			{
				if(ip)
					fputs(buf, ip);
			}
			memset(buf, 0, sizeof(buf));
		}
		return ERROR;
	}
	return ERROR;
}

static int os_uci_input_value_get(char *input, char *option, char *value)
{
	int flag = 0, j = 0;
	char *p = input, *brk = NULL;
	brk = strstr(p, option);
	p = brk + strlen(option);
	if(p)
	{
		while(p)
		{
			if(*p == '\'' || *p == '"')
			{
				if(flag == 0)
					flag = 1;
				else
					flag = 2;

			}
			if(flag == 1)
			{
				if(value)
					value[j++] = *p;
			}
			if(flag == 2)
				return OK;
			p++;
		}
	}
	return ERROR;
}

static int os_uci_input_option_value_get(FILE *fp, FILE *ip, char *option, char *value, char *b)
{
	if (fp)
	{
		char buf[512];
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			if(strstr(buf, "option"))
			{
				if(strstr(buf, option))
				{
					if(b)
						strcpy(b, buf);
					return os_uci_input_value_get(buf, option, value);
				}
				else
				{
					if(ip)
						fputs(buf, ip);
				}
			}
			else
			{
				if(ip)
					fputs(buf, ip);
			}
			memset(buf, 0, sizeof(buf));
		}
		return ERROR;
	}
	return ERROR;
}

static int os_uci_option_value_lookup(char *input, char *value, int *cnt)
{
	char file[128], type[128],  option[128],filepath[128];
	memset(file, 0, sizeof(file));
	memset(type, 0, sizeof(type));
	memset(option, 0, sizeof(option));
	memset(filepath, 0, sizeof(filepath));
	if(os_uci_input_split(input, file, type, option) == OK)
	{
		FILE *f = NULL;
		snprintf(filepath, sizeof(filepath), "/etc/config/%s", file);
		f = fopen(filepath, "r");
		if(f)
		{
			if(os_uci_input_type_get(f, NULL, type) == OK)
			{
				if(os_uci_input_option_value_get(f, NULL, option, value, NULL) == OK)
				{
					fclose(f);
					return OK;
				}
			}
			fclose(f);
		}
	}
	return ERROR;
}

static int os_uci_option_value_set(char *input, char *value, int *cnt)
{
	char file[128], type[128],  option[128], filepath[128], infilepath[128],
		option_val[128], option_valtmp[128];
	memset(file, 0, sizeof(file));
	memset(type, 0, sizeof(type));
	memset(option, 0, sizeof(option));
	memset(filepath, 0, sizeof(filepath));
	memset(infilepath, 0, sizeof(infilepath));
	memset(option_val, 0, sizeof(option_val));
	memset(option_valtmp, 0, sizeof(option_valtmp));
	if(os_uci_input_split(input, file, type, option) == OK)
	{
		FILE *f = NULL;
		FILE *i = NULL;
		snprintf(filepath, sizeof(filepath), "/etc/config/%s", file);
		f = fopen(filepath, "r");
		snprintf(infilepath, sizeof(infilepath), "/etc/config/%s.tmp", file);
		i = fopen(infilepath, "w+");
		if(f && i)
		{
			if(os_uci_input_type_get(f, i, type) == OK)
			{
				if(os_uci_input_option_value_get(f, i, option, option_val, option_valtmp) == OK)
				{
					int l = 0;
					char *p = option_valtmp, *brk = NULL;
					brk = strstr(p, option);
					p = brk + strlen(option);
					l = p - option_valtmp;
					if(p)
					{
						memset(p, 0, sizeof(option_valtmp)-l);
						strcat(p, "  ");
						strcat(p, "'");
						strcat(p, value);
						strcat(p, "'");
						fputs(option_valtmp, i);
					}
					memset(option_valtmp, 0, sizeof(option_valtmp));
					while (fgets(option_valtmp, sizeof(option_valtmp), f))
					{
						fputs(option_valtmp, i);
						memset(option_valtmp, 0, sizeof(option_valtmp));
					}
					fclose(f);
					fclose(i);
					remove(filepath);
					rename(infilepath, filepath);
					return OK;
				}
			}
			fclose(f);
			fclose(i);
			remove(infilepath);
		}
	}
	return ERROR;
}

int os_uci_set_string(char *name, char *value)
{
	return os_uci_option_value_set(name, value, NULL);
}
int os_uci_set_integer(char *name, int value)
{
	return os_uci_option_value_set(name, itoa(value, 0), NULL);
}

int os_uci_set_float(char *name, char *fmt, zpl_float value)
{
	char tmp[32];
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, fmt, value);

	return os_uci_option_value_set(name, tmp, NULL);
}

int os_uci_set_double(char *name, char *fmt, double value)
{
	char tmp[32];
	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, fmt, value);

	return os_uci_option_value_set(name, tmp, NULL);
}

int os_uci_get_string(char *name, char *value)
{
	return os_uci_option_value_lookup(name, value, NULL);
}

int os_uci_get_address(char *name, char *value)
{
	return os_uci_option_value_lookup(name, value, NULL);
}

int os_uci_get_integer(char *name, int *value)
{
	char ret_value[512];
	memset(ret_value, 0, sizeof(ret_value));
	if( os_uci_option_value_lookup(name, ret_value, NULL) == OK)
	{
		if(value)
			sscanf(ret_value, "%d", value);
		return OK;
	}
	return ERROR;
}

int os_uci_get_float(char *name, char *fmt, zpl_float *value)
{
	char ret_value[512];
	memset(ret_value, 0, sizeof(ret_value));
	if( os_uci_option_value_lookup(name, ret_value, NULL) == OK)
	{
		if(value)
			*value = (zpl_float)atof(ret_value);
		//if(value)
		//	sscanf(ret_value, fmt, value);
		return OK;
	}
	return ERROR;
}

int os_uci_get_double(char *name, char *fmt, double *value)
{
	char ret_value[512];
	memset(ret_value, 0, sizeof(ret_value));
	if( os_uci_option_value_lookup(name, ret_value, NULL) == OK)
	{
		if(value)
			*value = (double)atof(ret_value);
		//if(value)
		//	sscanf(ret_value, fmt, value);
		return OK;
	}
	return ERROR;
}

int os_uci_get_list(char *name, char **value, int *cnt)
{
	if( os_uci_option_value_lookup(name, *value, cnt) == OK)
	{
		return OK;
	}
	return ERROR;
}

int os_uci_list_add(char *name, char * value)
{
	return ERROR;
}

int os_uci_list_del(char *name, char * value)
{
	return ERROR;
}

int os_uci_del(char *name, char * section, char * option, char *value)
{
	return ERROR;
}

int os_uci_section_add(char *name, char * section)
{
	return ERROR;
}


int os_uci_commit(char *name)
{
	return OK;
}

int os_uci_save_config(char *name)
{
	return os_uci_commit(name);
}


int os_uci_init()
{
	return OK;
}
#endif /* ZPL_OPENWRT_UCI_LIB */

#endif /* ZPL_OPENWRT_UCI */

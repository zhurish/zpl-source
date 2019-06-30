/*
 * web_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

//#include "zebra.h"
#ifdef HAVE_CONFIG_H
#include "plconfig.h"
#endif /* HAVE_CONFIG_H */

#include "os_platform.h"
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include "module.h"
#include "memory.h"
#include "vector.h"
#include "zassert.h"
#include "host.h"
#include "log.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_task.h"

#include "goahead.h"
#include "webgui_app.h"


static int jst_html_file(webgui_app_t *webgui, char *filename)
{
	if(strstr(filename, ".html"))
	{
		char path[64];
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/html/%s", webgui->documents, filename);
		if(access(path, F_OK) == 0)
			return OK;
		return ERROR;
	}
	return ERROR;
}


static char * jst_html_dir(webgui_app_t *webgui, char *filename)
{
	static char path[64];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/html/%s", webgui->documents, filename);
	return path;
}

static int jst_header(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char buf[1024];
	if(jst_html_file(webgui_app, argv[0]) == ERROR)
		return ERROR;
	f = fopen(jst_html_dir(webgui_app, argv[0]), "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
		    websWrite(wp, buf);
		    memset(buf, 0, sizeof(buf));
		}
		fclose(f);
		return OK;
	}
    return ERROR;
}

static int jst_footer(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char buf[1024];
	if(jst_html_file(webgui_app, argv[0]) == ERROR)
		return ERROR;
	f = fopen(jst_html_dir(webgui_app, argv[0]), "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
		    websWrite(wp, buf);
		    memset(buf, 0, sizeof(buf));
		}
		fclose(f);
		return OK;
	}
    return ERROR;
}


static int jst_menu_save(webgui_app_t *webgui, char *menu, char *slide, char *li)
{
	if(menu)
	{
		if(webgui->menu1)
		{
			free(webgui->menu1);
			webgui->menu1 = NULL;
		}
		webgui->menu1 = strdup(menu);
	}
	if(slide)
	{
		if(webgui->menu2)
		{
			free(webgui->menu2);
			webgui->menu2 = NULL;
		}
		webgui->menu2 = strdup(slide);
	}
	if(li)
	{
		if(webgui->menu3)
		{
			free(webgui->menu3);
			webgui->menu3 = NULL;
		}
		webgui->menu3 = strdup(li);
	}
	return OK;
}

/*static int jst_tab_menu_save(webgui_app_t *webgui, char *tab_menu1, char *tab_menu2)
{
	if(tab_menu1)
	{
		if(webgui->tab_menu1)
		{
			free(webgui->tab_menu1);
			webgui->tab_menu1 = NULL;
		}
		webgui->tab_menu1 = strdup(tab_menu1);
	}
	if(tab_menu2)
	{
		if(webgui->tab_menu2)
		{
			free(webgui->tab_menu2);
			webgui->tab_menu2 = NULL;
		}
		webgui->tab_menu2 = strdup(tab_menu2);
	}
	return OK;
}*/

static int jst_menu_active(int eid, webs_t wp, char *sbuf, char *menu, char *slide, char *li)
{
	static char m = 0;
	char *p = NULL;
	if(strstr(sbuf, menu) && !strstr(sbuf, "active"))
	{
		websWrite(wp, "<a class=\"menu");
		websWrite(wp, " active\"");
		p = strstr(sbuf, "data-title");
		if(p)
			websWrite(wp, p);
		m = 1;

		jst_menu_save(webgui_app, menu, NULL, NULL);
		return OK;
	}
	if(m)
	{
		websWrite(wp, sbuf);
		m = 0;
		return OK;
	}
	if(strstr(sbuf, li) && !strstr(sbuf, "active"))
	{
		//<li><a data-title="System Log" href="syslog.html">System Log</a></li>
		websWrite(wp, "<li ");
		websWrite(wp, "class=\"active\" >");
		p = strstr(sbuf, "<a");
		if(p)
			websWrite(wp, p);
		m = 1;
		jst_menu_save(webgui_app, NULL, NULL, li);
		return OK;
	}
	return ERROR;
}

static int jst_menu(int eid, webs_t wp, int argc, char **argv)
{
	int i = 0;
	FILE *f;
	char buf[1024];

	if(jst_html_file(webgui_app, argv[0]) == ERROR)
		return ERROR;
	f = fopen(jst_html_dir(webgui_app, argv[0]), "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			i = jst_menu_active( eid,  wp, buf, argv[1], NULL, argv[2]);
			if(i == 0)
				continue;
			if(strstr(buf, "<li><a"))
			{
				if(strstr(buf, "System"))
				{
					websWrite(wp, buf);
				}
				else if(strstr(buf, "Administration"))
				{
					websWrite(wp, buf);
				}
				else
					websWrite(wp, buf);
			}
			else
				websWrite(wp, buf);
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
		return OK;
	}
    return ERROR;
}


static int jst_load(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char buf[1024];
	if(jst_html_file(webgui_app, argv[0]) == ERROR)
		return ERROR;
	f = fopen(jst_html_dir(webgui_app, argv[0]), "r");
	if (f)
	{
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			if(strstr(buf, argv[1]))
			{
				websWrite(wp, buf);
			}
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
		return OK;
	}
    return ERROR;
}

/*
static int jst_test(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "<li><a data-title=\"Logout\" href=\"#logout\">Logout</a></li>");
    return 0;
}
*/

int web_jst_html_init(void)
{
	websDefineJst("jst_header", jst_header);
	websDefineJst("jst_menu", jst_menu);
	websDefineJst("jst_footer", jst_footer);
	websDefineJst("jst_load", jst_load);

	//websDefineJst("jst_test", jst_test);
	return 0;
}



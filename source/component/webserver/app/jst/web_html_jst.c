/*
 * web_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "module.h"
#include "zmemory.hh"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


#ifndef THEME_V9UI
static int jst_html_file(web_app_t *webgui, char *filename)
{
	if(strstr(filename, ".html"))
	{
		char path[512];
		memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/html/%s", webgui->documents, filename);
		if(access(path, F_OK) == 0)
			return OK;
		return ERROR;
	}
	return ERROR;
}


static char * jst_html_dir(web_app_t *webgui, char *filename)
{
	static char path[512];
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/html/%s", webgui->documents, filename);
	return path;
}


static int jst_html_load(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char buf[1024];
	if(jst_html_file(web_app, argv[0]) == ERROR)
		return ERROR;
	f = fopen(jst_html_dir(web_app, argv[0]), "r");
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



static int jst_html_text(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char buf[1024];

	f = fopen(argv[0], "r");
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

static int jst_load(Webs *wp, char *path, char *query)
{
	FILE *f = NULL;
	int find = 0;
	int divcnt = 0;
	char buf[1024];
	char *filename = webs_get_var(wp, T("url"), T(""));
	char *div = webs_get_var(wp, T("div"), T(""));

	//printf("%s:filename=%s div=%s\r\n", __func__, filename, div);

	if(!filename || !div)
	{
		//printf("%s:filename=NULL||div=NULL\r\n", __func__);
		return web_return_text_plain(wp, ERROR);
	}
	if(jst_html_file(web_app, filename) == ERROR)
	{
		//printf("%s:%s/html/%s\r\n", __func__, web_app->documents, filename);
		return web_return_text_plain(wp, ERROR);
	}
	//printf("%s:%s\r\n", __func__, jst_html_dir(web_app, filename));
	f = fopen(jst_html_dir(web_app, filename), "r");
	if (f)
	{
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "text/plain");
		websWriteEndHeaders(wp);
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			if(divcnt && strstr(buf, "</div>"))
			{
				divcnt -= 1;
				if(divcnt == 0)
				{
					websWrite(wp, buf);
					websDone(wp);
					fclose(f);
					return OK;
				}
			}
			else
			{
				if(strstr(buf, div))
				{
					find = 1;
					if(strstr(buf, "div"))
						divcnt = 1;
				}
				else
				{
					if(divcnt && strstr(buf, "div"))
						divcnt += 1;
				}
			}
			if(divcnt)
				websWrite(wp, buf);

			memset(buf, 0, sizeof(buf));
		}
		websDone(wp);
		fclose(f);
		return OK;
	}
    return web_return_text_plain(wp, ERROR);
}
#endif /* THEME_V9UI */

int web_html_jst_init(void)
{
#ifndef THEME_V9UI
	websDefineJst("jst_html_load", jst_html_load);
	websDefineJst("jst_html_text", jst_html_text);
	websFormDefine("html_load", jst_load);
#endif /* THEME_V9UI */
	return 0;
}



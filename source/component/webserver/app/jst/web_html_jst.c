/*
 * web_html.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */

#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"

#include "web_api.h"
#include "web_jst.h"
#include "web_app.h"



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

	if(!filename || !div)
	{
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "can not get url path");
	}
	if(jst_html_file(web_app, filename) == ERROR)
	{
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "url path is not exist");
	}
	f = fopen(jst_html_dir(web_app, filename), "r");
	if (f)
	{
		websSetStatus(wp, 200);
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			if(divcnt && strstr(buf, "</div>"))
			{
				divcnt -= 1;
				if(divcnt == 0)
				{
					websWriteCache(wp, buf);
					websWriteHeaders (wp, websWriteCacheLen(wp), 0);
					websWriteHeader (wp, "Content-Type", "text/plain");
					websWriteEndHeaders (wp);	
					websWriteCacheFinsh(wp);
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
				websWriteCache(wp, buf);

			memset(buf, 0, sizeof(buf));
		}
		websWriteHeaders (wp, websWriteCacheLen(wp), 0);
		websWriteHeader (wp, "Content-Type", "text/plain");
		websWriteEndHeaders (wp);	
		websWriteCacheFinsh(wp);
		websDone(wp);
		fclose(f);
		return OK;
	}
    return web_return_text_fmt(wp, HTTP_CODE_BAD_REQUEST, "can not open url path:%s", jst_html_dir(web_app, filename));
}


int web_html_jst_init(void)
{
	websDefineJst("jst_html_load", jst_html_load);
	websDefineJst("jst_html_text", jst_html_text);
	websFormDefine("html_load", jst_load);
	return 0;
}



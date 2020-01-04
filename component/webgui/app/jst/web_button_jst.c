//#include "zebra.h"
#include "zebra.h"
#include "module.h"
#include "memory.h"
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



static struct web_button_cb *web_btn_cbtbl[WEB_BTN_CB_MAX];

int web_button_add_hook(char *action, char *btnid, int (*cb)(void *, void *),
		void *p)
{
	int i = 0;
	web_assert(action);
	web_assert(btnid);
	web_assert(cb);
	for (i = 0; i < WEB_BTN_CB_MAX; i++)
	{
		if (web_btn_cbtbl[i] == NULL)
		{
			web_btn_cbtbl[i] = XMALLOC(MTYPE_WEB_ROUTE,
					sizeof(struct web_button_cb));
			memset(web_btn_cbtbl[i], 0, sizeof(struct web_button_cb));
			strncpy(web_btn_cbtbl[i]->action, action, MIN(sizeof(web_btn_cbtbl[i]->action), strlen(action)));
			strncpy(web_btn_cbtbl[i]->ID, btnid, MIN(sizeof(web_btn_cbtbl[i]->ID), strlen(btnid)));
			web_btn_cbtbl[i]->btn_cb = cb;
			web_btn_cbtbl[i]->pVoid = p;
			return OK;
		}
	}
	return ERROR;
}

int web_button_del_hook(char *action, char *btnid)
{
	int i = 0;
	char iaction[WEB_ACTION_NAME_MAX];
	char iID[WEB_ACTION_NAME_MAX];
	web_assert(action);
	web_assert(btnid);
	memset(iaction, 0, sizeof(iaction));
	memset(iID, 0, sizeof(iID));
	memcpy(iaction, action, MIN(sizeof(iaction), strlen(action)));
	memcpy(iID, btnid, MIN(sizeof(iID), strlen(btnid)));
	for (i = 0; i < WEB_BTN_CB_MAX; i++)
	{
		if (web_btn_cbtbl[i] != NULL)
		{
			if (memcmp(web_btn_cbtbl[i]->action, iaction, sizeof(iaction)) == 0
					&& memcmp(web_btn_cbtbl[i]->ID, iID , sizeof(iID)) == 0)
			{
				web_btn_cbtbl[i]->btn_cb = NULL;
				XFREE(MTYPE_WEB_ROUTE, web_btn_cbtbl[i]);
				web_btn_cbtbl[i] = NULL;
				return OK;
			}
		}
	}
	return ERROR;
}

static int web_button_call_hook(char *action, char *btnid, Webs *wp)
{
	int i = 0;
	char iaction[WEB_ACTION_NAME_MAX];
	char iID[WEB_ACTION_NAME_MAX];
	web_assert(action);
	web_assert(btnid);
	memset(iaction, 0, sizeof(iaction));
	memset(iID, 0, sizeof(iID));
	memcpy(iaction, action, MIN(sizeof(iaction), strlen(action)));
	memcpy(iID, btnid, MIN(sizeof(iID), strlen(btnid)));
	for (i = 0; i < WEB_BTN_CB_MAX; i++)
	{
		if (web_btn_cbtbl[i] != NULL)
		{
			if (strncasecmp(web_btn_cbtbl[i]->action, iaction, sizeof(iaction)) == 0
					&& strncasecmp(web_btn_cbtbl[i]->ID, iID , sizeof(iID)) == 0
					&& web_btn_cbtbl[i]->btn_cb)
			{
				return (web_btn_cbtbl[i]->btn_cb)(wp, web_btn_cbtbl[i]->pVoid);
			}
		}
	}
	return ERROR;
}

static int jst_button_onclick(int eid, webs_t wp, int argc, char **argv)
{
	int i = 0;
	for (i = 0; i < argc; i++)
	{
		if (argv[i])
			printf("%s: %d %s\r\n", __func__, i, argv[i]);
	}
	return 0;
}

static int jst_button(int eid, webs_t wp, int argc, char **argv)
{
	//char *date = websGetDateString(NULL);
	websWrite(wp, "%s", "admin");
	return 0;
}

static int web_button_onclick(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char action[64];
	char ID[64];
	memset(action, 0, sizeof(action));
	memset(ID, 0, sizeof(ID));
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(action, strval);
	//printf("%s: ACTION=%s\r\n", __func__, strval);
	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(ID, strval);
/*
	printf("%s: ID=%s\r\n", __func__, strval);

	if (path)
		printf("%s: path=%s\r\n", __func__, path);
	if (query)
		printf("%s: query=%s\r\n", __func__, query);
*/

	if (web_button_call_hook(action, ID, wp) == OK)
	{
		return OK;//web_return_text_plain(wp, OK);
	}
	return web_return_text_plain(wp, ERROR);
}


static int web_progress(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char action[64];
	char ID[64];
	memset(action, 0, sizeof(action));
	memset(ID, 0, sizeof(ID));
	strval = webs_get_var(wp, T("VIEW"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(action, strval);
	//printf("%s: VIEW=%s\r\n", __func__, strval);
	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(ID, strval);
/*
	printf("%s: ID=%s\r\n", __func__, strval);

	if (path)
		printf("%s: path=%s\r\n", __func__, path);
	if (query)
		printf("%s: query=%s\r\n", __func__, query);
*/

	if (web_button_call_hook(action, ID, wp) == OK)
	{
		return web_return_text_plain(wp, OK);
	}

	return web_return_text_plain(wp, ERROR);
}

int web_button_cb_init(void)
{
	memset(web_btn_cbtbl, 0, sizeof(web_btn_cbtbl));
	return OK;
}

int web_button_jst_init(void)
{

	websDefineJst("jst_button", jst_button);
	websDefineJst("jst_button_onclick", jst_button_onclick);
	websFormDefine("button_onclick", web_button_onclick);
	websFormDefine("btnClick", web_button_onclick);
	websFormDefine("progress_view", web_progress);
	return 0;
}

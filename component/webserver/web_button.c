//#include "zpl_include.h"
#include "zpl_include.h"
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



static struct web_button_cb *web_btn_cbtbl[WEB_BTN_CB_MAX];

#if ME_GOAHEAD_UPLOAD
static struct web_upload_cb *web_upload_cbtbl[WEB_UPLOAD_CB_MAX];
static struct web_download_cb *web_download_cbtbl[WEB_UPLOAD_CB_MAX];
#endif


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
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ACTION Value");
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(action, strval);

	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get BTNID Value");
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(ID, strval);
	if(WEB_IS_DEBUG(MSG) && WEB_IS_DEBUG(DETAIL))
		zlog_debug(MODULE_WEB, "web Get ACTION=%s BTNID=%s", action, ID);
	if (web_button_call_hook(action, ID, wp) == OK)
	{
		return OK;
	}
	return web_return_text_plain(wp, ERROR);
}




#if ME_GOAHEAD_UPLOAD
int web_upload_add_hook(char *form, char *id, int (*cb)(void *, void *, void *),
		void *p)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] == NULL)
		{
			web_upload_cbtbl[i] = XMALLOC(MTYPE_WEB_ROUTE,
					sizeof(struct web_upload_cb));
			strcpy(web_upload_cbtbl[i]->form, form);
			strcpy(web_upload_cbtbl[i]->ID, id);
			web_upload_cbtbl[i]->upload_cb = cb;
			web_upload_cbtbl[i]->pVoid = p;
			return OK;
		}
	}
	return ERROR;
}

int web_upload_del_hook(char *form, char *id)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] != NULL)
		{
			if (strcmp(web_upload_cbtbl[i]->form, form) == 0
					&& strcmp(web_upload_cbtbl[i]->ID, id) == 0)
			{
				web_upload_cbtbl[i]->upload_cb = NULL;
				XFREE(MTYPE_WEB_ROUTE, web_upload_cbtbl[i]);
				web_upload_cbtbl[i] = NULL;
				return OK;
			}
		}
	}
	return ERROR;
}

int web_upload_call_hook(char *form, char *id, Webs *wp, WebsUpload *up)
{
	int i = 0;
	web_assert(wp);
	web_assert(up);
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] != NULL)
		{
			if (strcmp(web_upload_cbtbl[i]->form, form) == 0
					&& (id==NULL || (id && strcmp(web_upload_cbtbl[i]->ID, id) == 0))
					&& web_upload_cbtbl[i]->upload_cb)
			{
				return (web_upload_cbtbl[i]->upload_cb)(wp, up, web_upload_cbtbl[i]->pVoid);
			}
		}
	}
	return ERROR;
}

int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **))
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] == NULL)
		{
			web_download_cbtbl[i] = XMALLOC(MTYPE_WEB_ROUTE,
					sizeof(struct web_upload_cb));
			strcpy(web_download_cbtbl[i]->form, form);
			strcpy(web_download_cbtbl[i]->ID, id);
			web_download_cbtbl[i]->download_cb = cb;
			//web_download_cbtbl[i]->pVoid = p;
			return OK;
		}
	}
	return ERROR;
}

int web_download_del_hook(char *form, char *id)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] != NULL)
		{
			if (strcmp(web_download_cbtbl[i]->form, form) == 0
					&& strcmp(web_download_cbtbl[i]->ID, id) == 0)
			{
				web_download_cbtbl[i]->download_cb = NULL;
				XFREE(MTYPE_WEB_ROUTE, web_download_cbtbl[i]);
				web_download_cbtbl[i] = NULL;
				return OK;
			}
		}
	}
	return ERROR;
}

int web_download_call_hook(char *form, char *id, Webs *wp, char **filename)
{
	int i = 0;
	web_assert(wp);
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] != NULL)
		{
			if (strcmp(web_download_cbtbl[i]->form, form) == 0
					&& (id==NULL || (id && strcmp(web_download_cbtbl[i]->ID, id) == 0))
					&& web_download_cbtbl[i]->download_cb)
			{
				return (web_download_cbtbl[i]->download_cb)(wp, filename);
			}
		}
	}
	return ERROR;
}
#endif



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
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get VIEW Value");
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(action, strval);

	strval = webs_get_var(wp, T("ID"), T(""));
	if (NULL == strval)
	{
		if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
			zlog_debug(MODULE_WEB, "Can not Get ID Value");
		return web_return_text_plain(wp, ERROR);
	}
	strcpy(ID, strval);
	if(WEB_IS_DEBUG(MSG)&&WEB_IS_DEBUG(DETAIL))
		zlog_debug(MODULE_WEB, "web Get VIEW=%s ID=%s", action, ID);
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

#if ME_GOAHEAD_UPLOAD
int web_updownload_cb_init(void)
{
	memset(web_upload_cbtbl, 0, sizeof(web_upload_cbtbl));
	memset(web_download_cbtbl, 0, sizeof(web_download_cbtbl));
	return OK;
}
#endif

int web_button_init(void)
{
	websFormDefine("button", web_button_onclick);
	websFormDefine("button_onclick", web_button_onclick);
	websFormDefine("btnClick", web_button_onclick);
	websFormDefine("progress_view", web_progress);
	return 0;
}

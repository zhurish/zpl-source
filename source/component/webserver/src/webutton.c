#define HAS_BOOL 1
#include "src/goahead.h"
#include "src/webutil.h"


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
			web_btn_cbtbl[i] = walloc(sizeof(struct web_button_cb));
			memset(web_btn_cbtbl[i], 0, sizeof(struct web_button_cb));
			strncpy(web_btn_cbtbl[i]->action, action, MIN(sizeof(web_btn_cbtbl[i]->action), strlen(action)));
			strncpy(web_btn_cbtbl[i]->ID, btnid, MIN(sizeof(web_btn_cbtbl[i]->ID), strlen(btnid)));
			web_btn_cbtbl[i]->btn_cb = cb;
			web_btn_cbtbl[i]->pVoid = p;
			return 0;
		}
	}
	return -1;
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
				wfree(web_btn_cbtbl[i]);
				web_btn_cbtbl[i] = NULL;
				return 0;
			}
		}
	}
	return -1;
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
	return -1;
}


static int web_button_onclick(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char action[64];
	char ID[64];
	memset(action, 0, sizeof(action));
	memset(ID, 0, sizeof(ID));
#if ME_GOAHEAD_JSON
	cJSON *root = NULL;
#endif
	if(wp->flags & WEBS_FORM)
	{
		strval = webs_get_var(wp, T("button-action"), T(""));
		if (NULL == strval)
		{
			web_error( "Can not Get button-action Value");
			return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find button-action");
		}
		strcpy(action, strval);
		strval = webs_get_var(wp, T("button-ID"), T(""));
		if (NULL == strval)
		{
			web_error( "Can not Get button-ID Value");
			return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find button-ID");
		}
		strcpy(ID, strval);
	}
#if ME_GOAHEAD_JSON
	if(wp->flags & WEBS_JSON)
	{
		root = websGetJsonVar(wp);
		if(root == NULL)
		{
			strval = cJSON_GetStringValue(root, "button-action");
			if (NULL == strval)
			{
				web_error( "Can not Get button-action Value");
				return web_return_application_json_fmt(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find button-action");
			}
			strval = cJSON_GetStringValue(root, "button-ID");
			if (NULL == strval)
			{
				web_error( "Can not Get button-ID Value");
				return web_return_application_json_fmt(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find button-ID");
			}
		}
	}
#endif

	if (web_button_call_hook(action, ID, wp) == 0)
	{
		return 0;
	}
	if(wp->flags & WEBS_FORM)	
		return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find acton hander");
#if ME_GOAHEAD_JSON
	else if(wp->flags & WEBS_JSON)
		return web_return_application_json_fmt(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find acton hander");
#endif
	return web_return_text_plain(wp, HTTP_CODE_BAD_REQUEST, "button onclick from can not find acton hander");
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
			web_upload_cbtbl[i] = walloc(sizeof(struct web_upload_cb));
			strcpy(web_upload_cbtbl[i]->form, form);
			strcpy(web_upload_cbtbl[i]->ID, id);
			web_upload_cbtbl[i]->upload_cb = cb;
			web_upload_cbtbl[i]->pVoid = p;
			return 0;
		}
	}
	return -1;
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
				wfree( web_upload_cbtbl[i]);
				web_upload_cbtbl[i] = NULL;
				return 0;
			}
		}
	}
	return -1;
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
	return -1;
}

int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **))
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] == NULL)
		{
			web_download_cbtbl[i] = walloc(sizeof(struct web_upload_cb));
			strcpy(web_download_cbtbl[i]->form, form);
			strcpy(web_download_cbtbl[i]->ID, id);
			web_download_cbtbl[i]->download_cb = cb;
			//web_download_cbtbl[i]->pVoid = p;
			return 0;
		}
	}
	return -1;
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
				wfree( web_download_cbtbl[i]);
				web_download_cbtbl[i] = NULL;
				return 0;
			}
		}
	}
	return -1;
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
	return -1;
}
#endif




int web_button_onclick_init(void)
{
	memset(web_btn_cbtbl, 0, sizeof(web_btn_cbtbl));
	#if ME_GOAHEAD_UPLOAD
	memset(web_upload_cbtbl, 0, sizeof(web_upload_cbtbl));
	memset(web_download_cbtbl, 0, sizeof(web_download_cbtbl));	
	#endif
	return 0;
}

int web_button_init(void)
{
	websFormDefine("button_onclick", web_button_onclick);
	return 0;
}

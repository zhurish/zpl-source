/*
    action.c -- Action handler

    This module implements the /action handler. It is a simple binding of URIs to C functions.
    This enables a very high performance implementation with easy parsing and decoding of query
    strings and posted data.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*********************************** Includes *********************************/

#include    "goahead.h"

/************************************ Locals **********************************/

static WebsHash actionTable = -1;            /* Symbol table for actions */
#ifdef WEBS_PRIVATE_MAX
static websAppPrivate_t *websPrivateTbl[WEBS_PRIVATE_MAX] = {NULL};
#endif
/************************************* Code ***********************************/
/*
    Process an action request. Returns 1 always to indicate it handled the URL
    Return true to indicate the request was handled, even for errors.
 */
static bool actionHandler(Webs *wp)
{
    WebsKey     *sp;
    char        actionBuf[ME_GOAHEAD_LIMIT_URI + 1];
    char        *cp, *actionName;
    WebsAction  fn;

    web_assert(websValid(wp));
    web_assert(actionTable >= 0);

    /*
        Extract the action name
     */
    scopy(actionBuf, sizeof(actionBuf), wp->path);
    if ((actionName = strchr(&actionBuf[1], '/')) == NULL) {
        websError(wp, HTTP_CODE_NOT_FOUND, "Missing action name");
        return 1;
    }
    actionName++;
    if ((cp = strchr(actionName, '/')) != NULL) {
        *cp = '\0';
    }
    /*
        Lookup the C action function first and then try tcl (no javascript support yet).
     */
    sp = hashLookup(actionTable, actionName);
    if (sp == NULL) {
        websError(wp, HTTP_CODE_NOT_FOUND, "Action %s is not defined", actionName);
    } else {
        fn = (WebsAction) sp->content.value.symbol;
        web_assert(fn);
        if (fn) {
#if ME_GOAHEAD_LEGACY
            (*((WebsProc) fn))((void*) wp, actionName, wp->query);
#else
            (*fn)((void*) wp);
#endif
        }
    }
    return 1;
}


/*
    Define a function in the "action" map space
 */
PUBLIC int websDefineAction(cchar *name, void *fn)
{
    web_assert(name && *name);
    web_assert(fn);

    if (fn == NULL) {
        return -1;
    }
    hashEnter(actionTable, (char*) name, valueSymbol(fn), 0);
    return 0;
}


static void closeAction(void)
{
    if (actionTable != -1) {
        hashFree(actionTable);
        actionTable = -1;
    }
}


PUBLIC void websActionOpen(void)
{
    actionTable = hashCreate(WEBS_HASH_INIT);
    websDefineHandler("action", 0, actionHandler, closeAction, 0);
}


#if ME_GOAHEAD_LEGACY
/*
    Don't use these routes. Use websWriteHeaders, websEndHeaders instead.

    Write a webs header. This is a convenience routine to write a common header for an action back to the browser.
 */
PUBLIC void websHeader(Webs *wp)
{
    web_assert(websValid(wp));

    websWriteHeaders(wp, -1, 0);
    websWriteEndHeaders(wp);
    websWrite(wp, "<html>\n");
}


PUBLIC void websFooter(Webs *wp)
{
	web_assert(websValid(wp));
    websWrite(wp, "</html>\n");
}
#endif

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */
#ifdef WEBS_PRIVATE_MAX
PUBLIC void websAppPrivateOpen(void)
{
	int i = 0, j = 0;
	for(i = 0; i < WEBS_PRIVATE_MAX; i++)
	{
		if(websPrivateTbl[i])
		{
			for(j = 0; j < WEBS_PRIVATE_DATA_MAX; j++)
			{
				if(websPrivateTbl[i]->private_free[j])
				{
					(websPrivateTbl[i]->private_free[j])(websPrivateTbl[i]->private_data[j]);
				}
				if(websPrivateTbl[i]->private_data[j])
				{
					wfree(websPrivateTbl[i]->private_data[j]);
					websPrivateTbl[i]->private_data[j] = NULL;
				}
				websPrivateTbl[i]->private_num[j] = 0;
				websPrivateTbl[i]->private_size[j] = 0;
			}
			websPrivateTbl[i]->wid = -1;
			wfree(websPrivateTbl[i]);
			websPrivateTbl[i] = NULL;
		}
	}
}

PUBLIC void websAppPrivateClose(void)
{
	websAppPrivateOpen();
}

PUBLIC websAppPrivate_t *websAppPrivateGet(int wid)
{
	int i = 0;
	for(i = 0; i < WEBS_PRIVATE_MAX; i++)
	{
		if(websPrivateTbl[i] && websPrivateTbl[i]->wid == wid)
		{
			return websPrivateTbl[i];
		}
	}
	return NULL;
}


PUBLIC int websAppPrivateAlloc(int wid)
{
	int i = 0;
	for(i = 0; i < WEBS_PRIVATE_MAX; i++)
	{
		if(websPrivateTbl[i] && websPrivateTbl[i]->wid == wid)
		{
			//printf("=======================%s========get==============wid=%d\r\n", __func__, wid);
			return 0;
		}
	}
	for(i = 0; i < WEBS_PRIVATE_MAX; i++)
	{
		if(!websPrivateTbl[i] ||( websPrivateTbl[i] && websPrivateTbl[i]->wid == -1))
		{
			if(websPrivateTbl[i])
			{
				//printf("=======================%s========have null==================wid=%d\r\n", __func__, wid);
				memset(websPrivateTbl[i], 0, sizeof(websAppPrivate_t));
				websPrivateTbl[i]->wid = wid;
				return 0;
			}
			else
			{
				websPrivateTbl[i] = walloc(sizeof(websAppPrivate_t));
				if(websPrivateTbl[i])
				{
					//printf("=======================%s==========alloc===========wid=%d\r\n", __func__, wid);
					memset(websPrivateTbl[i], 0, sizeof(websAppPrivate_t));
					websPrivateTbl[i]->wid = wid;
					return 0;
				}
			}
		}
	}
	return -1;
}

PUBLIC int websAppPrivateFree(int wid)
{
	int i = 0, j = 0;
	//printf("=======================%s============================wid=%d\r\n", __func__, wid);
	for(i = 0; i < WEBS_PRIVATE_MAX; i++)
	{
		if(websPrivateTbl[i] && websPrivateTbl[i]->wid == wid)
		{
			for(j = 0; j < WEBS_PRIVATE_DATA_MAX; j++)
			{
				if(websPrivateTbl[i]->private_free[j])
				{
					(websPrivateTbl[i]->private_free[j])(websPrivateTbl[i]->private_data[j]);
				}
				if(websPrivateTbl[i]->private_data[j])
				{
					//printf("=======================%s============================wid=%d idex=%d\r\n", __func__, wid, j);
					wfree(websPrivateTbl[i]->private_data[j]);
					websPrivateTbl[i]->private_data[j] = NULL;
				}
				websPrivateTbl[i]->private_num[j] = 0;
				websPrivateTbl[i]->private_size[j] = 0;
			}
			websPrivateTbl[i]->wid = -1;
			wfree(websPrivateTbl[i]);
			websPrivateTbl[i] = NULL;
			return 0;
		}
	}
	return -1;
}
#endif

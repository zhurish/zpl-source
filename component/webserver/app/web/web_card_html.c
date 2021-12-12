/*
 * web_card_html.c
 *
 *  Created on: 2019年10月11日
 *      Author: DELL
 */

#define HAS_BOOL 1
#include "zpl_include.h"

#include "module.h"
#include "memory.h"
#include "zassert.h"
#include "command.h"
#include "prefix.h"
#include "host.h"
#include "log.h"
#include "vty.h"
#include "vty_user.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef ZPL_APP_MODULE
#include "application.h"

#ifdef APP_X5BA_MODULE
static int web_card_action(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if (strstr(strval, "GET"))
	{
		char cardNumber[128];
		memset(cardNumber, 0, sizeof(cardNumber));
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);
#ifdef ZPL_OPENWRT_UCI
		os_uci_get_string("userauth.db.cardid", cardNumber);
#endif
		websWrite(wp,
				"{\"response\":\"%s\",\"cardid\":\"%s\"}",
				"OK", cardNumber);

		websDone(wp);
		return OK;
	}
	return web_return_text_plain(wp, ERROR);
}
#endif
#endif

int web_card_app(void)
{
#ifdef ZPL_APP_MODULE
#ifdef APP_X5BA_MODULE
	websFormDefine("card", web_card_action);
#endif
#endif
	return 0;
}


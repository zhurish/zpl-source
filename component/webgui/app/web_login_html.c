/*
 * login.c
 *
 *  Created on: Mar 24, 2019
 *      Author: zhurish
 */

//#include "zebra.h"
#ifdef HAVE_CONFIG_H
#include "plconfig.h"
#endif /* HAVE_CONFIG_H */

#include "os_platform.h"

#include "module.h"
#include <netinet/in.h>
#include "memory.h"
#include "zassert.h"
#include "log.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_task.h"

#include "goahead.h"
#include "webgui_app.h"

char * web_encoded_password(char *username, char *password, char *realm, char *cipher)
{
    char *encodedPassword = websMD5(sfmt("%s:%s:%s", username, realm, password));

    if (smatch(cipher, "md5")) {//blowfish md5
        encodedPassword = websMD5(sfmt("%s:%s:%s", username, realm, password));
    } else {
        /* This uses the more secure blowfish cipher */
        encodedPassword = websMakePassword(sfmt("%s:%s:%s", username, realm, password), 16, 128);
    }
    return encodedPassword;
}

static int web_login_auth(Webs *wp, char *path, char *query)
{
    char *strval = NULL;
    char g_test_name[32];
    char g_test_pass[32];
    memset(g_test_name, 0, sizeof(g_test_name));
    memset(g_test_pass, 0, sizeof(g_test_pass));
    strval = websGetVar(wp, T("username"), T(""));
    /* websGetVar()可以从页面取出文本框内的数据 */
    if (NULL != strval)
    {
        /* 将从页面上取出的数据存入全局变量中 */
        strcpy(g_test_name, strval);
    }

    strval = websGetVar(wp, T("password"), T(""));
    if (NULL != strval)
    {

        strcpy(g_test_pass, strval);
    }
    printf("%s: %s %s\r\n", __func__, g_test_name, g_test_pass);
    websRedirect(wp, "/html/main.html");
/*    websHeader(wp);
    websFooter(wp);
    websDone(wp);*/
    return 0;
}

int web_login_app(char *actionname)
{
	websFormDefine("login", web_login_auth);
	return 0;
}

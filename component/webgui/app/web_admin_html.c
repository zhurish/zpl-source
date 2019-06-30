/*
 * web_admin.c
 *
 *  Created on: Apr 13, 2019
 *      Author: zhurish
 */




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

static int web_admin_user(Webs *wp, char *path, char *query)
{
    char *strval = NULL;
    char ssh_keys[128];
    memset(ssh_keys, 0, sizeof(ssh_keys));
    strval = websGetVar(wp, T("cbid.system.username"), T(""));
    /* websGetVar()可以从页面取出文本框内的数据 */
    if (NULL != strval)
    {
        /* 将从页面上取出的数据存入全局变量中 */
        strcpy(ssh_keys, strval);
    }
    printf("%s: %s(%s->%s)\r\n", __func__, ssh_keys, path, query);
	websHeader(wp);
	websFooter(wp);
	websDone(wp);
    return 0;
}

static int web_admin_change_password(Webs *wp, char *path, char *query)
{
    char *strval = NULL;
    char ssh_keys[128];
    memset(ssh_keys, 0, sizeof(ssh_keys));
    strval = websGetVar(wp, T("cbid.dropbear.ssh.keys.data"), T(""));
    /* websGetVar()可以从页面取出文本框内的数据 */
    if (NULL != strval)
    {
        /* 将从页面上取出的数据存入全局变量中 */
        strcpy(ssh_keys, strval);
    }
    printf("%s: %s\r\n", __func__, ssh_keys);
	websHeader(wp);
	websFooter(wp);
	websDone(wp);
    return 0;
}

static int web_admin_ssh_keys(Webs *wp, char *path, char *query)
{
    char *strval = NULL;
    char ssh_keys[128];
    memset(ssh_keys, 0, sizeof(ssh_keys));
    strval = websGetVar(wp, T("cbid.dropbear.ssh.keys.data"), T(""));
    /* websGetVar()可以从页面取出文本框内的数据 */
    if (NULL != strval)
    {
        /* 将从页面上取出的数据存入全局变量中 */
        strcpy(ssh_keys, strval);
    }
    printf("%s: %s\r\n", __func__, ssh_keys);
	websHeader(wp);
	websFooter(wp);
	websDone(wp);
    return 0;
}

int web_admin_app(char *actionname)
{
	websFormDefine("admin-user", web_admin_user);
	websFormDefine("admin-password", web_admin_change_password);
	websFormDefine("admin-sshkeys", web_admin_ssh_keys);
	return 0;
}

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


static int jst_button_onclick(int eid, webs_t wp, int argc, char **argv)
{
	int i = 0;
	for(i = 0; i < argc; i++)
	{
		if(argv[i])
			printf("%s: %d %s\r\n", __func__, i, argv[i]);
	}
    return 0;
}

int web_jst_onclick_init(void)
{
	websDefineJst("jst_button_onclick", jst_button_onclick);
	return 0;
}

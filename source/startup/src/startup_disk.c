/*
 * startup_disk.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "startup_disk.h"




static int zpl_base_dir_init(void)
{
	if(access(BASE_DIR, F_OK) != 0)
		mkdir(BASE_DIR, 0644);

	if(access(REAL_SYSLOGDIR, F_OK) != 0)
		mkdir(REAL_SYSLOGDIR, 0644);		// /log

	if(access(SYSCONFDIR, F_OK) != 0)
		mkdir(SYSCONFDIR, 0644);		// /etc/app
	if(access(SYSLIBDIR, F_OK) != 0)
		mkdir(SYSLIBDIR, 0644);			// /lib
	if(access(SYSSBINDIR, F_OK) != 0)
		mkdir(SYSSBINDIR, 0644);		// /sbin
	if(access(SYSBINDIR, F_OK) != 0)
		mkdir(SYSBINDIR, 0644);			// /bin
	if(access(SYSRUNDIR, F_OK) != 0)
		mkdir(SYSRUNDIR, 0644);			// /run
	if(access(SYSLOGDIR, F_OK) != 0)
		mkdir(SYSLOGDIR, 0644);			// /log
	if(access(SYSVARDIR, F_OK) != 0)
		mkdir(SYSVARDIR, 0644);			// /var
	if(access(SYSTMPDIR, F_OK) != 0)
		mkdir(SYSTMPDIR, 0644);			// /tmp

	if(access(SYSWWWDIR, F_OK) != 0)
		mkdir(SYSWWWDIR, 0644);			// /www
	if(access(SYSWWWCACHEDIR, F_OK) != 0)
		mkdir(SYSWWWCACHEDIR, 0644);			// /www/cache

	if(access(SYSTFTPBOOTDIR, F_OK) != 0)
		mkdir(SYSTFTPBOOTDIR, 0644);			// /tftpboot

	if(access(SYSUPLOADDIR, F_OK) != 0)
		mkdir(SYSUPLOADDIR, 0644);			// /tftpboot

	if(access(BASE_DIR"/img", F_OK) != 0)
		mkdir(BASE_DIR"/img", 0644);

	return 0;
}

static int zpl_base_dir_load(void)
{
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONF_REAL_DIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
	super_system("cp -arf " SYSCONF_REAL_DIR"/*" " " SYSCONFDIR"/");

#ifdef ZPL_WEBGUI_MODULE
	if(access(REAL_SYSWWWDIR"/build.tar.gz", F_OK) == 0)
	{
		//super_system("cp -arf " RSYSWWWDIR"/build.tar.gz /tmp/");
		//super_system("cd /tmp/; tar -zxvf build.tar.gz");
		if(access(SYSWWWDIR"/index.html", F_OK) == 0)
		{
			super_system("tar -zxvf  "REAL_SYSWWWDIR"/build.tar.gz -C /tmp");
			super_system("cp -arf /tmp/build/* " SYSWWWDIR"/");
			super_system("rm -rf /tmp/build ");
		}
	}
	else
	{
		if(access(SYSWWWDIR"/index.html", F_OK) != 0)
			super_system("cp -arf " REAL_SYSWWWDIR"/*" " " SYSWWWDIR"/");
	}
#endif

	return 0;
}


int zpl_base_env_init(void)
{
	lstLibInit();
	os_nvram_env_init();
	zpl_base_dir_init();
	return OK;
}

int zpl_base_env_load(void)
{
	zpl_base_dir_load();
	return OK;
}

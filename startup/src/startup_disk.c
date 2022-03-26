/*
 * startup_disk.c
 *
 *  Created on: Apr 30, 2017
 *      Author: zhurish
 */
#include "os_include.h"
#include <zpl_include.h>
#include "module.h"
#include "startup_disk.h"




static int zpl_base_dir_init(void)
{
	if(access(BASE_DIR, F_OK) != 0)
		mkdir(BASE_DIR, 0644);

	if(access(RSYSLOGDIR, F_OK) != 0)
		mkdir(RSYSLOGDIR, 0644);		// /etc

	if(access(PLSYSCONFDIR, F_OK) != 0)
		mkdir(PLSYSCONFDIR, 0644);		// /etc
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
#ifdef ZPL_BUILD_OS_OPENWRT
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONF_REAL_DIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
#else
#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	if(access(DEFAULT_CONFIG_FILE, F_OK) != 0)
		super_system("cp -af " SYSCONFDIR"/default-config.cfg  " DEFAULT_CONFIG_FILE);
	super_system("cp -arf " SYSCONFDIR"/*" " " PLSYSCONFDIR"/");
#endif
#endif

#ifdef ZPL_WEBGUI_MODULE
	if(access(RSYSWWWDIR"/build.tar.gz", F_OK) == 0)
	{
		//super_system("cp -arf " RSYSWWWDIR"/build.tar.gz /tmp/");
		//super_system("cd /tmp/; tar -zxvf build.tar.gz");
		if(access(SYSWWWDIR"/index.html", F_OK) != 0)
		{
			super_system("tar -zxvf  "RSYSWWWDIR"/build.tar.gz -C /tmp");
			super_system("cp -arf /tmp/build/* " SYSWWWDIR"/");
			super_system("rm -rf /tmp/build ");
		}
	}
	else
	{
		if(access(SYSWWWDIR"/index.html", F_OK) != 0)
			super_system("cp -arf " RSYSWWWDIR"/*" " " SYSWWWDIR"/");
	}
#endif

#ifdef ZPL_BUILD_OS_OPENWRT
#ifdef APP_X5BA_MODULE
	if(access("/etc/config/product", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/product", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/product  /etc/config/");
	}
	if(access("/etc/config/voipconfig", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/voipconfig", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/voipconfig  /etc/config/");
	}
	if(access("/etc/config/openconfig", F_OK) != 0)
	{
		if(access(SYSCONFDIR"/openconfig", F_OK) == 0)
			super_system("cp -af " SYSCONFDIR"/openconfig  /etc/config/");
	}
#endif
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

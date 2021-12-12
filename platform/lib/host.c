/*
 * _global_host.c
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"

struct zpl_host _global_host;


int host_sysconfig_sync()
{
#ifdef ZPL_BUILD_OS_OPENWRT
#else
	//cp -arf /app/etc/* /tmp/app/etc/
	//super_system("cp -a "SYSCONFDIR"/default-config.cfg " SYSCONF_REAL_DIR"/default-config.cfg");
#endif
	return OK;
}


int host_config_loading(char *config)
{
	fprintf(stdout, "==================os_load_config %s\r\n",config);
	vty_load_config(config);
	fprintf(stdout, "++++++++++++++++++os_load_config %s\r\n",config);
	fflush(stdout);
	//signal_init(NULL, array_size(os_signals), os_signals);
	//os_task_give_broadcast();
	return OK;
}

zpl_bool host_config_load_waitting(void)
{
	while(_global_host.load != LOAD_DONE)
	{
		os_sleep(1);
	}
	return zpl_true;
}

zpl_bool host_config_load_done(void)
{
	if(_global_host.load == LOAD_DONE)
	{
		return zpl_true;
	}
	return zpl_false;
}

int host_config_init(zpl_char *motd)
{
	os_memset(&_global_host, 0, sizeof(_global_host));
	/* Default host value settings. */
	_global_host.name = NULL;
	_global_host.logfile = NULL;
	//XFREE (MTYPE_HOST
	_global_host.config = XSTRDUP(MTYPE_HOST, STARTUP_CONFIG_FILE);
	_global_host.default_config = XSTRDUP(MTYPE_HOST, DEFAULT_CONFIG_FILE);
	_global_host.factory_config = XSTRDUP(MTYPE_HOST, FACTORY_CONFIG_FILE);
	_global_host.lines = -1;
	_global_host.motd = motd; //default_motd;
	_global_host.motdfile = NULL;
	#ifdef ZPL_SHELL_MODULE
	_global_host.vty_timeout_val = VTY_TIMEOUT_DEFAULT;
	#endif
	/* Vty access-class command */
	_global_host.vty_accesslist_name = NULL;
	/* Vty access-calss for IPv6. */
	_global_host.vty_ipv6_accesslist_name = NULL;
	/* VTY server thread. */

	/* Current directory. */
	_global_host.vty_cwd = NULL;
	/* Configure lock. */
	_global_host.vty_config = 0;
	/* Login password check. */
	_global_host.no_password_check = 0;
	_global_host.mutx = os_mutex_init();
	_global_host.cli_mutx = os_mutex_init();
	if(access("/etc/.serial_no", F_OK) == 0)
	{
		memset(_global_host.serial, '\0', sizeof(_global_host.serial));
		os_read_file("/etc/.serial_no", _global_host.serial, sizeof(_global_host.serial));
		_global_host.serial[strlen(_global_host.serial)-1] = '\0';
		printf("=================%s================:(%s)\r\n", __func__,_global_host.serial);
	}
	if(access("/etc/.wan-mac", F_OK) == 0)
	{
		zpl_char tmp[64];
		memset(tmp, 0, sizeof(tmp));
		os_read_file("/etc/.wan-mac", tmp, sizeof(tmp));
		if(strlen(tmp) && strstr(tmp, ":"))
		{
			struct ethaddr ether;
			ether_aton_r (tmp, &ether);
			host_config_set_api(API_SET_SYSMAC_CMD, ether.octet);
		}
	}
	return OK;
}

int host_config_exit(void)
{
	if (_global_host.name)
	{
		XFREE(MTYPE_HOST, _global_host.name);
		_global_host.name = NULL;
	}
	if (_global_host.vty_accesslist_name)
	{
		XFREE(MTYPE_VTY, _global_host.vty_accesslist_name);
		_global_host.vty_accesslist_name = NULL;
	}
	if (_global_host.vty_ipv6_accesslist_name)
	{
		XFREE(MTYPE_VTY, _global_host.vty_ipv6_accesslist_name);
		_global_host.vty_ipv6_accesslist_name = NULL;
	}
	if (_global_host.logfile)
	{
		XFREE(MTYPE_HOST, _global_host.logfile);
		_global_host.logfile = NULL;
	}
	if (_global_host.motdfile)
	{
		XFREE(MTYPE_HOST, _global_host.motdfile);
		_global_host.motdfile = NULL;
	}
	if (_global_host.config)
	{
		XFREE(MTYPE_HOST, _global_host.config);
		_global_host.config = NULL;
	}
	if (_global_host.default_config)
	{
		XFREE(MTYPE_HOST, _global_host.default_config);
		_global_host.default_config = NULL;
	}
	if (_global_host.factory_config)
	{
		XFREE(MTYPE_HOST, _global_host.factory_config);
		_global_host.factory_config = NULL;
	}
	if (_global_host.cli_mutx)
	{
		os_mutex_exit(_global_host.cli_mutx);
		_global_host.cli_mutx = NULL;
	}
	if (_global_host.mutx)
	{
		os_mutex_exit(_global_host.mutx);
		_global_host.mutx = NULL;
	}
	return OK;
}



/* Set config filename.  Called from vty.c */
void
host_config_set (zpl_char *filename)
{
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);
	if (_global_host.config)
		XFREE(MTYPE_HOST, _global_host.config);
	_global_host.config = XSTRDUP(MTYPE_HOST, filename);
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
}

const char *
host_config_get (void)
{
	return _global_host.config;
}

const char *
host_name_get (void)
{
	return _global_host.name;
}

int
host_config_set_api (zpl_uint32 cmd, void *pVoid)
{
	int ret = ERROR;
	zpl_char *strValue = (zpl_char *)pVoid;
	zpl_uint32 *intValue = (zpl_uint32 *)pVoid;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_SET_HOSTNAME_CMD:
		if (_global_host.name)
			XFREE(MTYPE_HOST, _global_host.name);
		if(strValue)
			_global_host.name = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.name = NULL;
		ret = OK;
		break;
	case API_SET_DESC_CMD:
		if (_global_host.description)
			XFREE(MTYPE_HOST, _global_host.description);
		if(strValue)
			_global_host.description = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.description = NULL;
		ret = OK;
		break;
	case API_SET_LINES_CMD:
		_global_host.lines = (zpl_uint32)*intValue;
		ret = OK;
		break;
	case API_SET_LOGFILE_CMD:
		if (_global_host.logfile)
			XFREE(MTYPE_HOST, _global_host.logfile);
		if(strValue)
			_global_host.logfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.logfile = NULL;
		ret = OK;
		break;
	case API_SET_CONFIGFILE_CMD:
		if (_global_host.config)
			XFREE(MTYPE_HOST, _global_host.config);
		if(strValue)
			_global_host.config = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.config = NULL;
		ret = OK;
		break;
	case API_SET_ENCRYPT_CMD:
		_global_host.encrypt = (zpl_bool)*intValue;
		ret = OK;
		break;
	case API_SET_MOTD_CMD:
		if (_global_host.motd)
			XFREE(MTYPE_HOST, _global_host.motd);
		if(strValue)
			_global_host.motd = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.motd = NULL;
		ret = OK;
		break;
	case API_SET_MOTDFILE_CMD:
		if (_global_host.motdfile)
			XFREE(MTYPE_HOST, _global_host.motdfile);
		if(strValue)
			_global_host.motdfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.motdfile = NULL;
		ret = OK;
		break;
	case API_SET_VTY_TIMEOUT_CMD:
		_global_host.vty_timeout_val = (zpl_ulong)*intValue;
		ret = OK;
		break;
	case API_SET_ACCESS_CMD:
		if (_global_host.vty_accesslist_name)
			XFREE(MTYPE_HOST, _global_host.vty_accesslist_name);
		if(strValue)
			_global_host.vty_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.vty_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_IPV6ACCESS_CMD:
		if (_global_host.vty_ipv6_accesslist_name)
			XFREE(MTYPE_HOST, _global_host.vty_ipv6_accesslist_name);
		if(strValue)
			_global_host.vty_ipv6_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			_global_host.vty_ipv6_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_NOPASSCHK_CMD:
		_global_host.no_password_check = (zpl_bool)*intValue;
		ret = OK;
		break;
	case API_SET_SYSMAC_CMD:
		memcpy(_global_host.sysmac, strValue, 6);
		ret = OK;
		break;
	case API_SET_SERIAL_CMD:
		memset(_global_host.serial, 0, sizeof(_global_host.serial));
		if(strValue)
			strncpy(_global_host.serial, strValue, MIN(strlen(strValue), sizeof(_global_host.serial)));
		ret = OK;
		break;

	default:
		break;
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return ret;
}

int
host_config_get_api (zpl_uint32 cmd, void *pVoid)
{
	int ret = ERROR;
	zpl_char *strValue = (zpl_char *)pVoid;
	zpl_uint32 *intValue = (zpl_uint32 *)pVoid;
	if (_global_host.mutx)
		os_mutex_lock(_global_host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_GET_HOSTNAME_CMD:
		if (_global_host.name)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.name);
		}
		ret = OK;
		break;
	case API_GET_DESC_CMD:
		if (_global_host.description)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.description);
		}
		ret = OK;
		break;
	case API_GET_LINES_CMD:
		if(intValue)
			*intValue = (zpl_uint32)_global_host.lines;
		ret = OK;
		break;
	case API_GET_LOGFILE_CMD:
		if (_global_host.logfile)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.logfile);
		}
		ret = OK;
		break;
	case API_GET_CONFIGFILE_CMD:
		if (_global_host.config)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.config);
		}
		ret = OK;
		break;
	case API_GET_ENCRYPT_CMD:
		if(intValue)
			*intValue = (zpl_bool)_global_host.encrypt;
		ret = OK;
		break;
	case API_GET_MOTD_CMD:
		if (_global_host.motd)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.motd);
		}
		ret = OK;
		break;
	case API_GET_MOTDFILE_CMD:
		if (_global_host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_VTY_TIMEOUT_CMD:
		if(intValue)
			*intValue = (zpl_uint32)_global_host.vty_timeout_val;
		ret = OK;
		break;
	case API_GET_ACCESS_CMD:
		if (_global_host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_IPV6ACCESS_CMD:
		if (_global_host.vty_ipv6_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, _global_host.vty_ipv6_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_NOPASSCHK_CMD:
		if(intValue)
			*intValue = (zpl_bool)_global_host.no_password_check;
		ret = OK;
		break;
	case API_GET_SYSMAC_CMD:
		if (!str_isempty(_global_host.sysmac, sizeof(_global_host.sysmac)))
		{
			if(strValue)
				memcpy(strValue, _global_host.sysmac, 6);
		}
		ret = OK;
		break;
	case API_GET_SERIAL_CMD:
		if (!str_isempty(_global_host.serial, sizeof(_global_host.serial)))
		{
			if(strValue)
				os_strcpy(strValue, _global_host.serial);
		}
		ret = OK;
		break;

	default:
		break;
	}
	if (_global_host.mutx)
		os_mutex_unlock(_global_host.mutx);
	return ret;
}


static int host_system_cpu_get(struct host_system *host_system)
{
	FILE *f = NULL;
	zpl_char buf[1024];
	f = fopen("/proc/cpuinfo", "r");
	if (f)
	{
		zpl_char *s = NULL;
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			/* work backwards to ignore trailling isspace() */
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
#ifdef ZPL_BUILD_ARCH_X86
			if(strstr(buf, "processor"))
				host_system->process++;

			if(strstr(buf, "model name"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->model_name == NULL && s)
					host_system->model_name = strdup(s);
			}
			if(strstr(buf, "MHz"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->freq == 0.0 && s)
				{
					sscanf(s, "%lf", &host_system->freq);
				}
			}
#else
			if(strstr(buf, "processor"))
				host_system->process++;

			if(strstr(buf, "system"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->system_type == NULL && s)
					host_system->system_type = strdup((s));
			}
			if(strstr(buf, "model"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->cpu_model == NULL && s)
					host_system->cpu_model = strdup((s));
			}
			if(strstr(buf, "BogoMIPS"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->freq == 0.0 && s)
				{
					host_system->freq = atof(s);
					//sscanf(s, "%f", &host_system->freq);
				}
			}
			if(strstr(buf, "ASEs"))
			{
				s = strstr(buf, ":");
				s++;
				while(s && isspace((int) *(s)))
					s++;
				if(host_system->ase == NULL && s)
					host_system->ase = strdup((s));
			}
#endif
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}
	return OK;
}


static int host_system_mem_get(struct host_system *host_system)
{
/*	MemTotal:       11831640 kB
	MemFree:          932640 kB*/
	FILE *f = NULL;
	zpl_char buf[1024];
	f = fopen("/proc/meminfo", "r");
	if (f)
	{
		zpl_uint32 off = 0;
		zpl_char *s = NULL;
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			/* work backwards to ignore trailling isspace() */
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
			if(strstr(buf, "MemTotal"))
			{
				off = strcspn(buf, "0123456789");
				if(host_system->mem_total == 0 && off)
				{
					sscanf(buf + off, "%d", &host_system->mem_total);
				}
			}

			if(strstr(buf, "MemFree"))
			{
				off = strcspn(buf, "0123456789");
				if(host_system->mem_free == 0 && off)
				{
					sscanf(buf + off, "%d", &host_system->mem_free);
				}
			}
			memset(buf, 0, sizeof(buf));
		}
		fclose(f);
	}
	return OK;
}

static int host_system_information_free(struct host_system *host_system)
{
	host_system->process = 0;
	host_system->freq = 0.0;
#ifdef ZPL_BUILD_ARCH_X86
	if(host_system->model_name)
	{
		free(host_system->model_name);
		host_system->model_name = NULL;
	}
#else
	if(host_system->system_type)
	{
		free(host_system->system_type);
		host_system->system_type = NULL;
	}
	if(host_system->cpu_model)
	{
		free(host_system->cpu_model);
		host_system->cpu_model = NULL;
	}
	if(host_system->ase)
	{
		free(host_system->ase);
		host_system->ase = NULL;
	}
#endif
	host_system->mem_total = 0;
	host_system->mem_free = 0;
	host_system->mem_uses = 0;
	os_memset(&host_system->s_info, 0, sizeof(struct sysinfo));
	return OK;
}

int host_system_information_get(struct host_system *host_system)
{
	host_system_information_free(host_system);
	host_system_cpu_get(host_system);
	host_system_mem_get(host_system);
	sysinfo(&host_system->s_info);
	return OK;
}


#if 0
static zpl_ullong  scale(struct globals *g, zpl_ulong d)
{
	return ((zpl_ullong )d * g->mem_unit) >> G_unit_steps;
}
int free_main(int argc UNUSED_PARAM, zpl_char **argv IF_NOT_DESKTOP(UNUSED_PARAM))
{
	struct globals G;
	struct sysinfo info;
	zpl_ullong  cached;

#if ENABLE_DESKTOP
	G.unit_steps = 10;
	if (argv[1] && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'b':
			G.unit_steps = 0;
			break;
		case 'k': /* 2^10 */
			/* G.unit_steps = 10; - already is */
			break;
		case 'm': /* 2^(2*10) */
			G.unit_steps = 20;
			break;
		case 'g': /* 2^(3*10) */
			G.unit_steps = 30;
			break;
		default:
			bb_show_usage();
		}
	}
#endif
	printf("       %11s%11s%11s%11s%11s%11s\n"
	"Mem:   ",
		"total",
		"used",
		"free",
		"shared", "buffers", "cached" /* swap and total don't have these columns */
	);

	sysinfo(&info);
	/* Kernels prior to 2.4.x will return info.mem_unit==0, so cope... */
	G.mem_unit = (info.mem_unit ? info.mem_unit : 1);
	/* Extract cached from /proc/meminfo and convert to mem_units */
	cached = ((zpl_ullong ) parse_cached_kb() * 1024) / G.mem_unit;

#define FIELDS_6 "%11llu%11llu%11llu%11llu%11llu%11llu\n"
#define FIELDS_3 (FIELDS_6 + 3*6)
#define FIELDS_2 (FIELDS_6 + 4*6)

	printf(FIELDS_6,
		scale(&G, info.totalram),                //total
		scale(&G, info.totalram - info.freeram), //used
		scale(&G, info.freeram),                 //free
		scale(&G, info.sharedram),               //shared
		scale(&G, info.bufferram),               //buffers
		scale(&G, cached)                        //cached
	);
	/* Show alternate, more meaningful busy/free numbers by counting
	 * buffer cache as free memory. */
	printf("-/+ buffers/cache:");
	cached += info.freeram;
	cached += info.bufferram;
	printf(FIELDS_2,
		scale(&G, info.totalram - cached), //used
		scale(&G, cached)                  //free
	);
#if BB_MMU
	printf("Swap:  ");
	printf(FIELDS_3,
		scale(&G, info.totalswap),                 //total
		scale(&G, info.totalswap - info.freeswap), //used
		scale(&G, info.freeswap)                   //free
	);
#endif
	return EXIT_SUCCESS;
}
#endif
#ifdef ZPL_SHELL_MODULE
int show_host_system_information(struct host_system *host_system, struct vty *vty)
{
#ifdef ZPL_BUILD_ARCH_X86
	if(host_system->model_name)
	{
		vty_out(vty, " CPU Type      : %s%s", host_system->model_name, VTY_NEWLINE);
	}
#else
	if(host_system->system_type)
	{
		vty_out(vty, " CPU Type      : %s%s", host_system->system_type, VTY_NEWLINE);
	}
	if(host_system->cpu_model)
	{
		vty_out(vty, " CPU Model     : %s%s", host_system->cpu_model, VTY_NEWLINE);
	}
	if(host_system->ase)
	{
		vty_out(vty, " CPU ASE       : %s%s", host_system->ase, VTY_NEWLINE);
	}
#endif
	vty_out(vty, " Processor     : %d%s", host_system->process, VTY_NEWLINE);
	vty_out(vty, " CPU FREQ      : %.2f MHz%s", host_system->freq, VTY_NEWLINE);

	//KB >> 10
	//MB >> 20
	//GB >> 30
	host_system->mem_total = host_system->s_info.totalram >> 10;//               //total
	host_system->mem_uses = (host_system->s_info.totalram - host_system->s_info.freeram) >> 10; //used
	host_system->mem_free = host_system->s_info.freeram >> 10;                 //free

	vty_out(vty, " MEM Total     : %d KB%s", host_system->mem_total, VTY_NEWLINE);
	vty_out(vty, " MEM Free      : %d KB%s", host_system->mem_free, VTY_NEWLINE);
	vty_out(vty, " MEM Uses      : %d KB%s", host_system->mem_uses, VTY_NEWLINE);
	return OK;
}
#endif
/*
root@OpenWrt:/tmp/test# cat /proc/cpuinfo
system type             : MediaTek MT7688 ver:1 eco:2
machine                 : Mediatek MT7628AN evaluation board
processor               : 0
cpu model               : MIPS 24KEc V5.5
BogoMIPS                : 385.84
wait instruction        : yes
microsecond timers      : yes
tlb_entries             : 32
extra interrupt vector  : yes
hardware watchpoint     : yes, count: 4, address/irw mask: [0x0ffc, 0x0ffc, 0x0ffb, 0x0ffb]
isa                     : mips1 mips2 mips32r1 mips32r2
ASEs implemented        : mips16 dsp
shadow register sets    : 1
kscratch registers      : 0
package                 : 0
core                    : 0
VCED exceptions         : not available
VCEI exceptions         : not available

root@OpenWrt:/tmp/test#
root@OpenWrt:/tmp/test# cat /proc/meminfo
MemTotal:         122524 kB
MemFree:           64384 kB
MemAvailable:      56616 kB
Buffers:            4908 kB
Cached:            30800 kB
SwapCached:            0 kB
Active:            16280 kB
Inactive:          24908 kB
Active(anon):       5608 kB
Inactive(anon):     5716 kB
Active(file):      10672 kB
Inactive(file):    19192 kB
Unevictable:           0 kB
Mlocked:               0 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:          5520 kB
Mapped:             5940 kB
Shmem:              5844 kB
Slab:               9464 kB
SReclaimable:       3752 kB
SUnreclaim:         5712 kB
KernelStack:         472 kB
PageTables:          408 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:       61260 kB
Committed_AS:     101412 kB
VmallocTotal:    1048372 kB
VmallocUsed:           0 kB
VmallocChunk:          0 kB
root@OpenWrt:/tmp/test#
*/

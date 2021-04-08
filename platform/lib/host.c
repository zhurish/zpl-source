/*
 * host.c
 *
 *  Created on: Jan 1, 2018
 *      Author: zhurish
 */

#include "zebra.h"

#include "log.h"
#include "memory.h"
#include "thread.h"
#include "vector.h"
#include "version.h"
#include "workqueue.h"
#include "command.h"
#include "vty.h"
#include "host.h"
#include "prefix.h"
#include "vty_user.h"

struct host host;


int host_sysconfig_sync()
{
#ifdef PL_BUILD_OS_OPENWRT
#else
	//cp -arf /app/etc/* /tmp/app/etc/
	//super_system("cp -a "SYSCONFDIR"/default-config.cfg " SYSCONF_REAL_DIR"/default-config.cfg");
#endif
	return OK;
}


int host_config_init(ospl_char *motd)
{
	os_memset(&host, 0, sizeof(host));
	/* Default host value settings. */
	host.name = NULL;
	host.logfile = NULL;
	//XFREE (MTYPE_HOST
	host.config = XSTRDUP(MTYPE_HOST, STARTUP_CONFIG_FILE);
	host.default_config = XSTRDUP(MTYPE_HOST, DEFAULT_CONFIG_FILE);
	host.factory_config = XSTRDUP(MTYPE_HOST, FACTORY_CONFIG_FILE);
	host.lines = -1;
	host.motd = motd; //default_motd;
	host.motdfile = NULL;
	host.vty_timeout_val = VTY_TIMEOUT_DEFAULT;
	/* Vty access-class command */
	host.vty_accesslist_name = NULL;
	/* Vty access-calss for IPv6. */
	host.vty_ipv6_accesslist_name = NULL;
	/* VTY server thread. */
	//host.Vvty_serv_thread;
	/* Current directory. */
	host.vty_cwd = NULL;
	/* Configure lock. */
	host.vty_config = 0;
	/* Login password check. */
	host.no_password_check = 0;
	host.mutx = os_mutex_init();
	host.cli_mutx = os_mutex_init();
	if(access("/etc/.serial_no", F_OK) == 0)
	{
		memset(host.serial, '\0', sizeof(host.serial));
		os_read_file("/etc/.serial_no", host.serial, sizeof(host.serial));
		host.serial[strlen(host.serial)-1] = '\0';
		printf("=================%s================:(%s)\r\n", __func__,host.serial);
	}
	if(access("/etc/.wan-mac", F_OK) == 0)
	{
		ospl_char tmp[64];
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
	if (host.name)
	{
		XFREE(MTYPE_HOST, host.name);
		host.name = NULL;
	}
	if (host.vty_accesslist_name)
	{
		XFREE(MTYPE_VTY, host.vty_accesslist_name);
		host.vty_accesslist_name = NULL;
	}
	if (host.vty_ipv6_accesslist_name)
	{
		XFREE(MTYPE_VTY, host.vty_ipv6_accesslist_name);
		host.vty_ipv6_accesslist_name = NULL;
	}
	if (host.logfile)
	{
		XFREE(MTYPE_HOST, host.logfile);
		host.logfile = NULL;
	}
	if (host.motdfile)
	{
		XFREE(MTYPE_HOST, host.motdfile);
		host.motdfile = NULL;
	}
	if (host.config)
	{
		XFREE(MTYPE_HOST, host.config);
		host.config = NULL;
	}
	if (host.default_config)
	{
		XFREE(MTYPE_HOST, host.default_config);
		host.default_config = NULL;
	}
	if (host.factory_config)
	{
		XFREE(MTYPE_HOST, host.factory_config);
		host.factory_config = NULL;
	}
	if (host.cli_mutx)
	{
		os_mutex_exit(host.cli_mutx);
		host.cli_mutx = NULL;
	}
	if (host.mutx)
	{
		os_mutex_exit(host.mutx);
		host.mutx = NULL;
	}
	return OK;
}



/* Set config filename.  Called from vty.c */
void
host_config_set (ospl_char *filename)
{
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);
	if (host.config)
		XFREE(MTYPE_HOST, host.config);
	host.config = XSTRDUP(MTYPE_HOST, filename);
	if (host.mutx)
		os_mutex_unlock(host.mutx);
}

const char *
host_config_get (void)
{
	return host.config;
}

const char *
host_name_get (void)
{
	return host.name;
}

int
host_config_set_api (ospl_uint32 cmd, void *pVoid)
{
	int ret = ERROR;
	ospl_char *strValue = (ospl_char *)pVoid;
	ospl_uint32 *intValue = (ospl_uint32 *)pVoid;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_SET_HOSTNAME_CMD:
		if (host.name)
			XFREE(MTYPE_HOST, host.name);
		if(strValue)
			host.name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.name = NULL;
		ret = OK;
		break;
	case API_SET_DESC_CMD:
		if (host.description)
			XFREE(MTYPE_HOST, host.description);
		if(strValue)
			host.description = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.description = NULL;
		ret = OK;
		break;
	case API_SET_LINES_CMD:
		host.lines = (ospl_uint32)*intValue;
		ret = OK;
		break;
	case API_SET_LOGFILE_CMD:
		if (host.logfile)
			XFREE(MTYPE_HOST, host.logfile);
		if(strValue)
			host.logfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.logfile = NULL;
		ret = OK;
		break;
	case API_SET_CONFIGFILE_CMD:
		if (host.config)
			XFREE(MTYPE_HOST, host.config);
		if(strValue)
			host.config = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.config = NULL;
		ret = OK;
		break;
	case API_SET_ENCRYPT_CMD:
		host.encrypt = (ospl_bool)*intValue;
		ret = OK;
		break;
	case API_SET_MOTD_CMD:
		if (host.motd)
			XFREE(MTYPE_HOST, host.motd);
		if(strValue)
			host.motd = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.motd = NULL;
		ret = OK;
		break;
	case API_SET_MOTDFILE_CMD:
		if (host.motdfile)
			XFREE(MTYPE_HOST, host.motdfile);
		if(strValue)
			host.motdfile = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.motdfile = NULL;
		ret = OK;
		break;
	case API_SET_VTY_TIMEOUT_CMD:
		host.vty_timeout_val = (ospl_ulong)*intValue;
		ret = OK;
		break;
	case API_SET_ACCESS_CMD:
		if (host.vty_accesslist_name)
			XFREE(MTYPE_HOST, host.vty_accesslist_name);
		if(strValue)
			host.vty_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.vty_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_IPV6ACCESS_CMD:
		if (host.vty_ipv6_accesslist_name)
			XFREE(MTYPE_HOST, host.vty_ipv6_accesslist_name);
		if(strValue)
			host.vty_ipv6_accesslist_name = XSTRDUP(MTYPE_HOST, strValue);
		else
			host.vty_ipv6_accesslist_name = NULL;
		ret = OK;
		break;
	case API_SET_NOPASSCHK_CMD:
		host.no_password_check = (ospl_bool)*intValue;
		ret = OK;
		break;
	case API_SET_SYSMAC_CMD:
		memcpy(host.sysmac, strValue, 6);
		ret = OK;
		break;
	case API_SET_SERIAL_CMD:
		memset(host.serial, 0, sizeof(host.serial));
		if(strValue)
			strncpy(host.serial, strValue, MIN(strlen(strValue), sizeof(host.serial)));
		ret = OK;
		break;

	default:
		break;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return ret;
}

int
host_config_get_api (ospl_uint32 cmd, void *pVoid)
{
	int ret = ERROR;
	ospl_char *strValue = (ospl_char *)pVoid;
	ospl_uint32 *intValue = (ospl_uint32 *)pVoid;
	if (host.mutx)
		os_mutex_lock(host.mutx, OS_WAIT_FOREVER);

	switch(cmd)
	{
	case API_GET_HOSTNAME_CMD:
		if (host.name)
		{
			if(strValue)
				os_strcpy(strValue, host.name);
		}
		ret = OK;
		break;
	case API_GET_DESC_CMD:
		if (host.description)
		{
			if(strValue)
				os_strcpy(strValue, host.description);
		}
		ret = OK;
		break;
	case API_GET_LINES_CMD:
		if(intValue)
			*intValue = (ospl_uint32)host.lines;
		ret = OK;
		break;
	case API_GET_LOGFILE_CMD:
		if (host.logfile)
		{
			if(strValue)
				os_strcpy(strValue, host.logfile);
		}
		ret = OK;
		break;
	case API_GET_CONFIGFILE_CMD:
		if (host.config)
		{
			if(strValue)
				os_strcpy(strValue, host.config);
		}
		ret = OK;
		break;
	case API_GET_ENCRYPT_CMD:
		if(intValue)
			*intValue = (ospl_bool)host.encrypt;
		ret = OK;
		break;
	case API_GET_MOTD_CMD:
		if (host.motd)
		{
			if(strValue)
				os_strcpy(strValue, host.motd);
		}
		ret = OK;
		break;
	case API_GET_MOTDFILE_CMD:
		if (host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_VTY_TIMEOUT_CMD:
		if(intValue)
			*intValue = (ospl_uint32)host.vty_timeout_val;
		ret = OK;
		break;
	case API_GET_ACCESS_CMD:
		if (host.vty_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_IPV6ACCESS_CMD:
		if (host.vty_ipv6_accesslist_name)
		{
			if(strValue)
				os_strcpy(strValue, host.vty_ipv6_accesslist_name);
		}
		ret = OK;
		break;
	case API_GET_NOPASSCHK_CMD:
		if(intValue)
			*intValue = (ospl_bool)host.no_password_check;
		ret = OK;
		break;
	case API_GET_SYSMAC_CMD:
		if (!str_isempty(host.sysmac, sizeof(host.sysmac)))
		{
			if(strValue)
				memcpy(strValue, host.sysmac, 6);
		}
		ret = OK;
		break;
	case API_GET_SERIAL_CMD:
		if (!str_isempty(host.serial, sizeof(host.serial)))
		{
			if(strValue)
				os_strcpy(strValue, host.serial);
		}
		ret = OK;
		break;

	default:
		break;
	}
	if (host.mutx)
		os_mutex_unlock(host.mutx);
	return ret;
}


static int host_system_cpu_get(struct host_system *host_system)
{
	FILE *f = NULL;
	ospl_char buf[1024];
	f = fopen("/proc/cpuinfo", "r");
	if (f)
	{
		ospl_char *s = NULL;
		memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), f))
		{
			/* work backwards to ignore trailling isspace() */
			for (s = buf + strlen(buf); (s > buf) && isspace((int) *(s - 1)); s--)
				;
			*s = '\0';
#ifdef PL_BUILD_ARCH_X86
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
	ospl_char buf[1024];
	f = fopen("/proc/meminfo", "r");
	if (f)
	{
		ospl_uint32 off = 0;
		ospl_char *s = NULL;
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
#ifdef PL_BUILD_ARCH_X86
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
static ospl_ullong  scale(struct globals *g, ospl_ulong d)
{
	return ((ospl_ullong )d * g->mem_unit) >> G_unit_steps;
}
int free_main(int argc UNUSED_PARAM, ospl_char **argv IF_NOT_DESKTOP(UNUSED_PARAM))
{
	struct globals G;
	struct sysinfo info;
	ospl_ullong  cached;

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
	cached = ((ospl_ullong ) parse_cached_kb() * 1024) / G.mem_unit;

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

int show_host_system_information(struct host_system *host_system, struct vty *vty)
{
#ifdef PL_BUILD_ARCH_X86
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

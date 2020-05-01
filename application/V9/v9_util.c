/*
 * v9_util.c
 *
 *  Created on: 2019年11月26日
 *      Author: DELL
 */


#include "zebra.h"
#include "network.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "host.h"
#include "tty_com.h"

#include <sys/statfs.h>
#include <sys/vfs.h>

#include "v9_device.h"
#include "v9_util.h"
#include "v9_video.h"
#include "v9_serial.h"
#include "v9_slipnet.h"
#include "v9_cmd.h"

#include "v9_video_disk.h"
#include "v9_user_db.h"
#include "v9_video_db.h"

#include "v9_board.h"
#include "v9_video_sdk.h"
#include "v9_video_user.h"
#include "v9_video_board.h"
#include "v9_video_api.h"


#ifndef FSHIFT
# define FSHIFT 16              /* nr of bits of precision */
#endif
#define FIXED_1      (1 << FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x)  (unsigned)((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)


int v9_cpu_load(u_int16 *use)
{
	u_int16 val = 0;
	struct sysinfo info;
	sysinfo(&info);

	val = (LOAD_INT(info.loads[0]) + LOAD_INT(info.loads[1]) + LOAD_INT(info.loads[2]))/3;
	val = (val << 8);
	val |= ((LOAD_FRAC(info.loads[0]) + LOAD_FRAC(info.loads[1]) + LOAD_FRAC(info.loads[2]))/3)&0xff;
	if(use)
		*use = val;
	return OK;//;
}

#undef FSHIFT
#undef FIXED_1
#undef LOAD_INT
#undef LOAD_FRAC



int v9_memory_load(u_int32 *total, u_int8 *use)
{

	struct host_system host_system;

	memset(&host_system, 0, sizeof(struct host_system));
	host_system_information_get(&host_system);

	host_system.mem_total = host_system.s_info.totalram >> 20;//               //total
	host_system.mem_uses = (host_system.s_info.totalram - host_system.s_info.freeram) >> 20; //used
	host_system.mem_free = host_system.s_info.freeram >> 20;                 //free

	if(total)
		*total = host_system.mem_total;
	if(use)
		*use = ((host_system.mem_uses*100)/host_system.mem_total);
	return OK;
}

int v9_disk_load(char *path, u_int32 *total, u_int32 *use, u_int8 *puse)
{
	struct statfs diskInfo;
	if(statfs(path, &diskInfo) == 0)
	{
		unsigned long long totalBlocks = diskInfo.f_bsize;
		unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
		size_t mbTotalsize = totalSize>>20;
		unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;
		size_t mbFreedisk = freeDisk >>20;

		if(total)
			*total = mbTotalsize;
		if(use)
			*use = mbTotalsize - mbFreedisk;
		if(puse)
			*puse = ((mbTotalsize - mbFreedisk)*100)/mbTotalsize;
		return OK;
	}
	return ERROR;
}

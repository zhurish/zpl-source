#include "zebra.h"
#include "memory.h"
#include "command.h"
#include "memory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "interface.h"
#include "log.h"
#include "vty.h"


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


/*
 * 获取当前挂载硬盘数量
 */
static int _disk_count = 0;

static int v9_video_disk_info_load(void);

int v9_video_disk_count(void)
{
	v9_video_disk_info_load();
	if(_disk_count == 0)
		_disk_count++;
	return _disk_count;
}

char * v9_video_disk_root_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_ROOT, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_ROOT, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_ROOT, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}


char * v9_video_disk_base_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset (dirbase, 0, sizeof(dirbase));
	if (v9_video_disk_count () == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_BASE, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_BASE, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DIR_BASE, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	return dirbase;
}


char * v9_video_disk_db_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DB_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DB_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_DB_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}

char * v9_video_disk_cap_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAP_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAP_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAP_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}


char * v9_video_disk_capdb_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAPDB_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAPDB_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_CAPDB_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}

char * v9_video_disk_recg_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_RECG_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_RECG_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_RECG_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}

char * v9_video_disk_warn_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_WARN_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_WARN_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_WARN_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}

/*char * v9_video_disk_user_dir(u_int32 id)
{
	static char dirbase[V9_APP_DIR_NAME_MAX];
	memset(dirbase, 0, sizeof(dirbase));
	if(v9_video_disk_count() == 2)
	{
		if(id == APP_BOARD_CALCU_3 || id == APP_BOARD_CALCU_4)
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_USER_DIR, V9_VIDEO_MOUNT_DISKB1, V9_APP_DB_ID_ABS(id));
		else
			snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_USER_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
	else
	{
		snprintf(dirbase, sizeof(dirbase), "%s"V9_VIDEO_USER_DIR, V9_VIDEO_MOUNT_DISKA1, V9_APP_DB_ID_ABS(id));
	}
    return dirbase;
}*/



static int v9_video_disk_info_load(void)
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));
	_disk_count = 0;
	if(super_output_system("df -h | grep /mnt", buf, sizeof(buf)) == OK)
	{
		if(strstr(buf, "sda"))
			_disk_count++;
		if(strstr(buf, "sdb"))
			_disk_count++;
	}
	//printf("========%s========:count=%d\r\n", __func__, count);
	return _disk_count;
}

/*
 * 	root@TSLSmart-X5B:/# cat /etc/exports
	/mnt    *(rw,all_squash,insecure,sync,no_subtree_check)
/dev/sda1               232.9G     71.8M    232.8G   0% /mnt/diska1
/dev/sdb1               931.5G    429.8G    501.7G  46% /mnt/diskb1
/dev/sda1               232.9G     71.8M    232.8G   0% /mnt/sda1
/dev/sdb1               931.5G    429.8G    501.7G  46% /mnt/sdb1
 */


static int v9_video_disk_test(void)
{
	char filepath[256];
	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s/%s", v9_video_disk_root_dir(APP_BOARD_CALCU_1), "test");
	os_write_file(filepath, "012345678987654321", strlen("012345678987654321"));
	remove(filepath);
	if(_disk_count == 2)
	{
		memset(filepath, 0, sizeof(filepath));
		sprintf(filepath, "%s/%s", v9_video_disk_root_dir(APP_BOARD_CALCU_3), "test");
		os_write_file(filepath, "012345678987654321", strlen("012345678987654321"));
		remove(filepath);
	}
	sync();
	return 0;
}


int v9_video_disk_dir_init(void)
{
/*	v9_video_disk_info_load();
	if(_disk_count == 1)
	{

	}
	if(_disk_count == 2)
	{

	}
	if(_disk_count == 0)
		_disk_count++;*/
	if(v9_video_disk_count() > 0)
	{
		if(access(v9_video_disk_root_dir(APP_BOARD_CALCU_1), F_OK) != 0)
			mkdir(v9_video_disk_root_dir(APP_BOARD_CALCU_1), 0644);

		if(access(v9_video_disk_root_dir(APP_BOARD_CALCU_2), F_OK) != 0)
			mkdir(v9_video_disk_root_dir(APP_BOARD_CALCU_2), 0644);

		if(access(v9_video_disk_root_dir(APP_BOARD_CALCU_3), F_OK) != 0)
			mkdir(v9_video_disk_root_dir(APP_BOARD_CALCU_3), 0644);

		if(access(v9_video_disk_root_dir(APP_BOARD_CALCU_4), F_OK) != 0)
			mkdir(v9_video_disk_root_dir(APP_BOARD_CALCU_4), 0644);

		if(access(V9_USER_DB_DIR, F_OK) != 0)
			mkdir(V9_USER_DB_DIR, 0644);

		if(access(v9_video_disk_base_dir(APP_BOARD_CALCU_1), F_OK) != 0)
			mkdir(v9_video_disk_base_dir(APP_BOARD_CALCU_1), 0644);

		if(access(v9_video_disk_base_dir(APP_BOARD_CALCU_2), F_OK) != 0)
			mkdir(v9_video_disk_base_dir(APP_BOARD_CALCU_2), 0644);

		if(access(v9_video_disk_base_dir(APP_BOARD_CALCU_3), F_OK) != 0)
			mkdir(v9_video_disk_base_dir(APP_BOARD_CALCU_3), 0644);

		if(access(v9_video_disk_base_dir(APP_BOARD_CALCU_4), F_OK) != 0)
			mkdir(v9_video_disk_base_dir(APP_BOARD_CALCU_4), 0644);


		if(access(v9_video_disk_db_dir(APP_BOARD_CALCU_1), F_OK) != 0)
			mkdir(v9_video_disk_db_dir(APP_BOARD_CALCU_1), 0644);

		if(access(v9_video_disk_db_dir(APP_BOARD_CALCU_2), F_OK) != 0)
			mkdir(v9_video_disk_db_dir(APP_BOARD_CALCU_2), 0644);

		if(access(v9_video_disk_db_dir(APP_BOARD_CALCU_3), F_OK) != 0)
			mkdir(v9_video_disk_db_dir(APP_BOARD_CALCU_3), 0644);

		if(access(v9_video_disk_db_dir(APP_BOARD_CALCU_4), F_OK) != 0)
			mkdir(v9_video_disk_db_dir(APP_BOARD_CALCU_4), 0644);


		if(access(v9_video_disk_cap_dir(APP_BOARD_CALCU_1), F_OK) != 0)
			mkdir(v9_video_disk_cap_dir(APP_BOARD_CALCU_1), 0644);

		if(access(v9_video_disk_cap_dir(APP_BOARD_CALCU_2), F_OK) != 0)
			mkdir(v9_video_disk_cap_dir(APP_BOARD_CALCU_2), 0644);

		if(access(v9_video_disk_cap_dir(APP_BOARD_CALCU_3), F_OK) != 0)
			mkdir(v9_video_disk_cap_dir(APP_BOARD_CALCU_3), 0644);

		if(access(v9_video_disk_cap_dir(APP_BOARD_CALCU_4), F_OK) != 0)
			mkdir(v9_video_disk_cap_dir(APP_BOARD_CALCU_4), 0644);


		if(access(v9_video_disk_warn_dir(APP_BOARD_CALCU_1), F_OK) != 0)
			mkdir(v9_video_disk_warn_dir(APP_BOARD_CALCU_1), 0644);

		if(access(v9_video_disk_warn_dir(APP_BOARD_CALCU_2), F_OK) != 0)
			mkdir(v9_video_disk_warn_dir(APP_BOARD_CALCU_2), 0644);

		if(access(v9_video_disk_warn_dir(APP_BOARD_CALCU_3), F_OK) != 0)
			mkdir(v9_video_disk_warn_dir(APP_BOARD_CALCU_3), 0644);

		if(access(v9_video_disk_warn_dir(APP_BOARD_CALCU_4), F_OK) != 0)
			mkdir(v9_video_disk_warn_dir(APP_BOARD_CALCU_4), 0644);

		v9_video_disk_test();
	}
	return 0;
}

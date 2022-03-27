#include "zebra.h"
#include "zmemory.h"
#include "command.h"
#include "zmemory.h"
#include "memtypes.h"
#include "prefix.h"
#include "if.h"
#include "nsm_interface.h"
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
static zpl_uint32 _disk_timer = 0;
static zpl_uint32 _disk_keep_day = 0;
static int v9_video_disk_info_load(void);

int v9_video_disk_count(void)
{
	//v9_video_disk_info_load();
	if(_disk_count == 0)
		_disk_count++;
	return _disk_count;
}

char * v9_video_disk_root_dir(zpl_uint32 id)
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


char * v9_video_disk_base_dir(zpl_uint32 id)
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


char * v9_video_disk_db_dir(zpl_uint32 id)
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

char * v9_video_disk_cap_dir(zpl_uint32 id)
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


char * v9_video_disk_capdb_dir(zpl_uint32 id)
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

char * v9_video_disk_recg_dir(zpl_uint32 id)
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

char * v9_video_disk_warn_dir(zpl_uint32 id)
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

/*char * v9_video_disk_user_dir(zpl_uint32 id)
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



char * v9_video_disk_urlpath(int id, char *picpath)
{
	static char pathff[V9_APP_DIR_NAME_MAX];
	memset(pathff, 0, sizeof(pathff));
	sprintf(pathff, "%s/%s", v9_video_disk_capdb_dir(id), picpath);
	return pathff;
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
	v9_video_disk_info_load();
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
#ifdef ZPL_OPENWRT_UCI
		os_uci_get_integer("product.global.keepday", &_disk_keep_day);
#endif
		v9_video_disk_monitor_start(zpl_true);
	}
	return 0;
}



/*
 * disk monitor
 */

int v9_video_disk_keep_day_set(zpl_uint32 day)
{
#ifdef ZPL_OPENWRT_UCI
	if(os_uci_set_integer("product.global.keepday", day) == OK)
	{
		os_uci_save_config("product");
		_disk_keep_day = day;
		return OK;
	}
#endif
	return ERROR;
}

int v9_video_disk_keep_day_get(void)
{
#ifdef ZPL_OPENWRT_UCI
	if(_disk_keep_day == 0)
	{
		if(os_uci_set_integer("product.global.keepday", V9_APP_DISK_KEEP_DAY) == OK)
		{
			os_uci_save_config("product");
			_disk_keep_day = V9_APP_DISK_KEEP_DAY;
			return _disk_keep_day;
		}
	}
#endif
	return _disk_keep_day;
}
//#include <dirent.h>

static int isdirempty(char *dirname)
{
    /* 打开要进行匹配的文件目录 */
	if(!dirname)
		return -1;
    DIR *dir = opendir(dirname);
    struct dirent *ent = NULL;
    if (dir == NULL)
    {
        return -1;
    }
    while (1)
    {
        ent = readdir (dir);
        if (ent <= 0)
        {
            break;
        }
        if ((strcmp(".", ent->d_name)==0) || (strcmp("..", ent->d_name)==0))
        {
            continue;
        }
        /*判断是否有目录和文件*/
        if((ent->d_type == DT_DIR))
        {
        	if(isdirempty(ent->d_name) == 0)
        	{
        		rmdir(ent->d_name);
        		sync();
        	}
/*        	else
        	{

        	}*/
        }
        if (ent->d_type == DT_REG)
        {
        	closedir(dir);
            return -1;
        }
    }
    closedir(dir);
    return 0;
}

static int v9_video_sqltbl_monitor_datetime(zpl_uint32 id, zpl_uint32 table, zpl_uint32 datecnt)
{
	sqlite3 *db = NULL;
	int datetime = os_time(NULL);
	v9_video_db_lock();
	db = v9_video_sqldb_open(id, table);
	if(db)
	{
		datetime -= OS_SEC_DAY_V(datecnt);
		v9_video_sqldb_del_by_datetime(db,  id,  table, os_time_fmt ("sql", datetime));
		v9_video_sqldb_close(db, id);
	}
	v9_video_db_unlock();
	if(isdirempty(v9_video_disk_base_dir(id)) == 0)
	{
		rmdir(v9_video_disk_base_dir(id));
		sync();
	}
	return OK;
}


static int v9_video_sqltbl_monitor(zpl_uint32 id, zpl_uint32 table, zpl_uint32 limit, zpl_uint32 delcnt)
{
	sqlite3 *db = NULL;
	int getlimit = 0;
	v9_video_db_lock();
	db = v9_video_sqldb_open(id, table);
	if(db)
	{
		if(v9_video_sqldb_count(db,  id,  table, &getlimit) == OK)
		{
			if(getlimit >= limit)
			{
				v9_video_sqldb_select_by_oldid(db, id, table, delcnt);
			}
		}
		v9_video_sqldb_close(db, id);
	}
	v9_video_db_unlock();
	return OK;
}


static int v9_video_disk_monitor_task(void *p)
{
	zpl_uint32 diskload1 = 0, diskload2 = 0;
	if(v9_video_board_isactive(APP_BOARD_CALCU_1))
	{
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_1, 0, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_1, 1, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
	}
	if(v9_video_board_isactive(APP_BOARD_CALCU_2))
	{
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_2, 0, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_2, 1, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
	}
	if(v9_video_board_isactive(APP_BOARD_CALCU_3))
	{
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_3, 0, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_3, 1, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
	}
	if(v9_video_board_isactive(APP_BOARD_CALCU_4))
	{
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_4, 0, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
		v9_video_sqltbl_monitor_datetime(APP_BOARD_CALCU_4, 1, _disk_keep_day?_disk_keep_day:V9_APP_DISK_KEEP_DAY);
	}
	sync();

	if (v9_video_board[APP_BOARD_MAIN-1].id == APP_BOARD_MAIN)
	{
		//zlog_trap(MODULE_APP, "===================%s===================", __func__);
		v9_video_board_lock();

		diskload1 = v9_video_board[APP_BOARD_MAIN-1].board.diskload1;
		diskload2 = v9_video_board[APP_BOARD_MAIN-1].board.diskload2;
		v9_video_board_unlock();

		if(diskload1 >= V9_APP_DISK_LOAD_LIMIT)
		{
			if(v9_video_board_isactive(APP_BOARD_CALCU_1))
			{
				v9_video_sqltbl_monitor(APP_BOARD_CALCU_1, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				v9_video_sqltbl_monitor(APP_BOARD_CALCU_1, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
			}
			if(v9_video_board_isactive(APP_BOARD_CALCU_2))
			{
				v9_video_sqltbl_monitor(APP_BOARD_CALCU_2, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				v9_video_sqltbl_monitor(APP_BOARD_CALCU_2, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
			}
			if(v9_video_disk_count() == 1)
			{
				if(v9_video_board_isactive(APP_BOARD_CALCU_3))
				{
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_3, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_3, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				}
				if(v9_video_board_isactive(APP_BOARD_CALCU_4))
				{
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_4, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_4, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				}
			}
		}
		if(v9_video_disk_count() == 2)
		{
			if(diskload1 >= V9_APP_DISK_LOAD_LIMIT)
			{
				if(v9_video_board_isactive(APP_BOARD_CALCU_3))
				{
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_3, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_3, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				}
				if(v9_video_board_isactive(APP_BOARD_CALCU_4))
				{
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_4, 0, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
					v9_video_sqltbl_monitor(APP_BOARD_CALCU_4, 1, V9_APP_DB_ROW_LIMIT, V9_APP_DB_DELETE_LIMIT);
				}
			}
		}
		sync();
	}
	return OK;
}

int v9_video_disk_monitor_start(zpl_bool enable)
{
	if(enable)
	{
		if(_disk_timer)
			os_time_restart(_disk_timer, V9_APP_MONITOR_TIME_H(1));
		else
			_disk_timer = os_time_create(v9_video_disk_monitor_task, NULL, V9_APP_MONITOR_TIME_H(1));
	}
	else
	{
		if(_disk_timer)
		{
			os_time_cancel(_disk_timer);
			_disk_timer = 0;
		}
		return OK;
	}
	return (_disk_timer > 0)? OK:ERROR;
}

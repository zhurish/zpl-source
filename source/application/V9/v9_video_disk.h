#ifndef __V9_VIDEO_DISK_H__
#define __V9_VIDEO_DISK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define V9_VIDEO_DISK_BASE

//#define V9_VIDEO_MOUNT_BASE		"/mnt/diska1"
#define V9_VIDEO_MOUNT_DISKA1		"/mnt/diska1"
#define V9_VIDEO_MOUNT_DISKA2		"/mnt/diska2"
#define V9_VIDEO_MOUNT_DISKB1		"/mnt/diskb1"
#define V9_VIDEO_MOUNT_DISKB2		"/mnt/diskb2"

#define V9_VIDEO_DIR_ROOT		"/board%d"
#define V9_VIDEO_DIR_BASE		"/board%d/base"
#define V9_VIDEO_DB_DIR			"/board%d/base/db"
#define V9_VIDEO_CAP_DIR		"/board%d/base/cap"
#define V9_VIDEO_WARN_DIR		"/board%d/base/video"

#define V9_VIDEO_CAPDB_DIR		"/board%d"
#define V9_VIDEO_RECG_DIR		"/board%d"

#define V9_USER_DB_DIR		V9_VIDEO_MOUNT_DISKA1"/board1/user"

enum
{
	V9_VIDEO_DIR_ROOT_EM,
	V9_VIDEO_DIR_BASE_EM,
	V9_VIDEO_DIR_DB_EM,
	V9_VIDEO_DIR_CAP_EM,
	V9_VIDEO_DIR_WARN_EM,
};

#define V9_APP_DB_ID_ABS(n)			((n)-1)



#define V9_APP_MONITOR_TIME_M(n)	((n)*1000*60) 	//监控硬盘空间的时间间隔
#define V9_APP_MONITOR_TIME_H(n)	((n)*V9_APP_MONITOR_TIME_M(60)) 	//监控硬盘空间的时间间隔

#define V9_APP_DB_ROW_LIMIT			100000			//数据表限制表格函数
#define V9_APP_DB_DELETE_LIMIT		(1000)			//每次删除表格数据行数
#define V9_APP_DISK_LOAD_LIMIT		(80)			//硬盘占用百分比
#define V9_APP_DISK_KEEP_DAY		(30)			//保留30天的记录


int v9_video_disk_count(void);

char * v9_video_disk_root_dir(zpl_uint32 id);
char * v9_video_disk_base_dir(zpl_uint32 id);
char * v9_video_disk_db_dir(zpl_uint32 id);
char * v9_video_disk_cap_dir(zpl_uint32 id);
char * v9_video_disk_warn_dir(zpl_uint32 id);
//char * v9_video_disk_user_dir(zpl_uint32 id);
char * v9_video_disk_capdb_dir(zpl_uint32 id);
char * v9_video_disk_recg_dir(zpl_uint32 id);

char * v9_video_disk_urlpath(int id, char *picpath);

int v9_video_disk_dir_init(void);

int v9_video_disk_keep_day_set(zpl_uint32 day);
int v9_video_disk_keep_day_get(void);

int v9_video_disk_monitor_start(zpl_bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __V9_VIDEO_DISK_H__ */

#ifndef __V9_VIDEO_DISK_H__
#define __V9_VIDEO_DISK_H__

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

int v9_video_disk_count(void);

char * v9_video_disk_root_dir(u_int32 id);
char * v9_video_disk_base_dir(u_int32 id);
char * v9_video_disk_db_dir(u_int32 id);
char * v9_video_disk_cap_dir(u_int32 id);
char * v9_video_disk_warn_dir(u_int32 id);
//char * v9_video_disk_user_dir(u_int32 id);
char * v9_video_disk_capdb_dir(u_int32 id);
char * v9_video_disk_recg_dir(u_int32 id);
int v9_video_disk_dir_init(void);


#endif /* __V9_VIDEO_DISK_H__ */

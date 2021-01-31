/*
 * updownload.c
 *
 *  Created on: Mar 24, 2019
 *      Author: zhurish
 */
#include "zebra.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"


#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"

#ifdef PL_APP_MODULE
#include "application.h"
#endif/* PL_APP_MODULE */

/*
#undef ME_GOAHEAD_UPLOAD_DIR
#ifndef ME_GOAHEAD_UPLOAD_DIR
#define ME_GOAHEAD_UPLOAD_DIR SYSUPLOADDIR
#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
#endif
*/

#if ME_GOAHEAD_UPLOAD
#ifndef THEME_V9UI
static char *web_upgrade_file = NULL;
#endif /* THEME_V9UI */


#ifndef THEME_V9UI
static int web_filelist_count()
{
	DIR *dir = NULL;
	struct dirent *d = NULL;
	int i = 0;
	dir = opendir(WEB_UPLOAD_BASE);
	if (!dir)
	{
		return -1;
	}
	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{
		if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		else //if(d->d_type == 8)    ///file
		{
			//_WEB_DBG_TRAP("%s:get file->%s\r\n", __func__, d->d_name);
			i++;
		}
	}
	closedir(dir);
	return i;
}

static int web_filelist_tbl(Webs *wp, char *path, char *query)
{
	DIR *dir = NULL;
	struct dirent *d = NULL;
	char filetmp[256], i = 0;
	int fsize = 0;
	web_assert(wp);
	memset(filetmp, 0, sizeof(filetmp));
	/* Open the /proc directory. */
	dir = opendir(WEB_UPLOAD_BASE);
	if (!dir)
	{
		return -1;
	}
	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{
		if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		else //if(d->d_type == 8)    ///file
		{
			snprintf(filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE,
					d->d_name);
			fsize = os_file_size(filetmp);
			if (i)
				websWrite(wp, ",");
			if (fsize != ERROR)
				websWrite(wp, "{\"name\":\"%s\", \"size\":\"%s\"}", d->d_name,
						os_file_size_string(fsize));
			else
				websWrite(wp, "{\"name\":\"%s\", \"size\":\"%d\"}", d->d_name,
						0);
			i++;
		}
	}
	closedir(dir);
	return OK;
}

static int web_file_list(Webs *wp, char *path, char *query)
{
	web_assert(wp);
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	if (web_filelist_count() > 0)
	{
		websWrite(wp, "%s", "[");
		web_filelist_tbl(wp, path, query);
		websWrite(wp, "%s", "]");
	}
	else
	{
		websWrite(wp, "%s", "[");
		//websWrite(wp, "{\"name\":\"\", \"size\":\"\"}");
		websWrite(wp, "%s", "]");
	}
	websDone(wp);
	return 0;
}

static int web_handle_file_tbl(Webs *wp, void *p)
{
	char *strval = NULL;
	char filetmp[256];
	web_assert(wp);
	memset(filetmp, 0, sizeof(filetmp));
	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL != strval)
	{
		//_WEB_DBG_TRAP("%s: BTNID=%s\r\n", __func__, strval);
		if (strstr(strval, "delete"))
		{
			strval = webs_get_var(wp, T("name"), T(""));
			if (NULL != strval)
			{
				snprintf(filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE,
						strval);
				remove(filetmp);
				sync();
				return web_return_text_plain(wp, OK);
			}
		}
		else
		{
			strval = webs_get_var(wp, T("name"), T(""));
			if (NULL != strval)
			{
				//int ret = 0;
				snprintf(filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE, strval);
				if (access (filetmp, F_OK) >= 0)
				{
#ifdef APP_V9_MODULE
					if(strstr(strval, "sysupgrade") || strstr(strval, "V9"))

#elif defined APP_X5BA_MODULE
					if(strstr(strval, "sysupgrade") || strstr(strval, "BYQ"))
#else					
					if(strstr(strval, "sysupgrade"))		
#endif
					{
						if(web_upgrade_file)
						{
							free(web_upgrade_file);
							web_upgrade_file = NULL;
						}
						web_upgrade_file = strdup(strval);
						_WEB_DBG_TRAP("============%s================web_upgrade_file=%s\r\n",
							   __func__, web_upgrade_file? web_upgrade_file:"null");
						return web_return_text_plain(wp, OK);
					}
					else if(strstr(strval, ".ipk"))
					{
						chdir(WEB_UPLOAD_BASE);
						memset(filetmp, 0, sizeof(filetmp));
						snprintf(filetmp, sizeof(filetmp), "opkg --nodeps install %s", strval);
						system(filetmp);
						return web_return_text_plain(wp, OK);
					}
#ifdef APP_X5BA_MODULE
					if(x5b_app_upgrade_handle(WEB_UPLOAD_BASE, strval) == OK)
						return web_return_text_plain(wp, OK);
					return web_return_text_plain(wp, ERROR);
#else
					return web_return_text_plain(wp, OK);
#endif
				}
				return web_return_text_plain(wp, ERROR);
			}
		}
	}
	return ERROR;//;
}

static int web_handle_upgrade(Webs *wp, void *p)
{
	char *strval = NULL;
	char filetmp[512];
	web_assert(wp);
	memset (filetmp, 0, sizeof(filetmp));
	strval = webs_get_var (wp, T("BTNID"), T(""));
	if (NULL != strval)
	{
		_WEB_DBG_TRAP("============%s================BTNID=%s  web_upgrade_file=%s\r\n", __func__,
			   strval, web_upgrade_file? web_upgrade_file:"null");
		if (strstr (strval, "webupgrade") && web_upgrade_file != NULL)
		{
			chdir (WEB_UPLOAD_BASE);
			memset (filetmp, 0, sizeof(filetmp));
			if(webs_get_var (wp, T("clear"), T("")) != NULL)
				snprintf (filetmp, sizeof(filetmp), "sysupgrade -n %s", web_upgrade_file);
			else
				snprintf (filetmp, sizeof(filetmp), "sysupgrade %s", web_upgrade_file);
			if(web_upgrade_file)
			{
				free(web_upgrade_file);
				web_upgrade_file = NULL;
			}
			system (filetmp);
			return web_return_text_plain (wp, OK);
		}
	}
	_WEB_DBG_TRAP("============%s=============2===BTNID=%s  web_upgrade_file=%s\r\n", __func__,
		   strval ? strval:"null", web_upgrade_file? web_upgrade_file:"null");
	return ERROR; //;
}
#endif /* THEME_V9UI */
/*
 Dump the file upload details. Don't actually do anything with the uploaded file.
 */
/*
static char *base64_encode(const unsigned char *str, int str_len)
{
	int len = 0;
	//long str_len;
	char *res = NULL;
	int i = 0, j = 0;
	//定义base64编码表
	const char base64_table[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	//计算经过base64编码后的字符串长度
	//str_len=strlen(str);
	if (str_len % 3 == 0)
		len = str_len / 3 * 4;
	else
		len = (str_len / 3 + 1) * 4;

	res = malloc (sizeof(unsigned char) * len + 1);
	res[len] = '\0';

	//以3个8位字符为一组进行编码
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符
		res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符
		res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
		res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符
	}

	switch (str_len % 3)
	{
		case 1:
			res[i - 2] = '=';
			res[i - 1] = '=';
			break;
		case 2:
			res[i - 1] = '=';
			break;
	}
	return res;
}


unsigned char *base64_decode(unsigned char *code)
{
	//根据base64表，以字符找到对应的十进制数据
	int table[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
			17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
			29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
			46, 47, 48, 49, 50, 51 };
	long len;
	long str_len;
	unsigned char *res;
	int i, j;

	//计算解码后的字符串长度
	len = strlen (code);
	//判断编码后的字符串后是否有=
	if (strstr (code, "=="))
		str_len = len / 4 * 3 - 2;
	else if (strstr (code, "="))
		str_len = len / 4 * 3 - 1;
	else
		str_len = len / 4 * 3;

	res = malloc (sizeof(unsigned char) * str_len + 1);
	res[str_len] = '\0';

	//以4个字符为一位进行解码
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[j] = ((unsigned char) table[code[i]]) << 2
				| (((unsigned char) table[code[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合
		res[j + 1] = (((unsigned char) table[code[i + 1]]) << 4)
				| (((unsigned char) table[code[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合
		res[j + 2] = (((unsigned char) table[code[i + 2]]) << 6)
				| ((unsigned char) table[code[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
	}
	return res;
}
*/
static void web_action_upload(Webs *wp, char *path, char *query)
{
	char *id = NULL;
	WebsKey *s = NULL;
	WebsUpload *up = NULL;
	web_assert(wp);
	id = webs_get_var(wp, "ID", NULL);

	if (scaselessmatch(wp->method, "POST"))
	{
		for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s))
		{
			up = s->content.value.symbol;

			if(web_upload_call_hook(s->name.value.string, id, wp, up) == ERROR)
			{
				if(id && !strstr(id, "pic"))
					web_return_text_plain(wp, ERROR);
				else if(id && !strstr(id, "dir"))
					web_return_text_plain(wp, ERROR);
				return ;
			}
		}
		if(id && !strstr(id, "pic"))
			web_return_text_plain(wp, OK);
		else if(id && !strstr(id, "dir"))
		{
			web_return_text_plain(wp, OK);
		}
		return ;
	}
	if(id && !strstr(id, "pic"))
		web_return_text_plain(wp, ERROR);
	else if(id && !strstr(id, "dir"))
		web_return_text_plain(wp, ERROR);
	return ;
}
#ifdef PL_APP_MODULE
#ifdef THEME_V9UI
static int pic_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char json[512];
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));

	sprintf(uploadfile, "mv %s %s/%s", up->filename, V9_USER_DB_DIR, up->clientFilename);
	//_WEB_DBG_TRAP("%s: rename %s %s\r\n", __func__, up->filename, uploadfile);
	super_system(uploadfile);
	_WEB_DBG_TRAP("%s: %s\r\n", __func__, uploadfile);

	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", V9_USER_DB_DIR, up->clientFilename);

	if (os_file_access(uploadfile) != OK)
	{
		//_WEB_DBG_TRAP("%s: rename error(%s)\r\n", __func__, strerror(errno));
		web_return_text_plain(wp, ERROR);
		return ERROR;
	}
	sync();
	memset(json, 0, sizeof(json));//http://192.168.3.100:8080/tmp/app/tftpboot/ffd7f79c.jpeg
	sprintf(json, "{\"filename\": \"%s\", \"url\": \"%s\"}", up->clientFilename, uploadfile);
	web_return_application_json(wp, json);
	return OK;
}
#endif /* THEME_V9UI */
#endif/* PL_APP_MODULE */
static int other_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	web_assert(wp);
	web_assert(up);
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);

	if (rename(up->filename, uploadfile) < 0)
	{
		_WEB_DBG_TRAP("%s: rename %s %s  error:%s\r\n", __func__, up->filename, uploadfile, strerror(errno));
		return ERROR;
	}
	sync();
	return OK;
}


#ifdef APP_V9_MODULE
struct v9_web_load_dir
{
	u_int32 tid;
	u_int32 id;
	char dirpath[APP_PATH_MAX];
};

struct v9_web_load_dir *dir_tmp = NULL;

static int v9_video_dir_load_job(void *a)
{
	struct v9_web_load_dir *tmp = a;
	if(tmp && dir_tmp)
	{
		if(strlen(tmp->dirpath))
			v9_video_user_dir_add(tmp->id, tmp->dirpath);

		memset(dir_tmp, 0, sizeof(struct v9_web_load_dir));
		free(dir_tmp);
		dir_tmp = NULL;
	}
	return OK;
}
/*PUBLIC int websStartEvent(int delay, WebsEventProc proc, void *arg)
PUBLIC void websRestartEvent(int id, int delay)
PUBLIC void websStopEvent(int id)*/
#endif /* APP_V9_MODULE*/

static int dir_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	web_assert(wp);
	web_assert(up);
#ifdef APP_V9_MODULE
	if(dir_tmp == NULL)
	{
		dir_tmp = malloc(sizeof(struct v9_web_load_dir));
		if(dir_tmp)
		{
			char *s = up->clientFilename;
			char *t = strchr(up->clientFilename, '/');
			char *idtmp = webs_get_var(wp, "BID", NULL);
			if(idtmp == NULL)
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(MODULE_WEB, "Can not create get Board ID");
				free(dir_tmp);
				dir_tmp = NULL;
				web_return_text_plain(wp, ERROR);
				return ERROR;
			}
			memset(dir_tmp, 0, sizeof(struct v9_web_load_dir));

			//dir_tmp->id = V9_APP_BOARD_CALCU_ID(1);
			dir_tmp->id = V9_APP_BOARD_CALCU_ID(atoi(idtmp));
			sprintf(dir_tmp->dirpath, "%s/", V9_USER_DB_DIR);
			if(t)
				strncpy(dir_tmp->dirpath + strlen(dir_tmp->dirpath), s, t - s);
			else
				strcpy(dir_tmp->dirpath + strlen(dir_tmp->dirpath), s);

			if(os_file_access(dir_tmp->dirpath) != OK)
			{
				websOsMkdir(dir_tmp->dirpath, 1);
			}
			dir_tmp->tid = os_time_create_once(v9_video_dir_load_job, dir_tmp, 2000);
			if(dir_tmp->tid <= 0)
			{
				if(WEB_IS_DEBUG(EVENT))
					zlog_debug(MODULE_WEB, "Can not create once timer job");
				web_return_text_plain(wp, ERROR);
				return ERROR;
			}
		}
		else
		{
			if(WEB_IS_DEBUG(EVENT))
				zlog_debug(MODULE_WEB, "Can not malloc once timer job memory");
			web_return_text_plain(wp, ERROR);
			return ERROR;
		}
	}
	else
	{
		if(dir_tmp->tid > 0)
		{
			os_time_restart(dir_tmp->tid, 2000);
		}
		else
		{
			web_return_text_plain(wp, ERROR);
			return ERROR;
		}
	}
	if(dir_tmp)
	{
		sprintf(uploadfile, "mv %s %s/%s", up->filename, V9_USER_DB_DIR, up->clientFilename);
		super_system(uploadfile);

		memset(uploadfile, 0, sizeof(uploadfile));
		sprintf(uploadfile, "%s/%s", V9_USER_DB_DIR, up->clientFilename);

		if (os_file_access(uploadfile) != OK)
		{
			web_return_text_plain(wp, ERROR);
			return ERROR;
		}
	}
	sync();
#else
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
	if (rename(up->filename, uploadfile) < 0)
	{
		if(WEB_IS_DEBUG(EVENT))
			zlog_debug(MODULE_WEB, "Can not rename %s %s (ERROR:%s)", up->filename, uploadfile, strerror(errno));
		//_WEB_DBG_TRAP("%s: rename %s %s  error:%s\r\n", __func__, up->filename, uploadfile, strerror(errno));
		return ERROR;
	}
	sync();
#endif /* APP_V9_MODULE*/

	return OK;
}

static int config_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	web_assert(wp);
	web_assert(up);
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", SYSCONFDIR, up->clientFilename);
	//_WEB_DBG_TRAP("%s: save path=%s\r\n", __func__, uploadfile);
	if (rename(up->filename, uploadfile) < 0)
	{
		_WEB_DBG_TRAP("%s: rename error\r\n", __func__);
		return ERROR;
	}
	sync();
	return OK;
}

/*
 * Download
 */

static void downloadFileWriteEvent(Webs *wp)
{
	char *buf;
	ssize len, wrote;
	int err;

	assert(wp);
	assert(websValid(wp));

	if ((buf = walloc(ME_GOAHEAD_LIMIT_BUFFER)) == NULL)
	{
		websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot get memory");
		return;
	}
	while ((len = websPageReadData(wp, buf, ME_GOAHEAD_LIMIT_BUFFER)) > 0)
	{
		if ((wrote = websWriteSocket(wp, buf, len)) < 0)
		{
			err = socketGetError(wp->sid);
			if (err == EWOULDBLOCK || err == EAGAIN)
			{
				websPageSeek(wp, -len, SEEK_CUR);
			}
			else
			{
				/* Will call websDone below */
				wp->state = WEBS_COMPLETE;
			}
			break;
		}
		if (wrote != len)
		{
			websPageSeek(wp, -(len - wrote), SEEK_CUR);
			break;
		}
	}
	wfree(buf);
	if (len <= 0)
	{
		websDone(wp);
	}
}

static void downloadFileClose()
{
	//wfree(websIndex);
	//websIndex = NULL;
	//wfree(websDocuments);
	//websDocuments = NULL;
}


static bool downloadFileHandler(Webs *wp)
{
	WebsFileInfo info;
	char *tmp = NULL, *date = NULL;
	ssize nchars = 0;
	int code = 0;
	char* ID = NULL;

	char* disposition = NULL; //临时保存 附件 标识
	char* filename = NULL; //文件名 用于下载后保存的文件名称

	assert(websValid(wp));
	assert(wp->method);
	assert(wp->filename && wp->filename[0]);

	ID = webs_get_var(wp, "ID", NULL);
	if (ID == NULL)
	{
		_WEB_DBG_TRAP("%s: can not get 'ID' option\r\n", __func__);
		return TRUE;
	}

	//_WEB_DBG_TRAP("%s: ID=%s\r\n", __func__, ID);
	// 取download.lua?video=C:\aaa.wmv 中的 C:\aaa.wmv
	tmp = webs_get_var(wp, "filename", NULL);
	if (tmp == NULL)
	{
		_WEB_DBG_TRAP("%s: can not get 'filename' option\r\n", __func__);
		return TRUE;
	}

	//_WEB_DBG_TRAP("%s: filename=%s\r\n", __func__, tmp);
	//_WEB_DBG_TRAP("%s: pathfilename=%s\r\n", __func__, tmp);
	//_WEB_DBG_TRAP("%s: wp->filename=%s\r\n", __func__, wp->filename);

	if(web_download_call_hook("filename", ID, wp, &filename) != OK)
	{
		_WEB_DBG_TRAP("%s: web_download_call_hook ERROR\r\n", __func__);
		return TRUE;
	}
	//filename = sclone("boot.log");
/*
	if (wp->path)
	{
		_WEB_DBG_TRAP("%s: pathfilename=%s\r\n", __func__, wp->path);
	}
	if (wp->filename)
	{
		_WEB_DBG_TRAP("%s: filename=%s\r\n", __func__, wp->filename);
	}
	if (wp->ext)
	{
		_WEB_DBG_TRAP("%s: filenameExt=%s\r\n", __func__, wp->ext);
	}
*/

	//If the file is a directory, redirect using the nominated default page
	if (websPageIsDirectory(wp))
	{
		nchars = strlen(wp->path);
		if (wp->path[nchars - 1] == '/' || wp->path[nchars - 1] == '\\')
		{
			wp->path[--nchars] = '\0';
		}
		char* websIndex = "testdownload";
		tmp = sfmt("%s/%s", wp->path, websIndex);
		websRedirect(wp, tmp);
		wfree(tmp);
		free(filename);
		return TRUE;
	}
	if (websPageOpen(wp, O_RDONLY | O_BINARY, 0666) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s",
				wp->path);
		free(filename);
		return TRUE;
	}
	if (websPageStat(wp, &info) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
		free(filename);
		return TRUE;
	}
	code = HTTP_CODE_OK;
	if (wp->since && info.mtime <= wp->since)
	{
		code = HTTP_CODE_NOT_MODIFIED;
	}
	websSetStatus(wp, code);
	websWriteHeaders(wp, info.size, 0);

	//浏览器下载文件时的文件名
	disposition = (char*) walloc(20 + strlen(filename) + 1);
	sprintf(disposition, "attachment;filename=%s", sclone(filename));
	websWriteHeader(wp, "Content-Disposition", sclone(disposition));
	free(filename);
	free(disposition);
	filename = NULL;
	disposition = NULL;

	if ((date = websGetDateString(&info)) != NULL)
	{
		websWriteHeader(wp, "Last-modified", "%s", date);
		wfree(date);
	}
	websWriteEndHeaders(wp);

	//All done if the browser did a HEAD request
	if (smatch(wp->method, "HEAD"))
	{
		websDone(wp);
		return TRUE;
	}
	websSetBackgroundWriter(wp, downloadFileWriteEvent);
	return TRUE;
}

static int web_action_downLoad(Webs *wp, char *path, char *query)
{
	web_assert(wp);
	WebsHandlerProc service = (*wp).route->handler->service;
	(*wp).route->handler->close = (*downloadFileClose);
	(*wp).route->handler->service = (*downloadFileHandler);
	(*wp).route->handler->service(wp);
	(*wp).route->handler->service = service;
	return 0;
}

int web_download_setup(Webs *wp, char *pathfilename, char *filename, char *filenameExt, char **outputname)
{
	web_assert(wp);
	if (wp->ext)
		wfree(wp->ext);

	if (filenameExt)
	{
		wp->ext = (char*) walloc(1 + strlen(filenameExt) + 1);
		sprintf(wp->ext, ".%s", sclone(filenameExt));
	}
	if (wp->filename)
	{
		wfree(wp->filename);
	}
	wp->filename = sclone(pathfilename);

	if (wp->path)
	{
		wfree(wp->path);
	}
	wp->path = sclone(pathfilename);

	if(outputname)
		*outputname = sclone(filename);
	return OK;
}


static int syslog_download_cb(Webs *wp, char **filename)
{
/*	if(pathfilename)
		*pathfilename = sclone("/var/log/boot.log");
	if(filename)
		*filename = sclone("boot.log");
	if(filenameExt)
	{
		*filenameExt = sclone("log");
	}*/
	char *apathfilename = "/var/log/boot.log";
	char *afilename = "boot.log";
	char *afilenameExt = "log";

	return web_download_setup(wp, apathfilename, afilename, afilenameExt, filename);
}

static int config_download_cb(Webs *wp, char **filename)
{
	//pathfilename =sclone("/home/zhurish/workspace/SWPlatform/debug/etc/default-config.cfg");
	//pathfilename = sclone(SYSCONFDIR"/default-config.cfg");

/*	if (pathfilename)
		*pathfilename = sclone(SYSCONFDIR"/default-config.cfg");
	if (filename)
		*filename = sclone("default-config.cfg");
	if (filenameExt)
	{
		*filenameExt = sclone("cfg");
	}
	return OK;*/
	web_assert(wp);
	char *apathfilename = SYSCONFDIR"/default-config.cfg";
	char *afilename = "default-config.cfg";
	char *afilenameExt = "cfg";

	return web_download_setup(wp, apathfilename, afilename, afilenameExt, filename);
}



int web_updownload_app(void)
{
	websDefineAction("download", web_action_downLoad);

	websDefineAction("upload", web_action_upload);
	websDefineAction("upgrade", web_action_upload);
#ifndef THEME_V9UI
	websFormDefine("file-list", web_file_list);

	web_button_add_hook("filetbl", "delete", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "install", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "webupgrade", web_handle_upgrade, NULL);
#endif /* THEME_V9UI */
#ifdef PL_APP_MODULE
#ifdef THEME_V9UI
	web_upload_add_hook("uploadpic", "pic", pic_upload_cb, NULL);
#endif /* THEME_V9UI */
#endif/* PL_APP_MODULE */

	web_upload_add_hook("uploadir", "dir", dir_upload_cb, NULL);

	web_upload_add_hook("upload_filename", "other", other_upload_cb, NULL);
	//web_upload_add_hook("upgrade_filename", "sysupgrade", sys_upgrade_cb, NULL);
	web_upload_add_hook("upgrade_filename", "upload", other_upload_cb, NULL);

	//web_upload_add_hook("upgrade_filename", "sysupgrade", config_upload_cb, NULL);
	web_upload_add_hook("upload_config", "config", config_upload_cb, NULL);

	web_download_add_hook("filename", "syslog", syslog_download_cb);
	web_download_add_hook("filename", "config", config_download_cb);

	return 0;
}

#endif

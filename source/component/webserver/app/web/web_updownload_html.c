/*
 * updownload.c
 *
 *  Created on: Mar 24, 2019
 *      Author: zhurish
 */
#include "zplos_include.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"


#include "web_api.h"
#include "web_jst.h"
#include "web_app.h"

#ifdef ZPL_APP_MODULE
#include "application.h"
#endif/* ZPL_APP_MODULE */

/*
#undef ME_GOAHEAD_UPLOAD_DIR
#ifndef ME_GOAHEAD_UPLOAD_DIR
#define ME_GOAHEAD_UPLOAD_DIR SYSUPLOADDIR
#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
#endif
*/

#if ME_GOAHEAD_UPLOAD

static char *web_upgrade_file = NULL;


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
				websWriteCache(wp, ",");
			if (fsize != ERROR)
				websWriteCache(wp, "{\"name\":\"%s\", \"size\":\"%s\"}", d->d_name,
						os_file_size_string(fsize));
			else
				websWriteCache(wp, "{\"name\":\"%s\", \"size\":\"%d\"}", d->d_name,
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

	if (web_filelist_count() > 0)
	{
		websWriteCache(wp, "%s", "[");
		web_filelist_tbl(wp, path, query);
		websWriteCache(wp, "%s", "]");
	}
	else
	{
		websWriteCache(wp, "%s", "[");
		websWriteCache(wp, "%s", "]");
	}
	websWriteHeaders (wp, websWriteCacheLen(wp), 0);
	websWriteHeader (wp, "Content-Type", "text/plain");
	websWriteEndHeaders (wp);	
	websWriteCacheFinsh(wp);
	websDone(wp);
	return 0;
}

static int web_handle_file_tbl(Webs *wp, void *p)
{
	char *strval = NULL;
	char filetmp[256];
	web_assert(wp);
	memset(filetmp, 0, sizeof(filetmp));
	strval = webs_get_var(wp, T("button-ID"), T(""));
	if (NULL != strval)
	{
		if (strstr(strval, "delete"))
		{
			strval = webs_get_var(wp, T("name"), T(""));
			if (NULL != strval)
			{
				snprintf(filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE,
						strval);
				remove(filetmp);
				sync();
				return web_return_text_plain(wp, OK, NULL);
			}
		}
		else
		{
			strval = webs_get_var(wp, T("name"), T(""));
			if (NULL != strval)
			{
				snprintf(filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE, strval);
				if (access (filetmp, F_OK) >= 0)
				{				
					if(strstr(strval, "sysupgrade"))		
					{
						if(web_upgrade_file)
						{
							free(web_upgrade_file);
							web_upgrade_file = NULL;
						}
						web_upgrade_file = strdup(strval);
						_WEB_DBG_TRAP("============%s================web_upgrade_file=%s\r\n",
							   __func__, web_upgrade_file? web_upgrade_file:"null");
						return web_return_text_plain(wp, OK, NULL);
					}
					else if(strstr(strval, ".ipk"))
					{
						chdir(WEB_UPLOAD_BASE);
						memset(filetmp, 0, sizeof(filetmp));
						snprintf(filetmp, sizeof(filetmp), "opkg --nodeps install %s", strval);
						system(filetmp);
						return web_return_text_plain(wp, OK, NULL);
					}
					return web_return_text_plain(wp, OK, NULL);
				}
				return web_return_text_plain(wp, ERROR, NULL);
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
	strval = webs_get_var (wp, T("button-ID"), T(""));
	if (NULL != strval)
	{
		_WEB_DBG_TRAP("============%s================button-ID=%s  web_upgrade_file=%s\r\n", __func__,
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
			return web_return_text_plain (wp, OK, NULL);
		}
	}
	_WEB_DBG_TRAP("============%s=============2===button-ID=%s  web_upgrade_file=%s\r\n", __func__,
		   strval ? strval:"null", web_upgrade_file? web_upgrade_file:"null");
	return ERROR; //;
}

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
					web_return_text_plain(wp, ERROR, NULL);
				else if(id && !strstr(id, "dir"))
					web_return_text_plain(wp, ERROR, NULL);
				return ;
			}
		}
		if(id && !strstr(id, "pic"))
			web_return_text_plain(wp, OK, NULL);
		else if(id && !strstr(id, "dir"))
		{
			web_return_text_plain(wp, OK, NULL);
		}
		return ;
	}
	if(id && !strstr(id, "pic"))
		web_return_text_plain(wp, ERROR, NULL);
	else if(id && !strstr(id, "dir"))
		web_return_text_plain(wp, ERROR, NULL);
	return ;
}

static int other_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	web_assert(wp);
	web_assert(up);
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);

	if (rename(up->filename, uploadfile) < 0)
	{
		_WEB_DBG_TRAP("%s: rename %s %s  error:%s\r\n", __func__, up->filename, uploadfile, strerror(ipstack_errno));
		return ERROR;
	}
	sync();
	return OK;
}



static int dir_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	web_assert(wp);
	web_assert(up);
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);
	if (rename(up->filename, uploadfile) < 0)
	{
		if(WEB_IS_DEBUG(EVENT))
			zlog_debug(MODULE_WEB, "Can not rename %s %s (ERROR:%s)", up->filename, uploadfile, strerror(ipstack_errno));
		//_WEB_DBG_TRAP("%s: rename %s %s  error:%s\r\n", __func__, up->filename, uploadfile, strerror(ipstack_errno));
		return ERROR;
	}
	sync();
	return OK;
}

static int config_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	web_assert(wp);
	web_assert(up);
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", SYSCONFDIR, up->clientFilename);
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
		return zpl_true;
	}

	//_WEB_DBG_TRAP("%s: ID=%s\r\n", __func__, ID);
	// 取download.lua?video=C:\aaa.wmv 中的 C:\aaa.wmv
	tmp = webs_get_var(wp, "filename", NULL);
	if (tmp == NULL)
	{
		_WEB_DBG_TRAP("%s: can not get 'filename' option\r\n", __func__);
		return zpl_true;
	}

	//_WEB_DBG_TRAP("%s: filename=%s\r\n", __func__, tmp);
	//_WEB_DBG_TRAP("%s: pathfilename=%s\r\n", __func__, tmp);
	//_WEB_DBG_TRAP("%s: wp->filename=%s\r\n", __func__, wp->filename);

	if(web_download_call_hook("filename", ID, wp, &filename) != OK)
	{
		_WEB_DBG_TRAP("%s: web_download_call_hook ERROR\r\n", __func__);
		return zpl_true;
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
		return zpl_true;
	}
	if (websPageOpen(wp, O_RDONLY | O_BINARY, 0666) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s",
				wp->path);
		free(filename);
		return zpl_true;
	}
	if (websPageStat(wp, &info) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
		free(filename);
		return zpl_true;
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
		return zpl_true;
	}
	websSetBackgroundWriter(wp, downloadFileWriteEvent);
	return zpl_true;
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

	websFormDefine("file-list", web_file_list);

	web_button_add_hook("filetbl", "delete", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "install", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "webupgrade", web_handle_upgrade, NULL);


	web_upload_add_hook("uploadir", "dir", dir_upload_cb, NULL);

	web_upload_add_hook("upload_filename", "other", other_upload_cb, NULL);
	web_upload_add_hook("upgrade_filename", "upload", other_upload_cb, NULL);

	web_upload_add_hook("upload_config", "config", config_upload_cb, NULL);

	web_download_add_hook("filename", "syslog", syslog_download_cb);
	web_download_add_hook("filename", "config", config_download_cb);

	return 0;
}

#endif

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


#undef ME_GOAHEAD_UPLOAD_DIR

#ifndef ME_GOAHEAD_UPLOAD_DIR
#define ME_GOAHEAD_UPLOAD_DIR SYSUPLOADDIR
#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
#endif



#if ME_GOAHEAD_UPLOAD
static char *web_upgrade_file = NULL;
static struct web_upload_cb *web_upload_cbtbl[WEB_UPLOAD_CB_MAX];
static struct web_download_cb *web_download_cbtbl[WEB_UPLOAD_CB_MAX];

int web_upload_add_hook(char *form, char *id, int (*cb)(void *, void *, void *),
		void *p)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] == NULL)
		{
			web_upload_cbtbl[i] = XMALLOC(MTYPE_WEB_ROUTE,
					sizeof(struct web_upload_cb));
			strcpy(web_upload_cbtbl[i]->form, form);
			strcpy(web_upload_cbtbl[i]->ID, id);
			web_upload_cbtbl[i]->upload_cb = cb;
			web_upload_cbtbl[i]->pVoid = p;
			return OK;
		}
	}
	return ERROR;
}

int web_upload_del_hook(char *form, char *id)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] != NULL)
		{
			if (strcmp(web_upload_cbtbl[i]->form, form) == 0
					&& strcmp(web_upload_cbtbl[i]->ID, id) == 0)
			{
				web_upload_cbtbl[i]->upload_cb = NULL;
				XFREE(MTYPE_WEB_ROUTE, web_upload_cbtbl[i]);
				web_upload_cbtbl[i] = NULL;
				return OK;
			}
		}
	}
	return ERROR;
}

static int web_upload_call_hook(char *form, char *id, Webs *wp, WebsUpload *up)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_upload_cbtbl[i] != NULL)
		{
			if (strcmp(web_upload_cbtbl[i]->form, form) == 0
					&& (id==NULL || (id && strcmp(web_upload_cbtbl[i]->ID, id) == 0))
					&& web_upload_cbtbl[i]->upload_cb)
			{
				return (web_upload_cbtbl[i]->upload_cb)(wp, up, web_upload_cbtbl[i]->pVoid);
			}
		}
	}
	return ERROR;
}

int web_download_add_hook(char *form, char *id, int (*cb)(void *, char **))
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] == NULL)
		{
			web_download_cbtbl[i] = XMALLOC(MTYPE_WEB_ROUTE,
					sizeof(struct web_upload_cb));
			strcpy(web_download_cbtbl[i]->form, form);
			strcpy(web_download_cbtbl[i]->ID, id);
			web_download_cbtbl[i]->download_cb = cb;
			//web_download_cbtbl[i]->pVoid = p;
			return OK;
		}
	}
	return ERROR;
}

int web_download_del_hook(char *form, char *id)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] != NULL)
		{
			if (strcmp(web_download_cbtbl[i]->form, form) == 0
					&& strcmp(web_download_cbtbl[i]->ID, id) == 0)
			{
				web_download_cbtbl[i]->download_cb = NULL;
				XFREE(MTYPE_WEB_ROUTE, web_download_cbtbl[i]);
				web_download_cbtbl[i] = NULL;
				return OK;
			}
		}
	}
	return ERROR;
}

static int web_download_call_hook(char *form, char *id, Webs *wp, char **filename)
{
	int i = 0;
	for (i = 0; i < WEB_UPLOAD_CB_MAX; i++)
	{
		if (web_download_cbtbl[i] != NULL)
		{
			if (strcmp(web_download_cbtbl[i]->form, form) == 0
					&& (id==NULL || (id && strcmp(web_download_cbtbl[i]->ID, id) == 0))
					&& web_download_cbtbl[i]->download_cb)
			{
				return (web_download_cbtbl[i]->download_cb)(wp, filename);
			}
		}
	}
	return ERROR;
}




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
			//printf("%s:get file->%s\r\n", __func__, d->d_name);
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
	memset(filetmp, 0, sizeof(filetmp));
	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL != strval)
	{
		//printf("%s: BTNID=%s\r\n", __func__, strval);
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
#endif
#ifdef APP_X5BA_MODULE
					if(strstr(strval, "sysupgrade") || strstr(strval, "BYQ"))
#endif
					{
/*						chdir(WEB_UPLOAD_BASE);
						memset(filetmp, 0, sizeof(filetmp));
						snprintf(filetmp, sizeof(filetmp), "sysupgrade %s", strval);
						system(filetmp);*/

						if(web_upgrade_file)
						{
							free(web_upgrade_file);
							web_upgrade_file = NULL;
						}
						web_upgrade_file = strdup(strval);
						printf("============%s================web_upgrade_file=%s\r\n",
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
	memset (filetmp, 0, sizeof(filetmp));
	strval = webs_get_var (wp, T("BTNID"), T(""));
	if (NULL != strval)
	{
		printf("============%s================BTNID=%s  web_upgrade_file=%s\r\n", __func__,
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
	printf("============%s=============2===BTNID=%s  web_upgrade_file=%s\r\n", __func__,
		   strval ? strval:"null", web_upgrade_file? web_upgrade_file:"null");
	return ERROR; //;
}
/*
 Dump the file upload details. Don't actually do anything with the uploaded file.
 */
static void web_action_upload(Webs *wp, char *path, char *query)
{
	char *id = NULL;
	WebsKey *s = NULL;
	WebsUpload *up = NULL;
	if (scaselessmatch(wp->method, "POST"))
	{
		for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s))
		{
			up = s->content.value.symbol;
			id = webs_get_var(wp, "ID", NULL);
			if (id != NULL)
				;//printf("%s: ID=%s\r\n", __func__, id);

/*			printf("%s: FILE=%s\r\n", __func__, s->name.value.string); //input=file id=
			printf("%s: FILENAME=%s\r\n", __func__, up->filename); //缓存文件
			printf("%s: CLIENT=%s\r\n", __func__, up->clientFilename); //实际文件名称
			//printf("%s: html FILENAME=%s\r\n", __func__, wp->filename); //缓存文件
			printf("%s: TYPE=%s\r\n", __func__, up->contentType);
			printf("%s: SIZE=%d\r\n", __func__, up->size);
			printf("%s/%s", WEB_UPLOAD_BASE, up->clientFilename);*/
			//upfile = sfmt("%s/tmp/%s", websGetDocuments(), up->clientFilename);

			if(web_upload_call_hook(s->name.value.string, id, wp, up) == ERROR)
			{
				web_return_text_plain(wp, ERROR);
				return ;
			}
		}
		//printf("%s:%s", __func__);
		web_return_text_plain(wp, OK);
		return ;
	}
	//printf("%s:wp->method:%s\r\n", __func__, wp->method);
	web_return_text_plain(wp, ERROR);
	return ;
}

static int other_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", WEB_UPLOAD_BASE, up->clientFilename);

	//printf("%s: save path=%s\r\n", __func__, uploadfile);

	if (rename(up->filename, uploadfile) < 0)
	{
		return ERROR;
	}
	return OK;
}

static int config_upload_cb(Webs *wp, WebsUpload *up, void *p)
{
	char uploadfile[256];
	memset(uploadfile, 0, sizeof(uploadfile));
	sprintf(uploadfile, "%s/%s", SYSCONFDIR, up->clientFilename);
	//printf("%s: save path=%s\r\n", __func__, uploadfile);
	if (rename(up->filename, uploadfile) < 0)
	{
		return ERROR;
	}
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
		printf("%s: can not get 'ID' option\r\n", __func__);
		return TRUE;
	}

	//printf("%s: ID=%s\r\n", __func__, ID);
	// 取download.lua?video=C:\aaa.wmv 中的 C:\aaa.wmv
	tmp = webs_get_var(wp, "filename", NULL);
	if (tmp == NULL)
	{
		printf("%s: can not get 'filename' option\r\n", __func__);
		return TRUE;
	}

	//printf("%s: filename=%s\r\n", __func__, tmp);
	//printf("%s: pathfilename=%s\r\n", __func__, tmp);
	//printf("%s: wp->filename=%s\r\n", __func__, wp->filename);

	if(web_download_call_hook("filename", ID, wp, &filename) != OK)
	{
		printf("%s: web_download_call_hook ERROR\r\n", __func__);
		return TRUE;
	}
	//filename = sclone("boot.log");
/*
	if (wp->path)
	{
		printf("%s: pathfilename=%s\r\n", __func__, wp->path);
	}
	if (wp->filename)
	{
		printf("%s: filename=%s\r\n", __func__, wp->filename);
	}
	if (wp->ext)
	{
		printf("%s: filenameExt=%s\r\n", __func__, wp->ext);
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
	WebsHandlerProc service = (*wp).route->handler->service;
	(*wp).route->handler->close = (*downloadFileClose);
	(*wp).route->handler->service = (*downloadFileHandler);
	(*wp).route->handler->service(wp);
	(*wp).route->handler->service = service;
	return 0;
}

int web_download_setup(Webs *wp, char *pathfilename, char *filename, char *filenameExt, char **outputname)
{
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
	char *apathfilename = SYSCONFDIR"/default-config.cfg";
	char *afilename = "default-config.cfg";
	char *afilenameExt = "cfg";

	return web_download_setup(wp, apathfilename, afilename, afilenameExt, filename);
}



int web_updownload_app(void)
{
	websDefineAction("download", web_action_downLoad);

	websDefineAction("upload", web_action_upload);
	websFormDefine("file-list", web_file_list);

	web_button_add_hook("filetbl", "delete", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "install", web_handle_file_tbl, NULL);
	web_button_add_hook("filetbl", "webupgrade", web_handle_upgrade, NULL);

	web_upload_add_hook("upload_filename", "other", other_upload_cb, NULL);
	web_upload_add_hook("upload_config", "config", config_upload_cb, NULL);
	//web_upload_add_hook("upload_pic", "config", config_upload_cb, NULL);

	web_download_add_hook("filename", "syslog", syslog_download_cb);
	web_download_add_hook("filename", "config", config_download_cb);

	return 0;
}


int web_updownload_cb_init(void)
{
	memset(web_upload_cbtbl, 0, sizeof(web_upload_cbtbl));
	memset(web_download_cbtbl, 0, sizeof(web_download_cbtbl));
	return OK;
}
#endif

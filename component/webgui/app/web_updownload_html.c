/*
 * updownload.c
 *
 *  Created on: Mar 24, 2019
 *      Author: zhurish
 */
//#include "zebra.h"
#ifdef HAVE_CONFIG_H
#include "plconfig.h"
#endif /* HAVE_CONFIG_H */

#include "os_platform.h"

#include "module.h"
#include <netinet/in.h>
#include "memory.h"
#include "zassert.h"
#include "log.h"
#include "os_list.h"
#include "os_sem.h"
#include "os_task.h"

#include "goahead.h"
#include "webgui_app.h"

#if ME_GOAHEAD_UPLOAD
/*
 * Upload
 */
/*
    Dump the file upload details. Don't actually do anything with the uploaded file.
 */
static void web_action_upload(Webs *wp, char *path, char *query)
{
    WebsKey         *s;
    WebsUpload      *up;
    //char            *upfile;

    websSetStatus(wp, 200);
    websWriteHeaders(wp, -1, 0);
    websWriteHeader(wp, "Content-Type", "text/plain");
    websWriteEndHeaders(wp);
    if (scaselessmatch(wp->method, "POST")) {
        for (s = hashFirst(wp->files); s; s = hashNext(wp->files, s)) {
            up = s->content.value.symbol;
/*            websWrite(wp, "FILE: %s\r\n", s->name.value.string);
            websWrite(wp, "FILENAME=%s\r\n", up->filename);
            websWrite(wp, "CLIENT=%s\r\n", up->clientFilename);
            websWrite(wp, "TYPE=%s\r\n", up->contentType);
            websWrite(wp, "SIZE=%d\r\n", up->size);*/
            //upfile = sfmt("%s/tmp/%s", websGetDocuments(), up->clientFilename);
            if (rename(up->filename, "/home/zhurish/Downloads/tftpboot/tmp-0") < 0) {
                error("Cannot rename uploaded file: %s to %s, errno %d", up->filename, "/home/zhurish/Downloads/tftpboot/tmp-0", errno);
            }
            //wfree(upfile);
        }
/*        websWrite(wp, "\r\nVARS:\r\n");
        for (s = hashFirst(wp->vars); s; s = hashNext(wp->vars, s)) {
            websWrite(wp, "%s=%s\r\n", s->name.value.string, s->content.value.string);
        }*/
    }
    websDone(wp);
}

#define WEB_UPLOAD_BASE ME_GOAHEAD_UPLOAD_DIR
static int jst_web_file_list(int eid, webs_t wp, int argc, char **argv)
{
	DIR *dir = NULL;
	struct dirent *d = NULL;
	struct stat fsize;
	char filetmp[256], i = 0;
	memset (filetmp, 0, sizeof(filetmp));
	/* Open the /proc directory. */
	dir = opendir(WEB_UPLOAD_BASE);
	if (!dir)
	{
		return -1;
	}
	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{
		if(strcmp(d->d_name,".")==0||strcmp(d->d_name,"..")==0)
			continue;
		else if(d->d_type == 8)    ///file
		{
			snprintf (filetmp, sizeof(filetmp), "%s/%s", WEB_UPLOAD_BASE, d->d_name);
			(void) stat (filetmp, &fsize);

			websWrite(wp, "<tr>");
			websWrite(wp, "<td align=\"center\" name=\"%s\" id=\"%s\">%s</td>", d->d_name, d->d_name, d->d_name);
			//websWrite(wp, "<td align=\"center\"><input type=\"text\" class=\"cbi-input-text\" readonly=\"true\""\
			//		"name=\"%s\" id=\"%s\" value=\"%s\"></td>", d->d_name, d->d_name, d->d_name);

			websWrite(wp, "<td align=\"center\">%s</td>", os_file_size(fsize.st_size));
			websWrite(wp, "<td align=\"center\"><input type=\"button\" id=\"remove%d\""\
					"class=\"cbi-button-remove\" value=\"Remove\" onclick=\"button_onclick('remove%d')\"></td>", i, i);
			if(strstr(d->d_name, "ipk"))
				websWrite(wp, "<td align=\"center\"><input type=\"button\" class=\"cbi-button-apply\" value=\"Install\"></td>");
			else
				websWrite(wp, "<td align=\"center\"><input type=\"button\" class=\"cbi-button-apply\" disabled=\"true\" value=\"Install\"></td>");
			websWrite(wp, "<tr>");

		}
		i++;
/*		else if(ptr->d_type == 10)    ///link file
			printf("d_name:%s/%s\n",basePath,ptr->d_name);
		else if(ptr->d_type == 4)    ///dir
		{
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			readFileList(base);
		}*/
	}
	closedir(dir);
    return 0;
}

int web_updownload_app(char *actionname)
{
	websDefineAction("upload", web_action_upload);
	websDefineJst("jst_web_filelist", jst_web_file_list);
	return 0;
}
#if 0
/*
 * Download
 */

static void avolfileClose(){
	//wfree(websIndex);
	//websIndex = NULL;
	//wfree(websDocuments);
	//websDocuments = NULL;
}


static bool avolfileHandler(Webs *wp)
{
	WebsFileInfo info;
	char *tmp, *date;
	ssize nchars;
	int code;

	char* pathfilename; //带路径的文件名 用于找到对应的文件
	char* filenameExt; //文件扩展名 用于 设置 MIME类型
	char* filename; //文件名 用于下载后保存的文件名称
	char* disposition; //临时保存 附件 标识

	assert(websValid(wp));
	assert(wp->method);
	assert(wp->filename && wp->filename[0]);

	// 取download.lua?video=C:\aaa.wmv 中的 C:\aaa.wmv
	pathfilename = websGetVar(wp, "video", NULL);
	if (pathfilename == NULL)
		return true;

	//取文件名和扩展名
	filename = sclone(getUrlLastSplit(sclone(pathfilename), "\\"));
	filenameExt = sclone(getUrlLastSplit(sclone(filename), "."));

	if (wp->ext)
		wfree(wp->ext);

	wp->ext = (char*) walloc(1 + strlen(filenameExt) + 1);
	sprintf(wp->ext, ".%s", sclone(filenameExt));
	free(filenameExt);
	filenameExt = NULL;

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
		return true;
	}
	if (websPageOpen(wp, O_RDONLY | O_BINARY, 0666) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s",
				wp->path);
		return true;
	}
	if (websPageStat(wp, &info) < 0)
	{
		websError(wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
		return true;
	}
	code = 200;
	if (wp->since && info.mtime <= wp->since)
	{
		code = 304;
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
		return true;
	}
	websSetBackgroundWriter(wp, fileWriteEvent);
	return true;
}

static int web_action_downLoad(Webs *wp, char *path, char *query)
{
	WebsHandlerProc service = (*wp).route->handler->service;
	(*wp).route->handler->close = (*avolfileClose);
	(*wp).route->handler->service =(*avolfileHandler);
	(*wp).route->handler->service(wp);
	(*wp).route->handler->service= service;
	return 0;
}
#endif
#endif

/*
    file.c -- File handler

    This module serves static file documents
 */

/********************************* Includes ***********************************/

#include    "goahead.h"

/*********************************** Locals ***********************************/

static char   *websIndex = NULL;                   /* Default page name */
static char   *websDocuments = NULL;               /* Default Web page directory */
static char   *websCfgBaseDir = NULL;               /* Default Web page directory */

/**************************** Forward Declarations ****************************/

static void fileWriteEvent(Webs *wp);

/*********************************** Code *************************************/
/*
    Serve static files
    Return true to indicate the request was handled, even for errors.
 */
static bool fileHandler(Webs *wp)
{
	WebsFileInfo info;
	char *tmp, *date;
	ssize nchars;
	int code;

	web_assert(websValid (wp));
	web_assert(wp->method);
	web_assert(wp->filename && wp->filename[0]);

#if (ME_ROM==0)
	if (smatch (wp->method, "DELETE"))
	{
		if (unlink (wp->filename) < 0)
		{
			websError (wp, HTTP_CODE_NOT_FOUND, "Cannot delete the URI");
		}
		else
		{
			/* No content */
			websResponse (wp, 204, 0);
		}
	}
	else if (smatch (wp->method, "PUT"))
	{
		/* Code is already set for us by processContent() */
		websResponse (wp, wp->code, 0);

	}
	else
#endif /* (ME_ROM==0) */
	{
		/*
		 If the file is a directory, redirect using the nominated default page
		 */
		if (websPageIsDirectory (wp))
		{
			nchars = strlen (wp->path);
			if (wp->path[nchars - 1] == '/' || wp->path[nchars - 1] == '\\')
			{
				wp->path[--nchars] = '\0';
			}
			tmp = sfmt ("%s/%s", wp->path, websIndex);
			websRedirect (wp, tmp);
			wfree (tmp);
			return 1;
		}
		if (websPageOpen (wp, O_RDONLY | O_BINARY, 0666) < 0)
		{
#if ME_DEBUG
			if (wp->referrer)
			{
				web_trace(WEBS_DEBUG, "From %s", wp->referrer);
			}
#endif
			websError (wp, HTTP_CODE_NOT_FOUND, "Cannot open document for: %s",
					   wp->path);
			return 1;
		}
		if (websPageStat (wp, &info) < 0)
		{
			websError (wp, HTTP_CODE_NOT_FOUND, "Cannot stat page for URL");
			return 1;
		}
		code = 200;
		if (wp->since && info.mtime <= wp->since)
		{
			code = 304;
			info.size = 0;
		}
		websSetStatus (wp, code);
		websWriteHeaders (wp, info.size, 0);
		if ((date = websGetDateString (&info)) != NULL)
		{
			websWriteHeader (wp, "Last-Modified", "%s", date);
			wfree (date);
		}
		websWriteEndHeaders (wp);

		/*
		 All done if the browser did a HEAD request
		 */
		if (smatch (wp->method, "HEAD"))
		{
			websDone (wp);
			return 1;
		}
		if (info.size > 0)
        {
		    wp->txRemaining = info.size;
			websSetBackgroundWriter (wp, fileWriteEvent);
		}
		else
		{
			websDone (wp);
		}
	}
	return 1;
}


/*
    Do output back to the browser in the background. This is a socket write handler.
    This bypasses the output buffer and writes directly to the socket.
 */
static void fileWriteEvent(Webs *wp)
{
    char    *buf;
    ssize   len, wrote, size;
    int     err;

    web_assert(wp);
    web_assert(websValid(wp));

    if ((buf = walloc(ME_GOAHEAD_LIMIT_BUFFER)) == NULL) {
        websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot get memory");
        return;
    }
    while (wp->txRemaining > 0) {
        size = min(wp->txRemaining, ME_GOAHEAD_LIMIT_BUFFER);
        len = websPageReadData(wp, buf, size);
        if (len <= 0) {
            websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot read file content");
            return;
        }
        if ((wrote = websWriteSocket(wp, buf, len)) < 0) {
            err = socketGetError(wp->sid);
            if (err == EWOULDBLOCK || err == EAGAIN) {
                websPageSeek(wp, -len, SEEK_CUR);
            } else {
                /* Will call websDone below */
                wp->state = WEBS_COMPLETE;
            }
            break;
        }
        wp->txRemaining -= wrote;
        if (wrote != len) {
            websPageSeek(wp, - (len - wrote), SEEK_CUR);
            break;
        }
    }
    wfree(buf);
    if (wp->txRemaining <= 0) {
        websDone(wp);
    }
}


#if (ME_ROM==0)
PUBLIC bool websProcessPutData(Webs *wp)
{
    ssize   nbytes;

    web_assert(wp);
    web_assert(wp->putfd >= 0);
    web_assert(wp->input.buf);

    nbytes = bufLen(&wp->input);
    wp->putLen += nbytes;
    if (wp->putLen > ME_GOAHEAD_LIMIT_PUT) {
        websError(wp, HTTP_CODE_REQUEST_TOO_LARGE | WEBS_CLOSE, "Put file too large");

    } else if (write(wp->putfd, wp->input.servp, (int) nbytes) != nbytes) {
        websError(wp, HTTP_CODE_INTERNAL_SERVER_ERROR | WEBS_CLOSE, "Cannot write to file");
    }
    websConsumeInput(wp, nbytes);
    return 1;
}
#endif


static void fileClose(void)
{
    wfree(websIndex);
    websIndex = NULL;
    wfree(websDocuments);
    websDocuments = NULL;
	wfree(websCfgBaseDir);
	websCfgBaseDir = NULL;
}


PUBLIC void websFileOpen(void)
{
    websIndex = sclone("index.html");
    websDefineHandler("file", 0, fileHandler, fileClose, 0);
}


/*
    Get the default page for URL requests ending in "/"
 */
PUBLIC cchar *websGetIndex(void)
{
    return websIndex;
}


PUBLIC char *websGetDocuments(void)
{
    return websDocuments;
}
PUBLIC char *websGetCfgBaseDir(void)
{
    return websCfgBaseDir;
}


/*
    Set the default page for URL requests ending in "/"
 */
PUBLIC void websSetIndex(cchar *page)
{
    web_assert(page && *page);

    if (websIndex) {
        wfree(websIndex);
    }
    websIndex = sclone(page);
}


/*
    Set the default web directory
 */
PUBLIC void websSetDocuments(cchar *dir)
{
    web_assert(dir && *dir);
    if (websDocuments) {
        wfree(websDocuments);
    }
    websDocuments = sclone(dir);
}
PUBLIC void websSetCfgBaseDir(cchar *dir)
{
    web_assert(dir && *dir);
    if (websCfgBaseDir) {
        wfree(websCfgBaseDir);
    }
    websCfgBaseDir = sclone(dir);
}
/*
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis GoAhead open source license or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */

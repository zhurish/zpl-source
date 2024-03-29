/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zpl_type.h"
#include "os_url.h"


/*
 * tftp://1.1.1.1:80/file
 * tftp://1.1.1.1/file
 * tftp://1.1.1.1:80:/file
 * tftp://1.1.1.1:/file
 * ftp://user@1.1.1.1:80/file
 * ftp://user:password@1.1.1.1:80/file
 * ftp://user@1.1.1.1/file
 * ftp://user:password@1.1.1.1/file
 *
 * scp://user@1.1.1.1:80/file
 * scp://user:password@1.1.1.1:80/file
 * scp://user@1.1.1.1/file
 * scp://user:password@1.1.1.1/file
 *
 * scp://user@1.1.1.1:80:/file
 * scp://user:password@1.1.1.1:80:/file
 * scp://user@1.1.1.1:/file
 * scp://user:password@1.1.1.1:/file
 *
 * ssh://user@1.1.1.1:80
 * ssh://user:password@1.1.1.1:80
 * ssh://user@1.1.1.1
 * ssh://user:password@1.1.1.1
 *
 * scp  global@194.169.13.45:/home/global/workspace/test/ipran_u3-20180609.tar.bz2
 * scp -P 9225 root@183.63.84.114:/root/ipran_u3-w.tat.bz ./
 * rtsp://admin:abc123456@192.168.3.64:554/av0_0
 * rtsp://admin:abc123456@192.168.1.64/av0_0
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&rtsp
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&multcast
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&tls
 * rtsp://user:pass@192.168.1.1/media/channel=1&level=1
 * rtsp://192.168.1.1:9988/media/channel=1&level=1
 * rtsp://192.168.1.1:9988/media/test.h264&multcast
 * rtsp://192.168.1.1:9988/media/test.h264
 */
int os_url_split(const zpl_char * URL, os_url_t *spliurl)
{
	zpl_char tmp[128];
	zpl_char buf[128];
	if(URL == NULL || !spliurl)
		return ERROR;
	zpl_char *url_dup = URL;
	zpl_char *p_slash = NULL, *p = NULL;

	p_slash = strstr(url_dup, "://");

	if(!p_slash)
		return ERROR;
	memset(tmp, 0, sizeof(tmp));
	sscanf(url_dup, "%[^:]", tmp);
	spliurl->proto = strdup(tmp);
	p_slash += 3;

	if(!p_slash)
	{
		return ERROR;
	}

	if(!strstr(p_slash, "@"))
	{
split_agent:
		p = strstr(p_slash, ":/");
		if(p)
		{
			p++;
			if(p[0] == '/')
			{
				char *bp = strrchr(p, '/');
				spliurl->filename = strdup(bp+1);
				memset(tmp, 0, sizeof(tmp));
				strncpy(tmp, p, bp - p);
				spliurl->path = strdup(tmp);
			}
			else
				spliurl->filename = strdup(p);
			memset(buf, 0, sizeof(buf));
			strncpy(buf, p_slash, p - p_slash);
			p_slash = buf;
			p = strstr(p_slash, ":");
			if(p)
			{
				memset(tmp, 0, sizeof(tmp));
				sscanf(p_slash, "%[^:]", tmp);
				spliurl->host = strdup(tmp);
				p++;
				//port
				spliurl->port = atoi(p);
			}
			else
			{
				spliurl->host = strdup(p_slash);
			}
		}
		else
		{
			p = strstr(p_slash, ":");
			if(p)
			{
				memset(tmp, 0, sizeof(tmp));
				sscanf(p_slash, "%[^:]", tmp);
				spliurl->host = strdup(tmp);
				p++;
				//port
				spliurl->port = atoi(p);
				p_slash = strstr(p, "/");
				if(p_slash)
					p_slash++;
			}
			else
			{
				p = strstr(p_slash, "/");
				if(p)
				{
					memset(tmp, 0, sizeof(tmp));
					sscanf(p_slash, "%[^/]", tmp);
					spliurl->host = strdup(tmp);
					p_slash = p + 1;
				}
				else
				{
					//for ssh
					spliurl->host = strdup(p_slash);
					p_slash = NULL;
				}
			}
			if(p_slash)
			{
				if(p_slash[0] == '/')
				{
					char *bp = strrchr(p_slash, '/');
					spliurl->filename = strdup(bp+1);
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, p_slash, bp - p_slash);
					spliurl->path = strdup(tmp);
				}
				else
					spliurl->filename = strdup(p_slash);
			}
		}
	}
	else
	{
		url_dup = p = strstr(p_slash, "@");
		if(p)
		{
			memset(buf, 0, sizeof(buf));
			strncpy(buf, p_slash, p - p_slash);
			p_slash = buf;
			p = strstr(p_slash, ":");
			if(p)
			{
				memset(tmp, 0, sizeof(tmp));
				sscanf(p_slash, "%[^:]", tmp);
				spliurl->user = strdup(tmp);
				p++;
				//pass
				spliurl->pass = strdup(p);
			}
			else
			{
				spliurl->user = strdup(p_slash);
			}

			p_slash = url_dup + 1;
			if(p_slash)
				goto split_agent;
/*			if(p_slash && !strstr(spliurl->proto, "ssh"))
				goto split_agent;
			if(strstr(spliurl->proto, "ssh"))
			{

			}*/
		}
	}

	if(strstr(spliurl->proto, "ssh"))
	{
		if(spliurl->proto && spliurl->host)
		{
			strcpy(spliurl->url, URL);
			return OK;
		}
		return ERROR;
	}
	if(spliurl->proto && spliurl->host && spliurl->filename)
	{
		strcpy(spliurl->url, URL);
		return OK;
	}
	return ERROR;
}

#if 1

static int os_url_debug_test(zpl_char *URL)
{
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	if (os_url_split(URL, &spliurl) != OK)
	{
		//os_url_free(&spliurl);
		//return -1;
	}

	fprintf(stdout, "================================================== \r\n");
	fprintf(stdout, "URL            :%s\r\n", URL);
	if(spliurl.proto)
	{
		fprintf(stdout, " proto         :%s\r\n", spliurl.proto);
	}
	if(spliurl.host)
	{
		fprintf(stdout, " host          :%s\r\n", spliurl.host);
	}
	if(spliurl.port)
	{
		fprintf(stdout, " port          :%d\r\n", spliurl.port);
	}
	if(spliurl.path)
	{
		fprintf(stdout, " path          :%s\r\n", spliurl.path);
	}
	if(spliurl.filename)
	{
		fprintf(stdout, " filename      :%s\r\n", spliurl.filename);
	}
	if(spliurl.user)
	{
		fprintf(stdout, " user          :%s\r\n", spliurl.user);
	}
	if(spliurl.pass)
	{
		fprintf(stdout, " pass          :%s\r\n", spliurl.pass);
	}
	fprintf(stdout, "===================================================\r\n");
	os_url_free(&spliurl);
	return OK;
}

int os_url_test(void)
{
	/*
	os_url_debug_test("tftp://1.1.1.1:80/file");
	os_url_debug_test("tftp://1.1.1.1/file");
	os_url_debug_test("tftp://1.1.1.1:80:/file");
	os_url_debug_test("tftp://1.1.1.1:/file");
	os_url_debug_test("ftp://user@1.1.1.1:80/file");
	os_url_debug_test("ftp://user:password@1.1.1.1:80/file");
	os_url_debug_test("ftp://user@1.1.1.1/file");
	os_url_debug_test("ftp://user:password@1.1.1.1/file");
	os_url_debug_test("scp://user@1.1.1.1:80/file");
	os_url_debug_test("scp://user:password@1.1.1.1:80/file");
	os_url_debug_test("scp://user@1.1.1.1/file");
	os_url_debug_test("scp://user:password@1.1.1.1/file");
	os_url_debug_test("scp://user@1.1.1.1:80:/file");
	os_url_debug_test("scp://user:password@1.1.1.1:80:/file");
	os_url_debug_test("scp://user@1.1.1.1:/file");
	os_url_debug_test("scp://user:password@1.1.1.1:/file");

	os_url_debug_test("ssh://user@1.1.1.1:80");
	os_url_debug_test("ssh://user:password@1.1.1.1:80");
	os_url_debug_test("ssh://user@1.1.1.1");
	os_url_debug_test("ssh://user:password@1.1.1.1");
	//proto://[user[:password@]] ip [:port][:][/file]
*/
	os_url_debug_test("ftp://zhurish:centos@127.0.0.1/home/zhurish/fsdd.pdf");
	os_url_debug_test("ftp://zhurish:centos@127.0.0.1:/home/zhurish/fsdd.pdf");

	// copy url-string ftp://zhurish:centos@127.0.0.1/home/zhurish/fsdd.pdf aa.pdf


	os_url_debug_test("rtsp://admin:abc123456@192.168.3.64:554/av0_0");
	os_url_debug_test("rtsp://admin:abc123456@192.168.1.64/av0_0");
	os_url_debug_test("rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1");
	os_url_debug_test("rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&rtsp");
	os_url_debug_test("rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&multcast");
	os_url_debug_test("rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&tls");
	os_url_debug_test("rtsp://user:pass@192.168.1.1/media/channel=1&level=1");
	os_url_debug_test("rtsp://192.168.1.1:9988/media/channel=1&level=1");
	os_url_debug_test("rtsp://192.168.1.1:9988/media/test.h264&multcast");
	os_url_debug_test("rtsp://192.168.1.1:9988/media/test.h264");
	return 0;
}
#endif
#if 0
int os_url_split(const zpl_char * URL, os_url_t *spliurl)
{
	zpl_char tmp[128];
	if(URL == NULL || !spliurl)
		return ERROR;
	zpl_char *url_dup = URL;
	zpl_char *p_slash = NULL;
	if(!strstr(url_dup, "://"))
		return ERROR;

	p_slash = strstr(url_dup, "@");
	if(p_slash)
	{
		p_slash = strstr(url_dup, "://");
		if(p_slash)
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(url_dup, "%[^:]", tmp);
			spliurl->proto = strdup(tmp);

			p_slash += 3;
		}
		else
			p_slash = url_dup;


		if(strstr(p_slash, ":"))
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(p_slash, "%[^:]", tmp);
			spliurl->user = strdup(tmp);

			p_slash = strstr(p_slash, ":");
			p_slash++;
			if(*p_slash == '@')
				p_slash++;
			else
			{
				memset(tmp, 0, sizeof(tmp));
				sscanf(p_slash, "%[^@]", tmp);
				spliurl->pass = strdup(tmp);
				p_slash = strstr(p_slash, "@");
				p_slash++;
			}
		}
		else
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(p_slash, "%[^@]", tmp);
			spliurl->user = strdup(tmp);
			p_slash = strstr(p_slash, "@");
			p_slash++;

/*			memset(tmp, 0, sizeof(tmp));
			sscanf(p_slash, "%[^@]", tmp);
			spliurl->pass = strdup(tmp);
			p_slash = strstr(p_slash, "@");
			p_slash++;*/
		}
	}
	else
	{
		p_slash = strstr(url_dup, "://");
		if(p_slash)
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(url_dup, "%[^:]", tmp);
			spliurl->proto = strdup(tmp);

			p_slash += 3;
		}
		else
			p_slash = url_dup;
	}
	if(strstr(p_slash, ":"))
	{
		memset(tmp, 0, sizeof(tmp));
		sscanf(p_slash, "%[^:]", tmp);
		spliurl->host = strdup(tmp);
		p_slash = strstr(p_slash, ":");
		p_slash++;
		spliurl->port = atoi(p_slash);
		p_slash = strstr(p_slash, "/");
	}
	else
	{
		if(p_slash && strstr(p_slash, "/"))
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(p_slash, "%[^/]", tmp);
			spliurl->host = strdup(tmp);
			p_slash += strlen(spliurl->host);
		}
		else if(p_slash)
		{
			spliurl->host = strdup(p_slash);
			//p_slash += strlen(spliurl->host);
			p_slash = NULL;
		}
	}
	if(strstr(spliurl->proto, "ssh"))
	{
		if(spliurl->proto && spliurl->host)
			return OK;
		return ERROR;
	}
	if(p_slash && strstr(p_slash, "/"))
	{
		p_slash++;
		if(p_slash && strlen(p_slash))
		{
			spliurl->filename = strdup(p_slash);
			if(spliurl->proto && spliurl->host && spliurl->filename)
				return OK;
			return ERROR;
		}
	}
	return ERROR;
}
#endif
int os_url_free(os_url_t *spliurl)
{
	if(!spliurl)
		return ERROR;
	if(spliurl->proto)
	{
		free(spliurl->proto);
		spliurl->proto = NULL;
	}
	if(spliurl->host)
	{
		free(spliurl->host);
		spliurl->host = NULL;
	}
	if(spliurl->path)
	{
		free(spliurl->path);
		spliurl->path = NULL;
	}
	if(spliurl->filename)
	{
		free(spliurl->filename);
		spliurl->filename = NULL;
	}
	if(spliurl->user)
	{
		free(spliurl->user);
		spliurl->user = NULL;
	}
	if(spliurl->pass)
	{
		free(spliurl->pass);
		spliurl->pass = NULL;
	}
	return OK;
}

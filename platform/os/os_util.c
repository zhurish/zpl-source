/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <log.h>
#include <os_util.h>


#define PROC_BASE "/proc"


static int os_log_file_printf(FILE *fp, const char *buf, va_list args)
{
	if(fp)
	{
		vfprintf(fp, buf, args);
		fprintf(fp, "\n");
		fflush(fp);
	}
	return OK;
}

void os_log(char *file, const char *format, ...)
{
	FILE *fp = fopen(file, "a+");
	if(fp)
	{
		va_list args;
		va_start(args, format);
		os_log_file_printf(fp, format, args);
		//vzlog(zlog_default, module, priority, format, args);
		va_end(args);
		fclose(fp);
	}
}


int super_system(const char *cmd)
{
	int ret = 0;
	errno = 0;
	ret = system(cmd);
	if(ret == -1 || ret == 127)
	{
		fprintf (stderr, "%s: execute cmd: %s(%s)\r\n",__func__,cmd,strerror (errno) );
		return ERROR;
	}
/*	if (WIFEXITED(ret))
	{
		if (0 == WEXITSTATUS(ret))
		{
			return OK;
		}
		else
		{
			fprintf (stderr, "%s: run shell script fail: script exit code: %d\r\n",__func__, WEXITSTATUS(ret) );
			return ERROR;
		}
	}
	else
	{
		fprintf (stderr, "%s: exit code: %d\r\n",__func__, WEXITSTATUS(ret) );
	}*/
	return ret;
}


int super_output_system(const char *cmd, char *output, int len)
{
	FILE * fp = NULL;
	fp = popen(cmd, "r");
	if(fp)
	{
		int offset = 0;
		while(fgets(output + offset, len - offset, fp) != NULL)
		{
		   //printf("%s", output);
		   offset += strlen(output);
		}
		//if(fread(output, len, 1, fp))
		{
			pclose(fp);
			return OK;
		}
		pclose(fp);
		return ERROR;
	}
	fprintf (stderr, "%s: execute cmd: %s(%s)",__func__,cmd,strerror (errno) );
	return ERROR;
}

int super_input_system(const char *cmd, char *input)
{
	FILE * fp = NULL;
	fp = popen(cmd, "w");
	if(fp)
	{
		if(fwrite(input, strlen(input), 1, fp))
		{
			pclose(fp);
			return OK;
		}
		pclose(fp);
		return ERROR;
	}
	fprintf (stderr, "%s: execute cmd: %s(%s)",__func__,cmd,strerror (errno) );
	return ERROR;
}

int super_system_execvp(const char *cmd, char **input)
{
	return execvp(cmd, input);
}

char * pid2name(int pid)
{
	static char name[128];
	FILE *fp = NULL;
	char filepath[256];
	os_memset(filepath, 0, sizeof(filepath));
	snprintf(filepath, sizeof(filepath), "%s/%d/comm",PROC_BASE, pid);
	fp = fopen(filepath, "r");
	if(fp)
	{
		os_memset(name, 0, sizeof(name));
		if(fgets(name, sizeof(name), fp))
		{
			fclose(fp);
			return name;
		}
		fclose(fp);
	}
	return NULL;
}


int name2pid(const char *name)
{
	DIR *dir;
	struct dirent *d;
	int pid, i;
	char *s;
	int pnlen;
	i = 0;
	pnlen = strlen(name);
	/* Open the /proc directory. */
	dir = opendir(PROC_BASE);
	if (!dir)
	{
		printf("cannot open /proc");
		return ERROR;
	}

	/* Walk through the directory. */
	while ((d = readdir(dir)) != NULL)
	{

		char comm[PATH_MAX + 1];
		char path[PATH_MAX + 1];
		int len;
		int namelen;

		/* See if this is a process */
		if ((pid = atoi(d->d_name)) == 0)
			continue;

		snprintf(comm, sizeof(comm), "/proc/%s/comm", d->d_name);
		if ((len = readlink(comm, path, PATH_MAX)) < 0)
			continue;
		path[len] = '\0';

		/* Find ProcName */
		s = strrchr(path, '/');
		if (s == NULL)
			continue;
		s++;

		/* we don't need small name len */
		namelen = strlen(s);
		if (namelen < pnlen)
			continue;

		if (!strncmp(name, s, pnlen))
		{
			/* to avoid subname like search proc tao but proc taolinke matched */
			if (s[pnlen] == ' ' || s[pnlen] == '\0')
			{
				//foundpid[i] = pid;
				i++;
			}
		}
	}
	//foundpid[i] = 0;
	closedir(dir);
	return pid;
}

#undef PROC_BASE


pid_t os_pid_set (const char *path)
{
	FILE *fp = NULL;
	pid_t pid = 0;
	mode_t oldumask = 0;

	pid = getpid();

	oldumask = umask(0777 & ~0644);
	fp = fopen(path, "w");
	if (fp != NULL)
	{
		fprintf(fp, "%d\n", (int) pid);
		fclose(fp);
		umask(oldumask);
		return pid;
	}
	/* XXX Why do we continue instead of exiting?  This seems incompatible
	 with the behavior of the fcntl version below. */
	fprintf(stderr, "Can't fopen pid lock file %s (%s), continuing", path,
			strerror(errno));
	umask(oldumask);
	return -1;
}


pid_t os_pid_get (const char *path)
{
	FILE *fp = NULL;
	pid_t pid = 0;
	mode_t oldumask = 0;
	if (access(path, 0) == 0)
	{
		oldumask = umask(0777 & ~0644);
		fp = fopen(path, "r");
		if (fp != NULL)
		{
			fscanf(fp, "%d", &pid);
			fclose(fp);
			umask(oldumask);
			return pid;
		}
		/* XXX Why do we continue instead of exiting?  This seems incompatible
		 with the behavior of the fcntl version below. */
		fprintf(stderr, "Can't fopen pid lock file %s (%s), continuing", path,
				strerror(errno));
		umask(oldumask);
	}
	return -1;
}

int child_process_create()
{
	pid_t pid = 0;
	pid = fork();
	return pid;
}

int child_process_destroy(int pid)
{
	if(pid)
	{
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
	}
	return OK;
}

int child_process_kill(int pid)
{
	if(pid)
	{
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
	return OK;
}

int child_process_wait(int pid, int wait)
{
	if(wait == 0)
		return waitpid(pid, NULL, WNOHANG);
	else
		return waitpid(pid, NULL, 0);
}
/*
int child_process_kill(int pid)
{
	if(pid)
	{
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
	}
	return OK;
}*/


int os_write_file(const char *name, const char *string, int len)
{
	FILE *fp = fopen(name, "w+");
	if(fp)
	{
		//fprintf(fp, "%s\n", string);
		fwrite(string, len, 1, fp);
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

int os_read_file(const char *name, const char *string, int len)
{
	FILE *fp = fopen(name, "r");
	if(fp)
	{
		fread(string, len, 1, fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

int os_get_blocking(int fd)
{
	int flags = 0;

	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d get-blocking : %s", fd,
				strerror(errno));
		return -1;
	}
	if(flags & O_NONBLOCK)
		return 0;
	return 1;
}
int os_set_nonblocking(int fd)
{
	int flags = 0;

	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d set-nonblocking: %s", fd,
				strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
	{
		fprintf(stdout, "fcntl failed setting fd %d set-nonblocking: %s", fd,
				strerror(errno));
		return -1;
	}
	return 0;
}

int os_set_blocking(int fd)
{
	int flags = 0;
	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d set-blocking: %s", fd,
				strerror(errno));
		return -1;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, (flags)) < 0)
	{
		fprintf(stdout, "fcntl failed setting fd %d non-blocking: %s", fd,
				strerror(errno));
		return -1;
	}
	return 0;
}

int os_pipe_create(char *name, int mode)
{
	int fd = 0;
	char path[128];
	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.pipe", OS_PIPE_BASE, name);
	if(access(path, 0) != 0)
	{
		if(mkfifo(path, 0655) != 0)
			return ERROR;
	}
/*	O_RDONLY 只读打开
	O_WRONLY 只写打开
	O_RDWR 可读可写打开*/
	fd = open(path, mode|O_CREAT, 0644);
	if(fd <= 0)
	{
		fprintf(stdout,"can not open file %s(%s)", path, strerror(errno));
		return ERROR;
	}
	return fd;
}

int os_pipe_close(int fd)
{
	if(fd)
		close(fd);
	return OK;
}

int os_file_access(char *filename)
{
	if(access(filename, F_OK) != 0)
		return ERROR;
	return OK;
}

int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, int timeout_ms)
{
	int num = 0;
	struct timeval timer_wait = { .tv_sec = 1, .tv_usec = 0 };
	timer_wait.tv_sec = timeout_ms/1000;
	timer_wait.tv_usec = (timeout_ms%1000) * 1000;
	while(1)
	{
		num = select(maxfd, rfdset, wfdset, NULL, timeout_ms ? &timer_wait:NULL);
		if (num < 0)
		{
			//fprintf(stdout, "%s (errno=%d -> %s)", __func__, errno, strerror(errno));
			if (errno == EINTR || errno == EAGAIN)
			{
				//fprintf(stdout, "%s (errno=%d -> %s)", __func__, errno, strerror(errno));
				continue;
			}
/*			if (errno == EPIPE || errno == EBADF || errno == EIO ECONNRESET ECONNABORTED ENETRESET ECONNREFUSED)
			{
				return RES_CLOSE;
			}*/
			return -1;
		}
		else if (num == 0)
		{
			return OS_TIMEOUT;
		}
		return num;
	}
	return -1;
}


int os_write_timeout(int fd, char *buf, int len, int timeout_ms)
{
	int ret = 0;
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	ret = os_select_wait(fd + 1, NULL, &writefds, timeout_ms);
	if (ret == OS_TIMEOUT)
	{
		printf("os_select_wait timeout on write\n");
		return OS_TIMEOUT;
	}
	if (ret < 0)
	{
		printf("connect error %s\n", strerror(errno));
		return ERROR;
	}
	if (!FD_ISSET(fd, &writefds))
	{
		printf("no events on sockfd found\n");
		return ERROR;
	}
	return write(fd, buf, len);
}

int os_read_timeout(int fd, char *buf, int len, int timeout_ms)
{
	int ret = 0;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	ret = os_select_wait(fd + 1, &readfds, NULL, timeout_ms);
	if (ret == OS_TIMEOUT)
	{
		printf("os_select_wait timeout on read\n");
		return ERROR;
	}
	if (ret < 0)
	{
		printf("connect error %s\n", strerror(errno));
		return ERROR;
	}
	if (!FD_ISSET(fd, &readfds))
	{
		printf("no events on sockfd found\n");
		return ERROR;
	}
	return read(fd, buf, len);
}

int os_register_signal(int sig, void (*handler)(int
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
		))
{
	struct sigaction sa;//, osa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;

	//sigaction(sig, NULL, &osa);
#ifdef SA_SIGINFO
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_SIGINFO;
#else
	sa.sa_handler = handler;
	sa.sa_flags = 0;
#endif

	if (sigfillset(&sa.sa_mask) != 0 ||
	    sigaction(sig, &sa, NULL) < 0)
	{
		fprintf(stdout, "Unable to set up signal handler for %d, %m", sig);
		return (-1);
	}
	return (0);
}



int os_mkdir(const char *dirpath, int mode, int pathflag)
{
	int ret = 0;
	char tmp[128];
	char *p = NULL;
	static char cupwdtmp[128];
	static unsigned char cupflag = 0;
	if(cupflag == 0)
	{
		cupflag = 1;
		memset (cupwdtmp, '\0', sizeof(cupwdtmp));
		getcwd(cupwdtmp, sizeof(cupwdtmp));
	}
	if (strlen (dirpath) == 0 || dirpath == NULL)
	{
		cupflag = 0;
		printf ("strlen(dir) is 0 or dir is NULL.\n");
		return -1;
	}
	if(pathflag == 0)
	{
		if( access(dirpath,   NULL) != 0)
		{
			ret = mkdir(dirpath, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));
			chdir (cupwdtmp);
			cupflag = 0;
			return ret;
		}
		cupflag = 0;
		return -1;
	}
	memset (tmp, '\0', sizeof(tmp));
	strncpy (tmp, dirpath, strlen (dirpath));
	if (tmp[0] == '/')
		p = strchr (tmp + 1, '/');
	else
		p = strchr (tmp, '/');

	if (p)
	{
		*p = '\0';
		//printf("===============%s=========0======%s\r\n", __func__, tmp);
		if( access(tmp,   NULL) != 0)
		{
			if(mkdir(tmp, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
			{
				chdir (tmp);
			}
		}
		else
			chdir (tmp);
	}
	else
	{
		//printf("===============%s=========1======%s\r\n", __func__, tmp);
		if( access(tmp,   NULL) != 0)
		{
			if(mkdir(tmp, mode?mode:(S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) == 0)
			{
				chdir (tmp);
			}
		}
		chdir (cupwdtmp);
		cupflag = 0;
		return 0;
	}
	return os_mkdir (p + 1, mode, pathflag);
}


int os_rmdir(const char *dirpath, int pathflag)
{
	if (strlen (dirpath) == 0 || dirpath == NULL)
	{
		printf ("strlen(dir) is 0 or dir is NULL.\n");
		return -1;
	}

	char tmp[128];
	memset (tmp, '\0', sizeof(tmp));
	snprintf (tmp, sizeof(tmp), "rm %s %s", pathflag?"-rf":" ",dirpath);
	if( access(dirpath,   NULL) == 0)
	{
		system(tmp);
		if( access(dirpath,   NULL) != 0)
			return -1;
		return 0;
	}
	return 0;
}

int os_getpwddir(const char *path, int pathsize)
{
	if(getcwd(path, pathsize))
		return OK;
	return ERROR;
}
/*
int os_register_signal(int sig)
{

}
*/

int os_file_size (const char *filename)
{
#if 0
	if(!filename)
		return ERROR;
	int filesize = -1;
	FILE *fp = NULL;
	fp = fopen(filename, "r");
	if(fp == NULL)
		return filesize;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);
	return filesize;
#else
	struct stat fsize;
	if(!filename)
		return ERROR;
	if (access (filename, F_OK) >= 0)
	{
		memset (&fsize, 0, sizeof(struct stat));
		if(stat (filename, &fsize) == 0)
			return fsize.st_size;
	}
#endif
	return ERROR;
}

#define KB_SIZE_MASK	(0X000003FF)

const char * os_file_size_string(u_int32 len)
{
	u_int glen = 0, mlen = 0, klen = 0, tlen = 0;
	static char buf[64];
	memset(buf, 0, sizeof(buf));
	tlen = (len >> 40) & KB_SIZE_MASK;
	glen = (len >> 30) & KB_SIZE_MASK;
	mlen = (len >> 20) & KB_SIZE_MASK;
	klen = (len >> 10) & KB_SIZE_MASK;
	if(tlen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d T",tlen, glen);
	}
	else if(glen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d G",glen, mlen);
	}
	else if(mlen > 0)
	{
		snprintf(buf, sizeof(buf), "%d.%02d M",mlen, klen);
	}
	else if(klen > 0)
	{
		snprintf(buf, sizeof(buf), "%u.%02u K",klen, (len) & KB_SIZE_MASK);
	}
	else
	{
		snprintf(buf, sizeof(buf), "%d Byte",len);
	}
	return buf;
}

/*const char * os_stream_size(long long len)
{
	return os_file_size(len);
}*/

#undef KB_SIZE_MASK



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
 */
int os_url_split(const char * URL, os_url_t *spliurl)
{
	char tmp[128];
	char buf[128];
	if(URL == NULL || !spliurl)
		return ERROR;
	char *url_dup = URL;
	char *p_slash = NULL, *p = NULL;

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
				spliurl->filename = strdup(p_slash);
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
			return OK;
		return ERROR;
	}
	if(spliurl->proto && spliurl->host && spliurl->filename)
		return OK;
	return ERROR;
}

#if 0
static int os_url_debug_test(char *URL)
{
	os_url_t spliurl;
	memset(&spliurl, 0, sizeof(os_url_t));
	if (os_url_split(URL, &spliurl) != OK)
	{
		//os_url_free(&spliurl);
		//return -1;
	}
	fprintf(stdout, "===================================================\n");
	fprintf(stdout, "URL            :%s\n", URL);
	if(spliurl.proto)
	{
		fprintf(stdout, " proto         :%s\n", spliurl.proto);
	}
	if(spliurl.host)
	{
		fprintf(stdout, " host          :%s\n", spliurl.host);
	}
	if(spliurl.port)
	{
		fprintf(stdout, " port          :%d\n", spliurl.port);
	}
	if(spliurl.path)
	{
		fprintf(stdout, " path          :%s\n", spliurl.path);
	}
	if(spliurl.filename)
	{
		fprintf(stdout, " filename      :%s\n", spliurl.filename);
	}
	if(spliurl.user)
	{
		fprintf(stdout, " user          :%s\n", spliurl.user);
	}
	if(spliurl.pass)
	{
		fprintf(stdout, " pass          :%s\n", spliurl.pass);
	}
	fprintf(stdout, "===================================================\n");
	os_url_free(&spliurl);
	return OK;
}

int os_url_test()
{
/*	os_url_debug_test("tftp://1.1.1.1:80/file");
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
	os_url_debug_test("ssh://user:password@1.1.1.1");*/
	//proto://[user[:password@]] ip [:port][:][/file]

	os_url_debug_test("rtsp://admin:abc123456@192.168.3.64:554/av0_0");
	os_url_debug_test("rtsp://admin:abc123456@192.168.1.64/av0_0");
	return 0;
}
#endif
#if 0
int os_url_split(const char * URL, os_url_t *spliurl)
{
	char tmp[128];
	if(URL == NULL || !spliurl)
		return ERROR;
	char *url_dup = URL;
	char *p_slash = NULL;
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

int os_thread_once(int (*entry)(void *), void *p)
{
	int td_thread = 0;
	if (pthread_create(&td_thread, NULL,
			entry, (void *) p) == 0)
		return td_thread;
	return ERROR;
}


int fdprintf
    (
    int fd,       /* fd of control connection socket */
	const char *format, ...
    )
    {
		va_list args;
		char buf[1024];
		int len = 0;
		memset(buf, 0, sizeof(buf));
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);

		if(len <= 0)
			return ERROR;
		return write(fd, buf, len);
    }


int hostname_ipv4_address(char *hostname, struct in_addr *addr)
{
	if(!addr)
		return ERROR;
	if(inet_aton (hostname, addr) == 0)
	{
		struct hostent * hoste = NULL;
		hoste = gethostbyname(hostname);
		if (hoste && hoste->h_addr_list && hoste->h_addrtype == AF_INET)
		{
			memcpy(addr, (struct in_addr*)hoste->h_addr_list[0], sizeof(struct in_addr));
			return OK;
		}
		return ERROR;
	}
	return OK;
}

int hostname_ipv6_address(char *hostname, struct in6_addr *addr)
{
	if(!addr)
		return ERROR;
	if(inet_pton (AF_INET6, hostname, addr) == 0)
	{
		struct hostent * hoste = NULL;
		hoste = gethostbyname(hostname);
		if (hoste && hoste->h_addr_list && hoste->h_addrtype == AF_INET)
		{
			memcpy(addr, (struct in6_addr*)hoste->h_addr_list[0], sizeof(struct in6_addr));
			//addr = *(struct in6_addr*)hoste->h_addr_list[0];
			return OK;
		}
		return ERROR;
	}
	//addr->s_addr = inet6_addr(hostname);
	return OK;
}


#ifdef DOUBLE_PROCESS

static int os_process_sock = 0;


static int os_process_split(process_head *head, process_action action, char *name,
		char *process, BOOL restart, char *argv[])
{
	int i = 0;
	head->action = action;
	head->restart = restart;
	if(name)
		os_strcpy(head->name, name);
	if(process)
		os_strcpy(head->process, process);
	if(argv)
	{
		for(i = 0; ; i++)
		{
			if(argv[i])
			{
				os_strcat(head->argvs, argv[i]);
				if(argv[i+1])
					os_strcat(head->argvs, " ");
			}
			else
				break;
		}
	}
	return OK;
}

int os_process_register(process_action action, char *name,
		char *process, BOOL restart, char *argv[])
{
	int ret = 0;
	process_head head;
	if(os_process_sock == 0)
	{
		os_process_sock = unix_sock_client_create(TRUE, PROCESS_MGT_UNIT_NAME);
	}
	if(os_process_sock <= 0)
		return ERROR;
	os_memset(&head, 0, sizeof(head));
	os_process_split(&head,  action, name,
			process,  restart, argv);
	errno = 0;
	ret = write(os_process_sock, &head, sizeof(process_head));
	//ret = os_stream_head_write(fd, &head, sizeof(process_head));
	//zlog_debug(ZLOG_NSM, "%s:name:%s(%d byte(%s))",__func__, head.name, ret, strerror(errno));
	if( ret == sizeof(process_head))
	{
		int num = 0;
		fd_set rfdset;
		FD_ZERO(&rfdset);
		FD_SET(os_process_sock, &rfdset);
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5000);
		if(num)
		{
			int respone = 0;
			if(FD_ISSET(os_process_sock, &rfdset))
			{
				if(read(os_process_sock, &respone, 4) == 4)
					return respone;
				if (errno == EPIPE || errno == EBADF || errno == EIO || errno == ECONNRESET
						|| errno == ECONNABORTED || errno == ENETRESET || errno == ECONNREFUSED)
				{
					ip_close(os_process_sock);
					os_process_sock = 0;
				}
			}
			return ERROR;
		}
		else if(num < 0)
		{
			ip_close(os_process_sock);
			os_process_sock = 0;
			return ERROR;
		}
		else
		{
			fprintf(stdout,"wait respone timeout (%s)", strerror(errno));
			return OK;
		}
	}
	if(ret < 0)
	{
		if (errno == EPIPE || errno == EBADF || errno == EIO || errno == ECONNRESET
				|| errno == ECONNABORTED || errno == ENETRESET || errno == ECONNREFUSED)
		{
			ip_close(os_process_sock);
			os_process_sock = 0;
		}
	}
	fprintf(stdout,"can not write (%s)", strerror(errno));
	return ret;
}

int os_process_action(process_action action, char *name, int id)
{
	int ret = 0;
	process_head head;
	if(os_process_sock == 0)
	{
		os_process_sock = unix_sock_client_create(TRUE, PROCESS_MGT_UNIT_NAME);
	}
	if(os_process_sock <= 0)
		return ERROR;
	os_memset(&head, 0, sizeof(head));
	os_process_split(&head,  action, name,
			NULL,  FALSE, NULL);
	head.id = id;
	errno = 0;
	ret = write(os_process_sock, &head, sizeof(process_head));
	//ret = os_stream_head_write(fd, &head, sizeof(process_head));
	//zlog_debug(ZLOG_NSM, "%s:name:%s(%d byte(%s))",__func__, head.name, ret, strerror(errno));
	if( ret == sizeof(process_head))
	{
		int num = 0;
		fd_set rfdset;
		FD_ZERO(&rfdset);
		FD_SET(os_process_sock, &rfdset);
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5000);
		if(num)
		{
			int respone = 0;
			if(FD_ISSET(os_process_sock, &rfdset))
			{
				if(read(os_process_sock, &respone, 4) == 4)
					return respone;
				if (errno == EPIPE || errno == EBADF || errno == EIO || errno == ECONNRESET
						|| errno == ECONNABORTED || errno == ENETRESET || errno == ECONNREFUSED)
				{
					ip_close(os_process_sock);
					os_process_sock = 0;
				}
			}
			return ERROR;
		}
		else if(num < 0)
		{
			ip_close(os_process_sock);
			os_process_sock = 0;
			return ERROR;
		}
		else
		{
			fprintf(stdout,"wait respone timeout (%s)", strerror(errno));
			return OK;
		}
	}
	if(ret < 0)
	{
		if (errno == EPIPE || errno == EBADF || errno == EIO || errno == ECONNRESET
				|| errno == ECONNABORTED || errno == ENETRESET || errno == ECONNREFUSED)
		{
			ip_close(os_process_sock);
			os_process_sock = 0;
		}
	}
	fprintf(stdout,"can not write (%s)", strerror(errno));
	return ret;
}

int os_process_action_respone(int fd, int respone)
{
	int value = respone;
	if(fd)
		return write(fd, &value, 4);
	return ERROR;
}

int os_process_start()
{
	int os_process_id = 0;
	os_process_id = child_process_create();
	if(os_process_id == 0)
	{
		char *plogfile = DAEMON_LOG_FILE_DIR"/ProcessMU.log";
		char *argvp[] = {"-D", "-d", "6", "-l", plogfile, NULL};
		super_system_execvp("ProcessMU", argvp);
	}
	return 0;
}

int os_process_stop()
{
	int os_process_id = os_pid_get(BASE_DIR"/run/process.pid");
	if(os_process_id > 0)
		return child_process_destroy(os_process_id);
	return 0;
}

#endif

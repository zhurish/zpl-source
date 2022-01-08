/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include "zpl_include.h"


#define PROC_BASE "/proc"



int os_pipe_create(zpl_char *name, zpl_uint32 mode)
{
	int fd = 0;
	zpl_char path[128];
	os_memset(path, 0, sizeof(path));
	os_snprintf(path, sizeof(path), "%s/%s.pipe", OS_PIPE_BASE, name);
	if (access(path, 0) != 0)
	{
		if (mkfifo(path, 0655) != 0)
			return ERROR;
	}
	/*	O_RDONLY 只读打开
		O_WRONLY 只写打开
		O_RDWR 可读可写打开*/
	fd = open(path, mode | O_CREAT, 0644);
	if (fd <= 0)
	{
		fprintf(stdout, "can not open file %s(%s)", path, strerror(ipstack_errno));
		return ERROR;
	}
	return fd;
}

int os_pipe_close(int fd)
{
	if (fd)
		close(fd);
	return OK;
}


int os_get_blocking(int fd)
{
	zpl_uint32 flags = 0;

	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d get-blocking : %s\r\n", fd,
				strerror(ipstack_errno));
		return -1;
	}
	if (flags & O_NONBLOCK)
		return 0;
	return 1;
}

int os_set_nonblocking(int fd)
{
	zpl_uint32 flags = 0;

	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d set-nonblocking: %s\r\n", fd,
				strerror(ipstack_errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
	{
		fprintf(stdout, "fcntl failed setting fd %d set-nonblocking: %s\r\n", fd,
				strerror(ipstack_errno));
		return -1;
	}
	return 0;
}

int os_set_blocking(int fd)
{
	zpl_uint32 flags = 0;
	/* According to the Single UNIX Spec, the return value for F_GETFL should
	 never be negative. */
	if ((flags = fcntl(fd, F_GETFL)) < 0)
	{
		fprintf(stdout, "fcntl(F_GETFL) failed for fd %d set-blocking: %s\r\n", fd,
				strerror(ipstack_errno));
		return -1;
	}
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, (flags)) < 0)
	{
		fprintf(stdout, "fcntl failed setting fd %d non-blocking: %s\r\n", fd,
				strerror(ipstack_errno));
		return -1;
	}
	return 0;
}

static int os_log_file_printf(FILE *fp, const zpl_char *buf, va_list args)
{
	if(fp)
	{
		vfprintf(fp, buf, args);
		fprintf(fp, "\n");
		fflush(fp);
	}
	return OK;
}

void os_log(zpl_char *file, const zpl_char *format, ...)
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

void os_vslog(zpl_char *herd, zpl_char *file, int line, const zpl_char *format, ...)
{
	va_list args;
	va_start(args, format);
	if(herd && strstr(herd,"ERROR"))
	{
		if(herd)
			fprintf(stderr, "%s", herd);
		if(file)
		{
			fprintf(stderr, "(%s:%d)", file, line);
		}
		fprintf(stderr, "(%s)", os_task_self_name_alisa());
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, "\n");
		fflush(stderr);
	}
	else
	{
		if(herd)
			fprintf(stdout, "%s", herd);
		if(file)
		{
			fprintf(stdout, "(%s:%d)", file, line);
		}
		fprintf(stdout, "(%s)", os_task_self_name_alisa());
		vfprintf(stdout, format, args);
		va_end(args);
		fprintf(stdout, "\n");
		fflush(stdout);
	}
}

zpl_char * pid2name(zpl_pid_t pid)
{
	static zpl_char name[128];
	FILE *fp = NULL;
	zpl_char filepath[256];
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


zpl_pid_t name2pid(const zpl_char *name)
{
	DIR *dir;
	struct dirent *d;
	zpl_pid_t pid;
	zpl_uint32 i;
	zpl_char *s;
	zpl_uint32 pnlen;
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

		zpl_char comm[PATH_MAX + 1];
		zpl_char path[PATH_MAX + 1];
		zpl_uint32 len;
		zpl_uint32 namelen;

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


zpl_pid_t os_pid_set (const zpl_char *path)
{
	FILE *fp = NULL;
	zpl_pid_t pid = 0;
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
			strerror(ipstack_errno));
	umask(oldumask);
	return -1;
}


zpl_pid_t os_pid_get (const zpl_char *path)
{
	FILE *fp = NULL;
	zpl_pid_t pid = 0;
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
				strerror(ipstack_errno));
		umask(oldumask);
	}
	return -1;
}




zpl_pthread_t os_thread_once(int (*entry)(void *), void *p)
{
	zpl_pthread_t td_thread = 0;
	if (pthread_create(&td_thread, NULL,
			entry, (void *) p) == 0)
		return td_thread;
	return ERROR;
}


int fdprintf
    (
    int fd,       /* fd of control connection socket */
	const zpl_char *format, ...
    )
    {
		va_list args;
		zpl_char buf[1024];
		zpl_uint32 len = 0;
		memset(buf, 0, sizeof(buf));
		va_start(args, format);
		len = vsnprintf(buf, sizeof(buf), format, args);
		va_end(args);

		if(len <= 0)
			return ERROR;
		return write(fd, buf, len);
    }


int hostname_ipv4_address(zpl_char *hostname, struct in_addr *addr)
{
	if(!addr)
		return ERROR;
	if(ipstack_inet_aton (hostname, addr) == 0)
	{
		struct ipstack_hostent * hoste = NULL;
		hoste = ipstack_gethostbyname(hostname);
		if (hoste && hoste->h_addr_list && hoste->h_addrtype == AF_INET)
		{
			memcpy(addr, (struct in_addr*)hoste->h_addr_list[0], sizeof(struct in_addr));
			return OK;
		}
		return ERROR;
	}
	return OK;
}

int hostname_ipv6_address(zpl_char *hostname, struct in6_addr *addr)
{
	if(!addr)
		return ERROR;
	if(ipstack_inet_pton (AF_INET6, hostname, addr) == 0)
	{
		struct ipstack_hostent * hoste = NULL;
		hoste = ipstack_gethostbyname(hostname);
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


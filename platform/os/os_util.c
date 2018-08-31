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

int super_system(const char *cmd)
{
	int ret = 0;
	errno = 0;
	ret = system(cmd);
	if(ret == -1 || ret == 127)
	{
		fprintf (stderr, "%s: execute cmd: %s(%s)",__func__,cmd,strerror (errno) );
		return ERROR;
	}
	return ret;
}


int super_output_system(const char *cmd, char *output, int len)
{
	FILE * fp = 0;
	fp = popen(cmd, "r");
	if(fp)
	{
		if(fread(output, len, 1, fp))
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
	FILE * fp = 0;
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
	dir = opendir("/proc");
	if (!dir)
	{
		printf("cannot open /proc");
		return -1;
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


int os_write_string(const char *name, const char *string)
{
	FILE *fp = fopen(name, "w");
	if(fp)
	{
		fprintf(fp, "%s\n", string);
		fflush(fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

int os_read_string(const char *name, const char *string, int len)
{
	FILE *fp = fopen(name, "r");
	if(fp)
	{
		fread(string, 1, len, fp);
		fclose(fp);
		return OK;
	}
	return ERROR;
}

int os_set_nonblocking(int fd)
{
  int flags;

  /* According to the Single UNIX Spec, the return value for F_GETFL should
     never be negative. */
  if ((flags = fcntl(fd, F_GETFL)) < 0)
    {
	  fprintf(stdout, "fcntl(F_GETFL) failed for fd %d: %s",
      		fd, strerror(errno));
      return -1;
    }
  if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
    {
	  fprintf(stdout, "fcntl failed setting fd %d non-blocking: %s",
      		fd, strerror(errno));
      return -1;
    }
  return 0;
}

int os_set_blocking(int fd)
{
  int flags;
  /* According to the Single UNIX Spec, the return value for F_GETFL should
     never be negative. */
  if ((flags = fcntl(fd, F_GETFL)) < 0)
    {
	  fprintf(stdout, "fcntl(F_GETFL) failed for fd %d: %s",
      		fd, strerror(errno));
      return -1;
    }
  flags &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, (flags)) < 0)
    {
	  fprintf(stdout, "fcntl failed setting fd %d non-blocking: %s",
      		fd, strerror(errno));
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
	fd = open(path, mode|O_CREAT);
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

int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, int timeout)
{
	int num = 0;
	struct timeval timer_wait = { .tv_sec = timeout, .tv_usec = 0 };
	timer_wait.tv_sec = timeout;
	while(1)
	{
		num = select(maxfd, rfdset, wfdset, NULL, timeout ? &timer_wait:NULL);
		if (num < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
/*			if (errno == EPIPE || errno == EBADF || errno == EIO ECONNRESET ECONNABORTED ENETRESET ECONNREFUSED)
			{
				return RES_CLOSE;
			}*/
			return -1;
		}
		else if (num == 0)
		{
			return 0;
		}
		return num;
	}
	return -1;
}


int os_stream_read(int fd, char *outbuf, int len)
{
	int offset = 0;
	int ret = 0;
	while(1)
	{
		ret = read(fd, outbuf + offset, len - offset);
		if(ret)
		{
			offset += ret;
			if(offset == len)
				return ret;
		}
		if(ret < 0)
			return ret;
	}
}

int os_stream_write(int fd, char *inbuf, int len)
{
	int ret = 0;
	int offset = 0;
	while(1)
	{
		ret = write(fd, inbuf + offset, len - offset);
		if(ret)
		{
			offset += ret;
			if(offset == len)
				return ret;
		}
		if(ret < 0)
			return ret;
	}
	return ERROR;
}

int os_stream_head_read(int fd, char *outbuf, int len)
{
	int offset = 0;
	int ret = 0, head = len;
	char buf[4096];
	head = sizeof(buf);
	os_memset(buf, 0, sizeof(buf));
	while(1)
	{
		ret = read(fd, buf, 2);
		if(ret)
		{
			offset += ret;

			if(offset == 2)
			{
				head = buf[0] << 8 | buf[1];
				break;
			}
		}
		if(ret < 0)
			return ret;
	}
	os_memset(buf, 0, sizeof(buf));
	head = MIN(head, sizeof(buf));
	if(os_stream_read(fd, buf, head) == head)
	{
		os_memcpy(outbuf, buf, MIN(head, len));
		return head;
	}
	return ERROR;
}

int os_stream_head_write(int fd, char *inbuf, int len)
{
	char buf[4096];
	os_memset(buf, 0, sizeof(buf));
	os_memcpy(buf + 2, inbuf, MIN(len, sizeof(buf)));
	buf[0] = len >> 8;
	buf[0] = len & 0xff;
	return os_stream_write(fd, buf,  len + 2);
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
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5);
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
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5);
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

#endif



int os_register_signal(int sig, void (*handler)(int))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;

	if (sigfillset(&sa.sa_mask) != 0 ||
	    sigaction(sig, &sa, NULL) < 0)
	{
		fprintf(stdout, "Unable to set up signal handler for %d, %m", sig);
		return (-1);
	}
	return (0);
}

#undef PROC_BASE

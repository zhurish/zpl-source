/*
 * os_util.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include <log.h>
#include <sys/wait.h>

int super_system(const zpl_char *cmd)
{
	int ret = 0;
	ipstack_errno = 0;
	ret = system(cmd);
	if(ret == -1 || ret == 127)
	{
		fprintf (stderr, "%s: execute cmd: %s(%s)\r\n",__func__,cmd,strerror (ipstack_errno) );
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


int super_output_system(const zpl_char *cmd, zpl_char *output, zpl_uint32 len)
{
	FILE * fp = NULL;
	fp = popen(cmd, "r");
	if(fp)
	{
		zpl_uint32 offset = 0;
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
	fprintf (stderr, "%s: execute cmd: %s(%s)",__func__,cmd,strerror (ipstack_errno) );
	return ERROR;
}

int super_input_system(const zpl_char *cmd, zpl_char *input)
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
	fprintf (stderr, "%s: execute cmd: %s(%s)",__func__,cmd,strerror (ipstack_errno) );
	return ERROR;
}

int super_system_execvp(const zpl_char *cmd, zpl_char **input)
{
	return execvp(cmd, input);
}




zpl_pid_t child_process_create(void)
{
	zpl_pid_t pid = 0;
	pid = fork();
	return pid;
}

int child_process_destroy(zpl_pid_t pid)
{
	if(pid)
	{
		kill(pid, SIGTERM);
		waitpid(pid, NULL, 0);
	}
	return OK;
}

int child_process_kill(zpl_pid_t pid)
{
	if(pid)
	{
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
	}
	return OK;
}

int child_process_wait(zpl_pid_t pid, zpl_uint32 wait)
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


#ifdef ZPL_TOOLS_PROCESS

static int os_process_sock = 0;


static int os_process_split(process_head *head, process_action action, zpl_char *name,
		zpl_char *process, zpl_bool restart, zpl_char *argv[])
{
	zpl_uint32 i = 0;
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

int os_process_register(process_action action, zpl_char *name,
		zpl_char *process, zpl_bool restart, zpl_char *argv[])
{
	int ret = 0;
	process_head head;
	if(os_process_sock == 0)
	{
		os_process_sock = os_sock_unix_client_create(zpl_true, PROCESS_MGT_UNIT_NAME);
	}
	if(os_process_sock <= 0)
		return ERROR;
	os_memset(&head, 0, sizeof(head));
	os_process_split(&head,  action, name,
			process,  restart, argv);
	ipstack_errno = 0;
	ret = write(os_process_sock, &head, sizeof(process_head));
	//ret = os_stream_head_write(fd, &head, sizeof(process_head));
	//zlog_debug(MODULE_NSM, "%s:name:%s(%d byte(%s))",__func__, head.name, ret, strerror(ipstack_errno));
	if( ret == sizeof(process_head))
	{
		zpl_int32 num = 0;
		fd_set rfdset;
		FD_ZERO(&rfdset);
		FD_SET(os_process_sock, &rfdset);
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5000);
		if(num)
		{
			zpl_uint32 respone = 0;
			if(FD_ISSET(os_process_sock, &rfdset))
			{
				if(read(os_process_sock, &respone, 4) == 4)
					return respone;
				if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
						|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
				{
					close(os_process_sock);
					os_process_sock = 0;
				}
			}
			return ERROR;
		}
		else if(num < 0)
		{
			close(os_process_sock);
			os_process_sock = 0;
			return ERROR;
		}
		else
		{
			fprintf(stdout,"wait respone timeout (%s)", strerror(ipstack_errno));
			return OK;
		}
	}
	if(ret < 0)
	{
		if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
				|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
		{
			close(os_process_sock);
			os_process_sock = 0;
		}
	}
	fprintf(stdout,"can not write (%s)", strerror(ipstack_errno));
	return ret;
}

int os_process_action(process_action action, zpl_char *name, zpl_uint32 id)
{
	zpl_int32 ret = 0;
	process_head head;
	if(os_process_sock == 0)
	{
		os_process_sock = os_sock_unix_client_create(zpl_true, PROCESS_MGT_UNIT_NAME);
	}
	if(os_process_sock <= 0)
		return ERROR;
	os_memset(&head, 0, sizeof(head));
	os_process_split(&head,  action, name,
			NULL,  zpl_false, NULL);
	head.id = id;
	ipstack_errno = 0;
	ret = write(os_process_sock, &head, sizeof(process_head));
	//ret = os_stream_head_write(fd, &head, sizeof(process_head));
	//zlog_debug(MODULE_NSM, "%s:name:%s(%d byte(%s))",__func__, head.name, ret, strerror(ipstack_errno));
	if( ret == sizeof(process_head))
	{
		zpl_int32 num = 0;
		fd_set rfdset;
		FD_ZERO(&rfdset);
		FD_SET(os_process_sock, &rfdset);
		num = os_select_wait(os_process_sock + 1, &rfdset, NULL, 5000);
		if(num)
		{
			zpl_uint32 respone = 0;
			if(FD_ISSET(os_process_sock, &rfdset))
			{
				if(read(os_process_sock, &respone, 4) == 4)
					return respone;
				if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
						|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
				{
					close(os_process_sock);
					os_process_sock = 0;
				}
			}
			return ERROR;
		}
		else if(num < 0)
		{
			close(os_process_sock);
			os_process_sock = 0;
			return ERROR;
		}
		else
		{
			fprintf(stdout,"wait respone timeout (%s)", strerror(ipstack_errno));
			return OK;
		}
	}
	if(ret < 0)
	{
		if (ipstack_errno == EPIPE || ipstack_errno == EBADF || ipstack_errno == EIO || ipstack_errno == ECONNRESET
				|| ipstack_errno == ECONNABORTED || ipstack_errno == ENETRESET || ipstack_errno == ECONNREFUSED)
		{
			close(os_process_sock);
			os_process_sock = 0;
		}
	}
	fprintf(stdout,"can not write (%s)", strerror(ipstack_errno));
	return ret;
}

int os_process_action_respone(int fd, zpl_uint32 respone)
{
	zpl_uint32 value = respone;
	if(fd)
		return write(fd, &value, 4);
	return ERROR;
}

int os_process_start(void)
{
	zpl_uint32 os_process_id = 0;
	os_process_id = child_process_create();
	if(os_process_id == 0)
	{
		zpl_char *plogfile = DAEMON_LOG_FILE_DIR"/ProcessMU.log";
		zpl_char *argvp[] = {"-D", "-d", "6", "-l", plogfile, NULL};
		super_system_execvp("ProcessMU", argvp);
	}
	return 0;
}

int os_process_stop(void)
{
	zpl_int32 os_process_id = os_pid_get(BASE_DIR"/run/process.pid");
	if(os_process_id > 0)
		return child_process_destroy(os_process_id);
	return 0;
}

#endif

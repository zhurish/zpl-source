/*
 * process.c
 *
 *  Created on: Aug 14, 2018
 *      Author: zhurish
 */




/*
 * process.c
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "os_util.h"
#include "process.h"
#include <sys/stat.h>




//#undef DOUBLE_PROCESS

#ifdef DOUBLE_PROCESS

static main_process_t gProcessMain;
static int main_process_id = 100;


static int process_del_node(process_t *node);


int process_init(void)
{
	os_memset(&gProcessMain, 0, sizeof(main_process_t));
	gProcessMain.list = XMALLOC(MTYPE_THREAD_MASTER, sizeof(LIST));
	if(gProcessMain.list)
	{
		gProcessMain.mutex = os_mutex_init();
		lstInit(gProcessMain.list);
		return OK;
	}
	return ERROR;
}

int process_exit(void)
{
	if(gProcessMain.list)
	{
		NODE index;
		process_t *pstNode = NULL;
		for(pstNode = (process_t *)lstFirst(gProcessMain.list);
				pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
		{
			index = pstNode->node;
			if(pstNode)
			{
				process_stop(pstNode);
				process_del_node(pstNode);
			}
		}

		if(lstCount(gProcessMain.list))
			lstFree(gProcessMain.list);
		XFREE(MTYPE_THREAD_MASTER, gProcessMain.list);
	}
	if(gProcessMain.mutex)
		os_mutex_exit(gProcessMain.mutex);
	return OK;
}


static process_t * process_lookup_node(char *name)
{
	NODE index;
	process_t *pstNode = NULL;
	assert(name);
	char pname[P_NAME_MAX];
	os_memset(pname, 0, P_NAME_MAX);
	os_strcpy(pname, name);
	for(pstNode = (process_t *)lstFirst(gProcessMain.list);
			pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(os_memcmp(pname, pstNode->name, P_NAME_MAX) == 0)
		{
			return pstNode;
		}
	}
	return NULL;
}

static process_t * process_lookup_pid_node(int pid)
{
	NODE index;
	process_t *pstNode = NULL;
	for(pstNode = (process_t *)lstFirst(gProcessMain.list);
			pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->pid == pid)
		{
			return pstNode;
		}
	}
	return NULL;
}

static process_t * process_lookup_id_node(int id)
{
	NODE index;
	process_t *pstNode = NULL;
	for(pstNode = (process_t *)lstFirst(gProcessMain.list);
			pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(pstNode->id == id)
		{
			return pstNode;
		}
	}
	return NULL;
}

static int process_add_node(process_t *process)
{
	if(process)
	{
		//process_t *node = XMALLOC(MTYPE_THREAD, sizeof(process_t));
		//if(node)
		{
			//os_memset(node, 0, sizeof(process_t));
			//os_memcpy(node, process, sizeof(process_t));
			int i =0;
			char 	argvs[P_PATH_MAX];
			os_memset(argvs, 0, sizeof(argvs));
			for(i = 0; i < P_ARGV_MAX; i++)
			{
				if(process->argv[i])
				{
					os_strcat(argvs, process->argv[i]);
					os_strcat(argvs, " ");
				}
			}
			process_log_debug("add process :%s execp: %s %s", process->name, process->process,
					os_strlen(argvs) ? argvs:" ");
			lstAdd(gProcessMain.list, (NODE*)process);
			return OK;
		}
		return ERROR;
	}
	return ERROR;
}

static int process_del_node(process_t *node)
{
	int i = 0;
	assert(node);
	if(node)
	{
		lstDelete(gProcessMain.list, (NODE*)node);
		for(i = 0; i < P_ARGV_MAX; i++)
		{
			if(node->argv[i])
				XFREE(MTYPE_LINK_NODE, node->argv[i]);
		}
		process_log_debug("del process :%s execp:%s", node->name, node->process);
		XFREE(MTYPE_THREAD, node);
		return OK;
	}
	return ERROR;
}


int process_add_api(process_t *process)
{
	int ret = 0;
	assert(process);
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	if(!process_lookup_node(process->name))
		ret = process_add_node(process);
	else
		ret = ERROR;
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return ret;
}

int process_del_api(process_t *process)
{
	int ret = ERROR;
	assert(process);
	//process_t *node = NULL;
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	//node = process_lookup_node(name);
	ret = process_del_node(process);
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return ret;
}


process_t * process_lookup_api(char *name)
{
	process_t *node = NULL;
	assert(name);
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	node = process_lookup_node(name);
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return node;
}

process_t * process_lookup_pid_api(int pid)
{
	process_t *node = NULL;
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	node = process_lookup_pid_node(pid);
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return node;
}

process_t * process_lookup_id_api(int id)
{
	process_t *node = NULL;
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	node = process_lookup_id_node(id);
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return node;
}



int process_start(process_t *process)
{
	process->active = TRUE;
	if(process->active && process->pid == 0)
	{
		int pid = 0;
		//char *argv[] = {"-f", "-i", "eth0", "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
		pid = child_process_create();
		if(pid == 0)
		{
			super_system_execvp(process->process, process->argv);
		}
		else
		{
			sleep(1);
			if(pid)
			{
				int i =0;
				char 	argvs[P_PATH_MAX];
				os_memset(argvs, 0, sizeof(argvs));
				for(i = 0; i < P_ARGV_MAX; i++)
				{
					if(process->argv[i])
					{
						os_strcat(argvs, process->argv[i]);
						os_strcat(argvs, " ");
					}
				}
				process_log_debug("start process :%s execp: %s %s", process->name, process->process,
						os_strlen(argvs) ? argvs:" ");
				process->pid = pid;
				return pid;
			}
			else
				process_log_err("start process :%s execp:%s", process->name, process->process);
		}
	}
	return 0;
}

int process_deamon_start(process_t *process)
{
	process->active = TRUE;
	if(process->active && process->pid == 0)
	{
		int i =0;
		char 	argvs[P_PATH_MAX];
		os_memset(argvs, 0, sizeof(argvs));
		os_strcat(argvs, "nohup ");
		os_strcat(argvs, process->process);
		os_strcat(argvs, " ");
		for(i = 0; i < P_ARGV_MAX; i++)
		{
			if(process->argv[i])
			{
				os_strcat(argvs, process->argv[i]);
				os_strcat(argvs, " ");
			}
		}
		super_system(argvs);
		process_log_debug("start deamon :%s execp: %s %s", process->name, process->process,
				os_strlen(argvs) ? argvs:" ");
		return 1;
	}
	return 0;
}

int process_stop(process_t *process)
{
	process_log_debug("stop process :%s execp:%s", process->name, process->process);
	if(process->active && process->pid)
	{
		process->active = FALSE;
		child_process_kill(process->pid);
		process->pid = 0;
	}
	return OK;
}


int process_restart(process_t *process)
{
	process_stop(process);
	//return process_start(process);
	process->active = TRUE;
	if(process->active && process->pid == 0)
	{
		int pid = 0;
		//char *argv[] = {"-f", "-i", "eth0", "-s", "/usr/share/udhcpc/udhcpc.script", "-S", NULL};
		pid = child_process_create();
		if(pid == 0)
		{
			super_system_execvp(process->process, process->argv);
		}
		else
		{
			sleep(1);
			if(pid)
			{
				int i =0;
				char 	argvs[P_PATH_MAX];
				os_memset(argvs, 0, sizeof(argvs));
				for(i = 0; i < P_ARGV_MAX; i++)
				{
					if(process->argv[i])
					{
						os_strcat(argvs, process->argv[i]);
						os_strcat(argvs, " ");
					}
				}
				process_log_debug("restart process :%s execp: %s %s", process->name, process->process,
						os_strlen(argvs) ? argvs:" ");
				process->pid = pid;
				return pid;
			}
			else
				process_log_err("restart process :%s execp:%s", process->name, process->process);
		}
	}
	return 0;
}

static int process_waitpid(process_t *process)
{
	int status = 0;
	int pid = process->pid;
	process->pid = waitpid(pid, &status, WNOHANG);
	if (WIFSIGNALED(status))
	{
		if ((WTERMSIG(status) != SIGKILL && WTERMSIG(status) != SIGTERM))
		{
			process_log_warn("exit process(signal=%d) :%s execp:%s", WTERMSIG(status),
					process->name, process->process);
			process->pid = 0;
			if(process->restart && process->active)
				process_start(process);
		}
		else
		{
			process_log_warn("exit process(signal=%d) :%s execp:%s", WTERMSIG(status),
					process->name, process->process);
		}
	}
	return 0;
}

int process_waitpid_api()
{
	NODE index;
	process_t *pstNode = NULL;
	if(lstCount(gProcessMain.list) ==  0)
		return OK;
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	for(pstNode = (process_t *)lstFirst(gProcessMain.list);
			pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		process_waitpid(pstNode);
	}
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return OK;
}


int process_callback_api(process_cb cb, void *pVoid)
{
	NODE index;
	process_t *pstNode = NULL;
	if(lstCount(gProcessMain.list) ==  0)
		return OK;
	if(gProcessMain.mutex)
		os_mutex_lock(gProcessMain.mutex, OS_WAIT_FOREVER);
	for(pstNode = (process_t *)lstFirst(gProcessMain.list);
			pstNode != NULL;  pstNode = (process_t *)lstNext((NODE*)&index))
	{
		index = pstNode->node;
		if(cb)
			(cb)(pstNode, pVoid);
	}
	if(gProcessMain.mutex)
		os_mutex_unlock(gProcessMain.mutex);
	return OK;
}

static int process_argvs_add(process_t *process, char *argvs)
{
	int i =0;
	for(i = 0; i < P_ARGV_MAX; i++)
	{
		if(!process->argv[i])
		{
			process->argv[i] = XSTRDUP(MTYPE_LINK_NODE, argvs);
			return OK;
		}
	}
	return ERROR;
}

static int process_argvs_split(process_t *process, char *argvs)
{
#if 1
	int i = 0, j = 0;
	char *brk = argvs;
	char tmp[128];
	os_memset(tmp, 0, sizeof(tmp));
	while(i < P_PATH_MAX)
	{
		if( (brk[i] >= 0x20) && (brk[i] <= 0x7e) )
		{
			if(!isspace(brk[i]))
				tmp[j++] = brk[i];
			else
			{
				if(j)
				{
					tmp[j] = '\0';
					if(process_argvs_add(process, tmp) != OK)
						return ERROR;
					os_memset(tmp, 0, sizeof(tmp));
					j = 0;
				}
			}
		}
		i++;
	}
	if(j)
	{
		tmp[j] = '\0';
		if(process_argvs_add(process, tmp) != OK)
			return ERROR;
	}
#else
	int offset = 0;
	char *brk = argvs;
	char tmp[128];
	while(brk && brk != '\0')
	{
		os_memset(tmp, 0, sizeof(tmp));
		os_sscanf(brk, "%[^ ]", tmp);
		if(os_strlen(tmp))
		{
			tmp[os_strlen(tmp)-1] = '\0';
			if(process_argvs_add(process, tmp) != OK)
				return ERROR;
			brk += os_strlen(tmp) + 1;
		}
		else
			break;
	}
#endif
	return OK;
}

static int process_free(process_t *process)
{
	int i = 0;
	for(i = 0; i < P_ARGV_MAX; i++)
	{
		if(process->argv[i])
			XFREE(MTYPE_LINK_NODE, process->argv[i]);
	}
	XFREE(MTYPE_THREAD, process);
	return OK;
}

process_t * process_get(process_head *head)
{
	process_t *node = XMALLOC(MTYPE_THREAD, sizeof(process_t));
	if(node)
	{
		os_memset(node, 0, sizeof(process_t));
		if(os_strlen(head->name))
			os_strcpy(node->name, head->name);
		if(os_strlen(head->process))
			os_strcpy(node->process, head->process);
		node->restart = head->restart;
		node->active = TRUE;
		node->id	= head->id;
		if(os_strlen(head->argvs))
		{
			if(process_argvs_split(node, head->argvs) != OK)
			{
				XFREE(MTYPE_THREAD, node);
				return NULL;
			}
		}
		//node->id = ++main_process_id;
		return node;
	}
	return NULL;
}



static int process_alloc_id()
{
	++main_process_id;
	return main_process_id;
}

int process_handle(int fd, process_action action, process_head *head)
{
	int pid = 0;
	process_t *lookup = NULL;
	if(!head)
	{
		process_log_err("process handle, head is NULL");
		return ERROR;
	}
	if(action == PROCESS_DEAMON)
	{
		lookup = process_get(head);
		if(lookup)
		{
			pid = process_deamon_start(lookup);
			if(pid)
			{
				//lookup->id = process_alloc_id();
				process_log_debug("start deamon :%s deamon :%s", head->name, head->process);
				os_process_action_respone(fd, 1);
			}
			else
				os_process_action_respone(fd, 1);
		}
		return OK;
	}
	if(os_strlen(head->name))
		lookup = process_lookup_api(head->name);
	else if(head->id)
		lookup = process_lookup_id_api(head->id);
	else
	{
		os_process_action_respone(fd, PROCESS_NONE);
		process_log_err("process handle, no key to lookup");
		return ERROR;
	}
	//if(process)
	{
		switch(action)
		{
		case PROCESS_ECHO:
			if(lookup)
			{
				os_process_action_respone(fd, lookup->id);
				process_log_debug("echo process :%s execp :%s", lookup->name, lookup->process);
			}
			else
			{
				os_process_action_respone(fd, PROCESS_NONE);
				if(os_strlen(head->name))
					process_log_err("process :%s execp :%s is not exist.", head->name, head->process);
				else
					process_log_err("process id :%d is not exist.", head->id);
			}
			break;
		case PROCESS_START:
			if(lookup)
			{
				if(lookup->pid == PROCESS_NONE )
				{
					process_start(lookup);
					process_log_debug("start process :%s execp :%s", lookup->name, lookup->process);
				}
				else
				{
					process_log_warn("process :%s execp :%s is already start", lookup->name, lookup->process);
				}
				if(waitpid(lookup->pid, NULL, WNOHANG) > 0)
					os_process_action_respone(fd, PROCESS_NONE);
				else
					os_process_action_respone(fd, lookup->id);
			}
			else
			{
				if(!os_strlen(head->name))
				{
					os_process_action_respone(fd, PROCESS_NONE);
					process_log_err("register process param is valid");
					break;
				}
				process_t *process = process_get(head);
				if(process)
				{
					pid = process_start(process);
					if(pid)
					{
						if(waitpid(pid, NULL, WNOHANG) > 0)
						{
							os_process_action_respone(fd, PROCESS_NONE);
							process_free(process);
						}
						else
						{
							process->id = process_alloc_id();
							if(process_add_api(process) == OK)
							{
								//process_log_debug("start process :%s execp :%s", head->name, head->process);
								os_process_action_respone(fd, process->id);
							}
							else
							{
								process_log_err("error start process :%s execp :%s", head->name, head->process);
								process_stop(process);
								os_process_action_respone(fd, PROCESS_NONE);
								process_free(process);
							}
						}
					}
					else
					{
						os_process_action_respone(fd, PROCESS_NONE);
						process_free(process);
						process_log_err("register process :%s execp :%s", head->name, head->process);
					}
				}
			}
			break;
		case PROCESS_STOP:
			if(lookup)
			{
				if(lookup->pid)
				{
					process_stop(lookup);
					process_log_debug("stop process :%s execp :%s", lookup->name, lookup->process);
					os_process_action_respone(fd, lookup->id);
					process_del_api(lookup);
				}
				else
				{
					process_log_warn("process :%s execp :%s is already stop", lookup->name, lookup->process);
					os_process_action_respone(fd, lookup->pid);
					process_del_api(lookup);
				}
			}
			else
			{
				os_process_action_respone(fd, PROCESS_NONE);
				if(os_strlen(head->name))
					process_log_err("process :%s execp :%s is not exist.", head->name, head->process);
				else
					process_log_err("process id :%d is not exist.", head->id);
			}
			break;
		case PROCESS_RESTART:
			if(lookup)
			{
				if(lookup->pid)
				{
					pid = process_restart(lookup);
				}
				else
				{
					pid = process_start(lookup);
				}
				if(pid)
				{
					if(waitpid(lookup->pid, NULL, WNOHANG) > 0)
						os_process_action_respone(fd, PROCESS_NONE);
					else
						os_process_action_respone(fd, lookup->id);
					process_log_debug("restart process :%s execp :%s", lookup->name, lookup->process);
				}
				else
				{
					os_process_action_respone(fd, PROCESS_NONE);
					process_log_err("error restart process :%s execp :%s", lookup->name, lookup->process);
				}
			}
			else
			{
				os_process_action_respone(fd, PROCESS_NONE);
				if(os_strlen(head->name))
					process_log_err("process :%s execp :%s is not exist.", head->name, head->process);
				else
					process_log_err("process id :%d is not exist.", head->id);
			}
			break;
		case PROCESS_LOOKUP:
			if(lookup)
			{
				os_process_action_respone(fd, lookup->id);
				process_log_debug("echo process :%s execp :%s", lookup->name, lookup->process);
			}
			else
			{
				os_process_action_respone(fd, PROCESS_NONE);
				if(os_strlen(head->name))
					process_log_err("process :%s execp :%s is not exist.", head->name, head->process);
				else
					process_log_err("process id :%d is not exist.", head->id);
			}
			break;
		case PROCESS_NONE:
			os_process_action_respone(fd, PROCESS_NONE);
			break;
		default:
			os_process_action_respone(fd, PROCESS_NONE);
			break;
		}
	}
	//else
	{
	//	process_waitpid_api();
	}
	return OK;
}




#endif

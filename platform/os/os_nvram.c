/*
 * os_nvram.c
 *
 *  Created on: 2019年2月28日
 *      Author: DELL
 */

#include "zebra.h"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>

#include "os_nvram.h"


static LIST *env_table = NULL;
static os_mutex_t *env_mutex = NULL;

#ifdef OS_NVRAM_ON_FILE
#define MTD_NVRAM_INFILE			"/etc/.nvram"
#endif
static int os_nvram_env_update_save(void);
static int os_nvram_env_loadall(void);


int os_nvram_env_init()
{
	if (env_mutex == NULL)
	{
		env_mutex = os_mutex_init();
	}
	if (env_table == NULL)
	{
		env_table = os_malloc(sizeof(LIST));
		if (env_table)
		{
			lstInit(env_table);
			os_nvram_env_loadall();
			return OK;
		}
		os_mutex_exit(env_mutex);
		return ERROR;
	}
	return OK;
}


int os_nvram_env_exit()
{
	if (env_mutex)
	{
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
		os_mutex_exit(env_mutex);
		if (env_table)
			lstFree(env_table);
		return OK;
	}
	return OK;
}


static os_nvram_env_t * os_nvram_env_node_lookup_by_name(char *nvram_env_name)
{
	char name[OS_NVRAM_MAX];
	NODE node;
	os_nvram_env_t *nvram_env = NULL;
	if (env_table == NULL)
		return NULL;
	memset(name, 0, sizeof(name));
	strncpy(name, nvram_env_name, MIN(sizeof(name), strlen(nvram_env_name)));
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);

	for (nvram_env = (os_nvram_env_t *) lstFirst(env_table);
			nvram_env != NULL; nvram_env = (os_nvram_env_t *) lstNext(&node))
	{
		node = nvram_env->node;
		if (nvram_env)
		{
			if (memcmp(nvram_env->name, name, sizeof(name)) == 0)
			{
				if (env_mutex)
					os_mutex_unlock(env_mutex);
				return nvram_env;
			}
		}
	}
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return NULL;
}

int os_nvram_env_add(char *name, char *value)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(node == NULL)
		node = os_malloc(sizeof(os_nvram_env_t));
	if(!node)
		return ERROR;
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	memset(node, 0, sizeof(os_nvram_env_t));
	strncpy(node->name, name, MIN(sizeof(node->name), strlen(name)));
	strncpy(node->ptr.va_p, value, MIN(sizeof(node->ptr.va_p), strlen(value)));
	node->len = strlen(value);
	node->type = OS_NVRAM_STR;
	lstAdd(env_table, (NODE *) node);
	os_nvram_env_update_save();
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}

int os_nvram_env_set(char *name, char *value)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(!node)
		return ERROR;

	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	memset(&node->ptr, 0, sizeof(node->ptr));
	strncpy(node->ptr.va_p, value, MIN(sizeof(node->ptr.va_p), strlen(value)));
	node->len = strlen(value);
	node->type = OS_NVRAM_STR;
	os_nvram_env_update_save();
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}

int os_nvram_env_add_integer(char *name, int len, int value)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(node == NULL)
		node = os_malloc(sizeof(os_nvram_env_t));
	if(!node)
		return ERROR;
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	memset(node, 0, sizeof(os_nvram_env_t));
	strncpy(node->name, name, MIN(sizeof(node->name), strlen(name)));
	switch(len)
	{
	case 1:
		node->ptr.va_8 = (u_int8 )value;
		break;
	case 2:
		node->ptr.va_16 = (u_int16 )value;
		break;
	case 4:
		node->ptr.va_32 = (u_int32 )value;
		break;
	default:
		break;
	}
	node->len = len;
	node->type = OS_NVRAM_VAL;
	//strcpy(node->ptr.va_p, value);
	lstAdd(env_table, (NODE *) node);
	os_nvram_env_update_save();
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}



int os_nvram_env_del(char *name)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(!node)
		return ERROR;
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	lstDelete(env_table, (NODE *) node);
	os_nvram_env_update_save();
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}

int os_nvram_env_get(char *name, char *value, int len)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(!node)
		return ERROR;
	if(node->type != OS_NVRAM_STR)
		return ERROR;
/*	if(node->len <= 4)
		return ERROR;*/
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);

	if(value)
	{
		memcpy(value, node->ptr.va_p, MIN(len, node->len));
	}
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}

int os_nvram_env_get_integer(char *name, int len)
{
	int value = 0;
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(!node)
		return ERROR;
	if(node->type != OS_NVRAM_VAL)
		return ERROR;
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	switch(len)
	{
	case 1:
		value = node->ptr.va_8 & 0x000000ff;
		break;
	case 2:
		value = node->ptr.va_16 & 0x0000ffff;
		break;
	case 4:
		value = node->ptr.va_32 & 0xffffffff;
		break;
	default:
		break;
	}
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return value;
}


const char * os_nvram_env_lookup(const char *name)
{
	os_nvram_env_t *node = os_nvram_env_node_lookup_by_name(name);
	if(!node)
		return NULL;
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);
	if(node->type == OS_NVRAM_VAL)
	{
		switch(node->len)
		{
		case 1:
			memset(node->ptr.va_p, 0, sizeof(node->ptr.va_p));
			snprintf(node->ptr.va_p, sizeof(node->ptr.va_p), "%u", node->ptr.va_8 & 0x000000ff);
			break;
		case 2:
			memset(node->ptr.va_p, 0, sizeof(node->ptr.va_p));
			snprintf(node->ptr.va_p, sizeof(node->ptr.va_p), "%u", node->ptr.va_16 & 0x0000ffff);
			break;
		case 4:
			memset(node->ptr.va_p, 0, sizeof(node->ptr.va_p));
			snprintf(node->ptr.va_p, sizeof(node->ptr.va_p), "%u", node->ptr.va_32 & 0xffffffff);
			break;
		default:
			memset(node->ptr.va_p, 0, sizeof(node->ptr.va_p));
			break;
		}
		if (env_mutex)
			os_mutex_unlock(env_mutex);
		return strlen(node->ptr.va_p) ? node->ptr.va_p:NULL;
	}
	else
	{
		if(node->type == OS_NVRAM_STR)
		{
			if (env_mutex)
				os_mutex_unlock(env_mutex);
			return node->ptr.va_p;
		}
		else
		{
			if (env_mutex)
				os_mutex_unlock(env_mutex);
		}
	}
	return NULL;
}

int os_nvram_env_show(char *name, int (*show_cb)(void *, os_nvram_env_t *), void *p)
{
	NODE node;
	os_nvram_env_t *nvram_env = NULL;

	if(name)
	{
		nvram_env = os_nvram_env_node_lookup_by_name(name);
		if(!nvram_env)
			return ERROR;
		if(show_cb)
			(show_cb)(p, nvram_env);
		return OK;
	}
	if (env_mutex)
		os_mutex_lock(env_mutex, OS_WAIT_FOREVER);

	for (nvram_env = (os_nvram_env_t *) lstFirst(env_table);
			nvram_env != NULL; nvram_env = (os_nvram_env_t *) lstNext(&node))
	{
		node = nvram_env->node;
		if (nvram_env)
		{
			if(show_cb)
				(show_cb)(p, nvram_env);
		}
	}
	if (env_mutex)
		os_mutex_unlock(env_mutex);
	return OK;
}


#ifdef OS_NVRAM_ON_FILE
static int os_nvram_env_read_one(int fd, os_nvram_env_t *node)
{
	os_nvram_env_t *addnode = NULL;
	if(read(fd, node, sizeof(os_nvram_env_t)) == sizeof(os_nvram_env_t))
	{
		addnode = os_malloc(sizeof(os_nvram_env_t));
		if(!addnode)
			return ERROR;
		memset(addnode, 0, sizeof(os_nvram_env_t));
		memcpy(addnode, node, sizeof(os_nvram_env_t));
		lstAdd(env_table, (NODE *) addnode);
		return OK;
	}
	return ERROR;
}


static int os_nvram_env_loadall(void)
{
	int ret = OK, fd = 0;
	os_nvram_env_t nvram_env;
	if (env_table == NULL)
		return ERROR;
	if(ret == 0)
	{
		fd = open(MTD_NVRAM_INFILE, O_RDONLY);
		if(fd <= 0)
		{
			return ERROR;
		}
		while(ret == OK)
		{
			memset(&nvram_env, 0, sizeof(os_nvram_env_t));
			ret = os_nvram_env_read_one(fd, &nvram_env);
		}
		close(fd);
		return OK;
	}
	return ERROR;
}

static int os_nvram_env_write_list(int fd)
{
	int ret = 0;
	NODE node;
	os_nvram_env_t *nvram_env = NULL;
	for (nvram_env = (os_nvram_env_t *) lstFirst(env_table);
			nvram_env != NULL; nvram_env = (os_nvram_env_t *) lstNext(&node))
	{
		node = nvram_env->node;
		if (nvram_env)
		{
			ret = write(fd, nvram_env, sizeof(os_nvram_env_t));
			if(ret != sizeof(os_nvram_env_t))
				break;
		}
	}
	return ret;
}

static int os_nvram_env_update_save(void)
{
	int ret = 0, fd = 0;
	if (env_table == NULL)
		return ERROR;
	if(lstCount(env_table) == 0)
	{
		if(access(MTD_NVRAM_INFILE, F_OK) == 0)
			remove(MTD_NVRAM_INFILE);
		sync();
		return OK;
	}
	if(ret == 0)
	{
		fd = open(MTD_NVRAM_INFILE".tmp", O_RDWR|O_CREAT, 0644);
		if(fd <= 0)
		{
			return ERROR;
		}
		if(os_nvram_env_write_list(fd) != sizeof(os_nvram_env_t))
		{
			close(fd);
			remove(MTD_NVRAM_INFILE".tmp");
			sync();
			return ERROR;
		}
		close(fd);
		rename(MTD_NVRAM_INFILE".tmp", MTD_NVRAM_INFILE);
		sync();
		return OK;
	}
	return ERROR;
}
#else

//static os_nvram_env_t os_nvram_env;


static int mtd_nvram_lookup()
{
	system("dd if=/dev/mtdblock2 of=/tmp/factory.bin");
	if(access("/tmp/factory.bin", 0) != 0)
		return -1;
	return 0;
}

static int mtd_nvram_restore(int hwreset)
{
	system("mtd write /tmp/factory.bin factory");
	remove(MTD_NVRAM_FILE);
	return 0;
}

static int mtd_nvram_offset(int fd, int offset)
{
	return lseek(fd, SEEK_SET, offset);
}

static int mtd_nvram_read(int fd, unsigned char *buf, int len)
{
	return read(fd, buf, len);
}

static int mtd_nvram_write(int fd, unsigned char *buf, int len)
{
	return write(fd, buf, len);
}

static int mtd_nvram_open(unsigned char *filename)
{
	return open(filename, O_RDWR);
}
#endif

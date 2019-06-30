/*
 * os_nvram.h
 *
 *  Created on: 2019年2月28日
 *      Author: DELL
 */

#ifndef __OS_NVRAM_H__
#define __OS_NVRAM_H__

#include "os_list.h"



//#define OS_NVRAM_ON_FLASH
#define OS_NVRAM_ON_FILE


#define OS_NVRAM_MAX	32

#pragma pack(1)
typedef struct os_nvram_env_s
{
	NODE		node;
	char		name[OS_NVRAM_MAX];
	u_int8		len;
	enum
	{
		OS_NVRAM_VAL,
		OS_NVRAM_STR,
		OS_NVRAM_FLOAT,
	}type;
	union
	{
		char 		va_p[OS_NVRAM_MAX];
		u_int8 		va_8;
		u_int16 	va_16;
		u_int32 	va_32;
		s_float		va_float;
	}ptr;
}os_nvram_env_t;
#pragma pack()


extern int os_nvram_env_init();
extern int os_nvram_env_exit();
extern int os_nvram_env_add(char *name, char *value);
extern int os_nvram_env_add_integer(char *name, int len, int value);
extern int os_nvram_env_set(char *name, char *value);

extern int os_nvram_env_del(char *name);
extern int os_nvram_env_get(char *name, char *value, int len);
extern int os_nvram_env_get_integer(char *name, int len);

extern const char * os_nvram_env_lookup(const char *name);

extern int os_nvram_env_show(char *name, int (*show_cb)(void *, os_nvram_env_t *), void *p);

extern int cmd_nvram_env_init();

#endif /* __OS_NVRAM_H__ */

/*
 * os_nvram.h
 *
 *  Created on: 2019年2月28日
 *      Author: DELL
 */

#ifndef __OS_NVRAM_H__
#define __OS_NVRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "os_list.h"



//#define OS_NVRAM_ON_FLASH
#define OS_NVRAM_ON_FILE


#define OS_NVRAM_MAX	32

#pragma pack(1)
typedef struct os_nvram_env_s
{
	NODE		node;
	ospl_char		name[OS_NVRAM_MAX];
	ospl_uint8		len;
	enum
	{
		OS_NVRAM_VAL,
		OS_NVRAM_STR,
		OS_NVRAM_FLOAT,
	}type;
	union
	{
		ospl_char 		va_p[OS_NVRAM_MAX];
		ospl_uint8 		va_8;
		ospl_uint16 	va_16;
		ospl_uint32 	va_32;
		ospl_float		va_float;
	}ptr;
}os_nvram_env_t;
#pragma pack()


extern int os_nvram_env_init();
extern int os_nvram_env_exit();
extern int os_nvram_env_add(ospl_char *name, ospl_char *value);
extern int os_nvram_env_add_integer(ospl_char *name, ospl_uint32 len, ospl_uint32 value);
extern int os_nvram_env_set(ospl_char *name, ospl_char *value);

extern int os_nvram_env_del(ospl_char *name);
extern int os_nvram_env_get(ospl_char *name, ospl_char *value, ospl_uint32 len);
extern int os_nvram_env_get_integer(ospl_char *name, ospl_uint32 len);

extern const ospl_char * os_nvram_env_lookup(const ospl_char *name);

extern int os_nvram_env_show(ospl_char *name, int (*show_cb)(void *, os_nvram_env_t *), void *p);

extern int cmd_nvram_env_init();

#ifdef __cplusplus
}
#endif

#endif /* __OS_NVRAM_H__ */

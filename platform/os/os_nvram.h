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
	zpl_char		name[OS_NVRAM_MAX];
	zpl_uint8		len;
	enum
	{
		OS_NVRAM_VAL,
		OS_NVRAM_STR,
		OS_NVRAM_FLOAT,
	}type;
	union
	{
		zpl_char 		va_p[OS_NVRAM_MAX];
		zpl_uint8 		va_8;
		zpl_uint16 	va_16;
		zpl_uint32 	va_32;
		zpl_float		va_float;
	}ptr;
}os_nvram_env_t;
#pragma pack()


extern int os_nvram_env_init();
extern int os_nvram_env_exit();
extern int os_nvram_env_add(zpl_char *name, zpl_char *value);
extern int os_nvram_env_add_integer(zpl_char *name, zpl_uint32 len, zpl_uint32 value);
extern int os_nvram_env_set(zpl_char *name, zpl_char *value);

extern int os_nvram_env_del(zpl_char *name);
extern int os_nvram_env_get(zpl_char *name, zpl_char *value, zpl_uint32 len);
extern int os_nvram_env_get_integer(zpl_char *name, zpl_uint32 len);

extern const zpl_char * os_nvram_env_lookup(const zpl_char *name);

extern int os_nvram_env_show(zpl_char *name, int (*show_cb)(void *, os_nvram_env_t *), void *p);

extern int cmd_nvram_env_init();

#ifdef __cplusplus
}
#endif

#endif /* __OS_NVRAM_H__ */

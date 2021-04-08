/*
 * os_uci.h
 *
 *  Created on: Jan 29, 2019
 *      Author: zhurish
 */

#ifndef __OS_UCI_H__
#define __OS_UCI_H__

//#define PL_OPENWRT_UCI

#ifdef PL_OPENWRT_UCI

#define OS_UCI_DEBUG
//#define _UCI_DEBUG(fmt,...)	printf(fmt, ##__VA_ARGS__)
#define _UCI_DEBUG(fmt,...)

//#if 1//def PL_BUILD_OS_OPENWRT
//#define PL_OPENWRT_UCI_LIB
//#endif
//#define PL_OPENWRT_UCI_SH

#ifndef PL_OPENWRT_UCI_LIB
enum
{
	UCI_OK = 0,
	UCI_ERR_MEM,
	UCI_ERR_INVAL,
	UCI_ERR_NOTFOUND,
	UCI_ERR_IO,
	UCI_ERR_PARSE,
	UCI_ERR_DUPLICATE,
	UCI_ERR_UNKNOWN,
	UCI_ERR_LAST
};
#endif

extern int os_uci_get_errno();
extern int os_uci_set_string(char *name, char *value);
extern int os_uci_set_integer(char *name, int value);
extern int os_uci_set_float(char *name, char *fmt, float value);
extern int os_uci_set_double(char *name, char *fmt, double value);

extern int os_uci_get_string(char *name, char *value);
extern int os_uci_get_integer(char *name, int *value);
extern int os_uci_get_float(char *name, char *fmt, float *value);
extern int os_uci_get_double(char *name, char *fmt, double *value);
extern int os_uci_get_address(char *name, char *value);
extern int os_uci_get_list(char *name, char **value, int *cnt);

extern int os_uci_list_add(char *name, char * value);
extern int os_uci_list_del(char *name, char * value);

extern int os_uci_del(char *name, char * section, char * option, char *value);

extern int os_uci_section_add(char *name, char * section);
extern int os_uci_commit(char *name);
extern int os_uci_save_config(char *name);

extern int os_uci_init();

#endif /* PL_OPENWRT_UCI */

#endif /* __OS_UCI_H__ */

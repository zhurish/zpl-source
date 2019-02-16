/*
 * os_uci.h
 *
 *  Created on: Jan 29, 2019
 *      Author: zhurish
 */

#ifndef __OS_UCI_H__
#define __OS_UCI_H__


typedef struct os_uci_s
{
	void	* context;
}os_uci_t;


extern int os_uci_set_string(char *name, char *value);
extern int os_uci_set_integer(char *name, int value);

extern int os_uci_section_add(char *name, char * section);
extern int os_uci_del(char *name, char * section, char * option);
extern int os_uci_list_add(char *name, char * value);
extern int os_uci_list_del(char *name, char * value);
extern int os_uci_commit(char *name);
extern int os_uci_get_string(char *name, char *value);
extern int os_uci_get_list(char *name, char **value, int *cnt);


#endif /* __OS_UCI_H__ */

/*
 * os_process.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_PROCESS_H__
#define __OS_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"


#define ZPL_TOOLS_PROCESS

#ifdef ZPL_TOOLS_PROCESS

#define PROCESS_MGT_UNIT_NAME	"ProcessMU"
#define P_NAME_MAX	64
#define P_PATH_MAX	256
#define P_ARGV_MAX	32

typedef enum p_action
{
	PROCESS_NONE = 0,
	PROCESS_ECHO,
	PROCESS_DEAMON,
	PROCESS_START,
	PROCESS_STOP,
	PROCESS_RESTART,
	PROCESS_LOOKUP,
	PROCESS_MAX

}process_action;

/*
 * 0:  handle error, not exit
 * >0: process id
 */
#pragma pack(1)
typedef struct process_head_s
{
	zpl_char 			name[P_NAME_MAX];
	zpl_char 			process[P_PATH_MAX];
	zpl_char 			argvs[P_PATH_MAX];
	zpl_uint8	action;
	zpl_uint8   restart;
	zpl_uint32				id;
}process_head;
#pragma pack(0)

extern int os_process_register(process_action action, zpl_char *name, zpl_char *process, zpl_bool restart, zpl_char *argv[]);
extern int os_process_action(process_action action, zpl_char *name, zpl_uint32 id);
extern int os_process_action_respone(int fd, zpl_uint32 respone);

extern int os_process_start(void);
extern int os_process_stop(void);

#endif


extern int super_system(const zpl_char *cmd);
extern int super_output_system(const zpl_char *cmd, zpl_char *output, zpl_uint32 len);
extern int super_input_system(const zpl_char *cmd, zpl_char *input);
extern int super_system_execvp(const zpl_char *cmd, zpl_char **input);



extern zpl_pid_t child_process_create(void);
extern int child_process_destroy(zpl_pid_t pid);
extern int child_process_kill(zpl_pid_t pid);
extern int child_process_wait(zpl_pid_t pid, zpl_uint32 wait);


#ifdef __cplusplus
}
#endif

#endif /* __OS_PROCESS_H__ */

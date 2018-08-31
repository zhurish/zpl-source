/*
 * os_util.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_UTIL_H__
#define __OS_UTIL_H__


#define OS_PIPE_BASE	"/var/run"


//#define DOUBLE_PROCESS

#ifdef DOUBLE_PROCESS

#define PROCESS_MGT_UNIT_NAME	"ProcessMU"
#define P_NAME_MAX	64
#define P_PATH_MAX	256
#define P_ARGV_MAX	32

typedef enum p_action
{
	PROCESS_NONE = 0,
	PROCESS_ECHO,
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
	char 			name[P_NAME_MAX];
	char 			process[P_PATH_MAX];
	char 			argvs[P_PATH_MAX];
	unsigned char	action;
	unsigned char   restart;
	int				id;
}process_head;
#pragma pack(0)

extern int os_process_register(process_action action, char *name, char *process, BOOL restart, char *argv[]);
extern int os_process_action(process_action action, char *name, int id);
extern int os_process_action_respone(int fd, int respone);
#endif



extern int super_system(const char *cmd);
extern int super_output_system(const char *cmd, char *output, int len);
extern int super_input_system(const char *cmd, char *input);


extern char * pid2name(int pid);
extern int name2pid(const char *name);

extern int child_process_create();
extern int child_process_destroy(int pid);

extern int super_system_execvp(const char *cmd, char **input);


extern int os_write_string(const char *name, const char *string);
extern int os_read_string(const char *name, const char *string, int len);

extern int os_set_nonblocking(int fd);
extern int os_set_blocking(int fd);
extern int os_pipe_create(char *name, int mode);
extern int os_pipe_close(int fd);
extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, int timeout);


extern int os_stream_write(int fd, char *inbuf, int len);
extern int os_stream_read(int fd, char *inbuf, int len);

extern int os_stream_head_write(int fd, char *inbuf, int len);
extern int os_stream_head_read(int fd, char *inbuf, int len);

extern int os_register_signal(int sig, void (*handler)(int));


#endif /* __OS_UTIL_H__ */

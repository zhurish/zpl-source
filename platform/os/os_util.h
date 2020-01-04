/*
 * os_util.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_UTIL_H__
#define __OS_UTIL_H__


#define OS_PIPE_BASE	SYSRUNDIR


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

extern int os_process_start();
extern int os_process_stop();

#endif


extern void os_log(char *file, const char *format, ...);


extern int super_system(const char *cmd);
extern int super_output_system(const char *cmd, char *output, int len);
extern int super_input_system(const char *cmd, char *input);


extern char * pid2name(int pid);
extern int name2pid(const char *name);

extern pid_t os_pid_set (const char *path);
extern pid_t os_pid_get (const char *path);

extern int child_process_create();
extern int child_process_destroy(int pid);
extern int child_process_kill(int pid);
extern int child_process_wait(int pid, int wait);

extern int super_system_execvp(const char *cmd, char **input);


extern int os_get_blocking(int fd);
extern int os_set_nonblocking(int fd);
extern int os_set_blocking(int fd);
extern int os_pipe_create(char *name, int mode);
extern int os_pipe_close(int fd);

extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, int timeout_ms);


extern int os_write_file(const char *name, const char *string, int len);
extern int os_read_file(const char *name, const char *string, int len);

extern int os_write_timeout(int fd, char *buf, int len, int timeout_ms);
extern int os_read_timeout(int fd, char *buf, int len, int timeout_ms);


extern int os_register_signal(int sig, void (*handler)(int
#ifdef SA_SIGINFO
	     , siginfo_t *siginfo, void *context
#endif
		));


/*
 * FILE SIZE
 */
#define MPLS_1_M(n)	(n << 20)
#define MPLS_1_K(n)	(n << 10)

#define MPLS_X_B(n)	(n)
#define MPLS_X_K(n)	(n >> 10)
#define MPLS_X_M(n)	(n >> 20)
#define MPLS_X_G(n)	(n >> 30)

extern int os_file_size (const char *filename);
extern const char * os_file_size_string(u_int len);

//extern const char * os_stream_size(long long len);

/*
 * URL
 */

typedef struct os_url_s
{
	char 		*proto;
	char 		*host;
	u_int16		port;
	char 		*user;
	char 		*pass;
	char 		*path;
	char 		*filename;
}os_url_t;

extern int os_url_split(const char * URL, os_url_t *spliurl);
//extern int os_url_show(os_url_t *spliurl);
extern int os_url_free(os_url_t *spliurl);
//extern int os_url_test();
/*
 * thread
 */
extern int os_thread_once(int (*entry)(void *), void *p);



extern int fdprintf ( int fd, const char *format, ...);

extern int hostname_ipv4_address(char *hostname, struct in_addr *addr);
extern int hostname_ipv6_address(char *hostname, struct in6_addr *addr);


#endif /* __OS_UTIL_H__ */

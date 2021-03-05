/*
 * os_util.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_UTIL_H__
#define __OS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

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
	ospl_char 			name[P_NAME_MAX];
	ospl_char 			process[P_PATH_MAX];
	ospl_char 			argvs[P_PATH_MAX];
	ospl_uint8	action;
	ospl_uint8   restart;
	ospl_uint32				id;
}process_head;
#pragma pack(0)

extern int os_process_register(process_action action, ospl_char *name, ospl_char *process, ospl_bool restart, ospl_char *argv[]);
extern int os_process_action(process_action action, ospl_char *name, ospl_uint32 id);
extern int os_process_action_respone(int fd, ospl_uint32 respone);

extern int os_process_start();
extern int os_process_stop();

#endif


extern void os_log(ospl_char *file, const ospl_char *format, ...);


extern int super_system(const ospl_char *cmd);
extern int super_output_system(const ospl_char *cmd, ospl_char *output, ospl_uint32 len);
extern int super_input_system(const ospl_char *cmd, ospl_char *input);


extern ospl_char * pid2name(ospl_pid_t pid);
extern ospl_pid_t name2pid(const ospl_char *name);

extern ospl_pid_t os_pid_set (const ospl_char *path);
extern ospl_pid_t os_pid_get (const ospl_char *path);

extern ospl_pid_t child_process_create();
extern int child_process_destroy(ospl_pid_t pid);
extern int child_process_kill(ospl_pid_t pid);
extern int child_process_wait(ospl_pid_t pid, ospl_uint32 wait);

extern int super_system_execvp(const ospl_char *cmd, ospl_char **input);

extern int os_mkdir(const ospl_char *dirpath, ospl_uint32 mode, ospl_uint32 pathflag);
extern int os_rmdir(const ospl_char *dirpath, ospl_uint32 pathflag);
extern int os_getpwddir(const ospl_char *path, ospl_uint32 pathsize);


extern int os_get_blocking(int fd);
extern int os_set_nonblocking(int fd);
extern int os_set_blocking(int fd);
extern int os_pipe_create(ospl_char *name, ospl_uint32 mode);
extern int os_pipe_close(int fd);

extern int os_file_access(ospl_char *filename);

extern int os_select_wait(int maxfd, fd_set *rfdset, fd_set *wfdset, ospl_uint32 timeout_ms);


extern int os_write_file(const ospl_char *name, const ospl_char *string, ospl_uint32 len);
extern int os_read_file(const ospl_char *name, const ospl_char *string, ospl_uint32 len);

extern int os_write_timeout(int fd, ospl_char *buf, ospl_uint32 len, ospl_uint32 timeout_ms);
extern int os_read_timeout(int fd, ospl_char *buf, ospl_uint32 len, ospl_uint32 timeout_ms);


extern int os_register_signal(ospl_int sig, void (*handler)(ospl_int
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

extern int os_file_size (const ospl_char *filename);
extern const ospl_char * os_file_size_string(ospl_uint32 len);

//extern const ospl_char * os_stream_size(long long len);

/*
 * URL
 */

typedef struct os_url_s
{
	ospl_char 		*proto;
	ospl_char 		*host;
	ospl_uint16		port;
	ospl_char 		*user;
	ospl_char 		*pass;
	ospl_char 		*path;
	ospl_char 		*filename;
}os_url_t;

extern int os_url_split(const ospl_char * URL, os_url_t *spliurl);
//extern int os_url_show(os_url_t *spliurl);
extern int os_url_free(os_url_t *spliurl);
//extern int os_url_test();
/*
 * thread
 */
extern ospl_pthread_t os_thread_once(int (*entry)(void *), void *p);



extern int fdprintf ( int fd, const ospl_char *format, ...);

extern int hostname_ipv4_address(ospl_char *hostname, struct in_addr *addr);
extern int hostname_ipv6_address(ospl_char *hostname, struct in6_addr *addr);

#ifdef __cplusplus
}
#endif

#endif /* __OS_UTIL_H__ */

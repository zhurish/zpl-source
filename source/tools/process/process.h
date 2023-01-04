/*
 * process.h
 *
 *  Created on: Aug 14, 2018
 *      Author: zhurish
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include <zplos_include.h>


//#undef ZPL_TOOLS_PROCESS
#ifdef ZPL_TOOLS_PROCESS

#define _PROCESS_DEBUG

#undef assert
#define assert(n)

#undef XFREE
#undef XMALLOC
#undef XSTRDUP

#define XFREE(n,m)		os_free((m))
#define XMALLOC(n,m)	os_malloc((m))
#define XSTRDUP(n,m)	os_strdup((m))

#define PROCESS_LOG_BASE	DAEMON_LOG_FILE_DIR
//#define PROCESS_LOG_BASE	"/var/log"


typedef struct process_s
{
	NODE	node;
	char 	name[P_NAME_MAX];
	zpl_bool	active;
	zpl_bool	restart;
	char 	process[P_PATH_MAX];
	char 	*argv[P_ARGV_MAX];
	int		pid;
	int		id;
}process_t;


typedef struct main_process_s
{
	LIST	*list;
	void	*mutex;

}main_process_t;



typedef int (*process_cb)(process_t *, void *);





extern int process_init(void);
extern int process_exit(void);


extern int process_add_api(process_t *process);
extern int process_del_api(process_t *process);


extern process_t * process_lookup_api(char *name);
extern process_t * process_lookup_pid_api(int pid);
extern process_t * process_lookup_id_api(int id);

extern int process_callback_api(process_cb cb, void *pVoid);



extern int process_restart(process_t *process);
extern int process_stop(process_t *process);
extern int process_start(process_t *process);
extern int process_deamon_start(process_t *process);
extern int process_waitpid_api(void);

extern process_t * process_get(process_head *head);

extern int process_handle(int fd, process_action action, process_head *head);

/*
 * log
 */
extern int open_log(char *file);

//#ifdef _PROCESS_DEBUG
extern void process_log_print(int priority, const char *func, int line, const char *format,...);



#define process_log_warn(fmt,...)	process_log_print(LOG_WARNING, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define process_log_debug(fmt,...)		process_log_print(LOG_DEBUG, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define process_log_err(fmt,...)	process_log_print(LOG_ERR, __func__, __LINE__,fmt, ##__VA_ARGS__)

#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __PROCESS_H__ */
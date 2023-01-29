/*
 * os_sem_errchk.c
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "os_sem_errchk.h"



#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
#define OS_LOCK_ERR_FILE	SYSLOGDIR"/sem-errchk.log"
struct task_graph *tg = NULL;

static int os_lock_search_for_cycle(int idx) ;
static void os_lock_print_deadlock(void);

static struct os_lock_vertex *os_lock_create_vertex(struct os_lock_source_type type) 
{
	struct os_lock_vertex *tex = (struct os_lock_vertex *)malloc(sizeof(struct os_lock_vertex ));
	tex->s = type;
	tex->next = NULL;
	return tex;
}

static int os_lock_search_vertex(struct os_lock_source_type type) 
{
	int i = 0;
	for (i = 0;i < tg->num;i ++) 
	{
		if (tg->list[i].s.type == type.type && tg->list[i].s.tid == type.tid) 
		{
			return i;
		}
	}
	return -1;
}
static void os_lock_add_vertex(struct os_lock_source_type type) 
{
	if (os_lock_search_vertex(type) == -1) 
	{
		tg->list[tg->num].s = type;
		tg->list[tg->num].next = NULL;
		tg->num ++;
	}
}

static int os_lock_add_edge(struct os_lock_source_type from, struct os_lock_source_type to) 
{
	int idx = 0;
	struct os_lock_vertex *v = NULL;
	os_lock_add_vertex(from);
	os_lock_add_vertex(to);
	idx = os_lock_search_vertex(from);
	if(idx >= 0)
	{
		v = &(tg->list[idx]);
		while (v->next != NULL) {
			v = v->next;
		}
		v->next = os_lock_create_vertex(to);
		return 0;
	}
	return -1;
}
static int os_lock_verify_edge(struct os_lock_source_type i, struct os_lock_source_type j) 
{
	int idx = 0;
	struct os_lock_vertex *v = NULL;
	if (tg->num == 0) 
		return 0;
	idx = os_lock_search_vertex(i);
	if (idx == -1) 
	{
		return -1;
	}
	v = &(tg->list[idx]);
	while (v != NULL) 
	{
		if (v->s.tid == j.tid) 
			return 1;
		v = v->next;
	}
	return 0;
}
static int os_lock_remove_edge(struct os_lock_source_type from, struct os_lock_source_type to) 
{
	int idxi = os_lock_search_vertex(from);
	int idxj = os_lock_search_vertex(to);
	if (idxi != -1 && idxj != -1) 
	{
		struct os_lock_vertex *v = &tg->list[idxi];
		struct os_lock_vertex *remove;
		while (v->next != NULL)
		{
			if (v->next->s.tid == to.tid) 
			{
				remove = v->next;
				v->next = v->next->next;
				free(remove);
				break;
			}
			v = v->next;
		}
		return 0;
	}
	return 0;
}


static int os_lock_get_lock(void * lock) {
	int i = 0;
	for (i = 0;i < tg->lockidx;i ++) {
		if (tg->locklist[i].lock == lock) {
			return i;
		}
	}
	return -1;
}

static int os_lock_get_empty_lock(void * lock) {
	int i = 0;
	for (i = 0;i < tg->lockidx;i ++) {
		if (tg->locklist[i].lock == 0) {
			return i;
		}
	}
	return tg->lockidx;
}



static int os_lock_before(zpl_pid_t thread_id, void * lockaddr) {
	int idx = 0;
	for(idx = 0;idx < tg->lockidx;idx ++) {
		if ((tg->locklist[idx].lock == lockaddr)) {

			struct os_lock_source_type from;
			from.tid = thread_id;
			from.type = PROCESS;
			os_lock_add_vertex(from);

			struct os_lock_source_type to;
			to.tid = tg->locklist[idx].tid;
			tg->locklist[idx].degress++;
			to.type = PROCESS;
			os_lock_add_vertex(to);
			
			if (!os_lock_verify_edge(from, to)) 
			{
				os_lock_add_edge(from, to); //
				if(os_lock_search_for_cycle(idx))
				{//判断是否有环
					os_lock_print_deadlock();
					os_lock_remove_edge(from, to);//如果有环则把这条边删去，就可以预防死锁 
				}
			}
		}
	}
	return 0;
}

static int os_lock_after(zpl_pid_t thread_id, void * lockaddr) {

	int idx = 0;
	if (-1 == (idx = os_lock_get_lock(lockaddr))) 
	{  // lock list opera 
		int eidx = os_lock_get_empty_lock(lockaddr);
		tg->locklist[eidx].tid = thread_id;
		tg->locklist[eidx].lock = lockaddr;
		tg->lockidx++;
		return 0;
	} else {
		struct os_lock_source_type from;
		from.tid = thread_id;
		from.type = PROCESS;
		struct os_lock_source_type to;
		to.tid = tg->locklist[idx].tid;
		tg->locklist[idx].degress --;
		to.type = PROCESS;
		if (os_lock_verify_edge(from, to))
			os_lock_remove_edge(from, to);
		tg->locklist[idx].tid = thread_id;
		return 0;
	}
	return 0;
}

static int os_unlock_after(zpl_pid_t thread_id, void *lockaddr) {
	int idx = os_lock_get_lock(lockaddr);
	if (tg->locklist[idx].degress == 0) {
		tg->locklist[idx].tid = 0;
		tg->locklist[idx].lock = 0;
		return 0;
	}	
	return 0;
}


static void os_lock_print_deadlock(void) {
	int i = 0;
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "deadlock : ");
	for (i = 0;i < tg->k-1;i ++) {
		os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "%s --> ", os_task_name_LWPID(tg->list[tg->path[i]].s.tid));
	}
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "%s\n", os_task_name_LWPID(tg->list[tg->path[i]].s.tid));
}

static int os_lock_DFS(int idx) 
{
	struct os_lock_vertex *ver = &tg->list[idx];
	if (tg->visited[idx] == 1) {
		tg->path[tg->k++] = idx;
		os_lock_print_deadlock();
		tg->deadlock = 1;
		return 0;
	}
	tg->visited[idx] = 1;
	tg->path[tg->k++] = idx;
	while (ver->next != NULL) {
		os_lock_DFS(os_lock_search_vertex(ver->next->s));
		tg->k --;	
		ver = ver->next;
	}
	return 1;
}
static int os_lock_search_for_cycle(int idx) 
{
	struct os_lock_vertex *ver = &tg->list[idx];
	tg->visited[idx] = 1;
	tg->k = 0;
	tg->path[tg->k++] = idx;
	while (ver->next != NULL) {
		int i = 0;
		for (i = 0;i < tg->num;i ++) {
			if (i == idx) continue;		
			tg->visited[i] = 0;
		}
		for (i = 1;i <= TASK_MUTEX_GRAPH_MAX;i ++) {
			tg->path[i] = -1;
		}
		tg->k = 1;
		os_lock_DFS(os_lock_search_vertex(ver->next->s));
		ver = ver->next;
	}
	return tg->deadlock;
}

static void os_lock_start_check(void) 
{
	if(tg == NULL)
	{
		tg = (struct task_graph*)malloc(sizeof(struct task_graph));
		if(tg == NULL)
			return;
		memset(tg, 0, sizeof(struct task_graph));	
		tg->num = 0;
		tg->lockidx = 0;
	}
}

int task_mutex_graph_lock_before(int selfid, void *p)
{
	os_lock_start_check();
	if(tg)
		return os_lock_before(selfid, p);
	return 0;	
}
int task_mutex_graph_lock_after(int selfid, void *p)
{
	os_lock_start_check();
	if(tg)
		return os_lock_after(selfid, p);
	return 0;
}
int task_mutex_graph_unlock_after(int selfid, void *p)
{
	os_lock_start_check();
	if(tg)
		return os_unlock_after(selfid, p);
	return 0;
}

#endif

#ifdef OS_LOCK_ERR_CHECK
#define OS_LOCK_ERR_FILE	SYSLOGDIR"/sem-errchk.log"

static struct task_mutex_graph  _task_mutex_graph[TASK_MUTEX_GRAPH_MAX];
static zpl_pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t _check_pid = 0;


static int task_mutex_graph_task(void *p)
{
	int intval = 0;
	while(1)
	{
		intval++;
		if(intval == 120)
		{
			task_mutex_graph_show();
			intval = 0;
		}
		sleep(1);
	}
	return 0;
}

int task_mutex_graph_add(int type, void *p)
{
	int i = 0;
	if(_check_pid == 0)
	{
		pthread_create(&_check_pid, NULL, task_mutex_graph_task, NULL);
	}
	pthread_mutex_lock(&_mutex);
	for(i = 0; i < TASK_MUTEX_GRAPH_MAX; i++)
	{
		if(_task_mutex_graph[i].use == 0)
		{
			_task_mutex_graph[i].use = 1;
			_task_mutex_graph[i].type = type;
			_task_mutex_graph[i].mutex = p;
			pthread_mutex_unlock(&_mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&_mutex);
	return 0;
}

int task_mutex_graph_del(int type, void *p)
{
	int i = 0;
	pthread_mutex_lock(&_mutex);
	for(i = 0; i < TASK_MUTEX_GRAPH_MAX; i++)
	{
		if(_task_mutex_graph[i].use == 1 && _task_mutex_graph[i].type == type && _task_mutex_graph[i].mutex == p)
		{
			_task_mutex_graph[i].use = 0;
			_task_mutex_graph[i].type = 0;
			_task_mutex_graph[i].mutex = NULL;
			pthread_mutex_unlock(&_mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&_mutex);
	return 0;
}
#if 1
static int task_mutex_graph_show_info(void *pp)
{
	int i = 0;
	os_task_t *taskinfo = pp;
	os_mutex_t *mutes_info;
	pthread_mutex_lock(&_mutex);
	for(i = 0; i < TASK_MUTEX_GRAPH_MAX; i++)
	{
		if(_task_mutex_graph[i].use == 1)
		{
			mutes_info = _task_mutex_graph[i].mutex;
			if((taskinfo->td_tid == mutes_info->wait_lock || taskinfo->td_tid == mutes_info->self_lock))
			{
				os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "task:%s, mutex:%s, lock:%s, wait lock:%s \r\n", taskinfo->td_name, mutes_info->name?mutes_info->name:"NULL",
				(mutes_info->self_lock==taskinfo->td_tid)?"Y":"N", (mutes_info->wait_lock==taskinfo->td_tid)?"Y":"N");
				fflush(stderr);
			}
		}
	}
	pthread_mutex_unlock(&_mutex);
	return 0;
}
int task_mutex_graph_show(void)
{
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "=====================================\r\n");
	os_task_foreach(task_mutex_graph_show_info, NULL);
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "=====================================\r\n");
	return 0;
}
#else
static int task_mutex_graph_show_info(struct task_mutex_graph *info)
{
	os_mutex_t *mutes_info = info->mutex;
	if(info->type == 1)
	{
		os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "mutex:%-20s, lock task:%s(%d), wait lock task:%s(%d) \r\n", mutes_info->name?mutes_info->name:"NULL",
			os_task_name_LWPID(mutes_info->self_lock), 
			mutes_info->self_lock, 
			os_task_name_LWPID(mutes_info->wait_lock),
			mutes_info->wait_lock);
	}
	return 0;
}

int task_mutex_graph_show(void)
{
	int i = 0;
	pthread_mutex_lock(&_mutex);
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "=====================================\r\n");
	for(i = 0; i < TASK_MUTEX_GRAPH_MAX; i++)
	{
		if(_task_mutex_graph[i].use == 1)
		{
			task_mutex_graph_show_info(&_task_mutex_graph[i]);
		}
	}
	os_log_entry(NULL, 0, OS_LOCK_ERR_FILE, "=====================================\r\n");
	pthread_mutex_unlock(&_mutex);
	return 0;
}
#endif
#endif
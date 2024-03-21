/*
 * os_sem_errchk.h
 *
 *  Created on: May 30, 2017
 *      Author: zhurish
 */

#ifndef __OS_SEM_ERRCHK_H__
#define __OS_SEM_ERRCHK_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef OS_LOCK_ERR_CHECK_GRAPH_VIEW
#define TASK_MUTEX_GRAPH_MAX	2048
struct os_lock_source_type {
	int tid;
	enum {PROCESS, RESOURCE} type;
	void * lock;
	int degress;
};
struct os_lock_vertex {
	struct os_lock_source_type s;
	struct os_lock_vertex *next;
};
struct task_graph {
	struct os_lock_vertex list[TASK_MUTEX_GRAPH_MAX];
	int num;
	struct os_lock_source_type locklist[TASK_MUTEX_GRAPH_MAX];
	int lockidx;

	int path[TASK_MUTEX_GRAPH_MAX+1];
	int visited[TASK_MUTEX_GRAPH_MAX]; //节点是否被访问
	int k;
	int deadlock;
};

int task_mutex_graph_lock_before(int selfid, void *p);
int task_mutex_graph_lock_after(int selfid, void *p);
int task_mutex_graph_unlock_after(int selfid, void *p);
#endif

#ifdef OS_LOCK_ERR_CHECK
#define TASK_MUTEX_GRAPH_MAX	2048
struct task_mutex_graph 
{
	int type; //0:sem,1:mutex,2:spin
	void * mutex;
	int use;
};
int task_mutex_graph_add(int type, void *p);
int task_mutex_graph_del(int type, void *p);
int task_mutex_graph_show(void);
#endif


#ifdef __cplusplus
}
#endif


#endif /* __OS_SEM_ERRCHK_H__ */

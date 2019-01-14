/*
 * voip_task.h
 *
 *  Created on: 2018年12月27日
 *      Author: DELL
 */

#ifndef __VOIP_TASK_H__
#define __VOIP_TASK_H__


typedef struct voip_task_s
{
	int 	taskid;
	BOOL	enable;
	BOOL	active;
	BOOL	stream;		//voip stream or ring
	void 	*pVoid;
	void 	*pVoid1;
	void 	*pVoid2;
	void 	*pVoid3;
}voip_task_t;

extern voip_task_t	voip_task;



extern int voip_task_module_init();
extern int voip_task_module_exit();





#endif /* __VOIP_TASK_H__ */

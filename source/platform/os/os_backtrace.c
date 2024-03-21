/*
 * os_backtrace.c
 *
 *  Created on: 2019年8月18日
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zpl_type.h"
#include "os_list.h"
#include "os_task.h"
#include "os_backtrace.h"

static struct zpl_backtrace_symb  m_backtrace_symb;

int zpl_backtrace_symb_set(char *funcname, char *schedfrom, zpl_uint32 schedfrom_line)
{
	zpl_taskid_t  taskid = os_task_id_self();
	if(taskid != (zpl_taskid_t)ERROR)
		m_backtrace_symb.taskname = os_task_2_name(taskid);
	m_backtrace_symb.funcname = funcname;
	m_backtrace_symb.schedfrom = schedfrom;
	m_backtrace_symb.schedfrom_line = schedfrom_line;
	return 0;
}

const char * zpl_backtrace_symb_info(void)
{
	static char tmpbuf[512];
	if(m_backtrace_symb.schedfrom_line)
	{
		os_memset(tmpbuf, 0, sizeof(tmpbuf));
		os_snprintf(tmpbuf, sizeof(tmpbuf),
		"task '%s' function '%s', scheduled from file '%s', line %u", 
				m_backtrace_symb.taskname?m_backtrace_symb.taskname:"?",
				m_backtrace_symb.funcname?m_backtrace_symb.funcname:"?",
				m_backtrace_symb.schedfrom?m_backtrace_symb.schedfrom:"?", 
				m_backtrace_symb.schedfrom_line);
		return 	tmpbuf;	
	}
	return "Unknow";
}

#if 0
static zpl_uint32 * __getpc(void) __attribute__((noinline))
{
zpl_uint32 *rtaddr;
__asm__ volatile ("move %0, $31" : "=r"(rtaddr));
return rtaddr;
}
#endif
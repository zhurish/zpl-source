/*
 * zplos_include.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __ZPLOS_INCLUDE_H__
#define __ZPLOS_INCLUDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"

#include "os_memory.h"
#include "os_list.h"
#include "os_task.h"
#include "os_sem.h"
#include "os_time.h"
#include "os_ipstack.h"
#include "os_log.h"
#include "os_job.h"
#include "os_ansync.h"
#include "os_socket.h"
#include "os_signal.h"
#include "os_file.h"
#include "os_url.h"
#include "os_netservice.h"

#include "os_util.h"
#include "os_backtrace.h"
#include "os_bitmap.h"
#include "os_process.h"
#include "libnetpro.h"

#include "zpl_ipcmsg.h"
#include "zpl_skbuffer.h"

#ifdef ZPL_OS_QUEUE	
#include "os_queue.h"			
#endif
#ifdef ZPL_OS_AVL
#include "avl.h"				
#endif
#ifdef ZPL_OS_NVRAM	
#include "os_nvram.h"			
#endif
#ifdef ZPL_OS_JSON	
#include "cJSON.h"			
#endif
#ifdef ZPL_OS_TLV	
#include "os_tlv.h"			
#endif
#ifdef ZPL_OS_RNG	
#include "os_rng.h"			
#endif
#ifdef ZPL_OS_XYZ_MODEM		
#include "xyz_modem.h"		
#endif
#ifdef ZPL_OS_TTYCOM
#include "tty_com.h"				
#endif

#ifdef ZPL_OS_UCI
#include "os_uci.h"				
#endif

#include "list_tree.h"
#include "osker_list.h"
#include "rbtree.h"

#define OS_THREAD       1
#ifdef ZPL_IPCOM_MODULE
#define ELOOP_THREAD    1
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ZPL_INCLUDE_H__ */

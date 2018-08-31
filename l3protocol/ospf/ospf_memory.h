/* ospf_memory.h --- memory api*/


#if !defined (_OSPF_MEMORY_H_)
#define _OSPF_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif 
#include "ospf_main.h"
#include "ospf_table.h"

#define OSPF_MAX_PROCESS_NUM        (32)
#define OSPF_MAX_IF_NUM             (32*OSPF_MAX_PROCESS_NUM)
#define OSPF_MAX_AREA_NUM           (32*OSPF_MAX_PROCESS_NUM)
#define OSPF_MAX_NBR_NUM            (512*OSPF_MAX_PROCESS_NUM)
#define OSPF_MAX_LSA_NUM            (4000*OSPF_MAX_PROCESS_NUM)
#define OSPF_MAX_ROUTE_NUM          (4000*OSPF_MAX_PROCESS_NUM)
#define OSPF_MAX_SPF_NUM            (1024)
#define OSPF_MAX_RANGE_NUM          (128)
#define OSPF_MAX_REDISTRIBUTE_NUM   (64)
#define OSPF_MAX_TEROUTER_NUM       (64)
#define OSPF_MAX_TELINK_NUM         (16)
#define OSPF_MAX_POLICY_NUM         (16)
#define OSPF_MAX_TETUNNEL_NUM       (10)
#define OSPF_MAX_DCN_NUM            (500)
#define OSPF_MAX_ASBRRANGE_NUM      (500)


 /*limit max and min memory zone size*/
 
 /*min zone size is 4M bytes*/
#define OSPF_MIN_MZONE_SIZE (4*1024*1024)
 
 /*max zone size is 64M bytes*/
#define OSPF_MAX_MZONE_SIZE (64*1024*1024)
 
 /*64K bytes == 1 page*/
#define OSPF_PAGE_SIZE 65536
 
 /*memory statistic unit for a special memory type*/
 struct ospf_mstat{
	 /*total alloate count*/
	 u_int allocate;
  
	 /*allocate failed,count*/
	 u_int fail;
  
	 /*total free count*/ 
	 u_int del;
 };
 
 /*memory type configure*/
 struct ospf_memconfig{
	 /*length of this type,0 means variable*/
	 u_int len;
	 
	 /*total number of this type*/
	 u_int count;
 }; 
  
 /*memory object*/
 struct ospf_mhdr{
	 struct ospf_mhdr *p_next;
 };
 
 /*memory table*/
 struct ospf_mtable{
	 /*max size of object in this table*/
	 u_int size;
	
	 /*allcoate page statistic count*/
	 u_int page_alloc;
	
	 /*free page statistic count*/
	 u_int page_free;
	 
	 /* pages have free buffer*/
	 struct ospf_lst page_list;
	
	 /*pages are full*/
	 struct ospf_lst full_page_list;
 };
 
 /*page struct*/
 struct ospf_mpage{
	 /*node linked to memory table*/
	 struct ospf_lstnode node;
 
	 struct ospf_mtable *p_mtable;
 
	 /*total count of memory object*/
	 u_short total;
 
	 /*total count of memory object not used*/
	 u_short free;
	 
	 /*first free memory object*/
	 struct ospf_mhdr *p_free;
 };  
 
 /*memory zone struct,contain a linear buffer for page*/
 struct ospf_mzone{
	 struct ospf_lstnode node;
	
	 /*buffer start*/
	 u_int8 *buf;
	
	 /*sizeof zone in bytes*/
	 u_int size : 31;
 
	 u_int expand : 1;
 };
 
 /*max count of memory zone*/
#define OSPF_MAX_MZONE 64 
 /*memory type defined in ospf*/
 enum{
	 OSPF_MPACKET = 0, 
	 OSPF_MAREA, 
	 OSPF_MSPF, 
	 OSPF_MIF,		
	 OSPF_MIPROUTE, 
	 OSPF_MRXMT, 
	 OSPF_MDBD, 
	 OSPF_MLSA, 
	 OSPF_MROUTE, 
	 OSPF_MNBR, 
	 OSPF_MRANGE, 
	 OSPF_MREQUEST, 
	 OSPF_MNETWORK, 
	 OSPF_MREDISTRIBUTE,  
	 OSPF_MTEROUTER, 
	 OSPF_MTELINK, 
	 OSPF_MINSTANCE, 
	 OSPF_MNEXTHOP, 
	 OSPF_MPOLICY,
	 OSPF_MLSTABLE,
	 OSPF_MSTAT,
    #ifdef OSPF_FRR
	 OSPF_MBACKSPF,
	 OSPF_MBACKROUTE,
    #endif
	 OSPF_MTETUNNEL,
     OSPF_MDCN,
     OSPF_MASBRRANGE,
	 OSPF_MTYPE_MAX
 };

 
 /*global struct for memory process*/
#define OSPF_MAX_MEMPOOL 9
 struct ospf_memory{
	 struct ospf_mstat stat[OSPF_MTYPE_MAX];
 
	 struct ospf_mstat histroy_stat[OSPF_MTYPE_MAX];
 
	 struct ospf_memconfig config[OSPF_MTYPE_MAX];
 
	 struct ospf_mtable table[OSPF_MAX_MEMPOOL];
 
	 struct ospf_lst free_page_list;
 
	 /*allocated zone list*/
	 struct ospf_lst zone_list;
 
	 /*timer to free not used mzone */
	 struct ospf_timer mzone_timer;
	 
	 /*total length of buffer*/
	 u_int total_len;
 
	 /*total allocated length of buffer,there may be error*/
	 u_int allocate_len;
 
	 /*zone buffer allocate failed count*/
	 u_int zone_allocate_failed;
 
	 /*total expand len*/
	 u_int expand_len;
	 
	 u_int max_process;
	 
	 u_int max_lsa;
 
	 u_int max_route;
 
	 u_int max_if;
 
	 u_int max_nbr;
 
	 u_int max_area;
 
	 /*zone resource*/
	 struct ospf_mzone zone[OSPF_MAX_MZONE];
 };
 
 /*for a special length of memory ,increase most matched unit count of memory*/
#define ospf_increase_memory(s, c) do{p_memory->total_len += (s)*(c);}while(0)
#define OSPF_EXPAND_MBLOCK_SIZE 4*1024*1024
#define OSPF_MEMORY_TIMER_INTERVAL 3600
#define OSPF_MEMORY_INIT_SPLIT_SIZE 32*1024*1024

 
/*allocate buffer according type*/
#define ospf_malloc2(type) ospf_malloc(0, type)

void *ospf_malloc(u_int size, u_short type);
void ospf_set_init_num(u_int max_process, u_int max_lsa, u_int max_route, u_int max_if, u_int max_area, u_int max_nbr);
void ospf_mfree(void *ptr, u_int type);
void ospf_init_memory(void);
void ospf_destory_memory(void);
void ospf_reinit_memory(void);
void ospf_display_memory_statistics(void);
void ospf_verify_memory(void);

#ifdef __cplusplus
}
#endif

#endif  


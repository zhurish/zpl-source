 

#if !defined (_OSPF_UTIL_H_)
#define _OSPF_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "vty.h"

#ifndef WIN32
#define ospf_logx(x, args...) do{if (x) ospf_log(1, args);}while(0)
#else
//#define ospf_logx ospf_log
#endif

/*network compare considering mask*/
#define ospf_netmatch(d1, d2, mask) (((d1) & (mask)) == ((d2) & (mask)))
u_int ospf_rand(u_int base);

int ospf_log(u_int on, const char * format,...);

u_int8 * ospf_print_lshdr(struct ospf_lshdr *p_lsh,u_int8 *p_string);

void ospf_clear_statistic(struct ospf_process *p_process);
void ospf_domain_update_check(struct ospf_process *p_process);
void ospf_domain_update_route(struct ospf_process *p_process);
void ospf_log_packet_hdr(struct ospf_hdr *p_hdr);
void ospf_log_hello_packet(struct ospf_hello_msg * p_hello);
void ospf_log_lsa_header(struct ospf_lshdr *p_lshdr);
void ospf_log_dd_packet(struct ospf_dd_msg *p_msg, u_int header_only);
void ospf_log_request_packet(struct ospf_request_msg * p_msg, u_int header_only);
void ospf_log_ack_packet(struct ospf_ack_msg * p_msg, u_int header_only);
void ospf_log_update_packet(struct ospf_update_msg * p_msg, u_int header_only);

void ospf_display_area(struct vty *vty, u_int area_id, u_long ulProId);
void ospf_display_lsa_table(
							struct vty *vty, 
                           int type, 
                           u_int areaid, 
                           u_int display_num);
void ospf_display_lsa_cnt(struct vty *vty );
void ospf_display_vif(struct vty *vty );
void ospf_display_interface(struct vty *vty, int if_addr);
void ospf_display_route_summary(struct vty *vty, u_long ulInstance, u_int display_num);
void ospf_display_route_detailed(
				struct vty *vty, 
                 u_int route_dest, 
                 u_int route_mask);
void ospf_display_neighbor(struct vty *vty, u_int peer);
void ospf_display_gr(struct vty *vty,u_int uiProcess);
void ospf_display_process(struct vty *vty, u_int process_id);
void ospf_display_global(struct vty *vty);
void ospf_display_stat(struct vty *vty, u_int count);
void ospf_display_nexthop(struct vty *vty, u_long ulInstance);
void ospf_display_timer(struct vty *vty, u_long ulInstance, u_long ulIfIndex);
void ospf_display_timer_thread(struct vty *vty);
void ospf_display_conflict_network(struct vty *vty, u_long ulInstance);
void ospf_ase_test_all(u_int dest, 
                  u_int mask, 
                  u_int lsacount, 
                  u_int active);
void ospf_display_lstable(struct vty *vty, u_long ulInstance);
#ifdef HAVE_BFD
void ospf_display_bfd_stat(struct vty *vty, u_int count);
#endif
//void ospf_display_lsa_router_id_count(uint32_t process)
void ospf_display_nm_table(struct vty *vty);
void ospf_display_import_table(struct vty *vty, u_long ulInstance, u_int count);
void ospf_display_backup_route(struct vty *vty, u_long ulProId);
void ospf_display_local_data(struct vty *vty, u_int instance_id, u_int uiIfIndx);
void ospf_display_nbr_root(struct vty * vty,u_int count);

void ospf_display_task_msg(struct vty *vty, u_int uiProId, u_int uiIfIndex);
void ospf_task_msg_cnt_clear(u_int uiProId);

void ospf_display_iftable_cnt(struct vty *vty, u_int uiProId);
void ospf_display_rttable_cnt(struct vty *vty, u_int uiProId);
void ospf_iftable_show(struct vty *vty, struct ospf_if *pstIf);
void ospf_msg_choose_show(struct vty *vty, u_int uiProId, u_int uiIfIndex, u_int uiMsgType);
void ospf_msg_cnt_choose_clear(u_int uiProId, u_int uiMsgType);
void ospf_display_pstlsa(struct vty *vty, u_int uiProId);
void ospf_display_iftable_pstIf(struct vty *vty, u_int uiProId);
void ospf_display_dcn_cnt(struct vty *vty, u_int uiProId);
void ospf_dcn_cnt_choose_clear(struct vty *vty, u_int uiProId, u_int uiType);


#ifdef __cplusplus
}
#endif 
#endif  

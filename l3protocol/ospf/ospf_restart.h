 

#if !defined (_OSPF_RESTART_H_)
#define _OSPF_RESTART_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*restart related*/ 
#define OSPF_DEFAULT_RESTART_TIME 120    /*默认的GR定时器时间*/

enum {
    OSPF_GR_REASON_UNKNOWN = 0,/*重启原因 0*/
    OSPF_GR_REASON_REBOOT,  /*重启原因 1*/
    OSPF_GR_REASON_UPGRADE,  /*重启原因 2*/
    OSPF_GR_REASON_SWITCH /*重启原因 3*/
};

#define OSPF_OPAQUE_TYPE_GR 3    /*GR LSA头部中的Opaque 类型*/

#define OSPF_GR_TLV_TYPE_TIME  1    /*GR TLV类型1*/
#define OSPF_GR_TLV_LEN_TIME  4    /*重启阶段TLV长度*/

#define OSPF_GR_TLV_TYPE_REASON  2    /*重启原因TLV类型*/
#define OSPF_GR_TLV_LEN_REASON  1    /*重启原因TLV长度*/

#define OSPF_GR_TLV_TYPE_ADDR  3    /*接口地址TLV类型*/
#define OSPF_GR_TLV_LEN_ADDR  4    /*接口地址TLV长度*/

void ospf_restart_request(struct ospf_process * p_process, u_int restart_status);
void ospf_restart_start(struct ospf_process *p_process) ;
void ospf_restart_finish(struct ospf_process * p_process, u_int reason);
void ospf_restart_lsa_originate(struct ospf_if * p_if);
void ospf_restart_helper_finish_all(struct ospf_process *p_process, u_int reason);
void ospf_restart_helper_finish(struct ospf_nbr * p_nbr,  u_int reason);
void ospf_restart_lsa_recv(struct ospf_lsa * p_current);
void ospf_restart_timeout(struct ospf_process *p_process);
void ospf_restart_helper_timeout(struct ospf_nbr *p_nbr);
void ospf_restart_wait_timeout(struct ospf_process *p_process);
u_int ospf_all_lsa_acked(struct ospf_process *p_process);
u_int ospf_restart_helper_affected_by_lsa(struct ospf_lsa *p_lsa);
u_int ospf_restart_topo_changed(struct ospf_process * p_process);
u_int ospf_restart_completed(struct ospf_process *p_process);

#ifdef __cplusplus
}
#endif 
#endif  

 

#if !defined (_OSPF_RESTART_H_)
#define _OSPF_RESTART_H_

#ifdef __cplusplus
extern "C" {
#endif 

/*restart related*/ 
#define OSPF_DEFAULT_RESTART_TIME 120    /*Ĭ�ϵ�GR��ʱ��ʱ��*/

enum {
    OSPF_GR_REASON_UNKNOWN = 0,/*����ԭ�� 0*/
    OSPF_GR_REASON_REBOOT,  /*����ԭ�� 1*/
    OSPF_GR_REASON_UPGRADE,  /*����ԭ�� 2*/
    OSPF_GR_REASON_SWITCH /*����ԭ�� 3*/
};

#define OSPF_OPAQUE_TYPE_GR 3    /*GR LSAͷ���е�Opaque ����*/

#define OSPF_GR_TLV_TYPE_TIME  1    /*GR TLV����1*/
#define OSPF_GR_TLV_LEN_TIME  4    /*�����׶�TLV����*/

#define OSPF_GR_TLV_TYPE_REASON  2    /*����ԭ��TLV����*/
#define OSPF_GR_TLV_LEN_REASON  1    /*����ԭ��TLV����*/

#define OSPF_GR_TLV_TYPE_ADDR  3    /*�ӿڵ�ַTLV����*/
#define OSPF_GR_TLV_LEN_ADDR  4    /*�ӿڵ�ַTLV����*/

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

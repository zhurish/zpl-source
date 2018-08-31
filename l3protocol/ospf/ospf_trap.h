/* ospf_trap.h */

#if !defined (_OSPF_trap_H_)
#define _OSPF_trap_H_

#ifdef __cplusplus
extern "C" {
#endif 




void ospf_trap_ifstate(u_int old_state, struct ospf_if *p_if);
void  ospf_trap_vifstate(u_int old_state, struct ospf_if *p_if); 
void  ospf_trap_nbrstate(u_int old_state,  struct ospf_nbr *p_nbr);
void  ospf_trap_vnbrstate(u_int old_state,  struct ospf_nbr *p_nbr);
void  ospf_trap_iferror(u_int error_type, u_int packet_type, u_int packet_source, struct ospf_if *p_if);
void  ospf_trap_viferror(u_int error_type, u_int packet_type, struct ospf_if *p_if);
void  ospf_trap_ifautherror(u_int error_type, u_int packet_type, u_int packet_source, struct ospf_if *p_if);
void  ospf_trap_vifautherror(u_int error_type, u_int packet_type, struct ospf_if *p_if);
void  ospf_trap_ifbadpacket(u_int packet_type, u_int packet_source, struct ospf_if *p_if);
void  ospf_trap_vifbadpacket(u_int packet_type,  struct ospf_if *p_if);
void  ospf_trap_ifretransmit(struct ospf_lshdr * p_lshdr, struct ospf_if * p_if, struct ospf_nbr * p_nbr);
void  ospf_trap_vifretransmit(struct ospf_lshdr * p_lshdr, struct ospf_if * p_if);
void  ospf_trap_origin( struct ospf_lsa * p_lsa);
void  ospf_trap_maxage( struct ospf_lsa * p_lsa);
void  ospf_trap_overflow(struct ospf_process *p_process);
void  ospf_trap_near_overflow(struct ospf_process *p_process);
void  ospf_trap_translator_state(struct ospf_area *p_area);
void  ospf_trap_restartstatus(struct ospf_process *p_process);
void  ospf_trap_helperstatus(struct ospf_nbr *p_nbr );
void  ospf_trap_virtual_helperstatus(struct ospf_nbr *p_nbr );



#ifdef __cplusplus
}
#endif

/*20090914 code minor adjust e*/
#endif /* _OSPF_STRUCTURES_H_ */


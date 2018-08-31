
#ifdef NEW_BGP_WANTED
#ifndef  BGP4GR_H
#define BGP4GR_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
void bgp4_peer_stale_route_set(tBGP4_PEER *p_peer,u_int af);
void bgp4_peer_restart_finish(tBGP4_PEER *p_peer);
void bgp4_peer_restart_timeout(tBGP4_PEER *p_peer);
void bgp4_schedule_all_init_update(void);
void bgp4_restart_waiting_timeout(void);
void bgp4_local_restart_state_update(void);
u_short bgp4_restart_capability_fill(tBGP4_PEER *p_peer,u_char *p_buf);
int bgp4_local_grace_restart(tBGP4_PEER * p_peer);
int bgp4_remote_grace_restart(tBGP4_PEER * p_peer);

#ifdef __cplusplus
}
#endif  

#endif

#else
#ifndef  BGP4GR_H
#define BGP4GR_H
#ifdef __cplusplus
      extern "C" {
     #endif
     
void bgp4_delete_stale_route(tBGP4_PEER *p_peer,u_int af,u_int force);
void bgp4_make_route_stale(tBGP4_PEER *p_peer,u_int af);
int bgp4_peer_execute_gr(tBGP4_PEER *p_peer,u_int role);
int bgp4_peer_down_to_gr(tBGP4_PEER *p_peer);
void bgp4_peer_exit_gr(tBGP4_PEER *p_peer);
void bgp4_peer_gr_timeout(tBGP4_PEER *p_peer);


int bgp4_extract_gr_capability(tBGP4_PEER *p_peer,u_char *p_msg,u_short msg_len);
u_short bgp4_build_gr_capability(tBGP4_PEER *p_peer,u_char *p_buf);


u_int bgp4_gr_if_can_update();
void bgp4_process_gr_open(tBGP4_PEER *p_peer,u_char gr_exist);
void bgp4_gr_send_all_init_update();
void bgp4_gr_update_deferral_route(tBGP4_ROUTE*p_route);
void bgp4_gr_deferral_seletion_timeout(void);

#ifdef __cplusplus
     }
     #endif  
#endif

#endif

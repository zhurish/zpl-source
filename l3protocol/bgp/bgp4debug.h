#ifdef NEW_BGP_WANTED

#ifndef BGP4DEBUG_H
#define BGP4DEBUG_H
#ifdef __cplusplus
 extern "C" {
#endif

#ifndef WIN32
#define bgp4_log(x,args...) do{if (gbgp4.debug & (x)) bgp4_logx(0xffffffff, args);}while(0)
#else
#define bgp4_log bgp4_logx
#endif 

//extern unsigned char*inet_ntoa_1(unsigned char * ip_str, long ipaddress);

short bgp4_logx(u_int u4_flag, const char *format, ...);
u_char *bgp4_printf_peer(tBGP4_PEER * pPeer, u_char * pAddr);
u_char *bgp4_printf_route(tBGP4_ROUTE * pRt, u_char * pAddr);
u_char *bgp4_printf_event(u_short event,u_char *p_str) ;
u_char *bgp4_printf_addr(tBGP4_ADDR *p, u_char *str);
u_char *bgp4_printf_syserror(u_int code,u_char *str);
u_char *bgp4_printf_state(u_short state , u_char *p_str);
u_char* bgp4_printf_vpn_rd(tBGP4_ROUTE *p_route ,u_char* p_rd_str);
u_char *bgp4_print_rd(u_char *p_ecom, u_char *p_str);
//u_char *bgp4_printf_af(u_int af, u_char *p_str);
void bgp4_printf_notify(u_char code,u_char subcode,u_char *p_string, u_char *p_substring);
void bgp4_printf_aspath(tBGP4_PATH *p_info,u_char *pstring);
void bgp4_debug_packet(u_char * p_msg, u_short len);
char *bgp4_get_route_desc(u_int proto );

#ifdef __cplusplus
}
#endif 
#endif

#else
#ifndef BGP4DEBUG_H
#define BGP4DEBUG_H
#ifdef __cplusplus
      extern "C" {
     #endif

#ifndef WIN32
#define bgp4_log(x,y,args...) do{if (gBgp4.dbg_flag & x) bgp4_logx(1,y, args);}while(0)
#else
#define bgp4_log bgp4_logx
#endif

short bgp4_logx(u_int u4_flag, u_int u4_time_flag,const char *format, ...);
u_char * bgp4_printf_peer(tBGP4_PEER * pPeer, u_char * pAddr);
u_char * bgp4_printf_route(tBGP4_ROUTE * pRt, u_char * pAddr);
//extern unsigned char*inet_ntoa_1(unsigned char * ip_str, long ipaddress);

void bgp4_debug_rtrefresh_msg(u_char *pu1Msg,u_short u2Len);
void bgp4_debug_open_msg(u_char *pu1Msg,u_short u2Len);
void bgp4_debug_update_msg(u_char *pu1Msg,u_short u2Len);
void bgp4_debug_notify_msg(u_char *pu1Msg,u_short u2Len);

void bgp4_debug_packet(u_char * pu1Msg, u_short u2Len) ;
u_short bgp4_debug_orf_capability(u_char *pu1Buf,u_char u1CapLen,short i2Len);
void bgp4_debug_ipv4_prefix(u_char *pu1Msg,u_char u1_prefix_len);
void bgp4_debug_ipv6_prefix(u_char *pu1Msg,u_char u1_prefix_len);
short bgp4_debug_mp_nlri_route(u_char *pu1Msg,u_char u1_mp_code);
void bgp4_debug_mp_reach_nlri(u_char *pu1Msg,u_short u2Len);
void bgp4_debug_mp_unreach_nlri(u_char *pu1Msg,u_short u2Len);
void bgp4_printf_notify(u_char code,u_char subcode,u_char *p_string, u_char *p_substring);
u_char * bgp4_printf_syserror(u_int code,u_char *str);
void bgp4_printf_aspath(tBGP4_PATH *p_info,u_char *pstring);
char* bgp4_get_route_desc(u_int proto );
u_char *bgp4_printf_event(u_short event,u_char *p_str) ;
u_char * bgp4_printf_addr(tBGP4_ADDR *p, u_char *str);
void bgp4_debug_row_buffer(u_char *p_buf,u_short len);

extern int log_time_print (char *buf) ;
extern int vty_log(char * proto_str, int len);
extern int NM_DEBUG_OUTPUT(const char *format, ...);

short bgp4_debug_mp_nlri_route(u_char *msg,u_char u1_mp_code);
u_short bgp4_debug_orf_capability(u_char *pu1Buf,u_char u1CapLen,short i2Len);
void bgp4_debug_rtrefresh_msg(u_char *msg,u_short len);
void bgp4_debug_open_msg(u_char *msg,u_short len);
void bgp4_debug_update_msg(u_char *msg,u_short len);
void bgp4_debug_notify_msg(u_char *msg,u_short len);
void bgp4_debug_packet(u_char * msg, u_short len) ;
void bgp4_debug_ipv4_prefix(u_char *msg,u_char u1_prefix_len);
void bgp4_debug_ipv6_prefix(u_char *msg,u_char u1_prefix_len);
void bgp4_debug_mp_reach_nlri(u_char *msg,u_short len);
void bgp4_debug_mp_unreach_nlri(u_char *msg,u_short len);

#ifdef __cplusplus
     }
     #endif 
#endif

#endif



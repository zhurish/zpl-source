#include "bgp4_api.h"
#include "bgp4main.h"

#ifdef NEW_BGP_WANTED
#ifndef BGP4UTIL_H
#define BGP4UTIL_H

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum
{
    BGP4_SAF_VLBL = 128,
    BGP4_SAF_UCAST = 1,
    BGP4_SAF_MCAST = 2,
    BGP4_SAF_LBL = 4,
    BGP4_SAF_VPLS = 65
}BGP4_SAF_TYPE_E;

#define bgp4_get_2bytes(a,b) do{\
              a = (*(b)); a = a<<8; a += *((b)+1);}while(0)

#define bgp4_get_4bytes(a,b) do{memcpy(&a,b,4);a = ntohl(a);}while(0)

#define bgp4_fill_2bytes(a,b) do{\
    u_short x=b;\
    *(a) = x>>8;\
    *(a + 1) = x&0x00ff;	 \
    }while(0)

#define bgp4_fill_4bytes(a,b) do{\
    u_int x=b;\
    *(a) = x>>24;\
    *(a + 1) = (x>>16)&0x000000ff;	 \
    *(a + 2) = (x>>8)&0x000000ff;	 \
    *(a + 3) = (x)&0x000000ff;	 \
    }while(0)

/*set msg val and move pointer forward*/
#define bgp4_put_char(x,y) do{*(x) = (y);(x)++;}while(0)
#define bgp4_put_2bytes(x,y) do{bgp4_fill_2bytes((x), (y));(x) += 2;}while(0)
#define bgp4_put_4bytes(x,y) do{bgp4_fill_4bytes((x), (y));(x) += 4;}while(0)
#define bgp4_put_string(x,y,z) do{memcpy((x),(y),(z));(x) += (z);}while(0)

#define bgp4_bit2byte(x) (((x) + 7) / 8)

#define  bgp4_get_prefix(a, p, len) do { register short x; for (x=0; x < len; x++) { *(((u_char *)(p))+(x)) = *(a); (a)++; } for (;x < 4;x++) { *(((u_char *)(p))+(x)) = 0; } } while (0) 

int bgp4_prefixcmp(tBGP4_ADDR *a1, tBGP4_ADDR *a2);
int bgp4_subnet_match( tBGP4_ADDR* ip1, tBGP4_ADDR*  ip2,u_int if_unit); 
int bgp4_prefixmatch(u_char *p1, u_char * p2, u_int len);
u_int bgp4_len2mask(u_char masklen);
u_int bgp4_mask2len(u_int mask);
u_int bgp4_mem_init_maximum(u_int type);
u_int bgp4_ip6_mask2len(struct in6_addr *mask);
u_short bgp4_index_to_afi(u_int flag);
u_short bgp4_index_to_safi(u_int flag);
u_int bgp4_afi_to_index(u_short afi, u_short safi);
u_int bgp4_rand(u_int base);
void bgp4_ip6_prefix_make(u_char *addr, u_int len);
void bgp4_mem_init(u_int max_route,u_int max_path,u_int max_peer,u_int max_vpn_instance);
void bgp4_free(void *p_buf, u_int type) ;
void *bgp4_malloc(u_int len, u_int type);
void bgp4_mem_deinit(void);
void bgp4_display_memory(void *vty);

#ifdef __cplusplus
}
#endif   
#endif

#else

#ifndef BGP4UTIL_H
#define BGP4UTIL_H

#ifdef __cplusplus
      extern "C" {
     #endif

#define bgp_sem_take() vos_pthread_lock(&gbgp4.sem)

#define bgp_sem_give() if(gBgp4.sem){\
    vos_pthread_unlock(&gbgp4.sem);}

#define bgp4_get_2bytes(a,b) do{\
              a = (*(b)); a = a<<8; a += *((b)+1);}while(0)

#define bgp4_get_4bytes(a,b) do{memcpy(&a,b,4);a = ntohl(a);}while(0)

#define bgp4_fill_2bytes(a,b) do{\
    u_short x=b;\
    *(a) = x>>8;\
    *(a + 1) = x&0x00ff;     \
    }while(0)

#define bgp4_fill_4bytes(a,b) do{\
    u_int x=b;\
    *(a) = x>>24;\
    *(a + 1) = (x>>16)&0x000000ff;   \
    *(a + 2) = (x>>8)&0x000000ff;    \
    *(a + 3) = (x)&0x000000ff;   \
    }while(0)

/*set msg val and move pointer forward*/
#define bgp4_put_char(x,y) do{*(x) = (y);(x)++;}while(0)
#define bgp4_put_2bytes(x,y) do{bgp4_fill_2bytes((x), (y));(x) += 2;}while(0)
#define bgp4_put_4bytes(x,y) do{bgp4_fill_4bytes((x), (y));(x) += 4;}while(0)
#define bgp4_put_string(x,y,z) do{memcpy((x),(y),(z));(x) += (z);}while(0)

#define bgp4_bit2byte(x) (((x) + 7) / 8)

#define  bgp4_get_prefix(a, p, len) do { register short x; for (x=0; x < len; x++) { *(((u_char *)(p))+(x)) = *(a); (a)++; } for (;x < 4;x++) { *(((u_char *)(p))+(x)) = 0; } } while (0) 

int bgp4_subnet_match( tBGP4_ADDR* ip1, tBGP4_ADDR*  ip2,u_int if_unit);
int bgp4_shutdown_timer();
void bgp4_stop_timer(tTimerNode *p_timer);

int bgp4_prefixmatch(u_char *p1, u_char * p2, u_int len);
u_int bgp4_len2mask(u_char masklen);
void bgp4_check_timer()  ;
u_char bgp4_mask2len(u_int mask);

tBGP4_LISTNODE* bgp4_lstfirst(tBGP4_LIST* p_list);

tBGP4_LISTNODE* bgp4_lstnext(tBGP4_LIST* p_list, tBGP4_LISTNODE* p_node);

u_int bgp4_lstcnt(tBGP4_LIST* p_list);

int bgp4_mem_init(u_int max_route,u_int max_path,u_int max_peer,u_int max_vpn_instance);
void bgp4_free(void *p_buf, u_int type) ;
void *bgp4_malloc(u_int len, u_int type);
void bgp4_mem_deinit();
u_int bgp4_mem_init_maximum(u_int type);


#ifdef BGP_IPV6_WANTED
void bgp4_ip6_prefix_make(u_char *addr, u_int len);
u_int bgp4_ip6_mask2len(struct in6_addr *mask);
#endif
int bgp4_prefixcmp(tBGP4_ADDR *a1, tBGP4_ADDR *a2);

/*init timerlist*/
int 
bgp4_init_timer_list(void);

/*close timerlist*/
void
bgp4_shutdown_timer_list(void);


/*check if any timer expired*/
void 
bgp4_check_timer_list(void);


/*peer timer expired callback function,call peer fsm according to special event*/
void 
bgp4_peer_timer_expired(
                   void* arg1,/*event*/
                   void* arg2,/*peer pointer*/
                   void* arg3/*timer pointer*/);

/*start peer timer*/
void 
bgp4_start_peer_timer(
                  tTimerNode *p_timer, 
                  tBGP4_PEER* p_peer, 
                  u_int duration,
                  u_int event);

u_int bgp4_deferral_timer_start(tTimerNode *p_timer,u_int duration);

int bgp_sem_take_timeout();

#ifdef __cplusplus
     }
     #endif   
#endif


#endif

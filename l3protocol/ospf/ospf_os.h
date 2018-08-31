/* ospf_os.h */


#if !defined (_OSPF_OS_H_)
#define _OSPF_OS_H_

#ifdef __cplusplus
extern "C" {
#endif 
typedef int 		(*OSPF_FUNCPTR) ();	   /* ptr to function returning int */
typedef int 		(*OSPF_VOIDFUNCPTR)();

#if 0
#define RTV_VALUE1      0x100   /* assign or lock first additional metric */
#define RTAX_IFA		5	/* interface addr sockaddr present */
#define RTAX_DST		0	/* destination sockaddr present */
#define RTAX_GATEWAY	1	/* gateway sockaddr present */
#define RTAX_NETMASK	2	/* netmask sockaddr present */
#define RTF_UP		0x0001		/* route usable		  	*/
#endif

#ifndef USE_LINUX_OS
#define IPTOS_PREC_NETCONTROL         0xe0
#define IPTOS_PREC_INTERNETCONTROL    0xc0
#endif

/*定义接口的操作状态*/
#define USP_IF_OPER_DOWN	2/*M2_ifOperStatus_down*/
#define USP_IF_OPER_UP	1/*M2_ifOperStatus_up*/

/*同步标志位的偏移及掩码，hwapi层可用*/
#define USP_SYNCFLAG_SHIFT	0
#define USP_SYNCFLAG_MASK	0xff	/*移位后再用mask进行匹配*/

/*同步到槽位的偏移及掩码，hwapi层可用*/
#define USP_SYNCSLOT_SHIFT	11
#define USP_SYNCSLOT_MASK	0x1f	/*移位后再用mask进行匹配*/

/*同步到线卡的偏移及掩码，hwapi层可用*/
#define USP_SYNCCARD_SHIFT	8
#define USP_SYNCCARD_MASK	0x07	/*移位后再用mask进行匹配*/

/*同步到系统的偏移及掩码，hwapi层可用*/
#define USP_SYNCSYS_SHIFT	16		
#define USP_SYNCSYS_MASK	0xff	/*移位后再用mask进行匹配*/

/*LDP-OSPF联动*/
typedef enum
{
    OSPF_LDP_ERR_MSG = 0,
    OSPF_LDP_INIT_MSG,
    OSPF_LDP_UP_MSG,
    OSPF_LDP_MSG_MAX
};
/*LDP-OSPF联动*/

#define OSPF_MAX_RTMSG_LEN 1024

/*max ip address count of an ip interface*/
#define OSPF_MAX_IP_PER_IF 256
#define USP_OSPF_PROTO	51

struct ospf_routetokernal
{
    /*dest of route*/
    u_int dest;

    /*mask of route*/
    u_int mask;

    /*cost of route*/
    u_int metric;

    /*nexthop of route*/
    u_int fwdaddr;

    /*nexthop interface unit*/
    u_int if_unit;
    
    /*tag of route*/
    u_int tag;
    
    u_int process_id;

    /*flag from kernal,currently only IFF_UP used,indicating nexthop's link 
    operational state,ospf do not import route whose out interface is down*/
    u_int8 flags;

   /*intra/inter/external1/external2*/
    u_int8 path_type : 4;
   
    /*redistribute from bgp, and flag is RTFLAG_VPN_REMOTE*/
    u_int8 vpn_route : 2;

    /*for vpn_route:TRUE:originate type 3 lsa,FALSE: originate type 5 lsa
    for other route, always FALSE*/
    u_int8 vpn_internal : 1;

    /*when domain_id changed cause the same route orginate type 3 lsa to type 5 lsa,
    or type 5 lsa to type 3 lsa*/
    u_int8 vpn_type_change : 1;
    /*stanard protocol*/
    u_int8 proto;

    /*true :add route, false :delete route*/
    u_int8 active : 1;

    /*add for MPLS*/
    u_int8 cost_change : 1;

    /*for nssa use,Translate设置为0，NoTranslate设置为1，0为默认值*/
    u_int8 no_translate : 1;

    /*if need check ase lsa for this route?*/
    u_int8 update : 1;

    /*nssa route,used for external lsa forwarding addr*/
    u_int8 nssa_route : 1;

    /*true:backup route, false:primary route*/
    u_int8 backup_route : 1;

    /*shamlink, tetunnel*/
    u_int8 nexthop_type : 2;
    
    /*vrf id*/
    u_int8 vrf_id;/*新增vrf id*/

};

/*redistributed route to ospf,and exported route to ip kernal*/
struct ospf_iproute{
    /*node in special table*/
    struct ospf_lstnode node;

    /*pointer to process*/
    struct ospf_process *p_process;
    
    /*dest of route*/
    u_int dest;

    /*mask of route*/
    u_int mask;

    /*cost of route*/
    u_int metric;

    /*nexthop of route*/
    u_int fwdaddr;

    /*back up nexthop of route*/
    u_int bak_hop;

    /*nexthop interface unit*/
    u_int if_unit;
    
    /*tag of route*/
    u_int tag;
    
    u_int process_id;

    /*flag from kernal,currently only IFF_UP used,indicating nexthop's link 
    operational state,ospf do not import route whose out interface is down*/
    u_int8 flags;

   /*intra/inter/external1/external2*/
    u_int8 path_type : 4;
   
    /*redistribute from bgp, and flag is RTFLAG_VPN_REMOTE*/
    u_int8 vpn_route : 2;

    /*for vpn_route:TRUE:originate type 3 lsa,FALSE: originate type 5 lsa
    for other route, always FALSE*/
    u_int8 vpn_internal : 1;

    /*when domain_id changed cause the same route orginate type 3 lsa to type 5 lsa,
    or type 5 lsa to type 3 lsa*/
    u_int8 vpn_type_change : 1;
    /*stanard protocol*/
    u_int8 proto;

    /*true :add route, false :delete route*/
    u_int8 active : 1;

    /*add for MPLS*/
    u_int8 cost_change : 1;

    /*for nssa use,Translate设置为0，NoTranslate设置为1，0为默认值*/
    u_int8 no_translate : 1;

    /*if need check ase lsa for this route?*/
    u_int8 update : 1;

    /*nssa route,used for external lsa forwarding addr*/
    u_int8 nssa_route : 1;

    /*true:backup route, false:primary route*/
    u_int8 backup_route : 1;

    /*shamlink, tetunnel*/
    u_int8 nexthop_type : 2;
};


#ifndef mbroffset
#define mbroffset(t, f) ((u_int)&(((t*)0)->f))
#endif

#define ospf_sys_ticks() (os_system_tick() & OSPF_MAX_TICKS)
/*generate md5 seqnum*/
#define ospf_generate_seq()  (os_system_tick()/os_system_rate()+ospf.seq_offset)

/*compare two time value to get offset in ticks*/
#define ospf_time_differ(a,b) (((a) >= (b)) ? ((a) - (b)) : ((a) + OSPF_MAX_TICKS - (b)))

#define ospf_fill_16(p_buf, x) do\
   {\
       u_short sv = htons(x);memcpy(p_buf, &sv,2);\
   }while(0)

#define ospf_fill_32(p_buf, x) do\
   {\
       u_int sv = htonl(x);memcpy(p_buf, &sv,4);\
   }while(0)

#define ospf_create_sem() ospf.lock_sem = os_mutex_init()
#define ospf_semtake() os_mutex_lock(ospf.lock_sem, OS_WAIT_FOREVER)
#define ospf_semgive() os_mutex_unlock(ospf.lock_sem)
#define ospf_semtake_forever() ospf_semtake() 
#define ospf_semtake_timeout() os_mutex_lock(ospf.lock_sem, 1)


/*try to take semphore,return error if failed,only used in NM operation*/
#define ospf_semtake_try() os_mutex_lock(ospf.lock_sem, OS_WAIT_NO)//ospf_semtake_forever()
/*do{\
    if (OK != ospf_semtake_timeout())\
    {\
            printf("OSPF system busying\r\n");\
            ospf.forbidden_debug = FALSE;\
            return ERR;\
    }}while(0)*/


/*translate ospf route into iproute*/
#define ospf_route_to_ip_route(ir,or, gateway, cost, ifunit) do \
{  \
    (ir)->fwdaddr = gateway;\
    (ir)->dest = (or)->dest;\
    (ir)->mask = (or)->mask;\
    (ir)->metric = cost;\
    (ir)->proto = M2_ipRouteProto_ospf;\
    (ir)->if_unit = ifunit;\
}while(0)


#define ospf_sys_delay(x) os_task_delay(x);

#define ospf_ms_delay(x) os_task_delay(1);  //10ms




#define NLA_HDRLEN      ((int) NLA_ALIGN(sizeof(struct nlattr)))
#define NLA_DATA(nla)   ((char *) nla + NLA_HDRLEN)
#define NLA_TYPE(nla)   (nla->nla_type & NLA_TYPE_MASK)
#define NLA_OK(nla,len) ((len) >= (int)sizeof(struct nlattr) && \
                        (nla)->nla_len >= sizeof(struct nlattr) && \
                        (nla)->nla_len <= (len))
#define NLA_NEXT(nla,attrlen)    ((attrlen) -= RTA_ALIGN((nla)->nla_len), \
                                 (struct nlattr*)(((int8_t*)(nla)) + RTA_ALIGN((nla)->nla_len)))
#define NLA_ADD(nla, attrlen, type, data, len) ((nla)->nla_type = type;\
                                         (nla)->nla_len = attrlen;\
                                         memset((unsigned char *) (nla) + (nla)->nla_len, 0, attrlen);\
                                         memcpy(NLA_DATA(nla), (data), len);\
                                         NLA_NEXT((nla),(attrlen));)


#ifdef OSPF_REDISTRIBUTE
void ospf_sys_route_list_get(u_int vrid, u_int proto,struct ospf_lst *p_list);
#endif
void ospf_sys_mcast_set(struct ospf_if *p_if, int request);
void ospf_sys_packet_input_set(struct ospf_if *p_check_if, u_int enable);
#ifdef HAVE_BFD
void ospf_bind_bfd(struct ospf_nbr *p_nbr);
void ospf_unbind_bfd(struct ospf_nbr *p_nbr);
u_long ospf_if_bfd_var_get(struct ospf_if *pstIf, u_int mod_type);
#endif

void ospf_sock_disable(int s);
void ospf_sock_enable(int s);
u_int ospf_sys_is_local_addr(u_short vrid, u_int ifip);
u_int ospf_sys_is_local_network(u_short vrid, u_int network);
u_int ospf_select_router_id(u_int vrid);
u_int ospf_redistribute_policy_verify(struct ospf_iproute * p_ip_route);
u_int ospf_filter_policy_verify(struct ospf_iproute * p_ip_route, struct ospf_policy * p_policy_exclude);
u_int ospf_ifunit_to_addrlist(u_int if_unit, u_int ifindex, u_int max_count, u_int * ap_addr, u_int * ap_mask);

#ifdef USP_MULTIINSTANCE_WANTED
STATUS ospf_sys_net2ifunit(u_int vrid, u_int ifnet, u_int *p_if_unit,u_int mask);
#endif
STATUS ospf_sys_addr2ifunit(u_int vrid, u_int ifaddr,u_int *p_if_unit);
STATUS ospf_sys_ifmask_get(u_int ifaddr, u_int if_unit, void * p_val);
STATUS ospf_sys_route_add(struct ospf_iproute *p_route, u_int *no_rtmsg);
STATUS ospf_sys_route_delete(struct ospf_iproute *p_route, u_int *no_rtmsg);
STATUS ospf_sys_ifindex_get(u_int if_unit, u_int ifaddr, u_int * p_index);
STATUS ospf_sys_ifflag_get(u_int if_unit, u_int ifaddr, u_int * p_flag);
STATUS ospf_sys_ifspeed_get(u_int if_unit, u_int ifaddr, u_int *p_speed) ;
STATUS ospf_sys_ifaddr_first(u_int vrid, u_int *p_unit, u_int *addr, u_int *mask);
STATUS ospf_sys_netmaskbingvrf(u_int processid,u_int ifnet,u_int mask);
STATUS ospf_sys_ifaddr_next(u_int vrid, u_int *p_unit, u_int *addr, u_int *mask);
STATUS ospf_sys_ifmtu_get(u_int if_unit, u_int ifaddr, u_int *p_mtu); 


int ospf_socket_init(u_int uiVrf);
int ospf_close_sock(u_int uiVrf);
void ospf_socket_recv(int s);
void ospf_socket_send(struct ospf_hdr *p_packet, struct ospf_if *p_if, u_int dest, u_int len);



void ospf_rtsock_workmode_change();
void ospf_rtsock_link_state_change(u_int ifindex, u_int cmd);
STATUS ospf_rtsock_route_msg_insert(struct ospf_iproute * p_route);
#ifdef OSPF_RTSOCK_ENABLE
int ospf_rtsocket_init(void);
STATUS ospf_rtsock_route_msg_output(struct ospf_process * p_process);
void ospf_rtsock_recv(int s);
void ospf_rtsock_option_update(int sock);
extern STATUS ospf_rtSockOptionSet(int sockfd,int protoType,int opType);
#endif

u_int8* ospf_inet_ntoa(u_int8 *ip_str, u_int ipinput);



int ospf_add_route(struct ospf_iproute *pstOspf, struct ospf_process *pstProcess);
int ospf_del_route(struct ospf_iproute *pstOspf,struct ospf_process *pstProcess);

#ifdef HAVE_BFD
void ospf_mod_bfd(struct ospf_nbr *p_nbr,u_int mod_type);
#endif

void ospf_ldp_control(u_int uiIfIndex, u_int uiType);



#ifdef __cplusplus
}
#endif
#endif




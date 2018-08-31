/* ospf_trap.c - SNMP trap support*/

#include "ospf.h"

#if 0/*#if OS == VXWORKS || OS == VXWORKS_M*/
extern int sendOspfVirtIfStateChangeTrap(int routerId,int areaId,int neighbor,int state);
extern int sendOspfNbrStateChangeTrap(int routerId,int ipaddr,int index,int id,int state);
extern int sendOspfVirtNbrStateChangeTrap(int routerId,int area,int id,int state);
extern int sendOspfIfConfigErrorTrap(int routerId,int ipaddr,int lessif,int src,int errtype,int pkttype);
extern int sendOspfVirtIfConfigErrorTrap(int routerId,int area,int nei,int errtype,int pkttype);
extern int sendOspfIfAuthFailureTrap(int routerId,int ipaddr,int lessif,int src,int errtype,int pkttype);
extern int sendOspfVirtIfAuthFailureTrap(int routerId,int area,int nei,int errtype,int pkttype);
extern int sendOspfIfRxBadPacketTrap(int routerId,int ipaddr,int lessif,int src,int pkttype);
extern int sendOspfVirtIfRxBadPacketTrap(int routerId,int area,int nei,int pkttype);
extern int sendOspfTxRetransmitTrap(int routerId,int ipaddr,int lessif,int nip,int nless,int id,int pkttype,int larea,int ltype,int lid,int lrouterid);
extern int sendOspfVirtIfTxRetransmitTrap(int routerId,int area,int nei,int pkttype,int larea,int ltype,int lid,int lrouterid);
extern int sendOspfOriginateLsaTrap(int routerId,int lareaId,int ltype,int lid,int lrouterid);
extern int sendOspfMaxAgeLsaTrap(int routerId,int lareaId,int ltype,int lid,int lrouterid);
extern int sendOspfLsdbTrap(int trapType,int routerId,int limit );
extern int sendOspfIfStateChangeTrap(int routerId,int ipaddr,int lessif,int state);
extern int sendOspfNssaTranslatorStatusChangeTrap(int routerId,int area,int state);
extern int sendOspfRestartStatusChangeTrap(int routerId,int status,int inter,int reason);
extern int sendOspfNbrRestartHelperStatusChangeTrap(int routerId,int ipaddr,int less,int id,int status,int age,int reas);
extern int sendOspfVirtNbrRestartHelperStatusChangeTrap(int routerId,int area,int id,int status,int age,int reas);
#else
int sendOspfVirtIfStateChangeTrap(int routerId,int areaId,int neighbor,int state)
{}
int sendOspfNbrStateChangeTrap(int routerId,int ipaddr,int index,int id,int state)
{}
 int sendOspfVirtNbrStateChangeTrap(int routerId,int area,int id,int state)
    {}
 int sendOspfIfConfigErrorTrap(int routerId,int ipaddr,int lessif,int src,int errtype,int pkttype)
    {}
 int sendOspfVirtIfConfigErrorTrap(int routerId,int area,int nei,int errtype,int pkttype)
    {}
 int sendOspfIfAuthFailureTrap(int routerId,int ipaddr,int lessif,int src,int errtype,int pkttype)
    {}
 int sendOspfVirtIfAuthFailureTrap(int routerId,int area,int nei,int errtype,int pkttype)
    {}
 int sendOspfIfRxBadPacketTrap(int routerId,int ipaddr,int lessif,int src,int pkttype)
    {}
 int sendOspfVirtIfRxBadPacketTrap(int routerId,int area,int nei,int pkttype)
    {}
 int sendOspfTxRetransmitTrap(int routerId,int ipaddr,int lessif,int nip,int nless,int id,int pkttype,int larea,int ltype,int lid,int lrouterid)
    {}
 int sendOspfVirtIfTxRetransmitTrap(int routerId,int area,int nei,int pkttype,int larea,int ltype,int lid,int lrouterid)
    {}
 int sendOspfOriginateLsaTrap(int routerId,int lareaId,int ltype,int lid,int lrouterid)
    {}
 int sendOspfMaxAgeLsaTrap(int routerId,int lareaId,int ltype,int lid,int lrouterid)
    {}
 int sendOspfLsdbTrap(int trapType,int routerId,int limit )
    {}
 int sendOspfIfStateChangeTrap(int routerId,int ipaddr,int lessif,int state)
    {}
 int sendOspfNssaTranslatorStatusChangeTrap(int routerId,int area,int state)
    {}
 int sendOspfRestartStatusChangeTrap(int routerId,int status,int inter,int reason)
    {}
 int sendOspfNbrRestartHelperStatusChangeTrap(int routerId,int ipaddr,int less,int id,int status,int age,int reas)
    {}
 int sendOspfVirtNbrRestartHelperStatusChangeTrap(int routerId,int area,int id,int status,int age,int reas)
    {}

#endif
#define ospf_trap_check(ins,trap_bit) do{if (!(((ins)->trap_enable>>(trap_bit-1)) & 0x00000001)) return;}while(0)

#define ospf_trap_interval_check(last)\
    do{\
        if (ospf_sys_ticks()<(10 +(last))) \
            return;\
        else \
            (last) = ospf_sys_ticks();\
       }while(0)

void 
ospf_trap_ifstate(
               u_int old_state, 
               struct ospf_if *p_if)
{
    u_int buf[16] = {0};
    u_int count = 0;

    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_IFSTATE);
    /*
      1:  ospfRouterId
      2:  ospfIfIpAddress
      3:  ospfAddressLessIf
      4:  ospfIfState
      { ospfTraps 16 }
     */            
    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->addr;
    buf[count++] = 0;
    buf[count++] = p_if->state;
    /*TX*/        

    sendOspfIfStateChangeTrap(buf[0], buf[1], buf[2], buf[3]);
    return;
}

void 
ospf_trap_vifstate(
         u_int old_state, 
         struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = ospf_nbr_first(p_if);
    u_int buf[16] = {0};
    u_int count = 0;

    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_VIFSTATE);
    /*1: ospfRouterId
      2: ospfVirtIfAreaId
      3: ospfVirtIfNeighbor
      4: ospfVirtIfState
      ospfTraps 1 
      */
    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->p_area->id; 
    if (p_nbr)
    {
        buf[count++] = p_nbr->id;
    }
    else
    {
        count++;
    }
    buf[count++] = p_if->state;  

    sendOspfVirtIfStateChangeTrap(buf[0], buf[1], buf[2], buf[3]);  
    return;
}

void 
ospf_trap_nbrstate(
              u_int old_state,  
              struct ospf_nbr *p_nbr)
{

    struct ospf_process *p_process = NULL;
    u_int8 *s_state[9] = {"","Down","Attempt","Init", "2-way","ExStart", "Exchange", "Loading","Full"};	
    u_int buf[16] = {0};
    u_int count = 0;
    u_int8 nbr_str[32] = {0};  
    if((p_nbr == NULL)||(p_nbr->p_if == NULL))
    { 
        return ;
    }
    p_process = p_nbr->p_if->p_process;
    ospf_trap_check(p_nbr->p_if->p_process, OSPF_TRAP_BIT_NBRSTATE);

    /*1: ospfRouterId
    2: ospfNbrIpAddr
    3: ospfNbrAddressLessIndex
    4: ospfNbrRtrId
    5: ospfNbrState   
    { ospfTraps 2 }
        */      
    buf[count++] = p_nbr->p_if->p_process->router_id;
    buf[count++] = p_nbr->addr;
    buf[count++] = 0;
    buf[count++] = p_nbr->id;
    buf[count++] = p_nbr->state; 
    
    /*trap enable ,send trap,or write ro logging file*/
    if ((p_process->trap_enable>>(OSPF_TRAP_BIT_NBRSTATE-1)) & 0x00000001)
    {
        sendOspfNbrStateChangeTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    }
    else
    {
        if ((OSPF_NS_FULL == old_state) || (OSPF_NS_FULL == p_nbr->state))
        {
        	#if 0
            zlog_dispatch_logging(LOG_CRIT,snmpTrapLogActionGet(),
                "[trap]Ospf neighbor %s, state change from %s to %s\r\n",
                inet_ntoa_1(nbr_str, buf[1]),
                s_state[old_state], s_state[p_nbr->state]);   
			#endif
        }
    }
    return;
}

void 
ospf_trap_vnbrstate(
           u_int old_state,  
           struct ospf_nbr *p_nbr)
{
    u_int buf[16] = {0};
    u_int count = 0;

    ospf_trap_check(p_nbr->p_if->p_process, OSPF_TRAP_BIT_VNBRSTATE);
    /*1: ospfRouterId
      2: ospfVirtNbrArea
      3: ospfVirtNbrRtrId
      4: ospfVirtNbrState
      { ospfTraps 3 }
      */         
    buf[count++] = p_nbr->p_if->p_process->router_id;
    if (p_nbr->p_if->p_area)
    {
        buf[count++] = p_nbr->p_if->p_area->id;
    }
    else
    {
        count++;
    }
    buf[count++] = p_nbr->id;
    buf[count++] = p_nbr->state;

    sendOspfVirtNbrStateChangeTrap(buf[0], buf[1], buf[2], buf[3]);
    return;
}

void 
ospf_trap_iferror(
              u_int error_type,
              u_int packet_type,
              u_int packet_source, 
              struct ospf_if *p_if)
{
    u_int buf[16] = {0};
    u_int count = 0;

    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_IFERROR);
    /*1: ospfRouterId
      2: ospfIfIpAddress
      3: ospfAddressLessIf
      4: ospfPacketSrc
      5: ospfConfigErrorType
      6: ospfPacketType    
      { ospfTraps 4 }
    */        
    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->addr;
    buf[count++] = 0;
    buf[count++] = packet_source;
    buf[count++] = error_type;
    buf[count++] = packet_type;
    
    sendOspfIfConfigErrorTrap(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    return;
}

void 
ospf_trap_viferror(
             u_int error_type, 
             u_int packet_type, 
             struct ospf_if *p_if)
{
    u_int buf[16] = {0};
    u_int count = 0 ;
    struct ospf_nbr *p_nbr = ospf_nbr_first(p_if);
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_VIFERROR);
    /*1: ospfRouterId
      2: ospfVirtIfAreaId
      3: ospfVirtIfNeighbor
      4: ospfConfigErrorType
      5: ospfPacketType
      { ospfTraps 5 }
     */ 
    buf[count++] = p_if->p_process->router_id;
    if (p_if->p_area)
    {
        buf[count++] = p_if->p_area->id;
    }
    else
    {
        count++;
    }
    if (p_nbr)
    {
        buf[count++] = p_nbr->id;
    }
    else 
    {
        count++;
    }
    
    buf[count++] = error_type;
    buf[count++] = packet_type;
    
    sendOspfVirtIfConfigErrorTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    return;
}

void 
ospf_trap_ifautherror(
                 u_int error_type, 
                 u_int packet_type, 
                 u_int packet_source, 
                 struct ospf_if *p_if)
{
    u_int buf[16] = {0};
    u_int count = 0;
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_IFAUTHERROR);
    /*
    1: ospfRouterId
    2: ospfIfIpAddress
    3: ospfAddressLessIf
    4: ospfPacketSrc
    5: ospfConfigErrorType
    6: ospfPacketType    
    { ospfTraps 6 }
    */
    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->addr;
    buf[count++] = 0;
    buf[count++] = packet_source;
    buf[count++] = error_type;
    buf[count++] = packet_type;

    sendOspfIfAuthFailureTrap(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    return;
}

void
ospf_trap_vifautherror(
                   u_int error_type, 
                   u_int packet_type, 
                   struct ospf_if *p_if)
{       
    u_int buf[16] = {0};
    u_int count = 0;
    struct ospf_nbr *p_nbr = ospf_nbr_first(p_if);
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_VIFAUTHERROR);
    /*1: ospfRouterId
      2: ospfVirtIfAreaId
      3: ospfVirtIfNeighbor
      4: ospfConfigErrorType
      5: ospfPacketType
      { ospfTraps 7 }
    */  
    buf[count++] = p_if->p_process->router_id;
    if (p_if->p_area)
    {
        buf[count++] = p_if->p_area->id;
    }
    else
    {
        count++;
    }
    if (p_nbr)
    {
        buf[count++] = p_nbr->id;
    }
    else
    {
        count++;
    }
    
    buf[count++] = error_type;
    buf[count++] = packet_type;

    sendOspfVirtIfAuthFailureTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    return;
}

void 
ospf_trap_ifbadpacket(
                    u_int packet_type, 
                    u_int packet_source, 
                    struct ospf_if *p_if)
{
    u_int buf[16] = {0};
    u_int count = 0;

    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_IFBADPKT);
    /*1: ospfRouterId
      2: ospfIfIpAddress
      3: ospfAddressLessIf
      4: ospfPacketSrc
      5: ospfPacketType
      { ospfTraps 8 }
      */ 
    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->addr;
    buf[count++] = 0;
    buf[count++] = packet_source;  
    buf[count++] = packet_type;

    sendOspfIfRxBadPacketTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    return;
}

void 
ospf_trap_vifbadpacket(
                     u_int packet_type, 
                     struct ospf_if *p_if)
{    
    u_int buf[16] = {0};
    u_int count = 0;
    struct ospf_nbr *p_nbr = ospf_nbr_first(p_if);
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_VIFBADPKT);
    /*1: ospfRouterId
      2: ospfVirtIfAreaId
      3: ospfVirtIfNeighbor
      4: ospfPacketType
      { ospfTraps 9 }
    */
    buf[count++] = p_if->p_process->router_id;
    if (p_if->p_area)
    {
        buf[count++] = p_if->p_area->id;
    }
    else
    {
        count++;
    }
    if (p_nbr)
    {
        buf[count++] = p_nbr->id;
    }
    else
    {
        count++;
    }
    buf[count++] = packet_type;

    sendOspfVirtIfRxBadPacketTrap(buf[0], buf[1], buf[2], buf[3]);
    return;
}

void 
ospf_trap_ifretransmit(
                  struct ospf_lshdr *p_lshdr,
                  struct ospf_if *p_if,
                  struct ospf_nbr *p_nbr)
{
    u_int buf[16] = {0};
    u_int count = 0;
     u_int last_if_retx_time = 0;
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_IFRXMT);
    ospf_trap_interval_check(last_if_retx_time);

    /*1: ospfRouterId
      2: ospfIfIpAddress
      3: ospfAddressLessIf
      4: ospfNbrRtrId
      5: ospfPacketType
      6: ospfLsdbType
      7: ospfLsdbLsid
      8: ospfLsdbRouterId
      { ospfTraps 10 }
    */

    buf[count++] = p_if->p_process->router_id;
    buf[count++] = p_if->addr;
    buf[count++] = 0;
    buf[count++] = p_nbr->id;
    buf[count++] = OSPF_PACKET_UPDATE;  
    buf[count++] = p_lshdr->type;
    buf[count++] = p_lshdr->id;
    buf[count++] = p_lshdr->adv_id;

    sendOspfTxRetransmitTrap(buf[0], buf[1], buf[2], p_nbr->addr, 0, buf[3], buf[4], p_if->p_area->id, buf[5], buf[6], buf[7]);
    return;
}

void 
ospf_trap_vifretransmit(
                struct ospf_lshdr *p_lshdr,
                struct ospf_if *p_if)
{   
    u_int buf[16] = {0};
    u_int count = 0;
     u_int last_vif_retx_time = 0;
    struct ospf_nbr *p_nbr = ospf_nbr_first(p_if);
    ospf_trap_check(p_if->p_process, OSPF_TRAP_BIT_VIFRXMT);
    ospf_trap_interval_check(last_vif_retx_time);

    /*1: ospfRouterId
      2: ospfVirtIfAreaId
      3: ospfVirtIfNeighbor
      4: ospfPacketType
      5: ospfLsdbType
      6: ospfLsdbLsid
      7: ospfLsdbRouterId
      { ospfTraps 11 }
     */          
    buf[count++] =p_if->p_process->router_id;
    if (p_if->p_area)
    {
        buf[count++] = p_if->p_area->id;
    }
    else
    {
        count++;
    }

    if (p_nbr)
    {
        buf[count++] = p_nbr->id;
    }
    else
    {
        count++;
    }
    buf[count++] = OSPF_PACKET_UPDATE;  
    buf[count++] = p_lshdr->type;
    buf[count++] = p_lshdr->id;
    buf[count++] = p_lshdr->adv_id;

    sendOspfVirtIfTxRetransmitTrap(buf[0], buf[1], buf[2], buf[3], 0, buf[4],buf[5],buf[6]);
    return;
}

void 
ospf_trap_origin(struct ospf_lsa *p_lsa)
{
    u_int buf[16] = {0};
    u_int count = 0;
     u_int last_originate_lsa_time = 0;
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    ospf_trap_check(p_process, OSPF_TRAP_BIT_ORIGIN);
    ospf_trap_interval_check(last_originate_lsa_time);
    /*1: ospfRouterId
      2: ospfLsdbAreaId
      3: ospfLsdbType
      4: ospfLsdbLsid
      5: ospfLsdbRouterId
      { ospfTraps 12 }
    */    
    buf[count++] = p_process->router_id;
    buf[count++] = p_area ? p_area->id : 0;
    buf[count++] = (p_lsa->lshdr->type);
    buf[count++] = ntohl(p_lsa->lshdr->id);
    buf[count++] = ntohl(p_lsa->lshdr->adv_id);

    sendOspfOriginateLsaTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    return;
}

void 
ospf_trap_maxage(struct ospf_lsa *p_lsa)
{       
    u_int buf[16] = {0};
    u_int count = 0;
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
     u_int last_maxage_time = 0;

    ospf_trap_check(p_process, OSPF_TRAP_BIT_MAXAGE);
    ospf_trap_interval_check(last_maxage_time);

    /*1: ospfRouterId
      2: ospfLsdbAreaId
      3: ospfLsdbType
      4: ospfLsdbLsid
      5: ospfLsdbRouterId
      { ospfTraps 13 }
    */
    buf[count++] = p_process->router_id;
    buf[count++] = p_area ? p_area->id : 0;
    buf[count++] = p_lsa->lshdr->type;
    buf[count++] = ntohl(p_lsa->lshdr->id);
    buf[count++] = ntohl(p_lsa->lshdr->adv_id);

    sendOspfMaxAgeLsaTrap(buf[0], buf[1], buf[2], buf[3], buf[4]);
    return;
}

void 
ospf_trap_overflow(struct ospf_process *p_process)
{
    u_int buf[16] = {0};
    u_int count = 0;
     u_int last_overflow_time = 0;

    ospf_trap_check(p_process, OSPF_TRAP_BIT_OVERFLOW);
    ospf_trap_interval_check(last_overflow_time);
    /* 
      1: ospfRouterId
      2: ospfExtLsdbLimit
      { ospfTraps 14 }
    */
    buf[count++] = p_process->router_id;
    buf[count++] = p_process->overflow_limit;

    sendOspfLsdbTrap(1, buf[0], buf[1]);
    return;
}

void 
ospf_trap_near_overflow(struct ospf_process *p_process)
{
    u_int buf[16] = {0};
    u_int count = 0;
     u_int last_near_overflow_time = 0;
    
    ospf_trap_check(p_process, OSPF_TRAP_BIT_NEAROVERFLOW);
    ospf_trap_interval_check(last_near_overflow_time);

    /*{ ospfTraps 15 }*/
    buf[count++] = p_process->router_id;
    buf[count++] = p_process->overflow_limit;

    sendOspfLsdbTrap(2,buf[0],buf[1]);
    return;
}
           
void 
ospf_trap_translator_state(struct ospf_area *p_area)
{
    u_int buf[16] = {0};
    u_int count = 0;
    ospf_trap_check(p_area->p_process, OSPF_TRAP_BIT_TRANSLATOR);
    /*ospfRouterId, -- The originator of the trap
      ospfAreaId,
      ospfAreaNssaTranslatorState 
      { ospfTraps 17 }*/
    buf[count++] = p_area->p_process->router_id;
    buf[count++] = p_area->id;
    buf[count++] = p_area->nssa_translator;

    sendOspfNssaTranslatorStatusChangeTrap(buf[0], buf[1], buf[2]); 
    return;
}

void 
ospf_trap_restartstatus(struct ospf_process *p_process)
{
    u_int buf[16] = {0};
    u_int count = 0;
    ospf_trap_check(p_process, OSPF_TRAP_BIT_RESTARTSTUTAS);
    /*ospfRouterId, -- The originator of the trap
      ospfRestartStatus,
      ospfRestartInterval,
      ospfRestartExitReason 
      { ospfTraps 18 }
      */
    buf[count++] = p_process->router_id;
    buf[count++] = p_process->in_restart;
    buf[count++] = p_process->restart_period;
    buf[count++] = p_process->restart_exitreason;

    sendOspfRestartStatusChangeTrap(buf[0], buf[1], buf[2], buf[3]);
    return;
}

void 
ospf_trap_helperstatus(struct ospf_nbr *p_nbr)
{
    u_int buf[16] = {0};
    u_int count = 0;
    ospf_trap_check(p_nbr->p_if->p_process, OSPF_TRAP_BIT_HELPERSTUTAS);
    /*ospfRouterId, -- The originator of the trap
      ospfNbrIpAddr,
      ospfNbrAddressLessIndex,
      ospfNbrRtrId,
      ospfNbrRestartHelperStatus,
      ospfNbrRestartHelperAge,
      ospfNbrRestartHelperExitReason 
      { ospfTraps 19 }
      */
    buf[count++] = p_nbr->p_if->p_process->router_id;              
    buf[count++] = p_nbr->addr;
    buf[count++] = 0;
    buf[count++] = p_nbr->id;
    buf[count++] = p_nbr->in_restart;
    buf[count++] = ospf_timer_remain(&p_nbr->restart_timer, ospf_sys_ticks())/OSPF_TICK_PER_SECOND;
    buf[count++] = p_nbr->restart_exitreason;

    sendOspfNbrRestartHelperStatusChangeTrap(buf[0] ,buf[1], buf[2], buf[3],buf[4],buf[5],buf[6]);
    return;
}

void 
ospf_trap_virtual_helperstatus(struct ospf_nbr *p_nbr)
{
    u_int buf[16] = {0};
    u_int count = 0;
    ospf_trap_check(p_nbr->p_if->p_process, OSPF_TRAP_BIT_VIRHELPERSTUTAS);
    /*ospfRouterId, -- The originator of the trap
      ospfVirtNbrArea,
      ospfVirtNbrRtrId,
      ospfVirtNbrRestartHelperStatus,
      ospfVirtNbrRestartHelperAge,
      ospfVirtNbrRestartHelperExitReason 
      { ospfTraps 20 }
      */
    buf[count++] = p_nbr->addr;
    if (p_nbr->p_if->p_area)
    {
        buf[count++] = p_nbr->p_if->p_area->id;
    }
    else
    {
        count++;
    }   
    buf[count++] = p_nbr->id;
    buf[count++] = p_nbr->in_restart;
    buf[count++] = ospf_timer_remain(&p_nbr->restart_timer, ospf_sys_ticks())/OSPF_TICK_PER_SECOND;
    buf[count++] = p_nbr->restart_exitreason;

    sendOspfVirtNbrRestartHelperStatusChangeTrap(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    return;
}

void 
ospf_trap_test(void)
{
    return;
}

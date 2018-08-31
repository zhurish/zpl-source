#include "ospf.h"
#include "ospf_table.h"
#include "ospf_nm.h"
#include "ospf_api.h"


extern int log_time_print (u_int8 *buf);
extern int NM_DEBUG_OUTPUT(const char *format, ...);
u_long ulOspfmemFlag = 0;
//int vty_log (u_int8 *proto_str,int len);
#define OSPF_NEWLINE "\r\n"
void ospf_display_spf_stat(struct vty *vty, u_int count);
void ospf_display_nbr_stat(struct vty *vty, u_int count);
void ospf_display_lsa_stat(struct vty *vty, u_int count);
#ifdef HAVE_BFD
void ospf_display_bfd_stat(struct vty *vty, u_int count);
#endif

void 
ospf_lstwarning(void *p_table)
{ 
    ospf_logx(ospf_debug,"\n\rtable %x not inited",(unsigned int)p_table);
    return;
}

/*compare function used to unsorted list,we use avl to simulate it,
   always add as the last node.do not support lookup related functions
 */
 int 
unsort_cmp(
       const void *p1, 
       const void *p2)
{
    return (p1 == p2) ? 0 : 1;/*always return 1,*/
}

void 
ospf_lstinit2(
        struct ospf_lst *p_table, 
        void *cmp, 
        u_int off) 
{
#ifndef OSPF_LIST_FUNCTION
    p_table->cookie = OSPF_TABLE_COOKIE;
#endif     
    p_table->p_first = NULL;
    if (!cmp)
    {
        avl_create(&(p_table->avl), (int(*)(const void *, const void *))unsort_cmp, 128 + off, off);
    }
    else
    {
        avl_create(&(p_table->avl), (int(*)(const void *, const void *))cmp, 128 + off, off);
    }
    return;
}

/*api for unsorted list table,only add and delete are different*/
void 
ospf_lstadd_unsort(
             struct ospf_lst *p_table,
             void *p_info)
{
    struct avl_node *p_node = (struct avl_node *)(((u_int8*)p_info) + p_table->avl.avl_offset);
 
    p_node->avl_child[1] = NULL;
    p_node->avl_parent = NULL;
 
    /*tree is empty,add new node as root, and it is the first node*/
    if (NULL == p_table->avl.avl_root)
    {
        p_node->avl_child[0] = NULL;
        p_table->avl.avl_root = p_node;
        p_table->p_first = p_info ;
    }
    else
    {
        /*set new node as root,and set old root as it's left child*/
        p_node->avl_child[0] = p_table->avl.avl_root;
        p_table->avl.avl_root->avl_parent = p_node;
        p_table->avl.avl_root = p_node;
    }
    p_table->avl.avl_numnodes++;
    return; 
}

/*before calling this function,you must ensure that info is in table,if not true,
   this function will has error
 */
void 
ospf_lstdel_unsort(
             struct ospf_lst *p_table,
             void *p_info)
{
    struct avl_node *p_node = (struct avl_node *)(((u_int8*)p_info) + p_table->avl.avl_offset);
 
    /*case1:there is only one node,clear all*/
    if (1 == p_table->avl.avl_numnodes)
    {
        p_table->avl.avl_root = NULL;
        p_table->p_first = NULL;
    }/*delete root*/
    else if (p_table->avl.avl_root == p_node)
    {
        p_table->avl.avl_root = p_node->avl_child[0];
        p_node->avl_child[0]->avl_parent = NULL;
    }/*delete the first one*/
    else if (p_table->p_first == p_info)    
    {
        p_node = p_node->avl_parent;
        p_node->avl_child[0] = NULL;
        p_table->p_first = (void *)(((u_int8*)p_node) - p_table->avl.avl_offset);
    }/*intermidate node*/
    else
    {
        p_node->avl_parent->avl_child[0] = p_node->avl_child[0];
        p_node->avl_child[0]->avl_parent = p_node->avl_parent ;
    }
    p_table->avl.avl_numnodes--;
    return; 
}

/*random function for timer usage*/

/*1/5 rand offset*/
#define OSPF_RAND_RATE 5

u_int
ospf_rand(u_int base)
{
    u_int a = rand();
    
    if (base < OSPF_RAND_RATE)
    {
       return base;
    }
    if (a%2)
    {
       return base + a%(base/OSPF_RAND_RATE);
    }
    return base - a%(base/OSPF_RAND_RATE);
}

/*
   ospf_printf_area
   construct output string for area id
   if area number is less than 65535,use interger format
   else use ip address format
   
   Input
        area id
   Output
        formated display string of area.caller must ensure the memory space
   Return
        same as output
*/ 
u_int8 *
ospf_printf_area(
             u_int8 *string,
             u_int id)
{
    if (id < 65535)
    {
        sprintf(string, "%u", (unsigned int)id);
    }
    else
    {
        ospf_inet_ntoa(string, id);
    }
    return string;
}

/*********************************************************
  build output string for lsa summary information
*/
u_int8 * 
ospf_print_lshdr(
            struct ospf_lshdr *p_lsh,
            u_int8 *p_string)
{
    u_int8 s_id[16],s_adv[16];
    u_int len;
    u_int8 tmiestr[64];
    
    log_time_print(tmiestr);
    ospf_inet_ntoa(s_id,ntohl(p_lsh->id));
    ospf_inet_ntoa(s_adv,ntohl(p_lsh->adv_id));
    len = sprintf(p_string,"t=%d,id=%s,adv=%s\r\n",p_lsh->type,s_id,s_adv);
    sprintf(p_string + len,"\r\n%s :age=%d,seq=%08x,len=%d",tmiestr,ntohs(p_lsh->age),ntohl(p_lsh->seqnum),ntohs(p_lsh->len));
    return p_string;
}
/*display ospf packet header*/
void 
ospf_log_packet_hdr(struct ospf_hdr *p_hdr)
{
    u_int8 str[32];
    /*Ver=2,Type=1,Len=100
      ID=1.1.1.1
      Area=2.2.2.2
      Checksum=0xb908
      AuthType=None
      AuthKey=
     */
    ospf_logx(ospf_debug,"  packet:"); 
    ospf_logx(ospf_debug, "    ver=%d, type=%d, len=%d", 
       p_hdr->version, p_hdr->type, ntohs(p_hdr->len)); 
    ospf_logx(ospf_debug, "    router=%s",ospf_inet_ntoa(str, ntohl(p_hdr->router)));
    ospf_logx(ospf_debug, "    area=%s",ospf_inet_ntoa(str, ntohl(p_hdr->area)));
    ospf_logx(ospf_debug, "    csum=0x%04x, auth=%d", 
       ntohs(p_hdr->checksum), p_hdr->auth_type);
    return;
} 
/*display full hello packet*/
void
ospf_log_hello_packet(struct ospf_hello_msg *p_hello)
{
    u_int *p_nbr = p_hello->nbr;
    u_int total = ntohs(p_hello->h.len);
    u_int len = 0;
    u_int8 str_dr[32] = {0},str_bdr[32] = {0};
    
    /*print packet header*/
    ospf_log_packet_hdr(&p_hello->h);

    /*print fixed part in hello*/
    ospf_logx(ospf_debug, "    mask=%s", ospf_inet_ntoa(str_dr, ntohl(p_hello->mask)));
    ospf_logx(ospf_debug, "    hello interval=%d, dead interval=%d", 
        ntohs(p_hello->hello_interval),
        ntohl(p_hello->dead_interval));
    ospf_logx(ospf_debug, "    option=0x%02x, priority=%d", 
        p_hello->option, p_hello->priority);

    ospf_logx(ospf_debug, "    dr=%s, bdr=%s", ospf_inet_ntoa(str_dr, ntohl(p_hello->dr)),
        ospf_inet_ntoa(str_bdr, ntohl(p_hello->bdr)));

    for (len = sizeof(*p_hello) ; len < total ; len += 4, p_nbr++)
    {
        ospf_logx(ospf_debug, "    nbr=%s", ospf_inet_ntoa(str_dr, ntohl(*p_nbr)));
    }
    return;
} 
/*display lsa header buffer*/
void
ospf_log_lsa_header(struct ospf_lshdr *p_lshdr)
{
    u_int8 str[32];
    ospf_logx(ospf_debug, "    age=%d,option=%02x,type=%d", 
        ntohs(p_lshdr->age), p_lshdr->option, p_lshdr->type);
    ospf_logx(ospf_debug, "    id=%s", ospf_inet_ntoa(str, ntohl(p_lshdr->id)));
    ospf_logx(ospf_debug, "    adv=%s", ospf_inet_ntoa(str, ntohl(p_lshdr->adv_id)));
    ospf_logx(ospf_debug, "    seqnum=%08x,csum=%04x,len=%d",
        ntohl(p_lshdr->seqnum), ntohs(p_lshdr->checksum), ntohs(p_lshdr->len));
    return;
} 

/*display database msg*/
void
ospf_log_dd_packet(
           struct ospf_dd_msg *p_msg,
           u_int header_only/*TRUE:display fixed part,FALSE:display all buffer*/)
{
    struct ospf_lshdr *p_lshdr = p_msg->lshdr;
    u_int total = ntohs(p_msg->h.len);
    u_int len = 0;
    
    /*print packet header*/
    ospf_log_packet_hdr(&p_msg->h);

    ospf_logx(ospf_debug, "    mtu=%d, option=0x%02x", ntohs(p_msg->mtu), p_msg->option);
    ospf_logx(ospf_debug, "    flag=0x%02x, seqnum=%d", p_msg->flags, ntohl(p_msg->seqnum));

    if (header_only)
    {
		ospf_logx(ospf_debug, " ");
        return;
    }

    for (len = sizeof(*p_msg) ; len < total; len += sizeof(*p_lshdr), p_lshdr++)
    {
        ospf_logx(ospf_debug, "  lsa:");
        ospf_log_lsa_header(p_lshdr);
    }
	ospf_logx(ospf_debug, " ");
    return;
} 

/*display request msg*/
void
ospf_log_request_packet(
            struct ospf_request_msg *p_msg,
            u_int header_only/*TRUE:display fixed part,FALSE:display all buffer*/)
{
    struct ospf_request_unit *p_request = p_msg->lsa;
    u_int total = ntohs(p_msg->h.len);
    u_int len = 0;
    u_int8 str[32];
    u_int8 str2[32];

    /*print packet header*/
    ospf_log_packet_hdr(&p_msg->h);

    if (header_only)
    {
		ospf_logx(ospf_debug, " ");
        return;
    }

    for (len = sizeof(*p_msg) ; len < total; len += sizeof(*p_request), p_request++)
    {
        /*display information in request msg*/
        ospf_logx(ospf_debug, "    type=%d,id=%s,router=%s",
           p_request->type, 
           ospf_inet_ntoa(str, ntohl(p_request->id)),
           ospf_inet_ntoa(str2, ntohl(p_request->adv_id)));
    }
    ospf_logx(ospf_debug, " ");
    return;
} 

void
ospf_log_ack_packet(
              struct ospf_ack_msg *p_msg,
              u_int header_only/*TRUE:display fixed part,FALSE:display all buffer*/)
{
    struct ospf_lshdr *p_lshdr = p_msg->lshdr;
    u_int total = ntohs(p_msg->h.len);
    u_int len = 0;
    
    /*print packet header*/
    ospf_log_packet_hdr(&p_msg->h);

    if (header_only)
    {
        ospf_logx(ospf_debug, " ");
        return;
    }

    for (len = sizeof(*p_msg) ; len < total; len += sizeof(*p_lshdr), p_lshdr++)
    {
        ospf_logx(ospf_debug, "  lsa header");
        ospf_log_lsa_header(p_lshdr);
    }
    ospf_logx(ospf_debug, " ");
    return;
} 

void
ospf_log_update_packet(
              struct ospf_update_msg *p_msg,
              u_int header_only/*TRUE:display fixed part,FALSE:display all buffer*/)
{
    struct ospf_lshdr *p_lshdr = p_msg->lshdr;
    u_int total = ntohs(p_msg->h.len);
    u_int len = 0;
    u_int lslen = 0;
    
    /*print packet header*/
    ospf_log_packet_hdr(&p_msg->h);

    ospf_logx(ospf_debug, "    count=%d", ntohl(p_msg->lscount));
    if (header_only)
    {
        ospf_logx(ospf_debug, " ");
        return;
    }

    for (len = sizeof(*p_msg) ; len < total; len += lslen, p_lshdr = (struct ospf_lshdr *)((u_long)p_lshdr + lslen))
    {
        lslen = ntohs(p_lshdr->len);
        ospf_logx(ospf_debug, "  LSA:");
        ospf_log_lsa_header(p_lshdr);
    }
    ospf_logx(ospf_debug, " ");
    return;
}

/*********************************************************
  output string function
*/
int 
ospf_log(
       u_int on, 
       const char * format,...)
{  
    if ((0 == on) || (TRUE == ospf.forbidden_debug))
    {
      return -1;	
    }
    {
        va_list args;
        int  len;
        int pos = 0 ;
        u_int8 outbuf[256];
     #ifdef WIN32
        u_int8 tmiestr[64];
     
        log_time_print(tmiestr);
        if (ospf.p_running_process)
        {
            if (ospf.p_running_process->vrid)
            {
                pos += sprintf(outbuf,"\n\r%s [OSPFv2-%d-vrf%d]:", tmiestr,ospf.p_running_process->process_id, ospf.p_running_process->vrid);
            }
            else
            {
                pos += sprintf(outbuf,"\n\r%s [OSPFv2-%d]:", tmiestr,ospf.p_running_process->process_id);
            }
        }
        else
        {
            pos += sprintf(outbuf,"\n\r%s [OSPFv2]:", tmiestr);
        }
     #else
        if (ospf.p_running_process)
        {
            if (ospf.p_running_process->vrid)
            {
                pos += sprintf(outbuf,"[OSPFv2-%d-vrf%d]:", (int)ospf.p_running_process->process_id, (int)ospf.p_running_process->vrid);
            }
            else
            {
                pos += sprintf(outbuf,"[OSPFv2-%d]:", (int)ospf.p_running_process->process_id);
            }
        }
        else
        {
            pos += sprintf(outbuf,"[OSPFv2]:");
        }
     #endif
        va_start(args, format);
        
        len = vsprintf (outbuf + pos, format, args);  
        if (len < 0)
        {
           va_end(args);
           return -1;
        }
        va_end (args);
        
       // outbuf[pos + len] = '\0';
		#if 0
        //NM_DEBUG_OUTPUT("%s\r\n", outbuf);
		printf("%s\r\n", outbuf);
		#endif

        /*NM_DEBUG_OUTPUT("%s\r\n", outbuf);*/
        if(ospf.debugFd != -1)
        {
            outbuf[pos+len] = '\r';
            outbuf[pos+len+1] = '\n';
            outbuf[pos+len+2] = '\0';
            write(ospf.debugFd, outbuf, strlen(outbuf));
        }
        else
        {
            outbuf[pos+len] = '\0';
            if(ospf.debug & (1<<OSPF_DBG_SYSLOG))
            {
                zlog_debug(ZLOG_OSPF,"%s",outbuf);
            }
            else
            {
               // printf("%s\r\n", outbuf);
                vty_out_to_all_terminal("%s", outbuf);
            }
        }
        
        return (len + pos);
     }  
}

void 
ospf_display_area_x(
		struct vty *vty, 
         struct ospf_process *p_process, 
         u_int area_id)
{
    struct ospf_area  *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    u_int8 key_str[20]; 
    u_int8 area_str[20];
    u_int8 ret = 0;
    int i;

    vty_out(vty,"-----------process : %d -------------%s",p_process->process_id,OSPF_NEWLINE);
    /*All of Areas*/
    if (0xffffffff == area_id)
    {
        if (NULL == ospf_lstfirst(&p_process->area_table))
        {
            return;
        }

        ret = vty_out(vty, "%s%-18s%-10s%-18s%-6s%-6s%-10s",
              OSPF_NEWLINE, "AreaId", "Flag(Hex)", "RouteUpdates", "ABR", "ASBR", "LSA   Count");   
        
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            ospf_printf_area(area_str, p_area->id);
            
            ret = vty_out(vty, "%s%-18s%-10x%-18d%-6d%-6d%-10d", OSPF_NEWLINE,
                area_str, 0,(int)p_area->spf_run,
                (int)p_area->abr_count,(int)p_area->asbr_count,
                (int)p_area->lscount);      
        }
        
        ret = vty_out(vty, "%s",OSPF_NEWLINE);   
        return ;
    }
     
    p_area = ospf_area_lookup(p_process, area_id);
    
    if (NULL == p_area)
    {
        ret = vty_out(vty, "%sNo Such Area %u%s", OSPF_NEWLINE, (unsigned int)area_id, OSPF_NEWLINE);
        return  ;
    }
    
    ospf_printf_area(area_str, p_area->id);
    
    ret = vty_out(vty, "%sDetailed Information of    Area %s%s", OSPF_NEWLINE, area_str, OSPF_NEWLINE);
    
    ret = vty_out(vty, "%-28s:%d%s","Number of Range", (int)ospf_lstcnt(&(p_area->range_table)),OSPF_NEWLINE);          
       
    if (ospf_lstfirst(&(p_area->range_table)))
    {
        struct ospf_range *p_range = NULL;
        struct ospf_range *p_next_range = NULL ;
        u_int8 dest_str[16],mask_str[16];        
        
        ret = vty_out(vty, "%-16s%-16s%-8s%s%s","dest","mask","state","action", OSPF_NEWLINE);                  
        
        for_each_ospf_range(p_area, p_range, p_next_range)
        {
            memset(dest_str, 0, 16);
            memset(mask_str, 0, 16);
            ospf_inet_ntoa(dest_str, p_range->network);
            ospf_inet_ntoa(mask_str, p_range->mask);
            
            ret = vty_out(vty, "%-16s%-16s%-8d%s%s", dest_str, mask_str, 1,
                p_range->advertise == TRUE ? "advertise" : "noadvertise", OSPF_NEWLINE);            
        }
    }
    
    ret = vty_out(vty, "%-28s:%d%s","Stub    ",p_area->is_stub,OSPF_NEWLINE);                     
    ret = vty_out(vty, "%-28s:%d%s","Stub    total ",p_area->nosummary ,OSPF_NEWLINE);                     
    
    ret = vty_out(vty, "%-28s:%d%s","NSSA    ",p_area->is_nssa,OSPF_NEWLINE);                                
    ret = vty_out(vty, "%-28s:%d%s","NSSA    translator role",p_area->nssa_always_translate,OSPF_NEWLINE);                                
    ret = vty_out(vty, "%-28s:%d%s","NSSA    translator state",p_area->nssa_translator,OSPF_NEWLINE);                                
    
    ret = vty_out(vty, "%-28s:%d%s","Default Cost",(int)p_area->stub_default_cost[0].cost,OSPF_NEWLINE);                   
    ret = vty_out(vty, "%-28s:%d%s","ASBR Route",p_area->asbr_count,OSPF_NEWLINE); 
    ret = vty_out(vty, "%-28s:%d%s","ABR Route",p_area->abr_count,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","LSA",(int)p_area->lscount,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%08x%s","LSA Checksum",(unsigned int)p_area->cksum,OSPF_NEWLINE);
    
    /*Different LSA Count*/                 
    ret = vty_out(vty, "%-28s:%d%s","Number of Router LSA",(int)ospf_area_lsa_count(p_area,1),OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Number of Network LSA",(int)ospf_area_lsa_count(p_area,2),OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Number of Summary LSA",(int)ospf_area_lsa_count(p_area,3),OSPF_NEWLINE);
    /*区域不会有type5lsa ，故应当一直为0*/
    ret = vty_out(vty, "%-28s:%d%s","Number of Ext LSA",(int)ospf_area_lsa_count(p_area,5),OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Number of NSSA LSA",(int)ospf_area_lsa_count(p_area,7),OSPF_NEWLINE);
        
    ret = vty_out(vty, "%-28s:%d%s","SPF Running Count",(int)p_area->spf_run,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","SPF Running Interval",OSPF_SPF_INTERVAL,OSPF_NEWLINE);
    
  
    memset(key_str, 0, 20);
    memcpy(key_str, p_area->key, 16);
    
    for (i=0; i < 20; i++)
    {
        if (0 == key_str[i])
        {
            key_str[i] = '\0';
            break;
        }
    }
    
    if (OSPF_AUTH_SIMPLE == p_area->authtype)
    {
        ret =   vty_out(vty, "%-28s:Simple Password%s","Authentication",OSPF_NEWLINE);
        ret =   vty_out(vty, "%-28s:%s%s","Simple Password",key_str,OSPF_NEWLINE);
    }
    else if (OSPF_AUTH_MD5 == p_area->authtype)
    {
        ret =   vty_out(vty, "%-28s:MD5 Digest%s","Authentication",OSPF_NEWLINE);
        ret =   vty_out(vty, "%-28s:%d%s","MD5 Key ID",p_area->keyid,OSPF_NEWLINE);
        ret =   vty_out(vty, "%-28s:%s%s","MD5 Key",key_str,OSPF_NEWLINE);
    }
    else
    {
        ret =   vty_out(vty, "%-28s:Not Used%s","Authentication",OSPF_NEWLINE);
    }

    ret = vty_out(vty, "%s SPFTree,spfid=%d, spfcost=%d%s",OSPF_NEWLINE, (int)ospf_lstcnt(&p_area->spf_table), 
          (int)ospf_lstcnt(&p_area->candidate_table),OSPF_NEWLINE);
    {
        struct ospf_spf_vertex *p_spf;
        struct ospf_spf_vertex *p_next_spf;
        u_int8 displaystr[20];          

        for_each_node(&p_area->spf_table, p_spf, p_next_spf)
        {
            ospf_inet_ntoa(displaystr,p_spf->id);
            ret += vty_out(vty, "%-28s:%s%s","id",displaystr,OSPF_NEWLINE); 
            ret += vty_out(vty, "%-28s:%d%s","type",(int)p_spf->type,OSPF_NEWLINE); 
            ret += vty_out(vty, "%-28s:%d%s","Cost",(unsigned int)p_spf->cost,OSPF_NEWLINE);  
            for (i = 0 ; i < OSPF_MAX_SPF_PARENT ; i ++)
            {
                if (p_spf->parent[i].p_node != NULL)
                {
                    ospf_inet_ntoa(displaystr,p_spf->parent[i].p_node->id);
                    ret += vty_out(vty, "%-28s:%s/%d%s","Parent",displaystr, 
                        (int)p_spf->parent[i].p_node->type,OSPF_NEWLINE);
                }
            }
            if (p_spf->p_nexthop)
            {
                u_int i = 0 ;

                for (i = 0 ; i < p_spf->p_nexthop->count ; i ++)
                {
                    if (0 == p_spf->p_nexthop->gateway[i].addr)
                    {
                        continue ;
                    }
                    ospf_inet_ntoa(displaystr,p_spf->p_nexthop->gateway[i].addr);
                    ret +=  vty_out(vty, "%-28s:%s/%d%s","Nexthop",displaystr, (int)p_spf->p_nexthop->gateway[i].if_uint,OSPF_NEWLINE);
                }
            }
            vty_out(vty, "%s",OSPF_NEWLINE);
        }
    }
    return;  
}

void 
ospf_display_area(struct vty *vty, u_int area_id, u_long ulProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    

    if (ospf_semtake_timeout() == ERR)

    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s", OSPF_NEWLINE, OSPF_NEWLINE);    
        return  ;                
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        if(ulProId && (p_process->process_id != ulProId))
        {
            continue;
        }
        ospf_display_area_x(vty, p_process, area_id);
    }
    
    ospf_semgive(); 
    return ;
}

void 
ospf_display_te_tlv_detailed (
                        u_int8 *p_buf, 
                        u_int  buflen)
{
    u_short tlv_type,tlv_len;
    u_int8 pad = 0;
    u_int8 flags[10] = {0,0,0,0,0,0,0,0,0,0};
    u_short i;
    
    /*top tlv length must include type  and length field*/
    if (buflen < 4)
    {
        return;
    }
    
    memcpy(&tlv_type, p_buf, 2);
    tlv_type = ntohs(tlv_type);
    p_buf += 2;        
    
    memcpy(&tlv_len, p_buf, 2);
    tlv_len = ntohs(tlv_len);
    p_buf += 2;        
    
    /*TE lsa can contain only one top tlv,and may be padding*/
    if ((tlv_len % 4) == 0)
    {
        if  (buflen != (tlv_len + 4))
              return;
    }
    else
    {
        pad = 4 - (tlv_len % 4);
        if (buflen != (tlv_len + pad + 4))
             return ;
    }
    
    /*check top tlv type*/
    /*router address :length must be 4*/
    if (tlv_type == OSPF_TE_TLV_RTR_ADDR)
    {
        if (tlv_len != 4)
             return ;
 
 	vty_out_to_all_terminal("  Router address TLV Information:"); 
 	vty_out_to_all_terminal("  Type :%d",tlv_type);
 	vty_out_to_all_terminal("  Length :%d",tlv_len); 
 	vty_out_to_all_terminal("  Router address :%d.%d.%d.%d",*p_buf,*(p_buf  +1),
            *(p_buf +2),*(p_buf +3)); 

        return  ;
    }/*ignore   unkown tlv*/
    else if (tlv_type != OSPF_TE_TLV_LINK)
    {
        return  ;
    }
    
     vty_out_to_all_terminal("  Link TLV Information:"); 
     vty_out_to_all_terminal("  Total Length :%d",tlv_len); 
     
    /*check each    sub-tlv for link top tlv*/
    buflen    = tlv_len ;
    
    while (buflen > 4)
    {
        pad = 0 ;/*first assume no padding*/
        
        /*get sub-tlv type and length*/
        memcpy(&tlv_type, p_buf, 2);
        tlv_type = ntohs(tlv_type);
        p_buf += 2;        
        
        memcpy(&tlv_len, p_buf, 2);
        tlv_len = ntohs(tlv_len);
        p_buf += 2;        
        
        /*same tlv must appear at most once*/
        if (tlv_type < 10)
        {
            if (flags[tlv_type]++ !=   0)
                return ;
        }
                          
        switch (tlv_type) {
        case OSPF_TE_SUB_LINK_TYPE  :

            /*length must be 1,and  must have 3 padding bytes*/
            if ((tlv_len != 1) || (buflen < 8))
                 return ;
 
            /*type must be 1 or 2*/
            if (((*p_buf) != 1) && ((*p_buf) != 2))
                  return ;
   
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Link Type");
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  value :(%d)%s",*p_buf,
            (*p_buf == 1)?"Point-to-point":"Multi-access");
            
             printf("%s",OSPF_NEWLINE);
            
            break;
        case OSPF_TE_SUB_LINK_ID :
        
            /*length must   be 4*/
            if ((tlv_len != 4) || (buflen < 8))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Link ID"); 
             vty_out_to_all_terminal( "  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :%d.%d.%d.%d",*p_buf,*(p_buf +1),
                *(p_buf +2),*(p_buf +3));
                
             printf( "%s",OSPF_NEWLINE);
            
            break ;
            
        case OSPF_TE_SUB_METRIC :
        
            /*length must be 4*/
            if  ((tlv_len != 4) || (buflen < 8))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Metric");
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :%d",(int)*(u_int*)p_buf); 
             printf("%s",OSPF_NEWLINE);
            
            break ;                 
            
        case OSPF_TE_SUB_MAX_BANDWIDTH  :
        
            /*length must be 4*/
            if  ((tlv_len != 4) || (buflen < 8))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Maximum Bandwidth"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :%d",(int)*(u_int*)p_buf);
             printf("%s",OSPF_NEWLINE);
            
            break ;         
            
        case OSPF_TE_SUB_MAX_RSVD_BANDWIDTH :  
        
            /*length must be 4*/
            if  ((tlv_len != 4) || (buflen < 8))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Maximum Reservable Bandwidth"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :%d",(int)*(u_int*)p_buf);
             printf("%s",OSPF_NEWLINE);
            
            break ;         
            
        case OSPF_TE_SUB_RESOURCE_CLASS_COLOR :      
                                        
            /*length must be 4*/
            if ((tlv_len != 4) || (buflen < 8))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Resource Class Color"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :%02x %02x %02x %02x",*p_buf,*(p_buf +1),
                *(p_buf +2),*(p_buf +3));
                
             printf("%s",OSPF_NEWLINE);
            
            break;
            
        case OSPF_TE_SUB_LOCAL_ADDRESS  :
        
            /*length must   be 4*N*/
            if ((tlv_len < 4) || ((tlv_len%4) != 0) || (buflen < (tlv_len + 4)))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Local Address"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value :");
            
            for (i    = 0 ; i < tlv_len/4; i ++)
            {
                 vty_out_to_all_terminal(" %d.%d.%d.%d ",*(p_buf + 4*i),*(p_buf   + 4*i+1),*(p_buf + 4*i+2),*(p_buf + 4*i+3));
            }
            
            printf("%s",OSPF_NEWLINE);
            printf( "%s",OSPF_NEWLINE);
            
            break;
            
        case OSPF_TE_SUB_REMOTE_ADDRESS :       
                                           
            /*length must be 4*N*/
            if ((tlv_len < 4) || ((tlv_len%4) != 0) || (buflen < (tlv_len + 4)))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Remote Address"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
            vty_out_to_all_terminal("  Value :");
            
            for (i = 0 ; i < tlv_len/4; i ++)
            {
                 vty_out_to_all_terminal(" %d.%d.%d.%d ",*(p_buf + 4*i),*(p_buf   + 4*i+1),*(p_buf + 4*i+2),*(p_buf + 4*i+3));
            }
            
             printf("%s",OSPF_NEWLINE);
                                                
            break;
            
        case OSPF_TE_SUB_UNRSVD_BANDWIDTH :
        
            /*length must be 32*/
            if ((tlv_len != 32) || (buflen < 36))
                return ;
            
             vty_out_to_all_terminal("  Sub Type :(%d)%s",tlv_type,"Unreserved bandwidth"); 
             vty_out_to_all_terminal("  Length :%d",tlv_len);
             vty_out_to_all_terminal("  Value : ");
            
            for (i = 0 ; i < 8 ; i ++)
            {
                 vty_out_to_all_terminal("%d ",(int)*(u_int*)(p_buf + 4*i));
            }
            
             printf("\n\r");  
            
            break;
            
        default :
            break;
       }
             
       if ((tlv_len % 4) != 0)
            pad   = 4 - (tlv_len % 4);
             
       p_buf += pad + tlv_len;
       buflen -= (pad + tlv_len + 4);
    }
      
   /*link    type and id must appear once*/         
    if ((flags[1] ==  0) || (flags[2] == 0))
          return ;
      
    return ;         
      
}

void 
ospf_display_lsax(
                  struct ospf_process *p_process,
                  int type, 
                  u_int areaid, 
                  u_int lsid, 
                  u_int advid, 
                  u_int addr)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_area  *p_area = NULL;
    u_int8   *p_lsbuf;
    u_int8   tmp_str[16];
    u_int8   lsidstr[64];
    u_int8   linkid[64];
    u_int8   linkdata[64];
    int lslen,rlen,linkcnt,linktype,toscnt;
    int link_index;
    u_int   lval;
    u_short   tlvtype,tlvlen,tlvpad;
    int    opalen;
    u_int   period;
    u_int8   *p_opabuf;
    u_int8   opaif[32];
    struct ospf_lstable *p_table = NULL;
    struct ospf_lshdr lshdr;
    
    p_area = ospf_area_lookup(p_process, areaid);
     #ifdef OSPF_DCN
    //p_table = ospf_lsa_scope_to_lstable(p_process, p_area, ospf_if_lookup_by_addr(p_process, addr), type);
    p_table = ospf_lsa_scope_to_lstable(p_process, p_area, ospf_if_lookup(p_process, addr), type);
     #else
    p_table = ospf_lsa_scope_to_lstable(p_process, p_area, ospf_if_lookup(p_process, addr), type);
     #endif
    lshdr.type = type;
    lshdr.id = htonl(lsid);
    lshdr.adv_id = htonl(advid);


    p_lsa = ospf_lsa_lookup(p_table, &lshdr);    
    if (NULL == p_lsa)
    {
        ospf_logx(ospf_debug,"%sNo Such LSA%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return ;
    }
    
     vty_out_to_all_terminal("%sDetailed LSA Information",OSPF_NEWLINE);

    if (p_area)
    {
         vty_out_to_all_terminal("%-28s:%u","Area  Id",(unsigned int)p_area->id);          
    }

     vty_out_to_all_terminal("%-28s:%d","Time Stamp",(int)p_lsa->rx_time);    
    
     vty_out_to_all_terminal("%-28s:%s","Expired",p_lsa->expired == TRUE ? "True" : "False");    
    
    p_lsbuf = (u_int8 *)p_lsa->lshdr;
    
     vty_out_to_all_terminal("%sDetailed LSA Body Information,ptr %08x",OSPF_NEWLINE,(unsigned int)p_lsbuf);
    
    /*If type is 10, only display the lsa header information */
    if (type == OSPF_LS_TYPE_10)
    {
        lslen = 20;
    }
    else
    {
        lslen = (*(u_short*)(p_lsbuf    + 18)) ;
    }
    
    for (link_index =   0 ; link_index < lslen; link_index ++)
    {
        if ((link_index%16) == 0)
             vty_out_to_all_terminal("%sBuffer:",OSPF_NEWLINE);                        

         vty_out_to_all_terminal(" %02x",p_lsbuf[link_index]);                               
    }
    
     printf("%s",OSPF_NEWLINE);
    
    switch(type) {
    case OSPF_LS_ROUTER :
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s", type,"Router Lsa");                             
    
        sprintf(lsidstr,"originating router's Router ID");                   
    
        break ;
        
    case OSPF_LS_NETWORK:
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"Network Lsa");                             
    
        sprintf(lsidstr,"Interface  address of DR");
    
        break ;
        
    case OSPF_LS_SUMMARY_NETWORK:
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"Network Summary Lsa");      
                               
        sprintf(lsidstr,"Destination network");
        
        break ; 
        
    case OSPF_LS_SUMMARY_ASBR:
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"ASBR  Summary Lsa");     
                               
        sprintf(lsidstr,"Router ID  of ASBR");
        
        break ;
        
    case OSPF_LS_AS_EXTERNAL:
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"AS-External Lsa"); 
        
        sprintf(lsidstr,"Destination network");
        
        break;
        
    case OSPF_LS_TYPE_7:
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"NSSA-type7-External Lsa"); 
        
        sprintf(lsidstr,"Destination network");
        
        break;
        
    /*opaque    lsa*/
    case OSPF_LS_TYPE_9 :
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"Link-scope Opaque LSA");  
        
        sprintf(lsidstr,"Opaque Type");
        
        break;
        
    case OSPF_LS_TYPE_10 :
    
         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"Area-scope Opaque LSA");  
        
        sprintf(lsidstr,"Opaque Type");
        
        if  (lsid == OSPF_GR_LSID)
            sprintf(lsidstr,"Graceful LSA");
        
        break;

    case OSPF_LS_TYPE_11 :

         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"AS-scope  Opaque LSA"); 

        sprintf(lsidstr,"Opaque Type");

        break;

    default :

         vty_out_to_all_terminal("  Lsa Type:(%d)%s",type,"Unkown Type Lsa");  
                                   
        sprintf(lsidstr,"Unkown");
        
        break ;
    }       
    
    /*lsid*/
    *(u_int*)tmp_str = (lsid)   ;
     vty_out_to_all_terminal("  Link State Id :%d.%d.%d.%d(%s)",tmp_str[0],tmp_str[1],tmp_str[2],tmp_str[3], lsidstr);
    
    /*adv   id*/
    *(u_int*)tmp_str = (advid) ;
     vty_out_to_all_terminal("  Advertising Router :%d.%d.%d.%d",tmp_str[0],tmp_str[1],tmp_str[2],tmp_str[3]);
    
     vty_out_to_all_terminal("  Lsa Age :%d",(*(u_short*)(p_lsbuf)));
    
     vty_out_to_all_terminal("  Options  :(0b 00(DC)(EA)(N/P)(MC)(E)0)%#x",*(p_lsbuf + 2));
    
     vty_out_to_all_terminal( "  Sequence Number :%#x",(unsigned int)(*(u_int*)(p_lsbuf + 12)));
    
     vty_out_to_all_terminal("  Checksum:%#x",(*(u_short*)(p_lsbuf + 16)));
    
     vty_out_to_all_terminal("  Length :%d",(*(u_short*)(p_lsbuf + 18)));
    
    lslen = (*(u_short*)(p_lsbuf + 18)) ;
    rlen = 20    ;
    switch(type) {
       case OSPF_LS_ROUTER:

            vty_out_to_all_terminal("  Flag:(0b:00000veb)%#x",*(p_lsbuf + rlen));
           rlen += 2 ;
           
           linkcnt = (*(u_short*)(p_lsbuf + rlen)) ;
            vty_out_to_all_terminal("  Link number:%d",linkcnt);
           rlen += 2 ;
           
           link_index = 1;
           while ((linkcnt > 0) && (rlen < lslen))
           {
                vty_out_to_all_terminal("  Link index:%d",link_index);
               
               linktype = *(p_lsbuf+rlen + 8) ;
               
               switch(linktype) {
               case OSPF_RTRLINK_PPP :

                   sprintf(linkid,"Neighbor Router ID");  
                      
                   sprintf(linkdata,"Local Interface Address");

                   break ;
                   
               case OSPF_RTRLINK_TRANSIT:
               
                   sprintf(linkid,"Interface address of DR");
                   
                   sprintf(linkdata,"Local Interface Address");
                   
                   break ;
                   
               case OSPF_RTRLINK_STUB:
               
                   sprintf(linkid,"Interface network");   
                              
                   sprintf(linkdata,"Local Interface Mask");
                   
                   break ;
                   
               case OSPF_RTRLINK_VLINK:
               
                   sprintf(linkid,"Neighbor Router ID");   
                                          
                   sprintf(linkdata,"Local Interface Address");
                   
                   break ;
                   
               default :
               
                   sprintf(linkid,"Unkown");
                   
                   break ;
              }
               
                vty_out_to_all_terminal("    Link id:%d.%d.%d.%d(%s)",*(p_lsbuf+rlen),
                   *(p_lsbuf+rlen+1),*(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3),linkid);
               rlen += 4 ;
               
                vty_out_to_all_terminal("    Link data:%d.%d.%d.%d(%s)",*(p_lsbuf+rlen),
                   *(p_lsbuf+rlen+1),*(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3),linkdata);
               rlen += 4 ;
               
               linktype    = *(p_lsbuf+rlen) ;
               switch(linktype){
               case OSPF_RTRLINK_PPP :
               
                    vty_out_to_all_terminal("    Link type:%d-%s",linktype,"Point to Point link");
                   
                   break ;
                   
               case OSPF_RTRLINK_TRANSIT:
               
                    vty_out_to_all_terminal("    Link type:%d-%s",linktype,"transit network link"); 
                                                         
                   break ;
                   
               case OSPF_RTRLINK_STUB:
               
                    vty_out_to_all_terminal("    Link type:%d-%s",linktype,"stub network link");   
                                                       
                   break ;
                   
               case OSPF_RTRLINK_VLINK:
               
                    vty_out_to_all_terminal("    Link type:%d-%s",linktype,"virtual link");  
                                                        
                   break ;
                   
               default  :
               
                    vty_out_to_all_terminal("    Link type:%d-%s",linktype,"unkown type link");   
                                                       
                   break ;
               }
               
                vty_out_to_all_terminal("  Link Tos Count :%d",*(p_lsbuf+rlen+1));                                       
               
                vty_out_to_all_terminal("  Link default Metric:%d",(*(u_short*)(p_lsbuf+rlen+2)));                                       
               
               toscnt =   *(p_lsbuf+rlen+1) ;
               
               rlen +=   4*(toscnt + 1) ;
               
               linkcnt-- ;                                             
               link_index++ ;
           }
           
           break ;
       case OSPF_LS_NETWORK:
            vty_out_to_all_terminal("  Network mask :%d.%d.%d.%d",*(p_lsbuf+rlen),*(p_lsbuf+rlen+1),
               *(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3));                                       
           
           rlen +=   4 ;
           
           while (rlen   < lslen)
           {
                vty_out_to_all_terminal("  Attach Router:%d.%d.%d.%d",*(p_lsbuf+rlen),*(p_lsbuf+rlen+1),
                   *(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3));                                      
               
               rlen += 4 ;
           }
           break ;
       case OSPF_LS_SUMMARY_NETWORK:
       case OSPF_LS_SUMMARY_ASBR:
            vty_out_to_all_terminal("  Network mask :%d.%d.%d.%d",*(p_lsbuf+rlen),*(p_lsbuf+rlen+1),
               *(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3));                                       
           
           rlen +=   4 ;
           
            vty_out_to_all_terminal("  Metric:%d",(int)(*(u_int*)(p_lsbuf+rlen)));                                         
           rlen +=   4 ;
           break ;
       case OSPF_LS_AS_EXTERNAL:
       case OSPF_LS_TYPE_7:      
            vty_out_to_all_terminal("  Network Mask:%d.%d.%d.%d",*(p_lsbuf+rlen),*(p_lsbuf+rlen+1),
               *(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3));
           
           rlen +=   4 ;
           
           lval = (*(u_int*)(p_lsbuf+rlen))   ;
           
            vty_out_to_all_terminal("  E-bit:%s",((lval&0x80000000)   != 0) ? "Set" : "Not set");
           
            vty_out_to_all_terminal("  Metric:%d",(int)(lval&0x00ffffff));
           
           rlen +=   4 ;
           
            vty_out_to_all_terminal("  Forwarding Address:%d.%d.%d.%d",*(p_lsbuf+rlen),*(p_lsbuf+rlen+1),
               *(p_lsbuf+rlen+2),*(p_lsbuf+rlen+3));
           
           rlen +=   4 ;
           
            vty_out_to_all_terminal("  External Route Tag:%#x",(unsigned int)(*(u_int*)(p_lsbuf+rlen)));
           
           rlen +=   4 ;                   
           break;
       case OSPF_LS_TYPE_9 :
           /*we only consider   graceful lsa*/
           if (lsid == OSPF_GR_LSID)
           {               
               if (p_table->p_if != NULL)
                   ospf_inet_ntoa(opaif, p_table->p_if->addr);
               else
                   opaif[0] = '\0';

                vty_out_to_all_terminal("  Opaque Interface:%s",opaif);                                
               
               /*display data*/
               p_opabuf = (u_int8 *)(p_lsa->lshdr+1);

               opalen = lslen - 20;
               
               for (tlvlen = 0 ; tlvlen < opalen; tlvlen++)
               {
                   if ((tlvlen%16) == 0)
                        vty_out_to_all_terminal("   \n\rOpaque Data %-2d:",tlvlen);

                    vty_out_to_all_terminal("%02x  ",p_opabuf[tlvlen]); 
               }
               
                printf("  %s%s",OSPF_NEWLINE,OSPF_NEWLINE);
               
               /*parse tlv*/
               while (opalen > 0)
               {
                   memcpy(&tlvtype,(p_opabuf), 2);
                   tlvtype = OSIX_NTOHS(tlvtype);
                   
                   memcpy(&tlvlen,(p_opabuf + 2), 2);
                   tlvlen =  OSIX_NTOHS(tlvlen);
                   
                    vty_out_to_all_terminal("   TLV Type:%d",tlvtype);
                    vty_out_to_all_terminal("   TLV Length:%d",tlvlen);
                   
                   if ((tlvlen % 4) == 0)
                       tlvpad    = tlvlen ;
                   else
                       tlvpad    = tlvlen + 4 - (tlvlen%4);
                   
                   switch (tlvtype){
                   case OSPF_GR_TLV_TYPE_TIME :

                       memcpy(&period,(p_opabuf + 4), 4);
                       period = OSIX_NTOHL(period);

                        vty_out_to_all_terminal("  Restart Period:%d",(int)period);

                       break;
                   case OSPF_GR_TLV_TYPE_REASON:

                        vty_out_to_all_terminal("  Restart Reason:%d",*(p_opabuf+4));

                       break;        
                   case OSPF_GR_TLV_TYPE_ADDR:

                        vty_out_to_all_terminal("  Interface Address:%d.%d.%d.%d",*(p_opabuf + 4),
                           *(p_opabuf + 5),*(p_opabuf + 6),
                           *(p_opabuf + 7));

                       break; 

                   default :
                       break;
                   }
                   
                   p_opabuf += (tlvpad + 4);
                   opalen -= (tlvpad + 4);
               }
           }
           
           break;
                                   
       case OSPF_LS_TYPE_10 :
           memcpy(&p_opabuf,(p_lsbuf+rlen), 4);     
                   
           for (link_index = 0 ; link_index < (lslen-rlen) ; link_index ++)
           {
               if ((link_index%16)  == 0)
                        vty_out_to_all_terminal("\n\r    TLV information:");

               vty_out_to_all_terminal("%02x ",p_opabuf[link_index]);
           }
            printf("\n\r ");
           
           ospf_display_te_tlv_detailed ( p_opabuf, lslen-rlen);
           break ;
       default :
            vty_out_to_all_terminal("  Lsa Type:(%d)%s", type,"Unkown Type Lsa");                            
           break ;
    }        
     vty_out_to_all_terminal("%-28s:%s","Graceful Restart Flag",p_lsa->self_rx_in_restart == TRUE ? "True" : "False");  
    return ;
}

void 
ospf_display_lsa_t(
                  int type, 
                  u_int areaid, 
                  u_int lsid, 
                  u_int advid, 
                  u_int addr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;


    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        ospf_display_lsax(p_process, type, areaid, lsid, advid, addr);
    }

    ospf_semgive();    

    return ;

}

void 
ospf_display_lsa_tablex(
							struct vty *vty,
                           struct ospf_process *p_process,
                           int type, 
                           u_int areaid, 
                           u_int display_num)
{
    u_int total_cnt = 0 ;
    u_int total_display_num = 0;
    u_int lsa_type =    0;
    u_int8 ret = 0;
    u_int8 areastr[20],linkStr[20],advRtrStr[20];
    u_short number_of_links;       
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsdb = NULL;
    

#define ls_display(p_lsdb) do\
    {\
      ospf_inet_ntoa(linkStr, (int)ntohl(p_lsdb->lshdr->id));\
      ospf_inet_ntoa(advRtrStr, (int)ntohl(p_lsdb->lshdr->adv_id));\
      ret = vty_out(vty, "%-17s %-17s %-8d 0x%-10x 0x%-10x %-10d%s",linkStr,advRtrStr,\
                            (int)(p_lsdb->lshdr->age + (ospf_sys_ticks() - p_lsa->update_time)/OSPF_TICK_PER_SECOND),\
                            (unsigned   int)ntohl(p_lsdb->lshdr->seqnum),\
                            (unsigned   int)ntohs(p_lsdb->lshdr->checksum),\
                            (unsigned   int)ntohs(p_lsdb->lshdr->len),\
                            OSPF_NEWLINE);\
   }while(0)

    /*summary display*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {                     
        if ((type == OSPF_LS_AS_EXTERNAL) || (type == OSPF_LS_TYPE_11))
              break;
                  
        if ((areaid != 0xffffffff) && (areaid != p_area->id))
              continue ;
        
        ospf_printf_area(areastr, p_area->id);
        
        for (lsa_type = OSPF_LS_ROUTER; lsa_type < OSPF_LS_TYPE_11; lsa_type++)
        {
            /*check type*/
            if  ((type != 0) && (type != lsa_type))
                 continue ;
            if(NULL == p_area->ls_table[lsa_type])
            {
                continue;
            }
            total_cnt = 0;                                            
            
            for_each_area_lsa(p_area, lsa_type, p_lsa, p_next_lsdb)
            {             
                if ( p_lsa->lshdr != NULL )
                {                                                       
                    total_cnt ++;
                    total_display_num++;
                    if (total_display_num> display_num)
                    {
                        goto end_of_display ;
                    }
                    switch (p_lsa->lshdr->type){
                    case OSPF_LS_ROUTER:
                        if (total_cnt == 1)        
                        {                               
                            ret =   vty_out(vty, "%s                         Router LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                            ret =   vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length","LinkCount",OSPF_NEWLINE);
                        }
                        
                        ospf_inet_ntoa(linkStr, ntohl(p_lsa->lshdr->id));
                        ospf_inet_ntoa(advRtrStr,p_lsa->lshdr->adv_id);
                        number_of_links = ((struct ospf_router_lsa *)p_lsa->lshdr)->link_count;
                        number_of_links = ntohs (number_of_links);
                         
                        ret = vty_out(vty, "%-17s %-17s %-8d 0x%-10x 0x%-10x %-10d%-8d%s",linkStr,advRtrStr,
                            (int)(ntohs(p_lsa->lshdr->age) + (ospf_sys_ticks() - p_lsa->update_time)/OSPF_TICK_PER_SECOND),
                            (unsigned   int)ntohl(p_lsa->lshdr->seqnum),
                            (unsigned   int)ntohs(p_lsa->lshdr->checksum),
                            (unsigned   int)ntohs(p_lsa->lshdr->len),
                            number_of_links,OSPF_NEWLINE);
                        
                        break;
                        
                    case OSPF_LS_NETWORK:
                        if (total_cnt == 1)        
                        {                                                                 
                            ret =   vty_out(vty, "%s                         Network LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                            ret =   vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                        }                                                               
                        
                        ls_display(p_lsa);
                        break;
                        
                    case OSPF_LS_SUMMARY_NETWORK:
                        if (total_cnt == 1)        
                        {                                                                 
                            ret =   vty_out(vty, "%s                         Summary Network LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                            ret =   vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                        }                                                                       
                        
                        ls_display(p_lsa);
                        break;
                    case OSPF_LS_SUMMARY_ASBR:
                        if (total_cnt == 1)        
                        {                                                                 
                            ret =   vty_out(vty, "%s                         Summary ASBR LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                            ret =   vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                        }                                                                       
                        
                        ls_display(p_lsa);
                        break;
                        
                    case OSPF_LS_TYPE_7 :
                        if (total_cnt == 1)        
                        {                                                                 
                            ret =   vty_out(vty, "%s                         NSSA Type7 LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                            ret =   vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                        }                                                                       
                        
                        ls_display(p_lsa);
                        break;

                    case OSPF_LS_TYPE_10:
                         if  (total_cnt == 1)        
                           {                                                                  
                                    ret = vty_out(vty, "%s                        Type 10 LSA (area %s)%s",OSPF_NEWLINE,areastr,OSPF_NEWLINE);
                                    ret = vty_out(vty, "%-17s    %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                            }                                                                        
                           ls_display(p_lsa);
                    default :
                        break;        
                    }
                }
                
            }      
            
        }
        
       }
    
        /*external  lsa*/
        if  ((type == OSPF_LS_AS_EXTERNAL) || ((type == 0) && (areaid == 0xffffffff)))
        {
            total_cnt = 0;
            
            for_each_external_lsa(p_process, p_lsa, p_next_lsdb) 
            {
                total_cnt++;

                total_display_num++;
                if (total_display_num> display_num)
                {
                     goto end_of_display ;
                }
                
                if (total_cnt   == 1)
                {
                    ret = vty_out(vty, "%s                         AS External LSA %s",OSPF_NEWLINE,OSPF_NEWLINE);
                    ret = vty_out(vty, "%-17s %-17s %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                }
                 ls_display(p_lsa);
            }                           
            
        }
        if ((type == OSPF_LS_TYPE_11)  || ((type == 0) && (areaid == 0xffffffff)))
        {
            total_cnt   = 0;
            
            for_each_t11_lsa(p_process, p_lsa, p_next_lsdb) 
            {
                total_cnt++;


                 total_display_num++;
                if (total_display_num> display_num)
                {
                     goto end_of_display ;
                }
                
                if (total_cnt ==    1)
                {
                    ret = vty_out(vty, "%s                          Type 11 LSA %s",OSPF_NEWLINE,OSPF_NEWLINE);
                    ret = vty_out(vty, "%-17s %-17s  %-8s %-10s %-10s %-10s%s","LinkId","ADV Router","Age","Seq#","CheckSum","Length",OSPF_NEWLINE);
                }
                
                ls_display(p_lsa);
            }
            
        }

#undef ls_display
  end_of_display : 
    
        return ;
}


void 
ospf_display_lsa_table(
							struct vty *vty, 
                           int type, 
                           u_int areaid, 
                           u_int display_num)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;


    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        vty_out(vty,"-----------------------process: %d-------------------%s",p_process->process_id,OSPF_NEWLINE);
        ospf_display_lsa_tablex(vty, p_process, type, areaid, display_num);
    }

    ospf_semgive();

    return ;
}

void 
ospf_display_lsa_cnt(struct vty *vty)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    struct ospf_if*p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    u_int lscount[12] = {0};
    u_int type5_cnt = 0 ;
    u_int type11_cnt    = 0 ;
    u_int total_cnt = 0 ;
    u_int display_flag =    0;
    u_int8 areastr[20],ifaddrStr[20];
    u_int i;

 

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }
    
    vty_out(vty," OSPF lsa database construction:%s%s",OSPF_NEWLINE,OSPF_NEWLINE);

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        vty_out(vty,"---------------process: %d-------------%s",p_process->process_id,OSPF_NEWLINE);
        vty_out(vty," %-8s%-7s%-8s%-8s%-9s%-6s%-8s%s",
            "Area","Router","Network","Sum-Net","Sum-ASBR","NSSA","Opaque9",OSPF_NEWLINE); 
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            /*init display infor*/
            memset(&lscount, 0, sizeof(lscount));
            ospf_printf_area(areastr, p_area->id);
            
            /*Each LSA*/
            for (i = OSPF_LS_ROUTER ; i <= OSPF_LS_TYPE_10 ; i++)
            {
               lscount[i] = ospf_area_lsa_count(p_area, i);
               total_cnt += lscount[i] ;
            }
            
            if (display_flag == 0)
            {
                display_flag ++;
        
            }
            
            vty_out(vty,"   %-8s%-7ld%-8ld%-8ld%-9ld%-6ld%-8ld%s",
                areastr, lscount[1], lscount[2],
                lscount[3], lscount[4], lscount[7],
                lscount[9],  OSPF_NEWLINE);                   
        }

       for_each_ospf_if(p_process, p_if, p_next_if)
       {         
           vty_out(vty," ifaddr %s, Number of type10 LSA:%ld%s", ospf_inet_ntoa(ifaddrStr, p_if->addr) ,ospf_lstcnt(&p_if->p_area->ls_table[OSPF_LS_TYPE_10]->list),OSPF_NEWLINE);
       }
        
        /*external*/     
        type5_cnt = ospf_lstcnt(&p_process->t5_lstable.list);
        total_cnt += type5_cnt ;
        
        type11_cnt = ospf_lstcnt(&p_process->t11_lstable.list);
        total_cnt += type11_cnt ;
        
        vty_out(vty," Number of AS  External LSA:%ld%s",type5_cnt,OSPF_NEWLINE);                
        vty_out(vty," Number of Type11  Opaque LSA:%ld%s",type11_cnt,OSPF_NEWLINE);
        vty_out(vty," Number of Total LSA:%ld%s",total_cnt,OSPF_NEWLINE); 
    }

    ospf_semgive();

    return ;
}
void ospf_display_vif(struct vty *vty)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
        struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ; 
    
  if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return  ;      
    }

    for_each_ospf_process(p_process, p_next_process)
    for_each_ospf_if(p_process, p_if, p_next_if)
     {
         if(p_if->type != OSPF_IFT_VLINK)
         {
             continue;
         }
         vty_out(vty,"if addr%x, state %d, transit area %d, nbr %x%s",(int)p_if->addr, p_if->state, (int)p_if->p_transit_area->id, (int)p_if->nbr,OSPF_NEWLINE);
     }
	
    ospf_semgive();
    return ;
}  
void 
ospf_display_interface(struct vty *vty, int if_addr)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ;    
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL ;    
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    u_int i ; 
    u_int first = 1;
    u_int8 addr[32];
    u_int8 ifaddrstr[20];
    u_int8 ifareastr[20];
    u_int rx_count = 0;
    u_int tx_count = 0;
	int ret = 0;
	
    u_int8 lsa_type[OSPF_PACKET_ACK+1][15]={"","HELLO","DD","REQUEST","UPDATE","ACK"};
	if(NULL == vty)
	{
		return;
	}
    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return  ;                
    }
    vty_out(vty, "%s",OSPF_NEWLINE);
    /*null interface means all*/
    if (0 == if_addr)
    {
        for_each_ospf_process(p_process, p_next_process)
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
             if (first)
             {
                 first = 0 ;
                 ret = vty_out(vty, " %s%-20s%-20s%s%s",OSPF_NEWLINE,"IfAddress","IfArea","State",OSPF_NEWLINE);    
             }

             ospf_inet_ntoa(ifaddrstr, p_if->addr);

             if (p_if->p_area)
                  ospf_printf_area(ifareastr, p_if->p_area->id);
             ret = vty_out(vty, " %-20s%-20s%s%s",ifaddrstr,ifareastr,
             (p_if->state == 1) ? "down" :
             (p_if->state == 2)    ? "loopback" :
             (p_if->state == 3)    ? "waiting" : 
             (p_if->state == 4)    ? "pointToPoint" : 
             (p_if->state == 5)    ? "DesignatedRouter" : 
             (p_if->state == 6)    ? "backupDesignatedRouter" : 
             (p_if->state == 7)    ?        "designatedRouterOther":"erorr state" ,OSPF_NEWLINE);                 
        }

         ospf_semgive(); 
         return ;
    }
    for_each_ospf_process(p_process, p_next_process)
    {
     #ifdef OSPF_DCN
       //p_if = ospf_if_lookup_by_addr(p_process, if_addr);
	   p_if = ospf_if_lookup(p_process, if_addr);
     #else
       p_if = ospf_if_lookup(p_process, if_addr);
     #endif
	 
		//printf("%s %d, p_if = %p\n", __FUNCTION__,__LINE__, p_if);
		if (p_if != NULL)
		{
		//	printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint);
		}
       if (p_if)
       {
          break;
       }
    }
    if (NULL == p_if)
    {
        ospf_semgive();  

        vty_out(vty," No such ospf  interface%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        
        return  ;     
    }
    
    ret = vty_out(vty, "%-28s:%#x%s","Interface Pointer",p_if,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%#x%s","Ifnet Index",p_if->ifnet_index,OSPF_NEWLINE);    
    ret = vty_out(vty, "%-28s:%#x%s","Ifnet Unit",p_if->ifnet_uint,OSPF_NEWLINE);  
    ret = vty_out(vty, "%-28s:%d%s","link up",p_if->link_up,OSPF_NEWLINE);

    if (p_if->p_area)
        ret = vty_out(vty, "%-28s:%d%s","Area Id",(int)p_if->p_area->id,OSPF_NEWLINE);
        
    ret = vty_out(vty, "%-28s:%d-%s%s","Type",p_if->type,(p_if->type == 1)? "broadcast" :
    (p_if->type == 2)?    "NBMA" :
    (p_if->type == 3)?    "pointToPoint" :
    (p_if->type == 4)?    "Virtual" : "pointToMultipoint",OSPF_NEWLINE);
    
    ret = vty_out(vty, "%-28s:%s%s","State", (p_if->state == 1) ? "down" :
    (p_if->state == 2)    ? "loopback" :
    (p_if->state == 3)    ? "waiting" : 
    (p_if->state == 4)    ? "pointToPoint" : 
    (p_if->state == 5)    ? "DesignatedRouter" : 
    (p_if->state == 6)    ? "backupDesignatedRouter" : 
    (p_if->state == 7)    ?        "designatedRouterOther":"erorr state",OSPF_NEWLINE);
    
    ret = vty_out(vty, "%-28s:%d%s","Event Change",(int)p_if->stat->state_change,OSPF_NEWLINE);
    ospf_inet_ntoa(addr,p_if->addr);
    ret = vty_out(vty, "%-28s:%s%s","Address",addr,OSPF_NEWLINE);
    ospf_inet_ntoa(addr,p_if->mask);
    ret = vty_out(vty, "%-28s:%s%s","Mask",addr,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","MTU",p_if->mtu,OSPF_NEWLINE);                 
    ret = vty_out(vty, "%-28s:%#x%s","Area  p_area",p_if->p_area,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Dead Interval",(int)p_if->dead_interval,OSPF_NEWLINE);                 
    ret = vty_out(vty, "%-28s:%d%s","Transmit Delay",p_if->tx_delay,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Priority",p_if->priority,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Cost",p_if->cost[0],OSPF_NEWLINE);
    
    ret = vty_out(vty, "%-28s:%s%s","Passive Mode",
        (p_if->passive == TRUE) ? "True"    : "False" ,OSPF_NEWLINE);                                  
    
    ret = vty_out(vty, "%-28s:%d(%s)%s","Cost Type",p_if->configcost,
        (p_if->configcost == 0)    ? "Auto" : "Manual",OSPF_NEWLINE);
    
    ret = vty_out(vty, "%-28s:%d%s","Hello Interval",(int)p_if->hello_interval,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Poll Interval",(int)p_if->poll_interval,OSPF_NEWLINE);
    
    
    ret = vty_out(vty, "%-28s:%d%s","Retransmit    Interval",p_if->rxmt_interval,OSPF_NEWLINE);
    
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr) 
    {
        ospf_inet_ntoa(addr,p_nbr->addr);
        ret = vty_out(vty, "%-28s:%s%s","Neighbor",addr,OSPF_NEWLINE);
    }
    
    ospf_inet_ntoa(addr,p_if->dr);
    ret = vty_out(vty, "%-28s:%s%s","DR",addr,OSPF_NEWLINE);
    
    ospf_inet_ntoa(addr,p_if->bdr);
    ret = vty_out(vty, "%-28s:%s%s","BDR",addr,OSPF_NEWLINE);
    
    /*Authentication*/
    {
        ret = vty_out(vty, "%-28s:%d(%s)%s","Auth Type",p_if->authtype,
        p_if->authtype == 0 ? "None" :
        p_if->authtype == 1 ? "Simple Password" :
        p_if->authtype == 2 ? "MD5 Key" : "Unknown",OSPF_NEWLINE);
        
        if (p_if->authtype == 1)
        {
            ret = vty_out(vty, "%-28s:%s%s","Password",p_if->key,OSPF_NEWLINE);   
        }
        else if (p_if->authtype == 2)
        {
            ret = vty_out(vty, "%-28s:%d%s","MD5 Key ID",(int)p_if->md5id,OSPF_NEWLINE);
            ret = vty_out(vty, "%-28s:%s%s","MD5 Key ",p_if->key,OSPF_NEWLINE);
        }
    }
    
    ret = vty_out(vty, "%-28s%s","Statistics of    this interface",OSPF_NEWLINE);

    rx_count = 0;
    tx_count = 0;
    /*
    for (i = 1 ; i < OSPF_PACKET_ACK+1; i++)
    {
       vty_out(vty,"\r\n");
       rx_count += p_if->stat->rx_packet[i];
       tx_count += p_if->stat->tx_packet[i];
       vty_out(vty,"%-28s-%d:%d%s","Received packets", i, (int)p_if->stat->rx_packet[i],OSPF_NEWLINE);
       vty_out(vty,"%-28s-%d:%d%s","Send packets", i, (int)p_if->stat->tx_packet[i],OSPF_NEWLINE);
       vty_out(vty,"%-28s-%d:%d%s","Received bytes", i, (int)p_if->stat->rx_byte[i],OSPF_NEWLINE);
       vty_out(vty,"%-28s-%d:%d%s","Send bytes", i, (int)p_if->stat->tx_byte[i],OSPF_NEWLINE);
       vty_out(vty,"%-28s-%d:%d%s","Received Lsainfo", i, (int)p_if->stat->rx_lsainfo[i],OSPF_NEWLINE);
       vty_out(vty,"%-28s-%d:%d%s","Send Lsainfo", i, (int)p_if->stat->tx_lsainfo[i],OSPF_NEWLINE);
    }*/
    ret = vty_out(vty, "          %-10s%-10s%-12s%-12s%-10s%-10s%s","Rcv pkt","Send pkt", "Rcv bytes","Send bytes","Rcv Lsa","Send Lsa",OSPF_NEWLINE);
    for (i = 1 ; i < OSPF_PACKET_ACK+1; i++)
    {
       vty_out(vty,"%-10s%-10d%-10d%-12d%-12d%-10d%-10d%s",lsa_type[i],
           (int)p_if->stat->rx_packet[i], (int)p_if->stat->tx_packet[i],(int)p_if->stat->rx_byte[i],
           (int)p_if->stat->tx_byte[i],(int)p_if->stat->rx_lsainfo[i],(int)p_if->stat->tx_lsainfo[i],OSPF_NEWLINE);
       rx_count += p_if->stat->rx_packet[i];
       tx_count += p_if->stat->tx_packet[i];
    }
    ret = vty_out(vty, "%s",OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","rcv packet total count",(int)rx_count,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","send packet total count",(int)tx_count,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Rxd Unicast",(int)p_if->stat->rx_unicast_pkt,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Rxd Multicast",(int)p_if->stat->rx_muti_pkt,OSPF_NEWLINE);    
    ret = vty_out(vty, "%-28s:%d%s","Txd Unicast",(int)p_if->stat->tx_unicast_pkt,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Txd Multicast",(int)p_if->stat->tx_muti_pkt,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Init DD rxmt count",(int)p_if->stat->init_dd_rxmt,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","DD rxmt count",(int)p_if->stat->dd_rxmt,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","muticast update count",(int)p_if->stat->mcast_update,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","unicast update count",(int)p_if->stat->ucast_update,OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","request rxmt count",(int)p_if->stat->req_rxmt,OSPF_NEWLINE);

    ret = vty_out(vty, "%-28s:%d%s","Always    send mcast",p_if->mcast_always,OSPF_NEWLINE);                   
    ret = vty_out(vty, "%-28s:%s%s","Unicast hello",
        (p_if->unicast_hello == 1 )? "True" : "False",OSPF_NEWLINE);
                                      
    
    ret = vty_out(vty, "%-28s:%d%s","Packet    from invalid source",(int)p_if->stat->error[OSPF_IFERR_SOURCE],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Packet    from unkown neighbor",(int)p_if->stat->error[OSPF_IFERR_NONBR],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d(last %d)%s","Packet with error auth type",(int)p_if->stat->error[OSPF_IFERR_AUTHTYPE],(int)p_if->stat->error_data[OSPF_IFERR_AUTHTYPE],OSPF_NEWLINE);            
    ret = vty_out(vty, "%-28s:%d%s","Packet    with error auth info",(int)p_if->stat->error[OSPF_IFERR_AUTH],OSPF_NEWLINE);            
    ret = vty_out(vty, "%-28s:%d(last %x)%s","Bad network mask in Hello",(int)p_if->stat->error[OSPF_IFERR_MASK],(unsigned int)p_if->stat->error_data[OSPF_IFERR_MASK],OSPF_NEWLINE);               
    
    ret = vty_out(vty, "%-28s:%d(last %d)%s","Bad hello interval in hello",(int)p_if->stat->error[OSPF_IFERR_HELLOTIME],(int)p_if->stat->error_data[OSPF_IFERR_HELLOTIME],OSPF_NEWLINE);                      
    ret = vty_out(vty, "%-28s:%d(last %d)%s","Bad dead interval in hello",(int)p_if->stat->error[OSPF_IFERR_DEADTIME],(int)p_if->stat->error_data[OSPF_IFERR_DEADTIME],OSPF_NEWLINE);                      
    ret = vty_out(vty, "%-28s:%d%s","Bad External bit in hello",(int)p_if->stat->error[OSPF_IFERR_EBIT],OSPF_NEWLINE);                         
    ret = vty_out(vty, "%-28s:%d%s","Bad NSSA bit in hello",(int)p_if->stat->error[OSPF_IFERR_NBIT],OSPF_NEWLINE);                         
    ret = vty_out(vty, "%-28s:%d%s","Neighbor negodone",(int)p_if->stat->nbr_event[OSPF_NE_NEGDONE],OSPF_NEWLINE);                               
    ret = vty_out(vty, "%-28s:%d%s","Neighbor exchang done",(int)p_if->stat->nbr_event[OSPF_NE_EXCHANGE_DONE],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Neighbor bad lsa request",(int)p_if->stat->nbr_event[OSPF_NE_BAD_REQUEST],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Neighbor loading done",(int)p_if->stat->nbr_event[OSPF_NE_LOADDONE],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Neighbor sequence mismatch",(int)p_if->stat->nbr_event[OSPF_NE_SEQ_MISMATCH],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Neighbor shutdown or timeout",(int)(p_if->stat->nbr_event[OSPF_NE_KILL]+p_if->stat->nbr_event[OSPF_NE_INACTIVITY_TIMER]),OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Interface up times",(int)p_if->stat->if_event[OSPF_IFE_UP],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Interface down times",(int)p_if->stat->if_event[OSPF_IFE_DOWN],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d%s","Interface neighbor changed",(int)p_if->stat->if_event[OSPF_IFE_NBR_CHANGE],OSPF_NEWLINE);
    ret = vty_out(vty, "%-28s:%d(last %d)%s","Mismatch MTU in DD packet",(int)p_if->stat->error[OSPF_IFERR_MTU],(int)p_if->stat->error_data[OSPF_IFERR_MTU],OSPF_NEWLINE);

    vty_out(vty, "%s",OSPF_NEWLINE);
    ospf_semgive(); 
    return ;
}

void 
ospf_display_one_route(
			struct vty *vty,
            struct ospf_route *p_rt, 
            u_int current)
{
    u_int8 dest[16];
    u_int8 mask[16];
    u_int8 nhop[16];
    u_int8 type[16];
    u_int8 path_type[16];

    ospf_inet_ntoa(dest, p_rt->dest);
    ospf_inet_ntoa(mask, p_rt->mask);
       
    ospf_inet_ntoa(nhop, p_rt->path[current].p_nexthop ? p_rt->path[current].p_nexthop->gateway[0].addr : 0);
       
        switch (p_rt->type){
            
        case OSPF_ROUTE_NETWORK :
            sprintf(type,"Network");
            break;
        case OSPF_ROUTE_ASBR :
            sprintf(type,"ASBR");
            break;
        case OSPF_ROUTE_ABR :
            sprintf(type,"ABR");
            break;       
        default :    
            sprintf(type,"Unkown");
            break;           
        }  
        switch (p_rt->path[current].type){
            
        case OSPF_PATH_INTRA:
            sprintf(path_type,"INTRA");
            break;
        case OSPF_PATH_INTER:
            sprintf(path_type,"INTER");
            break;
        case OSPF_PATH_ASE:
            sprintf(path_type,"ASE");
            break;       
        case OSPF_PATH_ASE2:
            sprintf(path_type,"ASE2");
            break;
        default :    
            sprintf(path_type,"Unkown");
            break;           
        }  
        vty_out(vty, "%-18s%-18s%-18s%-6d%-6d%s(%s)%s",dest,mask,nhop
            ,(int)p_rt->path[current].cost,(int)p_rt->path[current].cost2,type,path_type,OSPF_NEWLINE);  
      
}

void 
ospf_display_route_summaryx(
			  struct vty *vty, 
              struct ospf_process *p_process, 
              u_int display_num)
{
    struct ospf_route *p_rt = NULL;
    struct ospf_route *p_next_rt = NULL;
    u_int total = 0 ;
    u_int8 ret = 0;
    u_int abr_cnt = 0 ;
    u_int asbr_cnt =    0 ;
    u_int network_cnt = 0 ;
    u_int intra_network_cnt = 0 ;
    u_int inter_network_cnt = 0 ;
    u_int external_cnt =    0 ;
    u_int virtual_cnt = 0 ;
    u_int wildcard_cnt =    0 ;
    u_int total_cnt = 0 ;
    u_int current = p_process->current_route;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    
    
    for_each_node(&p_process->route_table, p_rt, p_next_rt)
    {        
        switch(p_rt->type)
        {
            case OSPF_ROUTE_NETWORK :
            {
                network_cnt++;  
                if( p_rt->path[current].type == OSPF_PATH_INTRA)
                    intra_network_cnt++;
                else if( p_rt->path[current].type == OSPF_PATH_INTER)
                    inter_network_cnt++;
                else
                    external_cnt++;
                break;
            }
            default :    
                wildcard_cnt++;
                break;           
        }
        total++;
    }
	for_each_ospf_area(p_process, p_area, p_next_area)
	{
	    asbr_cnt += ospf_lstcnt(&p_area->asbr_table);
	    abr_cnt += ospf_lstcnt(&p_area->abr_table);
	}
	vty_out(vty,"%s OSPF Process %d",OSPF_NEWLINE,p_process->process_id);
    ret = vty_out(vty, "%s Total %ld routes,Max SPF running time:%ld00ms%s",
        OSPF_NEWLINE,total,p_process->max_spf_time,OSPF_NEWLINE);
    ret = vty_out(vty, " ABR:%ld,ASBR:%ld,Network:%ld(intra=%ld,inter=%ld),External:%ld,Virtual:%ld%s",
        abr_cnt,asbr_cnt,network_cnt,intra_network_cnt,inter_network_cnt,external_cnt,virtual_cnt,OSPF_NEWLINE);  
    
   
    
    ret = vty_out(vty, "%-18s%-18s%-18s%-6s%-6s%s%s","Dest","Mask","Nexthop","Cost","Cost2","Type",OSPF_NEWLINE);
    
 
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        for_each_node(&p_area->asbr_table, p_rt, p_next_rt)
        {
            ospf_display_one_route(vty, p_rt, current);
            total_cnt ++ ;
            if (total_cnt>display_num)
            {
                 break;
            }
        }      
    }
     for_each_ospf_area(p_process, p_area, p_next_area)
    {
        for_each_node(&p_area->abr_table, p_rt, p_next_rt)
        {
             ospf_display_one_route(vty, p_rt, current);
             total_cnt ++ ;
             if (total_cnt>display_num)
             {
                 break;
             }
        }  
    }
    for_each_node(&p_process->route_table, p_rt, p_next_rt)
    {
        ospf_display_one_route(vty, p_rt, current);
        total_cnt ++ ;
         if (total_cnt>display_num)
         {
             break;
         }
    }       
    return ; 
}


void 
ospf_display_route_summary(struct vty *vty, u_long ulInstance, u_int display_num)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;


    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        if((ulInstance != 0) && (p_process->process_id != ulInstance))
        {
            continue;
        }
        ospf_display_route_summaryx(vty, p_process, display_num);
    }

    ospf_semgive();   

    return ;
}

void 
ospf_display_one_route_detailed(
				struct vty *vty, 
                 struct ospf_route *p_rt, 
                 u_int current)
{
    u_int8 dest[16];
    u_int8 mask[16];
    u_int8 nhop[16];
    u_int8 type[16];
    
        ospf_inet_ntoa(dest, p_rt->dest);
        ospf_inet_ntoa(mask, p_rt->mask);

        ospf_inet_ntoa(nhop,p_rt->path[current].p_nexthop ? p_rt->path[current].p_nexthop->gateway[0].addr : 0);
        switch (p_rt->type){
            
        case OSPF_ROUTE_NETWORK :
            sprintf(type,"Network");
            break;
        case OSPF_ROUTE_ASBR :
            sprintf(type,"ASBR");
            break;
        case OSPF_ROUTE_ABR :
            sprintf(type,"ABR");
            break;
        default :    
            sprintf(type,"Unkown");
            break;          
        }   
        
        vty_out(vty, "%-28s:%s%s","Route Dest",dest,OSPF_NEWLINE);         
        vty_out(vty, "%-28s:%s%s","Route Mask",mask,OSPF_NEWLINE);                    
        vty_out(vty, "%-28s:%s%s","Route Type",type,OSPF_NEWLINE);          
        
        if (p_rt->path[current].p_area == NULL)                  
            vty_out(vty, "%-28s:%s%s","Associated Area","NULL",OSPF_NEWLINE);                       
        else
            vty_out(vty, "%-28s:%d%s","Associated Area",(int)p_rt->path[current].p_area->id,OSPF_NEWLINE);         
        
        vty_out(vty, "%-28s:%s%s","Path Type",
            p_rt->path[current].type == OSPF_PATH_INTRA ? "IntraArea" :
        p_rt->path[current].type == OSPF_PATH_INTER ? "InterArea" :
        p_rt->path[current].type == OSPF_PATH_ASE ? "Type1 External" : "Type2 External",OSPF_NEWLINE);  
        
        vty_out(vty, "%-28s:%d%s","Path Cost",(int)p_rt->path[current].cost,OSPF_NEWLINE); 
        vty_out(vty, "%-28s:%d%s","Path Type2 Cost",(int)p_rt->path[current].cost2,OSPF_NEWLINE); 
        if (p_rt->path[current].p_nexthop)
         {
             u_int nidx = 0 ;

             for (nidx = 0 ; nidx < p_rt->path[current].p_nexthop->count; nidx ++)
             {         
                 if (p_rt->path[current].p_nexthop->gateway[nidx].addr)
                 {
                    ospf_inet_ntoa(dest, p_rt->path[current].p_nexthop->gateway[nidx].addr);
                    vty_out(vty, "%-28s:%s%s","Nexthop Address",dest,OSPF_NEWLINE);         
                }
             }
         }     
}

void 
ospf_display_route_detailedx(
			struct vty *vty,
           struct ospf_process *p_process,
           u_int route_dest, 
           u_int route_mask)
{
    struct ospf_route *p_rt = NULL;
    struct ospf_route *p_next_rt = NULL;    
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    u_int current = p_process->current_route;
    
    for_each_node(&p_process->route_table, p_rt, p_next_rt)
    {
        if ((p_rt->dest != route_dest) || 
            (p_rt->mask != route_mask))
        {
            continue ;
        }
       ospf_display_one_route_detailed(vty, p_rt, current);
    }
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        for_each_node(&p_area->abr_table, p_rt, p_next_rt)
        {
           if ((p_rt->dest != route_dest) || 
            (p_rt->mask != route_mask))
            {
                continue ;
            }
           ospf_display_one_route_detailed(vty, p_rt, current);
        }  
    }
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        for_each_node(&p_area->asbr_table, p_rt, p_next_rt)
        {
            if ((p_rt->dest != route_dest) || 
            (p_rt->mask != route_mask))
            {
                continue ;
            }
           ospf_display_one_route_detailed(vty, p_rt, current);
        }      
    }    
    return ;
}

void 
ospf_display_route_detailed(
				struct vty *vty, 
                 u_int route_dest, 
                 u_int route_mask)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;


    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        ospf_display_route_detailedx(vty, p_process, route_dest, route_mask);
    }

    ospf_semgive(); 

    return ;

}

void 
ospf_display_neighbor(struct vty *vty, u_int peer)
 {
     struct ospf_if *p_if = NULL;
     struct ospf_nbr *p_nbr = NULL;
     struct ospf_nbr *p_next_nbr = NULL ;     
     struct ospf_process *p_process = NULL;
     struct ospf_process *p_next_process = NULL;
     u_int8 addr[16];
     u_int8 *peer_state[9] = {" ","Down","Attempt","Init","2-way","ExStart","Exchange","Loading","Full"};
     int i = 0; 
         u_char *if_typestr[] = {" ", 
                       "Broadcast",
                       "NBMA ",
                       "P2p", 
                       "Vlink",
                       "P2mp",
                       "Shamlink",
                       "Unkown"};
     
     #define TMRSTR(x, str) ospf_logx(ospf_debug,"\n\r%s Timer:%d/%d",str, (x)->enable, (x)->count);
    

     if (ospf_semtake_timeout() == ERR)
     {
         vty_out(vty," OSPF    is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
         return    ;                
     }

	 vty_out(vty, "%s",OSPF_NEWLINE);
     for_each_ospf_process(p_process, p_next_process)
      for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr) 
     {
         if (peer && p_nbr->addr != peer)
         {
             continue ;
         }

         /*Peer*/
         vty_out(vty, "%sDetailed   Information of Neighbor%s",OSPF_NEWLINE,OSPF_NEWLINE);
         ospf_inet_ntoa(addr,p_nbr->p_if->addr);
         vty_out(vty, "%-28s:%s%s","Local   Interface Address",addr,OSPF_NEWLINE);
	     vty_out(vty, "%-28s:%s%s","Interface type",if_typestr[p_nbr->p_if->type],OSPF_NEWLINE);
         ospf_inet_ntoa(addr,p_nbr->addr);
         vty_out(vty, "%-28s:%s%s","Neighbor Address",addr,OSPF_NEWLINE);
         
         ospf_inet_ntoa(addr,p_nbr->id);
         vty_out(vty, "%-28s:%s%s","Neighbor Id",addr,OSPF_NEWLINE);
         
         vty_out(vty, "%-28s:%s%s","Neighbor State",peer_state[p_nbr->state],OSPF_NEWLINE); 
         vty_out(vty, "%-28s:%s%s","In Restarting",p_nbr->in_restart   == TRUE ? "True" : "False",OSPF_NEWLINE); 
         
         vty_out(vty, "%-28s:%d%s","Neighbor Priority",p_nbr->priority,OSPF_NEWLINE); 
         ospf_inet_ntoa(addr,p_nbr->dr);
         vty_out(vty, "%-28s:%s%s","Neighbor's DR",addr,OSPF_NEWLINE);
         ospf_inet_ntoa(addr,p_nbr->bdr);
         vty_out(vty, "%-28s:%s%s","Neighbor's BDR",addr,OSPF_NEWLINE);

         vty_out(vty, "%-28s:%d%s","time in state FULL",(int)p_nbr->full_time,OSPF_NEWLINE);   
         vty_out(vty, "%-28s:%d%s","Neighbor Event Count",p_nbr->events,OSPF_NEWLINE);    
         vty_out(vty, "%-28s:%d%s","Neighbor Mode",p_nbr->dd_state,OSPF_NEWLINE); 
         vty_out(vty, "%-28s:%08x%s","Neighbor Flag",p_nbr->dd_flag ,OSPF_NEWLINE);   
         vty_out(vty, "%-28s:%08x%s","Neighbor Option",p_nbr->option ,OSPF_NEWLINE);    
         vty_out(vty, "%-28s:%d%s","DD Sequence Number",(int)p_nbr->dd_seqnum,OSPF_NEWLINE);                       
         vty_out(vty, "%-28s:%d%s","Retransmit Queue Count",(int)p_nbr->rxmt_count,OSPF_NEWLINE);
         vty_out(vty, "%-28s:%d%s","Request Queue   Count",(int)p_nbr->req_count,OSPF_NEWLINE);                   

        
        if (ospf_lstfirst(&p_process->req_table)) 
        {
                 struct ospf_request_node *p_lrq = NULL;
                 struct ospf_request_node *p_next_lrq = NULL;
                 u_int8 lsid[16],advid[16];
                 vty_out(vty, "%sRequest Information of Neighbor,LSA Type   %d%s",OSPF_NEWLINE,i,OSPF_NEWLINE);                                  
                 vty_out(vty, "%-16s%-16s%-10s%-10s%s%s","LsId","AdvId","Sequence","Checksum","Age",OSPF_NEWLINE);   

                for_each_node(&p_process->req_table, p_lrq, p_next_lrq)
                 {
                     if (p_lrq->p_nbr != p_nbr)
                        continue ;
                     ospf_inet_ntoa(lsid,p_lrq->ls_hdr.id);
                     ospf_inet_ntoa(advid,p_lrq->ls_hdr.adv_id);
                     vty_out(vty, "%-16s%-16s%-8x  %-8x  %-4d%s",lsid,advid,
                         (unsigned int)p_lrq->ls_hdr.seqnum,p_lrq->ls_hdr.checksum,p_lrq->ls_hdr.age,OSPF_NEWLINE); 
                 }
        }
     }                   
     
     ospf_semgive();   
     return ;
}

struct ospf_process *ospf_prcess_search(u_int uiProcess)
{
    struct ospf_process *p_process  = NULL;

    p_process = ospf_process_lookup(&ospf, uiProcess);
	return p_process;
}

void 
ospf_display_gr(struct vty *vty,u_int uiProcess)
{    
    struct ospf_process *p_process  = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    char addrstr[32] = {0};
    u_int8 str[OSPF_RESTART_TOPLOGY_CHANGED+1][20] = {" ","none","inprogress","complete","timeout", "topo changed"};

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug," OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                
    }
    p_process = ospf_prcess_search(uiProcess);
    if (NULL == p_process)
	{
        return ;                
	}

	ospf_inet_ntoa(addrstr,p_process->router_id);
	vty_out(vty,"          OSPF Process %d with Router ID %s %s",p_process->process_id,addrstr,OSPF_NEWLINE);

	vty_out(vty,"%-40s:%s%s","Graceful-restart capability",p_process->restart_enable == TRUE ?  "enabled" : "disable",OSPF_NEWLINE);
	vty_out(vty,"%-40s:%s%s","Graceful-restart helper capability",p_process->restart_helper == TRUE ? "Support" : "Not Support",OSPF_NEWLINE);
	vty_out(vty,"%-40s:%s%s","Current GR state",p_process->in_restart  == TRUE ? "Under GR" : "Normal",OSPF_NEWLINE);
	vty_out(vty,"%-40s:%d seconds%s","Graceful-restart period",p_process->restart_period,OSPF_NEWLINE);          

	vty_out(vty,"Last exit reason:%s",OSPF_NEWLINE); 
	vty_out(vty,"  On graceful restart:%s%s",str[p_process->restart_reason],OSPF_NEWLINE); 
	vty_out(vty,"  On Helper:%s%s",str[p_process->restart_exitreason],OSPF_NEWLINE); 

	vty_out(vty,"%s",OSPF_NEWLINE); 

	ospf_semgive();

    return ;
}
void ospf_display_process(struct vty *vty, u_int process_id)
{
    struct ospf_process *p_process = NULL;
    
    p_process = ospf_process_lookup(&ospf, process_id);
    if(NULL == p_process)
    {
        return ;
    }

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return  ;                
    }

    vty_out(vty, "%s\t\t\tProcess:%d,vrid:%d%s",OSPF_NEWLINE,(int)p_process->process_id,(int)p_process->vrid,OSPF_NEWLINE);
    vty_out(vty, "  ABR Flag:%d, ASBR Flag:%d %s", p_process->abr, p_process->asbr,OSPF_NEWLINE);
    vty_out(vty, "  SPF Interval:%d,Scheduled:%d, Running:%d%s",
           (int)p_process->spf_interval, (int)p_process->spf_called_count,
           (int)p_process->spf_running_count,OSPF_NEWLINE);
    vty_out(vty, "  System Update Pending:%d%s", p_process->wait_export,OSPF_NEWLINE);
    vty_out(vty, "%s",OSPF_NEWLINE);
    ospf_semgive();  

    return ;
}
void 
ospf_display_global(struct vty *vty)
{
    struct ospf_process *p_process  = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_if_stat stat;
    u_int *dest = NULL;
    u_int *src = NULL;
    u_int rx_count = 0 ;
    u_int tx_count = 0 ;
    u_int rxmt_count = 0;
    u_int i = 0;
    u_int8 lsa_type[OSPF_PACKET_ACK+1][15]={"","HELLO","DD","REQUEST","UPDATE","ACK"};
    u_int8 syn_type[OSPF_SYN_TYPE_LSA_RXMT+1][16]= {"process","area","if","nbr",
               "lsa","lsaFragement", "lsa rsmt" };
    if(NULL == vty)
	{
		return;
	}
    vty_out(vty, " OSPF global structure size %d%s",  sizeof(ospf),OSPF_NEWLINE);        

    vty_out(vty, " RawSocket=%d, RouteSocket=%d%s", ospf.sock[0], ospf.rtsock,OSPF_NEWLINE);
    
    /*new memory show */
    ospf_display_memory_statistics();

    vty_out(vty, "  Working Mode:%s%s", (ospf.work_mode==OSPF_MODE_NORMAL)? "NOMAL" : ((ospf.work_mode==OSPF_MODE_MASTER)? "MASTER" :"SLAVE"),OSPF_NEWLINE);

    vty_out(vty, "%s OSPF System Statistics%s",OSPF_NEWLINE,OSPF_NEWLINE);

    vty_out(vty, "  kernel Route Table Statistic:%s",OSPF_NEWLINE);
    vty_out(vty, "  System Route Add: success=%d, failed=%d%s",
        (int)ospf.stat.sys_add_ok, (int)ospf.stat.sys_add_error,OSPF_NEWLINE);
    vty_out(vty, "  System Route Del: success=%d, failed=%d%s",
        (int)ospf.stat.sys_delete_ok,(int)ospf.stat.sys_delete_error,OSPF_NEWLINE);
    vty_out(vty, "  Backup Route Add: success=%d, failed=%d%s",ospf.stat.backup_add_ok, ospf.stat.backup_add_error,OSPF_NEWLINE);
    vty_out(vty, "  Backup Route Del: success=%d, not need del=%d, failed=%d%s",ospf.stat.backup_del_ok, (int)ospf.stat.backup_not_del, ospf.stat.backup_del_error,OSPF_NEWLINE);

    vty_out(vty, "  System Error: current errno=%d, normal errno=%d, normal error count=%d%s", 
        (int)ospf.stat.sys_errno, (int)ospf.stat.kernal_other_errno,(int)ospf.stat.kernal_other_errcnt,OSPF_NEWLINE);
    vty_out(vty, "  Last Error Route,rtsock=%d, vrid=%d, rt_add=%d, dest=%x/%x, fwd=%x, metric=%d%s",
        (int)ospf.stat.err_route.rtsock, (int)ospf.stat.err_route.vrid, (int)ospf.stat.err_route.rt_add,
        (int)ospf.stat.err_route.dest, (int)ospf.stat.err_route.mask, (int)ospf.stat.err_route.fwd,
        (int)ospf.stat.err_route.metric,OSPF_NEWLINE);
    
    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, "  Hardware Route Table Statistic:%s",OSPF_NEWLINE);

    vty_out(vty, "  Route Msg: success=%d, failed=%d%s",
        (int)ospf.stat.sys_msg_ok,(int)ospf.stat.sys_msg_error,OSPF_NEWLINE);
    vty_out(vty, "  Route Msg route add cnt:  total add=%d, add failed=%d%s",
        (int)ospf.stat.rt_msg_add_total, (int)ospf.stat.rt_msg_add_err,OSPF_NEWLINE);
    vty_out(vty, "  Route Msg route del cnt:  total del=%d, del failed-%d%s",
        (int)ospf.stat.rt_msg_del_total, (int)ospf.stat.rt_msg_del_err,OSPF_NEWLINE);
    vty_out(vty, "  Route Msg route cnt: total sucess=%d%s", (int)ospf.stat.sys_msg_rt_count,OSPF_NEWLINE);
    vty_out(vty, "  Route Msg Errorno: errorno=%d%s",(int)ospf.stat.sys_msg_errno,OSPF_NEWLINE);

    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, " RTM Msg RX Statistic%s",OSPF_NEWLINE);
    vty_out(vty, " RTM_IFADDR_ADD=%d,RTM_IFADDR_DEL=%d,RTM_IFINFO=%d%s",
        (int)ospf.stat.rtm.ifaddr_add,(int)ospf.stat.rtm.ifaddr_del,(int)ospf.stat.rtm.ifinfo_cnt,OSPF_NEWLINE);
    vty_out(vty, " RTM_ADD=%d,RTM_DEL=%d,RTM_NEW_MSG=%d,rt_add=%d,rt_del=%d%s",    
        (int)ospf.stat.rtm.rtm_add_cnt, (int)ospf.stat.rtm.rtm_del_cnt,
        (int)ospf.stat.rtm.rtmsg_cnt, (int)ospf.stat.rtm.rtmsg_rt_add, (int)ospf.stat.rtm.rtmsg_rt_del,OSPF_NEWLINE);
    vty_out(vty, "%s Pkt To Cpu Errcnt%d%s",OSPF_NEWLINE,ospf.stat.pkt_tocpu_error,OSPF_NEWLINE);
    vty_out(vty, "  mutigroup errcnt%d, errno=%d%s",ospf.stat.mutigroup_error, ospf.stat.mutigroup_errno,OSPF_NEWLINE);
    
    vty_out(vty, "%s mutigroup_jion_cnt=%d%s",OSPF_NEWLINE,ospf.stat.mutigroup_jion_cnt,OSPF_NEWLINE);
    vty_out(vty, "  mutigroup_drop_cnt=%d\r\n",ospf.stat.mutigroup_drop_cnt);
    
    vty_out(vty, "%s OSPF Sync Statistics%s",OSPF_NEWLINE,OSPF_NEWLINE);
    vty_out(vty, " TxPktcnt =%d, RcvPktcnt=%d%s",
        (int)ospf.stat.sync.send_pkt_cnt,(int)ospf.stat.sync.rcv_pkt_cnt,OSPF_NEWLINE);

    vty_out(vty, " TxLsa =%d, TxOldLsa =%d, RxLsa=%d, RxOldLsa=%d, RxError=%d%s",
        (int)ospf.stat.sync.send_lsa_add,(int)ospf.stat.sync.send_lsa_del, (int)ospf.stat.sync.rcv_lsa_add, (int)ospf.stat.sync.rcv_lsa_del, (int)ospf.stat.sync.rcv_lsa_error,OSPF_NEWLINE);
    vty_out(vty, " Sync Message Statistics%s",OSPF_NEWLINE);
    vty_out(vty, " %-15s%-15s%-15s%s","syn type", "send", "rcv",OSPF_NEWLINE);

    for(i=OSPF_SYN_TYPE_INSTANCE; i<OSPF_SYN_TYPE_LSA_RXMT+1; i++)
    {
        vty_out(vty, " %-15s%-15d%-15d%s",syn_type[i],(int)ospf.stat.sync.send_msg[i],(int)ospf.stat.sync.rcv_msg[i],OSPF_NEWLINE);
    } 
    vty_out(vty, " seq mismatch packet:%d, time:%s%s",(int)ospf.stat.sync.seq_mismatch, ospf.stat.sync.seq_mismatch_time,OSPF_NEWLINE);
    
    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, " avl api usage statistic%s",OSPF_NEWLINE);
    vty_out(vty, " GetFirst=%d, Lookup=%d, Add=%d, Delete=%d  %s",(int)ospf.stat.list_first,
             (int)ospf.stat.list_lookup, (int)ospf.stat.list_add, (int)ospf.stat.list_delete, OSPF_NEWLINE);
    

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty, " OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return  ;                
    }

    memset(&stat, 0, sizeof(stat));
    dest = (u_int *)&stat;

    for_each_ospf_process(p_process, p_nxtinstance)
    {

        for_each_ospf_if(p_process, p_if, p_next_if)
        {
           src = (u_int *)p_if->stat;
           for (i = 0 ; i < sizeof(struct ospf_if_stat)/4 ; i++)
           {
              dest[i] += src[i];
           }
        }
        for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr)
        {
            rxmt_count += p_nbr->rxmt_count;
        }
    }
    ospf_semgive();
    vty_out(vty, "%sTotal Packet Statistics%s",OSPF_NEWLINE,OSPF_NEWLINE);
    vty_out(vty, "          %-10s%-10s%-12s%-12s%-10s%-10s%s","Rcv pkt","Send pkt", "Rcv bytes","Send bytes","Rcv Lsa","Send Lsa",OSPF_NEWLINE);
    for (i = 1 ; i < OSPF_PACKET_ACK+1; i++)
    {
       vty_out(vty, "%-10s%-10d%-10d%-12d%-12d%-10d%-10d%s",lsa_type[i],
           (int)stat.rx_packet[i], (int)stat.tx_packet[i],(int)stat.rx_byte[i],(int)stat.tx_byte[i],
           (int)stat.rx_lsainfo[i],(int)stat.tx_lsainfo[i],OSPF_NEWLINE);
       rx_count += stat.rx_packet[i];
       tx_count += stat.tx_packet[i];
    }

    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, "  Total Discard:%d%s",(int)ospf.stat.discard_packet,OSPF_NEWLINE);
    vty_out(vty, "  Total Rxd:%d%s",(int)rx_count,OSPF_NEWLINE);
    vty_out(vty, "  Total Txd:%d%s",(int)tx_count,OSPF_NEWLINE);
    vty_out(vty, "  Rxd Unicast:%d%s",(int)stat.rx_unicast_pkt,OSPF_NEWLINE);
    vty_out(vty, "  Rxd Multicast:%d%s",(int)stat.rx_muti_pkt,OSPF_NEWLINE);    
    vty_out(vty, "  Txd Unicast:%d%s",(int)stat.tx_unicast_pkt,OSPF_NEWLINE);
    vty_out(vty, "  Txd Multicast:%d%s",(int)stat.tx_muti_pkt,OSPF_NEWLINE);    
    vty_out(vty, "  Init DD Retransmit:%d%s",(int)stat.init_dd_rxmt,OSPF_NEWLINE);    
    vty_out(vty, "  DD Retransmit:%d%s",(int)stat.dd_rxmt,OSPF_NEWLINE);        
    vty_out(vty, "  Request Retransmit:%d%s",(int)stat.req_rxmt,OSPF_NEWLINE);        
    vty_out(vty, "  Mcast Update:%d%s",(int)stat.mcast_update,OSPF_NEWLINE);        
    vty_out(vty, "  Ucast Update:%d%s",(int)stat.ucast_update,OSPF_NEWLINE);        
    vty_out(vty, "  Nbr Rxmt Count:%d%s",(int)rxmt_count,OSPF_NEWLINE);
#ifdef HAVE_BFD
    vty_out(vty, "  BFD admin down:%d%s",(int)ospf.stat.bfd_admin_down,OSPF_NEWLINE);
    vty_out(vty, "  BFD nbr down:%d%s",(int)ospf.stat.bfd_nbr_down,OSPF_NEWLINE);
#endif
    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, "  LSA Check Statistics%s",OSPF_NEWLINE);
    vty_out(vty, "  originate router  lsa count:%d%s",(int)ospf.stat.check_router_lsa_count,OSPF_NEWLINE);
    vty_out(vty, "  originate network lsa count:%d%s",(int)ospf.stat.check_network_lsa_count,OSPF_NEWLINE);
    vty_out(vty, "%s",OSPF_NEWLINE);
    
 /*self lsa originated counter*/
    vty_out(vty, "  Count of Local Lsa Originate%s",OSPF_NEWLINE);
    vty_out(vty, "  %-10s%-10s%-10s%s","Type","Build","Changed",OSPF_NEWLINE);
    for (i = OSPF_LS_ROUTER ; i < OSPF_LS_MAX ; i++)
    {
         if (ospf.stat.self_lsa_build[i])
         {
             vty_out(vty, "  %-10d%-10d%-10d%s",(int)i,(int)ospf.stat.self_lsa_build[i],(int)ospf.stat.self_lsa_change[i],OSPF_NEWLINE);
         }
    }

    /*rxd lsa counter*/    
    vty_out(vty, "%s    Count of Received Lsa%s",OSPF_NEWLINE,OSPF_NEWLINE);
    vty_out(vty, "  %-6s%-8s%-12s%-12s%-12s%-12s%s",
                  "Type","Rxd","MoreRecent","LessRecent","SameRecent","BodyChanged",OSPF_NEWLINE);
    for (i = OSPF_LS_ROUTER ; i < OSPF_LS_MAX ; i++)
    {
         if (ospf.stat.rxd_lsa[i])
         {
             vty_out(vty, "  %-6d%-8d%-12d%-12d%-12d%-12d%s",
                           (int)i,(int)ospf.stat.rxd_lsa[i],(int)ospf.stat.rxd_more_recent_lsa[i],
                           (int)ospf.stat.rxd_less_recent_lsa[i],
                           (int)ospf.stat.rxd_same_recent_lsa[i],
                           (int)ospf.stat.rxd_change_lsa[i],OSPF_NEWLINE);             
         }
    }
    vty_out(vty, "%s",OSPF_NEWLINE);

    return ;
}

void
ospf_display_stat(struct vty *vty, u_int count)
{
    if(count == 0)
    {
        count = 10;
    }
    vty_out(vty, "%s  Latest SPF running:%s",OSPF_NEWLINE,OSPF_NEWLINE);
    ospf_display_spf_stat(vty, count);

    vty_out(vty, "%s  Latest Neighbor Event:%s",OSPF_NEWLINE,OSPF_NEWLINE);
    ospf_display_nbr_stat(vty, count);

    vty_out(vty, "%s  Latest Lsa Stat:%s",OSPF_NEWLINE,OSPF_NEWLINE);
    ospf_display_lsa_stat(vty, count);
    return;
}
void 
ospf_clear_statistic(struct ospf_process *p_process)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    
    p_process->spf_called_count = 0;
    p_process->spf_running_count = 0;
    p_process->max_spf_time = 0;
    p_process->origin_lsa = 0;
    p_process->rx_lsa = 0;
    ospf_spf_log_clear(p_process);
    ospf_nbr_log_clear(p_process);
    ospf_lsa_log_clear(p_process);
#ifdef HAVE_BFD
    ospf_bfd_log_clear(p_process);
#endif

    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        memset(p_if->stat, 0, sizeof(*p_if->stat));
    }
    return ;
}

/*clear interface ospf packet counts*/
void 
ospf_clear_interface_statistic(u_long ulIfIndex)
{    
    struct ospf_process *p_process = NULL;
	struct ospf_process *p_nprocess = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_nextif = NULL;
	
    for_each_node(&ospf.process_table, p_process, p_nprocess)
	{
	    for_each_ospf_if(p_process, p_if, p_nextif)
		{
			if (ulIfIndex == p_if->ifnet_uint)
			{
			    memset(p_if->stat, 0, sizeof(*p_if->stat));
            }
        }
    }
    
    return ;
}


/*display nexthop table*/
void 
ospf_display_nexthop(struct vty *vty, u_long ulInstance)
{
    struct ospf_nexthop *p_nexthop = NULL;
    struct ospf_nexthop *p_next = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    u_int i = 0 ;
    u_int j = 0 ;
    u_int8 gstr[32];

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return  ;                
    }

    /*
        index reference count nexthop
         1         1           1       xxx/xxx
     */
    vty_out(vty, "%s Index Reference Count Nexthop IfIndex",OSPF_NEWLINE); 
    for_each_node(&ospf.process_table, p_process, p_next_process)
    {
        if((0 != ulInstance) && (ulInstance != p_process->process_id))
        {
            continue;
        }
        vty_out(vty, "%sprocess id %d",OSPF_NEWLINE, p_process->process_id);
        for_each_node(&p_process->nexthop_table, p_nexthop, p_next)
        {
            vty_out(vty, "%s %-6d%-9d%-5d ",OSPF_NEWLINE, (int)i++, 0, (int)p_nexthop->count);
            for (j = 0 ;  j < p_nexthop->count; j++)
            {
                vty_out(vty, "|| %s %#x", ospf_inet_ntoa(gstr, p_nexthop->gateway[j].addr), p_nexthop->gateway[j].if_uint);
            }
        }
    }
    vty_out(vty, "%s",OSPF_NEWLINE);
    

    ospf_semgive();

    return ;
}

void 
ospf_display_timer(struct vty *vty, u_long ulInstance, u_long ulIfIndex)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;  
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ;
    u_int count = 0;


    if (OK != ospf_semtake_timeout())
    {
        return ;
    }
    vty_out(vty, "%s",OSPF_NEWLINE);
    for_each_ospf_process(p_process, p_nxtinstance)
    {
        if((ulInstance != 0) && (p_process->process_id != ulInstance))
        {
            continue;
        }
        count= 0;
        vty_out(vty, " process id =%d%s",(int)p_process->process_id,OSPF_NEWLINE);
        
        vty_out(vty, " p_process->ipupdate_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                    p_process->ipupdate_timer,p_process->ipupdate_timer.node,p_process->ipupdate_timer.active,
                    p_process->ipupdate_timer.func,*(u_int *)p_process->ipupdate_timer.arg,p_process->ipupdate_timer.context,OSPF_NEWLINE);
        vty_out(vty, " p_process->backup_ipupdate_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                    p_process->backup_ipupdate_timer,p_process->backup_ipupdate_timer.node,p_process->backup_ipupdate_timer.active,
                    p_process->backup_ipupdate_timer.func,*(u_int *)p_process->backup_ipupdate_timer.arg,p_process->backup_ipupdate_timer.context,OSPF_NEWLINE);
        vty_out(vty, " p_process->spf_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                    p_process->spf_timer,p_process->spf_timer.node,p_process->spf_timer.active,
                    p_process->spf_timer.func,*(u_int *)p_process->spf_timer.arg,p_process->spf_timer.context,OSPF_NEWLINE);
        vty_out(vty, " p_process->fast_spf_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                    p_process->fast_spf_timer,p_process->fast_spf_timer.node,p_process->fast_spf_timer.active,
                    p_process->fast_spf_timer.func,*(u_int *)p_process->fast_spf_timer.arg,p_process->fast_spf_timer.context,OSPF_NEWLINE);                
        vty_out(vty, " p_process->delete_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                        p_process->delete_timer,p_process->delete_timer.node,p_process->delete_timer.active,
                        p_process->delete_timer.func,*(u_int *)p_process->delete_timer.arg,p_process->delete_timer.context,OSPF_NEWLINE);                
        vty_out(vty, " p_process->id_reset_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                        p_process->id_reset_timer,p_process->id_reset_timer.node,p_process->id_reset_timer.active,
                        p_process->id_reset_timer.func,*(u_int *)p_process->id_reset_timer.arg,p_process->id_reset_timer.context,OSPF_NEWLINE);                
        vty_out(vty, " p_process->redis_range_update_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                        p_process->redis_range_update_timer,p_process->redis_range_update_timer.node,p_process->redis_range_update_timer.active,
                        p_process->redis_range_update_timer.func,*(u_int *)p_process->redis_range_update_timer.arg,p_process->redis_range_update_timer.context,OSPF_NEWLINE);                
        vty_out(vty, " p_process->restart_wait_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
                        p_process->restart_wait_timer,p_process->restart_wait_timer.node,p_process->restart_wait_timer.active,
                        p_process->restart_wait_timer.func,*(u_int *)p_process->restart_wait_timer.arg,p_process->restart_wait_timer.context,OSPF_NEWLINE);                
        #if 0                
        if(p_process->process_id == OSPF_DCN_PROCESS)
        {
            vty_out(vty, " p_process->dcn_rtm_tx_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .\r\n",
                        p_process->dcn_rtm_tx_timer,p_process->dcn_rtm_tx_timer.node,p_process->dcn_rtm_tx_timer.active,
                        p_process->dcn_rtm_tx_timer.func,*(u_int *)p_process->dcn_rtm_tx_timer.arg,p_process->dcn_rtm_tx_timer.context);
        }
        #endif
        
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if((ulIfIndex != 0) && (p_if->ifnet_uint != ulIfIndex))
            {
                continue;
            }
            vty_out(vty, "%s\t if name %s",OSPF_NEWLINE, p_if->name);
            
	        vty_out(vty, "%s\t p_if hello_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .",OSPF_NEWLINE,
				p_if->hello_timer,p_if->hello_timer.node,p_if->hello_timer.active,
				p_if->hello_timer.func,*(u_int *)p_if->hello_timer.arg,p_if->hello_timer.context);
	        vty_out(vty, "%s\t p_if poll_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .",OSPF_NEWLINE,
				p_if->poll_timer,p_if->poll_timer.node,p_if->poll_timer.active,
				p_if->poll_timer.func,*(u_int *)p_if->poll_timer.arg,p_if->poll_timer.context);

		    vty_out(vty, "%s\t p_if wait_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .",OSPF_NEWLINE,
				p_if->wait_timer,p_if->wait_timer.node,p_if->wait_timer.active,
				p_if->wait_timer.func,*(u_int *)p_if->wait_timer.arg,p_if->wait_timer.context);
	        vty_out(vty, "%s\t p_if ack_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .",OSPF_NEWLINE,
				p_if->ack_timer,p_if->ack_timer.node,p_if->ack_timer.active,
				p_if->ack_timer.func,*(u_int *)p_if->ack_timer.arg,p_if->ack_timer.context);

		    vty_out(vty, "%s\t p_if flood_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .",OSPF_NEWLINE,
				p_if->flood_timer,p_if->flood_timer.node,p_if->flood_timer.active,
				p_if->flood_timer.func,*(u_int *)p_if->flood_timer.arg,p_if->flood_timer.context);
			count++;
        }
        vty_out(vty, "%s\t Total num : %d", OSPF_NEWLINE,count);
        vty_out(vty, "%s",OSPF_NEWLINE);
    }

    ospf_semgive();

}

void 
ospf_display_timer_thread(struct vty *vty)
{
    struct ospf_timer_block *p_head;
    struct ospf_timer_block *p_next;
    struct ospf_timer *p_check = NULL;
    struct list_head *list = NULL;
    struct list_head * listnode = NULL;
    u_int now = ospf_sys_ticks();
    u_int count = 0;

    if (OK != ospf_semtake_timeout())
    {
        return ;
    }
    vty_out(vty, "%s",OSPF_NEWLINE);
    for_each_node(&ospf.thread_table, p_head, p_next)
    {
        list = &p_head->list;                       
        
        list_for_each(listnode, list)
        {
            p_check = list_entry(listnode, struct ospf_timer, node);
            vty_out(vty, "%s timer %x, active %s, delay %d, fun %x, remain time %d",OSPF_NEWLINE, 
                (int)p_check, p_check->active?"Ture":"False", (int)p_check->delay, (int)p_check->func, (int)ospf_timer_remain(p_check, now));
            count++;
        }        
    }
    vty_out(vty, "%stimer total count:%d%s",OSPF_NEWLINE,(int)count,OSPF_NEWLINE);
    vty_out(vty, "%s",OSPF_NEWLINE);
    ospf_semgive();
}

void 
ospf_display_conflict_network(struct vty *vty, u_long ulInstance)
{    
    struct ospf_process *p_process  = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    struct ospf_conflict_network *p_conflict = NULL;
    struct ospf_conflict_network *p_next = NULL;

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug," OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                
    }

    for_each_ospf_process(p_process, p_nxtinstance)
    {
        if((ulInstance != 0) && (p_process->process_id != ulInstance))
        {
            continue;
        }
        vty_out(vty, "-------------process : %d-------------%s", p_process->process_id,OSPF_NEWLINE);
        for_each_node(&p_process->t5_lstable.conflict_list,p_conflict,p_next)
        {
            vty_out(vty, "conflict network%x%s", (int)p_conflict->network,OSPF_NEWLINE);
        }
    }   

    ospf_semgive();  

    return ;
}
void
ospf_ase_test_all(u_int dest, 
                  u_int mask, 
                  u_int lsacount, 
                  u_int active)
{
  struct ospf_iproute route ;
  struct ospf_process *p_process = NULL;
  struct ospf_process *p_next_process = NULL;
  u_int i = 0 ;
  u_int start = 0 ;
  u_int process_count = 0;
  u_int each_count = 0;
  u_int step = ~mask;
  step ++;

  ospf_logx(ospf_debug,"ase_lsa_test  all dest=%x,mask=%x,count=%d\n\r",(int)dest, (int)mask, (int)lsacount);
 
  process_count = ospf_lstcnt(&ospf.process_table);
  each_count = lsacount/process_count;
  for_each_ospf_process(p_process, p_next_process)
  {
      p_process->asbr = TRUE;
      ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);      

      memset(&route, 0, sizeof(route));
      route.mask = mask;
      route.active = active;
      route.p_process = p_process;
      //start = vos_get_system_tick();
      for (i = 0 ; i < each_count ; i ++)
      {
         if (OK != ospf_semtake_forever())
         {
             return ;
         }
         route.dest = dest; 
         dest += step;
         ospf_external_lsa_originate(&route) ;
         ospf_semgive();
      }
  }
  
    //start = vos_get_system_tick() - start;
  ospf_logx(ospf_debug,"\n\roriginate %u external lsas,use time=%u ticks\n\r",(int)lsacount,(int)start);
  return;
}

void
ospf_ase_lsa_test(
          u_int dest, 
          u_int mask, 
          u_int count, 
          u_int active,
          u_int process_id)
{
  struct ospf_iproute route ;
  struct ospf_process *p_process = ospf_process_lookup(&ospf, process_id);
  u_int i = 0 ;
  u_int start = 0 ;
  u_int step = ~mask;
  step ++;

  ospf_logx(ospf_debug,"ase_lsa_test dest=%x,mask=%x,count=%d\n\r",(int)dest, (int)mask, (int)count);

  if (!p_process)
  {
      ospf_logx(ospf_debug,"instance not exist");
      return;
  }
  p_process->asbr = TRUE;
  memset(&route, 0, sizeof(route));
  route.mask = mask;
  route.active = active;
  route.p_process = p_process;
  //start = vos_get_system_tick();

  for (i = 0 ; i < count ; i ++)
  {
     route.dest = dest;

     dest += step;
     if (OK != ospf_semtake_forever())
     {
         return ;
     }
     ospf_external_lsa_originate(&route) ;
     ospf_semgive();
  }

   //start = vos_get_system_tick() - start;
  ospf_logx(ospf_debug,"\n\roriginate %d external lsas,use time=%d ticks\n\r",(int)count,(int)start);
}

void
ospf_redistribute_ase_lsa_test(
          u_int dest, 
          u_int mask, 
          u_int count, 
          u_int active,
          u_int process_id)
{
  struct ospf_iproute route ;
  struct ospf_process *p_process = ospf_process_lookup(&ospf, process_id);
  u_int i = 0 ;
  u_int start = 0 ;
  u_int step = ~mask;
  step ++;

  ospf_logx(ospf_debug,"ase_lsa_test dest=%x,mask=%x,count=%d\n\r",(int)dest, (int)mask, (int)count);

  if (!p_process)
  {
      ospf_logx(ospf_debug,"instance not exist");
      return;
  }
  p_process->asbr = TRUE;
  memset(&route, 0, sizeof(route));
  route.mask = mask;
  route.active = active;
  route.p_process = p_process;
  route.proto = M2_ipRouteProto_netmgmt;

  //start = vos_get_system_tick();
  for (i = 0 ; i < count ; i ++)
  {
     route.dest = dest;

     dest += step;
     if (OK != ospf_semtake_forever())
     {
         return ;
     }
     ospf_import_iproute((active) ? TRUE : FALSE, &route);
     ospf_semgive();
  }

  //start = vos_get_system_tick() - start;

  ospf_logx(ospf_debug,"\n\roriginate %d external lsas,use time=%d ticks\n\r",(int)count,(int)start);
}

void 
ospf_display_lstable(struct vty *vty, u_long ulInstance)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_next = NULL;
    u_int uiHisType = 0;
    u_char szcAddr[32] = {0};

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF    is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return;                
    }
    vty_out(vty, "%s",OSPF_NEWLINE);
    for_each_ospf_process(p_process, p_next_process)
    {
        if(ulInstance && (ulInstance != p_process->process_id))
        {
            continue;
        }
        vty_out(vty, " process %d:%s", (int)p_process->process_id,OSPF_NEWLINE);
        for_each_node(&p_process->lstable_table, p_table, p_next)
        {
            /*因为在添加if的时候就是添加到type 9里面的*/
            if(p_table->type != uiHisType)
            {
                vty_out(vty, "\t table type %d:%s",(int)p_table->type,OSPF_NEWLINE);
                uiHisType = p_table->type;
            }
            #if 0
            if (p_table->p_area)
            {
                if(p_table->p_area->id != uiHisAreaId)
                vty_out(vty, "\t area id %d\r\n", (int)p_table->p_area->id);
            }
            #endif
            if (p_table->p_if)
            {
                ospf_inet_ntoa(szcAddr, p_table->p_if->addr);
                vty_out(vty, "\t if addr %s%s", szcAddr,OSPF_NEWLINE);
            }
        }
        vty_out(vty, "%s",OSPF_NEWLINE);
    }
    
    ospf_semgive();
    return;
}

void
ospf_display_spf_stat(struct vty *vty, u_int count)
{
    struct ospf_spf_loginfo *p_spf;
    struct ospf_spf_loginfo *p_next_spf;
    struct ospf_spf_loginfo *p_stat[OSPF_STAT_MAX_NUM];
    u_int i = 0;
    u_int x = 0 ;
    u_int8 running_str[64];
    u_int8 route_sr[64];


    if (OK != ospf_semtake_timeout())
    {
        return ;
    }

    if(count == 0)
    {
       count = OSPF_STAT_MAX_NUM;
    }
    
    i = 0;
    vty_out(vty, " %-8s%-20s%-18s%-15s%-8s%s","process","spf_start_time","calc(spf/ia/ase)","route/abr/asbr","callspf",OSPF_NEWLINE);
  
    memset(p_stat, 0, sizeof(p_stat));
    x = OSPF_STAT_MAX_NUM-1;
    for_each_node(&ospf.log_table.spf_table, p_spf, p_next_spf)
    {
       p_stat[x] = p_spf;
       x--;
       if (!x)
       {
          break;
       }
    }
    
    for (x = 0; x < (OSPF_STAT_MAX_NUM); x++)
    {
        
        p_spf = p_stat[x];
        if (NULL == p_spf)
        {
           continue;
        }
        if(i++ > count)
        {
            break;
        }
        sprintf(running_str,"%d(%d/%d/%d)",(int)p_spf->calculate_period*OSPF_MS_PER_TICK, p_spf->spf_period*OSPF_MS_PER_TICK,p_spf->ia_period*OSPF_MS_PER_TICK, p_spf->ase_period*OSPF_MS_PER_TICK);
        sprintf(route_sr,"%d/%d/%d",(int)p_spf->route_num, p_spf->abr_num, p_spf->asbr_num);
        vty_out(vty, " %-8d%-20s%-18s%-15s%-8d%s", (int)p_spf->process_id, p_spf->spf_start_time, running_str,route_sr,p_spf->spf_called,OSPF_NEWLINE);
    }
         
     ospf_semgive();

}

void
ospf_display_nbr_stat(struct vty *vty, u_int count)
{
    struct ospf_nbr_loginfo *p_nbr;
    struct ospf_nbr_loginfo *p_next_nbr;
    u_int8 *s_state[9] = {"","Down","Attempt","Init", "2-way","ExStart", "Exchange", "Loading","Full"};	
    u_int8 *s_event[] = {"hello-rcvd","Start","2-way","neg-done","exchange-done","bad-request","loading-done","Adj-OK","SeqMismatch","1-way","KillNeighbor","Expired","LowlayerDown"}; 
    u_int i = 0;
    u_int8 str[32];
    u_int total_cnt = 0;  


    if (OK != ospf_semtake_timeout())
    {
        return ;
    }

    if(count == 0)
    {
       count = OSPF_STAT_MAX_NUM;
    }
    
    i = 0;
    vty_out(vty, " %-8s%-20s%-16s%-12s%-10s%-15s%s","process","time", "nbr_addr","last_state","state","event",OSPF_NEWLINE);
  
    total_cnt = ospf_lstcnt(&ospf.log_table.nbr_table);
    for_each_node(&ospf.log_table.nbr_table, p_nbr, p_next_nbr)
    {
        if((total_cnt > count) && (i++ < (total_cnt - count)))
        {
            continue;
        }
        vty_out(vty, " %-8d%-20s%-16s%-12s%-10s%-15s%s", (int)p_nbr->process_id, p_nbr->time, ospf_inet_ntoa(str,p_nbr->nbr_addr),
        s_state[p_nbr->last_state], s_state[p_nbr->state], s_event[p_nbr->event],OSPF_NEWLINE);
  
    }
       
    ospf_semgive();

}

void
ospf_display_lsa_stat(struct vty *vty, u_int count)
{
    struct ospf_lsa_loginfo *p_stat;
    struct ospf_lsa_loginfo *p_next_stat;
    struct ospf_lshdr *p_hdr = NULL;
    u_int8 *s_action[] = {"add","del","mod"}; 
    u_int i = 0;
    u_int8 idstr[32];
    u_int8 advstr[32];
    u_int total_cnt = 0;


    if (OK != ospf_semtake_timeout())
    {
        return ;
    }

    if(count == 0)
    {
       count = OSPF_STAT_MAX_NUM;
    }
   
    i = 0;
    vty_out(vty, "%-8s%-20s%-2s%-16s%-16s%-10s%-5s%-5s%-5s%s","process","time", "t","id","adv","seqnum","age","len", "action",OSPF_NEWLINE);
  
    total_cnt = ospf_lstcnt(&ospf.log_table.lsa_table);
    for_each_node(&ospf.log_table.lsa_table, p_stat, p_next_stat)
    {
        if((total_cnt > count) && (i++ < (total_cnt - count)))
        {
            continue;
        }
        p_hdr = (struct ospf_lshdr *)p_stat->lsa_hdr;
  
        vty_out(vty, "%-8d%-20s%-2d%-16s%-16s%-10x%-5d%-5d%-5s%s", (int)p_stat->process_id,p_stat->time,p_hdr->type,ospf_inet_ntoa(idstr,ntohl(p_hdr->id)),
            ospf_inet_ntoa(advstr,ntohl(p_hdr->adv_id)),(int)ntohl(p_hdr->seqnum),ntohs(p_hdr->age),
            ntohs(p_hdr->len), s_action[p_stat->action],OSPF_NEWLINE);
  
    }
          
    ospf_semgive();

}

#ifdef HAVE_BFD
void
ospf_display_bfd_stat(struct vty *vty, u_int count)
{
    struct ospf_bfd_loginfo *p_bfd;
    struct ospf_bfd_loginfo *p_next_bfd;
    u_int8 *s_state[9] = {"","Admin down","Down","Init", "Up","Failing"};    
    u_int8 *s_diag[] = {"no diagnostic","control detection time expired","echo function failed","neighbor signaled session down",
        "forwarding plane reset","path down","concatenated path down","administratively down","Reverse concatenated down"}; 
    u_int i = 0;
    u_int8 str[32];
    u_int total_cnt = 0;  


    if (OK != ospf_semtake_timeout())
    {
        return ;
    }

    if(count == 0)
    {
       count = OSPF_STAT_MAX_NUM;
    }

    i = 0;
    vty_out(vty," %-8s%-20s%-16s%-12s%s%s", "process", "time", "nbr addr", "state", "diag",OSPF_NEWLINE);

    total_cnt = ospf_lstcnt(&ospf.log_table.bfd_table);
    for_each_node(&ospf.log_table.bfd_table, p_bfd, p_next_bfd)
    {
        if((total_cnt > count) && (i++ < (total_cnt - count)))
        {
            continue;
        }
        vty_out(vty," %-8d%-20s%-16s%-12s%s%s", (int)p_bfd->process_id, p_bfd->time, ospf_inet_ntoa(str,p_bfd->nbr_addr),
        s_state[p_bfd->state], s_diag[p_bfd->diag],OSPF_NEWLINE);
    }


    ospf_semgive();

}
#endif

  /*display lsa counter with special router id*/
struct ospf_lsa_router_count{
   u_int process;
   u_int id;
   u_int typecount[12];
};

void
ospf_display_lsa_router_id_count(u_int process)
{
  struct ospf_lsa_router_count counter[256];
  struct ospf_lstable *p_table = NULL;
  struct ospf_lstable *p_ntable = NULL;
  struct ospf_lsa *p_lsa = NULL;
  struct ospf_lsa *p_nlsa = NULL;
  struct ospf_process *p_process = NULL;
  struct ospf_process *p_nprocess = NULL;
  u_int8 routerstr[64];
  u_int i = 0 ;
  
  vty_out_to_all_terminal("\n\r LSDB count for different router");
  vty_out_to_all_terminal(" Proc   ID              router network type-3 type-4 external ");  
  memset(counter, 0, sizeof(counter));
  /*process router-id router network intra inter-prefix inter-router link external*/
  /*scan for all lsa table*/

  ospf_semtake_forever();

  for_each_node(&ospf.process_table, p_process, p_nprocess)
  {
     /*process filter*/
     if (process && (p_process->process_id != process))
     {
         continue ;
     }
     for_each_node(&p_process->lstable_table, p_table, p_ntable)
     {   
        for_each_node(&p_table->list, p_lsa, p_nlsa)
        {
            /*get special counter*/
            for (i = 0 ; i < 256 ; i++)
            {
                if (!counter[i].process)
                {
                   counter[i].process = p_process->process_id;
                   counter[i].id = ntohl(p_lsa->lshdr->adv_id);
                   break;
                }
                else if ((counter[i].process == p_process->process_id) 
                         && (counter[i].id == ntohl(p_lsa->lshdr->adv_id)))
                {
                   break;
                }
            }
            if (i >= 256)
            {
                for (i = 0 ; i < 256 ; i++)
                {
                    if (counter[i].process)
                    {
                        vty_out_to_all_terminal(" %-7d %-16s %-6d %-7d %-7d %-7d %-8d",
                          (int) counter[i].process,
                          ospf_inet_ntoa(routerstr, counter[i].id),
                          (int)counter[i].typecount[1],
                          (int)counter[i].typecount[2],
                          (int)counter[i].typecount[3],
                          (int)counter[i].typecount[4],
                          (int)counter[i].typecount[5]);  
                    }
                }
               memset(counter, 0, sizeof(counter)); 
               i = 0 ;
               counter[i].process = p_process->process_id;
               counter[i].id = ntohl(p_lsa->lshdr->adv_id);
            }

            /*incease counter*/
            counter[i].typecount[p_lsa->lshdr->type]++;
        }
  }
  }
  
  ospf_semgive();

  for (i = 0 ; i < 256 ; i++)
  {
      if (counter[i].process)
      {
                        vty_out_to_all_terminal(" %-7d %-16s %-6d %-7d %-7d %-7d %-8d",
                          (int)counter[i].process,
                          ospf_inet_ntoa(routerstr, counter[i].id),
                          (int)counter[i].typecount[1],
                          (int)counter[i].typecount[2],
                          (int)counter[i].typecount[3],
                          (int)counter[i].typecount[4],
                          (int)counter[i].typecount[5]);  
      }
  }
  return;
}
void ospf_if_group_set(u_int i)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ;    


    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }


    for_each_ospf_process(p_process, p_nxtinstance)
    {
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            p_if->flood_group = i;
        }
    }

    ospf_semgive();    

    return ;
}

void
ospf_display_nm_table(struct vty *vty)
{
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_ntable = NULL;
    

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    vty_out(vty, "%s",OSPF_NEWLINE);
    vty_out(vty, " Area Table Count:%d%s",(int)ospf_lstcnt(&ospf.nm.area_table),OSPF_NEWLINE);

    vty_out(vty, " Area lsa table Count:%d%s",(int)ospf_lstcnt(&ospf.nm.area_lsa_table),OSPF_NEWLINE);
    for_each_node(&ospf.nm.area_lsa_table, p_table, p_ntable)
    {
       vty_out(vty, " process=%d,area=%d,type=%d,lscount=%d%s",
        (int)p_table->p_process->process_id,
        (int)p_table->p_area->id,
        (int)p_table->type,(int) ospf_lstcnt(&p_table->list),OSPF_NEWLINE);
    }
    
    vty_out(vty, " AS lsa table Count:%d%s",(int)ospf_lstcnt(&ospf.nm.as_lsa_table),OSPF_NEWLINE);
    for_each_node(&ospf.nm.as_lsa_table, p_table, p_ntable)
    {
       vty_out(vty, " process=%d,type=%d,lscount=%d%s",(int)p_table->p_process->process_id,(int)p_table->type, (int)ospf_lstcnt(&p_table->list),OSPF_NEWLINE);
    }

    ospf_semgive();  

    return ;
}
void
ospf_display_import_table(struct vty *vty, u_long ulInstance, u_int count)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_iproute *p_route = NULL;
    struct ospf_iproute *p_next_route = NULL;
    u_int8 dest[32];
    u_int8 mask[32];
    u_int8 fwd[32];
    u_int i = 0;
    
    if (count == 0)
    {
        count = 100;
    }
    

    if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }

    for_each_ospf_process(p_process, p_next_process)
    {
        if((ulInstance != 0) && (p_process->process_id != ulInstance))
        {
            continue;
        }
        vty_out(vty, "process id:%d%s",(int)p_process->process_id,OSPF_NEWLINE);
        vty_out(vty, "%-16s%-16s%-16s%s", "dest", "mask", "nextop",OSPF_NEWLINE);
        for_each_node(&p_process->import_table, p_route, p_next_route)
        {
            vty_out(vty, "%-16s%-16s%-16s%s",ospf_inet_ntoa(dest, p_route->dest), ospf_inet_ntoa(mask, p_route->mask), ospf_inet_ntoa(fwd, p_route->fwdaddr),OSPF_NEWLINE);
            if (++i >= count)
            {
                break;
            }
        }
    }

    ospf_semgive();  

    return ;
}

#ifdef OSPF_FRR
void
ospf_display_backup_route(struct vty *vty, u_long ulProId)
{
    struct ospf_backup_spf_vertex *p_bvertex = NULL;
    struct ospf_spf_vertex *p_vertex = NULL;
    struct ospf_spf_vertex *p_next_vertex = NULL;   
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_backup_route *p_broute = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_next_route = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;   
    u_int backup_current = 0;
    u_int current = 0 ;
    u_int i;
    u_int8 dststr[32];
    u_int8 mskstr[32];
    u_int8 nhopstr[32];
    u_int8 parentstr[32];
    u_int8 path_type[5][8]={"INVALID","INTRA","INTER","ASE","ASE2"};

    ospf_semtake_forever();
    for_each_ospf_process(p_process, p_next_process)
    {
        if(ulProId && (p_process->process_id != ulProId))
        {
            continue;
        }
        current = p_process->current_route;
        backup_current = p_process->backup_current_route;
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            /* SPF table of area
               type id nexthop cost ifunit role
                                            main
                                            backup
             */
            vty_out(vty,"%s SPF table of area %s%s",OSPF_NEWLINE,ospf_printf_area(dststr, p_area->id),OSPF_NEWLINE);  
            
            vty_out(vty," %-8s %-17s %-17s %-6s %-8s %-20s %s%s","type","id","nexthop","cost","ifunit","parent","role",OSPF_NEWLINE);
            for_each_node(&p_area->spf_table, p_vertex, p_next_vertex)
            {
               struct ospf_backup_spf_vertex bvertex;
               /*do not check path for root*/
               if (NULL == p_vertex->p_nexthop)
               {

                 
                        vty_out(vty," %-8s %-17s %-17s %-6d %-8d %-20s main%s",
                       (1 == p_vertex->type) ? "router" : "network",
                       ospf_inet_ntoa(dststr, p_vertex->id),
                       ospf_inet_ntoa(nhopstr, 0),
                       (int)p_vertex->cost,
                       0,
                       (NULL != p_vertex->parent[0].p_node)?ospf_inet_ntoa(parentstr,p_vertex->parent[0].p_node->id) : 0,OSPF_NEWLINE);
                    continue;
               }
               
               for (i = 0 ; i < p_vertex->p_nexthop->count; i++)
               {
                  vty_out(vty," %-8s %-17s %-17s %-6d %-8d %-20s main%s",
                   (1 == p_vertex->type) ? "router" : "network",
                   ospf_inet_ntoa(dststr, p_vertex->id),
                   ospf_inet_ntoa(nhopstr, p_vertex->p_nexthop->gateway[i].addr),
                   (int)p_vertex->cost,(int)p_vertex->p_nexthop->gateway[i].if_uint,
                   (NULL != p_vertex->parent[0].p_node)?ospf_inet_ntoa(parentstr,p_vertex->parent[0].p_node->id) : 0,OSPF_NEWLINE);
               }
               /*backup*/
               bvertex.id = p_vertex->id;
               bvertex.type = p_vertex->type;
               p_bvertex = ospf_lstlookup(&p_area->backup_spf_table, &bvertex);
               if (p_bvertex)
               {
                  vty_out(vty," %-8s %-17s %-17s %-6d %-8d backup%s",
                   (1 == p_bvertex->type) ? "router" : "network",
                   ospf_inet_ntoa(dststr, p_bvertex->id),
                   ospf_inet_ntoa(nhopstr, p_bvertex->nexthop),
                   (int)p_bvertex->cost_total,(int)p_bvertex->ifunit,OSPF_NEWLINE); 

               }
            }
            
            vty_out(vty,"%sABR route for area %s%s",OSPF_NEWLINE,ospf_printf_area(dststr, p_area->id),OSPF_NEWLINE);
            vty_out(vty," %-17s %-17s %-6s %-8s %s%s","dest","nexthop","cost","ifunit","role",OSPF_NEWLINE);
            
            for_each_node(&p_area->abr_table, p_route, p_next_route)
            {
                 struct ospf_backup_route broute;
                 if (NULL == p_route->path[current].p_nexthop)
                 {
                     continue ;
                 }
                 
                 for (i = 0 ; i < p_route->path[current].p_nexthop->count; i++)
                 {
                    vty_out(vty," %-17s %-17s %-6d %-8d main%s",
                       ospf_inet_ntoa(dststr, p_route->dest),
                       ospf_inet_ntoa(nhopstr, p_route->path[current].p_nexthop->gateway[i].addr),
                       (int)p_route->path[current].cost, (int)p_route->path[current].p_nexthop->gateway[i].if_uint,OSPF_NEWLINE); 
                 }
                 broute.type = OSPF_ROUTE_ABR;
                 broute.dest = p_route->dest;
                 p_broute = ospf_lstlookup(&p_area->backup_abr_table, &broute);
                 if (p_broute)
                 {
                    vty_out(vty," %-17s %-17s %-6d %-8d backup%s",
                       ospf_inet_ntoa(dststr, p_broute->dest),
                       ospf_inet_ntoa(nhopstr, p_broute->path[backup_current].nexthop),
                       (int)p_broute->path[backup_current].cost, (int)p_broute->path[backup_current].ifunit,OSPF_NEWLINE); 
                 }
            }
            
            vty_out(vty,"%sASBR route for area %s%s",OSPF_NEWLINE,ospf_printf_area(dststr, p_area->id),OSPF_NEWLINE);
            vty_out(vty," %-17s %-17s %-6s %-8s %s%s","dest","nexthop","cost","ifunit","role",OSPF_NEWLINE);
            
            for_each_node(&p_area->asbr_table, p_route, p_next_route)
            {
                 struct ospf_backup_route broute;
                 if (NULL == p_route->path[current].p_nexthop)
                 {
                     continue ;
                 }
                 
                 for (i = 0 ; i < p_route->path[current].p_nexthop->count; i++)
                 {
                    vty_out(vty," %-17s %-17s %-6d %-8d main%s",
                       ospf_inet_ntoa(dststr, p_route->dest),
                       ospf_inet_ntoa(nhopstr, p_route->path[current].p_nexthop->gateway[i].addr),
                       (int)p_route->path[current].cost, (int)p_route->path[current].p_nexthop->gateway[i].if_uint,OSPF_NEWLINE); 
                 }
                 broute.type = OSPF_ROUTE_ASBR;
                 broute.dest = p_route->dest;
                 p_broute = ospf_lstlookup(&p_area->backup_asbr_table, &broute);
                 if (p_broute)
                 {
                    vty_out(vty," %-17s %-17s %-6d %-8d backup%s",
                       ospf_inet_ntoa(dststr, p_broute->dest),
                       ospf_inet_ntoa(nhopstr, p_broute->path[backup_current].nexthop),
                       (int)p_broute->path[backup_current].cost, (int)p_broute->path[backup_current].ifunit,OSPF_NEWLINE); 
                 }
            }
        }
        vty_out(vty,"%s network route for process %d%s",OSPF_NEWLINE,(int)p_process->process_id,OSPF_NEWLINE);
        vty_out(vty," %-8s%-17s %-17s %-17s %-6s %-8s %s%s","type","dest","mask","nexthop","cost","ifunit","role",OSPF_NEWLINE);
        for_each_node(&p_process->route_table, p_route, p_next_route)
        {
            struct ospf_backup_route broute;
            if (NULL == p_route->path[current].p_nexthop)
            {
                continue ;
            }
            
            for (i = 0 ; i < p_route->path[current].p_nexthop->count; i++)
            {
               vty_out(vty,"%-8s%-17s %-17s %-17s %-6d %-8d main%s",
                  path_type[p_route->path[current].type],
                  ospf_inet_ntoa(dststr, p_route->dest),
                  ospf_inet_ntoa(mskstr, p_route->mask),
                  ospf_inet_ntoa(nhopstr, p_route->path[current].p_nexthop->gateway[i].addr),
                  (int)p_route->path[current].cost, (int)p_route->path[current].p_nexthop->gateway[i].if_uint,OSPF_NEWLINE); 
            }
            broute.type = OSPF_ROUTE_NETWORK;
            broute.dest = p_route->dest;
            broute.mask = p_route->mask ;
            p_broute = ospf_lstlookup(&p_process->backup_route_table, &broute);
            if (p_broute)
            {
               vty_out(vty,"%-8s%-17s %-17s %-17s %-6d %-8d backup%s",
                  path_type[p_route->path[current].type],
                  ospf_inet_ntoa(dststr, p_broute->dest),
                  ospf_inet_ntoa(mskstr, p_broute->mask),
                  ospf_inet_ntoa(nhopstr, p_broute->path[backup_current].nexthop),
                  (int)p_broute->path[backup_current].cost, (int)p_broute->path[backup_current].ifunit,OSPF_NEWLINE); 
            }
        }
    }

    ospf_semgive();
}

#endif


void
ospf_display_process_packet_count(struct vty *vty, u_int instance_id, u_long ulIfIndex)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_if_stat stat;
    u_int *dest = NULL;
    u_int *src = NULL;
    u_int i = 0;
    u_int rx_count = 0 ;
    u_int tx_count = 0 ;      
    u_int8 lsa_type[OSPF_PACKET_ACK+1][15]={"","HELLO","DD","REQUEST","UPDATE","ACK"};

    ospf_semtake_forever();
    
    memset(&stat, 0, sizeof(stat));
    dest = (u_int *)&stat;
    
    for_each_ospf_process(p_process, p_next_process)
    {
        if(instance_id != 0)
        {
            if(p_process->process_id != instance_id)
            {
                continue;
            }
        }        
        vty_out(vty, "%s\t\t\tOSPF Process %-5d Packet Statistics%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
        
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if(ulIfIndex != 0)
            {
                if(p_if->ifnet_uint != ulIfIndex)
                {
                    continue;
                }
            }
            src = (u_int *)p_if->stat;
            for (i = 0 ; i < sizeof(struct ospf_if_stat)/4 ; i++)
            {
              dest[i] += src[i];
            }
            vty_out(vty, "%s\tp_if->ifnet_uint 0x%x:%s",OSPF_NEWLINE,p_if->ifnet_uint,OSPF_NEWLINE);
            vty_out(vty, "          %-10s%-10s%-12s%-12s%-10s%-10s%s","Rcv pkt","Send pkt", "Rcv bytes","Send bytes","Rcv Lsa","Send Lsa",OSPF_NEWLINE);
            for (i = 1 ; i < OSPF_PACKET_ACK+1; i++)
            {
               vty_out(vty, "%-10s%-10d%-10d%-12d%-12d%-10d%-10d%s",lsa_type[i],
                   (int)stat.rx_packet[i], (int)stat.tx_packet[i],(int)stat.rx_byte[i],(int)stat.tx_byte[i],
                   (int)stat.rx_lsainfo[i],(int)stat.tx_lsainfo[i],OSPF_NEWLINE);
               rx_count += stat.rx_packet[i];
               tx_count += stat.tx_packet[i];
            }
            vty_out(vty, "%s",OSPF_NEWLINE);
            vty_out(vty, "     Total Rxd:%-10d     Total Txd:%-10d%s",(int)rx_count,(int)tx_count,OSPF_NEWLINE);            
            memset(&stat,0,sizeof(struct ospf_if_stat));
            rx_count = 0;
            tx_count = 0;
            
            if(ulIfIndex != 0)
            {
                goto END;
            }
        }
    }
END:
    vty_out(vty,"%s",OSPF_NEWLINE);
    ospf_semgive();
    return ;
}


#ifdef OSPF_DCN
void
ospf_ospf_dcn_unnumber_interface_create(
                   uint32_t process_id,
                   uint32_t if_unit,
                   uint32_t area_id)
{
    tOSPF_AREA_INDEX index;
    tOSPF_IFINDEX if_index;
    uint32_t val = /*SNMP_CREATEANDGO*/4;
   
    index.area_id = area_id;
    index.process_id = process_id;         
    ospfAreaSetApi(&index, OSPF_AREA_STATUS, &val);

    if_index.process_id = process_id;
    if_index.ipaddr = 0;
    if_index.addrlessif = if_unit;
    ospfIfSetApi(&if_index, OSPF_IF_AREAID, &area_id);    

    return ;
}

#endif


void ospf_display_local_data(
				struct vty *vty, 
                 u_int instance_id, 
                 u_int uiIfIndx)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_nxtinstance = NULL;
    struct ospf_network *p_network = NULL;
    struct ospf_network *p_next = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_if search_if;
    struct ospf_if *p_searchif = NULL;
    u_long ulTmp = 0,ulNum = 0,ulTotalNum = 0;
    char addrstr[32] = {0};

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty,"   OSPF is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);    
        return ;                  
    }
    vty_out(vty,"%s ospf.debug = 0x%x,ospf.process_debug = 0x%x%s",OSPF_NEWLINE,ospf.debug,ospf.process_debug,OSPF_NEWLINE);

    vty_out(vty, "%s",OSPF_NEWLINE);
    for_each_ospf_process(p_process, p_nxtinstance)
    {   
        ulNum = 0;
        if((p_process->process_id != instance_id)&&(0 != instance_id))
        {
            continue;
        }
        memset(addrstr, 0, sizeof(addrstr));
        ospf_inet_ntoa(addrstr,p_process->router_id);
        vty_out(vty, " p_process=%p,process id:0x%x%s",p_process,(int)p_process->process_id,OSPF_NEWLINE);
        vty_out(vty, " process router_id:%s%s",addrstr,OSPF_NEWLINE);
		
        vty_out(vty, " process vrid:%d%s",p_process->vrid,OSPF_NEWLINE);
		vty_out(vty, "%s",OSPF_NEWLINE);
	    for_each_node(&p_process->network_table, p_network, p_next)
	    {
	        vty_out(vty, " p_network=%p, dest:0x%x%s",p_network,(int)p_network->dest,OSPF_NEWLINE);
	        vty_out(vty, " p_network mask:0x%x%s",(int)p_network->mask,OSPF_NEWLINE);
	        vty_out(vty, " p_network area_id:0x%x%s",(int)p_network->area_id,OSPF_NEWLINE);
 			vty_out(vty, "%s",OSPF_NEWLINE);
	    }
 		vty_out(vty, "%s",OSPF_NEWLINE);
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
    		if((p_if->ifnet_uint != uiIfIndx)&&(0 != uiIfIndx))
    		{
    			continue;
    		}
	        vty_out(vty, "\t p_if=%p, name=%s,ifnet_uint=0x%x,ifnet_index=0x%x%s",p_if,p_if->name,p_if->ifnet_uint,p_if->ifnet_index,OSPF_NEWLINE);
	        vty_out(vty, "\t p_if ulDcnflag=0x%x,uiOverlayflag=0x%x,addr=0x%x,mask=0x%x,maxlen = 0x%x%s",(int)p_if->ulDcnflag,p_if->uiOverlayflag,p_if->addr,p_if->mask,p_if->maxlen,OSPF_NEWLINE);
	        vty_out(vty, "\t p_if name=%s,nbr=0x%x,mtu=0x%x,cost=0x%x,dr=0x%x,bdr=0x%x%s"
				,p_if->name,p_if->nbr,p_if->mtu,p_if->cost[0],p_if->dr,p_if->bdr,OSPF_NEWLINE);
 	        vty_out(vty, "\t p_if hello_time=%d,rxmt=%d,dead=%d,poll_interval=%d,priority=%d,%s"
				,p_if->hello_interval,p_if->rxmt_interval,p_if->dead_interval,p_if->poll_interval,p_if->priority,OSPF_NEWLINE);
 	        vty_out(vty, "\t p_if type=%d,state=0x%x,authtype=0x%x,keylen=0x%x,md5id=0x%x,key=%s,%s"
				,p_if->type,p_if->state,p_if->authtype,p_if->keylen,p_if->md5id,p_if->key,OSPF_NEWLINE);
  	        vty_out(vty, "\t p_if flood_group=%d,tx_delay=0x%x,nbrchange=0x%x,passive=0x%x,mcast=0x%x,link_up=%x,%s"
				,p_if->flood_group,p_if->tx_delay,p_if->nbrchange,p_if->passive,p_if->mcast,p_if->link_up,OSPF_NEWLINE);
#ifdef HAVE_BFD
  	        vty_out(vty, "\t p_if te_instance=%d,te_cost=0x%x,te_group=0x%x,max_bd=0x%x,max_rsvdbd=0x%x,bfd_enable=%x,%s"
				,p_if->te_instance,p_if->te_cost,p_if->te_group,p_if->max_bd,p_if->max_rsvdbd,p_if->te_enable,p_if->bfd_enable,OSPF_NEWLINE);
#endif
	        vty_out(vty, "\t p_if hello_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
				p_if->hello_timer,p_if->hello_timer.node,p_if->hello_timer.active,
				p_if->hello_timer.func,*(u_int *)p_if->hello_timer.arg,p_if->hello_timer.context,OSPF_NEWLINE);
	        vty_out(vty, "\t p_if poll_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
				p_if->poll_timer,p_if->poll_timer.node,p_if->poll_timer.active,
				p_if->poll_timer.func,*(u_int *)p_if->poll_timer.arg,p_if->poll_timer.context,OSPF_NEWLINE);

		    vty_out(vty, "\t p_if wait_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
				p_if->wait_timer,p_if->wait_timer.node,p_if->wait_timer.active,
				p_if->wait_timer.func,*(u_int *)p_if->wait_timer.arg,p_if->wait_timer.context,OSPF_NEWLINE);
	        vty_out(vty, "\t p_if ack_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
				p_if->ack_timer,p_if->ack_timer.node,p_if->ack_timer.active,
				p_if->ack_timer.func,*(u_int *)p_if->ack_timer.arg,p_if->ack_timer.context,OSPF_NEWLINE);

		    vty_out(vty, "\t p_if flood_timer 0x%x: node=0x%x,active= %d,func=0x%x,arg=0x%x,context=0x%x .%s",
				p_if->flood_timer,p_if->flood_timer.node,p_if->flood_timer.active,
				p_if->flood_timer.func,*(u_int *)p_if->flood_timer.arg,p_if->flood_timer.context,OSPF_NEWLINE);
			
		    vty_out(vty, "\t p_if OSPF_MCAST_ALLSPF:\r\nmcast_jion_success %d,mcast_jion_error %d,mcast_drop_success %d,mcast_drop_error %d%s",
				p_if->stat->mcast_jion_success[OSPF_MCAST_ALLSPF],p_if->stat->mcast_jion_error[OSPF_MCAST_ALLSPF],
				p_if->stat->mcast_drop_success[OSPF_MCAST_ALLSPF],p_if->stat->mcast_drop_error[OSPF_MCAST_ALLSPF],OSPF_NEWLINE);
				
            vty_out(vty, "\t p_if OSPF_MCAST_ALLDR:\r\nmcast_jion_success %d,mcast_jion_error %d,mcast_drop_success %d,mcast_drop_error %d%s",
                p_if->stat->mcast_jion_success[OSPF_MCAST_ALLDR],p_if->stat->mcast_jion_error[OSPF_MCAST_ALLDR],
                p_if->stat->mcast_drop_success[OSPF_MCAST_ALLDR],p_if->stat->mcast_drop_error[OSPF_MCAST_ALLDR],OSPF_NEWLINE);
                
            vty_out(vty, "\t p_if OSPF_MCAST_ALLLOOP:\r\nmcast_jion_success %d,mcast_jion_error %d,mcast_drop_success %d,mcast_drop_error %d%s",
                p_if->stat->mcast_jion_success[OSPF_MCAST_ALLLOOP],p_if->stat->mcast_jion_error[OSPF_MCAST_ALLLOOP],
                p_if->stat->mcast_drop_success[OSPF_MCAST_ALLLOOP],p_if->stat->mcast_drop_error[OSPF_MCAST_ALLLOOP],OSPF_NEWLINE);
                    
		    search_if.p_process = p_process;
		    search_if.addr = p_if->addr;
		    search_if.ifnet_uint = p_if->ifnet_uint;
		    p_searchif = ospf_lstlookup(&ospf.nm.if_table,  &search_if);
			vty_out(vty, "\t ospf.nm.if_table=%p,p_process=%p,addr=0x%x,ifnet_uint=0x%x,p_searchif=%p%s",&ospf.nm.if_table,p_process,p_if->addr,p_if->ifnet_uint,p_searchif,OSPF_NEWLINE);
  			vty_out(vty, "%s",OSPF_NEWLINE);
            ulNum++;
            ulTotalNum++;
     	}
		vty_out(vty, "\t p_if count = %d%s",ulNum,OSPF_NEWLINE);
    }
    vty_out(vty, " Total count = %d%s",ulTotalNum,OSPF_NEWLINE);
    ospf_semgive();

    return ;
}


void
ospf_display_nbr_root(struct vty *vty, u_int count)
{
	int rc  = OK,ret = 0;
	u_int8 str[32];
	tOSPF_NBR_INDEX st_Index = {0},st_Next = {0};
	long value_l=0;
	struct ospf_nbr *p_nbr = NULL;
    struct ospf_process start_process;
    struct ospf_if search_if;
    struct ospf_nbr search_nbr;

	if (OK != ospf_semtake_timeout())
	{
		return ;
	}

	if(count == 0)
	{
	   count = OSPF_STAT_MAX_NUM;
	}
	
	vty_out(vty, " ospf.nm.nbr_table:	%s",OSPF_NEWLINE);
	for(rc = ospfNbrGetSelect(NULL,&st_Next); rc == OK; rc = ospfNbrGetSelect(&st_Index,&st_Next))
	{
#ifdef OSPF_DCN
		if((st_Next.process_id == 0)|| (st_Next.process_id== OSPF_DCN_PROCESS))
        {
			st_Index = st_Next;
			continue;
        }
#endif
		if(ospfGetApi(st_Next.process_id,OSPF_GBL_ADMIN,&value_l)!=OK)
		{	
			vty_out(vty," %%Failed to get ospf current state!%s",VTY_NEWLINE);
			return;
		}
		if(st_Next.process_id != st_Index.process_id)
		{
			vty_out(vty,"OSPF Process %d%s",st_Next.process_id,VTY_NEWLINE);
		}
		vty_out(vty," %-20s%-20s%-20s%-20s%-20s%-20s%s%s%s",
	    			"IpAddress","NeighborID","IfIpAddr","Root","NbrRoot","ProNbrRoot","Interface",VTY_NEWLINE); 

		start_process.process_id = ospf_nm_process_id(&st_Next);
	    search_if.p_process = &start_process;
	    search_nbr.p_if = &search_if;
	    search_nbr.addr = st_Next.ipaddr;
		p_nbr = ospf_lstlookup(&ospf.nm.nbr_table, &search_nbr);

		ospf_inet_ntoa(str, p_nbr->addr);
		vty_out(vty, " %-20s", str);//IpAddress
		memset(str, 0, sizeof(str));
		ospf_inet_ntoa(str, p_nbr->id);
		vty_out(vty, "%-20s", str);//NeighborID
		memset(str, 0, sizeof(str));
		ospf_inet_ntoa(str, p_nbr->p_if->addr);
		vty_out(vty, "%-20s", str);//IfIpAddr
		memset(str, 0, sizeof(str));
		ospf_inet_ntoa(str, ospf.nm.nbr_table.avl.avl_root);
		vty_out(vty, "%-20s", str);//Root
		memset(str, 0, sizeof(str));
		ospf_inet_ntoa(str, p_nbr->p_if->nbr_table.avl.avl_root);
		vty_out(vty, "%-20s", str);//NbrRoot
		memset(str, 0, sizeof(str));
		ospf_inet_ntoa(str, p_nbr->p_if->p_process->nbr_table.avl.avl_root);
		vty_out(vty, "%-20s", str);//ProNbrRoot
		vty_out(vty, "%s\n", p_nbr->p_if->name);//Interface
		memcpy(&st_Index,&st_Next,sizeof(tOSPF_NBR_INDEX));
		memset(&st_Next,0,sizeof(tOSPF_NBR_INDEX));
	}
	   
	ospf_semgive();

}


void ospf_display_task_msg(struct vty *vty, u_int uiProId, u_int uiIfIndex)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ; 
    char acIfName[64] = {0};
    
    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }
    vty_out(vty,"%s",OSPF_NEWLINE);
    vty_out(vty," master slave syn msg:%s",OSPF_NEWLINE);
    vty_out(vty,"SYN_STAT_WORK MSG cnt:%d old_time:%s new_time:%s%s",
        ospf.stat.syn_chg_msg_cnt[0],ospf.stat.syn_work_msg_time_old,ospf.stat.syn_work_msg_time,OSPF_NEWLINE);
    vty_out(vty,"SYN_STAT_BKP MSG cnt:%d old_time:%s new_time:%s%s",
        ospf.stat.syn_chg_msg_cnt[1],ospf.stat.syn_bkp_msg_time_old,ospf.stat.syn_bkp_msg_time,OSPF_NEWLINE);
    vty_out(vty,"SYN OTHER MSG cnt:%d old_time:%s new_time:%s%s",
        ospf.stat.syn_chg_msg_cnt[2],ospf.stat.syn_other_msg_time_old,ospf.stat.syn_other_msg_time,OSPF_NEWLINE);
    vty_out(vty,"SYN done MSG cnt:%d old_time:%s new_time:%s%s",
        ospf.stat.syn_done_msg_cnt,ospf.stat.syn_done_msg_time_old,ospf.stat.syn_done_msg_time,OSPF_NEWLINE);

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        if (uiIfIndex == 0)
        {
            vty_out(vty,"%s-----------Process %d msg-------------%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
        }
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if (p_if->stat)
            {
                if (uiIfIndex && (uiIfIndex != p_if->ifnet_uint))
                {
                    continue;
                }
                if (uiIfIndex != 0)
                {
                    vty_out(vty,"%s-----------Process %d msg-------------%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
                }
               
                if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
                {
                    continue;
                }
                vty_out(vty,"%s msg:%s",acIfName,OSPF_NEWLINE);
                vty_out(vty,"ldp msg:%s",OSPF_NEWLINE);
                vty_out(vty,"OSPF_LDP_ERR_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_ERR_MSG],p_if->stat->ldp_err_msg_time,OSPF_NEWLINE);
                vty_out(vty,"OSPF_LDP_INIT_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_INIT_MSG],p_if->stat->ldp_init_msg_time,OSPF_NEWLINE);
                vty_out(vty,"OSPF_LDP_UP_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_UP_MSG],p_if->stat->ldp_up_msg_time,OSPF_NEWLINE);
                vty_out(vty,"link msg:%s",OSPF_NEWLINE);
                vty_out(vty,"Link up %d %s%s",p_if->stat->linkup_msg_cnt,p_if->stat->linkup_msg_time,OSPF_NEWLINE);
                vty_out(vty,"Link down %d %s%s",p_if->stat->linkdown_msg_cnt,p_if->stat->linkdown_msg_time,OSPF_NEWLINE);
            }
        }
    }

    ospf_semgive();

}  


void ospf_task_msg_cnt_clear(u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ; 

    if (ospf_semtake_timeout() == ERR)
    {
        return  ;      
    }

    /*SYN_STAT_WORK*/
    ospf.stat.syn_chg_msg_cnt[0] = 0;
    memset(ospf.stat.syn_work_msg_time,0,sizeof(ospf.stat.syn_work_msg_time));
    memset(ospf.stat.syn_work_msg_time_old,0,sizeof(ospf.stat.syn_work_msg_time_old));
    /*SYN_STAT_BKP*/
    ospf.stat.syn_chg_msg_cnt[1] = 0;
    memset(ospf.stat.syn_bkp_msg_time,0,sizeof(ospf.stat.syn_bkp_msg_time));
    memset(ospf.stat.syn_bkp_msg_time_old,0,sizeof(ospf.stat.syn_bkp_msg_time_old));
    /*SYN_STAT_OTHER*/
    ospf.stat.syn_chg_msg_cnt[2] = 0;
    memset(ospf.stat.syn_other_msg_time,0,sizeof(ospf.stat.syn_other_msg_time));
    memset(ospf.stat.syn_other_msg_time_old,0,sizeof(ospf.stat.syn_other_msg_time_old));
    /*MSG_TYPE_SYN_NEGO_DONE_MSG*/
    ospf.stat.syn_done_msg_cnt = 0;
    memset(ospf.stat.syn_done_msg_time,0,sizeof(ospf.stat.syn_done_msg_time));
    memset(ospf.stat.syn_done_msg_time_old,0,sizeof(ospf.stat.syn_done_msg_time_old));

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if (p_if->stat)
            {
                /*ldp err msg*/
                p_if->stat->ldp_msg_cnt[OSPF_LDP_ERR_MSG] = 0;
                memset(p_if->stat->ldp_err_msg_time,0,sizeof(p_if->stat->ldp_err_msg_time));
                /*ldp init msg*/
                p_if->stat->ldp_msg_cnt[OSPF_LDP_INIT_MSG] = 0;
                memset(p_if->stat->ldp_init_msg_time,0,sizeof(p_if->stat->ldp_init_msg_time));
                /*ldp up msg*/
                p_if->stat->ldp_msg_cnt[OSPF_LDP_UP_MSG] = 0;
                memset(p_if->stat->ldp_up_msg_time,0,sizeof(p_if->stat->ldp_up_msg_time));
                /*link up msg*/
                p_if->stat->linkup_msg_cnt = 0;
                memset(p_if->stat->linkup_msg_time,0,sizeof(p_if->stat->linkup_msg_time));
                /*link down msg*/
                p_if->stat->linkdown_msg_cnt = 0;
                memset(p_if->stat->linkdown_msg_time,0,sizeof(p_if->stat->linkdown_msg_time));
            }
        }
    }

    ospf_semgive();
}


void ospf_display_iftable_cnt(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL ;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    char acIfName[64] = {0};
    
    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }
    vty_out(vty,"%sglobal if_table cnt %d%s",OSPF_NEWLINE,ospf_lstcnt(&ospf.nm.if_table),OSPF_NEWLINE);

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        vty_out(vty,"-----------Process %d-------------%s",p_process->process_id,OSPF_NEWLINE);
        
        vty_out(vty,"p_process if_table cnt %d:%s",ospf_lstcnt(&p_process->if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %-4s %-15s %-9s %-8s %-8s %-8s %-15s %-15s %s",
            "Interface","IfIndex","Link","IP Address","Type","State","Cost","Pri","DR","BDR",OSPF_NEWLINE);
        for_each_node(&p_process->if_table, p_if, p_next_if)
        {
            ospf_iftable_show(vty, p_if);
        }
        
        vty_out(vty,"p_process virtual_if_table cnt %d:%s",ospf_lstcnt(&p_process->virtual_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %-4s %-15s %-9s %-8s %-8s %-8s %-15s %-15s %s",
            "Interface","IfIndex","Link","IP Address","Type","State","Cost","Pri","DR","BDR",OSPF_NEWLINE);
        for_each_node(&p_process->virtual_if_table, p_if, p_next_if)
        {
            ospf_iftable_show(vty, p_if);
        }
        
        vty_out(vty,"p_process normal_if_table cnt %d:%s",ospf_lstcnt(&p_process->normal_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %-4s %-15s %-9s %-8s %-8s %-8s %-15s %-15s %s",
            "Interface","IfIndex","Link","IP Address","Type","State","Cost","Pri","DR","BDR",OSPF_NEWLINE);
        for_each_node(&p_process->normal_if_table, p_if, p_next_if)
        {
            ospf_iftable_show(vty, p_if);
        }
        
        vty_out(vty,"p_process shamlink_if_table cnt %d:%s",ospf_lstcnt(&p_process->shamlink_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %-4s %-15s %-9s %-8s %-8s %-8s %-15s %-15s %s",
            "Interface","IfIndex","Link","IP Address","Type","State","Cost","Pri","DR","BDR",OSPF_NEWLINE);
        for_each_node(&p_process->shamlink_if_table, p_if, p_next_if)
        {
            ospf_iftable_show(vty, p_if);
        }
        
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            vty_out(vty,"area id %d:%s",p_area->id,OSPF_NEWLINE);
            vty_out(vty,"p_area if_table cnt %d:%s",ospf_lstcnt(&p_area->if_table),OSPF_NEWLINE);
            vty_out(vty,"%-28s %-11s %-4s %-15s %-9s %-8s %-8s %-8s %-15s %-15s %s",
                "Interface","IfIndex","Link","IP Address","Type","State","Cost","Pri","DR","BDR",OSPF_NEWLINE);
            for_each_node(&p_area->if_table, p_if, p_next_if)
            {
                ospf_iftable_show(vty, p_if);
            }
        }
        vty_out(vty,"%s",OSPF_NEWLINE);
    }

    ospf_semgive();
	
	return;
}


void ospf_display_iftable_pstIf(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL ;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    char acIfName[64] = {0};
    
    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }
    vty_out(vty,"%sglobal if_table cnt %d%s",OSPF_NEWLINE,ospf_lstcnt(&ospf.nm.if_table),OSPF_NEWLINE);

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        vty_out(vty,"-----------Process %d-------------%s",p_process->process_id,OSPF_NEWLINE);
        
        vty_out(vty,"p_process if_table cnt %d:%s",ospf_lstcnt(&p_process->if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %s","Interface","PstIf",OSPF_NEWLINE);
        for_each_node(&p_process->if_table, p_if, p_next_if)
        {
            if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
            {
                continue;
            }
            vty_out(vty,"%-28s 0x%-11x %s",acIfName,p_if,OSPF_NEWLINE);
        }
        
        vty_out(vty,"p_process virtual_if_table cnt %d:%s",ospf_lstcnt(&p_process->virtual_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %s","Interface","PstIf",OSPF_NEWLINE);
        for_each_node(&p_process->virtual_if_table, p_if, p_next_if)
        {
            if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
            {
                continue;
            }
            vty_out(vty,"%-28s 0x%-11x %s",acIfName,p_if,OSPF_NEWLINE);
        }
        
        vty_out(vty,"p_process normal_if_table cnt %d:%s",ospf_lstcnt(&p_process->normal_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %s","Interface","PstIf",OSPF_NEWLINE);
        for_each_node(&p_process->normal_if_table, p_if, p_next_if)
        {
            if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
            {
                continue;
            }
            vty_out(vty,"%-28s 0x%-11x %s",acIfName,p_if,OSPF_NEWLINE);
        }
        
        vty_out(vty,"p_process shamlink_if_table cnt %d:%s",ospf_lstcnt(&p_process->shamlink_if_table),OSPF_NEWLINE);
        vty_out(vty,"%-28s %-11s %s","Interface","PstIf",OSPF_NEWLINE);
        for_each_node(&p_process->shamlink_if_table, p_if, p_next_if)
        {
            if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
            {
                continue;
            }
            vty_out(vty,"%-28s 0x%-11x %s",acIfName,p_if,OSPF_NEWLINE);
        }
        
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            vty_out(vty,"area id %d:%s",p_area->id,OSPF_NEWLINE);
            vty_out(vty,"p_area if_table cnt %d:%s",ospf_lstcnt(&p_area->if_table),OSPF_NEWLINE);
            vty_out(vty,"%-28s %-11s %s","Interface","PstIf",OSPF_NEWLINE);
            for_each_node(&p_area->if_table, p_if, p_next_if)
            {
                if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
                {
                    continue;
                }
                vty_out(vty,"%-28s 0x%-11x %s",acIfName,p_if,OSPF_NEWLINE);
            }
        }
        vty_out(vty,"%s",OSPF_NEWLINE);
    }

    ospf_semgive();
	
	return;
}


void ospf_display_pstlsa(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_ntable = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nlsa = NULL;
    u_int8 linkStr[20] = {0};
    int iCount = 0; 
    
    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }
    vty_out(vty,"%sglobal if_table cnt %d%s",OSPF_NEWLINE,ospf_lstcnt(&ospf.nm.if_table),OSPF_NEWLINE);

    for_each_ospf_process(p_process, p_next_process)
    {
        iCount = 0;
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        vty_out(vty,"-----------Process %d-------------%s",p_process->process_id,OSPF_NEWLINE);

        for_each_node(&p_process->lstable_table, p_table, p_ntable)
        {
            for_each_node(&p_table->list, p_lsa, p_nlsa)
            {
                ospf_inet_ntoa(linkStr, (int)ntohl(p_lsa->lshdr->id));
                vty_out(vty,"LinkId: %-15s pstLsa: 0x%-11x %s",linkStr,p_lsa,OSPF_NEWLINE);
                iCount++;
            }
        }
        vty_out(vty,"lsa cnt : %d %s",iCount,OSPF_NEWLINE);
    }
        
    ospf_semgive();
	
	return;
}


void ospf_iftable_show(struct vty *vty, struct ospf_if *pstIf)
{
    char acIfName[64] = {0};	
    char acAddr[32] = {0};
	char acDr[32] = {0};
	char acBdr[32] = {0};
    u_char *lstypestr[] = { "Unkown", 
                           "Broadcast",
                           "Nbma",
                           "P2p", 
                           "Vlink",
                           "P2mp",
                           "Shamlink"};

    u_char *lsStatstr[] = { "Unkown", 
        				   "Down",
        				   "Loopback",
        				   "Waiting", 
        				   "P-2-P",
        				   "DR",
        				   "BDR",
        				   "DROther"};

    if(pstIf == NULL)
    {
        return;
    }
    
    if(if_name_get_by_index(pstIf->ifnet_uint, acIfName) == ERROR)
    {
        return;
    }
    ospf_inet_ntoa(acAddr, pstIf->addr);
    ospf_inet_ntoa(acDr, pstIf->dr);
    ospf_inet_ntoa(acBdr, pstIf->bdr);
    vty_out(vty,"%-28s 0x%-9x %-4s %-15s %-9s %-8s %-8d %-8d %-15s %-15s %s",
    	acIfName,pstIf->ifnet_uint,pstIf->link_up ? "UP" : "DOWN",acAddr,lstypestr[pstIf->type],lsStatstr[pstIf->state],
    	pstIf->cost[0],pstIf->priority,acDr,acBdr,OSPF_NEWLINE);

	return;
}

void ospf_msg_choose_show(struct vty *vty, u_int uiProId, u_int uiIfIndex, u_int uiMsgType)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ; 
    char acIfName[64] = {0};

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return;      
    }
    
    if(uiMsgType == OSPF_SYN_MSG)
    {
        vty_out(vty,"%s",OSPF_NEWLINE);
        vty_out(vty," master slave syn msg:%s",OSPF_NEWLINE);
        vty_out(vty,"SYN_STAT_WORK MSG cnt:%d old_time:%s new_time:%s%s",
            ospf.stat.syn_chg_msg_cnt[0],ospf.stat.syn_work_msg_time_old,ospf.stat.syn_work_msg_time,OSPF_NEWLINE);
        vty_out(vty,"SYN_STAT_BKP MSG cnt:%d old_time:%s new_time:%s%s",
            ospf.stat.syn_chg_msg_cnt[1],ospf.stat.syn_bkp_msg_time_old,ospf.stat.syn_bkp_msg_time,OSPF_NEWLINE);
        vty_out(vty,"OTHER MSG cnt:%d old_time:%s new_time:%s%s",
            ospf.stat.syn_chg_msg_cnt[2],ospf.stat.syn_other_msg_time_old,ospf.stat.syn_other_msg_time,OSPF_NEWLINE);
        vty_out(vty,"done MSG cnt:%d old_time:%s new_time:%s%s",
            ospf.stat.syn_done_msg_cnt,ospf.stat.syn_done_msg_time_old,ospf.stat.syn_done_msg_time,OSPF_NEWLINE);
        goto END;    
    }
    
    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        if (uiIfIndex == 0)
        {
            vty_out(vty,"%s-----------Process %d msg-------------%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
        }
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if (p_if->stat)
            {
                if (uiIfIndex && (uiIfIndex != p_if->ifnet_uint))
                {
                    continue;
                }
                if (uiIfIndex != 0)
                {
                    vty_out(vty,"%s-----------Process %d msg-------------%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
                }
                
                if(if_name_get_by_index(p_if->ifnet_uint, acIfName) == ERROR)
                {
                    continue;
                }
                switch(uiMsgType)
                {
                    case OSPF_LDP_MSG :
                    {
                        vty_out(vty,"%s msg:%s",acIfName,OSPF_NEWLINE);
                        vty_out(vty,"ldp msg:%s",OSPF_NEWLINE);
                        vty_out(vty,"OSPF_LDP_ERR_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_ERR_MSG],p_if->stat->ldp_err_msg_time,OSPF_NEWLINE);
                        vty_out(vty,"OSPF_LDP_INIT_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_INIT_MSG],p_if->stat->ldp_init_msg_time,OSPF_NEWLINE);
                        vty_out(vty,"OSPF_LDP_UP_MSG %d %s%s",p_if->stat->ldp_msg_cnt[OSPF_LDP_UP_MSG],p_if->stat->ldp_up_msg_time,OSPF_NEWLINE);
                        break;
                    }
                    case OSPF_IFLINK_MSG :
                    {
                        vty_out(vty,"%s msg:%s",acIfName,OSPF_NEWLINE);
                        vty_out(vty,"link msg:%s",OSPF_NEWLINE);
                        vty_out(vty,"Link up %d %s%s",p_if->stat->linkup_msg_cnt,p_if->stat->linkup_msg_time,OSPF_NEWLINE);
                        vty_out(vty,"Link down %d %s%s",p_if->stat->linkdown_msg_cnt,p_if->stat->linkdown_msg_time,OSPF_NEWLINE);
                        break;
                    }
                    case OSPF_BFD_MSG :
                    {                       
                        vty_out(vty,"%s msg:%s",acIfName,OSPF_NEWLINE);
                        vty_out(vty,"bfd send msg:%s",OSPF_NEWLINE);
                        vty_out(vty,"OSPF_BFD_BIND_MSG %d %s",p_if->stat->bfd_send_msg_cnt[OSPF_BFD_BIND_MSG],OSPF_NEWLINE);
                        vty_out(vty,"OSPF_BFD_UNBIND_MSG %d %s",p_if->stat->bfd_send_msg_cnt[OSPF_BFD_UNBIND_MSG],OSPF_NEWLINE);
                        vty_out(vty,"OSPF_BFD_MOD_MSG %d %s",p_if->stat->bfd_send_msg_cnt[OSPF_BFD_MOD_MSG],OSPF_NEWLINE);
                        vty_out(vty,"bfd recv msg:%s",OSPF_NEWLINE);
                        vty_out(vty,"OSPF_BFD_DOWN_MSG %d %s",p_if->stat->bfd_recv_msg_cnt,OSPF_NEWLINE);
                        break;
                    }
                    default :
                    {
                        break;
                    }
                }
            }
        }
    }
END:    
    ospf_semgive();

    return;
}

void ospf_msg_cnt_choose_clear(u_int uiProId, u_int uiMsgType)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL ; 

    if (ospf_semtake_timeout() == ERR)
    {
        return;      
    }

    if(uiMsgType == OSPF_SYN_MSG)
    {
        /*SYN_STAT_WORK*/
        ospf.stat.syn_chg_msg_cnt[0] = 0;
        memset(ospf.stat.syn_work_msg_time,0,sizeof(ospf.stat.syn_work_msg_time));
        memset(ospf.stat.syn_work_msg_time_old,0,sizeof(ospf.stat.syn_work_msg_time_old));
        /*SYN_STAT_BKP*/
        ospf.stat.syn_chg_msg_cnt[1] = 0;
        memset(ospf.stat.syn_bkp_msg_time,0,sizeof(ospf.stat.syn_bkp_msg_time));
        memset(ospf.stat.syn_bkp_msg_time_old,0,sizeof(ospf.stat.syn_bkp_msg_time_old));
        /*SYN_STAT_OTHER*/
        ospf.stat.syn_chg_msg_cnt[2] = 0;
        memset(ospf.stat.syn_other_msg_time,0,sizeof(ospf.stat.syn_other_msg_time));
        memset(ospf.stat.syn_other_msg_time_old,0,sizeof(ospf.stat.syn_other_msg_time_old));
        /*MSG_TYPE_SYN_NEGO_DONE_MSG*/
        ospf.stat.syn_done_msg_cnt = 0;
        memset(ospf.stat.syn_done_msg_time,0,sizeof(ospf.stat.syn_done_msg_time));
        memset(ospf.stat.syn_done_msg_time_old,0,sizeof(ospf.stat.syn_done_msg_time_old));
        goto END;
    }
    
    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if (p_if->stat)
            {
                switch(uiMsgType)
                {
                    case OSPF_LDP_MSG :
                    {
                        /*ldp err msg*/
                        p_if->stat->ldp_msg_cnt[OSPF_LDP_ERR_MSG] = 0;
                        memset(p_if->stat->ldp_err_msg_time,0,sizeof(p_if->stat->ldp_err_msg_time));
                        /*ldp init msg*/
                        p_if->stat->ldp_msg_cnt[OSPF_LDP_INIT_MSG] = 0;
                        memset(p_if->stat->ldp_init_msg_time,0,sizeof(p_if->stat->ldp_init_msg_time));
                        /*ldp up msg*/
                        p_if->stat->ldp_msg_cnt[OSPF_LDP_UP_MSG] = 0;
                        memset(p_if->stat->ldp_up_msg_time,0,sizeof(p_if->stat->ldp_up_msg_time));
                        break;
                    }
                    case OSPF_BFD_MSG :
                    {
                        p_if->stat->bfd_recv_msg_cnt = 0;
                        p_if->stat->bfd_send_msg_cnt[OSPF_BFD_BIND_MSG] = 0;
                        p_if->stat->bfd_send_msg_cnt[OSPF_BFD_UNBIND_MSG] = 0;
                        p_if->stat->bfd_send_msg_cnt[OSPF_BFD_MOD_MSG] = 0;
                        break;
                    }
                    case OSPF_IFLINK_MSG :
                    {
                        /*link up msg*/
                        p_if->stat->linkup_msg_cnt = 0;
                        memset(p_if->stat->linkup_msg_time,0,sizeof(p_if->stat->linkup_msg_time));
                        /*link down msg*/
                        p_if->stat->linkdown_msg_cnt = 0;
                        memset(p_if->stat->linkdown_msg_time,0,sizeof(p_if->stat->linkdown_msg_time));
                        break;
                    }
                    default :
                    {
                        break;
                    }
                }


            }
        }
    }
    
END:

    ospf_semgive();

    return;
}

void ospf_display_dcn_cnt(struct vty *vty, u_int uiProId)
{
    struct ospf_process *pstProcess = NULL;
    struct ospf_process *pstNextProcess = NULL;
    
    if (ospf_semtake_timeout() == ERR)
    {
        return;      
    }

    for_each_ospf_process(pstProcess,pstNextProcess)
    {
        if(pstProcess->process_id != uiProId)
        {
            continue;
        }
        vty_out(vty,"------------------process id: %d---------------%s", pstProcess->process_id, VTY_NEWLINE);
        vty_out(vty,"uiDcnIfCnt=%-3d   uiDcnIfErrCnt=%d %s",pstProcess->uiDcnIfCnt, pstProcess->uiDcnIfErrCnt, VTY_NEWLINE);
        vty_out(vty,"uiDcnLsaCnt=%-3d  uiDcnLsaErrCnt=%d %s",pstProcess->uiDcnLsaCnt, pstProcess->uiDcnLsaErrCnt, VTY_NEWLINE);
        vty_out(vty,"uiDcnNeidCnt=%-3d uiDcnNeidErrCnt=%d %s",pstProcess->uiDcnNeidCnt, pstProcess->uiDcnNeidErrCnt, VTY_NEWLINE);
    }
    
    ospf_semgive();
    return;
}

void ospf_dcn_cnt_choose_clear(struct vty *vty, u_int uiProId, u_int uiType)
{
    struct ospf_process *pstProcess = NULL;
    struct ospf_process *pstNextProcess = NULL;
    
    if (ospf_semtake_timeout() == ERR)
    {
        return;      
    }
    
    for_each_ospf_process(pstProcess,pstNextProcess)
    {
        if(pstProcess->process_id != uiProId)
        {
            continue;
        }
        switch(uiType)
        {
            case OSPF_DCN_IF_CNT :
                {
                    pstProcess->uiDcnIfCnt = 0;
                    break;
                }
            case OSPF_DCN_IF_ERR_CNT :
                {
                    pstProcess->uiDcnIfErrCnt = 0;
                    break;
                }
            case OSPF_DCN_LSA_CNT :
                {
                    pstProcess->uiDcnLsaCnt = 0;
                    break;
                }
            case OSPF_DCN_LSA_ERR_CNT :
                {
                    pstProcess->uiDcnLsaErrCnt = 0;
                    break;
                }
            case OSPF_DCN_NEID_CNT :
                {
                    pstProcess->uiDcnNeidCnt = 0;
                    break;
                }
            case OSPF_DCN_NEID_ERR_CNT :
                {
                    pstProcess->uiDcnNeidErrCnt = 0;
                    break;
                }
            default :
                {
                    break;
                }
        }
    }
    
    ospf_semgive();
    return;
}


void ospf_display_rttable_cnt(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL ;
    struct ospf_route *pstRoute = NULL;
    struct ospf_route *pstNextRoute = NULL;   
    u_int8 deststr[20];

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        vty_out(vty,"%s-----------Process %d-------------%s",OSPF_NEWLINE,p_process->process_id,OSPF_NEWLINE);
        vty_out(vty,"p_process route_table cnt:%d%s",ospf_lstcnt(&p_process->route_table),OSPF_NEWLINE);
        vty_out(vty,"%-10s%-19s%s","pstRoute","Destination",OSPF_NEWLINE);
        for_each_node(&p_process->route_table, pstRoute, pstNextRoute)
        {
            memset(deststr, 0, sizeof(deststr));
            ospf_inet_ntoa(deststr, pstRoute->dest);
            vty_out(vty,"0x%-10x%-19s%s", pstRoute, deststr, OSPF_NEWLINE);
        }
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            vty_out(vty,"area id %d%s",p_area->id,OSPF_NEWLINE);
            vty_out(vty,"p_area abr_table cnt:%d abr_count:%d%s",ospf_lstcnt(&p_area->abr_table),p_area->abr_count,OSPF_NEWLINE);
            vty_out(vty,"p_area asbr_table cnt:%d asbr_count:%d%s",ospf_lstcnt(&p_area->asbr_table),p_area->asbr_count,OSPF_NEWLINE);
        }
    }

    ospf_semgive();
}


void ospf_display_rtchg_cnt(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }

    vty_out(vty,"route add:%d del:%d%s",ospf.stat.rt_cnt[1],ospf.stat.rt_cnt[0],OSPF_NEWLINE);
    vty_out(vty,"chg prefrence route:%d%s",ospf.stat.rt_pre_cnt,OSPF_NEWLINE);
    vty_out(vty,"slave route add:%d del:%d%s",ospf.stat.slaver_rt_cnt[1],ospf.stat.slaver_rt_cnt[0],OSPF_NEWLINE);

    ospf_semgive();
}


void ospf_clear_rtchg_cnt(struct vty *vty, u_int uiProId)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;

    if (ospf_semtake_timeout() == ERR)
    {
        vty_out(vty," OSPF  is busy now,call this command later%s%s",OSPF_NEWLINE,OSPF_NEWLINE);
        return  ;      
    }

    for_each_ospf_process(p_process, p_next_process)
    {
        if (uiProId && (uiProId != p_process->process_id))
        {
            continue;
        }
        ospf.stat.rt_cnt[0] = 0;
        ospf.stat.rt_cnt[1] = 0;
        ospf.stat.slaver_rt_cnt[0] = 0;
        ospf.stat.slaver_rt_cnt[1] = 0;
        ospf.stat.rt_pre_cnt = 0;
    }

    ospf_semgive();
}




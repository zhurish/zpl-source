#include "bgp4_api.h"
#ifdef NEW_BGP_WANTED
#include "bgp4com.h"


/*format output of an address*/
u_char * 
bgp4_printf_addr(
       tBGP4_ADDR *p, 
       u_char *str)
{
    switch (p->afi){
        case AF_INET:
        case BGP4_PF_IPUCAST:
             inet_ntoa_1(str, ntohl(bgp_ip4(p->ip)));
             break;
             
        case BGP4_PF_IP6UCAST:
        case AF_INET6:
        case BGP4_PF_IP6LABEL:
             inet_ntop(AF_INET6, p->ip, str, 64);
             break;
             
        case BGP4_PF_IPVPN:
             inet_ntoa_1(str, ntohl(bgp_ip4(p->ip + BGP4_VPN_RD_LEN)));
             break;
             
        case BGP4_PF_IP6VPN:
             inet_ntop(AF_INET6, p->ip + BGP4_VPN_RD_LEN, str, 64);
             break;
#ifdef BGP_VPLS_WANTED
        case BGP4_PF_L2VPLS:
			 {
			     tBGP4_VPLS_ADDR vpls;
				 u_char rdstr[64];
                 bgp4_print_rd(p->ip, rdstr);
                 bgp4_vpls_addr_get(p, &vpls);
                 sprintf(str, "vpls %s-%d-%d",rdstr, vpls.ve_id, vpls.label_base);             
        	 }
             break;
#endif             
        default:
             sprintf(str, "unkown");
             break;
    }
    return str;
}

/*log function for bgp*/
u_char *
bgp4_printf_peer(
    tBGP4_PEER *p_peer,
    u_char *str)
{
    return bgp4_printf_addr(&p_peer->ip, str);
}

/*log function for route*/
u_char *
bgp4_printf_route(
     tBGP4_ROUTE *p_route, 
     u_char * ostr)
{
    u_char str[64] = {0};
    
    bgp4_printf_addr(&p_route->dest, str);
    if (p_route->dest.afi == BGP4_PF_L2VPLS)
    {
        sprintf(ostr, "%s", str);
    }
    else
    {
        sprintf(ostr, "%s/%d", str, p_route->dest.prefixlen);
    }
    return ostr;
}

u_char *
bgp4_print_rd(
         u_char *p_ecom,
         u_char *p_str)
{
   tBGP4_ADDR ip;
   u_short type;
   u_int lval;
   u_short sval;
   u_char addstr[32];
   
   memcpy(&type, p_ecom, 2);
   type = ntohs(type);
   p_str[0] = '\0';
   switch (type){
       case 0:
            bgp4_get_2bytes(sval, (p_ecom + 2));
            bgp4_get_4bytes(lval,(p_ecom + 4));                   
            sprintf(p_str, "%d:%d", sval, lval); 
            break;
            
      case 1:
            bgp4_get_2bytes(sval, (p_ecom + 6));
            ip.afi = AF_INET;
            memcpy(&ip.ip, (p_ecom + 2), 4);
            sprintf(p_str, "%s:%d", bgp4_printf_addr(&ip, addstr), sval);             
            break;
            
      case 2 :   
            bgp4_get_2bytes(sval, (p_ecom + 6));
            bgp4_get_4bytes(lval,(p_ecom + 2));                                           
            sprintf(p_str, "%d:%d", lval, sval);
            break ;  
            
      default:
            break;
   };
   return p_str;
}

u_char * 
bgp4_printf_vpn_rd(
     tBGP4_ROUTE *p_route,
     u_char *p_str)
{     
    p_str[0] = '\0';
    
    if ((p_route->dest.afi != BGP4_PF_IPVPN)
        && (p_route->dest.afi != BGP4_PF_IP6VPN))
    {
        return p_str;
    }
    return bgp4_print_rd(p_route->dest.ip, p_str);
}

short 
bgp4_logx(
   u_int flag, 
   const char *format, ...)
{
    if ((gbgp4.debug & flag) == 0)
    {
        return -1;
    }
   {
        va_list args;
        int  len;
        int pos = 0 ;
        u_char outbuf[256];
     #ifdef WIN32
        u_char tmiestr[64];
     
        log_time_print(tmiestr);
        pos += sprintf(outbuf,"\n\r%s [BGP4]:", tmiestr);
     #else
        pos += sprintf(outbuf,"[BGP4]:");
     #endif
        va_start(args, format);
        
        len = vsprintf (outbuf + pos, format, args);  
        if (len < 0)
        {
           va_end(args);
           return -1;
        }
        va_end (args);
        outbuf[pos + len] = '\0';
       #if 0
        vty_log(outbuf, len + pos);
       #else
        if(gbgp4.debug & BGP_DEBUG_SYSLOG)
        {
            zlog_debug(MTYPE_BGP,"%s",outbuf);
        }
        else
        {
            printf("%s\r\n", outbuf);
        }
       #endif
        return (len + pos);
     }
}

/*display notify code and subcode*/
void 
bgp4_printf_notify(
       u_char code,
       u_char scode,
       u_char *str,
       u_char *sstr)
{
    switch (code){
        case BGP4_MSG_HDR_ERR :
             sprintf(str, "msg header error");

             switch (scode){
                 case BGP4_NO_SUBCODE:
                      sprintf(sstr, "none");
                      break;
                      
                 case BGP4_CONN_NOT_SYNC:
                      sprintf(sstr, "connection not synchronized");
                      break;
     
                 case BGP4_BAD_MSG_LEN:
                      sprintf(sstr, "bad msg length");
                      break;
                      
                 case BGP4_BAD_MSG_TYPE:
                      sprintf(sstr, "bad msg type");
                      break;
                      
                 default:
                      sprintf(sstr, "unrecognized");
                      break;
             }
             break;
             
        case BGP4_OPEN_MSG_ERR:
             sprintf(str, "open msg error");
             sprintf(sstr, "unrecognized");

             switch (scode){
                 case BGP4_NO_SUBCODE:
                      sprintf(sstr, "none");
                      break;
                      
                 case BGP4_UNSUPPORTED_VER_NO:
                      sprintf(sstr, "unsupported bgp version");
                      break;
                      
                 case BGP4_AS_UNACCEPTABLE:
                      sprintf(sstr, "unacceptable as num");
                      break;
                      
                 case BGP4_BGPID_INCORRECT:
                      sprintf(sstr, "incorrect bgp id");
                      break;
                      
                 case BGP4_OPT_PARM_UNRECOGNIZED:
                      sprintf(sstr, "unrecognized optional params");
                      break;
                      
                 case BGP4_AUTH_PROC_FAILED:
                      sprintf(sstr, "authentication failed");
                      break;
                      
                 case BGP4_HOLD_TMR_UNACCEPTABLE:
                      sprintf(sstr, "unacceptable hold time");
                      break;
                      
                 case BGP4_UNSUPPORTED_CAPABILITY:
                      sprintf(sstr, "Unsupported Capability");
                      break;
                      
                 default:
                      break;
             }
             break ;
             
        case BGP4_UPDATE_MSG_ERR:
             sprintf(str, "update msg error");

             switch (scode){
                 case BGP4_NO_SUBCODE:
                      sprintf(sstr, "none");
                      break;
                      
                 case BGP4_MALFORMED_ATTR_LIST:
                      sprintf(sstr, "malformed attribute list");
                      break;
                      
                 case BGP4_UNRECOGNISED_WELLKNOWN_ATTR:
                      sprintf(sstr, "unrecognized wellknown attribute");
                      break;
                      
                 case BGP4_MISSING_WELLKNOWN_ATTR:
                      sprintf(sstr, "missing wellknown attribute");
                      break;
                      
                 case BGP4_ATTR_FLAG_ERR:
                      sprintf(sstr, "bad attribute flag");
                      break;
                      
                 case BGP4_ATTR_LEN_ERR:
                      sprintf(sstr, "bad attribute length");
                      break;
                      
                 case BGP4_INVALID_ORIGIN:
                      sprintf(sstr, "invalid origin");
                      break;
                      
                 case BGP4_AS_ROUTING_LOOP:
                      sprintf(sstr, "as routing loop");
                      break;
                      
                 case BGP4_INVALID_NEXTHOP:
                      sprintf(sstr, "invalid nexthop");
                      break;
                      
                 case BGP4_OPTIONAL_ATTR_ERR:
                      sprintf(sstr, "bad optional attribute");
                      break;
                      
                 case BGP4_INVALID_NLRI:
                      sprintf(sstr, "invalid nlri");
                      break;
                      
                 case BGP4_MALFORMED_AS_PATH:
                      sprintf(sstr, "malformed as path");
                      break;
                      
                 default:
                      sprintf(sstr, "unrecoginzed");
                      break;
             }
             break;
             
        case BGP4_HOLD_TMR_EXPIRED_ERR:
             sprintf(str, "hold Timer expired");
             if (scode == BGP4_NO_SUBCODE)
             {
                 sprintf(sstr, "none");
             }
             else
             {
                 sprintf(sstr, "unrecoginzed");
             }
             break;
            
        case BGP4_FSM_ERR:
             sprintf(str,"FSM err");
             if (scode == BGP4_NO_SUBCODE)
             {
                 sprintf(sstr, "none");
             }
             else
             {
                 sprintf(sstr, "unrecoginzed");
             }        
             break;
            
        case BGP4_CEASE:
             sprintf(str, "cease");
             if (scode == BGP4_NO_SUBCODE)
             {
                 sprintf(sstr, "none");
             }
             else
             {
                 switch (scode){
                     case BGP4_OVER_MAX_PREFIX:
                          sprintf(sstr, "prefix limit exceed");
                          break;
                     case BGP4_ADMINISTRATIVE_SHUTDOWN:
                          sprintf(sstr, "admin-down");
                          break;
                     case BGP4_PEER_NOT_CONFIGURED:
                          sprintf(sstr, "deleted");
                          break;
                     case BGP4_ADMINISTRATIVE_RESET:
                          sprintf(sstr, "admin-reset");
                          break;
                     case BGP4_CONNECTION_REJECTED:
                          sprintf(sstr, "rejected");
                          break;
                     case BGP4_OTHER_CONFIGURATION_CHANGE:
                          sprintf(sstr, "config change");
                          break;
                     case BGP4_CONNECTION_COLLISION_RESOLUTION:
                          sprintf(sstr, "collision");
                          break;
                     default:
                          sprintf(sstr, "unrecoginzed");
                          break;
                 }
             }
             break;
            
        case BGP4_NOTIFY_MSG_ERR:
             sprintf(str, "notification");
             if (scode == BGP4_NO_SUBCODE)
             {
                 sprintf(sstr, "none");
             }
             else
             {
                 sprintf(sstr, "unrecognized");
             }
             break;
             
        default:
             sprintf(str, "unrecognized");
             sprintf(sstr, "unrecognized");
             break;
    }
    return;
}

void 
bgp4_debug_packet(
        u_char *p_msg,
        u_short len)
{
    u_char marker[16]; 
    u_short pkt_len; 
    u_char type; 
    u_char *p_buf = p_msg; 

    if ((p_msg == NULL) || (len < 19))
    {
        return;
    }

    bgp4_log(1, "Message parsing");

    memset(marker, 0xff, sizeof(marker));
    if (memcmp(p_buf ,marker, 16) != 0)
    {
        bgp4_log(1, "BGP4 Markers Invalid");
        return;
    }
    
    p_buf += 16;
    /*
    total length
    */
    
    bgp4_get_2bytes(pkt_len, p_buf);
    if (pkt_len != len)
    {
        bgp4_log(1, "packet length %d is not correct", pkt_len);
        return;
    }
    
    p_buf += 2;
    /*
    msg type
    */    
    type = *p_buf;
    p_buf++;
    len -= 19;
    switch(type){
        case 1 :/*Open*/
            bgp4_debug_open_msg(p_buf, len);
            break;
            
        case 2 :/*Update*/
            bgp4_debug_update_msg(p_buf, len);
            break;
            
        case 3:/*notify*/
            bgp4_debug_notify_msg(p_buf, len);        
            break;
            
        case 4 :/*keep alive*/
            bgp4_log(1, " keepalive");
            break;
            
        case 5 :/*route refresh*/
            bgp4_debug_rtrefresh_msg(p_buf, len);
            break;
            
        default :
            bgp4_log(1, " unkown,value is %d", type); 
            break;
    }
    return;
}

u_short 
bgp4_debug_orf_capability(
        u_char *p_buf,
        u_char caplen,
        short len)
{
    short caplen2 = caplen;
    u_short afi;
    u_char safi;
    u_char count;
    u_short i; 
    u_char type;
    u_char flag;
    
    /*rest length must >= cap len*/ 
    if (len < caplen) 
    {
        return 0;
    }
    /*Dump Orf Cap entry*/  
    while (caplen2 >= (5 + 2*(*(p_buf+4))))
    {
        bgp4_get_2bytes(afi,p_buf);
        p_buf += 3;
        
        safi = *p_buf;
        p_buf ++;
        
        bgp4_log(1,"    afi/safi %d/%d",afi,safi);    
        count = *p_buf;
        p_buf ++;
        bgp4_log(1,"    count %d",count);        
        for (i = 0 ;i < count ;i ++)
        {
            type = *p_buf;
            p_buf ++;
            
            flag = *p_buf;
            p_buf ++;
            bgp4_log(1,"    orf type:%s,flag %x",
                (type == BGP4_ORF_TYPE_IPPREFIX) ? "ip Prefix" : "unknown", flag);            
        }
        caplen2 -= (5 + 2*(*(p_buf+4)));
    }
    return 1;  
}

void 
bgp4_debug_open_msg(
        u_char *p_msg,
        u_short len)
{
    u_char ver ; 
    u_short roffset = 0 ;
    u_short as;
    u_short holdtime ;
    u_char total_optlen ; 
    u_char opt_rlen = 0;
    u_char opt_type ;
    u_char opt_len ;
    u_char cap_code;
    u_char cap_len;
    u_short afi;
    u_char safi ;
    u_char *p_tmpbuf ;
    u_int long_as;
    short tmp_optlen ;
    u_short rtime;
    u_char afflag;
    u_char *p_restart;
    u_char readlen;
    tBGP4_ADDR ip;
    u_char str[24];
    /*
    Length > 9
    */
    if (len < 9)
        return ;
    /*
    version
    */
    ver = *p_msg ;
    
    roffset++ ;
    p_msg++ ;
    
    /*AS
    */
    as = *(u_short *)p_msg ;
    
    roffset += 2 ;
    p_msg += 2 ;
    /*
    hold time
    */
    holdtime = *(u_short *)p_msg ;
    
    roffset += 2 ;
    p_msg += 2 ;
    /*
    BGP ID
    */
    ip.afi = AF_INET;
    memcpy(&ip.ip, p_msg, 4);
    roffset += 4 ;
    p_msg += 4 ;

    bgp4_log(1, " open MSG");
    
    bgp4_log(1, "  version %d",ver);
    bgp4_log(1, "  AS number %d", ntohs(as));
    bgp4_log(1, "  holdtime %d", ntohs(holdtime));
    bgp4_log(1, "  ID %s,", bgp4_printf_addr(&ip, str));
    
    /*
    if any option data ?
    */
    if (roffset >= len )
    {
        return ;
    }
    
    total_optlen = *p_msg ;
    
    bgp4_log(1, "  option total length %d", total_optlen);
    
    if (total_optlen == 0)
    {
        return;
    }
    
    /*
    enter a loop reading option data
    */
    p_msg ++ ;
    
    while (opt_rlen < total_optlen)
    {
        opt_type = *p_msg ;
        opt_len = *(p_msg + 1);
        tmp_optlen = opt_len ;  
        /*
        only support capability advertisment 2
        */
        bgp4_log(1,"   option type %d,length %d", opt_type, opt_len);     
        if (opt_type == 2)
        {
            bgp4_log(1,"    cap advertisment");
            
            p_tmpbuf = p_msg + 2 ;
            
            while (tmp_optlen >= 2)   
            {
                cap_code = *p_tmpbuf;
                cap_len = *(p_tmpbuf + 1);
                bgp4_log(1,"    cap code %d,length %d", cap_code, cap_len);
                switch(cap_code) {
                case 1:/*MPBGP AFI/SAFI,length must be 4*/
                    bgp4_log(1, "    multiple protocol");
                    if (cap_len == 4)
                    {
                        afi = *(p_tmpbuf + 3);/*an error,but has no other mean*/
                        safi = *(p_tmpbuf + 5) ;
                        bgp4_log(1, "     afi/safi %d/%d",afi,safi); 
                    }
                    else
                    {
                        bgp4_log(1, " length invalid");
                    }
                    break;
                case 2:/*RT refresh,length must be 0*/
                    bgp4_log(1, "    route refresh");
                    break;
                case BGP4_CAP_ORF : /*Outbound route filter*/
                    bgp4_log(1, "     outbound route filter");
                    if (bgp4_debug_orf_capability(p_tmpbuf+2,cap_len,tmp_optlen-2) == 0)
                    {
                        return ;
                    }
                    break;
                case BGP4_CAP_4BYTE_AS :/*4bytes as number,length must be 4*/ 
                    bgp4_log(1,"     long AS");
                    if (cap_len == 4)
                    {
                        memcpy(&long_as,p_tmpbuf+2,4);
                        long_as = ntohl(long_as);
                        bgp4_log(1," long AS is %u",long_as); 
                    }
                    else
                    {
                        bgp4_log(1," this length is invalid");
                    }
                    break;
                case BGP4_CAP_GRACEFUL_RESTART :
                    bgp4_log(1,"    bgp cap graceful restart");
                    
                    if (((cap_len+2) > tmp_optlen) || (((cap_len-2)%4) != 0) || (cap_len < 2))
                    {
                        bgp4_log(1," this length is invalid");
                    }
                    else
                    {
                        p_restart = p_tmpbuf+2;
                        readlen = cap_len ;
                        bgp4_get_2bytes(rtime, p_restart);
                        bgp4_log(1,"     restart bit %d,restart time:%d",(rtime&0x8000) == 0 ? 0 : 1,(rtime&0x0fff)); 
                        readlen -= 2;
                        p_restart += 2;
                        
                        while (readlen > 0)
                        {
                            bgp4_get_2bytes(afi,p_restart);
                            p_restart += 2;
                            
                            safi = *p_restart;
                            p_restart++;
                            
                            afflag = *p_restart;
                            p_restart++;
                            bgp4_log(1,"     afi/safi:%d/%d,forward %d",afi,safi,(afflag&0x80) == 0 ? 0 : 1); 
                            readlen -= 4;
                        }
                    }
                    break;
                default :
                    break;
                }
                
                tmp_optlen -= (cap_len + 2);
                p_tmpbuf +=  (cap_len + 2) ;   
            }
        }
        else if (opt_type == 1)/*add support for authentication.*/
        {
            bgp4_log(1, "    authentication information");
        }
        p_msg += (2 + opt_len) ;
        opt_rlen += (2 + opt_len) ;  
    }
    return;  
}

void 
bgp4_debug_rtrefresh_msg(
       u_char *p_msg,
       u_short len)
{
    u_short afi;
    u_char safi;
    u_char flag;
    short   restlen; 
    u_short orf_len;
    u_short readlen;
    u_char *p_buf;
    u_char orf_type;
    u_char orf_flag;
    u_char str[128];
    u_char action_str[64];
    u_char match_str[64];    
    
    if (len < 4)
    {
        return ;
    }
    bgp4_log(1,"   rt refresh route  information,length %d,",len);
    memcpy(&afi,p_msg,2);  
    afi = ntohs(afi);
    safi = *(p_msg +3);
    bgp4_log(1,"     afi/safi %d/%d,",afi,safi);
    
    /*if length > 4,mean orf followed*/ 
    if (len > 4)
    {
        flag = *(p_msg+4);
        
        bgp4_log(1,"    refresh time:%s",
                flag == 1 ? "immediate" : (flag == 2 ? "deferred" : "unknown"));
        
        p_buf = p_msg+5;
        restlen = len - 5;  
        /*Orf Entrys*/ 
        while (restlen > 0)
        {
            bgp4_get_2bytes(orf_len,(p_buf+1));
            if (restlen < (orf_len+3))
            {
                return ;
            }
            restlen -= (orf_len+3);
            
            orf_type = *p_buf;
            p_buf++;
            bgp4_log(1,"    orf type:%s,", orf_type == BGP4_ORF_TYPE_IPPREFIX ? "ip Prefix" : "Unknown");
            
            /*Length obtained*/     
            p_buf += 2;
            bgp4_log(1,"    orf len:%d",orf_len);
            
            /*READ*/  
            readlen = 0 ;
            while(readlen < orf_len)
            {
                tBGP4_ORF_ENTRY orf;
                /* ORF entry
                   +---------------------------------+
                   |   Action (2 bit)                |
                   +---------------------------------+
                   |   Match (1 bit)                 |
                   +---------------------------------+
                   |   Reserved (5 bits)             |
                   +--------------------------------+
                   |   Sequence (4 octets)          |
                   +--------------------------------+
                   |   Minlen   (1 octet)           |
                   +--------------------------------+
                   |   Maxlen   (1 octet)           |
                   +--------------------------------+
                   |   Length   (1 octet)           |
                   +--------------------------------+
                   |   Prefix   (variable length)   |
                   +--------------------------------+*/
                   
                /*1byte flag*/
                orf_flag = *p_buf;
                p_buf ++;
                readlen++;
                
                if ((orf_flag&0xc0) == 0x00)
                {
                    sprintf(action_str, "add");
                }    
                else if ((orf_flag&0xc0) == 0x40)
                {
                    sprintf(action_str, "rmv");
                }
                else if ((orf_flag&0xc0) == 0x80)
                {
                    sprintf(action_str, "rmv all");
                }
                else  
                {
                    sprintf(action_str, "add");
                }
                
                if ((orf_flag&0x20) == 0x00)
                {
                    sprintf(match_str, "permit");
                }
                else
                {
                    sprintf(match_str, "deny");
                }
                
                /*remove all not contain any value*/
                if ((orf_flag&0xc0) == 0x80)
                {
                    bgp4_log(1,"   rmv all");
                    continue;      
                }
                memset(&orf, 0, sizeof(orf));
                
                bgp4_get_4bytes(orf.seqnum, p_buf);
                p_buf += 4;
                readlen += 4;
                
                orf.minlen = *p_buf;
                p_buf += 1;
                readlen += 1;

                orf.maxlen = *p_buf;
                p_buf += 1;
                readlen += 1;

                orf.prefix.prefixlen = *p_buf;
                p_buf += 1;
                readlen += 1;

                memcpy(&orf.prefix.ip, p_buf, bgp4_bit2byte(orf.prefix.prefixlen));
                p_buf += bgp4_bit2byte(orf.prefix.prefixlen);
                readlen += bgp4_bit2byte(orf.prefix.prefixlen);

                orf.prefix.afi = bgp4_afi_to_index(afi, safi);
                bgp4_log(1,"    %s %d ip prefix %s/%d,max %d,min %d,%s", action_str,
                    orf.seqnum,
                    bgp4_printf_addr(&orf.prefix, str), 
                    orf.prefix.prefixlen,
                    orf.maxlen, orf.minlen,match_str);
            }
        }
    }
}

void 
bgp4_debug_ipv4_prefix(
          u_char *p_msg,
          u_char plen)
{
    tBGP4_ADDR ip;
    u_char str[24];
    u_char prefix[4];
    u_char byte;
    u_char bit;
    u_char mask[8]= {0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe};
    
    if (plen > 32) 
    {
        return ;  
    }
    
    byte = plen/8 ;
    bit =  plen%8 ;
    
    *(u_int *)prefix = 0;
    /*byte*/      
    if (byte != 0)     
    {
        memcpy(prefix,p_msg,byte);  
    }
    
    if (bit != 0)    
    {
        prefix[byte] = (*(p_msg + byte))& mask[bit];
    }
    ip.afi = AF_INET;
    *(u_int*)ip.ip = *(u_int*)prefix;
    bgp4_log(1,"      ip prefix %s/%d",bgp4_printf_addr(&ip, str),plen);
    return;
}

void 
bgp4_debug_ipv6_prefix(
        u_char *p_msg,
        u_char plen)
{
    tBGP4_ADDR ip;
    u_char  prefix[16];
    u_char str[64];
    u_char byte;
    u_char bit; 
    u_char mask[8]= {0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe};
    
    if ((p_msg == NULL) || (plen > 128)) 
    {
        return ;  
    }
    
    byte = plen/8 ;
    bit =  plen%8 ;
    
    memset(prefix,0,16);
    /*byte*/      
    if (byte != 0)     
    {
        memcpy(prefix,p_msg,byte);  
    }
    
    if (bit != 0)    
    {
        prefix[byte] = (*(p_msg + byte))& mask[bit];
    }

    ip.afi = AF_INET6;
    memcpy(ip.ip, prefix, 16);
    bgp4_log(1,"       ip prefix  %s/%d",bgp4_printf_addr(&ip, str),plen);
    return;
}

short 
bgp4_debug_mp_nlri_route(
          u_char *p_msg,
          u_char code)
{   
    u_char *p_buf = p_msg ;
    u_int lbl ;
    u_char bit ;
    u_char byte ;   
    u_int label;
    tBGP4_VPLS_ADDR vplsaddr;
    u_char rdstr[64];
    
    if (p_msg == NULL)
    {
        return -1;
    }
    /*   +------------------------------------+
      |  Length (2 octets)                 |--00 01 == 17
      +------------------------------------+
      |  Route Distinguisher  (8 octets)   | == 00 00 00 64 00 00 00 01 ---RD
      +------------------------------------+
      |  VE ID (2 octets)                  | 00 02 ID为2
      +------------------------------------+
      |  VE Block Offset (2 octets)        |00 00 Offset为0
      +------------------------------------+
      |  VE Block Size (2 octets)          |00 0a BLOCK
      +------------------------------------+
      |  Label Base (3 octets)             |18 6a 11 ---和普通标记封装一样，都是增加了4比特偏移
      +------------------------------------+*/
    if (code == BGP4_PF_L2VPLS)
    {
        /*ignore length*/
        p_buf += 2;
        
        bgp4_log(1, "     vpls rd %s", bgp4_print_rd(p_buf, rdstr));
        p_buf += 8;
        
        bgp4_get_2bytes(vplsaddr.ve_id, p_buf);
        p_buf += 2;
        bgp4_get_2bytes(vplsaddr.ve_block_offset, p_buf);
        p_buf += 2;
        bgp4_get_2bytes(vplsaddr.ve_block_size, p_buf);
        p_buf += 2;
        vplsaddr.label_base = bgp4_label_extract(p_buf);
        bgp4_log(1, "     id %d,size %d,offset %d,label %d", 
            vplsaddr.ve_id,
            vplsaddr.ve_block_size,
            vplsaddr.ve_block_offset,            
            vplsaddr.label_base);
        return BGP4_VPLS_NLRI_LEN+2;
    }
    
    /*get prefix bit len,and byte len*/    
    bit = *(p_buf);
    p_buf ++ ;
    
/*    bgp4_log(1, "      prefix len %d",bit);*/
    
    byte = bit/8 ;
    
    if ((bit % 8) != 0)
    {
        byte ++ ;  
    }
    
    switch(code){
        case BGP4_PF_IPUCAST   :      /*0<= prefix len <= 32*/
        case BGP4_PF_IPMCAST :    
            if ((bit > 32))
            {
                bgp4_log(1, "     prefix length should be less than 33,prefix length %d",bit);
                return -1 ; 
            }
            bgp4_debug_ipv4_prefix(p_buf,bit);           
            break ;
        case BGP4_PF_IP6UCAST   :/*pure ipv6 prefix,0<= len <= 128*/
        case BGP4_PF_IP6MCAST :     
            if ((bit > 128))
            {
                bgp4_log(1, "     ipv6 prefix length Should be less than 129,ipv6 prefix length %d",bit);
                return -1 ; 
            }
            
            bgp4_debug_ipv6_prefix(p_buf,bit);         
            
            break;
        case BGP4_PF_IP6LABEL     :/*3 bytes label + prefix
                                        24 <= <= 152
                                    */      
            if ((bit < 24) || (bit > 152))                                
            {
                bgp4_log(1, "     vpn prefix length The range is <24-152>,%d out range",bit);
                return -1; 
            }
                
            lbl = ntohl(*(u_int *)(p_buf)) ;
            lbl = lbl >> 12 ;
            bgp4_log(1,"     label %d",lbl);          
                
            p_buf += 3 ;
                
            bgp4_debug_ipv6_prefix(p_buf,bit - 24);                                      
            break;
            
        case BGP4_PF_IPVPN:
            /*label 3byte
              rd 8byte
              route ...
             */
            label = bgp4_label_extract(p_buf);
            bgp4_log(1,"      ");
            
            bgp4_log(1,"      label %d(%02x.%02x.%02x),rd %s", 
                label, p_buf[0], p_buf[1], p_buf[2],
                bgp4_print_rd(p_buf+3, rdstr));
            p_buf += 3;

            p_buf += 8;

            bgp4_debug_ipv4_prefix(p_buf, bit - 88); 
            break;
            
        case BGP4_PF_IP6VPN :/*currently,do not consider it*/        
            break;
        case BGP4_PF_L2VPLS:
            
            break;
        default :
            break;         
    }    
    return (byte + 1)/*include prefixlen byte*/   ;
}

/******************************************************************************
Function Name : bgp4_debug_update_msg                                                  
Description   :
damp mpbgp reachable nlri attribute
Input(s)      : packet,packet len                   
Output(s)    : none
Return(s)    : none                                                       
******************************************************************************/
void 
bgp4_debug_mp_reach_nlri(
         u_char *p_msg,
         u_short len)
{
    u_char *p_buf =  p_msg ;
    u_char *p_last = (p_msg + len) ;
    u_short afi ;
    u_char safi ;
    u_char mp_code = 0 ;
    u_char nexthop_len ;  
    u_char addr[64];
    u_char snpa_num;
    u_char snpa_len;
    signed short nlri_len;
    tBGP4_ADDR ip;
    u_char str[64];

    if ((p_msg == NULL)||(len < 5))
    {
        return ;
    }
    
    bgp4_get_2bytes(afi, p_buf);
    p_buf +=2 ;
    
    safi = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,"     afi/safi:%d/%d",afi,safi);
    
    /*get mpcode*/
    if (afi == 1)/*ipv4*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IPUCAST;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IPMCAST;
            break ;           
        case 128:/**/
            mp_code = BGP4_PF_IPVPN;
            break ;           
        default :
            break ;         
        }
    }
    else if (afi == 2)/*ipv6*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IP6UCAST;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IP6MCAST;
            break ;           
        case 4:/*label*/
            mp_code = BGP4_PF_IP6LABEL;
            break ;              
        case 128:/**/
            mp_code = BGP4_PF_IP6VPN;
            break ;           
        default :
        break ;         
        } 
    }

    /*VPLS*/
    if ((afi == BGP4_AF_L2VPN) && (safi == BGP4_SAF_VPLS))
    {
        mp_code = BGP4_PF_L2VPLS;
    }
    
    if (mp_code == 0)
    {
        bgp4_log(1,"Unkown afi/safi");  
        return ;
    }    
    
    /*parse nexthop*/     
    nexthop_len = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,"      nexthop length:%d", nexthop_len);
    
    if ((nexthop_len + 4) > len)
    {
        bgp4_log(1,"     nexthop length exce option length %d",len);  
        return ;
    }
    
    /*for different afi/safi,address is different*/     
    switch(mp_code){
    case BGP4_PF_IPUCAST :/*MUST be 4*/ 
    case BGP4_PF_IPMCAST  :
    case BGP4_PF_L2VPLS:
        if (nexthop_len != 4)
        {
            bgp4_log(1,"     invalid length of nexthop");  
            return ;  
        }
        ip.afi = AF_INET;
        memcpy(&ip.ip, p_buf, 4);
        bgp4_log(1,"     nexthop:%s",bgp4_printf_addr(&ip, str)); 
        p_buf += 4 ;                 
        break;
    case BGP4_PF_IPVPN  :/*must be 12,need skip 8byts of 0*/
        if (nexthop_len != 12)
        {
            bgp4_log(1,"     invalid length of nexthop");  
            return ;  
        }
        /*display rd*/
        p_buf += 8 ;    
        ip.afi = AF_INET;
        memcpy(&ip.ip, p_buf, 4);
        bgp4_log(1,"      nexthop %s",bgp4_printf_addr(&ip, str)); 
        p_buf += 4 ;                    
        break;  
        
    case BGP4_PF_IP6UCAST    :/*must be 16 or 32*/
    case BGP4_PF_IP6MCAST  :
        if ((nexthop_len != 16) && (nexthop_len != 32))
        {
            bgp4_log(1,"     invalid nexthop length");
            return ;  
        }
        /*first,a global address*/          
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
  
        p_buf += 16 ;
        bgp4_log(1, "      nexthop %s",addr);   
        
        if (nexthop_len == 32)
        {
            ip.afi = AF_INET6;
            memcpy(&ip.ip, p_buf, 16);
            bgp4_printf_addr(&ip, addr);
            p_buf += 16 ;
            bgp4_log(1, "      nexthop %s",addr);     
        }
        break;
    case BGP4_PF_IP6LABEL      :/*must be 16,is a ipv4 mapped ipv6 address*/
        if ((nexthop_len != 16))
        {
            bgp4_log(1,"     invalid nexthop length");
            return ;  
        }
        /*first,a global address*/           
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
        p_buf += 16 ;
        bgp4_log(1, "     nexthop %s",addr);   
        
        break;
    case BGP4_PF_IP6VPN  :/*must be 24 or 48*/
        if ((nexthop_len != 24) && (nexthop_len != 48))
        {
            bgp4_log(1,"     invalid nexthop length");
            return ;  
        }
        /*first,a global address*/           
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
        p_buf += 24 ;
        bgp4_log(1, "     nexthop %s",addr);   
        
        if (nexthop_len == 48)
        {
            ip.afi = AF_INET6;
            memcpy(&ip.ip, p_buf, 16);
            bgp4_printf_addr(&ip, addr);
            p_buf += 24 ;
            bgp4_log(1,"     nexthop %s",addr);     
        }
        break;
    default :
        return ;
    }
    
    /*SNPA,skip*/   
    snpa_num = *(p_buf);
    p_buf ++ ;
    
    bgp4_log(1,"      snpa number:%d", snpa_num);   
    
    while ((snpa_num > 0)&&((u_long)p_buf < (u_long)p_last))
    {
        snpa_len = *(p_buf) ;
        p_buf ++ ;
        
        p_buf += snpa_len ;
        snpa_num -- ;      
    }
    
    /*NLRI information,for different afi/safi,result is different*/    
    while ((u_long)p_buf < (u_long)p_last)
    {
        nlri_len = bgp4_debug_mp_nlri_route(p_buf, mp_code);   
        if (nlri_len <0)
        break;  
        
        p_buf += nlri_len ;  
    }

    return;
}

void 
bgp4_debug_mp_unreach_nlri(
       u_char *p_msg,
       u_short len)
{
    u_char *p_buf = p_msg ;
    u_char *p_last = (p_msg + len);  
    u_short afi ;
    u_char safi ;
    u_char mp_code = 0 ;
    short   nlrilen ; 
    if ((p_msg == NULL) || (len < 3)) 
    {
        bgp4_log(1,"invalid length");  
        return ;
    }
    
    /*getting AFI and SAFI*/
    bgp4_get_2bytes(afi, p_buf);
    p_buf +=2 ;
    
    safi = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,"     af %d/%d",afi,safi);
    
    /*get mpcode*/
    if (afi == 1)/*ipv4*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IPUCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IPMCAST ;
            break ;           
        case 128:/**/
            mp_code = BGP4_PF_IPVPN;
            break ;           
        default :
            break ;         
        }
    }
    else if (afi == 2)/*ipv6*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IP6UCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IP6MCAST ;
            break ;           
        case 4:/*label*/
            mp_code = BGP4_PF_IP6LABEL ;
            break ;              
        case 128:/**/
            mp_code = BGP4_PF_IP6VPN ;
            break ;           
        default :
            break ;         
        } 
    }

    /*VPLS*/
    if ((afi == BGP4_AF_L2VPN) && (safi == BGP4_SAF_VPLS))
    {
        mp_code = BGP4_PF_L2VPLS;
    }
    
    if (mp_code == 0)
    {
        bgp4_log(1,"Unkown afi/safi");  
        return ;
    }    
    
    /*NLRI information,for different afi/safi,result is different*/    
    while ((u_long)p_buf < (u_long)p_last)
    {
        nlrilen = bgp4_debug_mp_nlri_route(p_buf,mp_code);     
        if (nlrilen < 0)  
        {
            return ;
        }
        p_buf += nlrilen;
    }
    return;
}

void 
bgp4_debug_update_msg(
     u_char *p_msg,
     u_short len)
{
    tBGP4_ADDR ip;
    short   unfea_len; 
    short   total_attr_len; 
    short   nlrilen;
    u_char plen;
    u_char byte;
    u_char aflag;
    u_char acode;
    u_short alen;
    u_short rlen;
    u_char origin;
    u_char segtype;
    u_char seglen;
    u_int attrindex = 0;
    u_short as;
    u_int as4;
    u_int med;
    u_char str[64];
    u_char pathAttr[96];

    bgp4_log(1," update bgp msgs");
    bgp4_get_2bytes(unfea_len, p_msg);
    p_msg += 2 ;
    
    bgp4_log(1, " unfeasible length %d", unfea_len);
    
    if (unfea_len > (len - 2))
    {
        return ;
    }
    
    if (unfea_len > 0)
    {
        rlen =  0 ;
        while (rlen < unfea_len)
        {
            plen = *p_msg ;
            
            p_msg++ ;
            
            if(plen > 32)
            {
                bgp4_log(1,"invalid prefix length %d",plen);
                return ;
            }
            
            byte = plen/8 ;
            
            if (plen%8 != 0)
            {
                byte++ ;
            }
            ip.afi = AF_INET;
            *(u_int*)ip.ip = 0 ;
            memcpy(ip.ip, p_msg, byte);
            bgp4_printf_addr(&ip, str);
            bgp4_log(1, "     unfeasible nlri %s/%d", str, plen);
            /*Get Prefix*/
            p_msg += byte ;
            rlen += (byte + 1);     
        }
    }
    
    /*
    total path attribute length
    */    
    bgp4_get_2bytes(total_attr_len, p_msg);
    p_msg += 2 ;
    
    bgp4_log(1, " attribute length %d", total_attr_len);
    
    if (total_attr_len > (len - 4 - unfea_len) ) 
    {
        return ;
    }
    if (total_attr_len > 0)
    {
        rlen = 0;
    
        while (rlen < total_attr_len)
        {
            /*
            0                   1
            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |  Attr. Flags  |Attr. Type Code|
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            */
            aflag = *p_msg ;
            p_msg ++ ; 
            
            acode = *p_msg ;
            p_msg ++ ; 

            memset(pathAttr,0,96);         
            /*
            flag : bit 0 :1 optional,0 well-known
            bit 1 :1 transitive ,0 non-transitive
            bit2 : 1 partial,0 complete
            bit3 : 1 ,length 2bytes,0 length 1byte
            */   
            sprintf(pathAttr, "  attribute flag %#x",aflag);
            
            /*get length*/
            if (aflag & 0x10)
            {
                bgp4_get_2bytes(alen, p_msg);
                p_msg += 2 ;  
                rlen += 4 ;
            }
            else
            {
                alen = *p_msg ;
                p_msg++ ;
                rlen += 3 ;  
            }
            bgp4_log(1,"  #%d:flag 0x%02x,type %d,length %d", ++attrindex, aflag, acode, alen); 
            /*
            enter path attributes
            */      
            switch (acode ) {
            case 1 :/*Origin*/
                origin = *p_msg ;
                bgp4_log(1,"     origin %d %s",origin,
                    (origin == 0 ) ? "igp" : ((origin == 1 ) ? "egp" : ((origin == 2 ) ? "incompele" : "Invalid")));                
                break;
            case 2 :/*AS Path*/    
                bgp4_log(1,"     as-path");
                {
                    u_short readlen = 0 ;
                    while (readlen < alen)
                    {
                        segtype = *(p_msg + readlen) ;
                        seglen = *(p_msg + readlen + 1) ;  
                        
                        bgp4_log(1,"     as %s,count %d",
                            segtype == 1 ? "set" : "seqence",seglen);
                        
                        readlen += 2 ;
                        while (seglen > 0) 
                        {
                            bgp4_get_2bytes(as, (p_msg + readlen));
                            bgp4_log(1,"     as %d",as);
                            readlen += 2 ;
                            seglen -- ;
                        }
                        
                    }
                }            
                break ;
            case 3 :/*Next hop*/
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg, 4);                
                bgp4_log(1,"     nexthop %s",bgp4_printf_addr(&ip, str));
                break;
                
            case 4 :/*MED*/
                bgp4_get_4bytes(med, (p_msg));
                bgp4_log(1,"     msg %d",med);                
                break;
            case 5 :/*Local Pref*/
                bgp4_get_4bytes(med, (p_msg));     
                bgp4_log(1,"     local preference  %d",med);                
                break;
            case 6 :/*ATOMIC_AGGREGATE*/
                bgp4_log(1, "     atomic aggregate");
                /*
                Length is 0,no value
                */
                break;
            case 7 :/*AGGREGATOR*/
                /*
                Length is 6,AS+IP
                */
                bgp4_get_2bytes(as, p_msg);
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg+2, 4);

                bgp4_log(1,"    aggreagator,as %d,ip %s",
                    as,bgp4_printf_addr(&ip, str));
                break;
                
            case 8:/*community*/
                {
                    u_int community;
                    u_int readlen = 0;
                    u_char *p_val = p_msg;

                    for (readlen = 0 ; readlen < alen ; readlen += 4, p_val += 4)
                    {
                        bgp4_get_4bytes(community, p_val);
                        bgp4_log(1,"    comminity %x", community);
                    }                    
                }
                break;
                
            case 9 :/*originator id*/
                /*4 bytes id*/
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg, 4);
                
                bgp4_log(1, "     source ip %s",bgp4_printf_addr(&ip, str));
                
                break;
            case 10 :/*CLUSTER_LIST*/
                /*
                multiple 4 bytes id
                */
                {
                    u_short readlen = 0 ;
                    
                    while (readlen < alen)
                    {
                        ip.afi = AF_INET;
                        memcpy(&ip.ip, (p_msg + readlen), 4);
                        bgp4_log(1,"    cluster ip %s",bgp4_printf_addr(&ip, str));
                        
                        readlen += 4 ;
                    }
                }
                
                break;
                /*add support for route reflector .ddp  E*/   
            case 14 :/*MP_REACH_NLRI*/
                bgp4_log(1, "    mp reach nlri");
                bgp4_debug_mp_reach_nlri(p_msg,alen);
                break ;
            case 15 :/*MP_UNREACH_NLRI*/
                bgp4_log(1, "    mp unreach nlri");
                bgp4_debug_mp_unreach_nlri(p_msg,alen);
                break;
            case 16 :/*ex-community*/                
                /*
                just consider RT 00 02
                */
                bgp4_log(1, "    extend community");
                {
                    u_short readlen = 0;
                    
                    u_char etype ;
                    u_char subetype ;            
                    u_char ebuf[32];
                    u_char exCmn[96];
                    u_char Type[160];
                    
                    while (readlen < alen)
                    {
                        etype = *(p_msg + readlen) ;
                        subetype = *(p_msg + readlen + 1) ;    
                        memset(exCmn,0,96);
                        memset(Type,0,16);
                        sprintf(exCmn,"    type %x-",etype);

                        /*support for vpn*/
                        if (subetype == 5)
                        {
                            bgp4_log(1,"    (type,stype)=%d.%d",etype, subetype);
                            bgp4_log(1,"     ospf domain id:%02x.%02x.%02x.%02x.%02x.%02x",
                                p_msg[readlen+2], p_msg[readlen+3],
                                p_msg[readlen+4], p_msg[readlen+5],
                                p_msg[readlen+6], p_msg[readlen+7]);
                            readlen += 8;
                            continue;
                        }
                        if (((etype == 0x03) && (subetype == 0x06))
                            || ((etype == 0x80) && (subetype == 0x06)))
                        {
                            bgp4_log(1,"    (type,stype)=%d.%d",etype, subetype);
                            bgp4_log(1,"     ospf routeType id:%02x.%02x.%02x.%02x.%02x.%02x",
                                p_msg[readlen+2], p_msg[readlen+3],
                                p_msg[readlen+4], p_msg[readlen+5],
                                p_msg[readlen+6], p_msg[readlen+7]);
                            readlen += 8;
                            continue;
                        }    
                        if (((etype == 0x01) && (subetype == 0x07))
                            || ((etype == 0x80) && (subetype == 0x01)))
                        {
                            bgp4_log(1,"    (type,stype)=%d.%d",etype, subetype);
                            bgp4_log(1,"     ospf router id:%0d.%0d.%0d.%0d",
                                p_msg[readlen+2], p_msg[readlen+3],
                                p_msg[readlen+4], p_msg[readlen+5]);
                            readlen += 8;
                            continue;
                        }    
                     #if 0   
                        if (etype & 0x80)             
                        {
                            strcat(exCmn,",IANA-first come first serve");
                        }
                        else
                        {
                            strcat(exCmn,",IANA-IETF consensus");
                        }
                        
                        if (etype & 0x40)             
                        {
                            strcat(exCmn,",non-transitive");
                        }
                        else
                        {
                            strcat(exCmn,",transitive");
                        }
                     #endif                         
                        /*type:
                        0:2+4
                        1:4+2
                        2:4+2 as
                        */    
                        etype &= 0x3f ;
                        switch (etype){
                        case 0 :
                            strcat(exCmn,"2 bytes as number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 2));
                            bgp4_get_4bytes(med,(p_msg + readlen + 4));                   
                            sprintf(ebuf,"%d:%d",as,med); 
                            break ;
                        case 1 :
                            strcat(exCmn,"4 bytes ip number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 6));
                            ip.afi = AF_INET;
                            memcpy(&ip.ip, (p_msg + readlen + 2), 4);
                            sprintf(ebuf,"%s:%d",bgp4_printf_addr(&ip, str),as);             
                            
                            break ;                 
                        case 2 :   
                            strcat(exCmn,"4 bytes as number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 6));
                            bgp4_get_4bytes(med,(p_msg + readlen + 2));                                           
                            sprintf(ebuf,"%d:%d",med,as);
                            break ;                  
                        default :  
                            strcat(exCmn,"unkown");
                            ebuf[0] = '\0';/*sprintf(au1Buf,"");*/                  
                            break ;                
                        }
                        
                        bgp4_log(1,"%s",exCmn);
                        /*
                        sub type
                        */                             
                        switch (subetype){
                        case 2 :
                            sprintf(Type,"route-target %s",ebuf);
                            break ;
                        case 10:
                            sprintf(Type,"vpls community");
                            break;
                        case 3:
                            sprintf(Type,"route origin community");
                            break;
                        default :
                            sprintf(Type,"unkown");
                            break ;                
                        }
                        bgp4_log(1,"    sub-type %d  %s",subetype,Type);
                        if (subetype == 10)
                        {
                           u_char *p_vpls = p_msg + readlen + 2;
                           u_char encap = p_vpls[0];
                           u_char ctrl = p_vpls[1];
                           u_short mtu = 0;
                           bgp4_get_2bytes(mtu, (p_vpls + 2));
                           bgp4_log(1,"     encap %d,ctrlwd %d,mtu %d", encap, ctrl, mtu);
                           /*insert vpls community
                           +------------------------------------+
                            | Extended community type (2 octets) | 80 0a
                            +------------------------------------+
                            |  Encaps Type (1 octet)             | 4????
                            +------------------------------------+
                            |  Control Flags (1 octet)           |  
                            +------------------------------------+
                            |  Layer-2 MTU (2 octet)             |
                            +------------------------------------+
                            |  Reserved (2 octets)               |
                            +------------------------------------+
                          */
                        }
                        readlen += 8 ;
                    }                        
                    
                }
                break;

            case BGP4_ATTR_NEW_PATH:
                bgp4_log(1,"     4 byte attr new path");
                {
                    u_short readlen = 0 ;
                    while (readlen < alen)
                    {
                        segtype = *(p_msg + readlen) ;
                        seglen = *(p_msg + readlen + 1) ;  
                        
                        bgp4_log(1,"     as %s,count %d",
                            segtype == 1 ? "set" : "seqence",seglen);
                        
                        readlen += 2 ;
                        while (seglen > 0) 
                        {
                            bgp4_get_4bytes(as4, (p_msg + readlen));
                            bgp4_log(1,"     as %d",as4);
                            readlen += 4 ;
                            seglen -- ;
                        }
                        
                    }
                }            
                break;

            case BGP4_ATTR_NEW_AGGREGATOR:
                /*
                Length is 8,AS+IP
                */
                bgp4_get_4bytes(as4, p_msg);
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg+4, 4);

                bgp4_log(1,"    4byte aggregate,as %d,ip %s",
                    as4,bgp4_printf_addr(&ip, str));
                break;
                
            default :
                break;    
            }
    
            p_msg += alen;
            rlen += alen;    
        }
    }
    attrindex = 0;
    
    nlrilen = len - 4 - unfea_len - total_attr_len ;
    bgp4_log(1, "  nlri length %d", nlrilen);
    if (nlrilen > 0) 
    {
        rlen =  0 ;
        while (rlen < nlrilen)
        {
            plen = *p_msg ;
          
            p_msg++ ;
          
            if(plen > 32)
            {
                bgp4_log(1,"invalid prefix length %d",plen);
                return ;
            }
          
            byte = plen/8 ;
          
            if (plen%8 != 0)
            {
                byte++ ;
            }

            ip.afi = AF_INET;
            *(u_int*)ip.ip = 0 ;
            memcpy(&ip.ip, p_msg, byte);
            bgp4_printf_addr(&ip, str);
            bgp4_log(1,"     #%-03d %s/%d",++attrindex, str,plen); 
            p_msg += byte ;
            rlen += (byte + 1);     
        }  
    }
    return;
}

void 
bgp4_debug_notify_msg(
      u_char *p_msg,
      u_short len)
{
    u_char code ;
    u_char scode;
    u_char estr[32];
    u_char sestr[64];
    
    if (len < 2)
    {
        return ;
    }
    
    code = *(p_msg) ;
    scode = *(p_msg + 1) ;
    bgp4_printf_notify(code, scode, estr, sestr) ;

    bgp4_log(1, " notify msg");
    
    bgp4_log(1,"  code %d(%s),scode %d(%s)", code, estr, scode, sestr);
    return;
}

u_char * 
bgp4_printf_syserror(
      u_int code,
      u_char *str)
 {
    if (!str)
    {
        return NULL;
    }
#if !defined(WIN32)     
    switch (code){
    case EDESTADDRREQ:
        sprintf(str,"Destination address required");
        break;
    case ENETUNREACH :
        sprintf(str,"Network is unreachable");
        break;
    case ENETRESET :
        sprintf(str,"Network dropped connection on reset");
        break;  
    case ECONNABORTED :
        sprintf(str,"Software caused connection abort");
        break;    
    case ECONNRESET :
        sprintf(str,"Connection reset by peer");
        break;
    case ENOBUFS :
        sprintf(str,"No buffer space available");
        break;    
    case EISCONN :
        sprintf(str,"Socket is already connected");
        break;    
    case ENOTCONN :
        sprintf(str,"Socket is not connected");
        break;  
    case ESHUTDOWN :
        sprintf(str,"Can't send after socket shutdown");
        break;  
    case ETOOMANYREFS :
        sprintf(str,"Too many references: can't splice");
        break;      
    case ETIMEDOUT :
        sprintf(str,"Connection timed out");
        break;      
    case ECONNREFUSED :
        sprintf(str,"Connection refused");
        break;    
    case ENETDOWN :
        sprintf(str,"Network is down");
        break;    
    case EHOSTUNREACH :
        sprintf(str,"No route to host");
        break;      
    case EINPROGRESS :
        sprintf(str,"Operation now in progress");
        break;  
    case EALREADY :
        sprintf(str,"Operation already in progress");
        break;      
    case EWOULDBLOCK :
        sprintf(str,"Operation would block");
        break;        
    case EINVAL:
        sprintf(str,"Invalid argument");  
        break; 
    case EHOSTDOWN :
        sprintf(str,"Host is down"); 
        break; 
    case 3997700 :
        sprintf(str,"Object Time Out");  
        break; 
    case 3997698 :
        sprintf(str,"Object Unavaliable"); 
        break; 
    case 851971 :
        sprintf(str,"Invalid FileDescriptor");  
        break;  
    default :
        sprintf(str,"Unkown(%d)",code); 
        break; 
    }
#else
    sprintf(str, "%d", code);
#endif
    return str;
}

static char* route_desc[BGP4_MAX_PROTO] = 
{
    "unknown", 
    "other",  
    "direct",  
    "static",    
    "icmp",     
    "egp",      
    "ggp",     
    "hello",     
    "rip",     
    "is-is",     
    "es_is",     
    "ciscoIgrp",     
    "bbnSpfIgp",     
    "ospf",  
    "bgp",  
    "arp"
};

char * 
bgp4_get_route_desc(u_int proto)
{
    if (proto >= BGP4_MAX_PROTO)
    {
        return route_desc[0];
    }
    else
    {
        return route_desc[proto];
    }
}

u_char *
bgp4_printf_af(
        u_int af,
        u_char *p_str)
{
    switch (af){
        case BGP4_PF_IPUCAST:
             sprintf(p_str, "ipv4");
             break;
             
        case BGP4_PF_IPMCAST:
             sprintf(p_str, "mult cast");
             break;
             
        case BGP4_PF_IPVPN:
             sprintf(p_str, "vpn ipv4");
             break;
             
        case BGP4_PF_IP6UCAST:
             sprintf(p_str, "ipv6");
             break;
             
        case BGP4_PF_IP6MCAST:
             sprintf(p_str, "mult cast6");
             break;
             
        case BGP4_PF_IP6LABEL:
             sprintf(p_str, "ipv6 label");
             break;
             
        case BGP4_PF_IP6VPN:
             sprintf(p_str, "vpn ipv6");
             break;
             
        case BGP4_PF_L2VPLS:
             sprintf(p_str, "vpls");
             break;
             
        default:
             sprintf(p_str, "unkown");
             break;
    }
    return p_str;
}

void 
bgp4_printf_aspath(
     tBGP4_PATH *p_path,
     u_char *pstring)
{
    u_char flag = 0;
    tBGP4_ASPATH *p_aspath = NULL;
    u_char *type[5] = {"none","set ","seq ","con-seq ","con-set "};
    u_short as;
    u_short i;
    u_char u1_path[8];

    sprintf(pstring," ");
    
    bgp4_avl_for_each(&p_path->aspath_list, p_aspath)
    {
        flag = 1;        
        /*type*/
        strcat(pstring,type[p_aspath->type]);
        for (i = 0;i < p_aspath->count;i++)
        {
            memcpy(&as, p_aspath->as+(2*i), 2);
			as = ntohs(as);
            sprintf(u1_path,"%d,",as);
            strcat(pstring,u1_path);   
        }
    }
    if (flag == 0)
    {
        sprintf(pstring," ");
    }
    return;
}

u_char * 
bgp4_printf_state(
       u_short state , 
       u_char *p_str)
{
    /*
    BGP4_NS_NONE                        0
    BGP4_NS_IDLE                      1
    BGP4_NS_CONNECT                   2
    BGP4_NS_ACTIVE                    3
    BGP4_NS_OPENSENT                  4
    BGP4_NS_OPENCONFIRM               5
    BGP4_NS_ESTABLISH               6
    */
    
    switch(state){
        case BGP4_NS_IDLE :
             sprintf(p_str,"idle");
             break ;
             
        case BGP4_NS_CONNECT :
             sprintf(p_str,"connect");
             break ;
             
        case BGP4_NS_ACTIVE :
             sprintf(p_str,"active");
             break ;
             
        case BGP4_NS_OPENSENT :
             sprintf(p_str,"opensent");
             break ;
             
        case BGP4_NS_OPENCONFIRM :
             sprintf(p_str,"openconfirm");
             break ;
             
        case BGP4_NS_ESTABLISH :
             sprintf(p_str,"established");
             break ;
             
        default :
             sprintf(p_str,"unknown(%d)",state);
             break ;
    }    
    return p_str;
}

u_char *
bgp4_printf_event(
      u_short event,
      u_char *p_str) 
{
    if(p_str == NULL)
    {
        return NULL;
    }
        
    switch (event) {
        case BGP4_EVENT_START :
             sprintf(p_str,"start connection");
             break;    
             
        case BGP4_EVENT_STOP :
             sprintf(p_str,"stop connection");
             break;    
             
        case BGP4_EVENT_TCP_OPENED :
             sprintf(p_str,"tcp opened");
             break;    
             
        case BGP4_EVENT_TCP_CLOSED :
             sprintf(p_str,"tcp closed");
             break;    
             
        case BGP4_EVENT_CONNECT_FAIL :
             sprintf(p_str,"tcp connection failed");
             break;    
             
        case BGP4_EVENT_FATAL_ERROR :
             sprintf(p_str,"tcp fatal error");
             break;   
             
        case BGP4_EVENT_RETRY_TIMEOUT :
             sprintf(p_str,"retry timeout");
             break;    
             
        case BGP4_EVENT_HOLD_TMR_EXP :
             sprintf(p_str,"holdtimer expired");
             break;    
             
        case BGP4_EVENT_KEEPALIVE_TMR_EXP :
             sprintf(p_str,"keeptimer timeout expired");
             break;   
             
        case BGP4_EVENT_GR_TMR_EXP:
             sprintf(p_str,"restarttimer expired");
             break; 
             
        default :
            break;
    }
    return p_str;
}
#else

#include "bgp4com.h"

extern int log_time_print (char *buf) ;
extern int vty_log(char * proto_str, int len);

u_char * bgp4_printf_addr(tBGP4_ADDR *p, u_char *str)
{
    if (p->afi == AF_INET || p->afi == BGP4_PF_IPUCAST)
    {
        inet_ntoa_1(str,bgp_ip4(p->ip));
    }
#ifdef BGP_IPV6_WANTED
    else if(p->afi == BGP4_PF_IP6UCAST || p->afi == AF_INET6 || p->afi == BGP4_PF_IP6LABEL)
    {
        inet_ntop(AF_INET6,p->ip, str, 64);
    }
#endif
    else if(p->afi == BGP4_PF_IPVPN)
    {
        inet_ntoa_1(str,bgp_ip4(p->ip+BGP4_VPN_RD_LEN));
    }
    else if(p->afi == BGP4_PF_IP6VPN)
    {
        inet_ntop(AF_INET6,p->ip+BGP4_VPN_RD_LEN, str, 64);
    }
    return str;
}

/*log function for bgp*/
u_char * bgp4_printf_peer(tBGP4_PEER *p_peer,u_char *str)
{
    return bgp4_printf_addr(&p_peer->remote.ip, str);
}

/*log function for route*/
u_char * bgp4_printf_route(tBGP4_ROUTE *p_route, u_char * str)
{
    u_char addrstr[64] = {0};
    bgp4_printf_addr(&p_route->dest, addrstr);
    sprintf(str,"%s/%d",addrstr,p_route->dest.prefixlen);
    return str;
}

short bgp4_logx(u_int flag, u_int time_flag,const char *format, ...)
{
    va_list args;
    short len;
    u_char tstr[64];
    u_char  buf[256];
    u_short  pos = 0 ;

    if ((gBgp4.dbg_flag & flag) == 0)
    {
        return -1;
    }
         
#if 0        
    if (time_flag != 0)
    {
        log_time_print(tstr);        
        pos += sprintf(buf,"\n\r%s BGP4:",tstr) ;
    }
#else
    pos += sprintf(buf,"[BGP4]:") ;
#endif        
        
    va_start(args, format);
        
    len = vsprintf (buf + pos, format, args);
        
    if(len<0)
    {
        va_end (args);
        return -1;
    }
    va_end (args);
    buf[pos + len] = '\0' ;
  #if 0  
    vty_log(buf,len+pos);
  #else
    printf("%s\r\n", buf);
  #endif
    return (len +pos)  ;
}


void bgp4_printf_notify(u_char code,u_char scode,u_char *str,
                            u_char *sstr)
{
    if((str == NULL)||(sstr == NULL))
    {
        return ;
    }

    switch(code){
    case BGP4_MSG_HDR_ERR :
        sprintf(str,"msg header error");
        
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        if(scode == BGP4_CONN_NOT_SYNC)
        {
            sprintf(sstr,"connection not synchronized");
        }
        if(scode == BGP4_BAD_MSG_LEN)
        {
            sprintf(sstr,"bad msg length");
        }        
        if(scode == BGP4_BAD_MSG_TYPE)
        {
            sprintf(sstr,"bad msg type");
        }
        if(scode > BGP4_BAD_MSG_TYPE)
        {
            sprintf(sstr,"unrecognized");
        }
        break ;
    case BGP4_OPEN_MSG_ERR :
        sprintf(str,"open msg error");
        sprintf(sstr,"unrecognized");
        
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        if(scode == BGP4_UNSUPPORTED_VER_NO)
        {
            sprintf(sstr,"unsupported bgp version");
        }
        if(scode == BGP4_AS_UNACCEPTABLE)
        {
            sprintf(sstr,"unacceptable as num");
        }
        if(scode == BGP4_BGPID_INCORRECT)
        {
            sprintf(sstr,"incorrect bgp id");
        }
        if(scode == BGP4_OPT_PARM_UNRECOGNIZED)
        {
            sprintf(sstr,"unrecognized optional params");
        }
        if(scode == BGP4_AUTH_PROC_FAILED)
        {
            sprintf(sstr,"authentication failed");
        }
        if(scode == BGP4_HOLD_TMR_UNACCEPTABLE)
        {
            sprintf(sstr,"unacceptable hold time");
        }
        if(scode == BGP4_UNSUPPORTED_CAPABILITY)
        {
            sprintf(sstr,"Unsupported Capability");
        }
        break ;
    case BGP4_UPDATE_MSG_ERR :
        sprintf(str,"update msg err");
        
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        if(scode == BGP4_MALFORMED_ATTR_LIST)
        {
            sprintf(sstr,"malformed attribue list");
        }
        if(scode == BGP4_UNRECOGNISED_WELLKNOWN_ATTR)
        {
            sprintf(sstr,"unrecognized wellknown attribute");
        }
        if(scode == BGP4_MISSING_WELLKNOWN_ATTR)
        {
            sprintf(sstr,"missing wellknown attribute");
        }
        if(scode == BGP4_ATTR_FLAG_ERR)
        {
            sprintf(sstr,"bad attribute flag");
        }
        if(scode == BGP4_ATTR_LEN_ERR)
        {
            sprintf(sstr,"bad attribute len");
        }
        if(scode == BGP4_INVALID_ORIGIN)
        {
            sprintf(sstr,"invalid origin");
        }
        if(scode == BGP4_AS_ROUTING_LOOP)
        {
            sprintf(sstr,"as routing loop");
        }
        if(scode == BGP4_INVALID_NEXTHOP)
        {
            sprintf(sstr,"invalid nexthop");
        }
        if(scode == BGP4_OPTIONAL_ATTR_ERR)
        {
            sprintf(sstr,"bad optional attribute");
        }
        if(scode == BGP4_INVALID_NLRI)
        {
            sprintf(sstr,"invalid nlri");
        }
        if(scode == BGP4_MALFORMED_AS_PATH)
        {
            sprintf(sstr,"malformed as path");
        }
        if(scode > BGP4_MALFORMED_AS_PATH)
        {
            sprintf(sstr,"unrecoginzed");
        }
        break ;
    case BGP4_HOLD_TMR_EXPIRED_ERR :
        sprintf(str,"hold Timer expired");
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        else
        {
            sprintf(sstr,"unrecoginzed");
        }
        break ;
    case BGP4_FSM_ERR :
        sprintf(str,"FSM err");
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        else
        {
            sprintf(sstr,"unrecoginzed");
        }        
        break ;
    case BGP4_CEASE :
        sprintf(str,"cease");
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        else
        {
            sprintf(sstr,"unrecoginzed");
        }
        break ;
    case BGP4_NOTIFY_MSG_ERR :
        sprintf(str,"notification msg err");
        if(scode == BGP4_NO_SUBCODE)
        {
            sprintf(sstr,"none");
        }
        else
        {
            sprintf(sstr,"unrecoginzed");
        }
        break ;
    default :
        sprintf(str,"unrecognized");
        sprintf(sstr,"unrecognized");
        break ;
    }
    return;
}

void bgp4_debug_packet(u_char *p_msg,u_short len)
{
    u_char marker[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; 
    u_short pkt_len ; 
    u_char msgtype ; 
    u_char *p_buf = p_msg ; 
    u_short i = 0 ;

    if ((p_msg == NULL) || (len < 19))
    {
        return ;
    }
   #if 0 
    bgp4_log(1,0,"buffer :");
    
    while (i < len)
    {
        bgp4_log(1,0,"%02x ",*(p_msg + i));
        i ++ ;
    }
  #endif  
    bgp4_log(1,0,"buffer parse:");
    
    if (memcmp(p_buf,marker,16) != 0)
    {
        bgp4_log(1,0,"Invalid BGP4 Markers");
        return;
    }
    
    p_buf += 16 ;
    /*
    total length
    */
    
    bgp4_get_2bytes(pkt_len, p_buf);
    if (pkt_len != len)
    {
        bgp4_log(1,0,"Invalid packet length %d",pkt_len);
        return ;
    }
    
    p_buf += 2 ;
    /*
    msg type
    */    
    msgtype = *p_buf ;
    p_buf++ ;
    len -= 19;
    switch(msgtype){
    case 1 :/*Open*/
        bgp4_debug_open_msg(p_buf,len);
        break;
    case 2 :/*Update*/
        bgp4_debug_update_msg(p_buf,len);    
        break;
    case 3:/*notify*/
        bgp4_debug_notify_msg(p_buf,len);        
        break;
    case 4 :/*keep alive*/
        bgp4_log(1,0,"keepalive message");
        break;
    case 5 :/*route refresh*/
        bgp4_debug_rtrefresh_msg(p_buf,len);
        break;
    default :
        bgp4_log(1,0,"unkown type message %d",msgtype); 
        break;
    }
    
    return ;
}

u_short bgp4_debug_orf_capability(u_char *p_buf,u_char caplen,short len)
{
    short caplen2 = caplen;
    u_short afi;
    u_char safi;
    u_char count;
    u_short i; 
    u_char type;
    u_char flag;
    
    /*rest length must >= cap len*/ 
    if (len < caplen) 
    {
        return 0;
    }
    /*Dump Orf Cap entry*/  
    while (caplen2 >= (5 + 2*(*(p_buf+4))))
    {
        bgp4_get_2bytes(afi,p_buf);
        p_buf += 3;
        
        safi = *p_buf;
        p_buf ++;
        
        bgp4_log(1,0,"AFI/SAFI %d/%d",afi,safi);    
        count = *p_buf;
        p_buf ++;
        bgp4_log(1,0,"Number of ORF %d",count);        
        for (i = 0 ;i < count ;i ++)
        {
            type = *p_buf;
            p_buf ++;
            
            flag = *p_buf;
            p_buf ++;
            bgp4_log(1,0,"ORF Type:%s,Flag ",
                type == BGP4_ORF_TYPE_COMMUNITY ? "Community" : (type == BGP4_ORF_TYPE_EXTCOMMUNITY ? "ExtCommunity" : "Unknown"));             
            if (flag & BGP4_ORF_SEND_FLAG)    
            {
                bgp4_log(1,0,"Send ");
            }
            if (flag & BGP4_ORF_RECV_FLAG)    
            {
                bgp4_log(1,0,"Recv ");
            }
        }
        caplen2 -= (5 + 2*(*(p_buf+4)));
    }
    return 1;  
}

void bgp4_debug_open_msg(u_char *p_msg,u_short len)
{
    u_char ver ; 
    u_short roffset = 0 ;
    u_short as;
    u_short holdtime ;
    u_char total_optlen ; 
    u_char opt_rlen = 0;
    u_char opt_type ;
    u_char opt_len ;
    u_char cap_code;
    u_char cap_len;
    u_short afi;
    u_char safi ;
    u_char *p_tmpbuf ;
    u_int long_as;
    short tmp_optlen ;
    u_short rtime;
    u_char afflag;
    u_char *p_restart;
    u_char readlen;
    tBGP4_ADDR ip;
    u_char str[24];
    /*
    Length > 9
    */
    if (len < 9)
        return ;
    /*
    version
    */
    ver = *p_msg ;
    
    roffset++ ;
    p_msg++ ;
    
    /*AS
    */
    as = *(u_short *)p_msg ;
    
    roffset += 2 ;
    p_msg += 2 ;
    /*
    hold time
    */
    holdtime = *(u_short *)p_msg ;
    
    roffset += 2 ;
    p_msg += 2 ;
    /*
    BGP ID
    */
    ip.afi = AF_INET;
    memcpy(&ip.ip, p_msg, 4);
    roffset += 4 ;
    p_msg += 4 ;

    bgp4_log(1,0,"open msg,version %d",ver);
    bgp4_log(1,0,"as number %d,hold time %d Seconds,bgp Identifier %s,", 
        ntohs(as),ntohs(holdtime),bgp4_printf_addr(&ip, str));
    
    /*
    if any option data ?
    */
    if (roffset >= len )
    {
        return ;
    }
    
    total_optlen = *p_msg ;
    
    bgp4_log(1,0,"option length %d bytes,",total_optlen);
    
    if (total_optlen == 0)
    {
        return ;
    }
    /*
    enter a loop reading option data
    */
    p_msg ++ ;
    
    while (opt_rlen < total_optlen)
    {
        opt_type = *p_msg ;
        opt_len = *(p_msg + 1);
        tmp_optlen = opt_len ;  
        /*
        only support capability advertisment 2
        */
        if (opt_type == 2)
        {
            bgp4_log(1,0,"option type %d(Cap-Adv),option length %d",opt_type,opt_len);     
            
            p_tmpbuf = p_msg + 2 ;
            
            while (tmp_optlen >= 2)   
            {
                cap_code = *p_tmpbuf;
                cap_len = *(p_tmpbuf + 1);
                switch(cap_code) {
                case 1:/*MPBGP AFI/SAFI,length must be 4*/
                    bgp4_log(1,0,"cap code %d(mp-bgp) ,cap length %d",cap_code,cap_len);
                    if (cap_len == 4)
                    {
                        afi = *(p_tmpbuf + 3);/*an error,but has no other mean*/
                        safi = *(p_tmpbuf + 5) ;
                        bgp4_log(1,0,",support afi/safi %d/%d",afi,safi); 
                    }
                    else
                    {
                        bgp4_log(1,0," length invalid");
                    }
                    break;
                case 2:/*RT refresh,length must be 0*/
                    bgp4_log(1,0,"cap code %d(rt-refresh),cap length %d",cap_code,cap_len);
                    break;
                case BGP4_CAP_ORF : /*Outbound route filter*/
                    bgp4_log(1,0,"cap code %d(Out Route Filter),cap length %d",cap_code,cap_len);
                    if (bgp4_debug_orf_capability(p_tmpbuf+2,cap_len,tmp_optlen-2) == 0)
                    {
                        return ;
                    }
                    break;
                case BGP4_CAP_4BYTE_AS :/*4bytes as number,length must be 4*/ 
                    bgp4_log(1,0,"cap code %d(4bytes-AS) ,cap length %d",cap_code,cap_len);
                    if (cap_len == 4)
                    {
                        memcpy(&long_as,p_tmpbuf+2,4);
                        long_as = ntohl(long_as);
                        bgp4_log(1,0,",4Bytes AS Number %u",long_as); 
                    }
                    else
                    {
                        bgp4_log(1,0," length invalid");
                    }
                    break;
                case BGP4_CAP_GRACEFUL_RESTART :
                    bgp4_log(1,0,"cap code %d(Graceful Restart) ,cap length %d",cap_code,cap_len);
                    
                    if (((cap_len+2) > tmp_optlen) || (((cap_len-2)%4) != 0) || (cap_len < 2))
                    {
                        bgp4_log(1,0," length invalid");
                    }
                    else
                    {
                        p_restart = p_tmpbuf+2;
                        readlen = cap_len ;
                        bgp4_get_2bytes(rtime, p_restart);
                        bgp4_log(1,0,"Reset Bit:%d,Reset Time:%d",(rtime&0x8000) == 0 ? 0 : 1,(rtime&0x0fff)); 
                        readlen -= 2;
                        p_restart += 2;
                        
                        while (readlen > 0)
                        {
                            bgp4_get_2bytes(afi,p_restart);
                            p_restart += 2;
                            
                            safi = *p_restart;
                            p_restart++;
                            
                            afflag = *p_restart;
                            p_restart++;
                            bgp4_log(1,0,"Afi/Safi:%d/%d,Forwarding Bit:%d",afi,safi,(afflag&0x80) == 0 ? 0 : 1); 
                            readlen -= 4;
                        }
                    }
                    break;
                default :
                    bgp4_log(1,0,"cap code %d ,cap length %d",
                        cap_code,cap_len);
                    break;
                }
                
                tmp_optlen -= (cap_len + 2);
                p_tmpbuf +=  (cap_len + 2) ;   
            }
        }
        else if (opt_type == 1)/*add support for authentication.*/
        {
            bgp4_log(1,0,"option type %d(authentication),option length %d,auth code %d",opt_type,opt_len,*(p_msg + 2));
        }
        else
        {
            bgp4_log(1,0,"option type %d,option length %d",opt_type,opt_len);
        }
        p_msg += (2 + opt_len) ;
        opt_rlen += (2 + opt_len) ;  
    }

    return;
  
}

void bgp4_debug_rtrefresh_msg(u_char *p_msg,u_short len)
{
    u_short afi;
    u_char safi;
    u_char flag;
    short   restlen; 
    u_short orf_len;
    u_short readlen;
    u_char *p_buf;
    u_char orf_type;
    u_char orf_flag;    
    u_int com;
    
    if (len < 4)
    {
        return ;
    }
    bgp4_log(1,0,"route refresh msg,length %d,",len);
    memcpy(&afi,p_msg,2);  
    afi = ntohs(afi);
    safi = *(p_msg +3);
    bgp4_log(1,0,"AFI/SAFI %d/%d,",afi,safi);
    
    /*if length > 4,mean orf followed*/ 
    if (len > 4)
    {
        flag = *(p_msg+4);
        
        bgp4_log(1,0,"ORF When-to-Refresh Flag:%s",
                flag == 1 ? "Immediate" : (flag == 2 ? "Deffr" : "Unknown"));
        
        p_buf = p_msg+5;
        restlen = len - 5;  
        /*Orf Entrys*/ 
        while (restlen > 0)
        {
            bgp4_get_2bytes(orf_len,(p_buf+1));
            if (restlen < (orf_len+3))
            {
                return ;
            }
            restlen -= (orf_len+3);
            
            orf_type = *p_buf;
            p_buf++;
            bgp4_log(1,0,"ORF Type:%d-%s,",orf_type,
                orf_type == 2 ? "Community" : (orf_type == 3 ? "ExtCommunity" : "Unknown"));
            
            /*Length obtained*/     
            p_buf += 2;
            bgp4_log(1,0,"ORF Length:%d",orf_len);
            
            /*READ*/  
            readlen = 0 ;
            while(readlen < orf_len)
            {
                /*1byte flag*/
                orf_flag = *p_buf;
                p_buf ++;
                
                if ((orf_flag&0xc0) == 0x00)
                {
                    bgp4_log(1,0,"ORF Action:Add");
                }   
                else if ((orf_flag&0xc0) == 0x40)
                {
                    bgp4_log(1,0,"ORF Action:Remove");    
                }
                else if ((orf_flag&0xc0) == 0x80)
                {
                    bgp4_log(1,0,"ORF Action:Remove All");
                }
                else  
                {
                    bgp4_log(1,0,"ORF Action:Unkown");
                }
                
                if ((orf_flag&0x20) == 0x00)
                {
                    bgp4_log(1,0,"ORF Match:Permit");
                }
                else
                {
                    bgp4_log(1,0,"ORF Match:Deny");
                }
                
                /*remove all not contain any value*/
                if ((orf_flag&0xc0) == 0x80)
                {
                    readlen++;
                    continue;      
                }
                if (orf_type == 2)
                {
                    bgp4_get_4bytes(com, p_buf);
                    p_buf += 4;
                    bgp4_log(1,0,"Community:%u",com);  
                    readlen += 5;
                }
                else if (orf_type == 3)
                {
                    bgp4_log(1,0,"ExtCommunity:%02x %02x %02x %02x %02x %02x %02x %02x ",
                        p_buf[0],p_buf[1],p_buf[2],p_buf[3],p_buf[4],p_buf[5],p_buf[6],p_buf[7]);  
                    p_buf += 8;
                    readlen += 9;
                }
                else
                {
                    readlen++;
                }
            }
        }
    }
}

void bgp4_debug_ipv4_prefix(u_char *p_msg,u_char plen)
{
    tBGP4_ADDR ip;
    u_char str[24];
    u_char prefix[4];
    u_char byte;
    u_char bit;
    u_char mask[8]= {0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe};
    
    if (plen > 32) 
    {
        return ;  
    }
    
    byte = plen/8 ;
    bit =  plen%8 ;
    
    *(u_int *)prefix = 0;
    /*byte*/      
    if (byte != 0)     
    {
        memcpy(prefix,p_msg,byte);  
    }
    
    if (bit != 0)    
    {
        prefix[byte] = (*(p_msg + byte))& mask[bit];
    }
    ip.afi = AF_INET;
    *(u_int*)ip.ip = *(u_int*)prefix;
    bgp4_log(1,0,",route %s/%d",bgp4_printf_addr(&ip, str),plen);
}
/******************************************************************************
Function Name : bgp4_debug_vpn_rd                                                  
Description   :
damp 8 byte rd
Input(s)      : packet,packet len                   
Output(s)    : none
Return(s)    : none                                                       
******************************************************************************/
void bgp4_debug_vpn_rd(u_char *p_msg)
{
    u_char *p_buf = p_msg ;
    u_short type ;
    u_short as ;
    u_int num ; 
    u_short lnum ;
    tBGP4_ADDR ip;
    u_char str[24];

    if (p_msg == NULL)
    {
        return ;  
    }
    
    bgp4_get_2bytes(type, p_buf);
    p_buf += 2;
    
    if (type == 0)
    {
        bgp4_get_2bytes(as, p_buf);
        
        p_buf += 2 ;
        
        bgp4_get_4bytes(num, p_buf);
        
        p_buf += 4;          
        
        bgp4_log(1,0,"rd %d:%d",as,num);       
    }
    else
    {
        ip.afi = AF_INET;
        *(u_int*)ip.ip = (*(u_int *)(p_buf)) ; 
        bgp4_printf_addr(&ip, str);
        p_buf += 4 ;
        
        lnum = ntohs(*(u_short *)(p_buf));
        bgp4_get_2bytes(lnum, p_buf);
        
        p_buf += 2 ;
        bgp4_log(1,0,"rd %s:%d",str,lnum); 
    }     
}

void bgp4_debug_ipv6_prefix(u_char *p_msg,u_char plen)
{
    tBGP4_ADDR ip;
    u_char  prefix[16];
    u_char str[64];
    u_char byte;
    u_char bit; 
    u_char mask[8]= {0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe};
    
    if ((p_msg == NULL) || (plen > 128)) 
    {
        return ;  
    }
    
    byte = plen/8 ;
    bit =  plen%8 ;
    
    memset(prefix,0,16);
    /*byte*/      
    if (byte != 0)     
    {
        memcpy(prefix,p_msg,byte);  
    }
    
    if (bit != 0)    
    {
        prefix[byte] = (*(p_msg + byte))& mask[bit];
    }

    ip.afi = AF_INET6;
    memcpy(ip.ip, prefix, 16);
    bgp4_log(1,0,"prefix %s/%d",bgp4_printf_addr(&ip, str),plen);
}


short bgp4_debug_mp_nlri_route(u_char *p_msg,u_char code)
{   
    u_char *p_buf = p_msg ;
    u_int lbl ;
    u_char bit ;
    u_char byte ;   
    
    if (p_msg == NULL)
    {
        return -1;
    }
    
    /*get prefix bit len,and byte len*/    
    bit = *(p_buf);
    p_buf ++ ;
    
    bgp4_log(1,0,"prefix length %d",bit);
    
    byte = bit/8 ;
    
    if ((bit % 8) != 0)
    {
        byte ++ ;  
    }
    
    switch(code){
        case BGP4_PF_IPUCAST   :      /*0<= prefix len <= 32*/
        case BGP4_PF_IPMCAST :    
            if ((bit > 32))
            {
                return -1 ; 
            }
            bgp4_debug_ipv4_prefix(p_buf,bit);           
            break ;
        case BGP4_PF_IP6UCAST   :/*pure ipv6 prefix,0<= len <= 128*/
        case BGP4_PF_IP6MCAST :     
            if ((bit > 128))
            {
                bgp4_log(1,0,"invalid ipv6 prefix length %d",bit);
                return -1 ; 
            }
            
            bgp4_debug_ipv6_prefix(p_buf,bit);         
            
            break;
        case BGP4_PF_IP6LABEL     :/*3 bytes label + prefix
                                        24 <= <= 152
                                    */      
            if ((bit < 24) || (bit > 152))                                
            {
                return -1; 
            }
                
            lbl = ntohl(*(u_int *)(p_buf)) ;
            lbl = lbl >> 12 ;
            bgp4_log(1,0,"label %d",lbl);          
                
            p_buf += 3 ;
                
            bgp4_debug_ipv6_prefix(p_buf,bit - 24);                                      
            break;
        case BGP4_PF_IP6VPN :/*currently,do not consider it*/        
            break;
                
        default :
            break;         
    }
    
    return (byte + 1)/*include prefixlen byte*/   ;
}

/******************************************************************************
Function Name : bgp4_debug_update_msg                                                  
Description   :
damp mpbgp reachable nlri attribute
Input(s)      : packet,packet len                   
Output(s)    : none
Return(s)    : none                                                       
******************************************************************************/
void bgp4_debug_mp_reach_nlri(u_char *p_msg,u_short len)
{
    u_char *p_buf =  p_msg ;
    u_char *p_last = (p_msg + len) ;
    u_short afi ;
    u_char safi ;
    u_char mp_code = 0 ;
    u_char nexthop_len ;  
    u_char addr[64];
    u_char snpa_num;
    u_char snpa_len;
    signed short nlri_len;
    tBGP4_ADDR ip;
    u_char str[64];

    if ((p_msg == NULL)||(len < 5))
    {
        return ;
    }
    
    bgp4_get_2bytes(afi, p_buf);
    p_buf +=2 ;
    
    safi = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,0,"AFI/SAFI %d/%d",afi,safi);
    
    /*get mpcode*/
    if (afi == 1)/*ipv4*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IPUCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IPMCAST;
            break ;           
        case 128:/**/
            mp_code = BGP4_PF_IPVPN;
            break ;           
        default :
            break ;         
        }
    }
    else if (afi == 2)/*ipv6*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IP6UCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IP6MCAST;
            break ;           
        case 4:/*label*/
            mp_code = BGP4_PF_IP6LABEL ;
            break ;              
        case 128:/**/
            mp_code = BGP4_PF_IP6VPN ;
            break ;           
        default :
        break ;         
        } 
    }
    
    if (mp_code == 0)
    {
        bgp4_log(1,0,"Unkown AFI/SAFI");  
        return ;
    }    
    
    /*parse nexthop*/     
    nexthop_len = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,0,"length of nexthop %d",nexthop_len);
    
    if ((nexthop_len + 4) > len)
    {
        bgp4_log(1,0,"nvalid length of nexthop,exceed attribute length %d",len);  
        return ;
    }
    
    /*for different afi/safi,address is different*/     
    switch(mp_code){
    case BGP4_PF_IPUCAST :/*MUST be 4*/ 
    case BGP4_PF_IPMCAST  :
        if (nexthop_len != 4)
        {
            bgp4_log(1,0,"invalid length of nexthop");  
            return ;  
        }
        ip.afi = AF_INET;
        memcpy(&ip.ip, p_buf, 4);
        bgp4_log(1,0,"nexthop %s",bgp4_printf_addr(&ip, str)); 
        p_buf += 4 ;                 
        break;
    case BGP4_PF_IPVPN  :/*must be 12,need skip 8byts of 0*/
        if (nexthop_len != 12)
        {
            bgp4_log(1,0,"invalid length of nexthop");  
            return ;  
        }
        p_buf += 8 ;    
        ip.afi = AF_INET;
        memcpy(&ip.ip, p_buf, 4);
        bgp4_log(1,0,"nexthop %s",bgp4_printf_addr(&ip, str)); 
        p_buf += 4 ;                    
        break;  
        
    case BGP4_PF_IP6UCAST    :/*must be 16 or 32*/
    case BGP4_PF_IP6MCAST  :
        if ((nexthop_len != 16) && (nexthop_len != 32))
        {
            bgp4_log(1,0,"invalid length of nexthop");  
            return ;  
        }
        /*first,a global address*/          
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
  
        p_buf += 16 ;
        bgp4_log(1,0,"nexthop %s",addr);   
        
        if (nexthop_len == 32)
        {
            ip.afi = AF_INET6;
            memcpy(&ip.ip, p_buf, 16);
            bgp4_printf_addr(&ip, addr);
            p_buf += 16 ;
            bgp4_log(1,0,"nexthop %s",addr);     
        }
        break;
    case BGP4_PF_IP6LABEL      :/*must be 16,is a ipv4 mapped ipv6 address*/
        if ((nexthop_len != 16))
        {
            bgp4_log(1,0,"invalid length of nexthop");  
            return ;  
        }
        /*first,a global address*/           
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
        p_buf += 16 ;
        bgp4_log(1,0,"nexthop %s",addr);   
        
        break;
    case BGP4_PF_IP6VPN  :/*must be 24 or 48*/
        if ((nexthop_len != 24) && (nexthop_len != 48))
        {
            bgp4_log(1,0,"invalid length of nexthop");  
            return ;  
        }
        /*first,a global address*/           
        ip.afi = AF_INET6;
        memcpy(&ip.ip, p_buf, 16);
        bgp4_printf_addr(&ip, addr);
        p_buf += 24 ;
        bgp4_log(1,0,"nexthop %s",addr);   
        
        if (nexthop_len == 48)
        {
            ip.afi = AF_INET6;
            memcpy(&ip.ip, p_buf, 16);
            bgp4_printf_addr(&ip, addr);
            p_buf += 24 ;
            bgp4_log(1,0,"nexthop %s",addr);     
        }
        break;
    default :
        return ;
    }
    
    /*SNPA,skip*/   
    snpa_num = *(p_buf);
    p_buf ++ ;
    
    bgp4_log(1,0,"number of NSPA %d",snpa_num);   
    
    while ((snpa_num > 0)&&((u_long)p_buf < (u_long)p_last))
    {
        snpa_len = *(p_buf) ;
        p_buf ++ ;
        
        p_buf += snpa_len ;
        snpa_num -- ;      
    }
    
    /*NLRI information,for different afi/safi,result is different*/    
    while ((u_long)p_buf < (u_long)p_last)
    {
        nlri_len = bgp4_debug_mp_nlri_route(p_buf,mp_code);   
        if (nlri_len <0)
        break;  
        
        p_buf += nlri_len ;  
    }

    return;
}

void bgp4_debug_mp_unreach_nlri(u_char *p_msg,u_short len)
{
    u_char *p_buf = p_msg ;
    u_char *p_last = (p_msg + len);  
    u_short afi ;
    u_char safi ;
    u_char mp_code = 0 ;
    short   nlrilen ; 
    if ((p_msg == NULL) || (len < 3)) 
    {
        bgp4_log(1,0,"invalid length");  
        return ;
    }
    
    /*getting AFI and SAFI*/
    bgp4_get_2bytes(afi, p_buf);
    p_buf +=2 ;
    
    safi = *(p_buf) ;
    p_buf ++ ;    
    
    bgp4_log(1,0,"AFI/SAFI %d/%d",afi,safi);
    
    /*get mpcode*/
    if (afi == 1)/*ipv4*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IPUCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IPMCAST ;
            break ;           
        case 128:/**/
            mp_code = BGP4_PF_IPVPN;
            break ;           
        default :
            break ;         
        }
    }
    else if (afi == 2)/*ipv6*/
    {
        switch (safi){
        case 1:/*unicast*/
            mp_code = BGP4_PF_IP6UCAST ;
            break ;  
        case 2:/*mcast*/
            mp_code = BGP4_PF_IP6MCAST ;
            break ;           
        case 4:/*label*/
            mp_code = BGP4_PF_IP6LABEL ;
            break ;              
        case 128:/**/
            mp_code = BGP4_PF_IP6VPN ;
            break ;           
        default :
            break ;         
        } 
    }
    
    if (mp_code == 0)
    {
        bgp4_log(1,0,"Unkown AFI/SAFI");  
        return ;
    }    
    
    /*NLRI information,for different afi/safi,result is different*/    
    while ((u_long)p_buf < (u_long)p_last)
    {
        nlrilen = bgp4_debug_mp_nlri_route(p_buf,mp_code);     
        if (nlrilen < 0)  
        {
            return ;
        }
        
        p_buf += nlrilen;
    }

    return;
}

void bgp4_debug_update_msg(u_char *p_msg,u_short len)
{
    tBGP4_ADDR ip;
    short   unfea_len ; 
    short   total_attr_len ; 
    short   nlrilen ;
    u_char plen;
    u_char byte;
    u_char aflag ;
    u_char acode ;
    u_short alen ;
    u_short rlen ;
    u_char origin ;
    u_char segtype;
    u_char seglen;
    u_short as;
    u_int med;
    u_char str[64];
    u_char pathAttr[96];
    u_char space = 32;/*ASCII of space*/
    

    bgp4_log(1,0,"update message");
    bgp4_get_2bytes(unfea_len, p_msg);
    p_msg += 2 ;
    
    bgp4_log(1,0,"unfeasible routes length %dbyte(s)",(unfea_len));
    
    if (unfea_len > (len - 2))
    {
        return ;
    }
    
    if (unfea_len > 0)
    {
        rlen =  0 ;
        while (rlen < unfea_len)
        {
            plen = *p_msg ;
            
            p_msg++ ;
            
            if(plen > 32)
            {
                bgp4_log(1,0,"invalid prefix length %d",plen);
                return ;
            }
            
            byte = plen/8 ;
            
            if (plen%8 != 0)
            {
                byte++ ;
            }
            ip.afi = AF_INET;
            *(u_int*)ip.ip = 0 ;
            memcpy(ip.ip, p_msg, byte);
            bgp4_printf_addr(&ip, str);
            bgp4_log(1,0,"unfeasible route %s/%d",str,plen);
            /*Get Prefix*/
            p_msg += byte ;
            rlen += (byte + 1);     
        }
    }
    
    /*
    total path attribute length
    */    
    bgp4_get_2bytes(total_attr_len, p_msg);
    p_msg += 2 ;
    
    bgp4_log(1,0,"total path attribute length %dbyte(s)",total_attr_len);
    
    if (total_attr_len > (len - 4 - unfea_len) ) 
    {
        return ;
    }
    if (total_attr_len > 0)
    {
        rlen = 0 ;
    
        while (rlen < total_attr_len)
        {
            /*
            0                   1
            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |  Attr. Flags  |Attr. Type Code|
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            */
            aflag = *p_msg ;
            p_msg ++ ; 
            
            acode = *p_msg ;
            p_msg ++ ; 

            memset(pathAttr,0,96);         
            /*
            flag : bit 0 :1 optional,0 well-known
            bit 1 :1 transitive ,0 non-transitive
            bit2 : 1 partial,0 complete
            bit3 : 1 ,length 2bytes,0 length 1byte
            */   
            sprintf(pathAttr, "attribute flag %#x",aflag);
            
            if (aflag & 0x80)   
            {
                strcat(pathAttr , "(optional,");
            }
            else
            {
                strcat(pathAttr , "(well-known,");
            }
            
            if (aflag & 0x40)   
            {
                strcat(pathAttr , "transitive,");
            }
            else
            {
                strcat(pathAttr , "non-transitive,");
            }
            
            if (aflag & 0x20)   
            {
                strcat(pathAttr , "partial,");
            }
            else
            {
                strcat(pathAttr , "complete,");
            }
            
            if (aflag & 0x10)   
            {
                strcat(pathAttr , "2bytes length)");
            }
            else
            {
                strcat(pathAttr , "1bytes length)");
            }

            bgp4_log(1,0,"%s",pathAttr);
            
            /*get length*/
            if (aflag & 0x10)
            {
                bgp4_get_2bytes(alen, p_msg);
                p_msg += 2 ;  
                rlen += 4 ;
            }
            else
            {
                alen = *p_msg ;
                p_msg++ ;
                rlen += 3 ;  
            }
            /*
            enter path attributes
            */      
            switch (acode ) {
            case 1 :/*Origin*/
                bgp4_log(1,0,"attribute type %d,origin,attribute length %d",acode,alen);
                origin = *p_msg ;
                bgp4_log(1,0,"origin %d %s",origin,
                    (origin == 0 ) ? "IGP" : ((origin == 1 ) ? "EGP" : ((origin == 2 ) ? "INCOMPLETE" : "Invalid")));               
                break;
            case 2 :/*AS Path*/ 
                bgp4_log(1,0,"attribute type %d,as path,attribute length %d",acode,alen);
                {
                    u_short readlen = 0 ;
                    while (readlen < alen)
                    {
                        segtype = *(p_msg + readlen) ;
                        seglen = *(p_msg + readlen + 1) ;  
                        
                        bgp4_log(1,0,"path type %d(%s),as count %d",segtype,
                            segtype == 1 ? "AS_SET" : "AS_SEQUENCE",seglen);
                        
                        readlen += 2 ;
                        while (seglen > 0) 
                        {
                            bgp4_get_2bytes(as, (p_msg + readlen));
                            bgp4_log(1,0,"as number %d",as);
                            readlen += 2 ;
                            seglen -- ;
                        }
                        
                    }
                }           
                break ;
            case 3 :/*Next hop*/
                bgp4_log(1,0,"attribute type %d,nexthop,attribute length %d",acode,alen);
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg, 4);               
                bgp4_log(1,0,"nexthop %s",bgp4_printf_addr(&ip, str));
                break;
            case 4 :/*MED*/
                bgp4_log(1,0,"attribute type %d,MED,attribute length %d",acode,alen);
                bgp4_get_4bytes(med, (p_msg));
                
                bgp4_log(1,0,"MED %d",med);             
                break;
            case 5 :/*Local Pref*/
                bgp4_log(1,0,"attribute type %d,Local Pref,attribute length %d",acode,alen);
                bgp4_get_4bytes(med, (p_msg));     
                bgp4_log(1,0,"local preference  %d",med);               
                break;
            case 6 :/*ATOMIC_AGGREGATE*/
                bgp4_log(1,0,"attribute type %d,Atomic Aggregate,attribute length %d",acode,alen);
                /*
                Length is 0,no value
                */
                break;
            case 7 :/*AGGREGATOR*/
                bgp4_log(1,0,"attribute type %d,Aggregator,attribute length %d",acode,alen);
                /*
                Length is 6,AS+IP
                */
                bgp4_get_2bytes(as, p_msg);
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg+2, 4);

                bgp4_log(1,0,"as number %d,address %s",
                    as,bgp4_printf_addr(&ip, str));
                break;
            case 9 :/*originator id*/
                bgp4_log(1,0,"attribute type %d,Originator-id,attribute length %d",acode,alen);
                /*4 bytes id*/
                ip.afi = AF_INET;
                memcpy(&ip.ip, p_msg, 4);
                
                bgp4_log(1,0,"Originator-id %s",bgp4_printf_addr(&ip, str));
                
                break;
            case 10 :/*CLUSTER_LIST*/
                bgp4_log(1,0,"attribute type %d,Cluster List,attribute length %d",acode,alen);              
                /*
                multiple 4 bytes id
                */
                {
                    u_short readlen = 0 ;
                    
                    while (readlen < alen)
                    {
                        ip.afi = AF_INET;
                        memcpy(&ip.ip, (p_msg + readlen), 4);
                        bgp4_log(1,0,"Cluster-id %s",bgp4_printf_addr(&ip, str));
                        
                        readlen += 4 ;
                    }
                }
                
                break;
                /*add support for route reflector .ddp E*/   
            case 14 :/*MP_REACH_NLRI*/
                bgp4_log(1,0,"attribute type %d,MP Reachable NLRI,attribute length %d",acode,alen);
                bgp4_debug_mp_reach_nlri(p_msg,alen);
                break ;
            case 15 :/*MP_UNREACH_NLRI*/
                bgp4_log(1,0,"attribute type %d,MP Unreachable NLRI,attribute length %d",acode,alen);
                bgp4_debug_mp_unreach_nlri(p_msg,alen);
                break;
            case 16 :/*ex-community*/               
                bgp4_log(1,0,"attribute type %d,Extension Community,attribute length %d",acode,alen);
                /*
                just consider RT 00 02
                */
                {
                    u_short readlen = 0;
                    
                    u_char etype ;
                    u_char subetype ;            
                    u_char ebuf[32];
                    u_char exCmn[96];
                    u_char Type[16];
                    
                    while (readlen < alen)
                    {
                        etype = *(p_msg + readlen) ;
                        subetype = *(p_msg + readlen + 1) ;    
                        memset(exCmn,0,96);
                        memset(Type,0,16);
                        sprintf(exCmn,"community type %d ",etype);
                        
                        if (etype & 0x80)             
                        {
                            strcat(exCmn,",IANA-first come first serve");
                        }
                        else
                        {
                            strcat(exCmn,",IANA-IETF consensus");
                        }
                        
                        if (etype & 0x40)             
                        {
                            strcat(exCmn,",non-transitive");
                        }
                        else
                        {
                            strcat(exCmn,",transitive");
                        }
                        /*type:
                        0:2+4
                        1:4+2
                        2:4+2 as
                        */    
                        etype &= 0x3f ;
                        switch (etype){
                        case 0 :
                            strcat(exCmn,",2bytes as number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 2));
                            bgp4_get_4bytes(med,(p_msg + readlen + 4));                   
                            sprintf(ebuf,"%d:%d",as,med); 
                            break ;
                        case 1 :
                            strcat(exCmn,",4bytes ip number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 6));
                            ip.afi = AF_INET;
                            memcpy(&ip.ip, (p_msg + readlen + 2), 4);
                            sprintf(ebuf,"%s:%d",bgp4_printf_addr(&ip, str),as);             
                            
                            break ;                 
                        case 2 :   
                            strcat(exCmn,",4bytes as number");
                            bgp4_get_2bytes(as, (p_msg + readlen + 6));
                            bgp4_get_4bytes(med,(p_msg + readlen + 2));                                           
                            sprintf(ebuf,"%d:%d",med,as);
                            break ;                  
                        default :  
                            strcat(exCmn,",unkown");
                            ebuf[0] = '\0';/*sprintf(au1Buf,"");*/                  
                            break ;                
                        }
                        
                        bgp4_log(1,0,"%s",exCmn);
                        /*
                        sub type
                        */                          
                        switch (subetype){
                        case 2 :
                            sprintf(Type,"route-target %s",ebuf);
                            break ;
                        default :
                            sprintf(Type,"unkown");
                            break ;                
                        }
                        bgp4_log(1,0,"community sub-type %d  %s",subetype,Type);
                        readlen += 8 ;
                    }                        
                    
                }
                break;
            default :
                bgp4_log(1,0,"attribute type %d,attribute length %d",acode,alen);
                break;    
            }
    
            p_msg += alen;
            rlen += alen;    
        }
    }
  
    nlrilen = len - 4 - unfea_len - total_attr_len ;
  
    if (nlrilen > 0) 
    {
        rlen =  0 ;
        while (rlen < nlrilen)
        {
            plen = *p_msg ;
          
            p_msg++ ;
          
            if(plen > 32)
            {
                bgp4_log(1,0,"invalid prefix length %d",plen);
                return ;
            }
          
            byte = plen/8 ;
          
            if (plen%8 != 0)
            {
                byte++ ;
            }

            ip.afi = AF_INET;
            *(u_int*)ip.ip = 0 ;
            memcpy(&ip.ip, p_msg, byte);
            bgp4_printf_addr(&ip, str);
            bgp4_log(1,0,"route %s/%d",str,plen); 
            p_msg += byte ;
            rlen += (byte + 1);     
        }  
    }
    else
    {
        bgp4_log(1,0,"no NLRI Information");
    }

    return;
}

void bgp4_debug_notify_msg(u_char *p_msg,u_short len)
{
    u_char code ;
    u_char scode;
    u_short roffset = 0 ;
    u_short i ;
    u_char estr[32];
    u_char sestr[64];
    
    if (len < 2)
    {
        return ;
    }
    
    code = *(p_msg) ;
    scode = *(p_msg + 1) ;
    bgp4_printf_notify(code, scode, estr, sestr) ;
    
    bgp4_log(1,0,"notification msg,error code %d(%s),sub error code %d(%s)",
        code,estr,scode,sestr);
    /*error data*/
    bgp4_log(1,0,"total error data length %d byte(s)",len - 2);
    
    roffset = 2 ;
    i = 0 ;
    
    while (roffset < len)
    {
#if 0
        bgp4_log(1,0,"%s%02x ",(i%16) ? "" : "\n\r          ",*(p_msg + roffset));
        i ++ ;
        roffset ++ ;  
#endif
        bgp4_log(BGP_DEBUG_PKT,0,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",
            *(p_msg + roffset),*(p_msg + roffset +1),*(p_msg + roffset +2),*(p_msg + roffset +3),
            *(p_msg + roffset +4),*(p_msg + roffset +5),*(p_msg + roffset +6),*(p_msg + roffset +7),
            *(p_msg + roffset +8),*(p_msg + roffset +9),*(p_msg + roffset +10),*(p_msg + roffset +11),
            *(p_msg + roffset +12),*(p_msg + roffset +13),*(p_msg + roffset +14),*(p_msg + roffset +15));
        
        roffset +=16;       
    }

    return;
}

 u_char * bgp4_printf_syserror(u_int code,u_char *str)
 {
    if (!str)
    {
        return NULL;
    }
#if !defined(WIN32)     
    switch (code){
    case EDESTADDRREQ:
        sprintf(str,"Destination address required");
        break;
    case ENETUNREACH :
        sprintf(str,"Network is unreachable");
        break;
    case ENETRESET :
        sprintf(str,"Network dropped connection on reset");
        break;  
    case ECONNABORTED :
        sprintf(str,"Software caused connection abort");
        break;    
    case ECONNRESET :
        sprintf(str,"Connection reset by peer");
        break;
    case ENOBUFS :
        sprintf(str,"No buffer space available");
        break;    
    case EISCONN :
        sprintf(str,"Socket is already connected");
        break;    
    case ENOTCONN :
        sprintf(str,"Socket is not connected");
        break;  
    case ESHUTDOWN :
        sprintf(str,"Can't send after socket shutdown");
        break;  
    case ETOOMANYREFS :
        sprintf(str,"Too many references: can't splice");
        break;      
    case ETIMEDOUT :
        sprintf(str,"Connection timed out");
        break;      
    case ECONNREFUSED :
        sprintf(str,"Connection refused");
        break;    
    case ENETDOWN :
        sprintf(str,"Network is down");
        break;    
    case EHOSTUNREACH :
        sprintf(str,"No route to host");
        break;      
    case EINPROGRESS :
        sprintf(str,"Operation now in progress");
        break;  
    case EALREADY :
        sprintf(str,"Operation already in progress");
        break;      
    case EWOULDBLOCK :
        sprintf(str,"Operation would block");
        break;        
    case EINVAL:
        sprintf(str,"Invalid argument");  
        break; 
    case EHOSTDOWN :
        sprintf(str,"Host is down"); 
        break; 
    case 3997700 :
        sprintf(str,"Object Time Out");  
        break; 
    case 3997698 :
        sprintf(str,"Object Unavaliable"); 
        break; 
    case 851971 :
        sprintf(str,"Invalid FileDescriptor");  
        break;  
    default :
        sprintf(str,"Unkown(%d)",code); 
        break; 
    }
#else
    sprintf(str, "%d", code);
#endif
    return str;
}


void bgp4_printf_af(u_int u4AfFlag,u_char *str)
{
    u_int u4Flag[9] = {BGP4_PF_IPUCAST ,
                        BGP4_PF_IPMCAST,
                        BGP4_PF_IPLABEL,
                        BGP4_PF_IPVPN,
                        BGP4_PF_IP6UCAST,
                        BGP4_PF_IP6MCAST,
                        BGP4_PF_IP6LABEL,
                        BGP4_PF_IP6VPN,
                        BGP4_PF_L2VPLS};
    u_char *afiString[9] = {"ip.unicast ","ip.mcast ","ip.label ","ip.vpn ",
                            "ipv6.unicast ", "ipv6.mcast ","ipv6.label ","ipv6.vpn ","vpls "};
    u_char i ;  
    str[0] = '\0';
    
    
    for (i = 0 ;i < 9 ;i++)
    {
        if (af_isset(u4AfFlag, u4Flag[i]))  
        {
            strcat(str,afiString[i])  ;
        }
    }

    return;
}

static char* route_desc[BGP4_MAX_PROTO] = 
{
    "unknown", 
    "unknown",  
    "connect",  
    "static",    
    "unknown",     
    "unknown",      
    "unknown",     
    "unknown",     
    "rip",     
    "is-is",     
    "unknown",     
    "unknown",     
    "unknown",     
    "ospf",  
    "bgp",  
    "unknown"
};

char* bgp4_get_route_desc(u_int proto )
{
    if (proto >= BGP4_MAX_PROTO)
    {
        return route_desc[0];
    }
    else
    {
        return route_desc[proto];
    }
}


void bgp4_printf_aspath(tBGP4_PATH *p_path,u_char *pstring)
{
    u_char flag = 0;
    tBGP4_ASPATH *p_aspath;
    u_char *type[5] = {"none","set ","seq ","con-seq ","con-set "};
    u_short as;
    u_short i;
    u_char u1_path[8];

    sprintf(pstring," ");
    
    bgp4_avl_for_each(&p_path->aspath_list, p_aspath
    {
        if (p_aspath->p_asseg == NULL)
        {
            continue ;
        }
        flag = 1;        
        /*type*/
        strcat(pstring,type[p_aspath->type]);
        for (i = 0;i < p_aspath->len;i++)
        {
            memcpy(&as,p_aspath->p_asseg+(2*i),2);
            sprintf(u1_path,"%d,",as);
            strcat(pstring,u1_path);   
        }
    }
    
    if (flag == 0)
    {
        sprintf(pstring," ");
    }

    return;
}
u_char * bgp4_printf_state(u_short state , u_char *p_str)
{
    /*
    BGP4_NS_NONE                        0
    BGP4_NS_IDLE                      1
    BGP4_NS_CONNECT                   2
    BGP4_NS_ACTIVE                    3
    BGP4_NS_OPENSENT                  4
    BGP4_NS_OPENCONFIRM               5
    BGP4_NS_ESTABLISH               6
    */
    
    switch(state){
    case BGP4_NS_IDLE :
        sprintf(p_str,"idle");
        break ;
    case BGP4_NS_CONNECT :
        sprintf(p_str,"connect");
        break ;
    case BGP4_NS_ACTIVE :
        sprintf(p_str,"active");
        break ;
    case BGP4_NS_OPENSENT :
        sprintf(p_str,"opensent");
        break ;
    case BGP4_NS_OPENCONFIRM :
        sprintf(p_str,"openconfirm");
        break ;
    case BGP4_NS_ESTABLISH :
        sprintf(p_str,"established");
        break ;
    default :
        sprintf(p_str,"unknown(%d)",state);
        break ;
    }
    
    return p_str;
}

u_char *bgp4_printf_event(u_short event,u_char *p_str) 
{
    if(p_str == NULL)
    {
        return NULL;
    }
        
    switch (event) {
        case BGP4_EVENT_START :
               sprintf(p_str,"start connection");
               break;    
        case BGP4_EVENT_STOP :
               sprintf(p_str,"stop connection");
               break;    
        case BGP4_EVENT_TCP_OPENED :
               sprintf(p_str,"tcp connection opened");
               break;    
        case BGP4_EVENT_TCP_CLOSED :
               sprintf(p_str,"tcp connection closed");
               break;    
        case BGP4_EVENT_CONNECT_FAIL :
               sprintf(p_str,"tcp connection open failed");
               break;    
        case BGP4_EVENT_FATAL_ERROR :
               sprintf(p_str,"tcp fatal error");
               break;    
        case BGP4_EVENT_RETRY_TIMEOUT :
               sprintf(p_str,"connect retry timer expired");
               break;    
        case BGP4_EVENT_HOLD_TMR_EXP :
               sprintf(p_str,"hold timer expired");
               break;    
        case BGP4_EVENT_KEEPALIVE_TMR_EXP :
               sprintf(p_str,"keepalive timer expired");
               break;    
        case BGP4_EVENT_GR_TMR_EXP:
             sprintf(p_str,"graceful restart timer expired");
                       break; 
        default :
            break;
    }

    return p_str;
}
#if 0
/*display process time*/
void
bgp4_show_time(u_int clear)
{
   tBGP4_ROUTE  *p_route = NULL; 
   tBGP4_ROUTE  *p_next = NULL; 
   tBGP4_PEER *p_peer = NULL;
   u_char zero[32] = {0};
   u_int un_update_add=0;
   u_int un_update_del=0;
   u_char peerstr[128];
 
   printf("\n\r");
   printf("\tTotal message input process TIMER_INVALIDs=%d\n\r", gBgp4.input_time);
   printf("\tTotal message output process TIMER_INVALIDs=%d\n\r", gBgp4.output_time);
   printf("\tTotal ip update process TIMER_INVALIDs=%d\n\r", gBgp4.ipupdate_time);
   printf("\tTotal select wait TIMER_INVALIDs=%d\n\r", gBgp4.select_time);
   printf("\tTotal msg send=%d,error=%d,total msg len=%d\n\r",gBgp4.stat.ipmsgsend,gBgp4.stat.ipmsgfail,gBgp4.stat.ipmsglen);

   printf("\trt update need=%d,recent rib route ponter=%x\n\r",gBgp4.rib_walkup_need, (u_int)gBgp4.p_route_update);

   if(gBgp4.stat.ipmsgsend!=0)
   {
       printf("\taverage msg length=%d\n\r",(gBgp4.stat.ipmsglen/gBgp4.stat.ipmsgsend));
   }
   bgp_sem_take();
   RIB_LOOP(&gBgp4.rib, p_route, p_next)
   {
        if (p_route->ip_action == BGP4_IP_ACTION_ADD) 
        {
            un_update_add++;
        }
        else if(p_route->ip_action == BGP4_IP_ACTION_DELETE)
        {
            un_update_del++;
        }
   }

   /*peer*/
   LST_LOOP(&gBgp4.peer_list, p_peer, node, tBGP4_PEER )
   {
       bgp4_printf_peer(p_peer, peerstr);
       printf("Peer:%s\n\r", peerstr);
       printf("\t Local Address family:%x\n\r", p_peer->local.af);
       printf("\t Remote Address family:%x\n\r", p_peer->remote.af);
       printf("\t Message Sent:%d\n\r", p_peer->tx_msg);
       printf("\t Message Rxd:%d\n\r", p_peer->rx_msg);
       printf("\t Length Error:%d\n\r", p_peer->msg_len_err);
       printf("\t Marker Error:%d\n\r", p_peer->msg_marker_err);       
       printf("\t Open Rxd:%d\n\r", p_peer->open_rx);              
       printf("\t Open Sent:%d\n\r", p_peer->open_tx);
       printf("\t Open Error:%d\n\r", p_peer->open_err);
       printf("\t Update Sent:%d\n\r", p_peer->tx_update);
       printf("\t Update Rxd:%d\n\r", p_peer->rx_update);       
       printf("\t Update Error:%d\n\r", p_peer->update_err); 
       printf("\t Route Discard:%d\n\r", p_peer->discard_route);
       printf("\t Feasible Rxd:%d\n\r", p_peer->rx_fea_route);
       printf("\t Feasible Sent:%d\n\r", p_peer->tx_feasible);
       printf("\t Withdraw Rxd:%d\n\r", p_peer->rx_with_route);
       printf("\t Withdraw Sent:%d\n\r", p_peer->tx_withdraw);
       
       printf("\t Notify Rxd:%d\n\r", p_peer->notify_rx); 
       printf("\t Notify Sent:%d\n\r", p_peer->notify_tx);
       printf("\t Keepalive Rxd:%d\n\r", p_peer->keepalive_rx);
       printf("\t Keepalive Sent:%d\n\r", p_peer->keepalive_tx);
       
   }
   
   bgp_sem_give();

   printf("\tun update add number=%d,un update del number=%d\n\r",un_update_add,un_update_del);   

   if (clear != 0)
   {
       gBgp4.input_time = 0 ;
       gBgp4.output_time = 0 ;
       gBgp4.ipupdate_time = 0 ;
       gBgp4.select_time = 0 ;
   }
}
#endif
void bgp4_tcp_sock_show()
{
    tBGP4_PEER *p_peer = NULL;
    u_char peerstr[128];
    tBGP4_VPN_INSTANCE* p_instance = NULL;
   
    bgp_sem_take();
    /*peer*/
    LST_LOOP(&gBgp4.vpn_instance_list, p_instance, node, tBGP4_VPN_INSTANCE)
    {
        LST_LOOP(&p_instance->peer_list, p_peer, node, tBGP4_PEER )
        {
            bgp4_printf_peer(p_peer, peerstr);
            printf("\n\rpeer %s socket %d last sync tcp socket %d",peerstr,p_peer->sock,p_peer->last_sync_fd);
        }
    }
   
    bgp_sem_give();

    printf("\r\nreceived add tcp %d,checked add tcp %d",gBgp4.add_tcp_msg_count,gBgp4.add_check_tcp_count);
    printf("\r\nreceived del tcp %d,checked del tcp %d",gBgp4.del_tcp_msg_count,gBgp4.del_check_tcp_count);
}

void bgp4_debug_row_buffer(u_char *p_buf,u_short len)
{
    u_short i = 0 ;

    if ((p_buf == NULL) || (len == 0))
    {
        return ;
    }
    bgp4_log(BGP_DEBUG_PKT,0,"row buffer :");
    
    while (i < len)
    {                       
        bgp4_log(BGP_DEBUG_PKT,0,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
                ,*(p_buf + i),*(p_buf + i +1),*(p_buf + i +2),*(p_buf + i +3),
                *(p_buf + i +4),*(p_buf + i +5),*(p_buf + i +6),*(p_buf + i +7),
                *(p_buf + i +8),*(p_buf + i +9),*(p_buf + i +10),*(p_buf + i +11),
                *(p_buf + i +12),*(p_buf + i +13),*(p_buf + i +14),*(p_buf + i +15));       
        
        i +=16;
    }
    return ;
}

#endif

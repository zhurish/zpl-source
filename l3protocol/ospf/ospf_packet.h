/* ospf_packet.h --- packet strut*/


#if !defined (_OSPF_PACKET_H_)
#define _OSPF_PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif 


/* IP TTL for OSPF protocol. */
#define OSPF_IP_TTL 1
#define OSPF_VL_IP_TTL 100
#define OSPF_MAX_TTL 255
#define OSPF_MAX_RX_MSG 256



/*control flooding count*/
#define OSPF_MIN_FLOOD_IF_NUM 3
#define OSPF_MAX_FLOOD_IF_NUM 10

/*md5 digest*/
//#define MD5 5

/*delay ack interval*/
#define OSPF_DELAY_ACK_INTERVAL 1

#define OSPF_BIGPKT_BUF ((64*1024) - 128)

/*allocate max size when sending packet*/
#define OSPF_MAX_TXBUF 2800

/*max buffer length for rx*/
#define OSPF_MAX_RXBUF (OSPF_MAX_TXBUF + 64)

/*default ip mtu*/
#define OSPF_DEFAULT_IP_MTU 1500
#define OSPF_MAX_IP_MTU 9500


/*ip protocol id*/
#define OSPF_IP_ID 89

/*default version*/
#define OSPF_VERSION 2

/*wellknown mcast address for ospf*/
#define OSPF_ADDR_ALLSPF 0xe0000005        /* 224.0.0.5 */
#define OSPF_ADDR_ALLDR 0xe0000006        /* 224.0.0.6 */

/*ospf packet common header length*/
#define OSPF_PACKET_HLEN ((u_int) 24) 

/*hello packet fixed part,do not include common header*/
#define OSPF_HELLO_HLEN ((u_int) 20) 

/*dbd packet fixed part,do not include common header*/
#define OSPF_DBD_HLEN        ((u_int) 8) 

/*update packet fixed part,do not include common header*/
#define OSPF_UPDATE_HLEN ((u_int) 4) 

/*dbd packet min length,just include packet hdr and dbd hdr*/
#define OSPF_DBD_MIN_LEN (OSPF_PACKET_HLEN + OSPF_DBD_HLEN) 

/*update packet header,no lsa contained*/
#define OSPF_UPDATE_MIN_LEN (OSPF_PACKET_HLEN + OSPF_UPDATE_HLEN) 

/*hello packet header,no neighbor contained*/
#define OSPF_HELLO_MIN_LEN   (OSPF_PACKET_HLEN + OSPF_HELLO_HLEN) 

/*unit length in dbd packet*/
#define OSPF_DBD_UNIT_LEN 20 

/*request unit length*/
#define OSPF_REQ_UNIT_LEN 12 

/*option: rsvd-opaque-dc-dna-nssa-mcast-ext-tos*/
#define ospf_option_dn(x) (((x)&0x80) ? TRUE : FALSE)
#define ospf_option_opaque(x)  (((x)&0x40) ? TRUE : FALSE)
#define ospf_option_demand(x)  (((x)&0x20) ? TRUE : FALSE)
#define ospf_option_dna(x)  (((x)&0x10) ? TRUE : FALSE)
#define ospf_option_nssa(x)  (((x)&0x08) ? TRUE : FALSE)
#define ospf_option_mcast(x)  (((x)&0x04) ? TRUE : FALSE)
#define ospf_option_external(x)  (((x)&0x02) ? TRUE : FALSE)
#define ospf_option_tos(x)  (((x)&0x01) ? TRUE : FALSE)

/*set option field*/
#define ospf_set_option_dn(x)   do{(x) |= 0x80;}while(0)
#define ospf_set_option_opaque(x)   do{(x) |= 0x40;}while(0)
#define ospf_set_option_demand(x)   do{(x) |= 0x20;}while(0)
#define ospf_set_option_dna(x)   do{(x) |= 0x10;}while(0)
#define ospf_set_option_nssa(x)   do{(x) |= 0x08;}while(0)
#define ospf_set_option_mcast(x)   do{(x) |= 0x04;}while(0)
#define ospf_set_option_external(x)   do{(x) |= 0x02;}while(0)
#define ospf_set_option_tos(x)   do{(x) |= 0x01;}while(0)

/*dd flag 5-i-m-ms*/
#define ospf_dd_flag_init(x) (((x)&0x04) ? TRUE : FALSE)
#define ospf_dd_flag_more(x) (((x)&0x02) ? TRUE : FALSE)
#define ospf_dd_flag_master(x) (((x)&0x01) ? TRUE : FALSE)

/*set dd flag*/
#define ospf_set_dd_flag_init(x) do{(x) |= 0x04;}while(0)
#define ospf_set_dd_flag_more(x) do{(x) |= 0x02;}while(0)
#define ospf_set_dd_flag_master(x) do{(x) |= 0x01;}while(0)

/*clear M flag ,do not change other flags*/
#define ospf_reset_dd_flag_more(x) do{(x) &= ~0x02;}while(0)

#pragma pack(1)

/*auth field when MD5 is used*/
struct ospf_authinfo{
    /*length offset for auth digiest*/
    u_short offset;

    /*key id used*/
    u_int8 keyid;

    /*appended auth data length*/
    u_int8 len;

    /*auth sequeuce*/
    u_int seqnum;
};

/* the standard 24 byte header for OSPF (page 178) */
struct ospf_hdr{
    /* set to 2 for this implementation */    
    u_int8 version;   

    /*packet type*/
    u_int8 type; 

    /* length of entire OSPF protocol packet in bytes, 
       including the standard OSPF header*/
    u_short len;    

    /* identity of the router originating the packet */
    u_int router; 

    /* a 32 bit number identifying the area this packet belongs to */   
    u_int area;   

    /* checksum */    
    u_short checksum;  

    /*rfc6549 :muti instance*/
    u_int8 instance; 
    
    /*authentication type*/                                                    
    u_int8 auth_type; 

    /*for simple password,this field contain key;
      for md5,this field is struct authinfo*/
    u_int8 auth[OSPF_KEY_LEN];
};

#define OSPF_HELLO_NBR_LEN 4

/*ospf hello packet headr after standard ospf packet header*/
struct ospf_hello_msg{
    struct ospf_hdr h;
    
    /* network mask associated with this interface */
    u_int mask;    

    /* number of seconds between this router's Hello packets */
    u_short hello_interval;  

    u_int8 option;

    u_int8 priority;   

    u_int dead_interval;   

    /* identity of the designated router for  this network */
    u_int dr;   

    /* identity of the backup designated router for this network */
    u_int bdr;  

    /* IDs of each router from whom valid Hello packets 
        have been seen recently on this network */
    u_int nbr[0];    
} ;

/*ospf database packet head after standarc ospf packet header*/
struct ospf_dd_msg{
    struct ospf_hdr h;
     
    u_short mtu;

    u_int8 option;

    u_int8 flags;

    /*sequnce number*/
    u_int seqnum;

    /*first linkstate header*/
    struct ospf_lshdr lshdr[0];
} ; 

/*linkstate request unit in message*/
struct ospf_request_unit{
    u_int8 rsvd[3];

    /* Type of advertisement */
    u_int8 type;      

    /* Link State Id */
    u_int id;        

    /*advertise router id*/
    u_int adv_id;
} ; 

/*request message has no headr part,it is structed by request unit*/
struct ospf_request_msg{
    struct ospf_hdr h;
    
    struct ospf_request_unit lsa[0];
} ;

/*head part of update packet*/
struct ospf_update_msg{
    struct ospf_hdr h;
    
    /*lsa count in this update packet*/
    u_int lscount;

    /*first lsa buffer*/
    struct ospf_lshdr lshdr[0];
} ;

/*head part of ack packet*/
struct ospf_ack_msg {
    struct ospf_hdr h;

    struct ospf_lshdr lshdr[0];
} ;
#pragma pack()


/*update infrmation used by interface and neighbor*/
struct ospf_updateinfo{
    /*interface it belong to,if struct is used for neighbor retransmit,no interface need*/
    struct ospf_if *p_if;

    /*neighbor it belong to,if struct is used for interface flooding,no neighbor need*/
    struct ospf_nbr *p_nbr;

    /*maxlength of message buffer,set when message created*/
    u_int maxlen;

    /*buffer containing the whole update,only one update message*/
    struct ospf_update_msg *p_msg; 
};


//void ospf_ack_output(struct ospf_ackinfo *p_ack);
void ospf_input (u_int8 *p_buf,u_int len,u_int ifindex);
void ospf_output(void * p_msgbuf, struct ospf_if * p_if, u_int dest, u_short len);
void ospf_request_output(struct ospf_nbr *p_nbr);
void ospf_dd_output(struct ospf_nbr * p_nbr);
void ospf_hello_output(struct ospf_if * p_if, u_int goingdown, u_int poll_flag);
void ospf_lsa_retransmit(struct ospf_nbr * p_nbr);
void ospf_initial_dd_output (struct ospf_nbr *p_nbr);
void ospf_update_output(struct ospf_updateinfo *p_update);
void ospf_request_input(struct ospf_nbr * p_nbr, struct ospf_hdr * p_req_packet);
void ospf_update_input(struct ospf_nbr * p_nbr, struct ospf_hdr * p_update);
//void ospf_ack_input(struct ospf_nbr *p_nbr, struct ospf_hdr *p_packet) ;
void ospf_update_buffer_insert(struct ospf_updateinfo *p_update, struct ospf_lsa *p_rxmt, u_int8 *p_buf);
//void ospf_ack_add(struct ospf_ackinfo *p_ack,struct ospf_lshdr *p_lshdr) ;
void ospf_dd_input(struct ospf_nbr * p_nbr, struct ospf_hdr * p_hdr);
void ospf_hello_input(struct ospf_hdr * p_msg, struct ospf_nbr * p_nbr, struct ospf_if * p_if, u_int source);
u_int ospf_packet_dest_select(struct ospf_if * p_if, struct ospf_nbr * p_nbr, int type);
u_int ospf_dcn_creat_nbr(struct ospf_if *p_if,u_int source);
int ospf_nbr_search(struct ospf_if *p_if);

#ifdef __cplusplus
}
#endif

#endif /* _OSPF_STRUCTURES_H_ */


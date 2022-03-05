
#ifndef __SDK_NETPKT_H__
#define __SDK_NETPKT_H__

//#define LINUX
//#define NETLINK_TEST (31)

#ifdef NETLINK_TEST
struct nk_pkt_start
{
    int ifindex;
    int value;
    int value1;
    int value2;
};

#define NK_PKT_START    1
#define NK_PKT_STOP     2
#define NK_PKT_SETUP    3
#define NK_PKT_DATA     4
#define NK_PKT_CPU      5
#define NK_PKT_TEST     6
#endif

//opcode
#define BRCM_OPCODE_SHIFT	5
#define BRCM_OPCODE_MASK	0x07
#define BRCM_OPCODE_TYPE0	0x00
#define BRCM_OPCODE_TYPE1	0x01

#define BRCM_INGRESS_PORT_MASK	0x1F

#define BRCM_EGRESS_TC_SHIFT	2
#define BRCM_EGRESS_TC_MASK		0x7
#define BRCM_EGRESS_TE_SHIFT	0
#define BRCM_EGRESS_TE_MASK		0x3
//reason
#define BRCM_REASON_MIRROR	        0x01
#define BRCM_REASON_SALEARNING	    0x02
#define BRCM_REASON_SWITCHING	    0x04
#define BRCM_REASON_PROTO_TERM	    0x08
#define BRCM_REASON_PROTO_SNOOP	    0x10
#define BRCM_REASON_FLOODING	    0x20

//TE
#define BRCM_TE_NOT_CARE	        0x00
#define BRCM_TE_UNTAG	            0x01
#define BRCM_TE_TAG	                0x02

struct brcm_ingress_header
{
    uint8_t reason;
    uint8_t TC;
    uint8_t port;  
    uint8_t TR; 
    uint8_t TR_PID; 
    uint32_t timestamp; 
};
struct brcm_egress_header
{  
    uint8_t TC;
    uint8_t TE;
    uint8_t TS;
    uint32_t dstport;
};

struct brcm_tag_header
{
    uint8_t opcode; //000
    uint8_t ingress;
    union 
    {
        struct brcm_egress_header egress; 
        struct brcm_ingress_header ingress;
    }header;
};


int brcm_ingress_hdr_getandshow(unsigned char *brcm_tag, struct brcm_tag_header *tagheader);
int brcm_ingress_hdr_set(unsigned char *brcm_tag, struct brcm_tag_header *tagheader);


typedef int (*rx_cb)(int unit, unsigned char *buf, int len);

extern int eth_drv_init(int unit);
extern int eth_drv_register(int unit, rx_cb _rx);
extern int eth_drv_start(int unit);
extern int eth_drv_stop(int unit);
extern int eth_drv_tx(int unit, unsigned char * pkt, int len);

extern int eth_drv_unicast(int unit, zpl_phyport_t, int, zpl_void *, int);
extern int eth_drv_vlan_flood(int unit, vlan_t, int, zpl_void *, int);


#endif /* __SDK_NETPKT_H__ */

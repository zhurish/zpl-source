#ifndef __B53_EAP_REGS_H__
#define __B53_EAP_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* EAP Register */
#define B53_EAP_PAGE			0x42


/* EAP Global Register */
#define B53_EAP_GLOBAL			0x00
#define 	B53_EAP_RARP_EN		BIT(6)
#define 	B53_EAP_BPDU_EN		BIT(5)
#define 	B53_EAP_RMC_EN		BIT(4)
#define 	B53_EAP_DHCP_EN		BIT(3)
#define 	B53_EAP_ARP_EN		BIT(2)
#define 	B53_EAP_2DIP_EN		BIT(1)

/* EAP Multiport Address Register */
#define B53_EAP_MULT_ADDR		0x01
#define 	B53_EAP_MPORT(n)	BIT((n))

/* EAP Destnation IP Address Register */
#define B53_EAP_DST_IP_ADDR0		0x02
#define B53_EAP_DST_IP_ADDR1		0x0A
#define 	B53_EAP_DIP_ADD			32
#define 	B53_EAP_DMSK_ADD		0


/* EAP Port Register */
#define B53_EAP_PORT_ADDR		0x20
#define 	B53_EAP_PORT(n)		(0x20 + 8*(n))
#define 	B53_EAP_MODE		51
#define 	B53_EAP_MODE_MASK	3
#define 	B53_EAP_BLK_MODE	49
#define 	B53_EAP_BLK_MODE_M	3
#define 	B53_EAP_DA_EN		48



#ifdef __cplusplus
}
#endif


#endif /* __B53_EAP_REGS_H__ */
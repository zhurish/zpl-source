#ifndef __B53_QOS_REGS_H__
#define __B53_QOS_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* Quality of Service (QoS) Registers */
#define B53_QOS_PAGE			0x30

/* Traffic Remarking Register */
#define B53_TC_REMARK_PAGE			0x91

/* Broadcast Storm Suppression Register */
#define B53_BROADCAST_STROM_PAGE			0x41


/*************************************************************************
 * 802.1Q Page Registers
 *************************************************************************/

/* Global QoS Control (8 bit) */
#define B53_QOS_GLOBAL_CTL		0x00
#define 	B53_AGG_MODE		BIT(7)
#define 	B53_PORT_QOS_EN		BIT(6)
#define 	B53_QOS_LAYER_SEL_S	2

/* Enable 802.1Q for individual Ports (16 bit) */
#define B53_802_1P_CTL			0x04

/* Enable 802.1Q for individual Ports (16 bit) */
#define B53_QOS_DIFFSERV_CTL	0x06


#define B53_PCP_TO_TC_PORT_CTL(n)	(0x10 + ((n) * 4))
#define 	B53_PCP_TO_TC(n)			((n)*3 + 0x00)
#define 	B53_TC_MASK			7

#define B53_QOS_DIFFSERV_MAP_CTL0	0x30
#define B53_QOS_DIFFSERV_MAP_CTL1	0x36
#define B53_QOS_DIFFSERV_MAP_CTL2	0x3C
#define B53_QOS_DIFFSERV_MAP_CTL3	0x42
#define 	B53_DIFFSERV_TO_TC(n)			((n)*3 + 0x00)

#define B53_TC_TO_QUEUE_CTL	0x62
#define 	B53_TC_TO_QUEUE(n)			((n)*2 + 0x00)
#define 	B53_TC_QUEUE_MASK			3

#define B53_CPU_TO_QUEUE_CTL	0x64
#define 	B53_CPU_TC_TO_QUEUE(n)			((n))
#define 	B53_CPU_TC_QUEUE_MASK			7

#define 	B53_CPU_TC_PROTO_FLOOD	15
#define 	B53_CPU_TC_PROTO_SNOOP	12
#define 	B53_CPU_TC_PROTO_TERM	9
#define 	B53_CPU_TC_SWITCH		6
#define 	B53_CPU_TC_SALEARN		3
#define 	B53_CPU_TC_MIRROR		0

#define 	B53_CPU_PROTO_FLOOD_QUEUE	0
#define 	B53_CPU_PROTO_SNOOP_QUEUE	3
#define 	B53_CPU_PROTO_TERM_QUEUE		4
#define 	B53_CPU_SWITCH_QUEUE			1
#define 	B53_CPU_SALEARN_QUEUE		2
#define 	B53_CPU_MIRROR_QUEUE			0

#define B53_TX_QUEUE_CTL	0x80
#define B53_TX_QUEUE_WEIGHT(c)	0x81 + (c)
#define B53_COS4_SERVICE_WEIGHT	0x85

/* Traffic Remarking Register */
#define B53_TC_REMARK_CTL		0x00
#define 	B53_PCP_REMARK_EN(p)	(p)<<16
#define 	B53_CFI_REMARK_EN(p)	BIT(p)

#define B53_TX_TO_PCP_CTL(n)	((n)*8 + 0x10)
#define 	B53_TC_PCP_MASK			0x0F
#define 	B53_TC_PCP_PRI(n)			((n)*4)


//Egress Non-BroadSync HD Packet TC to PCP Mapping Register
#define B53_EG_NON_BHD_CTL(n)				0x10 + 8*(n)






#ifdef __cplusplus
}
#endif


#endif /* __B53_QOS_REGS_H__ */

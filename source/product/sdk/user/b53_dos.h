#ifndef __B53_DOS_REGS_H__
#define __B53_DOS_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif


/* DOS Registers */
#define B53_DOS_PAGE			0x36


/* DOS Registers */
#define B53_DOS_CTL			0x00
#define 	B53_ICMP6_LONGPING_DROP_EN	BIT(13)
#define 	B53_ICMP4_LONGPING_DROP_EN	BIT(12)
#define 	B53_ICMP6_FRAGMENT_DROP_EN	BIT(11)
#define 	B53_ICMP4_FRAGMENT_DROP_EN	BIT(10)
#define 	B53_TCP_FRAGERROR_DROP_EN	BIT(9)
#define 	B53_TCP_SHORTHDR_DROP_EN	BIT(8)
#define 	B53_TCP_SYNERROR_DROP_EN	BIT(7)
#define 	B53_TCP_SYNFINSCAN_DROP_EN	BIT(6)
#define 	B53_TCP_XMASSCAN_DROP_EN	BIT(5)
#define 	B53_TCP_NULLSCAN_DROP_EN	BIT(4)
#define 	B53_UDP_BLAT_DROP_EN		BIT(3)
#define 	B53_TCP_BLAT_DROP_EN		BIT(2)
#define 	B53_IP_LAN_DRIP_EN			BIT(1)

#define B53_MIN_TCPHDR_SIZE_CTL			0x04
#define B53_MAX_ICMPV4_SIZE_CTL			0x08
#define B53_MAX_ICMPV6_SIZE_CTL			0x0C

#define B53_DOS_DIS_LEARN_CTL			0x10


#ifdef __cplusplus
}
#endif


#endif /* __B53_DOS_REGS_H__ */

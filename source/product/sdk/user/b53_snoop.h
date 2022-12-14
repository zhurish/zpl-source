#ifndef __B53_SNOOP_REGS_H__
#define __B53_SNOOP_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* High-Level Protocol Control Register (32 bit) */
#define B53_HIGH_LEVEL_CTL			0x50
#define   B53_MLD_QRY_FWD_MODE		BIT(18)
#define   B53_MLD_QRY_EN			BIT(17)
#define   B53_MLD_RPTDONE_FWD_MODE	BIT(16)
#define   B53_MLD_RPTDONE_EN		BIT(15)
#define   B53_IGMP_UKN_FWD_MODE		BIT(14)
#define   B53_IGMP_UKN_EN			BIT(13)
#define   B53_IGMP_QRY_FWD_MODE		BIT(12)
#define   B53_IGMP_QRY_EN			BIT(11)
#define   B53_IGMP_RPTLVE_FWD_MODE	BIT(10)
#define   B53_IGMP_RPTLVE_EN		BIT(9)
#define   B53_IGMP_DIP_EN			BIT(8)
#define   B53_ICMPV6_FWD_MODE		BIT(5)
#define   B53_ICMPV6_EN				BIT(4)
#define   B53_ICMPV4_EN				BIT(3)
#define   B53_DHCP_EN				BIT(2)
#define   B53_RARP_EN				BIT(1)
#define   B53_ARP_EN				BIT(0)


#ifdef __cplusplus
}
#endif


#endif /* __B53_SNOOP_REGS_H__ */
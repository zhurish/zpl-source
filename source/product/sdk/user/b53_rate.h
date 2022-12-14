#ifndef __B53_RATE_REGS_H__
#define __B53_RATE_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif


/*Ingress Rate Control Configuration Register*/
#define B53_INGRESS_RATE_CTL			0x00
#define 	B53_IPG_XLENEN_EN			BIT(18)
#define 	B53_BUCK1_BRM_SEL_EN		BIT(17)
#define 	B53_BUCK1_PACKET_TYPE		9

#define 	B53_BUCK0_BRM_SEL_EN		BIT(8)
#define 	B53_IPG_XLENEN_EG_EN		BIT(6)
#define 	B53_BUCK0_PACKET_TYPE		0

/*Port Receive Rate Control Register*/
#define B53_PORT_RECEIVE_RATE_CTL(n)	(0x10 + (n)*4)
#define 	B53_STRM_SUPP_EN			BIT(28)
#define 	B53_RSVMC_SUPP_EN			BIT(27)
#define 	B53_BC_SUPP_EN				BIT(26)
#define 	B53_MC_SUPP_EN				BIT(25)
#define 	B53_DLF_SUPP_EN				BIT(24)

#define 	B53_BUCKET1_EN				BIT(23)
#define 	B53_BUCKET0_EN				BIT(22)

#define 	B53_BUCKET1_SIZE_S			19
#define 	B53_BUCKET1_RATE_CNT		11
#define 	B53_BUCKET0_SIZE_S			8
#define 	B53_BUCKET_SIZE_MASK		0x07
#define 	B53_BUCKET0_RATE_CNT		0
#define 	B53_BUCKET_RATE_MASK		0xff

/*Port Egress Rate Control Configuration Register*/
#define B53_PORT_EGRESS_RATE_CTL(n)	(0x80 + (n)*2)
#define 	B53_ERC_EN		BIT(11)
#define 	B53_BKT_SZE_S	8
#define 	B53_BKT_SZE_MASK	0x07
#define 	B53_RATE_CNT_MASK	0xFF

#define B53_IMP_PORT_CTL				0xC0
#define 	B53_RATE_INDEX_MASK			0x3f

#define B53_IMP_PORT5_CTL				0xC1


#ifdef __cplusplus
}
#endif


#endif /* __B53_RATE_REGS_H__ */
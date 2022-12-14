#ifndef __B53_EEE_REGS_H__
#define __B53_EEE_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif



/* Mirror capture control register (16 bit) */
#define B53_MIR_CAP_CTL			0x10
#define  CAP_PORT_MASK			0xf
#define  BLK_NOT_MIR			BIT(14)
#define  MIRROR_EN			BIT(15)

/* Ingress mirror control register (16 bit) */
#define B53_IG_MIR_CTL			0x12
#define  MIRROR_MASK			0x1ff
#define  DIV_EN				BIT(13)
#define  MIRROR_FILTER_MASK		0x3
#define  MIRROR_FILTER_SHIFT		14
#define  MIRROR_ALL			0
#define  MIRROR_DA			1
#define  MIRROR_SA			2

/* Ingress mirror divider register (16 bit) */
#define B53_IG_MIR_DIV			0x14
#define  IN_MIRROR_DIV_MASK		0x3ff

/* Ingress mirror MAC address register (48 bit) */
#define B53_IG_MIR_MAC			0x16

/* Egress mirror control register (16 bit) */
#define B53_EG_MIR_CTL			0x1C

/* Egress mirror divider register (16 bit) */
#define B53_EG_MIR_DIV			0x1E

/* Egress mirror MAC address register (48 bit) */
#define B53_EG_MIR_MAC			0x20



#ifdef __cplusplus
}
#endif


#endif /* __B53_EEE_REGS_H__ */
#ifndef __B53_STP_REGS_H__
#define __B53_STP_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define BR_STATE_DISABLED 0
#define BR_STATE_LISTENING 1
#define BR_STATE_LEARNING 2
#define BR_STATE_FORWARDING 3
#define BR_STATE_BLOCKING 4

/* MSPT Register */
#define B53_MSTP_PAGE			0x43


/* MSPT Control Register */
#define B53_MSTP_CTL		0x00
#define 	B53_MSTP_EN		BIT(0)

#define B53_MSTP_AGE_CTL		0x02
#define 	B53_MSTP_AGE_MASK	0xFF

#define B53_MSTP_TBL_CTL(n)				0x10 + 4*(n)
#define 	B53_MSTP_TBL_PORT_MASK(n)	(0x3 << ((n)*3))
#define 	B53_MSTP_TBL_PORT(n, s)		((s) << ((n)*3))

#define B53_MSTP_BYPASS_CTL		0x50
#define 	B53_MSTP_BYPASS_EN(n)	BIT(n)





#ifdef __cplusplus
}
#endif


#endif /* __B53_STP_REGS_H__ */

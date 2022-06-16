#ifndef __B53_CPU_REGS_H__
#define __B53_CPU_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* IMP Control Register (8 bit) */
#define B53_IMP_CTRL			0x08


#define B53_MGMT_CTRL			0x00
#define  B53_IMP_EN				(2<<6)
#define  B53_DOUBLE_IMP_EN		(3<<6)
#define  B53_IMP_DIS			(0<<6)


#define B53_RGMII_CTRL_IMP		0x60
#define   RGMII_CTRL_ENABLE_GMII	BIT(7)
#define   RGMII_CTRL_TIMING_SEL		BIT(2)
#define   RGMII_CTRL_DLL_RXC		BIT(1)
#define   RGMII_CTRL_DLL_TXC		BIT(0)

#define B53_RGMII_CTRL_P(i)		(B53_RGMII_CTRL_IMP + (i))


/* Broadcom header RX control (16 bit) */
#define B53_BRCM_HDR_RX_DIS		0x60

/* Broadcom header TX control (16 bit)	*/
#define B53_BRCM_HDR_TX_DIS		0x62


/* Broadcom Header control register (8 bit) */
#define B53_BRCM_HDR			0x03
#define   BRCM_HDR_P8_EN		BIT(0) /* Enable tagging on port 8 */
#define   BRCM_HDR_P5_EN		BIT(1) /* Enable tagging on port 5 */
#define   BRCM_HDR_P7_EN		BIT(2) /* Enable tagging on port 7 */



#ifdef __cplusplus
}
#endif


#endif /* __B53_CPU_REGS_H__ */
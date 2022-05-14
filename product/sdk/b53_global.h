#ifndef __B53_GLOBAL_REGS_H__
#define __B53_GLOBAL_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* Management Port (SMP) Page offsets */
#define B53_CTRL_PAGE			0x00 /* Control */
#define B53_STAT_PAGE			0x01 /* Status */
#define B53_MGMT_PAGE			0x02 /* Management Mode */
#define B53_MIB_AC_PAGE			0x03 /* MIB Autocast */





/*************************************************************************
 * Management Mode Page Registers
 *************************************************************************/

/* Global Management Config Register (8 bit) */
#define B53_GLOBAL_CONFIG		0x00
#define   GC_RESET_MIB			0x01
#define   GC_RX_BPDU_EN			0x02


/* Device ID register (8 or 32 bit) */
#define B53_DEVICE_ID			0x30

/* Revision ID register (8 bit) */
#define B53_REV_ID			0x40

/* Revision ID register for BCM5325 */
#define B53_REV_ID_25			0x50

/*************************************************************************
 * Control Page registers
 *************************************************************************/

/* Switch Mode Control Register (8 bit) */
#define B53_SWITCH_MODE			0x0b
#define   SM_SW_FWD_MODE		BIT(0)	/* 1 = Managed Mode */
#define   SM_SW_FWD_EN			BIT(1)	/* Forwarding Enable */



/* Software reset register (8 bit) */
#define B53_SOFTRESET			0x79
#define   SW_RST			BIT(7)
#define   EN_CH_RST			BIT(6)
#define   EN_SW_RST			BIT(4)




/*************************************************************************
 * Jumbo Frame Page Registers
 *************************************************************************/
/* Jumbo Frame Registers */
#define B53_JUMBO_PAGE			0x40

/* Jumbo Enable Port Mask (bit i == port i enabled) (32 bit) */
#define B53_JUMBO_PORT_MASK		0x01

/* Good Frame Max Size without 802.1Q TAG (16 bit) */
#define B53_JUMBO_MAX_SIZE		0x05
#define   JMS_MIN_SIZE			1518
#define   JMS_MAX_SIZE			9724


#ifdef __cplusplus
}
#endif


#endif /* __B53_GLOBAL_REGS_H__ */

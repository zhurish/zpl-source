#ifndef __B53_EEE_REGS_H__
#define __B53_EEE_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif


/* EEE Control Registers Page */
#define B53_EEE_PAGE			0x92


/*************************************************************************
 * EEE Configuration Page Registers
 *************************************************************************/

/* EEE Enable control register (16 bit) */
#define B53_EEE_EN_CTRL			0x00

/* EEE LPI assert status register (16 bit) */
#define B53_EEE_LPI_ASSERT_STS		0x02

/* EEE LPI indicate status register (16 bit) */
#define B53_EEE_LPI_INDICATE		0x4

/* EEE Receiving idle symbols status register (16 bit) */
#define B53_EEE_RX_IDLE_SYM_STS		0x6

/* EEE Pipeline timer register (32 bit) */
#define B53_EEE_PIP_TIMER		0xC

/* EEE Sleep timer Gig register (32 bit) */
#define B53_EEE_SLEEP_TIMER_GIG(i)	(0x10 + 4 * (i))

/* EEE Sleep timer FE register (32 bit) */
#define B53_EEE_SLEEP_TIMER_FE(i)	(0x34 + 4 * (i))

/* EEE Minimum LP timer Gig register (32 bit) */
#define B53_EEE_MIN_LP_TIMER_GIG(i)	(0x58 + 4 * (i))

/* EEE Minimum LP timer FE register (32 bit) */
#define B53_EEE_MIN_LP_TIMER_FE(i)	(0x7c + 4 * (i))

/* EEE Wake timer Gig register (16 bit) */
#define B53_EEE_WAKE_TIMER_GIG(i)	(0xa0 + 2 * (i))

/* EEE Wake timer FE register (16 bit) */
#define B53_EEE_WAKE_TIMER_FE(i)	(0xb2 + 2 * (i))



#ifdef __cplusplus
}
#endif


#endif /* __B53_EEE_REGS_H__ */
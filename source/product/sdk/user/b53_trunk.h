#ifndef __B53_TRUNK_REGS_H__
#define __B53_TRUNK_REGS_H__

#ifdef	__cplusplus
extern "C" {
#endif


/* Trunk Registers */
#define B53_TRUNK_PAGE			0x32



/* Trunk Registers */
#define B53_TRUNK_CTL			0x00
#define 	B53_MAC_BASE_TRNK_EN	BIT(3)
#define 	B53_TRK_HASH_DASA		0
#define 	B53_TRK_HASH_DA			1
#define 	B53_TRK_HASH_SA			2
#define 	B53_TRK_HASH_ILLEGAL	3

#define B53_TRUNK_GROUP0			0x10
#define B53_TRUNK_GROUP1			0x12

#ifdef __cplusplus
}
#endif


#endif /* __B53_TRUNK_REGS_H__ */

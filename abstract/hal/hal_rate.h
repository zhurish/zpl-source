/*
 * hal_rate.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_RATE_H__
#define __HAL_RATE_H__
#ifdef __cplusplus
extern "C" {
#endif

/* Rate flags. */
#define BCM_RATE_DLF            0x01       
#define BCM_RATE_MCAST          0x02       
#define BCM_RATE_BCAST          0x04       
#define BCM_RATE_UCAST          0x08       
#define BCM_RATE_SALF           0x10       
#define BCM_RATE_RSVD_MCAST     0x20       
#define BCM_RATE_CTRL_BUCKET_1  0x00010000 
#define BCM_RATE_CTRL_BUCKET_2  0x00020000 
#define BCM_RATE_ALL            (BCM_RATE_BCAST | BCM_RATE_MCAST | BCM_RATE_DLF | BCM_RATE_UCAST | BCM_RATE_SALF | BCM_RATE_RSVD_MCAST) 

/* hal_rate_limit_s */
typedef struct hal_rate_limit_s {
    zpl_uint32 br_dlfbc_rate; 
    zpl_uint32 br_mcast_rate; 
    zpl_uint32 br_bcast_rate; 
    zpl_uint32 flags; 
} hal_rate_limit_t;


/* 
 * Configure/retrieve rate limit and on/off state of DLF, MCAST, and
 * BCAST limiting.
 */
extern int hal_rate_set(zpl_uint32 pps, 
    zpl_uint32 flags);


/* 
 * Configure or retrieve rate limit for specified packet type on given
 * port.
 */
extern int hal_rate_mcast_set(ifindex_t ifindex, zpl_uint32 pps, 
    zpl_uint32 flags);


/* 
 * Configure or retrieve rate limit for specified packet type on given
 * port.
 */
extern int hal_rate_dlfbc_set(ifindex_t ifindex, zpl_uint32 pps, 
    zpl_uint32 flags);

/* 
 * Configure or retrieve rate limit for specified packet type on given
 * port.
 */
extern int hal_rate_bcast_set(ifindex_t ifindex, zpl_uint32 pps, 
    zpl_uint32 flags);

/* 
 * Front ends to hal_*_rate_set/get functions. Uses a single data
 * structure to write into all the 3 rate control registers.
 */
extern int hal_rate_type_set(hal_rate_limit_t *rl);

/* 
 * Configure/retrieve metering rate limit for specified packet type on
 * given port.
 */
extern int hal_rate_bandwidth_set(ifindex_t ifindex, 
    zpl_uint32 flags, 
    zpl_uint32 kbits_sec, 
    zpl_uint32 kbits_burst);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_RATE_H__ */

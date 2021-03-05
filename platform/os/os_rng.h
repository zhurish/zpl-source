/*
 * os_rng.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __OS_RNG_H__
#define __OS_RNG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FAST

/* HIDDEN */

/* typedefs */

typedef struct		/* RING - ring buffer */
    {
    ospl_uint32 pToBuf;		/* offset from start of buffer where to write next */
    ospl_uint32 pFromBuf;	/* offset from start of buffer where to read next */
    ospl_uint32 bufSize;	/* size of ring in bytes */
    ospl_char *buf;		/* pointer to start of buffer */
    } RING;

/* END_HIDDEN */

typedef RING *RING_ID;

extern ospl_bool 	rngIsEmpty (RING_ID ringId);
extern ospl_bool 	rngIsFull (RING_ID ringId);
extern RING_ID 	rngCreate (ospl_uint32 nbytes);
extern int 	rngBufGet (RING_ID rngId, ospl_char *buffer, ospl_uint32 maxbytes);
extern int 	rngBufPut (RING_ID rngId, ospl_char *buffer, ospl_uint32 nbytes);
extern int 	rngFreeBytes (RING_ID ringId);
extern int 	rngNBytes (RING_ID ringId);
extern void 	rngDelete (RING_ID ringId);
extern void 	rngFlush (RING_ID ringId);
extern void 	rngMoveAhead (RING_ID ringId, ospl_uint32 n);
extern void 	rngPutAhead (RING_ID ringId, ospl_char byte, ospl_uint32 offset);


#ifdef __cplusplus
}
#endif

#endif /* __OS_RNG_H__ */

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
    int pToBuf;		/* offset from start of buffer where to write next */
    int pFromBuf;	/* offset from start of buffer where to read next */
    int bufSize;	/* size of ring in bytes */
    char *buf;		/* pointer to start of buffer */
    } RING;

/* END_HIDDEN */

typedef RING *RING_ID;

extern BOOL 	rngIsEmpty (RING_ID ringId);
extern BOOL 	rngIsFull (RING_ID ringId);
extern RING_ID 	rngCreate (int nbytes);
extern int 	rngBufGet (RING_ID rngId, char *buffer, int maxbytes);
extern int 	rngBufPut (RING_ID rngId, char *buffer, int nbytes);
extern int 	rngFreeBytes (RING_ID ringId);
extern int 	rngNBytes (RING_ID ringId);
extern void 	rngDelete (RING_ID ringId);
extern void 	rngFlush (RING_ID ringId);
extern void 	rngMoveAhead (RING_ID ringId, int n);
extern void 	rngPutAhead (RING_ID ringId, char byte, int offset);


#ifdef __cplusplus
}
#endif

#endif /* __OS_RNG_H__ */

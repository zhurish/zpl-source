/*
 * voip_ring.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef _VOIP_RING_H_
#define _VOIP_RING_H_

struct ring_sound
{
	char id;
	char *file;
};

struct ring_session
{
	char 	id;
	void 	*r;	//RingStream
	void 	*f;	//MSFactory
	void 	*sc;	//MSSndCard
	BOOL	use;
	BOOL	start;
};

extern int voip_call_ring_lookup_api(int id);
extern int voip_call_ring_set_api(int id);
extern int voip_call_ring_get_api(int *id);

extern int voip_call_ring_start(int id);
extern int voip_call_ring_stop(int id);

extern int voip_call_ring_module_init();


extern int voip_call_ring_running(void *pVoid);
extern int voip_call_ring_start_api();
extern int voip_call_ring_stop_api();
extern BOOL voip_call_ring_active_api();

#ifdef PL_VOIP_MEDIASTREAM
extern int ring_test();
#endif
#endif /* _VOIP_RING_H_ */

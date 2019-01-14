/*
 * voip_state.c
 *
 *  Created on: Dec 31, 2018
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_state.h"
#include "voip_event.h"
#include "voip_state.h"
#include "voip_app.h"

/*
voip_state_t voip_state_get()
{
	return voip_call.state;
}

int voip_state_set(voip_state_t state)
{
	voip_call.state = state;
	return OK;
}
*/

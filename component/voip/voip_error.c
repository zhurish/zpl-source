/*
 * voip_error.c
 *
 *  Created on: 2019年1月2日
 *      Author: DELL
 */

#include "zebra.h"
#include "voip_error.h"

static int _voip_errno = VOIP_E_NONE;




int voip_set_errno(int err)
{
	_voip_errno = err;
	return OK;
}

int voip_get_errno(int *err)
{
	if(err)
		*err = _voip_errno;
	return OK;
}

int voip_errno()
{
	return _voip_errno;
}

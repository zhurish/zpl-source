/*
 * zpl_video_region.c
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_video_region.h"



int zpl_video_hwregion_destroy(zpl_video_hwregion_t *hwregion)
{
		if(hwregion)
		{
			os_free(hwregion);	
		}
	return OK;
}

zpl_video_hwregion_t * zpl_video_hwregion_create(zpl_int32 rng_id)
{
	zpl_video_hwregion_t *t = os_malloc(sizeof(zpl_video_hwregion_t));
	zpl_video_assert(t);
	if(t)
	{
		memset(t, 0, sizeof(zpl_video_hwregion_t));
		t->rng_id = rng_id;

		return t;
	}
	return NULL;
}




int zpl_video_hwregion_active(zpl_video_hwregion_t *hwregion)
{
    int ret = -1;
    zpl_video_assert(hwregion);

    return ret;
}

int zpl_video_hwregion_inactive(zpl_video_hwregion_t *hwregion)
{
    int ret = -1;
    zpl_video_assert(hwregion);


    return ret;
}




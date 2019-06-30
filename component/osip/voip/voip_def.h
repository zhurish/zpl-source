/*
 * voip_def.h
 *
 *  Created on: Dec 9, 2018
 *      Author: zhurish
 */

#ifndef __VOIP_DEF_H__
#define __VOIP_DEF_H__

//#define PL_VOIP_MEDIASTREAM_TEST
//#define PL_VOIP_MEDIASTREAM


#ifdef PL_VOIP_MEDIASTREAM
#include "ortp/port.h"
#include <ortp/b64.h>
//#include "mediastreamer2/mediastreamer-config.h"
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/msvolume.h"
#else
typedef int bool_t;
#endif

#include "plconfig.h"
#include "zassert.h"
#include "os_uci.h"
#include "uci_ubus.h"
/*
#include "voip_log.h"
#include "voip_util.h"
*/
/*
#include "voip_state.h"
#include "voip_event.h"
#include "voip_osip.h"
#include "voip_sip.h"
#include "voip_socket.h"
#include "voip_stream.h"
#include "voip_task.h"
#include "voip_volume.h"
*/
//#include "voip_app.h"


//typedef struct voip_app_s voip_app_t;


#endif /* __VOIP_DEF_H__ */

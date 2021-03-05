/*
 * pjsip_cfg.h
 *
 *  Created on: Feb 17, 2019
 *      Author: zhurish
 */

#ifndef __PJSIP_CFG_H__
#define __PJSIP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pjsip_jsoncfg.h"


int pjsip_endpoint_config_init(pjsip_ep_config_t *ua);
int pjsip_endpoint_config_destroy(pjsip_ep_config_t *ua);


int pjsip_account_config_init(pjsip_account_config_t *ua);
int pjsip_account_config_destroy(pjsip_account_config_t *ua);



#ifdef __cplusplus
}
#endif


#endif /* __PJSIP_CFG_H__ */

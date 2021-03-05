/*
 * mqtt_app_show.h
 *
 *  Created on: 2020年5月17日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_SHOW_H__
#define __MQTT_APP_SHOW_H__


int mqtt_app_config_show(struct mqtt_app_config  *cfg, struct vty *vty, ospl_bool detail, ospl_bool wrshow);
int mqtt_app_debug_show(struct mqtt_app_config  *cfg, struct vty *vty);

#endif /* __MQTT_APP_SHOW_H__ */

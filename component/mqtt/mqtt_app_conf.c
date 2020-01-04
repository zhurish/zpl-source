/*
 * mqtt_app_conf.c
 *
 *  Created on: 2019年12月15日
 *      Author: zhurish
 */


#include "zebra.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mqtt_app_api.h"
#include "mqtt_app_conf.h"

struct mqtt_app_config mqtt_app_config;

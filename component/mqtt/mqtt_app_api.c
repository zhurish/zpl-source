/*
 * mqtt_app_api.c
 *
 *  Created on: 2019年7月19日
 *      Author: DELL
 */

#include "zebra.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mqtt_app_api.h"


int mqtt_module_init()
{
	mosquitto_lib_init();
	return OK;
}

int mqtt_module_exit()
{
	mosquitto_lib_cleanup();
	return OK;
}

int mqtt_module_task_init()
{
	return OK;
}

int mqtt_module_task_exit()
{
	return OK;
}

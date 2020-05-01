/*
Copyright (c) 2009-2020 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include "mqtt-config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mosquitto_internal.h"
#include "mosquitto.h"
#include "memory_mosq.h"
#include "logging_mosq.h"


#ifndef WITH_BROKER
int __log__printf(const char *file, const char *func, const int line,
		struct mosquitto *mosq, int priority, const char *fmt, ...)
{
	va_list va;
	char *s = NULL;
	//,*p = NULL;
	int len;

	assert(mosq);
	assert(fmt);

	pthread_mutex_lock(&mosq->log_callback_mutex);
	if(mosq->on_log){
		len = strlen(fmt) + 500;
		s = mosquitto__malloc(len*sizeof(char));
		if(!s){
			pthread_mutex_unlock(&mosq->log_callback_mutex);
			return MOSQ_ERR_NOMEM;
		}

		va_start(va, fmt);
		//p = strrchr(file, '/');
		//snprintf(s, len, "%s:(%s:%d)", func, p+1, line);
		//vsnprintf(s+ strlen(s), len-strlen(s), fmt, va);
		vsnprintf(s, len, fmt, va);
		va_end(va);
		s[len-1] = '\0'; /* Ensure string is null terminated. */
/*		if(p)
			mosq->on_log(func, p+1, line, mosq, mosq->userdata, priority, s);
		else*/
			mosq->on_log(func, file, line, mosq, mosq->userdata, priority, s);

		mosquitto__free(s);
	}
	pthread_mutex_unlock(&mosq->log_callback_mutex);

	return MOSQ_ERR_SUCCESS;
}
#else
int log__printf(struct mosquitto *mosq, int priority, const char *fmt, ...)
{
	va_list va;
	char *s = NULL,*p = NULL;
	int len;

	assert(mosq);
	assert(fmt);

	pthread_mutex_lock(&mosq->log_callback_mutex);
	if(mosq->on_log){
		len = strlen(fmt) + 500;
		s = mosquitto__malloc(len*sizeof(char));
		if(!s){
			pthread_mutex_unlock(&mosq->log_callback_mutex);
			return MOSQ_ERR_NOMEM;
		}

		va_start(va, fmt);
		vsnprintf(s, len, fmt, va);
		va_end(va);
		s[len-1] = '\0'; /* Ensure string is null terminated. */
		mosq->on_log( mosq, mosq->userdata, priority, s);
		mosquitto__free(s);
	}
	pthread_mutex_unlock(&mosq->log_callback_mutex);

	return MOSQ_ERR_SUCCESS;
}
#endif


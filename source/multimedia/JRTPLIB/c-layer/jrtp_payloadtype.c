/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of oRTP 
 * (see https://gitlab.linphone.org/BC/public/ortp).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jrtp_payloadtype.h"
#include "jrtp_rtpprofile.h"



static char * jrtp_strdup_vprintf(const char *fmt, va_list ap)
{
/* Guess we need no more than 100 bytes. */
	int n, size = 200;
	char *p,*np;
#ifndef _WIN32
	va_list cap;/*copy of our argument list: a va_list cannot be re-used (SIGSEGV on linux 64 bits)*/
#endif
	if ((p = (char *) malloc (size)) == NULL)
		return NULL;
	while (1){
/* Try to print in the allocated space. */
#ifndef _WIN32
		va_copy(cap,ap);
		n = vsnprintf (p, size, fmt, cap);
		va_end(cap);
#else
		/*this works on 32 bits, luckily*/
		n = vsnprintf (p, size, fmt, ap);
#endif
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
//printf("Reallocing space.\n");
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		if ((np = (char *) realloc (p, size)) == NULL)
		{
			free(p);
			return NULL;
		} else {
			p = np;
		}
	}
}

static char *jrtp_strdup_printf(const char *fmt,...){
	char *ret;
	va_list args;
	va_start (args, fmt);
	ret=jrtp_strdup_vprintf(fmt, args);
	va_end (args);
	return ret;
}

jrtp_PayloadType *jrtp_payload_type_new()
{
	jrtp_PayloadType *newpayload=(jrtp_PayloadType *)malloc(sizeof(jrtp_PayloadType));
	newpayload->flags|=JRTP_PAYLOAD_TYPE_ALLOCATED;
	return newpayload;
}


jrtp_PayloadType *jrtp_payload_type_clone(const jrtp_PayloadType *payload)
{
	jrtp_PayloadType *newpayload=(jrtp_PayloadType *)malloc(sizeof(jrtp_PayloadType));
	memcpy(newpayload,payload,sizeof(jrtp_PayloadType));
    strcpy(newpayload->mime_type, payload->mime_type);
	if (payload->recv_fmtp!=NULL) {
		newpayload->recv_fmtp=strdup(payload->recv_fmtp);
	}
	if (payload->send_fmtp!=NULL){
		newpayload->send_fmtp=strdup(payload->send_fmtp);
	}
	newpayload->flags|=JRTP_PAYLOAD_TYPE_ALLOCATED;
	return newpayload;
}

static bool canWrite(jrtp_PayloadType *pt){
	if (!(pt->flags & JRTP_PAYLOAD_TYPE_ALLOCATED)) {
		printf("Cannot change parameters of statically defined payload types: make your"
			" own copy using jrtp_payload_type_clone() first.");
		return 0;
	}
	return 1;
}

/**
 * Sets a recv parameters (fmtp) for the jrtp_PayloadType.
 * This method is provided for applications using RTP with SDP, but
 * actually the ftmp information is not used for RTP processing.
**/
void jrtp_payload_type_set_recv_fmtp(jrtp_PayloadType *pt, const char *fmtp){
	if (canWrite(pt)){
		if (pt->recv_fmtp!=NULL) free(pt->recv_fmtp);
		if (fmtp!=NULL) pt->recv_fmtp=strdup(fmtp);
		else pt->recv_fmtp=NULL;
	}
}

/**
 * Sets a send parameters (fmtp) for the jrtp_PayloadType.
 * This method is provided for applications using RTP with SDP, but
 * actually the ftmp information is not used for RTP processing.
**/
void jrtp_payload_type_set_send_fmtp(jrtp_PayloadType *pt, const char *fmtp){
	if (canWrite(pt)){
		if (pt->send_fmtp!=NULL) free(pt->send_fmtp);
		if (fmtp!=NULL) pt->send_fmtp=strdup(fmtp);
		else pt->send_fmtp=NULL;
	}
}



void jrtp_payload_type_append_recv_fmtp(jrtp_PayloadType *pt, const char *fmtp){
	if (canWrite(pt)){
		if (pt->recv_fmtp==NULL)
			pt->recv_fmtp=strdup(fmtp);
		else{
			char *tmp=jrtp_strdup_printf("%s;%s",pt->recv_fmtp,fmtp);
			free(pt->recv_fmtp);
			pt->recv_fmtp=tmp;
		}
	}
}

void jrtp_payload_type_append_send_fmtp(jrtp_PayloadType *pt, const char *fmtp){
	if (canWrite(pt)){
		if (pt->send_fmtp==NULL)
			pt->send_fmtp=strdup(fmtp);
		else{
			char *tmp=jrtp_strdup_printf("%s;%s",pt->send_fmtp,fmtp);
			free(pt->send_fmtp);
			pt->send_fmtp=tmp;
		}
	}
}

void jrtp_payload_type_set_avpf_params(jrtp_PayloadType *pt, jrtp_PayloadTypeAvpfParams params) {
	if (canWrite(pt)) {
		memcpy(&pt->avpf, &params, sizeof(pt->avpf));
	}
}

bool jrtp_payload_type_is_vbr(const jrtp_PayloadType *pt) {
	if (pt->type == JRTP_PAYLOAD_VIDEO) return 1;
	return !!(pt->flags & JRTP_PAYLOAD_TYPE_IS_VBR);
}


/**
 * Frees a jrtp_PayloadType.
**/
void jrtp_payload_type_destroy(jrtp_PayloadType *pt)
{
	if (pt->mime_type) free(pt->mime_type);
	if (pt->recv_fmtp) free(pt->recv_fmtp);
	if (pt->send_fmtp) free(pt->send_fmtp);
	free(pt);
}


static const char *find_param_occurence_of(const char *fmtp, const char *param){
	const char *pos=fmtp;
	int param_len = (int)strlen(param);
	do{
		pos=strstr(pos,param);
		if (pos){
			/*check that the occurence found is not a subword of a parameter name*/
			if (pos==fmtp){
				if (pos[param_len] == '=') break; /* found it */
			}else if ((pos[-1]==';' || pos[-1]==' ') && pos[param_len] == '='){
				break; /* found it */
			}
			pos+=strlen(param);
		}
	}while (pos!=NULL);
	return pos;
}

static const char *find_last_param_occurence_of(const char *fmtp, const char *param){
	const char *pos=fmtp;
	const char *lastpos=NULL;
	do{
		pos=find_param_occurence_of(pos,param);
		if (pos) {
			lastpos=pos;
			pos+=strlen(param);
		}
	}while(pos!=NULL);
	return lastpos;
}
/**
 * Parses a fmtp string such as "profile=0;level=10", finds the value matching
 * parameter param_name, and writes it into result. 
 * If a parameter name is found multiple times, only the value of the last occurence is returned.
 * Despite fmtp strings are not used anywhere within oRTP, this function can
 * be useful for people using RTP streams described from SDP.
 * @param fmtp the fmtp line (format parameters)
 * @param param_name the parameter to search for
 * @param result the value given for the parameter (if found)
 * @param result_len the size allocated to hold the result string
 * @return 1 if the parameter was found, else 0.
**/ 
bool jrtp_payload_fmtp_get_value(const char *fmtp, const char *param_name, char *result, size_t result_len){
	const char *pos=find_last_param_occurence_of(fmtp,param_name);
	memset(result, '\0', result_len);
	if (pos){
		const char *equal=strchr(pos,'=');
		if (equal){
			int copied;
			const char *end=strchr(equal+1,';');
			if (end==NULL) end=fmtp+strlen(fmtp); /*assuming this is the last param */
			copied=MIN((int)(result_len-1),(int)(end-(equal+1)));
			strncpy(result,equal+1,copied);
			result[copied]='\0';
			return 1;
		}
	}
	return 0;
}

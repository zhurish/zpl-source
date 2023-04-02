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

/**
 * \file rtpprofile.h
 * \brief Using and creating standart and custom RTP profiles
 *
**/

#ifndef __JRTPPROFILE_H__
#define __JRTPPROFILE_H__

#include "jrtp_payloadtype.h"

#ifdef __cplusplus
extern "C"{
#endif

#define JRTP_PROFILE_MAX_PAYLOADS 128

/**
 * The RTP profile is a table JRTP_PROFILE_MAX_PAYLOADS entries to make the matching
 * between RTP payload type number and the jrtp_PayloadType that defines the type of
 * media.
**/
struct _jRtpProfile
{
	char *name;
	jrtp_PayloadType *payload[JRTP_PROFILE_MAX_PAYLOADS];
};


typedef struct _jRtpProfile jrtp_RtpProfile;

extern jrtp_RtpProfile av_profile;

#define jrtp_profile_get_name(profile) 	(const char*)((profile)->name)
void jrtp_av_profile_init(jrtp_RtpProfile *profile);
extern void jrtp_profile_set_payload(jrtp_RtpProfile *prof, int idx, jrtp_PayloadType *pt);

extern void jrtp_profile_payload_update(int pt, const jrtp_PayloadType *type);
/**
 *	Set payload type number \a index unassigned in the profile.
 *
 *@param profile an RTP profile
 *@param index	the payload type number
**/
#define jrtp_profile_clear_payload(profile,index) \
	jrtp_profile_set_payload(profile,index,NULL)

/* I prefer have this function inlined because it is very often called in the code */
/**
 *
 *	Gets the payload description of the payload type \a index in the profile.
 *
 *@param prof an RTP profile (a #_RtpProfile object)
 *@param idx	the payload type number
 *@return the payload description (a jrtp_PayloadType object)
**/
static inline jrtp_PayloadType * jrtp_profile_get_payload(const jrtp_RtpProfile *prof, int idx){
	if (idx<0 || idx>=JRTP_PROFILE_MAX_PAYLOADS) {
		return NULL;
	}
	return prof->payload[idx];
}
extern void jrtp_profile_clear_all(jrtp_RtpProfile *prof);
extern void jrtp_profile_set_name(jrtp_RtpProfile *prof, const char *name);
extern jrtp_PayloadType * jrtp_profile_get_payload_from_mime(jrtp_RtpProfile *profile,const char *mime);
extern jrtp_PayloadType * jrtp_profile_get_payload_from_rtpmap(jrtp_RtpProfile *profile, const char *rtpmap);
extern int jrtp_profile_get_payload_number_from_mime(jrtp_RtpProfile *profile, const char *mime);
extern int jrtp_profile_get_payload_number_from_mime_and_flag(jrtp_RtpProfile *profile, const char *mime, int flag);
extern int jrtp_profile_get_payload_number_from_rtpmap(jrtp_RtpProfile *profile, const char *rtpmap);
extern int jrtp_profile_find_payload_number(jrtp_RtpProfile *prof,const char *mime,int rate, int channels);
extern jrtp_PayloadType * jrtp_profile_find_payload(jrtp_RtpProfile *prof,const char *mime,int rate, int channels);
extern int jrtp_profile_move_payload(jrtp_RtpProfile *prof,int oldpos,int newpos);

extern jrtp_RtpProfile * jrtp_profile_new(const char *name);
/* clone a profile, payload are not cloned */
extern jrtp_RtpProfile * jrtp_profile_clone(jrtp_RtpProfile *prof);


/*clone a profile and its payloads (ie payload type are newly allocated, not reusing payload types of the reference profile) */
extern jrtp_RtpProfile * jrtp_profile_clone_full(jrtp_RtpProfile *prof);
/* frees the profile and all its PayloadTypes*/
extern void jrtp_profile_destroy(jrtp_RtpProfile *prof);

//extern char *payload_type_get_rtpmap(jrtp_PayloadType *pt);
extern char *jrtp_profile_get_rtpmap(int payload);
extern uint32_t jrtp_profile_get_clock_rate(int payload);
#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * \file sessionset.h
 * \brief Sending and receiving multiple streams together with only one thread.
 *
**/
#ifndef SESSIONSET_H
#define SESSIONSET_H

#ifdef __cplusplus
extern "C"{
#endif




#define ORTP_FD_SET(d, s)     FD_SET(d, s)
#define ORTP_FD_CLR(d, s)     FD_CLR(d, s)
#define ORTP_FD_ISSET(d, s)   FD_ISSET(d, s)
#define ORTP_FD_ZERO(s)		  FD_ZERO(s)

typedef fd_set ortp_fd_set;



typedef struct _SessionSet
{
	ortp_fd_set rtpset;
}SessionSet;



#define session_set_init(ss)		ORTP_FD_ZERO(&(ss)->rtpset)

ORTP_PUBLIC SessionSet * session_set_new(void);
/**
 * This macro adds the rtp session to the set.
 * @param ss a set (SessionSet object)
 * @param rtpsession a RtpSession
**/
#define session_set_set(ss,rtpsession)		ORTP_FD_SET((rtpsession)->mask_pos,&(ss)->rtpset)

/**
 * This macro tests if the session is part of the set. 1 is returned if true, 0 else.
 *@param ss a set
 *@param rtpsession a rtp session
 *
**/
#define session_set_is_set(ss,rtpsession)	ORTP_FD_ISSET((rtpsession)->mask_pos,&(ss)->rtpset)

/**
 * Removes the session from the set.
 *@param ss a set of sessions.
 *@param rtpsession a rtp session.
 *
 *
**/
#define session_set_clr(ss,rtpsession)		ORTP_FD_CLR((rtpsession)->mask_pos,&(ss)->rtpset)

#define session_set_copy(dest,src)		memcpy(&(dest)->rtpset,&(src)->rtpset,sizeof(ortp_fd_set))


/**
 * Frees a SessionSet.
**/
ORTP_PUBLIC void session_set_destroy(SessionSet *set);


ORTP_PUBLIC int session_set_select(SessionSet *recvs, SessionSet *sends, SessionSet *errors);
ORTP_PUBLIC int session_set_timedselect(SessionSet *recvs, SessionSet *sends, SessionSet *errors,  struct timeval *timeout);

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef rtpsignaltable_h
#define rtpsignaltable_h
#ifdef __cplusplus
extern "C"
{
#endif
#include "rtpsession_priv.h"





void rtp_signal_table_init(RtpSignalTable *table, RtpSession *session, const char *signal_name);

int rtp_signal_table_add(RtpSignalTable *table,RtpCallback cb, void *user_data);

void rtp_signal_table_emit(RtpSignalTable *table);

/* emit but with a second arg */
void rtp_signal_table_emit2(RtpSignalTable *table, void *arg);

/* emit but with a third arg */
void rtp_signal_table_emit3(RtpSignalTable *table, void *arg1, void *arg2);

int rtp_signal_table_remove_by_callback(RtpSignalTable *table,RtpCallback cb);

#ifdef __cplusplus
}
#endif
#endif


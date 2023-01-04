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



#include <ortp/port.h>
#include <ortp/logging.h>




static OrtpLogFunc _ortp_log_func = NULL;
static unsigned int _ortp_log_level = ORTP_LOGLEV_END;

void ortp_set_log_handler(OrtpLogFunc func){
    _ortp_log_func = func;
}

OrtpLogFunc ortp_get_log_handler(void){
    return _ortp_log_func;
}



void ortp_set_log_level(int level)
{
    _ortp_log_level = level;
}

unsigned int ortp_get_log_level(void)
{
    return _ortp_log_level;
}

void ortp_hdr_log_out(char *buf)
{
    /*rtp_header_t *rtp = (rtp_header_t *)buf;
    fprintf(stdout, "============================================\r\n");
    fprintf(stdout," version          %d\r\n", rtp->version);
    fprintf(stdout," padbit           %d\r\n", rtp->padbit);
    fprintf(stdout," extbit           %d\r\n", rtp->extbit);
    fprintf(stdout," markbit          %d\r\n", rtp->markbit);
    fprintf(stdout," cc               %d\r\n", rtp->cc);
    fprintf(stdout," paytype          %d\r\n", rtp->paytype);
    fprintf(stdout," ssrc             %u\r\n", ntohl(rtp->ssrc));
    fprintf(stdout," timestamp        %u\r\n", ntohl(rtp->timestamp));
    fprintf(stdout," seq_number       %u\r\n", ntohl(rtp->seq_number));
    fprintf(stdout, "============================================\r\n");
    fflush(stdout);*/
}



void ortp_log_out(OrtpLogLevel level, const char *func, int line, const char *fmt,...)
{
    va_list args;
    if(level > _ortp_log_level)
        return;
    va_start (args, fmt);
    switch(level)
    {
    case ORTP_FATAL:
        fprintf(stdout, "FATAL:%s[%d]", func, line);
    break;
    case ORTP_ERROR:
        fprintf(stdout, "ERROR:%s[%d]", func, line);
    break;
    case ORTP_WARNING:
        fprintf(stdout, "WARNING:%s[%d]", func, line);
    break;
    case ORTP_MESSAGE:
        fprintf(stdout, "MESSAGE:%s[%d]", func, line);
    break;
    case ORTP_TRACE:
        fprintf(stdout, "TRACE:%s[%d]", func, line);
    break;
    case ORTP_DEBUG:
        fprintf(stdout, "DEBUG:%s[%d]", func, line);
    break;
    case ORTP_END:
        fprintf(stdout, "DEBUG:%s[%d]", func, line);
    break;
    case ORTP_LOGLEV_END:
        fprintf(stdout, "DEBUG:%s[%d]", func, line);
    break;
    }
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}


void ortp_log(OrtpLogLevel level,const char *fmt,...)
{
    va_list args;
    if(level > _ortp_log_level)
        return;
    va_start (args, fmt);
    switch(level)
    {
    case 0:
        fprintf(stdout, "FATAL:");
    break;
    case 1:
        fprintf(stdout, "ERROR:");
    break;
    case 2:
        fprintf(stdout, "WARNING:");
    break;
    case 3:
        fprintf(stdout, "MESSAGE:");
    break;
    case 4:
        fprintf(stdout, "TRACE:");
    break;
    case 5:
        fprintf(stdout, "DEBUG:");
    break;
    }
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}

#ifdef ORTP_WINDOWS_DESKTOP
void ortp_message(const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    fprintf(stdout, "MESSAGE:");
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
void ortp_warning(const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    fprintf(stdout, "WARNING:");
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
void ortp_error(const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    fprintf(stdout, "ERROR:");
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
void ortp_fatal(const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    fprintf(stdout, "FATAL:");
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
void ortp_debug(const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    fprintf(stdout, "DEBUG:");
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
#endif
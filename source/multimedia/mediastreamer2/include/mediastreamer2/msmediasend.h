/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2010  Belledonne Communications SARL (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MS_MEDIA_SEND_H
#define MS_MEDIA_SEND_H

#include <mediastreamer2/mssndcard.h>
#include <mediastreamer2/msinterfaces.h>
#include <mediastreamer2/msvideo.h>

/**
 * Media file player
 */
typedef struct _MSMediaSend MSMediaSend;

/**
 * Callbacks definitions */
typedef void (*MSMediaSendEofCallback)(void *user_data);

typedef enum {
	MS_FILE_FORMAT_UNKNOWN,
	MS_FILE_FORMAT_WAVE,
	MS_FILE_FORMAT_H263,
	MS_FILE_FORMAT_H264,
	MS_FILE_FORMAT_H265,
	MS_FILE_FORMAT_MJPEG,
	MS_FILE_FORMAT_MATROSKA
} MSFileFormat;


#ifdef __cplusplus
extern "C"{
#endif

/**
 * Instanciate a media player
 * @param factory a MSFactory
 * @param snd_card Playback sound card
 * @param video_display_name Video out
 * @param window_id Pointer on the drawing window
 * @return A pointer on the created MSMediaSend
 */
MS2_PUBLIC MSMediaSend *ms_media_send_new(MSFactory *factory, MSSndCard *snd_card, const char *video_display_name, void *window_id);

/**
 * Free a media player
 * @param obj Pointer on the MSMediaSend to free
 */
MS2_PUBLIC void ms_media_send_free(MSMediaSend *obj);

/**
 * Get the window ID
 * @param obj The player
 * @return The window ID
 */
MS2_PUBLIC void * ms_media_send_get_window_id(const MSMediaSend *obj);

/**
 * Set the "End of File" callback
 * @param obj The player
 * @param cb Function to call
 * @param user_data Data which will be passed to the function
 */
MS2_PUBLIC void ms_media_send_set_eof_callback(MSMediaSend *obj, MSMediaSendEofCallback cb, void *user_data);

/**
 * Require the player for playing the file again when the end is reached. Then,
 * the player loops indefinitely until ms_media_send_stop() is called. That function
 * can be called while the player is running.
 * @param obj The MSMediaSend instance
 * @param loop_interval_ms Time interval beetween two plays. If a negative value is
 * set, the player does not loop.
 */
MS2_PUBLIC void ms_media_send_set_loop(MSMediaSend *obj, int loop_interval_ms);

/**
 * Open a media file
 * @param obj The player
 * @param filepath Path of the file to open
 * @return TRUE if the file could be opened
 */
MS2_PUBLIC bool_t ms_media_send_open(MSMediaSend *obj, const char *filepath);

/**
 * Close a media file
 * That function can be safly call even if no file has been opend
 * @param obj The player
 */
MS2_PUBLIC void ms_media_send_close(MSMediaSend *obj);

/**
 * Start playback
 * @param obj The player
 * @return TRUE if playback has been successfuly started
 */
MS2_PUBLIC bool_t ms_media_send_start(MSMediaSend *obj);

/**
 * Stop a playback
 * When a playback is stoped, the player automatically seek at
 * the begining of the file.
 * @param obj The player
 */
MS2_PUBLIC void ms_media_send_stop(MSMediaSend *obj);

/**
 * Turn playback to paused.
 * @param obj The player
 */
MS2_PUBLIC void ms_media_send_pause(MSMediaSend *obj);

/**
 * Seek into the opened file
 * Can be safly call when playback is runing
 * @param obj The player
 * @param seek_pos_ms Position where to seek on (in milliseconds)
 * @return
 */
MS2_PUBLIC bool_t ms_media_send_seek(MSMediaSend *obj, int seek_pos_ms);

/**
 * Get the state of the player
 * @param obj The player
 * @return An MSPLayerSate enum
 */
MS2_PUBLIC MSPlayerState ms_media_send_get_state(MSMediaSend *obj);

/**
 * Get the duration of the opened media
 * @param obj The player
 * @return The duration in milliseconds. -1 if failure
 */
MS2_PUBLIC int ms_media_send_get_duration(MSMediaSend *obj);

/**
 * Get the position of the playback
 * @param obj The player
 * @return The position in milliseconds. -1 if failure
 */
MS2_PUBLIC int ms_media_send_get_current_position(MSMediaSend *obj);

/**
 * Check whether Matroska format is supported by the player
 * @return TRUE if supported
 */
MS2_PUBLIC bool_t ms_media_send_matroska_supported(void);

/**
 * Return format of the current opened file
 * @param obj Player
 * @return Format of the file. UNKNOWN_FORMAT when no file is opened
 */
MS2_PUBLIC MSFileFormat ms_media_send_get_file_format(const MSMediaSend *obj);

#ifdef __cplusplus
}
#endif


#endif

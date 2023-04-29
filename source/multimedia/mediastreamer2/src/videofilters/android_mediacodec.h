/*
mediastreamer2 android_mediacodec.h
Copyright (C) 2015 Belledonne Communications SARL

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
#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AMEDIA_OK = 0,

    AMEDIA_ERROR_BASE                  = -10000,
    AMEDIA_ERROR_UNKNOWN               = AMEDIA_ERROR_BASE,
    AMEDIA_ERROR_MALFORMED             = AMEDIA_ERROR_BASE - 1,
    AMEDIA_ERROR_UNSUPPORTED           = AMEDIA_ERROR_BASE - 2,
    AMEDIA_ERROR_INVALID_OBJECT        = AMEDIA_ERROR_BASE - 3,
    AMEDIA_ERROR_INVALID_PARAMETER     = AMEDIA_ERROR_BASE - 4,

    AMEDIA_DRM_ERROR_BASE              = -20000,
    AMEDIA_DRM_NOT_PROVISIONED         = AMEDIA_DRM_ERROR_BASE - 1,
    AMEDIA_DRM_RESOURCE_BUSY           = AMEDIA_DRM_ERROR_BASE - 2,
    AMEDIA_DRM_DEVICE_REVOKED          = AMEDIA_DRM_ERROR_BASE - 3,
    AMEDIA_DRM_SHORT_BUFFER            = AMEDIA_DRM_ERROR_BASE - 4,
    AMEDIA_DRM_SESSION_NOT_OPENED      = AMEDIA_DRM_ERROR_BASE - 5,
    AMEDIA_DRM_TAMPER_DETECTED         = AMEDIA_DRM_ERROR_BASE - 6,
    AMEDIA_DRM_VERIFY_FAILED           = AMEDIA_DRM_ERROR_BASE - 7,
    AMEDIA_DRM_NEED_KEY                = AMEDIA_DRM_ERROR_BASE - 8,
    AMEDIA_DRM_LICENSE_EXPIRED         = AMEDIA_DRM_ERROR_BASE - 9,

} media_status_t;

typedef struct {
	int format;
	int width;
	int height;
	MSRect crop_rect;
	uint64_t timestamp;
	int nplanes;
	int row_strides[4];
	int pixel_strides[4];
	uint8_t *buffers[4];
	void *priv_ptr;
} AMediaImage;

/*
void AMediaCodec_reset(AMediaCodec *codec);
void AMediaCodec_setParams(AMediaCodec *codec, const char *params);
bool AMediaCodec_getInputImage(AMediaCodec *codec, int index, AMediaImage *image);
bool AMediaCodec_getOutputImage(AMediaCodec *codec, int index, AMediaImage *image);
void AMediaImage_close(AMediaImage *image);
bool_t AMediaImage_isAvailable(void);
*/
#ifdef __cplusplus
} // extern "C"
#endif

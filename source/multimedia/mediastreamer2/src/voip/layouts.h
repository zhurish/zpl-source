/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of mediastreamer2 
 * (see https://gitlab.linphone.org/BC/public/mediastreamer2).
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

#ifndef mslayouts_hh
#define mslayouts_hh

#include "mediastreamer2/msvideo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MS_LAYOUT_MIN_SIZE 40

void ms_layout_center_rectangle(MSVideoSize wsize, MSVideoSize vsize, MSRect *rect);
	
void ms_layout_compute(MSVideoSize wsize, MSVideoSize vsize, MSVideoSize orig_psize,
                       int localrect_pos, float scalefactor, MSRect *mainrect, MSRect *localrect);

#ifdef __cplusplus
}
#endif

#endif

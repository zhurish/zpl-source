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

#include <stdexcept>

#include "h264-utils.h"
#include "h265-utils.h"

#include "h26x-utils.h"

using namespace std;

namespace mediastreamer {

void H26xUtils::naluStreamToNalus(const std::vector<uint8_t> &byteStream, os_queue_t *out) {
	H26xUtils::naluStreamToNalus(byteStream.data(), byteStream.size(), out);
}

void H26xUtils::naluStreamToNalus(const uint8_t *bytestream, size_t size, os_queue_t *out) {
	const uint8_t *ptr = bytestream;
	while (ptr < bytestream + size) {
		uint32_t nalu_size;
		memcpy(&nalu_size, ptr, 4);
		nalu_size = ntohl(nalu_size);

		os_qdata_t *nalu = os_qdata_create(out, nalu_size);
		memcpy(nalu->b_wptr, ptr + 4, nalu_size);
		ptr += nalu_size + 4;
		nalu->b_wptr += nalu_size;

		os_queue_add(out, nalu);
	}
}

void H26xUtils::byteStreamToNalus(const std::vector<uint8_t> &byteStream, os_queue_t *out, bool removePreventionBytes) {
	H26xUtils::byteStreamToNalus(byteStream.data(), byteStream.size(), out, removePreventionBytes);
}

static bool isPictureStartCode(const uint8_t *bytestream, size_t size){
	if (size <= 4) return false;
	if (bytestream[0] == 0 && bytestream[1] == 0 && bytestream[2] == 0 && bytestream[3] == 1) return true;
	return false;
}

os_qdata_t * H26xUtils::makeNalu(const uint8_t *byteStream, size_t naluSize, bool removePreventionBytes, int *preventionBytesRemoved){
	os_qdata_t *nalu = os_qdata_create(NULL, naluSize);
	const uint8_t *it;
	const uint8_t *end = byteStream + naluSize;
	for(it = byteStream; it < end ;){
		if (removePreventionBytes && it[0] == 0 && it + 3 < end && it[1] == 0 && it[2] == 3 && it[3] == 1){
			/* Found 0x00000301, replace by 0x000001*/
			it += 3;
			*nalu->b_wptr++ = 0;
			*nalu->b_wptr++ = 0;
			*nalu->b_wptr++ = 1;
			(*preventionBytesRemoved)++;
		}else{
			*nalu->b_wptr++ = *it++;
		}
	}
	return nalu;
}

void H26xUtils::byteStreamToNalus(const uint8_t *byteStream, size_t size, os_queue_t *out, bool removePreventionBytes) {
	int preventionBytesRemoved = 0;
	size_t i;
	size_t begin, end;
	size_t naluSize;
	
	if (!isPictureStartCode(byteStream, size)){
		ms_error("no picture start code found in H26x byte stream");
		throw invalid_argument("no picutre start code found in H26x byte stream");
		return;
	}
	begin = 4;
	for (i = begin; i + 3 < size ; ++i){
		if (byteStream[i] == 0 && byteStream[i+1] == 0 && byteStream[i+2] == 1){
			end = i;
			naluSize = end - begin;
			os_queue_add(out, makeNalu(byteStream + begin, naluSize, removePreventionBytes, &preventionBytesRemoved));
			i+=3;
			begin = i;
		}
	}
	naluSize = size - begin;
	os_queue_add(out, makeNalu(byteStream + begin, naluSize, removePreventionBytes, &preventionBytesRemoved));
	
	if (preventionBytesRemoved > 0){
		ms_message("Removed %i start code prevention bytes", preventionBytesRemoved);
	}
}

size_t H26xUtils::nalusToByteStream(os_queue_t *nalus, uint8_t* byteStream, size_t size) {
	bool startPicture = true;
	uint8_t *byteStreamEnd = byteStream + size;
	uint8_t *it = byteStream;
	
	if (size < 4) throw invalid_argument("Insufficient buffer size");
	
	while (os_qdata_t *im = os_queue_get(nalus)) {
		if (startPicture) {
			// starting picture extra zero byte
			*it++ = 0;
			startPicture = false;
		}

		// starting NALu marker
		*it++ = 0;
		*it++ = 0;
		*it++ = 1;

		// copy NALu content
		for (const uint8_t *src = im->b_rptr; src < im->b_wptr && it < byteStreamEnd ;) {
			if (src[0] == 0 && src+2 < im->b_wptr && src[1] == 0 && (/*src[2] == 0 ||*/ src[2] == 1)) {
				if (it + 3 <  byteStreamEnd){
					*it++ = 0;
					*it++ = 0;
					*it++ = 3; // emulation prevention three byte
					src += 2;
				}else throw invalid_argument("Insufficient buffer size");
			} else {
				*it++ = *src++;
			}
		}
		os_queue_finsh(nalus, im);
		if (it == byteStreamEnd) throw invalid_argument("Insufficient buffer size");
	}
	return it - byteStream;
}

void H26xParameterSetsInserter::replaceParameterSet(os_qdata_t *&ps, os_qdata_t *newPs) {
	if (ps) os_qdata_destroy(ps);
	ps = newPs;
}

H26xParameterSetsStore::H26xParameterSetsStore(const std::string &mime, const std::initializer_list<int> &psCodes) {
	_naluHeader.reset(H26xToolFactory::get(mime).createNaluHeader());
	for (int psCode : psCodes) {
		_ps[psCode] = nullptr;
	}
}
H26xParameterSetsStore::H26xParameterSetsStore(const char *mime, const std::initializer_list<int> &psCodes)
{
	std::string lomime = mime;
	_naluHeader.reset(H26xToolFactory::get(lomime).createNaluHeader());
	for (int psCode : psCodes) {
		_ps[psCode] = nullptr;
	}
}
H26xParameterSetsStore::~H26xParameterSetsStore() {
	for(auto it = _ps.begin(); it != _ps.end(); it++) {
		if (it->second) os_qdata_destroy(it->second);
	}
}

bool H26xParameterSetsStore::psGatheringCompleted() const {
	for(const auto &item : _ps) {
		if (item.second == nullptr) return false;
	}
	return true;
}

void H26xParameterSetsStore::extractAllPs(os_queue_t *frame) {
	NODE node;
	os_qdata_t *nalu = NULL;
	if (frame->mutex)
		os_mutex_lock(frame->mutex, OS_WAIT_FOREVER);
	for (nalu = (os_qdata_t *)lstFirst(&frame->list); nalu != NULL;
		 nalu = (os_qdata_t *)lstNext(&node))
	{
		node = nalu->node;
		if (nalu && nalu->skb_len)
		{
			_naluHeader->parse(nalu->b_rptr);
			int type = _naluHeader->getAbsType();
			if (_ps.find(type) != _ps.end()) {
				lstDelete(&frame->list, (NODE *)nalu);
				addPs(type, nalu);
				continue;
			}
		}
	}
	if (frame->mutex)
		os_mutex_unlock(frame->mutex);
	/*
	for (os_qdata_t *nalu = ms_queue_peek_first(frame); !ms_queue_end(frame, nalu);) {
		_naluHeader->parse(nalu->b_rptr);
		int type = _naluHeader->getAbsType();
		if (_ps.find(type) != _ps.end()) {
			os_qdata_t *ps = nalu;
			nalu = ms_queue_next(frame, nalu);
			ms_queue_remove(frame, ps);
			addPs(type, ps);
			continue;
		}
		nalu = ms_queue_next(frame, nalu);
	}*/
}

void H26xParameterSetsStore::fetchAllPs(os_queue_t *outq) const {
	os_queue_t q;
	os_queue_init(&q, "q", 0, 0);
	for(const auto &item : _ps) {
		if (item.second) {
			os_queue_add(outq, os_qdata_clone(outq, item.second));
		}
	}
}

void H26xParameterSetsStore::addPs(int naluType, os_qdata_t *nalu) {
	bool replaceParam = false;
	os_qdata_t *lastPs = _ps[naluType];

	if (lastPs == nullptr || nalu == nullptr) {
		replaceParam = true;
	} else {
		ssize_t naluSize = (ssize_t)(nalu->b_wptr - nalu->b_rptr);
		ssize_t lastPsSize = (ssize_t)(lastPs->b_wptr - lastPs->b_rptr);
		if (naluSize != lastPsSize || memcmp(nalu->b_rptr, lastPs->b_rptr, naluSize) != 0) {
			replaceParam = true;
		}
	}

	if (replaceParam) {
		if (lastPs) os_queue_finsh(nalu, lastPs);
		_ps[naluType] = nalu ? os_qdata_clone(NULL, nalu) : nullptr;
		_newParameters = true;
	}
}

const H26xToolFactory &H26xToolFactory::get(const std::string &mime) {
	unique_ptr<H26xToolFactory> &instance = _instances[mime];
	if (instance == nullptr) {
		if (mime == "video/avc") instance.reset(new H264ToolFactory());
		else if (mime == "video/hevc") instance.reset(new H265ToolFactory());
		else throw invalid_argument("no H26xToolFactory class associated to '" + mime + "' mime");
	}
	return *instance;
}

std::unordered_map<std::string, std::unique_ptr<H26xToolFactory>> H26xToolFactory::_instances;

} // namespace mediastreamer

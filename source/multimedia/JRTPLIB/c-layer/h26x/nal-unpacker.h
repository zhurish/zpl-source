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

#pragma once

#include <memory>

#include "os_queue.h"

namespace mediastreamer {

class NalUnpacker {
public:
	struct Status {
		bool frameAvailable = false;
		bool frameCorrupted = false;
		bool isKeyFrame = false;

		Status &operator|=(const Status &s2);
		unsigned int toUInt() const;
	};

	class FuAggregatorInterface {
	public:
		virtual ~FuAggregatorInterface() {if (_m) freemsg(_m);}
		virtual os_qdata_t *feed(os_qdata_t *packet) = 0;
		virtual bool isAggregating() const = 0;
		virtual void reset() = 0;
		virtual os_qdata_t *completeAggregation() = 0;

	protected:
		os_qdata_t *_m = nullptr;
	};

	class ApSpliterInterface {
	public:
		ApSpliterInterface() {ms_queue_init(&_q);}
		virtual ~ApSpliterInterface() {ms_queue_flush(&_q);}
		virtual void feed(os_qdata_t *packet) = 0;
		virtual os_queue_t *getNalus() = 0;

	protected:
		os_queue_t _q;
	};

	NalUnpacker(FuAggregatorInterface *aggregator, ApSpliterInterface *spliter);
	virtual ~NalUnpacker() {ms_queue_flush(&_q);}

	/**
	 * Process incoming rtp data and output NALUs, whenever possible.
	 * @param ctx the Rfc3984Context object
	 * @param im a new H264 packet to process
	 * @param naluq a os_queue_t into which a frame ready to be decoded will be output, in the form of a sequence of NAL units.
	 * @return a bitmask of Rfc3984Status values.
	 * The return value is a bitmask of the #Rfc3984Status enum.
	 **/
	 Status unpack(os_qdata_t *im, os_queue_t *out);
	void reset();

protected:
	enum class PacketType {
	    SingleNalUnit,
	    AggregationPacket,
	    FragmentationUnit
	};

	virtual Status outputFrame(os_queue_t *out, const Status &flags);
	virtual void storeNal(os_qdata_t *nal);

	virtual PacketType getNaluType(const os_qdata_t *nalu) const = 0;

	os_queue_t _q;
	Status _status;
	uint32_t _lastTs = 0x943FEA43;
	bool _initializedRefCSeq = false;
	uint16_t _refCSeq = 0;
	std::unique_ptr<FuAggregatorInterface> _fuAggregator;
	std::unique_ptr<ApSpliterInterface> _apSpliter;
};

}

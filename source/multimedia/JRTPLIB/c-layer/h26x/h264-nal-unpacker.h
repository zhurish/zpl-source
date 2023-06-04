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

#include "nal-unpacker.h"

namespace mediastreamer {

class H264FuaAggregator: public NalUnpacker::FuAggregatorInterface {
public:
	os_qdata_t *feed(os_qdata_t *im) override;
	bool isAggregating() const override {return _m != nullptr;}
	void reset() override;
	os_qdata_t *completeAggregation() override;
};

class H264StapaSpliter: public NalUnpacker::ApSpliterInterface {
public:
	void feed(os_qdata_t *im) override;
	os_queue_t *getNalus() override {return &_q;}
};

class H264NalUnpacker: public NalUnpacker {
public:
	H264NalUnpacker(): NalUnpacker(new H264FuaAggregator(), new H264StapaSpliter()) {}
	~H264NalUnpacker();

	void setOutOfBandSpsPps(os_qdata_t *sps, os_qdata_t *pps);

private:
	NalUnpacker::PacketType getNaluType(const os_qdata_t *nalu) const override;
	Status outputFrame(os_queue_t *out, const Status &flags) override;

	os_qdata_t *_sps = nullptr;
	os_qdata_t *_pps = nullptr;
};

} // namespace mediastreamer

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

#include "nal-packer.h"

namespace mediastreamer {

class H264NalPacker: public NalPacker {
public:
	enum PacketizationMode {
		SingleNalUnitMode,
		NonInterleavedMode
	};

	H264NalPacker(size_t maxPayloadSize): NalPacker(new NaluAggregator(maxPayloadSize), new NaluSpliter(maxPayloadSize), maxPayloadSize) {}

private:

	class NaluAggregator: public NaluAggregatorInterface {
	public:
		using NaluAggregatorInterface::NaluAggregatorInterface;
		~NaluAggregator() override {reset();}

		os_qdata_t *feed(os_qdata_t *nalu) override;
		bool isAggregating() const override {return _stap != nullptr;}
		void reset() override;
		os_qdata_t *completeAggregation() override;

	private:
		static os_qdata_t *concatNalus(os_qdata_t *m1, os_qdata_t *m2);
		static os_qdata_t *prependStapA(os_qdata_t *m);
		static void putNalSize(os_qdata_t *m, size_t sz);

		os_qdata_t *_stap = nullptr;
		size_t _size = 0;
	};

	class NaluSpliter: public NaluSpliterInterface {
	public:
		using NaluSpliterInterface::NaluSpliterInterface;
		void feed(os_qdata_t *nalu) override;
	};

};

} // namespace mediastreamer

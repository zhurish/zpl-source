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

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "os_queue.h"

#include "nal-packer.h"
#include "nal-unpacker.h"

namespace mediastreamer {


class H26xNaluType {
public:
	H26xNaluType() = default;
	H26xNaluType(uint8_t value);
	virtual ~H26xNaluType() = default;

	operator uint8_t() const {return _value;}

	virtual bool isVcl() const = 0;
	virtual bool isParameterSet() const = 0;
	virtual bool isKeyFramePart() const = 0;

protected:
	uint8_t _value = 0;
};

class H26xNaluHeader {
public:
	H26xNaluHeader() = default;
	virtual ~H26xNaluHeader() = default;

	void setFBit(bool val) {_fBit = val;}
	bool getFBit() const {return _fBit;}

	virtual const H26xNaluType &getAbsType() const = 0;

	virtual void parse(const uint8_t *header) = 0;
	virtual os_qdata_t *forge() const = 0;

protected:
	bool _fBit = false;
};

class  H26xUtils {
public:
	H26xUtils() = delete;

	static void naluStreamToNalus(const std::vector<uint8_t> &byteStream, os_queue_t *out);
	static void naluStreamToNalus(const uint8_t *byteStream, size_t size, os_queue_t *out);

	static void byteStreamToNalus(const std::vector<uint8_t> &byteStream, os_queue_t *out, bool removePreventionBytes = true);
	static void byteStreamToNalus(const uint8_t *byteStream, size_t size, os_queue_t *out, bool removePreventionBytes = true);

	/* Convert nalus to byte stream. If byteStream buffer is not large enough std::invalid_argument is thrown.*/
	static size_t nalusToByteStream(os_queue_t *nalus, uint8_t* byteStream, size_t size);
	
private:
	static os_qdata_t * makeNalu(const uint8_t *byteStream, size_t naluSize, bool removePreventionBytes, int *preventionBytesRemoved);
};


class H26xParameterSetsInserter {
public:
	virtual ~H26xParameterSetsInserter() = default;
	virtual void process(os_queue_t *in, os_queue_t *out) = 0;
	virtual void flush() = 0;

protected:
	static void replaceParameterSet(os_qdata_t *&ps, os_qdata_t *newPs);
};

class H26xParameterSetsStore {
public:
	virtual ~H26xParameterSetsStore();

	bool hasNewParameters() const {return _newParameters;}
	void acknowlege() {_newParameters = false;}

	void extractAllPs(os_queue_t *frame);
	bool psGatheringCompleted() const;
	void fetchAllPs(os_queue_t *outq) const;

protected:
	H26xParameterSetsStore(const std::string &mime, const std::initializer_list<int> &psCodes);
	H26xParameterSetsStore(const char *, const std::initializer_list<int> &psCodes);
	void addPs(int naluType, os_qdata_t *nalu);

	std::map<int, os_qdata_t *> _ps;
	std::unique_ptr<H26xNaluHeader> _naluHeader;
	bool _newParameters = false;
};

class H26xToolFactory {
public:
	H26xToolFactory() = default;
	virtual ~H26xToolFactory() = default;

	 static const H26xToolFactory &get(const std::string &mime);

	virtual H26xNaluHeader *createNaluHeader() const = 0;
	virtual NalPacker *createNalPacker(size_t maxPayloadSize) const = 0;
	virtual NalUnpacker *createNalUnpacker() const = 0;
	virtual H26xParameterSetsInserter *createParameterSetsInserter() const = 0;
	virtual H26xParameterSetsStore *createParameterSetsStore() const = 0;

private:
	static std::unordered_map<std::string, std::unique_ptr<H26xToolFactory>> _instances;
};

}
